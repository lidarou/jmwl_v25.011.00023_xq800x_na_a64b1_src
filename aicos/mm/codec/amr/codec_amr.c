

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1d699d
#define LOG_MODULE    AMOD_AUDIO

#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "audio_util.h"
#include "codec_mng.h"

#include "audio_client.h"
#include "timer_inc.h"
#if defined CONFIG_DEC_CODEC_AMR_AD || defined CONFIG_ENC_CODEC_AMR_AD
#include "amr_codec_api.h"
#endif


#define AMR_NB_HD_SIZE  6
#define AMR_WB_HD_SIZE  9

#define AMR_NB_HD       "#!AMR\n"
#define AMR_WB_HD       "#!AMR-WB\n"
#define READ_AMR_MODE(frm_hd)   (((frm_hd) >> 3) & 0xf)

#define AMR_WB_FRAM_PCM_SIZE    640
#define AMR_NB_FRAM_PCM_SIZE    320
#define AMR_WB_COMPRESS_MAX_FRAME_SIZE    61
#define AMR_NB_COMPRESS_MAX_FRAME_SIZE    1024

enum get_data_mode {
    AMR_GET_DATA_MODE_NORMAL = 0,
    AMR_GET_DATA_MODE_FRAME,
};

struct codec_amr_obj {
    struct codec_inf inf;
    BOOL is_wb;
    u8 header[AMR_WB_HD_SIZE];
    u8 header_size;
    u8 frm_mode;
    u8 frm_mode_valid;
    u8 base_header;
    u8 get_mode; /*get mode for file & buffer and frame mode*/
};

#if defined CONFIG_DEC_CODEC_AMR_AD || defined CONFIG_ENC_CODEC_AMR_AD
enum dec_mode
{
    AMR_DECMODE_NORMAL = 0,
    AMR_DECMODE_FRAME,
};

struct ad_codec_ops
{
    audio_create *codec_create;
    audio_delete *codec_delete;
    audio_process *codec_process;
    audio_control *codec_control;
};

struct amr_ad_dec_res {
    u8* tmp_ptr; /*backup data for decoded amr data*/
    u32 tmp_size; /*backup data size for decoded amr data*/
    u32 tmp_offset; /*backup data offset for decoded amr data*/
    u8 *tmp_raw_buf; /*amr rawbuf, for original compressed data*/
    u8 raw_buf_valid;
    u32 raw_buf_offset;
    u32 raw_buf_size;
    struct ad_codec_ops codec_ops;
};

struct amr_ad_enc_res {
    u8* tmp_ptr;
    u8 tmp_buf_empty;
    u32 tmp_buf_size;
    u32 tmp_offset;
    u8* tmp_ptr1;
    u32 tmp_ptr1_buf_size;
    u32 raw_frame_size;
    u32 compressed_frame_size;
    struct ad_codec_ops codec_ops;
};
#endif

struct codec_amr_ad_obj {
    struct codec_amr_obj *base_codec;
    void *priv_res;
    void *priv_codec_handle;
    u8 base_header;
    u8 frm_mode;
    u8 frm_mode_ready;
};

#if defined CONFIG_DEC_CODEC_AMR_COMPR || defined CONFIG_DEC_CODEC_AMR_AD
static u32 amrwb_frmsize_tbl[] =
{
    18,
    24,
    33,
    37,
    41,
    47,
    51,
    59,
    61,
    6,
    1,
    1,
    1,
    1,
    1,
    1
};

static u32 amrnb_frmsize_tbl[] =
{
    13,
    14,
    16,
    18,
    20,
    21,
    27,
    32,
    6,
    7,
    6,
    6,
    0,
    0,
    0,
    1
};

#endif

#if defined CONFIG_ENC_CODEC_AMR_AD || defined CONFIG_DEC_CODEC_AMR_AD
static void* amr_codec_malloc(void *mem_state, u32 size)
{
    return aos_malloc(size);
}

static void amr_codec_free(void *mem_state, void *mem_ptr)
{
    aos_free(mem_ptr);
}
#endif

#if defined CONFIG_DEC_CODEC_AMR_COMPR || defined CONFIG_DEC_CODEC_AMR_AD
static int amr_dec_seek(u32 obj, u32 timestamp)
{
    struct codec_amr_obj *codec = (struct codec_amr_obj *)obj;
    u32 ms_perframe = 20;
    u32 frame_no, frame_size, dst_offset;
    int ret;

    frame_no = timestamp/ms_perframe;
    frame_size = codec->is_wb ? amrwb_frmsize_tbl[codec->frm_mode] : \
                 amrnb_frmsize_tbl[codec->frm_mode];

    dst_offset = frame_no * frame_size + codec->header_size;

    AIC_ASSERT(dst_offset <
               codec->inf.codec_seek(codec->inf.priv, 0, CODEC_SEEK_SIZE));
    ret = codec->inf.codec_seek(codec->inf.priv, dst_offset, CODEC_SEEK_BEGIN);
    if (ret < 0) {
        ALOGE("amr_dec_seek failed:%d\n", ret);
        return ret;
    }
    ALOGI("amr_dec_seek to %d, frame_no:%d, frame_size:%d\n",
          dst_offset, frame_no, frame_size);
    return 0;
}

