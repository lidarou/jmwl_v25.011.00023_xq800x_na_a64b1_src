/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is mp3 codec adapt file, codec standard APIs are defined.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x023907
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>
#include <limits.h>
#include "audio_util.h"
#include "codec_mng.h"
#include "audio_client.h"
#include "hwcodec.h"
#include "audio_stream.h"

#ifdef CONFIG_DEC_CODEC_MP3_AD
#include "mp3_dec_api.h"
#endif

#define MP3_HD_BUF_SIZE               2048
#define MP3_DATA_VBR_XING_HEADER_SIZE 156

#define MP3_VER_MPEG2P5     0
#define MP3_VER_MPEG2       2
#define MP3_VER_MPEG1       3

#define MP3_LAYER3          1
#define MP3_LAYER2          2
#define MP3_LAYER1          3

#define MP3_ID3_LEN         3
#define STR_MP3_ID3         "ID3"

#define MP3_TAG_ID_LEN      4

#define MP3_FRAME_FLAG      0xff
#define MP3_FRAME_FLAG2     0xe0
#define MP3_FRAME_FLAG3     0x7

#define MP3_HD_LEN          sizeof(struct ID3v2_header)

#define MP3_MASK 0xfffe0ccf
#define MKBETAG(a,b,c,d)    ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))
#define MAX_AB(a,b)    ((a) > (b) ? (a) : (b))
#ifdef ABS
#undef ABS
#define ABS(a)  ((a) >= 0 ? (a) : (-(a)))
#endif
#define XING_FLAG_FRAMES 0x01
#define XING_FLAG_SIZE   0x02
#define XING_FLAG_TOC    0x04
#define XING_FLAG_QSCALE 0x08
#define XING_TOC_COUNT   100
#define MP3_SUPPORT_FAST_SEEK   0x80000
#define SEEK_SIZE     0x1000
#define MP3_MIN(a,b)   ((a) > (b) ? (b):(a))
#define MP3_MAX(a,b)   ((a) > (b) ? (a):(b))
#define RB32(x)                                \
    (((u32)((const u8*)(x))[0] << 24) |    \
               (((const u8*)(x))[1] << 16) |    \
               (((const u8*)(x))[2] <<  8) |    \
                ((const u8*)(x))[3])


enum seek_dir {
    SEEK_FORWARD = 0,
    SEEK_BACKWARD
};

enum header_check {
   CHECK_WRONG_HEADER = -1,
   CHECK_SEEK_FAILED = -2,
};

enum round_type {
    ROUND_ZERO     = 0, ///< Round toward zero.
    ROUND_INF      = 1, ///< Round away from zero.
    ROUND_DOWN     = 2, ///< Round toward -infinity.
    ROUND_UP       = 3, ///< Round toward +infinity.
    ROUND_NEAR_INF = 5, ///< Round to nearest and halfway cases away from zero.
    /**
     * Flag telling rescaling functions to pass `INT64_MIN`/`MAX` through
     * unchanged, avoiding special cases for #AV_NOPTS_VALUE.
     *
     * Unlike other values of the enumeration AVRounding, this value is a
     * bitmask that must be used in conjunction with another value of the
     * enumeration through a bitwise OR, in order to set behavior for normal
     * cases.
     *
     * @code{.c}
     * av_rescale_rnd(3, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
     * // Rescaling 3:
     * //     Calculating 3 * 1 / 2
     * //     3 / 2 is rounded up to 2
     * //     => 2
     *
     * av_rescale_rnd(AV_NOPTS_VALUE, 1, 2, AV_ROUND_UP | AV_ROUND_PASS_MINMAX);
     * // Rescaling AV_NOPTS_VALUE:
     * //     AV_NOPTS_VALUE == INT64_MIN
     * //     AV_NOPTS_VALUE is passed through
     * //     => AV_NOPTS_VALUE
     * @endcode
     */
    ROUND_PASS_MINMAX = 8192,
};

struct mp3_toc_entry {
    u32 pos;
    u32 timestamp;
    u32 min_distance;
};

struct mp3_rational {
    s32 num; //numerator
    s32 den; // denominator
};


struct ID3v2_header {
   char id[MP3_ID3_LEN];     //ID3v2̶־ID3
   char ver;       //汾ţID3v23
   char rever;     //汾ţһ㶼Ϊ0
   char flag;      //־λһΪ0Ϊabc00000
   char size[4];   //ǩСһĸֽڣÿֻֽʹ7λ
                   //λʹúΪ0Ըʽ
                   //   0xxxxxxx 0xxxxxxx 0xxxxxxx 0xxxxxxx
};

struct ID3v2_tag { //ǩ֡10ֽ
   char id[MP3_TAG_ID_LEN];     //־շTEXTTOPETDAT....
   char size[4];   //֡ĴС8λ洢ģ
                   //   FSize = Size[0]*0x100000000 +
                   //           Size[1]*0x10000 + Size[2]*0x100 + Size[3];
   char flag[2];   //ű־6־
};

#if __BIG_ENDING__
struct frame_header {
    u8 sync    : 8;

    u8 sync2   : 3;
    u8 ver     : 2;    //00-MPEG2.5     01-undef     10-MPEG2     11-MPEG1
    u8 layer   : 2;    //00-undef       01-Layer3    10-Layer2    11-Layer1
    u8 crc     : 1;    //0:yes          1:no

    u8 brate   : 4;
    u8 srate   : 2;
    u8 fill    : 1;    //0: no          1: yes
    u8 rsv1    : 1;

    u8 chn     : 2;    //00-Stereo      01-Joint Stereo   10-dual     11-mono
    u8 rsv2    : 2;
    u8 cr      : 1;
    u8 orig_cr : 1;
    u8 empr    : 2;    //00-undef       01-50/15ms    10-rsv      11-CCITT J.17
};
#else
struct frame_header {
    u8 sync    : 8;

    u8 crc     : 1;    //0:yes          1:no
    u8 layer   : 2;    //00-undef       01-Layer3    10-Layer2    11-Layer1
    u8 ver     : 2;    //00-MPEG2.5     01-undef     10-MPEG2     11-MPEG1
    u8 sync2   : 3;

    u8 rsv1    : 1;
    u8 fill    : 1;    //0: no          1: yes
    u8 srate   : 2;
    u8 brate   : 4;

    u8 empr    : 2;    //00-undef       01-50/15ms    10-rsv      11-CCITT J.17
    u8 orig_cr : 1;
    u8 cr      : 1;
    u8 rsv2    : 2;
    u8 chn     : 2;    //00-Stereo      01-Joint Stereo   10-dual     11-mono
};
#endif

/* part of tag definition */
const char *ID3v2_tag_name[] = {
    "TEXT",        //
    "TENC",        //
    "WXXX",        //URL(URL)
    "TCOP",        //Ȩ(Copyright)
    "TOPE",        //ԭ
    "TCOM",        //
    "TDAT",        //
    "TPE3",        //ָ
    "TPE2",        //ֶ
    "TPE1",        //൱ID3v1Artist
    "TPE4",        //루¼Ա޸Ա
    "TYER",        //ID3v1Year
    "USLT",        //
    "TSIZ",        //С
    "TALB",        //ר൱ID3v1Album
    "TIT1",        //
    "TIT2",        //൱ID3v1Title
    "TIT3",        //
    "TCON",        //ɣ൱ID3v1Genre
    "AENC",        //Ƶܼ
    "TBPM",        //ÿӽCOMM ע൱ID3v1Comment
    "TDLY",        //б¼
    "TRCK",        //죨ţ൱ID3v1Track
    "TFLT",        //ļ
    "TIME",        //ʱ䡡
    "TKEY",        //ؼ
    "TLAN",        //
    "TLEN",        //
    "TMED",        //ý
    "TOAL",        //ԭƬ
    "TOFN",        //ԭļ
    "TOLY",        //ԭ
    "TORY",        //
    "TOWM",        //ļߣ֤ߣ
    "TPOS",        //Ʒ
    "TPUB",        //
    "TRDA",        //¼
    "TRSN",        //Intenet̨
    "TRSO",        //Intenet̨
    "UFID",        //Ψһļʶ
    "TSRC",        //ISRCʵı׼¼룩
    "TSSE",        //ʹõӲã
};

