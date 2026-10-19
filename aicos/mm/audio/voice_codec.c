/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines APIs about voice process between ap and cp.
 *
 */
#define AIC_THIS_FILE    0x0850ba
#define LOG_MODULE    AMOD_AUDIO

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_bitmap.h>

#include "audio_util.h"
#include "audio_system.h"
#include "audio_dump.h"
#include "codec_mng.h"
#include "voice_codec.h"
#include "amr_codec_api.h"
#include "dtmf_gen.h"

#define AMR_WB_FRAM_PCM_SIZE    640
#define AMR_NB_FRAM_PCM_SIZE    320
#define AMR_WB_COMPRESS_MAX_FRAME_SIZE    61
#define AMR_NB_COMPRESS_MAX_FRAME_SIZE    32

typedef void* (*vocodec_create)(void *inparam, void *outparam);
typedef int (*vocodec_process)(void *handle, void *inparam, void *outparam);
typedef int (*vocodec_control)(void *handle, void *inparam, void *outparam);
typedef void (*vocodec_delete)(void *handle);

#define CODEC_CREATE(idx, info, m)  \
    if (codec_info[idx].ops && codec_info[idx].ops->codec_create)  \
        codec_info[idx].codec_handle = codec_info[idx].ops->codec_create(info, m);

#define CODEC_DELETE(idx)  \
    if (codec_info[idx].ops && codec_info[idx].ops->codec_delete)  \
        codec_info[idx].ops->codec_delete(codec_info[idx].codec_handle);

#define CODEC_CONTROL(r, c, in, out)  \
    if (c.ops && c.ops->codec_control)  \
        r = c.ops->codec_control(c.codec_handle, in, out);

struct vo_codec_in_param {
    u8 *in_buf;
    u32 in_size;
    u8 *out_buf;
    u32 out_size;
};

struct vo_codec_out_param {
    u32 out_size;
};

struct vo_codec_ctrl_param {
    int32_t cmd;
    void *param;
    u32 param_size;
};

enum vo_codec_type {
    VO_CODEC_WB_DEC,
    VO_CODEC_WB_ENC,
    VO_CODEC_NB_DEC,
    VO_CODEC_NB_ENC,
    VO_CODEC_DTMF,
    VO_CODEC_TYPE_NUM,
};

struct vo_codec_ops {
    vocodec_create codec_create;
    vocodec_delete codec_delete;
    vocodec_process codec_process;
    vocodec_control codec_control;
};

struct vo_codec_info {
    struct vo_codec_ops *ops;
    void *codec_handle;
};

struct voice_proc_obj {
    struct vo_codec_info vocodec_info[VO_CODEC_TYPE_NUM];
};

static void *ad_malloc(void *mem_state, u32 size)
{
    return audio_malloc(size);
}

static void ad_free(void *mem_state, void *mem_ptr)
{
    audio_free(mem_ptr);
}

#ifdef CONFIG_DEC_CODEC_AMR_WB_AD
void *ad_amrwb_dec_create(void *inpuaram, void *outparam)
{
    memory_api_t mem_api;
    amr_dec_initparam_t param;
    param.dec_mode = AMR_DEC_MODE_FRAME;
    param.log_func = NULL;
    param.scratch_ptr = NULL;
    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = ad_malloc;
    mem_api.pf_heap_free = ad_free;
    mem_api.scratch_state = NULL;
    mem_api.pf_scratch_malloc = ad_malloc;
    mem_api.pf_scratch_free = ad_free;

    return amrwb_dec_create(&mem_api, &param, 0);
}

int ad_amrwb_dec_proc(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_in_param *param = (struct vo_codec_in_param *)inparam;
    struct vo_codec_out_param *ouparam = (struct vo_codec_out_param *)outparam;
    char *inbuf;
    u32 insize;
    char *outbuf;
    u32 outsize;
    int ret;

    inbuf = param->in_buf;
    insize = param->in_size;
    outbuf = param->out_buf;
    outsize = param->out_size;
    ret = amrwb_dec_process(handle, inbuf, insize, outbuf, &outsize);
    if (!ret && ouparam)
            ouparam->out_size = outsize;

    return ret;
}