static u32 amr_dec_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    struct codec_amr_obj *codec;
    int ret_size = 0, ret = 0;
    u8 *phd;
    u32 timestamp;

    AENTRY

    if (!ops->get_func) {
        ALOGE("amr open, fget is null\n");
        return 0;
    }
    codec = audio_malloc(sizeof(*codec));

    if (!codec) {
        ALOGE("amr open, no mem\n");
        return 0;
    }
    memset(codec, 0, sizeof(*codec));
    codec->inf.codec_get = ops->get_func;
    codec->inf.codec_seek = ops->seek_func;
    codec->inf.priv = priv;
    timestamp = info->seek_timestamp;
    codec->get_mode = AMR_GET_DATA_MODE_NORMAL;
    if (info->flag & CODEC_INFO_FRAME_MODE) {
        codec->get_mode = AMR_GET_DATA_MODE_FRAME;
    }
    if (info->flag & CODEC_INFO_NO_HEADER) {
        ALOGW("no amr file header!");
        if (info->srate == 8000)
            codec->is_wb = 0;
        else
            codec->is_wb = 1;
    } else {
        phd = codec->header;
        ops->get_func(priv, phd, AMR_NB_HD_SIZE, &ret_size);
        if (ret >= 0 && ret_size != AMR_NB_HD_SIZE) {
            ALOGE("read amr header failed\n");
            goto header_err;
        }
        codec->header_size = AMR_NB_HD_SIZE;
        /* compare "#!AMR" first */
        if (memcmp(phd, AMR_NB_HD, AMR_NB_HD_SIZE - 1)) {
            /* Not AMR format */
            ALOGE("Invalid AMR file\n");
            goto header_err;
        }
        if (phd[5] == '\n') {
            codec->is_wb = FALSE;
        } else if (phd[5] == '-') {
            /* read 2 bytes more to check last two char */
            ret = ops->get_func(priv, phd, 3, &ret_size);
            if (ret >= 0 && ret_size == 3) {
                if (phd[0] == 'W' && phd[1] == 'B') {
                    codec->is_wb = TRUE;
                    codec->header_size += 3;
                } else {
                    ALOGE("Invalid AMR-WB file\n");
                    goto header_err;
                }
            } else {
                ALOGE("read 'WB' failed\n");
                goto header_err;
            }
        } else {
            ALOGE("AMR header failed\n");
            goto header_err;
        }

        if (codec->is_wb)
            info->srate = 16000;
        else
            info->srate = 8000;
    }
    info->chn = 1;
    info->brate = 0;

    if (timestamp) {
        if (codec->inf.codec_seek) {
            u8 tmp;
            int sucflag = 0;
            ret = codec->inf.codec_get(priv, &tmp, 1, &ret_size);
            if (ret >= 0 && ret_size == 1) {
                codec->frm_mode = READ_AMR_MODE(tmp);
                codec->frm_mode_valid = 1;
                codec->base_header = tmp;
                ret = amr_dec_seek((u32)codec, timestamp);
                if (!ret)
                    sucflag = 1;
            }
            if (!sucflag) {
                ALOGW("amr dec seek failed, seek to fist frame!!!\n");
                codec->inf.codec_seek(priv, codec->header_size,
                                      CODEC_SEEK_BEGIN);
            }
         }
    }
    ALOGI("amr header info:chn:%d,srate=%d,brate=%d,wbnb:%d,frame mode:%d\n",
          info->chn, info->srate, info->brate, codec->is_wb, codec->frm_mode);

    return (u32)codec;

header_err:
    audio_free(codec);

    return 0;
}

int amr_dec_close(u32 obj)
{
    struct codec_amr_obj *codec = (struct codec_amr_obj *)obj;

    if (codec)
        audio_free(codec);

    return 0;
}
#endif

#ifdef CONFIG_DEC_CODEC_AMR_COMPR
/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int amr_dec_get_data(u32 obj, short *pbuf, u32 count, u32 *real_count)
{
    struct codec_amr_obj *codec = (struct codec_amr_obj *)obj;
    struct codec_inf *inf = &codec->inf;

    return inf->codec_get(inf->priv, (u8 *)pbuf, count, real_count);
}

/* do not define as const */
static struct codec_engine amr_dec = {
    .type = CODEC_TYPE_AMR,
    .dir = CODEC_DEC,
    .open = amr_dec_open,
    .close = amr_dec_close,
    .get_data = amr_dec_get_data,
};

#endif

#ifdef CONFIG_DEC_CODEC_AMR_AD
static int amr_get_wrapper(codec_get get_func, u32 priv, u32 mode, u8 *buf,
                           u32 size, u32 *rd_len)
{
    u32 get_len = 0;
    u32 temp_len = 0;
    int ret;

    if (mode == AMR_DECMODE_NORMAL) {
        while (get_len < size) {
            ret = get_func(priv, buf + get_len, size - get_len, &temp_len);
            get_len += temp_len;
            if (0 == ret)
                continue;
            else if (ASTREAM_DATA_EOS == ret){
                ALOGW("amr fget eos break!\n");
                *rd_len = get_len;
                return ret;
            }
            else {
                *rd_len = get_len;
                if (0 == *rd_len)
                    return AIC_EPIPE;
                /*return 0 here, but maybe not get total required size*/
                return 0;
            }
        }
        AIC_ASSERT(get_len == size);
        *rd_len = get_len;
    } else if (mode == AMR_DECMODE_FRAME) {
        /*get frame mode*/
        ret = get_func(priv, buf, 0, &temp_len);
        if (ret < 0) {
            rd_len = 0;
            return ret;
        }
        *rd_len = temp_len;
    } else {
        ALOGE("input mode maybe error mode:%d!", mode);
        return AIC_EINVAL;
    }
    return 0;
}

