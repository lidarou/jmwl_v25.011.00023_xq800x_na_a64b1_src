/******************************************************************************/
/*                                                                            */
/*         Copyright 2021 by AIC Micro Corporation. All rights reserved.      */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file audio diag module file, it contains serial diag command to
 *    communication with audio PC tool.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x173c8e
#define LOG_MODULE    AMOD_AUDIO

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <cmsis_compiler.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <aic_md_api.h>
#include <aic_at_process_common.h>
#include <aic_at_cmd_handle.h>

#include "audio_util.h"
#include "audio_md.h"
#include "adsp_mng.h"
#include "voice_proc.h"
#include "audio_sink.h"
#include "audio_record.h"

struct audio_md_info {
    u32 cur_id;
};

static struct audio_md_info g_audio_md_info;

const u16 audio_md_list_music[] = {
    AUD_MD_MUSIC_RCV,
    AUD_MD_MUSIC_SPK,
    AUD_MD_MUSIC_EP,
    AUD_MD_MUSIC_SPK_EP,
};

const u16 audio_md_list_rec[] = {
    AUD_MD_REC_MAIN_MIC,
    AUD_MD_REC_EP_MIC,
};

const u16 audio_md_list_volte[] = {
    AUD_MD_VOLTE_RCV,
    AUD_MD_VOLTE_SPK,
    AUD_MD_VOLTE_EP,
};

const u16 audio_md_list_voip[] = {
    AUD_MD_VOIP_RCV,
    AUD_MD_VOIP_SPK,
    AUD_MD_VOIP_EP,
};

void audio_md_set_cur(u32 id)
{
    g_audio_md_info.cur_id = id;
}

u32 audio_md_get_cur(void)
{
    return g_audio_md_info.cur_id;
}

static u32 audio_md_get_size(u32 id)
{
    u32 size = 0;

    if (id >= AUD_MD_MUSIC_START && id <= AUD_MD_MUSIC_END)
        size = AUD_MD_SIZE_MUSIC;
    else if (id >= AUD_MD_REC_START && id <= AUD_MD_REC_END)
        size = AUD_MD_SIZE_REC;
    else if (id >= AUD_MD_VOLTE_START && id < AUD_MD_VOLTE_END)
        size = AUD_MD_SIZE_VOLTE;
    else if (id >= AUD_MD_VOIP_START && id < AUD_MD_VOIP_END)
        size = AUD_MD_SIZE_VOIP;

    return size;
}

u32 audio_md_get_cmd(u32 id)
{
    u32 cmd = 0;

    if (id >= AUD_MD_MUSIC_START && id <= AUD_MD_MUSIC_END)
        cmd = ADSP_CMD_MD_MUSIC;
    else if (id >= AUD_MD_REC_START && id <= AUD_MD_REC_END)
        cmd = ADSP_CMD_MD_REC;
    else if (id >= AUD_MD_VOLTE_START && id < AUD_MD_VOLTE_END)
        cmd = ADSP_CMD_MD_VOLTE;
    else if (id >= AUD_MD_VOIP_START && id < AUD_MD_VOIP_END)
        cmd = ADSP_CMD_MD_VOIP;

    return cmd;
}

u32 audio_md_get_count(u16 scen_id)
{
    if (scen_id == AUD_SCEN_MUSIC)
        return sizeof(audio_md_list_music) / sizeof(audio_md_list_music[0]);
    else if (scen_id == AUD_SCEN_REC)
        return sizeof(audio_md_list_rec) / sizeof(audio_md_list_rec[0]);
    else if (scen_id == AUD_SCEN_VOLTE)
        return sizeof(audio_md_list_volte) / sizeof(audio_md_list_volte[0]);
    else if (scen_id == AUD_SCEN_VOIP)
        return sizeof(audio_md_list_voip) / sizeof(audio_md_list_voip[0]);

    return 0;
}