/** TODO: check later.
    32, 32, 32, 32(32), 32(8), 8(8)
    64, 48, 40, 64(48), 48(16), 16, (16)
    96, 56, 48, 96(56), 56(24), 24, (24)
    128, 64, 56, 128(64), 64(32), 32, (32)
    160, 80, 64, 160(80), 80(40), 64, (40)
    192, 96, 80, 192(96), 96(48), 80, (48)
    224, 112, 96, 224(112), 112(56), 56, (56)
    256, 128, 112, 256(128), 128(64), 64, (64)
    288, 160, 128, 288(144), 160(80), 128, (80)
    320, 192, 160, 320(160), 192(96), 160, (96)
    352, 224, 192, 352(176), 224(112), 112, (112)
    384, 256, 224, 384(192), 256(128), 128, (128)
    416, 320, 256, 416(224), 320(144), 256, (144)
    448, 384, 320, 448(256), 384(160), 320, (160)
*/

const static u16 brate_tbl[][9] = {
/* TODO: To be confirmed  */
/*  |  MPEG1     |    MPEG2     |   MPEG2P5   |  */
    {32,  32,  32,  32,  32,  8,   32,  8,   8},
    {64,  48,  40,  64,  48,  16,  48,  16,  16},
    {96,  56,  48,  96,  56,  24,  56,  24,  24},
    {128, 64,  56,  128, 64,  32,  64,  32,  32},
    {160, 80,  64,  160, 80,  64,  80,  40,  40},
    {192, 96,  80,  192, 96,  80,  96,  48,  48},
    {224, 112, 96,  224, 112, 56,  112, 56,  56},
    {256, 128, 112, 256, 128, 64,  128, 64,  64},
    {288, 160, 128, 288, 160, 128, 144, 80,  80},
    {320, 192, 160, 320, 192, 160, 160, 96,  96},
    {352, 224, 192, 352, 224, 112, 176, 112, 112},
    {384, 256, 224, 384, 256, 128, 192, 128, 128},
    {416, 320, 256, 416, 320, 256, 224, 144, 144},
    {448, 384, 320, 448, 384, 320, 256, 160, 160},
};

/**
 *   MPEG-1    00-44.1kHz   01-48kHz   10-32kHz     11-undef
 *
 *   MPEG-2    00-22.05kHz  01-24kHz   10-16kHz     11-undef
 *
 *   MPEG-2.5 00-11.025kHz 01-12kHz   10-8kHz       11-undef
 */
const static u32 srate_tbl[][3] = {
 /* MPEG1  MPEG2  MPEG2P5 */
    {44100, 22050, 11025},
    {48000, 24000, 12000},
    {32000, 16000, 8000},
};

const static u8 chn_tbl[] = {
    2, 2, 2, 1
};

struct codec_mp3_obj {
    struct list_head list;
    u8 *buff;
    u32 buff_size;
    codec_get fget;
    codec_send fsend;
    codec_seek fseek;
    u32 priv;
    u32 pos;
    u32 brate;
    u32 srate;
    u32 chn;
    u32 frmsize;
    s32 layer;
    s32 lsf;
    u32 spf;
    s32 is_cbr;
    u32 frames;
    u32 header_filesize;
    u32 header_offset;
    u32 xing_toc;
    u32 flags; /*support fast seek*/
    s64 duration;
    struct mp3_rational time_base;
    struct mp3_toc_entry *tocentry_ptr;
    aos_mutex_ptr seek_mutex;
};

#ifdef CONFIG_DEC_CODEC_MP3_AD
struct codec_mp3_ad_obj {
    struct codec_mp3_obj *base_codec;
    void *adcodec;
    u8 *back_buf;
    u32 back_offset;
    u32 back_size;
};
#endif

#ifdef CONFIG_DEC_CODEC_MP3_HWCODEC
struct codec_mp3_hwcodec_obj {
    struct codec_mp3_obj *base_codec;
    void *hwcodec;
};
#endif

static int mp3_get_wrapper(void *obj, u8* buf, u32 size, u32 *rd_len)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *) obj;
    u32 get_len = 0;
    u32 temp_len;
    int ret;

    while (get_len < size) {
        ret = codec->fget(codec->priv, buf + get_len, size - get_len, &temp_len);
        get_len += temp_len;
        /*for stream may return no data , not fatal error , so continue*/
        if (0 == ret /*|| ASTREAM_DATA_NODATA == ret*/)
            continue;
        else if (ASTREAM_DATA_EOS == ret){
            ALOGW("mp3 fget eos break!\n");
            *rd_len = get_len;
            return ret;
        }
        else {
            *rd_len = get_len;
            if (*rd_len == 0)
                return AIC_EPIPE;
            //now return 0 , but maybe not get all size.
            return 0;
        }
    }
    AASSERT(get_len == size);
    *rd_len = get_len;
    return 0;
}

static int mp3_rd8(void *in)
{
    u8 *pdata = (u8*)in;
    return *pdata;
}

static int mp3_rdb16(void *in)
{
    u32 val;
    u8 *temp = (u8*)in;

    val = mp3_rd8(temp) << 8;
    val |= mp3_rd8(temp + 1);
    return val;
}

static int mp3_rdb32(void *in)
{
    u32 val;
    u16 *temp = (u16*)in;

    val = mp3_rdb16(temp) << 16;
    val |= mp3_rdb16(temp + 1);

    return val;
}

static s64 mp3_rescale_rnd(s64 a, s64 b, s64 c, enum round_type rnd)
{
    s64 r = 0;

    if (c <= 0 || b < 0 || !((unsigned)(rnd&~ROUND_PASS_MINMAX)<=5 && (rnd&~ROUND_PASS_MINMAX)!=4))
        return INT64_MIN;

    if (rnd & ROUND_PASS_MINMAX) {
        if (a == INT64_MIN || a == INT64_MAX)
            return a;
        rnd -= ROUND_PASS_MINMAX;
    }

    if (a < 0)
        return -(uint64_t)mp3_rescale_rnd(-MAX_AB(a, -INT64_MAX), b, c, rnd ^ ((rnd >> 1) & 1));

    if (rnd == ROUND_NEAR_INF)
        r = c / 2;
    else if (rnd & 1)
        r = c - 1;

    if (b <= INT_MAX && c <= INT_MAX) {
        if (a <= INT_MAX)
            return (a * b + r) / c;
        else {
            int64_t ad = a / c;
            int64_t a2 = (a % c * b + r) / c;
            if (ad >= INT32_MAX && b && ad > (INT64_MAX - a2) / b)
                return INT64_MIN;
            return ad * b + a2;
        }
    } else {
        uint64_t a0  = a & 0xFFFFFFFF;
        uint64_t a1  = a >> 32;
        uint64_t b0  = b & 0xFFFFFFFF;
        uint64_t b1  = b >> 32;
        uint64_t t1  = a0 * b1 + a1 * b0;
        uint64_t t1a = t1 << 32;
        int i;

        a0  = a0 * b0 + t1a;
        a1  = a1 * b1 + (t1 >> 32) + (a0 < t1a);
        a0 += r;
        a1 += a0 < r;

        for (i = 63; i >= 0; i--) {
            a1 += a1 + ((a0 >> i) & 1);
            t1 += t1;
            if (c <= a1) {
                a1 -= c;
                t1++;
            }
        }
        if (t1 > INT64_MAX)
            return INT64_MIN;
        return t1;
    }

}