int ad_amrwb_dec_ctl(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_ctrl_param *param = (struct vo_codec_ctrl_param *)inparam;
    int32_t cmd = param->cmd;
    void *ctrl_param = param->param;
    u32 param_size = param->param_size;

    return amrwb_dec_control(handle, cmd, ctrl_param, param_size);
}

void ad_amrwb_dec_delete(void *handle)
{
    amrwb_dec_delete(handle);
}

const struct vo_codec_ops amrwb_dec_ops = {
    .codec_create = ad_amrwb_dec_create,
    .codec_delete = ad_amrwb_dec_delete,
    .codec_process = ad_amrwb_dec_proc,
    .codec_control = ad_amrwb_dec_ctl,
};
#endif

#ifdef CONFIG_ENC_CODEC_AMR_WB_AD
void *ad_amrwb_enc_create(void *inparam, void *outparam)
{
    memory_api_t mem_api;
    amr_enc_initparam_t init = {0};

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = ad_malloc;
    mem_api.pf_heap_free = ad_free;
    mem_api.scratch_state = NULL;
    mem_api.pf_scratch_malloc = ad_malloc;
    mem_api.pf_scratch_free = ad_free;
    if (inparam) {
        struct codec_info *info = (struct codec_info *)inparam;
        init.frameType = VOAMRWB_RFC3267;
        if (info->flag & CODEC_INFO_DTX)
            init.allow_dtx = 1;
        init.enc_mode = info->brate;
    }

    return amrwb_enc_create(&mem_api, &init, 0);
}

int ad_amrwb_enc_proc(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_in_param *param = (struct vo_codec_in_param *)inparam;
    struct vo_codec_out_param *ouparam = (struct vo_codec_out_param *)outparam;
    char *inbuf = param->in_buf;
    u32 insize = param->in_size;
    char *outbuf = param->out_buf;
    u32 outsize = param->out_size;
    int ret;

    ret = amrwb_enc_process(handle, inbuf, insize, outbuf, &outsize);
    if (!ret && outparam)
            ouparam->out_size = outsize;

    return ret;
}

int ad_amrwb_enc_ctl(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_ctrl_param *param = (struct vo_codec_ctrl_param *)inparam;
    int32_t cmd;
    int ctrl_param;// = param->param;
    u32 param_size = param->param_size/sizeof(int32_t);

    switch(param->cmd) {
    case VOICE_CMD_GET_FORMAT:
        cmd = AMR_CMD_GET_FROMAT;
        break;
    case VOICE_CMD_GET_CHANNELS:
        cmd = AMR_CMD_GET_CHANNELS;
        break;
    case VOICE_CMD_GET_SAMPLERATE:
        cmd = AMR_CMD_GET_SAMPLERATE;
        break;
    case VOICE_CMD_SET_FRAMETYPE:
        cmd = AMR_CMD_SET_FRAMETYPE;
        switch (*((int*)param->param)) {
        case VOICE_AMRWB_DEFAULT:
            ctrl_param = VOAMRWB_DEFAULT;
            break;
        case VOICE_AMRWB_ITU:
            ctrl_param = VOAMRWB_ITU;
            break;
        case VOICE_AMRWB_RFC3267:
            ctrl_param = VOAMRWB_RFC3267;
            break;
        default:
            return AIC_EINVAL;
        }
        break;
    case VOICE_CMD_SET_MODE:
        cmd = AMR_CMD_SET_MODE;
        ctrl_param = *((int32_t*)param->param);
        break;
    case VOICE_CMD_SET_DTX:
        cmd = AMR_CMD_SET_DTX;
        ctrl_param = *((int32_t*)param->param);
        break;
    default:
        return AIC_EINVAL;
    }
    return amrwb_enc_control(handle, cmd, &ctrl_param, param_size);
}

