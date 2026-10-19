/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is audio AT command file, in which we defined audio play&record
 *  etc. AT command.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x157658
#define LOG_MODULE    AMOD_AUDIO

#include <aic_at_cmd_handle.h>
#include <aic_at_process_common.h>
#include <aic_cmd_api.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_type.h>

#include "audio_system.h"
#include "audio_service.h"

struct aud_at_comm_ctx {
#ifdef CONFIG_AUDIO_AT_DTMF
    u8 vtd;
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    u32 d2d_state;
#endif
};

enum {
    AT_CLVL = 0,
#ifdef CONFIG_AUDIO_AT_DTMF
    AT_VTD,
    AT_VTS,
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    AT_DVR,
    AT_DVC,
#endif
    AT_MAX,
};

const static char *rsp_ok = "OK";
const static char *rsp_err = "ERROR";
/* const static char *rsp_err = "ERROR";*/

const static char *aud_tag_arr[] = {
    "+CLVL: ",
#ifdef CONFIG_AUDIO_AT_DTMF
    "+VTD: ",
    "+VTS: ",
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    "+DVR: ",
    "+DVC: ",
#endif
};

const static char *rsp_query_arr[] = {
    "+CLVL:(1-5)",
#ifdef CONFIG_AUDIO_AT_DTMF
    "+VTD: (0-255)",
    "+VTS: (0-9,*,#,A,B,C,D),(0-255)",
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    "+DVR: (0-7)",
    "+DVC: (0, 1)",
#endif
};

static struct aud_at_comm_ctx g_at_comm_ctx;

static s32 rsp_cmd(AT_REQ_RESP *at_req, const char *str1, const char *str2)
{
    ulong rsp[2];
    u16 cnt;

    if (str1) {
        cnt = 1;
        rsp[0] = (ulong)str1;
        if (str2) {
            cnt++;
            rsp[1] = (ulong)str2;
        }
    } else
        return AIC_EPERM;

    return aic_fill_transmit_cmd_resp(at_req, rsp, cnt);
}

static u32 test_cmd(AT_REQ_RESP *at_req, u32 id)
{
    ulong rsp[2];
    u16 num;

    if (id > AT_MAX) {
        AIDXE(0x0cecb2, "d", "invalid audio at id = %d\n", id);

        return AIC_EPERM;
    }

    if (rsp_query_arr[id]) {
        rsp[0] = (ulong)rsp_query_arr[id];
        rsp[1] = (ulong)rsp_ok;
        num = 2;
    } else {
        rsp[0] = (ulong)rsp_ok;
        num = 1;
    }

    return aic_fill_transmit_cmd_resp(at_req, rsp, num);
}

static u32 maudclvl_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_CLVL);
}

static u32 maudclvl_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s%d", aud_tag_arr[AT_CLVL], audio_get_master_volume() / 3);

    return rsp_cmd(at_req, str, rsp_ok);
}

static u32 maudclvl_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u32 vol;
    u16 count;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+CLVL=", &count);
    if (!paras_arr || count == 0) {
        AIDXE(0x15ff43, "", "CLVL split failed\n");
        goto err_param;
    }

    vol = atoi((char *)paras_arr[0]);
    if (vol > 5) {
        AIDXE(0x132d3b, "", "CLVL invalid vol\n");
        goto err;
    }

    ret = audio_set_master_volume(vol * 3);
    if (ret) {
        AIDXE(0x02597e, "d", "CLVL set failed, ret=%d\n", ret);
        goto err;
    }
    aic_free_paras_mem(paras_arr, count);
    rsp_cmd(at_req, rsp_ok, 0);

    return 0;
err:
    aic_free_paras_mem(paras_arr, count);
err_param:
    return rsp_cmd(at_req, rsp_err, 0);
}

#ifdef CONFIG_AUDIO_AT_DTMF
static u32 maudvtd_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_VTD);
}

static u32 maudvtd_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s%d", aud_tag_arr[AT_VTD], g_at_comm_ctx.vtd);

    return rsp_cmd(at_req, str, rsp_ok);
}

static u32 maudvtd_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line0[20];
    AT_RESP_FROM_CP resp_to_save;

    ulong *paras_arr;
    u32 vtd;
    u16 count;

    resp_to_save.resp_buf = result_line0;
    resp_to_save.buf_len = 20;
    aic_send_cmd_and_wait(at_req->at_req, &resp_to_save);
    if (strcasestr(resp_to_save.resp_buf, "ERROR")) {
        aic_reply_to_certain_com(at_req->serial_port_id, resp_to_save.resp_buf, strlen(resp_to_save.resp_buf));

        return AIC_SUCCESS;
    }

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+VTD=", &count);
    if (!paras_arr || count == 0) {
        AIDXE(0x13d936, "", "VTD split failed\n");
        goto err_param;
    }

    vtd = atoi((char *)paras_arr[0]);
    if (vtd > 255) {
        AIDXE(0x02d6f2, "", "VTD invalid vol\n");
        goto err;
    }

    g_at_comm_ctx.vtd = vtd;
    aic_free_paras_mem(paras_arr, count);
    rsp_cmd(at_req, rsp_ok, 0);

    return 0;
err:
    aic_free_paras_mem(paras_arr, count);