static s64 mp3_rescale_q_rnd(s64 a, struct mp3_rational bq,
                             struct mp3_rational cq, enum round_type rnd)
{
    s64 b = bq.num * (int64_t)cq.den;
    s64 c = cq.num * (int64_t)bq.den;
    return mp3_rescale_rnd(a, b, c, rnd);
}

static void add_toc_entry(struct codec_mp3_obj *codec, u32 index,
                          u32 pos, u32 timestamp)
{
    codec->tocentry_ptr[index].pos = pos;
    codec->tocentry_ptr[index].timestamp = timestamp;
    codec->tocentry_ptr[index].min_distance = 0;

    return;
}

static s64 mp3_get_size(struct codec_mp3_obj *codec)
{
    s64 size;

    if (codec->fseek) {
        size = codec->fseek(codec->priv, 0, CODEC_SEEK_SIZE);
        return size;
    }
    else {
        return 0;
    }
}

static int read_xing_toc(struct codec_mp3_obj *codec, u8* toc_ptr,
                         s64 filesize, u32 duration)
{
    u32 fill_index = (duration > 0);
    u32 i;
    u8* pcodec_temp = toc_ptr;
    s64 a;
    u32 b;
    s32 c;

    a = 0xfffffffff;
    b = 0xffffffff;
    c = 0xffffffff;
    if (!filesize && !(filesize = mp3_get_size(codec))) {
        fill_index = 0;
        codec->xing_toc = 0;
        ALOGW("Cannot determine file size, skipping TOC table!\n");
        return -1;
    }
    if (!codec->tocentry_ptr) {
        codec->tocentry_ptr = audio_malloc(sizeof(struct mp3_toc_entry) * XING_TOC_COUNT);
        if (NULL == codec->tocentry_ptr) {
            ALOGE("Cannot alloc tocentry table\n");
            return -1;
        }
    }
    for(i = 0;  i < XING_TOC_COUNT; i++)
    {
        u8 b = mp3_rd8(pcodec_temp);
        add_toc_entry(codec, i, mp3_rescale_rnd(b, filesize, 256, ROUND_NEAR_INF),
        mp3_rescale_rnd(i, duration, XING_TOC_COUNT, ROUND_NEAR_INF));
        pcodec_temp++;
    }
    codec->xing_toc = 1;
    return 0;
}

static void mp3_parse_vbri_tag(struct codec_mp3_obj *codec, u8 *p_header)
{
    u8 *pcodec_temp = p_header; /*pointer frame header*/
    u32 temp_v;
    struct mp3_rational time_base;

    pcodec_temp = p_header; /*pointer frame header*/
    pcodec_temp += 36;
    temp_v = mp3_rdb32(pcodec_temp);
    pcodec_temp += (sizeof(s32)/sizeof(u8));
    if (temp_v == MKBETAG('V' , 'B', 'R' , 'I')) {
        ALOGI("vbri tag find\n");
        if (1 == mp3_rdb16(pcodec_temp)) {
            pcodec_temp += ((sizeof(s16)/sizeof(u8)) + 4);
            codec->header_filesize = mp3_rdb32(pcodec_temp);
            pcodec_temp += (sizeof(s32)/sizeof(u8));
            codec->frames = mp3_rdb32(pcodec_temp);
            time_base.den = time_base.num = 1;
            ALOGV("parse vbri header_filesize:%d, frames:%d\n",
                  codec->header_filesize, codec->frames);
        }
    }
}

static int mp3_parse_vbr_info(struct codec_mp3_obj *codec,
                              u8 *p_header, u32 header_pos)
{
    u32 xing_offtbl[2][2] = {{36 , 21} , {21 , 13}};
    u32 xing_offset;
    u32 temp_v;
    s64 duration;
    s64 fsize = 0;
    struct mp3_rational time_base;

    u8 *pcodec_temp = p_header; /*pointer frame header*/

    time_base.num = time_base.den = 1; /* 1:1 */
    xing_offset = xing_offtbl[codec->lsf == 1][codec->chn == 1];
    fsize = codec->fseek(codec->priv , 0 , CODEC_SEEK_SIZE);
    fsize = fsize >= (header_pos+4) ? fsize - (header_pos + 4) : 0;
    pcodec_temp += xing_offset;
    temp_v = mp3_rdb32(pcodec_temp);
    pcodec_temp += (sizeof(s32)/sizeof(u8));
    codec->is_cbr = temp_v == MKBETAG('I' , 'n' , 'f' , 'o');
    if (temp_v != MKBETAG('X' , 'i' , 'n' , 'g') && !codec->is_cbr) {
        ALOGI("not vbr\n");
        return -1;
    }

    temp_v = mp3_rdb32(pcodec_temp);
    pcodec_temp += (sizeof(s32)/sizeof(u8));
    if (temp_v & XING_FLAG_FRAMES) {
        codec->frames = mp3_rdb32(pcodec_temp);
        pcodec_temp += (sizeof(s32)/sizeof(u8));
    }
    if (temp_v & XING_FLAG_SIZE) {
        codec->header_filesize = mp3_rdb32(pcodec_temp);
        pcodec_temp += (sizeof(s32)/sizeof(u8));
    }
    //header_filesize
    if (fsize && codec->header_filesize) {
        u64 min , delta;
        min = MP3_MIN(fsize , codec->header_filesize);
        delta = MP3_MAX(fsize , codec->header_filesize) - min;
        if (fsize > codec->header_filesize && delta > min >> 4) {
            codec->frames = 0;
            ALOGW("invalid concatenated file detected - using bitrate for duration\n");
        } else if (delta > min >> 4) {
            ALOGW("filesize and duration do not match (growing file?)\n");
        }
    }
    //calc duration
    duration =  mp3_rescale_q_rnd(codec->frames,
                                  (struct mp3_rational){codec->spf, codec->srate},
                                  codec->time_base, ROUND_NEAR_INF);
    if (temp_v & XING_FLAG_TOC) {
        ALOGI("find xing toc ,read_xing_toc\n");
        read_xing_toc(codec , pcodec_temp, codec->header_filesize, duration);
    }
    return 0;
}