static u32 amr_ad_dec_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    u32 base_obj, alloc_size;
    struct amr_ad_dec_res *ad_res;
    struct codec_amr_obj *base_codec;
    struct codec_amr_ad_obj *codec;
    memory_api_t priv_codec_api;

    codec = audio_malloc(sizeof(struct codec_amr_ad_obj));
    if (!codec) {
        ALOGE("_error_alloc_obj\n");
        goto _error_alloc_obj;
    }
    memset(codec, 0, sizeof(struct codec_amr_ad_obj));
    base_obj = amr_dec_open(priv, ops, info);
    if (base_obj == 0) {
        ALOGE("_error_open_base\n");
        goto _error_open_base;
    }
    base_codec = (struct codec_amr_obj*) base_obj;

    if (base_codec->frm_mode_valid) {
        codec->frm_mode = base_codec->frm_mode;
        codec->base_header = base_codec->base_header;
    }
    ad_res = audio_malloc(sizeof(struct amr_ad_dec_res));
    if (!ad_res) {
        ALOGE("_error_alloc_ad\n");
        goto _error_alloc_ad;
    }
    memset(ad_res, 0, sizeof(struct amr_ad_dec_res));
    if (base_codec->is_wb) {
#ifdef CONFIG_DEC_CODEC_AMR_WB_AD
        ad_res->codec_ops.codec_create = amrwb_dec_create;
        ad_res->codec_ops.codec_delete = amrwb_dec_delete;
        ad_res->codec_ops.codec_control = amrwb_dec_control;
        ad_res->codec_ops.codec_process = amrwb_dec_process;
        alloc_size = AMR_WB_FRAM_PCM_SIZE;
#else
        ALOGE("No AD AMR WB dec lib\n");
        goto _error_alloc_1;
#endif
    }
    else {
#ifdef CONFIG_DEC_CODEC_AMR_NB_AD
        /*need add nb codec ops*/
        ad_res->codec_ops.codec_create = amrnb_dec_create;
        ad_res->codec_ops.codec_delete = amrnb_dec_delete;
        ad_res->codec_ops.codec_control = amrnb_dec_control;
        ad_res->codec_ops.codec_process = amrnb_dec_process;
        alloc_size = AMR_NB_FRAM_PCM_SIZE;
#else
        ALOGE("No AD AMR NB dec lib\n");
        goto _error_alloc_1;
#endif
    }
    ad_res->tmp_ptr = audio_malloc(alloc_size);
    if (!ad_res->tmp_ptr) {
        ALOGE("_error_alloc_1\n");
        goto _error_alloc_1;
    }
    ad_res->tmp_offset = 0;
    ad_res->tmp_size = 0;

    alloc_size = AMR_WB_COMPRESS_MAX_FRAME_SIZE * 2;
    ad_res->tmp_raw_buf = audio_malloc(alloc_size);
    if (!ad_res->tmp_raw_buf) {
        ALOGE("_error_alloc_2\n");
        goto _error_alloc_2;
    }
    priv_codec_api.heap_state = NULL;
    priv_codec_api.pf_heap_free = amr_codec_free;
    priv_codec_api.pf_heap_malloc = amr_codec_malloc;
    priv_codec_api.scratch_state = NULL;
    priv_codec_api.pf_scratch_malloc = NULL;
    priv_codec_api.pf_scratch_free = NULL;
    codec->priv_codec_handle = ad_res->codec_ops.codec_create(&priv_codec_api,
                                                              NULL, 0);
    if (!codec->priv_codec_handle) {
        ALOGE("_error_ad_codec_error\n");
        goto _error_ad_codec_error;
    }

    codec->base_codec = base_codec;
    codec->priv_res = ad_res;

    return (u32)codec;

_error_ad_codec_error:
    audio_free(ad_res->tmp_raw_buf);
_error_alloc_2:
    audio_free(ad_res->tmp_ptr);
_error_alloc_1:
    audio_free(ad_res);
_error_alloc_ad:
    amr_dec_close(base_obj);
_error_open_base:
    audio_free(codec);
_error_alloc_obj:

    return 0;
}

int amr_ad_dec_close(u32 obj)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct codec_amr_obj *base_codec = codec->base_codec;
    struct amr_ad_dec_res *ad_res = (struct amr_ad_dec_res *)codec->priv_res;

    ad_res->codec_ops.codec_delete(codec->priv_codec_handle);
    audio_free(ad_res->tmp_raw_buf);
    audio_free(ad_res->tmp_ptr);
    audio_free(ad_res);
    amr_dec_close((u32)base_codec);
    audio_free(codec);

    return 0;
}