err_param:
    return rsp_cmd(at_req, rsp_err, 0);
}

static u32 maudvts_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_VTS);
}

static u32 maudvts_query_cmd(AT_REQ_RESP *at_req)
{
    return rsp_cmd(at_req, rsp_ok, NULL);
}

static u32 maudvts_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line0[20];
    AT_RESP_FROM_CP resp_to_save;
    ulong *paras_arr;
    u32 tone_id, duration;
    u16 count;

    resp_to_save.resp_buf = result_line0;
    resp_to_save.buf_len = 20;
    aic_send_cmd_and_wait(at_req->at_req, &resp_to_save);
    if (strcasestr(resp_to_save.resp_buf, "ERROR")) {
        aic_reply_to_certain_com(at_req->serial_port_id, resp_to_save.resp_buf, strlen(resp_to_save.resp_buf));

        return AIC_SUCCESS;
    }

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+VTS=", &count);
    if (!paras_arr || count == 0) {
        AIDXE(0x13d936, "", "VTD split failed\n");
        return rsp_cmd(at_req, rsp_err, 0);
    }

    tone_id = atoi((char *)paras_arr[0]);
    duration = count > 1 ? atoi((char *)paras_arr[1]) : g_at_comm_ctx.vtd;
    duration *= 100;
    audio_play_tone(tone_id, duration);

    aic_free_paras_mem(paras_arr, count);

    return rsp_cmd(at_req, rsp_ok, 0);
}
#endif

#ifdef CONFIG_AUDIO_AT_D2D
static u32 dvr_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_DVR);
}

static u32 dvr_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];
    u32 rate = audio_get_voice_rate();

    rate &= 0xFFFF;
    snprintf(str, 30, "%s%d\n", aud_tag_arr[AT_DVR], rate);

    return rsp_cmd(at_req, str, 0);
}

static u32 dvr_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u32 rate, err = 0;
    u16 count;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+DVR=", &count);
    if (!paras_arr || count != 1) {
        AIDXE(0x11747b, "", "+DVR split failed\n");
        err = 100;
        goto ERROR_EXIT;
    }

    rate = *(u8 *)paras_arr[0];
    if (rate < '0' || rate > '7') {
        err = 101;
        goto ERROR_EXIT;
    }
    rate = rate - '0';
    audio_set_voice_rate(8000, rate);
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err);

    return 0;
}

static u32 dvc_test_cmd(AT_REQ_RESP *at_req)
{
    return test_cmd(at_req, AT_DVC);
}

static u32 dvc_query_cmd(AT_REQ_RESP *at_req)
{
    char str[30];

    snprintf(str, 30, "%s%s\n", aud_tag_arr[AT_DVC],
             g_at_comm_ctx.d2d_state ? "ON" : "OFF");

    return rsp_cmd(at_req, str, 0);
}

static u32 dvc_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u32 ctl, err = 0;
    u16 count;
    int ret;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+DVC=", &count);
    if (!paras_arr || count != 1) {
        AIDXE(0x04b29e, "", "+DVC split failed\n");
        err = 100;
        goto ERROR_EXIT;
    }

    ctl = *(u8 *)paras_arr[0];
    if (ctl != '0' && ctl != '1') {
        err = 101;
        goto ERROR_EXIT;
    }

    if (ctl == '1' && !g_at_comm_ctx.d2d_state) {
        ret = audio_start_voice(TRUE, TRUE);
        if (ret) {
            err = -ret;
            goto ERROR_EXIT;
        }
        g_at_comm_ctx.d2d_state = TRUE;
    } else if (ctl == '0' && g_at_comm_ctx.d2d_state) {
        ret = audio_stop_voice();
        if (ret) {
            ret = -err;
            goto ERROR_EXIT;
        }
        g_at_comm_ctx.d2d_state = FALSE;
    } else {
        err = 102;
        goto ERROR_EXIT;
    }
    aic_reply_ok_resp(at_req);

    return 0;

ERROR_EXIT:
    aic_free_paras_mem(paras_arr, count);
    aic_reply_error_resp(at_req, err);

    return 0;
}
#endif

const static AT_CMD_HANDLE audio_at_comm_handles[] = {
    {"AT+CLVL", maudclvl_test_cmd, maudclvl_query_cmd, maudclvl_set_cmd, AIC_NULL},
#ifdef CONFIG_AUDIO_AT_DTMF
    {"AT+VTD", maudvtd_test_cmd, maudvtd_query_cmd, maudvtd_set_cmd, AIC_NULL},
    {"AT+VTS", maudvts_test_cmd, maudvts_query_cmd, maudvts_set_cmd, AIC_NULL},
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    {"AT+DVR", dvr_test_cmd, dvr_query_cmd, dvr_set_cmd, AIC_NULL},
    {"AT+DVC", dvc_test_cmd, dvc_query_cmd, dvc_set_cmd, AIC_NULL},
#endif
    {AIC_NULL},
};

static int audio_at_comm_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)audio_at_comm_handles);
#ifdef CONFIG_AUDIO_AT_DTMF
    g_at_comm_ctx.vtd = 3;                       /* 300ms */
#endif
#ifdef CONFIG_AUDIO_AT_D2D
    g_at_comm_ctx.d2d_state = 0;
#endif

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(audio_at_comm_init);