u32 audio_md_get_base(u16 scen_id)
{
    if (scen_id == AUD_SCEN_MUSIC)
        return AUD_MD_BASE_MUSIC;
    else if (scen_id == AUD_SCEN_REC)
        return AUD_MD_BASE_REC;
    else if (scen_id == AUD_SCEN_VOLTE)
        return AUD_MD_BASE_VOLTE;
    else if (scen_id == AUD_SCEN_VOIP)
        return AUD_MD_BASE_VOIP;

    return 0;
}

int audio_md_read(u32 id, u8 *buf)
{
    u32 size;
    int ret;

    if (!buf) {
        aicx_error(0x0edcf8, "", "audio md read failed, null buff\n");
        return AIC_EINVAL;
    }
    size = audio_md_get_size(id);
    if (size == 0) {
        aicx_error(0x1726c6, "d", "wrong audio md id=%d\n", id);
        return AIC_EINVAL;
    }

    ret = aic_md_read(id, size, buf);
    if (ret) {
        aicx_error(0x044898, "d", "audio md read failed, ret=%d\n", ret);
        return ret;
    }

    return size;
}

int audio_md_write(u32 id, u8 *buf, u32 size)
{
    u32 md_size;
    int ret;

    if (!buf) {
        aicx_error(0x0e6e9f, "", "audio md write failed, null buff\n");
        return AIC_EINVAL;
    }
    md_size = audio_md_get_size(id);
    if (md_size == 0) {
        aicx_error(0x1726c6, "d", "wrong audio md id=%d\n", id);
        return AIC_EINVAL;
    }
    if (md_size != size) {
        aicx_error(0x03da62, "d", "audio md set buf failed, size(%d) not match\n", size);
        return AIC_EINVAL;
    }

    ret = aic_md_write(id, size, buf);
    if (ret)
        aicx_error(0x0a7bf0, "d", "audio md write failed, ret=%d\n", ret);

    return ret;
}

#ifdef CONFIG_ADSP_MNG
int audio_md_set(u32 id)
{
#ifdef CONFIG_SND_IPC
    struct adsp_cmd_md *cmd_ptr;
    u32 cmd;
    int ret = 0;

    cmd = audio_md_get_cmd(id);
    if (!cmd) {
        aicx_error(0x122592, "d", "audio md set, wrong id=%d\n", id);
        return AIC_EINVAL;
    }

    cmd_ptr = snd_ipc_get_cmd_buf();
    cmd_ptr->cmd = cmd;
    if (audio_md_read(id, (u8 *)&cmd_ptr->data) > 0) {
#ifdef CONFIG_ADSP_MNG
        ret = adsp_send_command(cmd_ptr);
#endif
        if (ret) {
            aicx_error(0x0b500d, "d", "audio md set failed, ret=%d\n", ret);
            return ret;
        }
        audio_md_set_cur(id);
    } else {
        snd_ipc_put_cmd_buf();
        aicx_error(0x169bd8, "", "audio md set failed, read error!\n");
        return AIC_EPERM;
    }
#endif

    return 0;
}
#elif defined(CONFIG_MCU_AUDIO)
__attribute__((weak)) int voice_proc_md_set(void *pmd)
{
    aicx_error(0x12cc24, "", "voice proc is not opened\n");
    return 0;
};

u8 audio_md_buff[AUD_MD_SIZE_MUSIC];
int audio_md_set(u32 id)
{
    int ret = 0;

    if (audio_md_read(id, audio_md_buff) > 0) {
        if ((id >= AUD_MD_VOLTE_START && id < AUD_MD_VOLTE_END) ||
            (id >= AUD_MD_VOIP_START && id < AUD_MD_VOIP_END))
            ret = voice_proc_md_set(audio_md_buff);
        else if (id >= AUD_MD_MUSIC_START && id < AUD_MD_MUSIC_END) {
#ifdef CONFIG_PLAYBACK_EFFECT
            ret = audio_sink_param_update(audio_md_buff);
#endif
        } else if (id >= AUD_MD_REC_START && id < AUD_MD_REC_END) {
#ifdef CONFIG_REC_EFFECT
            ret = audio_record_param_update(audio_md_buff);
#endif
        } else
            AIC_ASSERT(0);
        if (ret) {
            aicx_error(0x0b500d, "d", "audio md set failed, ret=%d\n", ret);
            return ret;
        }
        audio_md_set_cur(id);
    } else {
        aicx_error(0x169bd8, "", "audio md set failed, read error!\n");
        return AIC_EPERM;
    }

    return 0;
}