static int ad_dec_framemode(u32 obj, u8 *pbuf, u32 count, u32 *real_count)
{
    int ret;
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct amr_ad_dec_res *ad_res = (struct amr_ad_dec_res *)codec->priv_res;
    u32 real_raw_count, dec_out_count = 0;
    u8 tmp_raw_buf[AMR_WB_COMPRESS_MAX_FRAME_SIZE];

    *real_count = 0;
    ret = amr_get_wrapper(codec->base_codec->inf.codec_get,
                          codec->base_codec->inf.priv,
                          AMR_GET_DATA_MODE_FRAME,
                          tmp_raw_buf,
                          0,
                          &real_raw_count);
    if (ret) {
        ALOGE("ad_dec_framemode get data error!");
        return AIC_EFAULT;
    }
    if (codec->base_codec->is_wb)
        dec_out_count = AMR_WB_FRAM_PCM_SIZE;
    else {
        ALOGE("ad_dec_framemode amr mode is not wb!");
        return AIC_EINVAL;
    }

    ret = ad_res->codec_ops.codec_process(codec->priv_codec_handle,
                                          tmp_raw_buf,
                                          real_raw_count,
                                          pbuf,
                                          &dec_out_count);
    if (ret) {
        ALOGW("amrwb dec single frame ret:%d, insize:%d, dec_outcount:%d",
              ret, count, dec_out_count);
        ALOGW("size=%d, %x, %x, %x, %x, %x, %x\n", real_raw_count,
              tmp_raw_buf[0], tmp_raw_buf[1],
              tmp_raw_buf[2], tmp_raw_buf[3],
              tmp_raw_buf[4], tmp_raw_buf[5]);
        return AIC_EFAULT;
    }
    *real_count = dec_out_count;

    return 0;
}

static void write_seq_backbuf(void *obj, void *data, u32 size)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct amr_ad_dec_res *ad_res = (struct amr_ad_dec_res *)codec->priv_res;
    AIC_ASSERT(size <= (AMR_WB_COMPRESS_MAX_FRAME_SIZE * 2));
    memcpy(ad_res->tmp_raw_buf,
           data,
           size);
    ad_res->raw_buf_valid = 1;
    ad_res->raw_buf_offset = 0;
    ad_res->raw_buf_size = size;

    return;
}

static int read_seq(void *obj, void *data, u32 size, u32 *out_size)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct amr_ad_dec_res *ad_res = (struct amr_ad_dec_res *)codec->priv_res;
    u32 tmp;
    int ret = 0;

    *out_size = 0;
    if (ad_res->raw_buf_valid) {
        u32 back_num = ad_res->raw_buf_size - ad_res->raw_buf_offset;
        u8 back_suff = back_num >= size ? 1 : 0;
        u32 copy_size = back_suff ? size : back_num;

        memcpy(data, ad_res->tmp_raw_buf + ad_res->raw_buf_offset, copy_size);
        if (back_suff) {
            ad_res->raw_buf_offset += size;
            *out_size = size;
            if (ad_res->raw_buf_offset == ad_res->raw_buf_size)
                ad_res->raw_buf_valid = 0;
            ALOGI("back buf valid 0");
        } else {

            ALOGI("back buf valid 1");
            ad_res->raw_buf_valid = 0;
            ret = amr_get_wrapper(codec->base_codec->inf.codec_get,
                                  codec->base_codec->inf.priv,
                                  AMR_DECMODE_NORMAL,
                                  (uint8_t *)data + copy_size,
                                  (size - copy_size), &tmp);
            if (ret != 0 && ret != ASTREAM_DATA_EOS)
                return -1;
            *out_size = tmp + copy_size;
        }
    } else {
        ret = amr_get_wrapper(codec->base_codec->inf.codec_get,
                              codec->base_codec->inf.priv,
                              AMR_DECMODE_NORMAL,
                              (uint8_t *)data, size, &tmp);

        if (ret != 0 && ret != ASTREAM_DATA_EOS) {
            return -1;
        }
        *out_size = tmp;
        ALOGV("back buf not valid, size:%d, outsize:%d", size, *out_size);

    }

    return ret;
}

static int get_amr_sync_frame(void *obj, u8 *pinbuf,
                              u32 insize, u8 *outbuf,
                              u32 *out_size)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    u32 tmp_size;
    u32 framesize;
    u8 *tmpbuf, *base;
    u32 tmpbuf_size = AMR_WB_COMPRESS_MAX_FRAME_SIZE * 2;
    u32 checklen, skip_byte;
    u8 sync_ok = 0;
    int ret;

    *out_size = 0;
    framesize = codec->base_codec->is_wb ?
                amrwb_frmsize_tbl[codec->frm_mode] : \
                amrnb_frmsize_tbl[codec->frm_mode];
    tmpbuf = audio_malloc(tmpbuf_size);
    if (!tmpbuf) {
        goto _error_malloc;
    }
    if (insize > AMR_WB_COMPRESS_MAX_FRAME_SIZE) {
        ALOGE("error insize:%d , is too larger", insize);
        goto _error_param;
    }
    /*check pinbuf for in isze*/
    memcpy(tmpbuf, pinbuf, insize);

    while (1) {
        u32 comp_buf_redunt;
        ret = read_seq(codec, tmpbuf + insize, (tmpbuf_size -insize), &tmp_size);
        if (ret && ASTREAM_DATA_EOS != ret) {
            break;
        }
        //check sync
        if (tmp_size + insize >= framesize)
            checklen = (tmp_size + insize) - framesize;
        else {
            AIC_ASSERT(ret == ASTREAM_DATA_EOS);
            ALOGE("amr sync exit eos no enough data to check");
            break;
        }
        base  = tmpbuf;
        comp_buf_redunt = checklen;
        for(skip_byte = 0; skip_byte < checklen; skip_byte++) {
            if ((base[0] == codec->base_header) &&
                (base[framesize] == codec->base_header))
                break;
            base += 1;
        }

        if (skip_byte == checklen) {
            if (ret == ASTREAM_DATA_EOS)
                break;
            memmove(tmpbuf, tmpbuf + checklen, framesize);
            insize = framesize;
            continue;
        }
        else {
            sync_ok = 1;
            break;
        }
    }
    if (sync_ok) {
        u32 redunt_size = 0;
        memcpy(outbuf, base, framesize);
        *out_size = framesize;
        redunt_size = (tmp_size + insize) - ((base - tmpbuf) + framesize);
        if (redunt_size) {
            write_seq_backbuf(obj, base + framesize, redunt_size);
        }
    }
    audio_free(tmpbuf);
    return ret;