static int parse_vbr(struct codec_mp3_obj *codec, u32 cur_pos)
{
    u32 real_bytes;
    u8* frame_header;
    s32 vbrtag_size = 0;
    u32 header_pos = cur_pos;
    int ret;

    if (codec->layer != 3) {
        ALOGE("error layer\n");
        return AIC_EFAULT;
    }
    vbrtag_size = codec->frmsize;
    if (codec->fseek(codec->priv, cur_pos, CODEC_SEEK_BEGIN) < 0) {
        return AIC_EPIPE;
    }
    //read 156 bytes
    frame_header = (u8*)audio_malloc(156);
    if (NULL == frame_header)
        return AIC_ENOMEM;
    ret = mp3_get_wrapper(codec, frame_header, 156, &real_bytes);
    if (ret) {
        audio_free(frame_header);
        return AIC_EIO;
    }

    codec->frames = codec->header_filesize =  0;

    mp3_parse_vbr_info(codec, frame_header , header_pos);
    mp3_parse_vbri_tag(codec, frame_header);
    audio_free(frame_header);

    if (!codec->frames && !codec->header_filesize) {
        return AIC_EACCES;
    }
    //calc duration
    if (codec->frames) {
        codec->duration = mp3_rescale_q_rnd(codec->frames,
                                            (struct mp3_rational){codec->spf, codec->srate},
                                            codec->time_base, ROUND_NEAR_INF);
    }
    /*seek to skip vbr tag frame*/
    if (codec->fseek(codec->priv, cur_pos + vbrtag_size, CODEC_SEEK_BEGIN) < 0) {
        ALOGW("skip vbr tag failed curpos:%d, vbrtag_size:%d\n", cur_pos, vbrtag_size);
    }

    return 0;
}

static inline s32 mpeg_check_header(u32 header)
{
    /* header */
    if ((header & 0xffe00000) != 0xffe00000)
        return -1;
    /* version check */
    if ((header & (3<<19)) == 1<<19)
        return -1;
    /* layer check */
    if ((header & (3<<17)) == 0)
        return -1;
    /* bit rate */
    if ((header & (0xf<<12)) == 0xf<<12)
        return -1;
    /* frequency */
    if ((header & (3<<10)) == 3<<10)
        return -1;
    return 0;
}

static s32 mpeg_check_decode_header(struct codec_mp3_obj *codec,
                                    u32 *frameheader, u32 *outframesize)
{
    struct frame_header *p_header = (struct frame_header *)(frameheader);
    u32 ver_idx = 0, layer_idx = 0;

    switch (p_header->ver) {
    case MP3_VER_MPEG2P5:
        ver_idx = 2;
        break;
    case MP3_VER_MPEG2:
        ver_idx = 1;
        break;
    case MP3_VER_MPEG1:
        ver_idx = 0;
        break;
    default:
        ALOGE("Invalid MP3 ver, ver=%d!\n", p_header->ver);
        return AIC_EPERM;
    }

    switch (p_header->layer) {
    case MP3_LAYER1:
        layer_idx = 0;
        break;
    case MP3_LAYER2:
        layer_idx = 1;
        break;
    case MP3_LAYER3:
        layer_idx = 2;
        break;
    default:
        ALOGE("undef mp3 layer = %d!\n", p_header->layer);
        return AIC_EPERM;
    }
    codec->layer = layer_idx + 1; /*layer 1 ,2 ,3*/
    /*lsf*/
    if (p_header->ver & 0x2) {
        codec->lsf = (p_header->ver & 0x1) ? 0 : 1;
    } else {
        codec->lsf = 1;
    }

    if (p_header->brate == 0)
        codec->brate = 0;
    else if (p_header->brate < 0xf) {
        u32 ver_offset = codec->lsf ? 2:0;
        codec->brate = brate_tbl[p_header->brate - 1][ver_offset * 3 + layer_idx];
    }
    else {/* 0xf */
        ALOGE("unknown bit rate=%d!\n", p_header->brate);
        return AIC_EPERM;
    }

    if (p_header->srate < 3)
        codec->srate = srate_tbl[p_header->srate][ver_idx];
    else {/* 3 */
        ALOGE("unknown sample rate=%d!\n", p_header->srate);
        return AIC_EPERM;
    }

    codec->chn = chn_tbl[p_header->chn];
    /*calc frame size*/
    if (codec->brate != 0) {
        u32 bitrate = codec->brate;
        u32 framesize;
        u32 samplerate = codec->srate;
        u32 padding = p_header->fill;
        u8 lsf = codec->lsf;
        switch (p_header->layer) {
        case MP3_LAYER1:
            framesize = (bitrate * 12000) / samplerate; /*sample num:384,framesize = ((sample_num/8) * bitrate)/sample_rate + 4 *padding */
            framesize = (framesize + padding) * 4;
            break;
        case MP3_LAYER2:
            framesize = (bitrate * 144000) / samplerate; /*sample num:1152 framesize = ((sample_num/8) * bitrate)/sample_rate + padding*/
            framesize += padding;
            break;
        default:
        case MP3_LAYER3:
            framesize = (bitrate * 144000) / (samplerate << lsf); /*sample num:1152*/
            framesize += padding;
            break;
        }
        ALOGD("mp3 coded layer:%d, framesize:%d\n", (u32)p_header->layer, framesize);
        codec->frmsize = framesize;
    }
    else {
        ALOGE("unknown frame size\n");
        codec->frmsize = 0;
        return AIC_EPERM;
    }
    *outframesize = codec->frmsize;
    /* get file header len */
    return 0;
}

static s32 check_header_for_framesize(u32 obj, u32 pos, u32 *retheader)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *) obj;
    s64 ret;
    u32 header;
    u8 header_buf[4];
    u32 *p_header = (u32*)header_buf;
    u32 realcount, framesize = 0;

    if (!codec->fseek) {
        ALOGE("no support fseek!\n");
        return CHECK_SEEK_FAILED;
    }
    ret = codec->fseek(codec->priv, pos, CODEC_SEEK_BEGIN);
    if (ret < 0) {
        ALOGE("seek pos:%d failed!\n", pos);
        return CHECK_SEEK_FAILED;
    }
    mp3_get_wrapper(codec, header_buf, 4, &realcount);
    if (realcount != 4) {
        return CHECK_SEEK_FAILED;
    }
    /* read bigenddin */
    header = RB32(&header_buf[0]);
    if (mpeg_check_header(header) < 0) {
        return CHECK_WRONG_HEADER;
    }

    if (mpeg_check_decode_header(codec, p_header, &framesize) < 0) {
        ALOGI("mpeg_check_decode_header failed\n");
        return CHECK_WRONG_HEADER;
    }
    if (retheader) {
        *retheader = header;
    }

    return framesize;
}

#define MIN_VALID 3
#define SEEK_WINDOW 4096
static s64 mp3_sync(u32 obj, u32 pos, enum seek_dir dir) {
    s32 ret;
    u32 best_pos;
    s32 best_score, i, j, dir_v;
    struct codec_mp3_obj *codec = (struct codec_mp3_obj*)obj;

    if (codec->fseek(codec->priv, pos, CODEC_SEEK_BEGIN) < 0) {
        return AIC_EFAULT;
    }
    best_pos = pos;
    best_score = 999;
    dir_v = (dir == SEEK_FORWARD) ? 1 : -1;
    for(i = 0; i < SEEK_WINDOW; i++) {
        s64 tmppos = (s64) pos + (s64)(dir_v > 0 ? i - SEEK_WINDOW/4 : -i);
        s64 candidate = -1;
        s32 score = 999;
        if (tmppos < 0)
           continue;
        for(j = 0; j < MIN_VALID; j++) {
            ret = check_header_for_framesize(obj, (u32)tmppos, NULL);
            if (ret < 0) {
                if (ret == CHECK_WRONG_HEADER) {
                    //ALOGI("check wrong header tmppos::%d, i:%d\n", (s32)tmppos, i);
                    break;
                }
                else if (ret == CHECK_SEEK_FAILED){
                    ALOGE("Could not seek to %lld.\n", tmppos);
                    return AIC_EINVAL;
                }
            }
            if (((s64)pos - tmppos) * dir_v <= 0 && ABS(MIN_VALID/2 - j) < score) {
                candidate = tmppos;
                score = ABS(MIN_VALID/2 - j);
            }
            tmppos += ret;
        }
        if (best_score > score && j == MIN_VALID) {
            best_pos = candidate;
            best_score = score;
            if (0 == score) {
                ALOGI("mp3_sync find best score pos socre\n");
                break;
            }
        }
    }

    return codec->fseek(codec->priv, best_pos, CODEC_SEEK_BEGIN);
}