void ad_amrwb_enc_delete(void *handle)
{
    amrwb_enc_delete(handle);
}

const struct vo_codec_ops amrwb_enc_ops = {
    .codec_create = ad_amrwb_enc_create,
    .codec_delete = ad_amrwb_enc_delete,
    .codec_process = ad_amrwb_enc_proc,
    .codec_control = ad_amrwb_enc_ctl,
};
#endif

#ifdef CONFIG_DEC_CODEC_AMR_NB_AD
void *ad_amrnb_dec_create(void *inpuaram, void *outparam)
{
    memory_api_t mem_api;
    amr_dec_initparam_t param;
    param.dec_mode = AMR_DEC_MODE_FRAME;
    param.log_func = NULL;
    param.scratch_ptr = NULL;
    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = ad_malloc;
    mem_api.pf_heap_free = ad_free;
    mem_api.scratch_state = NULL;
    mem_api.pf_scratch_malloc = ad_malloc;
    mem_api.pf_scratch_free = ad_free;

    return amrnb_dec_create(&mem_api, &param, 0);
}

int ad_amrnb_dec_proc(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_in_param *param = (struct vo_codec_in_param *)inparam;
    char *inbuf = param->in_buf;
    u32 insize = param->in_size;
    char *outbuf = param->out_buf;
    u32 outsize = param->out_size;

    return amrnb_dec_process(handle, inbuf, insize, outbuf, &outsize);
}

int ad_amrnb_dec_ctl(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_ctrl_param *param = (struct vo_codec_ctrl_param *)inparam;
    int32_t cmd = param->cmd;
    void *ctrl_param = param->param;
    u32 param_size = param->param_size;

    return amrnb_dec_control(handle, cmd, ctrl_param, param_size);
}

void ad_amrnb_dec_delete(void *handle)
{
    amrnb_dec_delete(handle);
}

const struct vo_codec_ops amrnb_dec_ops = {
    .codec_create = ad_amrnb_dec_create,
    .codec_delete = ad_amrnb_dec_delete,
    .codec_process = ad_amrnb_dec_proc,
    .codec_control = ad_amrnb_dec_ctl,
};
#endif

#ifdef CONFIG_ENC_CODEC_AMR_NB_AD
void *ad_amrnb_enc_create(void *inparam, void *outparam)
{
    memory_api_t mem_api;
    amr_enc_initparam_t init = {0};

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = ad_malloc;
    mem_api.pf_heap_free = ad_free;
    mem_api.scratch_state = NULL;
    mem_api.pf_scratch_malloc = ad_malloc;
    mem_api.pf_scratch_free = ad_free;

    if (inparam) {
        struct codec_info *info = (struct codec_info *)inparam;
        if (info->flag & CODEC_INFO_DTX)
            init.allow_dtx = 1;
        init.output_format = AMR_TX_WMF;
        init.enc_mode = info->brate;
    }

    return amrnb_enc_create(&mem_api, &init, 0);
}

int ad_amrnb_enc_proc(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_in_param *param = (struct vo_codec_in_param *)inparam;
    struct vo_codec_out_param *out_param = (struct vo_codec_out_param *)outparam;
    char *inbuf = param->in_buf;
    u32 insize = param->in_size;
    char *outbuf = param->out_buf;
    u32 outsize = param->out_size;
    int ret;

    ret = amrnb_enc_process(handle, inbuf, insize, outbuf, &outsize);
    if(!ret && out_param)
        out_param->out_size = outsize;
    return ret;
}

int ad_amrnb_enc_ctl(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_ctrl_param *param = (struct vo_codec_ctrl_param *)inparam;
    int32_t cmd = param->cmd;
    void *ctrl_param = param->param;
    u32 param_size = param->param_size;

    return amrnb_enc_control(handle, cmd, ctrl_param, param_size);
}