#else
int audio_md_set(u32 id)
{
    return 0;
}
#endif

#ifdef CONFIG_ADSP_MNG
int audio_md_set_buf(u32 id, u8 *buf, u32 size)
{
    struct adsp_cmd_md *cmd_ptr;
    u32 cmd, md_size;

    if (!buf) {
        aicx_error(0x01df36, "", "audio md set buf failed, null buff\n");
        return AIC_EINVAL;
    }

    cmd = audio_md_get_cmd(id);
    md_size = audio_md_get_size(id);
    if (cmd == 0 || md_size == 0) {
        aicx_error(0x122592, "d", "audio md set, wrong id=%d\n", id);
        return AIC_EINVAL;
    }
    if (md_size != size) {
        aicx_error(0x03da62, "d", "audio md set buf failed, size(%d) not match\n", size);
        return AIC_EINVAL;
    }

    cmd_ptr = snd_ipc_get_cmd_buf();
    cmd_ptr->cmd = cmd;
    memcpy(&cmd_ptr->data, buf, size);

    return adsp_send_command(cmd_ptr);
}
#elif defined(CONFIG_MCU_AUDIO)
int audio_md_set_buf(u32 id, u8 *buf, u32 size)
{
    u32 md_size;
    int ret = 0;

    if (!buf) {
        aicx_error(0x01df36, "", "audio md set buf failed, null buff\n");
        return AIC_EINVAL;
    }

    md_size = audio_md_get_size(id);
    if (md_size == 0) {
        aicx_error(0x122592, "d", "audio md set, wrong id=%d\n", id);
        return AIC_EINVAL;
    }
    if (md_size != size) {
        aicx_error(0x03da62, "d", "audio md set buf failed, size(%d) not match\n", size);
        return AIC_EINVAL;
    }

    memcpy(audio_md_buff, buf, size);
    if ((id >= AUD_MD_VOLTE_START && id < AUD_MD_VOLTE_END) ||
        (id >= AUD_MD_VOIP_START && id < AUD_MD_VOIP_END))
        ret = voice_proc_md_set(audio_md_buff);
    else if (id >= AUD_MD_MUSIC_START && id < AUD_MD_MUSIC_END) {
#ifdef CONFIG_PLAYBACK_EFFECT
        ret = audio_sink_param_update(audio_md_buff);
#endif
    } else if (id >= AUD_MD_REC_START && id < AUD_MD_REC_END) {
#ifdef CONFIG_REC_EFFECT
        ret = audio_record_param_update(audio_md_buff);
#endif
    } else
        AIC_ASSERT(0);

    return ret;
}
#else
int audio_md_set_buf(u32 id, u8 *buf, u32 size)
{
    return 0;
}
#endif

bool audio_md_check(u16 id)
{
    return !!aic_md_get_length(id);
}

#ifdef AUDIO_MD_AT_SUPPORT
static const char *rsp_ok =    "OK";
static const char *rsp_err =   "ERROR";
static const char *rsp_mddata = "MDDATA";
static const char *rsp_mdtest = "^AUDMD=<rw>,<type>,<index>,<size>";

#define AUDMD_R         0
#define AUDMD_W         1
#define AUDMD_S         2
#define AUDMD_R_CUR     3

#define BYTE_OF_MD_LEN  12