static int parse_seek_info(struct codec_mp3_obj *obj)
{
    s64 off;
    u32 i;
    u32 header_pos;
    int ret = AIC_SUCCESS;
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *) obj;

    codec->flags |= MP3_SUPPORT_FAST_SEEK;
    codec->time_base.den = codec->time_base.num = 1;
    //parse vbr
    codec->spf = codec->lsf ? 576 : 1152;
    codec->seek_mutex = aos_create_mutex("mp3_seek_mutex");
    if (codec->seek_mutex == NULL) {
        goto _err_exit;
    }
    /*seek to first frame header*/
    header_pos = codec->header_offset;
    ALOGV("header codec pos:%d, pos %d\n", codec->pos, header_pos);

    if (parse_vbr(codec, header_pos)) {
        /*seek to header*/
        codec->fseek(codec->priv, header_pos, CODEC_SEEK_BEGIN);
    }
    if (codec->xing_toc) {
        /*skip for xing vbr headers*/
        off = codec->fseek(codec->priv, 0, CODEC_SEEK_CUR);
        for(i = 0; i < 64 * 1024; i++) {
            u32 header ,header2;
            s32 frame_size;

            frame_size = check_header_for_framesize((u32)codec, off + i, &header);
            if (frame_size > 0) {
                ret = check_header_for_framesize((u32)codec, off + i + frame_size, &header2);
                if (ret >= 0 && (header & MP3_MASK) == (header2 & MP3_MASK)) {
                    break;
                } else if (ret == CHECK_SEEK_FAILED) {
                    ALOGE("Invalid frame size:%d Could not seek to %lld\n", frame_size, off + i + frame_size);
                    ret = AIC_EFAULT;
                    goto _err_exit;
                }
            } else if (frame_size == CHECK_SEEK_FAILED) {
                ALOGE("Faile to read frame size Could not seek to %d\n", (i + 1024 + frame_size + 4));
                ret = AIC_EFAULT;
                goto _err_exit;
            }
        }
        if (64 * 1024 == i) {
            off = codec->fseek(codec->priv, off, CODEC_SEEK_BEGIN);
        } else {
            ALOGW("Skipping %d bytes of junk at %lld\n" , i , off);
            off = codec->fseek(codec->priv, off + i, CODEC_SEEK_BEGIN);
        }
        if (off < 0) {
            ret = AIC_EFAULT;
            goto _err_exit;
        }
        ALOGV("re-set toc entry offset:%d\n" , off);
        for(i = 0;  i < XING_TOC_COUNT; i++) {
            codec->tocentry_ptr[i].pos += off;
        }
    }
    /*seek original header pos*/
    codec->fseek(codec->priv, header_pos, CODEC_SEEK_BEGIN);
    return AIC_SUCCESS;

_err_exit:
    if (codec->seek_mutex) {
        aos_delete_mutex(codec->seek_mutex);
        codec->seek_mutex = AIC_NULL;
    }
    if (codec->tocentry_ptr) {
        audio_free(codec->tocentry_ptr);
        codec->tocentry_ptr = AIC_NULL;
    }
    return ret;
}

static int mp3_calc_duration(struct codec_mp3_obj *codec)
{
    if ((codec->duration == 0) && (codec->brate != 0)) {
        u32 size = mp3_get_size(codec);
        u32 filesize;
        if (size > 0) {
            assert(size >= codec->header_offset);
            filesize = size - codec->header_offset;
            codec->duration  = filesize * 8 / (codec->brate * 1000);
        }
    }

    return 0;
}

static int mp3_seek(u32 obj, u32 timestamp , u32 *real_timestamp, u32 *result_pos)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *) obj;
    u32 fast_seek = codec->flags & MP3_SUPPORT_FAST_SEEK;
    u32 filesize = codec->header_filesize;
    s64 target_pos;
    u32 duration , ratio;
    struct mp3_toc_entry *ie, ietmp;

    //need to be do may get from app duration interface
    duration = codec->duration * 1000;
    *real_timestamp = 0;
    if (timestamp > duration || duration == 0 || (!codec->fseek)) {
        return -1;
    }

    if (filesize <= 0) {
        s64 size = mp3_get_size(codec);
        if (size > 0) {
            assert(size >= codec->header_offset);
            filesize = size - codec->header_offset;
        }
    }

    ALOGV("mp3_seek timestamp:%d, filesize:%d\n", timestamp, filesize);
    aos_get_mutex(codec->seek_mutex, AOS_WAIT_FOREVER);

    if (codec->xing_toc && (fast_seek && !codec->is_cbr)) {
        ratio = (timestamp * 100) / duration;
        ie = &codec->tocentry_ptr[ratio];
        ie->timestamp = duration * ratio / 100;
        ALOGI("use xingtoc entry pos:%d, timestamp:%d\n", ie->pos, ie->timestamp);
    } else if (fast_seek && filesize > 0) {
        if (!codec->is_cbr)
            ALOGW("Using scaling to seek VBR MP3: may be imprecise.\n");
        ie = &ietmp;
        ratio = ((u64)((u64)timestamp * 10000)) / duration;
        ie->timestamp = timestamp;
        ie->pos = mp3_rescale_rnd(ratio, filesize, 10000, ROUND_NEAR_INF);
        ie->pos += codec->header_offset;
        ALOGI("no xing toc pos:%d, ratio:%d, timestamp:%d, header_offset:%d\n", ie->pos, ratio, ie->timestamp, codec->header_offset);
    } else {
        aos_put_mutex(codec->seek_mutex);
        return -1;
    }
    target_pos = mp3_sync((u32)codec, ie->pos, SEEK_BACKWARD);
    ALOGI("mp3_sync return value:%lld\n", target_pos);

    if (target_pos < 0) {
        aos_put_mutex(codec->seek_mutex);
        return target_pos;
    }
    aos_put_mutex(codec->seek_mutex);
    *real_timestamp = ie->timestamp;
    if (result_pos)
        *result_pos = target_pos;
    return 0;
}

static int check_frame_header(struct frame_header *header)
{
    if ((header->sync != MP3_FRAME_FLAG) ||
        ((header->sync2 & MP3_FRAME_FLAG3) != MP3_FRAME_FLAG3))
        return 0;
    else
        return 1;
}

static int skip_stream_bytes(struct codec_mp3_obj *codec, u32 len, uint8_t *tempbuf, uint32_t tempsize)
{
    int ret;

    if (len != 0) {
        if (codec->fseek) {
            ret = codec->fseek(codec->priv, len, CODEC_SEEK_CUR);
            if (ret < 0)
                 return ret;
            return 0;
        } else {
            uint32_t remain = len;
            u32 real_len, get_len;
            int ret;
            while (remain > 0) {
                get_len = (remain > tempsize) ? tempsize : remain;
                ret = mp3_get_wrapper(codec, tempbuf, get_len, &real_len);
                if (ret) {
                    /*if failed return*/
                    return ret;
                }
                remain -= real_len;
            }

            return 0;
        }
    }
    // skip 0 bytes comes here
    return 0;
}

/**
 *   return:
 *       >= 0: header size;
 *       <  0: error;
 */
