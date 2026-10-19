#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MINIMP3_ENABLE_DEC_DURATION_ONLY
#define MINIMP3_MAX_SAMPLES_PER_FRAME (1152*2)

/* return error codes */
#define MP3D_E_PARAM   -1
#define MP3D_E_MEMORY  -2
#define MP3D_E_IOERROR -3
#define MP3D_E_USER    -4  /* can be used to stop processing from callbacks without indicating specific error */
#define MP3D_E_DECODE  -5  /* decode error which can't be safely skipped, such as sample rate, layer and channels change */


#define MINIMP3_BUF_SIZE (16*1024) /* buffer which can hold minimum 10 consecutive mp3 frames (~16KB) worst case */
#define MAX_FREE_FORMAT_FRAME_SIZE  2304    /* more than ISO spec's */
#ifndef MAX_FRAME_SYNC_MATCHES
#define MAX_FRAME_SYNC_MATCHES      10
#endif /* MAX_FRAME_SYNC_MATCHES */

#define MAX_L3_FRAME_PAYLOAD_BYTES  MAX_FREE_FORMAT_FRAME_SIZE /* MUST be >= 320000/8/32000*1152 = 1440 */

#define MAX_BITRESERVOIR_BYTES      511
#define SHORT_BLOCK_TYPE            2
#define STOP_BLOCK_TYPE             3
#define MODE_MONO                   3
#define MODE_JOINT_STEREO           1
#define HDR_SIZE                    4
#define HDR_IS_MONO(h)              (((h[3]) & 0xC0) == 0xC0)
#define HDR_IS_MS_STEREO(h)         (((h[3]) & 0xE0) == 0x60)
#define HDR_IS_FREE_FORMAT(h)       (((h[2]) & 0xF0) == 0)
#define HDR_IS_CRC(h)               (!((h[1]) & 1))
#define HDR_TEST_PADDING(h)         ((h[2]) & 0x2)
#define HDR_TEST_MPEG1(h)           ((h[1]) & 0x8)
#define HDR_TEST_NOT_MPEG25(h)      ((h[1]) & 0x10)
#define HDR_TEST_I_STEREO(h)        ((h[3]) & 0x10)
#define HDR_TEST_MS_STEREO(h)       ((h[3]) & 0x20)
#define HDR_GET_STEREO_MODE(h)      (((h[3]) >> 6) & 3)
#define HDR_GET_STEREO_MODE_EXT(h)  (((h[3]) >> 4) & 3)
#define HDR_GET_LAYER(h)            (((h[1]) >> 1) & 3)
#define HDR_GET_BITRATE(h)          ((h[2]) >> 4)
#define HDR_GET_SAMPLE_RATE(h)      (((h[2]) >> 2) & 3)
#define HDR_GET_MY_SAMPLE_RATE(h)   (HDR_GET_SAMPLE_RATE(h) + (((h[1] >> 3) & 1) + ((h[1] >> 4) & 1))*3)
#define HDR_IS_FRAME_576(h)         ((h[1] & 14) == 2)
#define HDR_IS_LAYER_1(h)           ((h[1] & 6) == 6)

#define MINIMP3_MIN(a, b)           ((a) > (b) ? (b) : (a))
#define MINIMP3_MAX(a, b)           ((a) < (b) ? (b) : (a))

typedef int16_t mp3d_sample_t;

typedef struct
{
    float mdct_overlap[2][9*32], qmf_state[15*2*32];
    int reserv, free_format_bytes;
    unsigned char header[4], reserv_buf[511];
} mp3dec_t;

typedef struct
{
    int frame_bytes, frame_offset, channels, hz, layer, bitrate_kbps;
} mp3dec_frame_info_t;

typedef size_t (*MP3D_READ_CB)(void *buf, size_t size, void *user_data);
typedef int (*MP3D_SEEK_CB)(uint64_t position, void *user_data);
typedef int (*MP3D_PROGRESS_CB)(void *user_data, size_t file_size, uint64_t offset, mp3dec_frame_info_t *info);