_error_param:
    audio_free(tmpbuf);
_error_malloc:
    return -1;
}

static int read_frame(void *obj, u8 *pbuf, u32 *real_count)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    u32 real_raw_count = 0;
    u32 framesize;
    u8 eos_flag = 0;
    int ret;

    *real_count = 0;
    if ((codec->frm_mode_ready == 0) && (!codec->base_codec->frm_mode_valid)) {
        /*get frame mode*/
        ret = read_seq(codec, pbuf, 1, &real_raw_count);

        codec->frm_mode_ready = 1;
        if (ret != 0) {
            ALOGE("get frame mode error! fatal error!");
            return ret;
        }
        codec->frm_mode = READ_AMR_MODE(pbuf[0]);
        codec->base_header = pbuf[0];
        codec->base_codec->frm_mode_valid = 1;
        ALOGI("get amr frm mode:%d", codec->frm_mode);
        framesize = codec->base_codec->is_wb ?
                    amrwb_frmsize_tbl[codec->frm_mode] : \
                    amrnb_frmsize_tbl[codec->frm_mode];

        ret = read_seq(obj, pbuf + 1, framesize - 1, &real_raw_count);
        real_raw_count += 1;
        *real_count = real_raw_count;
        return ret;
    }
    framesize = codec->base_codec->is_wb ?
                amrwb_frmsize_tbl[codec->frm_mode] : \
                amrnb_frmsize_tbl[codec->frm_mode];

    ret = read_seq(codec, pbuf,
                   framesize, &real_raw_count);
    *real_count = real_raw_count;
    if (ret == ASTREAM_DATA_EOS)
        eos_flag = 1;
    else if (ret)
        return ret;

    if (pbuf[0] != codec->base_header) {
        if (eos_flag) {
            return ASTREAM_DATA_EOS;
        }
        ret = get_amr_sync_frame(codec, pbuf,
                                 real_raw_count,
                                 pbuf,
                                 &real_raw_count);
        *real_count = real_raw_count;
        ALOGW("resync frame, real_raw_count:%d, ret:%d", real_raw_count, ret);
    }

    return ret;
}

static int ad_dec_frames(u32 obj, u8 *pbuf, u32 count, u32 *real_count)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct amr_ad_dec_res *ad_res = (struct amr_ad_dec_res *)codec->priv_res;
    u32 real_raw_count = 0, dec_out_count = 0,
        out_offset, eos_flag , max_frame_out_size;
    u8 tmp_raw_buf[AMR_WB_COMPRESS_MAX_FRAME_SIZE] = {0};
    u8 *tmp_out = 0;
    int ret;

    *real_count = 0;
    out_offset = 0;
    eos_flag = 0;
    max_frame_out_size = codec->base_codec->is_wb ? \
                         AMR_WB_FRAM_PCM_SIZE : AMR_NB_FRAM_PCM_SIZE;
    while (out_offset < count) {
        if (eos_flag) {
            break;
        }
        if (ad_res->tmp_size > ad_res->tmp_offset) {
            u32 temp_count;
            u32 redunt = ad_res->tmp_size > ad_res->tmp_offset;
            temp_count = redunt >= (count - out_offset) ?\
                         (count - out_offset) : redunt;
            memcpy(pbuf + out_offset, ad_res->tmp_ptr + ad_res->tmp_offset, temp_count);
            out_offset += temp_count;
            ad_res->tmp_offset += temp_count;
            continue;
        }

        ret = read_frame(codec, tmp_raw_buf, &real_raw_count);
        if (0 == ret || ASTREAM_DATA_EOS == ret) {
            if (ret == ASTREAM_DATA_EOS) {
                /*do some thing for eos*/
                eos_flag = 1;
            }
            if (codec->base_codec->is_wb)
                dec_out_count = AMR_WB_FRAM_PCM_SIZE;
            else
                dec_out_count = AMR_NB_FRAM_PCM_SIZE;
            if (out_offset + max_frame_out_size <= count) {
                tmp_out = pbuf + out_offset;
            } else {
                tmp_out = ad_res->tmp_ptr;
            }
            ret = ad_res->codec_ops.codec_process(codec->priv_codec_handle,
                                                  tmp_raw_buf,
                                                  real_raw_count,
                                                  tmp_out,
                                                  &dec_out_count);

            if (ret) {
                ALOGW("amrwb dec error 0 ret:%d, insize:%d, dec_outcount:%d",
                      ret, real_raw_count, dec_out_count);
                continue;
            }
            if (out_offset + max_frame_out_size <= count) {
                out_offset += dec_out_count;
            } else {
                u32 temp_count, redunt_flag = 0;
                if (dec_out_count <= (count - out_offset)) {
                    temp_count = dec_out_count;
                } else {
                    redunt_flag = 1;
                    temp_count = count - out_offset;
                }
                memcpy(pbuf + out_offset, tmp_out, temp_count);
                out_offset += temp_count;
                if (redunt_flag) {
                    ad_res->tmp_offset = temp_count;
                    ad_res->tmp_size = dec_out_count;
                    /*done*/
                    continue;
                }
            }

            if (eos_flag) {
                break;
            }
        }else {
            return AIC_EFAULT;
        }
    }
    *real_count = out_offset;
    if (eos_flag)
        return ASTREAM_DATA_EOS;

    return 0;
}