int mp3_get_header_info(struct codec_mp3_obj *codec, uint32_t tag_len)
{
    struct frame_header *frame_header;
    u32 ver_idx = 0, layer_idx = 0,rd_len;
    u8 *buf;
    int ret;

    buf = codec->buff;

    frame_header = (struct frame_header *)buf;
    /*read more for parse header*/
    if (!check_frame_header(frame_header)) {
        u8 *p = (u8 *)frame_header;
        AIC_ASSERT(codec->buff_size < MP3_HD_BUF_SIZE);
        ret = mp3_get_wrapper(codec, buf + codec->buff_size,
                              MP3_HD_BUF_SIZE - codec->buff_size, &rd_len);
        if (ret) {
            ALOGE("mp3 parse failed no taglen read frame ret=%d\n", ret);
            if(ret != ASTREAM_DATA_EOS)
                return ret;
        }
        codec->buff_size += rd_len;
        ALOGW("need check more for header, buffsize:%d, get rdlen:%d\n",
              codec->buff_size, rd_len);
        rd_len = codec->buff_size;
        /* move to a real frame header, loop for ever? */
        while ((*(++p) != MP3_FRAME_FLAG ||
               ((*(++p) & MP3_FRAME_FLAG2) != MP3_FRAME_FLAG2)) &&
               (p - buf < (rd_len - 2)));

        if (p - buf >= (rd_len - 2)) {
            ALOGE("could not find frame header!\n");
            return AIC_EPERM;
        }
        frame_header = (struct frame_header *)(--p);

    } else
        ALOGE("just frame header %x\n", *((uint32_t *)(frame_header)));

    switch (frame_header->ver) {
    case MP3_VER_MPEG2P5:
        ver_idx = 2;
        break;
    case MP3_VER_MPEG2:
        ver_idx = 1;
        break;
    case MP3_VER_MPEG1:
        ver_idx = 0;
        break;
    default:
        ALOGE("Invalid MP3 ver, ver=%d!\n", frame_header->ver);
        return AIC_EPERM;
    }

    switch (frame_header->layer) {
    case MP3_LAYER1:
        layer_idx = 0;
        break;
    case MP3_LAYER2:
        layer_idx = 1;
        break;
    case MP3_LAYER3:
        layer_idx = 2;
        break;
    default:
        ALOGE("undef mp3 layer = %d!\n", frame_header->layer);
        return AIC_EPERM;
    }
    codec->layer = layer_idx + 1; /*layer 1 ,2 ,3*/

    /*lsf*/
    if (frame_header->ver & 0x2) {
        codec->lsf = (frame_header->ver & 0x1) ? 0 : 1;
    } else {
        codec->lsf = 1;
    }
    if (frame_header->brate == 0)
        codec->brate = 0;
    else if (frame_header->brate < 0xf) {
        u32 ver_offset = codec->lsf ? 2:0;
        codec->brate = brate_tbl[frame_header->brate - 1][/*ver_idx*/ver_offset * 3 + layer_idx];
    }
    else {/* 0xf */
        ALOGE("unknown bit rate=%d!\n", frame_header->brate);
        return AIC_EPERM;
    }

    if (frame_header->srate < 3)
        codec->srate = srate_tbl[frame_header->srate][ver_idx];
    else {/* 3 */
        ALOGE("unknown sample rate=%d!\n", frame_header->srate);
        return AIC_EPERM;
    }

    codec->chn = chn_tbl[frame_header->chn];

    /*calc frame size*/
    if (codec->brate != 0) {
        u32 bitrate = codec->brate;
        u32 framesize;
        u32 samplerate = codec->srate;
        u32 padding = frame_header->fill;
        u8 lsf = codec->lsf;
        switch (frame_header->layer) {
        case MP3_LAYER1:
            framesize = (bitrate * 12000) / samplerate; /*sample num:384,framesize = ((sample_num/8) * bitrate)/sample_rate + 4 *padding */
            framesize = (framesize + padding) * 4;
            break;
        case MP3_LAYER2:
            framesize = (bitrate * 144000) / samplerate; /*sample num:1152 framesize = ((sample_num/8) * bitrate)/sample_rate + padding*/
            framesize += padding;
            break;
        default:
        case MP3_LAYER3:
            framesize = (bitrate * 144000) / (samplerate << lsf); /*sample num:1152*/
            framesize += padding;
            break;
        }
        ALOGI("mp3 coded framesize:%d\n" , framesize);
        codec->frmsize = framesize;
    }
    else {
        ALOGE("unknown frame size\n");
        codec->frmsize = 0;
        return AIC_EPERM;
    }

    /* get file header len */
    codec->header_offset = (u32) (((u8*) frame_header - buf) + tag_len);

    return  (u32)((u8 *)frame_header - buf);
}

static int mp3_parse_header(struct codec_mp3_obj *codec, uint32_t tag_len)
{
    int ret;

    ret = mp3_get_header_info(codec, tag_len);
    if (ret < 0) {
        ALOGE("mp3 parse error, ret=%d\n", ret);
        return ret;
    }

    codec->pos = ret;

    ALOGW("mp3 info: brate=%d, srate=%d, chn=%d, pos=0x%x\n",
          codec->brate, codec->srate, codec->chn, codec->pos);

    return 0;
}

static u32 mp3_codec_init(u32 priv, struct codec_ops *ops, struct codec_info *info)
{
    u8 *buf;
    int ret;
    u32 rd_len, tag_len = 0;
    struct codec_mp3_obj *codec;
    struct ID3v2_header *file_header;

    if (!ops->get_func) {
        ALOGE("mp3 open, invalid input param\n");
        return 0;
    }
    codec = audio_malloc(sizeof(*codec) + 16 + MP3_HD_BUF_SIZE);

    if (!codec) {
        ALOGE("mp3 open, no mem\n");
        return 0;
    }
    memset(codec, 0, sizeof(*codec) + 16 + MP3_HD_BUF_SIZE);
    codec->buff = (u8 *)codec + sizeof(*codec) + 16;

    codec->priv = priv;
    codec->fget = ops->get_func;
    codec->fseek = ops->seek_func;
    buf = codec->buff;
    ret = mp3_get_wrapper(codec, buf, sizeof(struct ID3v2_header), &rd_len);
    if (ret) {
        ALOGE("mp3 parse header in open failed: ret=%d\n", ret);
        audio_free(codec);
        return 0;
    }
    codec->buff_size = rd_len;
    file_header = (struct ID3v2_header *)buf;
    if (strncmp(file_header->id, STR_MP3_ID3, MP3_ID3_LEN) == 0) {
        tag_len =  (file_header->size[0] & 0x7F) * 0x200000 +
                   (file_header->size[1] & 0x7F) * 0x4000 +
                   (file_header->size[2] & 0x7F) * 0x80 +
                   (file_header->size[3] & 0x7F);
                //skip taglen
        AIC_ASSERT(codec->buff_size < MP3_HD_BUF_SIZE);
        ret = skip_stream_bytes(codec, tag_len, buf + codec->buff_size, MP3_HD_BUF_SIZE - codec->buff_size);
        if (ret) {
            ALOGE("sikp stream bytes failed or reach eos ret:%d", ret);
            audio_free(codec);
            return 0;
        }
    }
    ALOGI("mp3 tag_len:%d", tag_len);
    if (mp3_parse_header(codec, tag_len)) {
        ALOGE("mp3 parse header failed\n");
        audio_free(codec);
        return 0;
    }

    info->brate = codec->brate;

    info->chn = codec->chn;
    info->srate = codec->srate;

    info->wid = 16;
    info->fmt = 0;
    if (codec->fseek) {
        if (parse_seek_info(codec) != AIC_SUCCESS) {
            audio_free(codec);
            return 0;
        }
        // set codec->pos == codec->buff_size, for current file already seeked to header pos.
        codec->pos = codec->buff_size = 0;
    }
    mp3_calc_duration(codec);
    ALOGI("codec %p seek:%p, brate:%d , srate:%d , chn:%d ,\
          framsize:%d , lsf:%d , is_cbr:%d , frames:%d , header_filesize:%d ,\
          xing_toc:%d , duration:%lld\n",
          codec , codec->fseek , codec->brate , codec->srate , codec->chn ,
          codec->frmsize , codec->lsf , codec->is_cbr , codec->frames ,
          codec->header_filesize, codec->xing_toc , codec->duration);

    return (u32)codec;
}