void ad_amrnb_enc_delete(void *handle)
{
    amrnb_enc_delete(handle);
}

const struct vo_codec_ops amrnb_enc_ops = {
    .codec_create = ad_amrnb_enc_create,
    .codec_delete = ad_amrnb_enc_delete,
    .codec_process = ad_amrnb_enc_proc,
    .codec_control = ad_amrnb_enc_ctl,
};
#endif

#ifdef CONFIG_VOICE_DTMF
void *dtmf_create(void *inpuaram, void *outparam)
{
    memory_api_t mem_api;

    mem_api.heap_state = NULL;
    mem_api.pf_heap_malloc = ad_malloc;
    mem_api.pf_heap_free = ad_free;
    mem_api.scratch_state = NULL;
    mem_api.pf_scratch_malloc = ad_malloc;
    mem_api.pf_scratch_free = ad_free;

    return aic_dtmf_create(&mem_api, NULL, 0);
}

int dtmf_ctl(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_ctrl_param *param = (struct vo_codec_ctrl_param *)inparam;
    dtmf_cmd_property_t property;
    u32 value;

    /**
     *  bit0-7:   NB or WB indication;
     *  bit8-15:  key;
     *  bit16-32: volume;
     */
    value = (u32)param->param;
    property.key = (value >> 8) & 0xFF;
    property.volume_level = (value >> 16) & 0xFFFF;
    if (value & 0xFF)
        property.sampleRate = 16000;
    else
        property.sampleRate = 8000;

    AIDXI(0x12416f, "ddd", "DTMF ctl: key=%d, srate=%d, vol=%d\n",
          property.key, property.sampleRate, property.volume_level);

    return aic_dtmf_control(handle, AIC_DTMF_CMD_PROPERTY, (int *)&property,
                            sizeof(property) / sizeof(int32_t));
}

int dtmf_proc(void *handle, void *inparam, void *outparam)
{
    struct vo_codec_in_param *param = (struct vo_codec_in_param *)inparam;
    struct vo_codec_ctrl_param ctl;
    char *outbuf = param->out_buf;
    u32 outsize = param->out_size;
    int ret;

    ctl.param = (void *)param->in_size;
    ret = dtmf_ctl(handle, &ctl, NULL);
    if (ret)
        AIDXE(0x0f6d30, "d", "DTMF ctl set failed, ret=%d\n", ret);

    ret = aic_dtmf_process(handle, outbuf, outsize);
    if (ret)
        AIDXE(0x184695, "d", "dtmf proc failed, ret=%d\n", ret);

    return ret;
}

void dtmf_delete(void *handle)
{
    aic_dtmf_delete(handle);
}

const struct vo_codec_ops dtmf_ops = {
    .codec_create = dtmf_create,
    .codec_delete = dtmf_delete,
    .codec_process = dtmf_proc,
    .codec_control = dtmf_ctl,
};
#endif

static void init_codec_info(struct vo_codec_info *codec_info)
{
#ifdef CONFIG_DEC_CODEC_AMR_WB_AD
    codec_info[VO_CODEC_WB_DEC].ops = (struct vo_codec_ops *)&amrwb_dec_ops;
#endif
#ifdef CONFIG_ENC_CODEC_AMR_WB_AD
    codec_info[VO_CODEC_WB_ENC].ops = (struct vo_codec_ops *)&amrwb_enc_ops;
#endif
#ifdef CONFIG_DEC_CODEC_AMR_NB_AD
    codec_info[VO_CODEC_NB_DEC].ops = (struct vo_codec_ops *)&amrnb_dec_ops;
#endif
#ifdef CONFIG_ENC_CODEC_AMR_NB_AD
    codec_info[VO_CODEC_NB_ENC].ops = (struct vo_codec_ops *)&amrnb_enc_ops;
#endif
#ifdef CONFIG_VOICE_DTMF
    codec_info[VO_CODEC_DTMF].ops = (struct vo_codec_ops *)&dtmf_ops;
#endif
}