static int amr_ad_dec_get_data(u32 obj, short *pbuf, u32 count, u32 *real_count)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct codec_amr_obj *base_codec = codec->base_codec;

    if (base_codec->get_mode == AMR_GET_DATA_MODE_NORMAL)
        return ad_dec_frames(obj, (u8 *)pbuf, count, real_count);
    else if (base_codec->get_mode == AMR_GET_DATA_MODE_FRAME)
        return ad_dec_framemode(obj, (u8 *)pbuf, count, real_count);
    else
        return AIC_EINVAL;
}

/* do not define as const */
static struct codec_engine amr_ad_dec = {
    .type = CODEC_TYPE_AMR,
    .dir = CODEC_DEC,
    .open = amr_ad_dec_open,
    .close = amr_ad_dec_close,
    .get_data = amr_ad_dec_get_data,
};
#endif

#if defined CONFIG_ENC_CODEC_AMR_COMPR || defined CONFIG_ENC_CODEC_AMR_AD
static u32 amr_enc_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    struct codec_amr_obj *codec;

    if (!ops->send_func) {
        ALOGE("amr open, fsend is null\n");
        return 0;
    }

    codec = audio_malloc(sizeof(*codec));

    if (!codec) {
        ALOGE("amr open, no mem\n");
        audio_free(codec);

        return 0;
    }
    memset(codec, 0, sizeof(*codec));
    if (info->srate == 8000) {
        memcpy(codec->header, AMR_NB_HD, AMR_NB_HD_SIZE);
        codec->is_wb = FALSE;
    }
    else if (info->srate == 16000) {
        memcpy(codec->header, AMR_WB_HD, AMR_WB_HD_SIZE);
        codec->is_wb = TRUE;
    }
    codec->inf.codec_send = ops->send_func;
    codec->inf.priv = priv;

    return (u32)codec;
}

int amr_enc_stop(u32 obj)
{
    return 0;
}

int amr_enc_close(u32 obj)
{
    if (obj)
        audio_free(obj);

    return 0;
}
#endif

#ifdef CONFIG_ENC_CODEC_AMR_COMPR
/* if < 0: error; if == 0: finished; if > 0: real sample count */
static int amr_enc_send_data(u32 obj, short *pbuf, u32 count)
{
    struct codec_amr_obj *codec = (struct codec_amr_obj *)obj;
    struct codec_inf *inf = &codec->inf;

    return inf->codec_send(inf->priv, (u8 *)pbuf, count);
}

int amr_enc_get_header(u32 obj, u8 **buf, u32 *size)
{
    struct codec_amr_obj *codec = (struct codec_amr_obj *)obj;

    *buf = codec->header;
    if (codec->is_wb)
        *size = AMR_WB_HD_SIZE;
    else
        *size = AMR_NB_HD_SIZE;

    return 0;
}

/* do not define as const */
static struct codec_engine amr_enc = {
    .type = CODEC_TYPE_AMR,
    .dir = CODEC_ENC,
    .open = amr_enc_open,
    .stop = amr_enc_stop,
    .close = amr_enc_close,
    .send_data = amr_enc_send_data,
    .get_header = amr_enc_get_header,
};
#endif

#ifdef CONFIG_ENC_CODEC_AMR_AD
static u32 amr_ad_enc_open(u32 priv, struct codec_ops *ops,
                        struct codec_info *info)
{
    struct codec_amr_ad_obj *codec;
    struct codec_amr_obj *base_codec;
    struct amr_ad_enc_res *ad_res;
    memory_api_t priv_codec_api;
    u32 alloc_size;

    codec = audio_malloc(sizeof(struct codec_amr_ad_obj));
    if (!codec) {
        ALOGE("amr_ad_enc_open malloc error");
        goto _error_alloc_obj;
    }
    memset(codec, 0, sizeof(struct codec_amr_ad_obj));