typedef struct
{
    MP3D_READ_CB read;
    void *read_data;
    MP3D_SEEK_CB seek;
    void *seek_data;
} mp3dec_io_t;

typedef struct
{
    const uint8_t *buf;
    int pos, limit;
} bs_t;

typedef struct
{
    const uint8_t *sfbtab;
    uint16_t part_23_length, big_values, scalefac_compress;
    uint8_t global_gain, block_type, mixed_block_flag, n_long_sfb, n_short_sfb;
    uint8_t table_select[3], region_count[3], subblock_gain[3];
    uint8_t preflag, scalefac_scale, count1_table, scfsi;
} L3_gr_info_t;

typedef struct
{
    mp3d_sample_t *buffer;
    size_t samples; /* channels included, byte size = samples*sizeof(mp3d_sample_t) */
    int channels, hz, layer, avg_bitrate_kbps;
} mp3dec_file_info_t;

static void bs_init(bs_t *bs, const uint8_t *data, int bytes)
{
    bs->buf   = data;
    bs->pos   = 0;
    bs->limit = bytes*8;
}

static uint32_t get_bits(bs_t *bs, int n)
{
    uint32_t next, cache = 0, s = bs->pos & 7;
    int shl = n + s;
    const uint8_t *p = bs->buf + (bs->pos >> 3);
    if ((bs->pos += n) > bs->limit)
        return 0;
    next = *p++ & (255 >> s);
    while ((shl -= 8) > 0)
    {
        cache |= next << shl;
        next = *p++;
    }
    return cache | (next >> -shl);
}


static int hdr_valid(const uint8_t *h)
{
    return h[0] == 0xff &&
        ((h[1] & 0xF0) == 0xf0 || (h[1] & 0xFE) == 0xe2) &&
        (HDR_GET_LAYER(h) != 0) &&
        (HDR_GET_BITRATE(h) != 15) &&
        (HDR_GET_SAMPLE_RATE(h) != 3);
}

static int hdr_compare(const uint8_t *h1, const uint8_t *h2)
{
    return hdr_valid(h2) &&
        ((h1[1] ^ h2[1]) & 0xFE) == 0 &&
        ((h1[2] ^ h2[2]) & 0x0C) == 0 &&
        !(HDR_IS_FREE_FORMAT(h1) ^ HDR_IS_FREE_FORMAT(h2));
}

static unsigned hdr_bitrate_kbps(const uint8_t *h)
{
    static const uint8_t halfrate[2][3][15] = {
        { { 0,4,8,12,16,20,24,28,32,40,48,56,64,72,80 }, { 0,4,8,12,16,20,24,28,32,40,48,56,64,72,80 }, { 0,16,24,28,32,40,48,56,64,72,80,88,96,112,128 } },
        { { 0,16,20,24,28,32,40,48,56,64,80,96,112,128,160 }, { 0,16,24,28,32,40,48,56,64,80,96,112,128,160,192 }, { 0,16,32,48,64,80,96,112,128,144,160,176,192,208,224 } },
    };
    return 2*halfrate[!!HDR_TEST_MPEG1(h)][HDR_GET_LAYER(h) - 1][HDR_GET_BITRATE(h)];
}

static unsigned hdr_sample_rate_hz(const uint8_t *h)
{
    static const unsigned g_hz[3] = { 44100, 48000, 32000 };
    return g_hz[HDR_GET_SAMPLE_RATE(h)] >> (int)!HDR_TEST_MPEG1(h) >> (int)!HDR_TEST_NOT_MPEG25(h);
}

static unsigned hdr_frame_samples(const uint8_t *h)
{
    return HDR_IS_LAYER_1(h) ? 384 : (1152 >> (int)HDR_IS_FRAME_576(h));
}

static int hdr_frame_bytes(const uint8_t *h, int free_format_size)
{
    int frame_bytes = hdr_frame_samples(h)*hdr_bitrate_kbps(h)*125/hdr_sample_rate_hz(h);
    if (HDR_IS_LAYER_1(h))
    {
        frame_bytes &= ~3; /* slot align */
    }
    return frame_bytes ? frame_bytes : free_format_size;
}