static int open_codec(struct vo_codec_info *codec_info, struct codec_info *info)
{

    CODEC_CREATE(VO_CODEC_WB_DEC, info, NULL);
    if (!codec_info[VO_CODEC_WB_DEC].codec_handle) {
        AIDXE(0x01fe4c, "", "open wb dec codec error!");
    }

    CODEC_CREATE(VO_CODEC_WB_ENC, info, NULL);
    if (!codec_info[VO_CODEC_WB_ENC].codec_handle) {
        AIDXE(0x19ea51, "", "open wb enc codec error!");
    }

    CODEC_CREATE(VO_CODEC_NB_DEC, info, NULL);
    if (!codec_info[VO_CODEC_NB_DEC].codec_handle) {
        AIDXE(0x1c92ff, "", "open nb dec codec error!");
    }

    CODEC_CREATE(VO_CODEC_NB_ENC, info, NULL);
    if (!codec_info[VO_CODEC_NB_ENC].codec_handle) {
        AIDXE(0x17018e, "", "open nb enc codec error!");
    }

    CODEC_CREATE(VO_CODEC_DTMF, info, NULL);
    if (!codec_info[VO_CODEC_DTMF].codec_handle) {
        AIDXE(0x1a584d, "", "open dtmf codec error!");
    }

    return AIC_SUCCESS;
}

static void close_codec(struct vo_codec_info *codec_info)
{
    CODEC_DELETE(VO_CODEC_WB_DEC);
    CODEC_DELETE(VO_CODEC_WB_ENC);
    CODEC_DELETE(VO_CODEC_NB_DEC);
    CODEC_DELETE(VO_CODEC_NB_ENC);
    CODEC_DELETE(VO_CODEC_DTMF);
}

int voice_codec_set_mode(void *handle, u32 fmt, u32 mode)
{
    struct voice_proc_obj *proc_obj = (struct voice_proc_obj *)handle;
    //struct vo_codec_info *codec_info;
    struct vo_codec_ctrl_param ctrl_param;
    u32 codec_index;
    int ret = -1;

    if (fmt == ASTREAM_FORMAT_AMR_WB)
        codec_index = VO_CODEC_WB_ENC;
    else if (fmt == ASTREAM_FORMAT_AMR_NB)
        codec_index = VO_CODEC_NB_ENC;
    else {
        AIDXE(0x0515f3, "", "voice dec: error mode!");
        return AIC_EINVAL;
    }

    ctrl_param.cmd = VOICE_CMD_SET_MODE;
    ctrl_param.param = &mode;
    ctrl_param.param_size = sizeof(mode);

    CODEC_CONTROL(ret, proc_obj->vocodec_info[codec_index], &ctrl_param, NULL);

    return ret;
}

void *voice_codec_open(struct codec_info *info)
{
    struct voice_proc_obj *proc_obj;

    proc_obj = audio_malloc(sizeof(struct voice_proc_obj));
    if (!proc_obj) {
        AIDXE(0x1aa59e, "", "alloc proj_obj error!");
        return AIC_NULL;
    }

    memset(proc_obj, 0, sizeof(*proc_obj));
    /*init codec info*/
    init_codec_info(proc_obj->vocodec_info);

    /*open codec*/
    if (open_codec(proc_obj->vocodec_info, info) != AIC_SUCCESS) {
        AIDXE(0x034067, "", "voice codec open error!");
        audio_free(proc_obj);

        return AIC_NULL;
    }

    return proc_obj;
}

int voice_codec_close(void *handle)
{
    struct voice_proc_obj *proc_obj = (struct voice_proc_obj *)handle;

    if (proc_obj) {
        close_codec(proc_obj->vocodec_info);
        audio_free(handle);
        return AIC_SUCCESS;
    }

    return AIC_EINVAL;
}