static void mp3_codec_deinit(u32 obj)
{
    void *codec = (void*)obj;

    audio_free(codec);
}

static int mp3_seek_internal(u32 obj, u32 seek_time, struct codec_info *info, u32 *target_pos)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj*)obj;
    int ret;

    if (seek_time && codec->fseek) {
        u32 temp_stamp;
        if (info->duration != 0)
            codec->duration = info->duration;
        ret = mp3_seek((u32)codec, seek_time , &temp_stamp, target_pos);
        if (!ret)
            info->seek_timestamp = temp_stamp;
        else {
            codec->fseek(codec->priv, codec->header_offset, CODEC_SEEK_BEGIN);
            info->seek_timestamp = 0;
            if (target_pos)
                *target_pos = codec->header_offset;
        }
        return 0;
    }
    return -1;
}

static int mp3_pause(u32 obj)
{
    return 0;
}

static int mp3_resume(u32 obj)
{
    return 0;
}

static int mp3_set_speed(u32 obj, u32 speed)
{
    return 0;
}

static int mp3_set_gain(u32 obj, u32 gain)
{
    return 0;
}

#ifdef CONFIG_DEC_CODEC_MP3_HWCODEC
static u32 mp3_hwcodec_open(u32 priv, struct codec_ops *ops,
                            struct codec_info *info)
{
    struct codec_mp3_hwcodec_obj *codec;
    struct codec_mp3_obj *base_codec;
    struct hwcodec_init_param param;
    u32 seek_timestamp = info->seek_timestamp;

    codec = audio_malloc(sizeof(struct codec_mp3_hwcodec_obj));
    if (!codec) {
        ALOGE("malloc codec error!");
        goto _error_alloc_codec;
    }
    base_codec = (struct codec_mp3_obj*)mp3_codec_init(priv, ops, info);
    if (!base_codec) {
        ALOGE("mp3_codec_parse failed\n");
        goto _error_codec_init;
    }
    codec->base_codec = base_codec;
    //reset info srate&chn for hwcodec
#if defined(CONFIG_MUSIC_FIX_48K)
#ifdef CONFIG_SOUND_CONV
    info->chn = 1;
#else
    info->chn = 2;
#endif
    info->srate = 48000;
#endif
    param.brate = info->brate * 1000;
    param.chn = base_codec->chn;
    param.dl_fmt = 3; /* TODO: AUDIO_FMT_MP3, but now ASTREAM_FORMAT_MP3 */
    param.dl_stream_id = STREAM_CHN_HWCODEC_DL;
    param.fget = ops->get_func;
    param.period_cnt = 2;
    param.srate = base_codec->srate;
    param.ul_fmt = AUDIO_FMT_PCM;
    param.ul_stream_id = STREAM_CHN_HWCODEC_UL;
    param.client_obj = base_codec->priv;
    param.residue_data = base_codec->buff + base_codec->pos;
    param.residue_size = base_codec->buff_size > base_codec->pos ?
                         (base_codec->buff_size - base_codec->pos) : 0;

    if ((codec->hwcodec = hwcodec_init(&param)) == NULL) {
        ALOGE("hwcodec init error!");
        goto _error_hwcodec_init;
    }

    mp3_seek_internal((u32)base_codec, seek_timestamp, info, NULL);

    return (u32)codec;

_error_hwcodec_init:
    mp3_codec_deinit((u32)base_codec);
_error_codec_init:
    audio_free(codec);
_error_alloc_codec:
    return 0;
}

int mp3_hwcodec_close(u32 obj)
{
    struct codec_mp3_hwcodec_obj *codec = (struct codec_mp3_hwcodec_obj *)obj;
    struct codec_mp3_obj *base_codec = (struct codec_mp3_obj*)codec->base_codec;

    if (codec) {
        hwcodec_deinit(codec->hwcodec);
        if (base_codec->seek_mutex)
            aos_delete_mutex(base_codec->seek_mutex);

        if (base_codec->tocentry_ptr)
            audio_free(base_codec->tocentry_ptr);
        mp3_codec_deinit((u32)base_codec);
        audio_free(codec);
    }

    return 0;
}

static int mp3_hwcodec_start(u32 obj)
{
    struct codec_mp3_hwcodec_obj *codec = (struct codec_mp3_hwcodec_obj *)obj;

    return hwcodec_start(codec->hwcodec);
}

static int mp3_hwcodec_stop(u32 obj)
{
    struct codec_mp3_hwcodec_obj *codec = (struct codec_mp3_hwcodec_obj *)obj;

    return hwcodec_stop(codec->hwcodec);
}

/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int mp3_hwcodec_get_data(u32 obj, short *pbuf, u32 count, u32 *real_count)
{
    struct codec_mp3_hwcodec_obj *codec = (struct codec_mp3_hwcodec_obj *)obj;

    return hwcodec_get_data(codec->hwcodec, pbuf, count, real_count);
}

/* do not define as const */
static struct codec_engine mp3_hw_codec = {
    .type = CODEC_TYPE_MP3,
    .open = mp3_hwcodec_open,
    .close = mp3_hwcodec_close,
    .start = mp3_hwcodec_start,
    .stop = mp3_hwcodec_stop,
    .pause = mp3_pause,
    .resume = mp3_resume,
    .get_data = mp3_hwcodec_get_data,
    .seek = NULL,
    .set_gain = mp3_set_gain,
    .set_speed = mp3_set_speed,
};
#endif

#ifdef CONFIG_DEC_CODEC_MP3_COMPR
static int mp3_comprcodec_get_data(u32 obj, short *pbuf, u32 count,
                                   u32 *real_count)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *)obj;
    u32 rd_count;
    u32 copy = 0;
    int ret = 0;

    if (!pbuf || !codec) {
        ALOGE("mp3 get data failed, buff NULL\n");
        return AIC_EPERM;
    }

    /* copy from header buffer fist */
    if (codec->pos < codec->buff_size) {
        if (codec->pos + count < codec->buff_size)
            copy = count;
        else
            copy = codec->buff_size - codec->pos;
        memcpy(pbuf, codec->buff + codec->pos, copy);
        codec->pos += copy;
    }
    /* then get from source client */
    if (copy < count)
        ret = mp3_get_wrapper(codec, (u8 *)pbuf + copy,
                              count - copy, &rd_count);
    else {
        *real_count = copy;
        return 0;
    }
    *real_count = copy + rd_count;

    return ret;

}

static u32 mp3_comprcodec_open(u32 priv, struct codec_ops *ops,
                               struct codec_info *info)
{
    struct codec_mp3_obj *codec;
    u32 seek_timestamp = info->seek_timestamp;

    codec = (struct codec_mp3_obj*)mp3_codec_init(priv, ops, info);
    if (!codec) {
        ALOGE("mp3_codec_parse failed\n");
        return 0;
    }