    base_codec = (struct codec_amr_obj*)amr_enc_open(priv, ops, info);
    if (!base_codec) {
        ALOGE("amr_ad_enc_open _error_open_base!");
        goto _error_open_base;
    }
    ad_res = audio_malloc(sizeof(struct amr_ad_enc_res));
    if (!ad_res) {
        ALOGE("amr_ad_enc_open _error_alloc_ad!");
        goto _error_alloc_ad;
    }
    if (base_codec->is_wb) {
#ifdef CONFIG_ENC_CODEC_AMR_WB_AD
        ad_res->codec_ops.codec_create = amrwb_enc_create;
        ad_res->codec_ops.codec_delete = amrwb_enc_delete;
        ad_res->codec_ops.codec_process = amrwb_enc_process;
        ad_res->codec_ops.codec_control = amrwb_enc_control;
        alloc_size = AMR_WB_FRAM_PCM_SIZE;
#else
        ALOGE("No AD AMR WB enc lib\n");
        goto _error_alloc_1;
#endif
    }
    else {
#ifdef CONFIG_ENC_CODEC_AMR_NB_AD
        /*to do , add nv codec ops*/
        ad_res->codec_ops.codec_create = amrnb_enc_create;
        ad_res->codec_ops.codec_delete = amrnb_enc_delete;
        ad_res->codec_ops.codec_process = amrnb_enc_process;
        ad_res->codec_ops.codec_control = amrnb_enc_control;
        alloc_size = AMR_NB_FRAM_PCM_SIZE;
        /*now not support nb, so return error*/
        ALOGE("amr nb ad !");
#else
        ALOGE("No AD AMR NB enc lib\n");
        goto _error_alloc_1;
#endif
    }
    ad_res->raw_frame_size = alloc_size;
    /*alloc backup buffer for not encoded src raw data*/
    ad_res->tmp_ptr = audio_malloc(alloc_size);
    if (!ad_res->tmp_ptr) {
        ALOGE("amr_ad_enc_open _error_alloc_1!");
        goto _error_alloc_1;
    }
    ad_res->tmp_offset = 0;
    ad_res->tmp_buf_size = alloc_size;
    ad_res->tmp_buf_empty = 1;

    /*alloc temp encoded output buffer*/
    if (base_codec->is_wb) {
        alloc_size = AMR_WB_COMPRESS_MAX_FRAME_SIZE * 100;
        ad_res->compressed_frame_size = AMR_WB_COMPRESS_MAX_FRAME_SIZE;
    }
    else {
        alloc_size = AMR_NB_COMPRESS_MAX_FRAME_SIZE * 100;
        ad_res->compressed_frame_size = AMR_NB_COMPRESS_MAX_FRAME_SIZE;
    }
    ad_res->tmp_ptr1 = audio_malloc(alloc_size);
    if (!ad_res->tmp_ptr1) {
        ALOGE("amr_ad_enc_open _error_alloc_2!");
        goto _error_alloc_2;
    }
    ad_res->tmp_ptr1_buf_size = alloc_size;
    priv_codec_api.heap_state = NULL;
    priv_codec_api.pf_heap_free = amr_codec_free;
    priv_codec_api.pf_heap_malloc = amr_codec_malloc;
    priv_codec_api.scratch_state = NULL;
    priv_codec_api.pf_scratch_malloc = NULL;
    priv_codec_api.pf_scratch_free = NULL;
    if (base_codec->is_wb) {
        amr_enc_initparam_t init_param;
        init_param.allow_dtx = 0;
        init_param.frameType = VOAMRWB_RFC3267;
        init_param.enc_mode = info->brate;
        init_param.log_func = NULL;
        init_param.scratch_ptr = NULL;
        codec->priv_codec_handle = ad_res->codec_ops.codec_create(&priv_codec_api,
                                                                  (int *)(&init_param), 3);
    }
    else {
        amr_enc_initparam_t init_param;
        init_param.allow_dtx = 0;
        init_param.output_format = AMR_TX_WMF;
        init_param.enc_mode = info->brate;
        init_param.log_func = NULL;
        init_param.scratch_ptr = NULL;
        codec->priv_codec_handle = ad_res->codec_ops.codec_create(&priv_codec_api,
                                                                  (int *)(&init_param), 3);
    }
    if (!codec->priv_codec_handle) {
        ALOGE("amr_ad_enc_open _error_ad_codec_error!");
        goto _error_ad_codec_error;
    }
    codec->priv_res = ad_res;
    codec->base_codec = base_codec;

    return (u32) codec;

_error_ad_codec_error:
    audio_free(ad_res->tmp_ptr1);
_error_alloc_2:
    audio_free(ad_res->tmp_ptr);
_error_alloc_1:
    audio_free(ad_res);
_error_alloc_ad:
    amr_enc_close((u32)base_codec);
_error_open_base:
    audio_free(codec);
_error_alloc_obj:

    return 0;
}

static int amr_ad_enc_close(u32 obj)
{

    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct codec_amr_obj* base_codec = codec->base_codec;
    struct amr_ad_enc_res *ad_res = (struct amr_ad_enc_res*) codec->priv_res;

    ad_res->codec_ops.codec_delete(codec->priv_codec_handle);
    audio_free(ad_res->tmp_ptr);
    audio_free(ad_res->tmp_ptr1);
    audio_free(ad_res);
    amr_enc_close((u32)base_codec);
    audio_free(codec);

    return 0;
}