static u32 audio_md_at_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    rsp[0] = (ulong)rsp_mdtest;
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);

    return 0;
}

static u32 audio_md_at_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[3];
    ulong *params;
    u32 copy = 0;
    u16 count;
    int ret = 0;
    static u8  action, type, index, size;
    static char *p_md = 0, *p_head = 0;
    static u32 recv_len;

    if (at_req->at_mode == AT_DATA_MODE) {
        AIDXE(0x1c29a7, "dddd", "audmd data mode, size=%d, recv=%d, len=%d, action=%d\n", size, recv_len,
              at_req->upstream_data_len, action);
        copy = (recv_len + at_req->upstream_data_len) > size ?
                size : at_req->upstream_data_len;
        memcpy(p_md + recv_len, at_req->at_upstream_data, copy);
        recv_len += copy;
        if (recv_len >= size) {
            if (action == AUDMD_W)
                ret = audio_md_write(index, p_md, size);
            else if (action == AUDMD_S)
                ret = audio_md_set_buf(index, p_md, size);
            else
                AASSERT(0);

            if (ret >= 0)
               rsp[0] = (ulong)rsp_ok;
            else
               rsp[0] = (ulong)rsp_err;

            aic_fill_transmit_cmd_resp(at_req, rsp, 1);
            at_req->at_mode = AT_CMD_MODE;
        }
        return 0;
    }

    params = aic_split_paras_to_str_array(at_req->at_req, "AT^AUDMD=", &count);
    if (!params) {
        rsp[0] = (ulong)rsp_err;
        AIDXE(0x139fab, "s", "%s:malloc fail for splitting\n", __func__);
        return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    }

    if (count != 4) {
        AIDXE(0x0958d2, "d", "param count(%d) is not match\n", count);
        goto free_param;
    }

    if (!p_head)
        p_head = audio_malloc(BYTE_OF_MD_LEN + AUD_MD_SIZE_MAX);
    if (!p_head) {
        AIDXE(0x13b32c, "", "no enough memory\n");
        goto free_param;
    }
    memset(p_head, 0, BYTE_OF_MD_LEN + AUD_MD_SIZE_MAX);
    if (!p_md)
        p_md = p_head + BYTE_OF_MD_LEN;
    action = atoi((char *)params[0]);
    type = atoi((char *)params[1]);
    index = atoi((char *)params[2]);
    size = atoi((char *)params[3]);

    AIDXI(0x143616, "ddddd", "+AUDMD params: %d, %d, %d, %d=%d\n", action, type, index, size);
    switch(action) {
    case AUDMD_R_CUR:
        index = audio_md_get_cur();
    case AUDMD_R:
        ret = audio_md_read(index, p_md);
        if (ret <= 0 || ret != size) {
            rsp[0] = (ulong)rsp_err;
            aic_fill_transmit_cmd_resp(at_req, rsp, 1);
            AIDXE(0x1008c1, "d", "read failed or size not match, ret_size=%d\n", ret);
            goto free_param;
        }
        sprintf(p_head, "%d", ret);
        rsp[0] = (ulong)p_head;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);

        aic_transmit_at_data(at_req->serial_port_id, p_md, size);

        rsp[0] = (ulong)rsp_ok;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        break;
    case AUDMD_W:
    case AUDMD_S:
        rsp[0] = (ulong)rsp_mddata;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        recv_len = 0;
        at_req->at_mode = AT_DATA_MODE;
        break;
    default:
        break;
    }

free_param:
    aic_free_paras_mem(params, count);

    return 0;
}

/* +AUDMD=<action>,<type>,<index>,<size> */
const static AT_CMD_HANDLE audio_md_at_handles[] =
{
    {
        "AT^AUDMD",
        audio_md_at_test_cmd,
        AIC_NULL,
        audio_md_at_set_cmd,
        AIC_NULL
    },
    {
        AIC_NULL
    },
};

static int audio_md_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)audio_md_at_handles);

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(audio_md_init);

#endif