    mp3_seek_internal((u32)codec, seek_timestamp, info, NULL);

    return (u32)codec;
}

int mp3_comprcodec_close(u32 obj)
{
    struct codec_mp3_obj *codec = (struct codec_mp3_obj *)obj;

    if (codec) {
        if (codec->seek_mutex)
            aos_delete_mutex(codec->seek_mutex);

        if (codec->tocentry_ptr)
            audio_free(codec->tocentry_ptr);
        mp3_codec_deinit(obj);
    }

    return 0;
}

static int mp3_comprcodec_start(u32 obj)
{
    return 0;
}

static int mp3_comprcodec_stop(u32 obj)
{
    return 0;
}

static struct codec_engine mp3_codec = {
    .type = CODEC_TYPE_MP3,
    .open = mp3_comprcodec_open,
    .close = mp3_comprcodec_close,
    .start = mp3_comprcodec_start,
    .stop = mp3_comprcodec_stop,
    .pause = mp3_pause,
    .resume = mp3_resume,
    .get_data = mp3_comprcodec_get_data,
    .seek = NULL,
    .set_gain = mp3_set_gain,
    .set_speed = mp3_set_speed,
};
#endif

#ifdef CONFIG_DEC_CODEC_MP3_AD


static int mp3_adcodec_get_data(u32 obj, short *pbuf, u32 count,
                                u32 *real_count)
{
    struct codec_mp3_ad_obj *codec = (struct codec_mp3_ad_obj *)obj;
    struct mp3_dec_procparam param;
    u32 offset = 0;
    u8 eos_flag = 0;
    int ret = 0;

    while (offset < count) {
        if (codec->back_size) {
            u32 copy_size, need_size, back_size;
            need_size = count - offset;
            back_size = codec->back_size - codec->back_offset;
            copy_size = need_size > back_size ? back_size : need_size;
            memcpy((u8 *)pbuf + offset,
                   (u8 *)codec->back_buf + codec->back_offset,
                   copy_size);
            offset += copy_size;
            codec->back_offset += copy_size;
            if (codec->back_size == codec->back_offset) {
                codec->back_offset = 0;
                codec->back_size = 0;
            }
            if (eos_flag) {
                codec->back_size = codec->back_offset = 0;
                break;
            }
            continue;
        }
        if (eos_flag)
            break;
        param.outbuf = (int16_t *)((u8 *)pbuf + offset);
        param.outsize = 0;
        if (count - offset < MP3_DEC_OUTPUT_SIZE) {
            param.outbuf = (int16_t *)codec->back_buf;
            ret = mp3_dec_proc(codec->adcodec, &param);
            if (ret) {
                ALOGW("adcodec mp3 dec back buf error frame ret:%d!", ret);
                if (ret == MP3_STREAM_EOS) {
                    eos_flag = 1;
                    continue;
                }
                return AIC_EPIPE;
            }
            codec->back_size += param.outsize;
            continue;
        } else {
            ALOGI("mp3 dec not reach");
            ret = mp3_dec_proc(codec->adcodec , &param);
            offset += param.outsize;
            if (ret) {
                ALOGW("adcodec mp3 dec error frame!");
                if (ret == MP3_STREAM_EOS)
                    eos_flag = 1;
                continue;
            }
        }
   }

    *real_count = offset;

    if (eos_flag) {
        ALOGI("return eos real count:%d", *real_count);
        return ASTREAM_DATA_EOS;
    }
    return 0;
}

static u32 mp3_adcodec_open(u32 priv, struct codec_ops *ops,
                            struct codec_info *info)
{
    struct codec_mp3_ad_obj *codec;
    struct mp3_dec_inparam init_param;
    u32 seek_timestamp = info->seek_timestamp;
    int64_t pos;
    u32 pos_tmp = 0;
    int ret;

    codec = audio_malloc(sizeof(struct codec_mp3_ad_obj));
    if (!codec) {
        ALOGE("alloc adcodec obj error!");
        goto _error_malloc_obj;
    }

    memset(codec, 0, sizeof(struct codec_mp3_ad_obj));
    codec->back_buf = (u8 *)audio_malloc(MP3_DEC_OUTPUT_SIZE);
    if (!codec->back_buf) {
        ALOGE("alloc back_buff error!");
        goto _error_malloc_backbuf;
    }
    codec->base_codec = (void*)mp3_codec_init(priv, ops, info);
    if (!codec->base_codec) {
        ALOGE("mp3_codec_parse failed\n");
        goto _error_codec_init;
    }

    ret = mp3_seek_internal((u32)codec->base_codec, seek_timestamp, info, &pos_tmp);
    if (ret) {
        pos_tmp = codec->base_codec->header_offset;
    }
    pos = pos_tmp;

    init_param.priv = priv;
    init_param.reader_ops.read_func = ops->get_func;
    init_param.reader_ops.seek_func = ops->seek_func;
    init_param.reader_ext_info.size = codec->base_codec->buff_size -
                                 codec->base_codec->pos;
    init_param.reader_ext_info.data = codec->base_codec->buff
                                      + codec->base_codec->pos;
    codec->adcodec = mp3_dec_create(&init_param);
    if (!codec->adcodec) {
        ALOGE("mp3_dec_create error!");
        goto _error_dec_create;
    }

    mp3_dec_ctrl(codec->adcodec, MP3_DEC_CTRL_CMD_SET_POS, &pos);
    ALOGI("adcodec open returned");

    return (u32)codec;

_error_dec_create:
    mp3_codec_deinit((u32)codec->base_codec);
_error_codec_init:
    audio_free(codec->back_buf);
_error_malloc_backbuf:
    audio_free(codec);
_error_malloc_obj:
    return 0;
}

int mp3_adcodec_close(u32 obj)
{
    struct codec_mp3_ad_obj *codec = (struct codec_mp3_ad_obj *)obj;
    struct codec_mp3_obj *base_codec = NULL;

    if (codec) {
        base_codec = (struct codec_mp3_obj *)codec->base_codec;
        if (base_codec) {
            if (base_codec->seek_mutex)
                aos_delete_mutex(base_codec->seek_mutex);

            if (base_codec->tocentry_ptr)
                audio_free(base_codec->tocentry_ptr);
            mp3_codec_deinit((u32)base_codec);
        }
        if (codec->back_buf)
            audio_free(codec->back_buf);
        mp3_dec_delete(codec->adcodec);
        audio_free(codec);
    }

    return 0;
}

static int mp3_adcodec_start(u32 obj)
{
    return 0;
}

static int mp3_adcodec_stop(u32 obj)
{
    return 0;
}

static struct codec_engine mp3_sw_codec = {
    .type = CODEC_TYPE_MP3,
    .open = mp3_adcodec_open,
    .close = mp3_adcodec_close,
    .start = mp3_adcodec_start,
    .stop = mp3_adcodec_stop,
    .pause = mp3_pause,
    .resume = mp3_resume,
    .get_data = mp3_adcodec_get_data,
    .seek = NULL,
    .set_gain = mp3_set_gain,
    .set_speed = mp3_set_speed,
};
#endif
static int mp3_codec_register(void)
{
#ifdef CONFIG_DEC_CODEC_MP3_COMPR
    codec_register(&mp3_codec);
#endif
#ifdef CONFIG_DEC_CODEC_MP3_AD
    codec_register(&mp3_sw_codec);
#endif
#ifdef CONFIG_DEC_CODEC_MP3_HWCODEC
    codec_register(&mp3_hw_codec);
#endif

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(mp3_codec_register);