int voice_codec_dec(void *handle, u32 fmt, u8 *inbuf, u32 in_size,
                    u8 *outbuf, u32 *out_size)
{
    struct voice_proc_obj *proc_obj = (struct voice_proc_obj *)handle;
    u32 dec_out_count, codec_index;
    struct vo_codec_info *codec_info;
    struct vo_codec_in_param in_param;
    int ret = -1;

    *out_size = 0;
    if (fmt == ASTREAM_FORMAT_AMR_WB) {
        dec_out_count = AMR_WB_FRAM_PCM_SIZE;
        codec_index = VO_CODEC_WB_DEC;
    } else if (fmt == ASTREAM_FORMAT_AMR_NB) {
        dec_out_count = AMR_NB_FRAM_PCM_SIZE;
        codec_index = VO_CODEC_NB_DEC;
    } else if (fmt == ASTREAM_FORMAT_VOICE_DTMF) {
        dec_out_count = AMR_WB_FRAM_PCM_SIZE;
        codec_index = VO_CODEC_DTMF;
    } else {
        AIDXE(0x138d1f, "d", "voice dec: error fmt=%d!", fmt);
        return AIC_EINVAL;
    }
    codec_info = &proc_obj->vocodec_info[codec_index];
    in_param.in_buf = inbuf;
    in_param.in_size = in_size;
    in_param.out_buf = outbuf;
    in_param.out_size = dec_out_count;
    if (codec_info->ops && codec_info->ops->codec_process)
        ret = codec_info->ops->codec_process(codec_info->codec_handle,
                                             &in_param, NULL);
    if (ret) {
        AIDXE(0x122037, "ddddx", "voice dec single frame ret:%d, fmt=%d, insize:%d, dec_outcount:%d, d=0x%x",
              ret, fmt, in_size, dec_out_count, inbuf[0]);
        audio_dump(inbuf, in_size, 15);
        return AIC_EFAULT;
    }
    *out_size = dec_out_count;

    return AIC_SUCCESS;
}

int voice_codec_enc(void *handle, u32 fmt, u8 *inbuf,
                    u32 in_size, u8 *outbuf, u32 *out_size)
{
    struct voice_proc_obj *proc_obj = (struct voice_proc_obj *)handle;
    u32 enc_in_size, out_max_size, codec_index;
    struct vo_codec_info *codec_info;
    struct vo_codec_in_param in_param;
    struct vo_codec_out_param out_param;
    int ret = -1;

    *out_size = 0;
    if (fmt == ASTREAM_FORMAT_AMR_WB) {
        enc_in_size = AMR_WB_FRAM_PCM_SIZE;
        codec_index = VO_CODEC_WB_ENC;
        out_max_size = AMR_WB_COMPRESS_MAX_FRAME_SIZE;
    } else if (fmt == ASTREAM_FORMAT_AMR_NB) {
        enc_in_size = AMR_NB_FRAM_PCM_SIZE;
        codec_index = VO_CODEC_NB_ENC;
        out_max_size = AMR_NB_COMPRESS_MAX_FRAME_SIZE;
    } else {
        AIDXE(0x064e10, "d", "voice enc, error fmt = %d\n", fmt);
        return AIC_EINVAL;
    }
    codec_info = &proc_obj->vocodec_info[codec_index];
    in_param.in_buf = inbuf;
    in_param.in_size = enc_in_size;
    in_param.out_buf = outbuf;
    in_param.out_size = out_max_size;
    if (codec_info->ops && codec_info->ops->codec_process)
        ret = codec_info->ops->codec_process(codec_info->codec_handle,
                                             &in_param, &out_param);
    if (ret) {
        AIDXE(0x16d1fe, "ddd", "voice enc single frame ret:%d, insize:%d, dec_outcount:%d",
              ret, enc_in_size, out_max_size);
        return AIC_EFAULT;
    }
    /* this is rule with MODEM */
    if (fmt == ASTREAM_FORMAT_AMR_WB)
        outbuf[0] |= BIT_7;
    *out_size = out_param.out_size;

    return AIC_SUCCESS;
}