static int ad_enc_frames(u32 obj, u8 *pbuf, u32 count, u32 *out_size)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj*)obj;
    struct amr_ad_enc_res *ad_res = (struct amr_ad_enc_res*)codec->priv_res;
    u32 input_frame_size, input_offset, output_offset;
    int ret;

    input_frame_size = codec->base_codec->is_wb ?\
                       AMR_WB_FRAM_PCM_SIZE : AMR_NB_FRAM_PCM_SIZE;
    input_offset = 0;
    output_offset = 0;
    *out_size = 0;
    /*if there exist redundent raw data , reconstruct one frame*/
    if (!ad_res->tmp_buf_empty) {
        u32 temp_count = 0 , temp_full = 0;
        if (ad_res->tmp_offset + count >= ad_res->tmp_buf_size) {
            temp_count = ad_res->tmp_buf_size - ad_res->tmp_offset;
            temp_full = 1;
        }
        else
            temp_count = count;
        memcpy(ad_res->tmp_ptr + ad_res->tmp_offset, pbuf, temp_count);
        ad_res->tmp_offset += temp_count;
        input_offset += temp_count;
        if (temp_full) {
            /*encode*/
            output_offset = ad_res->compressed_frame_size;
            ret = ad_res->codec_ops.codec_process(codec->priv_codec_handle,
                                                  ad_res->tmp_ptr,
                                                  ad_res->tmp_buf_size,
                                                  ad_res->tmp_ptr1,
                                                  &output_offset);
            if (ret) {
                ALOGW("amrwb_enc_process error:%d", ret);
            }
            ad_res->tmp_buf_empty = 1;
        } else {
            ALOGW("not fill temp buf, input buff maybe too small:%d", count);
            *out_size = 0;
            return 0;
        }
    }
    /*continue*/
    while (input_offset + input_frame_size < count) {
        u32 temp_out_size, input_frames, need_output_size, input_size;

        input_frames = (count - input_offset)/ad_res->raw_frame_size;
        need_output_size = input_frames * ad_res->compressed_frame_size;
        if (need_output_size + output_offset > ad_res->tmp_ptr1_buf_size) {
            u8* temp = audio_malloc(need_output_size + output_offset);
            memcpy(temp, ad_res->tmp_ptr1, output_offset);
            audio_free(ad_res->tmp_ptr1);
            ad_res->tmp_ptr1 = temp;
            ad_res->tmp_ptr1_buf_size = need_output_size + output_offset;
        }
        input_size = input_frames * ad_res->raw_frame_size;
        temp_out_size = input_frames * ad_res->compressed_frame_size;
        ret = ad_res->codec_ops.codec_process(codec->priv_codec_handle,
                                              pbuf + input_offset,
                                              input_size,
                                              ad_res->tmp_ptr1 + output_offset,
                                              &temp_out_size);

        if (ret) {
            ALOGW("amrwb_enc_process error:%d", ret);
        }
        output_offset += temp_out_size;
        input_offset += input_size;
    }
    /*check whether redundent raw data leaved*/
    if (input_offset != count) {
        ad_res->tmp_buf_empty = 0;
        memcpy(ad_res->tmp_ptr, pbuf + input_offset, count - input_offset);
        ad_res->tmp_offset = count - input_offset;
    }
    *out_size = output_offset;

    return 0;
}

static int amr_ad_enc_send_data(u32 obj, short *pbuf, u32 count)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct codec_inf *inf = &codec->base_codec->inf;
    struct amr_ad_enc_res* ad_res = (struct amr_ad_enc_res*)codec->priv_res;
    u32 out_size;

    ad_enc_frames(obj, (u8*)pbuf, count, &out_size);
    if (out_size)
        return inf->codec_send(inf->priv, (u8 *)ad_res->tmp_ptr1, out_size);
    else
        return AIC_EFAULT;
}

int amr_ad_enc_get_header(u32 obj, u8 **buf, u32 *size)
{
    struct codec_amr_ad_obj *codec = (struct codec_amr_ad_obj *)obj;
    struct codec_amr_obj *base_codec = codec->base_codec;

    *buf = base_codec->header;
    if (base_codec->is_wb)
        *size = AMR_WB_HD_SIZE;
    else
        *size = AMR_NB_HD_SIZE;

    return 0;
}


static struct codec_engine amr_ad_enc = {
    .type = CODEC_TYPE_AMR,
    .dir = CODEC_ENC,
    .open = amr_ad_enc_open,
    .stop = amr_enc_stop,
    .close = amr_ad_enc_close,
    .send_data = amr_ad_enc_send_data,
    .get_header = amr_ad_enc_get_header,
};
#endif

static int amr_codec_init(void)
{
#ifdef CONFIG_DEC_CODEC_AMR_COMPR
    codec_register(&amr_dec);
#endif
#ifdef CONFIG_DEC_CODEC_AMR_AD
    codec_register(&amr_ad_dec);
#endif

#ifdef CONFIG_ENC_CODEC_AMR_COMPR
    codec_register(&amr_enc);
#endif

#ifdef CONFIG_ENC_CODEC_AMR_AD
    codec_register(&amr_ad_enc);
#endif

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(amr_codec_init);