static int hdr_padding(const uint8_t *h)
{
    return HDR_TEST_PADDING(h) ? (HDR_IS_LAYER_1(h) ? 4 : 1) : 0;
}

static int L3_read_side_info(bs_t *bs, L3_gr_info_t *gr, const uint8_t *hdr)
{
    static const uint8_t g_scf_long[8][23] = {
        { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
        { 12,12,12,12,12,12,16,20,24,28,32,40,48,56,64,76,90,2,2,2,2,2,0 },
        { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
        { 6,6,6,6,6,6,8,10,12,14,16,18,22,26,32,38,46,54,62,70,76,36,0 },
        { 6,6,6,6,6,6,8,10,12,14,16,20,24,28,32,38,46,52,60,68,58,54,0 },
        { 4,4,4,4,4,4,6,6,8,8,10,12,16,20,24,28,34,42,50,54,76,158,0 },
        { 4,4,4,4,4,4,6,6,6,8,10,12,16,18,22,28,34,40,46,54,54,192,0 },
        { 4,4,4,4,4,4,6,6,8,10,12,16,20,24,30,38,46,56,68,84,102,26,0 }
    };
    static const uint8_t g_scf_short[8][40] = {
        { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
        { 8,8,8,8,8,8,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0 },
        { 4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },
        { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 },
        { 4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
        { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0 },
        { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0 },
        { 4,4,4,4,4,4,4,4,4,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0 }
    };
    static const uint8_t g_scf_mixed[8][40] = {
        { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
        { 12,12,12,4,4,4,8,8,8,12,12,12,16,16,16,20,20,20,24,24,24,28,28,28,36,36,36,2,2,2,2,2,2,2,2,2,26,26,26,0 },
        { 6,6,6,6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,14,14,14,18,18,18,26,26,26,32,32,32,42,42,42,18,18,18,0 },
        { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,32,32,32,44,44,44,12,12,12,0 },
        { 6,6,6,6,6,6,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,24,24,24,30,30,30,40,40,40,18,18,18,0 },
        { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,10,10,10,12,12,12,14,14,14,18,18,18,22,22,22,30,30,30,56,56,56,0 },
        { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,6,6,6,10,10,10,12,12,12,14,14,14,16,16,16,20,20,20,26,26,26,66,66,66,0 },
        { 4,4,4,4,4,4,6,6,4,4,4,6,6,6,8,8,8,12,12,12,16,16,16,20,20,20,26,26,26,34,34,34,42,42,42,12,12,12,0 }
    };

    unsigned tables, scfsi = 0;
    int main_data_begin, part_23_sum = 0;
    int sr_idx = HDR_GET_MY_SAMPLE_RATE(hdr); sr_idx -= (sr_idx != 0);
    int gr_count = HDR_IS_MONO(hdr) ? 1 : 2;

    if (HDR_TEST_MPEG1(hdr))
    {
        gr_count *= 2;
        main_data_begin = get_bits(bs, 9);
        scfsi = get_bits(bs, 7 + gr_count);
    } else
    {
        main_data_begin = get_bits(bs, 8 + gr_count) >> gr_count;
    }

    do
    {
        if (HDR_IS_MONO(hdr))
        {
            scfsi <<= 4;
        }
        gr->part_23_length = (uint16_t)get_bits(bs, 12);
        part_23_sum += gr->part_23_length;
        gr->big_values = (uint16_t)get_bits(bs,  9);
        if (gr->big_values > 288)
        {
            return -1;
        }
        gr->global_gain = (uint8_t)get_bits(bs, 8);
        gr->scalefac_compress = (uint16_t)get_bits(bs, HDR_TEST_MPEG1(hdr) ? 4 : 9);
        gr->sfbtab = g_scf_long[sr_idx];
        gr->n_long_sfb  = 22;
        gr->n_short_sfb = 0;
        if (get_bits(bs, 1))
        {
            gr->block_type = (uint8_t)get_bits(bs, 2);
            if (!gr->block_type)
            {
                return -1;
            }
            gr->mixed_block_flag = (uint8_t)get_bits(bs, 1);
            gr->region_count[0] = 7;
            gr->region_count[1] = 255;
            if (gr->block_type == SHORT_BLOCK_TYPE)
            {
                scfsi &= 0x0F0F;
                if (!gr->mixed_block_flag)
                {
                    gr->region_count[0] = 8;
                    gr->sfbtab = g_scf_short[sr_idx];
                    gr->n_long_sfb = 0;
                    gr->n_short_sfb = 39;
                } else
                {
                    gr->sfbtab = g_scf_mixed[sr_idx];
                    gr->n_long_sfb = HDR_TEST_MPEG1(hdr) ? 8 : 6;
                    gr->n_short_sfb = 30;
                }
            }
            tables = get_bits(bs, 10);
            tables <<= 5;
            gr->subblock_gain[0] = (uint8_t)get_bits(bs, 3);
            gr->subblock_gain[1] = (uint8_t)get_bits(bs, 3);
            gr->subblock_gain[2] = (uint8_t)get_bits(bs, 3);
        } else
        {
            gr->block_type = 0;
            gr->mixed_block_flag = 0;
            tables = get_bits(bs, 15);
            gr->region_count[0] = (uint8_t)get_bits(bs, 4);
            gr->region_count[1] = (uint8_t)get_bits(bs, 3);
            gr->region_count[2] = 255;
        }
        gr->table_select[0] = (uint8_t)(tables >> 10);
        gr->table_select[1] = (uint8_t)((tables >> 5) & 31);
        gr->table_select[2] = (uint8_t)((tables) & 31);
        gr->preflag = HDR_TEST_MPEG1(hdr) ? get_bits(bs, 1) : (gr->scalefac_compress >= 500);
        gr->scalefac_scale = (uint8_t)get_bits(bs, 1);
        gr->count1_table = (uint8_t)get_bits(bs, 1);
        gr->scfsi = (uint8_t)((scfsi >> 12) & 15);
        scfsi <<= 4;
        gr++;
    } while(--gr_count);

    if (part_23_sum + bs->pos > bs->limit + main_data_begin*8)
    {
        return -1;
    }

    return main_data_begin;
}

static int mp3dec_check_vbrtag(const uint8_t *frame, int frame_size, uint32_t *frames, int *delay, int *padding)
{
    static const char g_xing_tag[4] = { 'X', 'i', 'n', 'g' };
    static const char g_info_tag[4] = { 'I', 'n', 'f', 'o' };
#define FRAMES_FLAG     1
#define BYTES_FLAG      2
#define TOC_FLAG        4
#define VBR_SCALE_FLAG  8
    /* Side info offsets after header:
    /                Mono  Stereo
    /  MPEG1          17     32
    /  MPEG2 & 2.5     9     17*/
    bs_t bs[1];
    L3_gr_info_t gr_info[4];
    bs_init(bs, frame + HDR_SIZE, frame_size - HDR_SIZE);
    if (HDR_IS_CRC(frame))
        get_bits(bs, 16);
    if (L3_read_side_info(bs, gr_info, frame) < 0)
        return 0; /* side info corrupted */

    const uint8_t *tag = frame + HDR_SIZE + bs->pos/8;
    if (memcmp(g_xing_tag, tag, 4) && memcmp(g_info_tag, tag, 4))
        return 0;
    int flags = tag[7];
    if (!((flags & FRAMES_FLAG)))
        return -1;
    tag += 8;
    *frames = (uint32_t)(tag[0] << 24) | (tag[1] << 16) | (tag[2] << 8) | tag[3];
    tag += 4;
    if (flags & BYTES_FLAG)
        tag += 4;
    if (flags & TOC_FLAG)
        tag += 100;
    if (flags & VBR_SCALE_FLAG)
        tag += 4;
    *delay = *padding = 0;
    if (*tag)
    {   /* extension, LAME, Lavc, etc. Should be the same structure. */
        tag += 21;
        if (tag - frame + 14 >= frame_size)
            return 0;
        *delay   = ((tag[0] << 4) | (tag[1] >> 4)) + (528 + 1);
        *padding = (((tag[1] & 0xF) << 8) | tag[2]) - (528 + 1);
    }
    return 1;
}

static void mp3dec_skip_id3v1(const uint8_t *buf, size_t *pbuf_size)
{
    size_t buf_size = *pbuf_size;
#ifndef MINIMP3_NOSKIP_ID3V1
    if (buf_size >= 128 && !memcmp(buf + buf_size - 128, "TAG", 3))
    {
        buf_size -= 128;
        if (buf_size >= 227 && !memcmp(buf + buf_size - 227, "TAG+", 4))
            buf_size -= 227;
    }
#endif
#ifndef MINIMP3_NOSKIP_APEV2
    if (buf_size > 32 && !memcmp(buf + buf_size - 32, "APETAGEX", 8))
    {
        buf_size -= 32;
        const uint8_t *tag = buf + buf_size + 8 + 4;
        uint32_t tag_size = (uint32_t)(tag[3] << 24) | (tag[2] << 16) | (tag[1] << 8) | tag[0];
        if (buf_size >= tag_size)
            buf_size -= tag_size;
    }
#endif
    *pbuf_size = buf_size;
}

static size_t mp3dec_skip_id3v2(const uint8_t *buf, size_t buf_size)
{
#define MINIMP3_ID3_DETECT_SIZE 10
#ifndef MINIMP3_NOSKIP_ID3V2
    if (buf_size >= MINIMP3_ID3_DETECT_SIZE && !memcmp(buf, "ID3", 3) && !((buf[5] & 15) || (buf[6] & 0x80) || (buf[7] & 0x80) || (buf[8] & 0x80) || (buf[9] & 0x80)))
    {
        size_t id3v2size = (((buf[6] & 0x7f) << 21) | ((buf[7] & 0x7f) << 14) | ((buf[8] & 0x7f) << 7) | (buf[9] & 0x7f)) + 10;
        if ((buf[5] & 16))
            id3v2size += 10; /* footer */
        return id3v2size;
    }
#endif
    return 0;
}

static void mp3dec_skip_id3(const uint8_t **pbuf, size_t *pbuf_size)
{
    uint8_t *buf = (uint8_t *)(*pbuf);
    size_t buf_size = *pbuf_size;
    size_t id3v2size = mp3dec_skip_id3v2(buf, buf_size);
    if (id3v2size)
    {
        if (id3v2size >= buf_size)
            id3v2size = buf_size;
        buf      += id3v2size;
        buf_size -= id3v2size;
    }
    mp3dec_skip_id3v1(buf, &buf_size);
    *pbuf = (const uint8_t *)buf;
    *pbuf_size = buf_size;
}

static int mp3d_match_frame(const uint8_t *hdr, int mp3_bytes, int frame_bytes)
{
    int i, nmatch;
    for (i = 0, nmatch = 0; nmatch < MAX_FRAME_SYNC_MATCHES; nmatch++)
    {
        i += hdr_frame_bytes(hdr + i, frame_bytes) + hdr_padding(hdr + i);
        if (i + HDR_SIZE > mp3_bytes)
            return nmatch > 0;
        if (!hdr_compare(hdr, hdr + i))
            return 0;
    }
    return 1;
}

static int mp3d_find_frame(const uint8_t *mp3, int mp3_bytes, int *free_format_bytes, int *ptr_frame_bytes)
{
    int i, k;
    for (i = 0; i < mp3_bytes - HDR_SIZE; i++, mp3++)
    {
        if (hdr_valid(mp3))
        {
            int frame_bytes = hdr_frame_bytes(mp3, *free_format_bytes);
            int frame_and_padding = frame_bytes + hdr_padding(mp3);

            for (k = HDR_SIZE; !frame_bytes && k < MAX_FREE_FORMAT_FRAME_SIZE && i + 2*k < mp3_bytes - HDR_SIZE; k++)
            {
                if (hdr_compare(mp3, mp3 + k))
                {
                    int fb = k - hdr_padding(mp3);
                    int nextfb = fb + hdr_padding(mp3 + k);
                    if (i + k + nextfb + HDR_SIZE > mp3_bytes || !hdr_compare(mp3, mp3 + k + nextfb))
                        continue;
                    frame_and_padding = k;
                    frame_bytes = fb;
                    *free_format_bytes = fb;
                }
            }
            if ((frame_bytes && i + frame_and_padding <= mp3_bytes &&
                mp3d_match_frame(mp3, mp3_bytes - i, frame_bytes)) ||
                (!i && frame_and_padding == mp3_bytes))
            {
                *ptr_frame_bytes = frame_and_padding;
                return i;
            }
            *free_format_bytes = 0;
        }
    }
    *ptr_frame_bytes = 0;
    return mp3_bytes;
}

int mp3dec_decode_frame(mp3dec_t *dec, const uint8_t *mp3, int mp3_bytes, mp3d_sample_t *pcm, mp3dec_frame_info_t *info)
{
    int i = 0, frame_size = 0;
    const uint8_t *hdr;
#ifndef MINIMP3_ENABLE_DEC_DURATION_ONLY
    int igr, success = 1;
    bs_t bs_frame[1];
    mp3dec_scratch_t scratch;
#endif

    if (mp3_bytes > 4 && dec->header[0] == 0xff && hdr_compare(dec->header, mp3))
    {
        frame_size = hdr_frame_bytes(mp3, dec->free_format_bytes) + hdr_padding(mp3);
        if (frame_size != mp3_bytes && (frame_size + HDR_SIZE > mp3_bytes || !hdr_compare(mp3, mp3 + frame_size)))
        {
            frame_size = 0;
        }
    }
    if (!frame_size)
    {
        memset(dec, 0, sizeof(mp3dec_t));
        i = mp3d_find_frame(mp3, mp3_bytes, &dec->free_format_bytes, &frame_size);
        if (!frame_size || i + frame_size > mp3_bytes)
        {
            info->frame_bytes = i;
            return 0;
        }
    }

    hdr = mp3 + i;
    memcpy(dec->header, hdr, HDR_SIZE);
    info->frame_bytes = i + frame_size;
    info->frame_offset = i;
    info->channels = HDR_IS_MONO(hdr) ? 1 : 2;
    info->hz = hdr_sample_rate_hz(hdr);
    info->layer = 4 - HDR_GET_LAYER(hdr);
    info->bitrate_kbps = hdr_bitrate_kbps(hdr);

    return hdr_frame_samples(hdr);
}

void mp3dec_init(mp3dec_t *dec)
{
    dec->header[0] = 0;
}

int mp3dec_load_cb(mp3dec_t *dec, mp3dec_io_t *io, uint8_t *buf, size_t buf_size, mp3dec_file_info_t *info, MP3D_PROGRESS_CB progress_cb, void *user_data)
{
    if (!dec || !buf || !info || (size_t)-1 == buf_size || (io && buf_size < MINIMP3_BUF_SIZE))
        return MP3D_E_PARAM;
    uint64_t detected_samples = 0;
    size_t orig_buf_size = buf_size;
    int to_skip = 0;
    mp3dec_frame_info_t frame_info;
    memset(info, 0, sizeof(*info));
    memset(&frame_info, 0, sizeof(frame_info));

    /* skip id3 */
    size_t filled = 0, consumed = 0;
    int eof = 0, ret = 0;
    if (io)
    {
        if (io->seek(0, io->seek_data))
            return MP3D_E_IOERROR;
        filled = io->read(buf, MINIMP3_ID3_DETECT_SIZE, io->read_data);
        if (filled > MINIMP3_ID3_DETECT_SIZE)
            return MP3D_E_IOERROR;
        if (MINIMP3_ID3_DETECT_SIZE != filled)
            return 0;
        size_t id3v2size = mp3dec_skip_id3v2(buf, filled);
        if (id3v2size)
        {
            if (io->seek(id3v2size, io->seek_data))
                return MP3D_E_IOERROR;
            filled = io->read(buf, buf_size, io->read_data);
            if (filled > buf_size)
                return MP3D_E_IOERROR;
        } else
        {
            size_t readed = io->read(buf + MINIMP3_ID3_DETECT_SIZE, buf_size - MINIMP3_ID3_DETECT_SIZE, io->read_data);
            if (readed > (buf_size - MINIMP3_ID3_DETECT_SIZE))
                return MP3D_E_IOERROR;
            filled += readed;
        }
        if (filled < MINIMP3_BUF_SIZE)
            mp3dec_skip_id3v1(buf, &filled);
    } else
    {
        mp3dec_skip_id3((const uint8_t **)&buf, &buf_size);
        if (!buf_size)
            return 0;
    }
    /* try to make allocation size assumption by first frame or vbr tag */
    mp3dec_init(dec);
    int samples;
    do
    {
        uint32_t frames;
        int i, delay, padding, free_format_bytes = 0, frame_size = 0;
        const uint8_t *hdr;
        if (io)
        {
            if (!eof && filled - consumed < MINIMP3_BUF_SIZE)
            {   /* keep minimum 10 consecutive mp3 frames (~16KB) worst case */
                memmove(buf, buf + consumed, filled - consumed);
                filled -= consumed;
                consumed = 0;
                size_t readed = io->read(buf + filled, buf_size - filled, io->read_data);
                if (readed > (buf_size - filled))
                    return MP3D_E_IOERROR;
                if (readed != (buf_size - filled))
                    eof = 1;
                filled += readed;
                if (eof)
                    mp3dec_skip_id3v1(buf, &filled);
            }
            i = mp3d_find_frame(buf + consumed, filled - consumed, &free_format_bytes, &frame_size);
            consumed += i;
            hdr = buf + consumed;
        } else
        {
            i = mp3d_find_frame(buf, buf_size, &free_format_bytes, &frame_size);
            buf      += i;
            buf_size -= i;
            hdr = buf;
        }
        if (i && !frame_size)
            continue;
        if (!frame_size)
            return 0;
        frame_info.channels = HDR_IS_MONO(hdr) ? 1 : 2;
        frame_info.hz = hdr_sample_rate_hz(hdr);
        frame_info.layer = 4 - HDR_GET_LAYER(hdr);
        frame_info.bitrate_kbps = hdr_bitrate_kbps(hdr);
        frame_info.frame_bytes = frame_size;
        samples = hdr_frame_samples(hdr)*frame_info.channels;
        if (3 != frame_info.layer)
            break;
        int ret = mp3dec_check_vbrtag(hdr, frame_size, &frames, &delay, &padding);
        if (ret > 0)
        {
            padding *= frame_info.channels;
            to_skip = delay*frame_info.channels;
            detected_samples = samples*(uint64_t)frames;
            if (detected_samples >= (uint64_t)to_skip)
                detected_samples -= to_skip;
            if (padding > 0 && detected_samples >= (uint64_t)padding)
                detected_samples -= padding;
            if (!detected_samples)
                return 0;
        }
        if (ret)
        {
            if (io)
            {
                consumed += frame_size;
            } else
            {
                buf      += frame_size;
                buf_size -= frame_size;
            }
        }
        break;
    } while(1);
    size_t allocated = MINIMP3_MAX_SAMPLES_PER_FRAME*sizeof(mp3d_sample_t);
    if (detected_samples)
        allocated += detected_samples*sizeof(mp3d_sample_t);
    else
        allocated += (buf_size/frame_info.frame_bytes)*samples*sizeof(mp3d_sample_t);
#ifndef MINIMP3_ENABLE_DEC_DURATION_ONLY
    info->buffer = (mp3d_sample_t*)malloc(allocated);
    if (!info->buffer)
        return MP3D_E_MEMORY;
#endif
    /* save info */
    info->channels = frame_info.channels;
    info->hz       = frame_info.hz;
    info->layer    = frame_info.layer;
    /* decode all frames */
    size_t avg_bitrate_kbps = 0, frames = 0;
    do
    {
#ifndef MINIMP3_ENABLE_DEC_DURATION_ONLY
        if ((allocated - info->samples*sizeof(mp3d_sample_t)) < MINIMP3_MAX_SAMPLES_PER_FRAME*sizeof(mp3d_sample_t))
        {
            allocated *= 2;
            mp3d_sample_t *alloc_buf = (mp3d_sample_t*)realloc(info->buffer, allocated);
            if (!alloc_buf)
                return MP3D_E_MEMORY;
            info->buffer = alloc_buf;
        }
#endif
        if (io)
        {
            if (!eof && filled - consumed < MINIMP3_BUF_SIZE)
            {   /* keep minimum 10 consecutive mp3 frames (~16KB) worst case */
                memmove(buf, buf + consumed, filled - consumed);
                filled -= consumed;
                consumed = 0;
                size_t readed = io->read(buf + filled, buf_size - filled, io->read_data);
                if (readed != (buf_size - filled))
                    eof = 1;
                filled += readed;
                if (eof)
                    mp3dec_skip_id3v1(buf, &filled);
            }
#ifdef MINIMP3_ENABLE_DEC_DURATION_ONLY
            samples = mp3dec_decode_frame(dec, buf + consumed, filled - consumed, NULL, &frame_info);
#else
            samples = mp3dec_decode_frame(dec, buf + consumed, filled - consumed, info->buffer + info->samples, &frame_info);
#endif
            consumed += frame_info.frame_bytes;
        } else
        {
            samples = mp3dec_decode_frame(dec, buf, MINIMP3_MIN(buf_size, (size_t)INT_MAX), info->buffer + info->samples, &frame_info);
            buf      += frame_info.frame_bytes;
            buf_size -= frame_info.frame_bytes;
        }
        if (samples)
        {
            if (info->hz != frame_info.hz || info->layer != frame_info.layer)
            {
                ret = MP3D_E_DECODE;
                break;
            }
            if (info->channels && info->channels != frame_info.channels)
            {
#ifdef MINIMP3_ALLOW_MONO_STEREO_TRANSITION
                info->channels = 0; /* mark file with mono-stereo transition */
#else
                ret = MP3D_E_DECODE;
                break;
#endif
            }
            samples *= frame_info.channels;
            if (to_skip)
            {
                size_t skip = MINIMP3_MIN(samples, to_skip);
                to_skip -= skip;
                samples -= skip;
#ifndef MINIMP3_ENABLE_DEC_DURATION_ONLY
                memmove(info->buffer, info->buffer + skip, samples*sizeof(mp3d_sample_t));
#endif
            }
            info->samples += samples;
            avg_bitrate_kbps += frame_info.bitrate_kbps;
            frames++;
            if (progress_cb)
            {
                ret = progress_cb(user_data, orig_buf_size, orig_buf_size - buf_size, &frame_info);
                if (ret)
                    break;
            }
        }
    } while (frame_info.frame_bytes);
    if (detected_samples && info->samples > detected_samples)
        info->samples = detected_samples; /* cut padding */
    /* reallocate to normal buffer size */
#ifndef MINIMP3_ENABLE_DEC_DURATION_ONLY
    if (allocated != info->samples*sizeof(mp3d_sample_t))
    {
        mp3d_sample_t *alloc_buf = (mp3d_sample_t*)realloc(info->buffer, info->samples*sizeof(mp3d_sample_t));
        if (!alloc_buf && info->samples)
            return MP3D_E_MEMORY;
        info->buffer = alloc_buf;
    }
#endif
    if (frames)
        info->avg_bitrate_kbps = avg_bitrate_kbps/frames;
    return ret;
}
