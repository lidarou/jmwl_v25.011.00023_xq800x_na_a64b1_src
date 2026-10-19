
/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the blt source file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12cb44
#define LOG_MODULE    AMOD_BLT


#include <aic_errno.h>
#include <aic_io_dev.h>
#include <aic_osa.h>
#include <aic_at_process_common.h>
#include <aic_init.h>
#include <aic_sys.h>
#include "aic_blt_at_resp_handle.h"
#ifdef CONFIG_KPC
#include "kpc.h"
#endif
#ifdef CONFIG_BL6XX3
#include "tp_ts.h"
#include "bl_chip_common.h"
#endif

/**---------------------------------------------------------------------------*
 * Static Global Variables;
 **---------------------------------------------------------------------------*/
/* *********************************************************************
 * AT^SETGPIOOUT cmd start
 ***********************************************************************/


/* *********************************************************************
 * AT^READGPIOIN cmd start
 ***********************************************************************/


/* *********************************************************************
 * AT^LCDCTRL cmd start
 ***********************************************************************/


/* *********************************************************************
 * AT^BACKLIGHTCTRL cmd start
 ***********************************************************************/


/* *********************************************************************
 * AT^LCDOFF cmd start
 ***********************************************************************/

/* *********************************************************************
 * AT^CAMERATEST cmd start
 ***********************************************************************/

/* *********************************************************************
 * AT^CAMERAOFF cmd start
 ***********************************************************************/

/* *********************************************************************
 * AT^GETKEYVALUE cmd start
 ***********************************************************************/
#ifdef CONFIG_KPC
#define GET_ROW(x)                  (((x) >> 4) & 0x7)
#define GET_COL(x)                  (((x) & 0x7))
struct kpc_msg {
    u16 key0_code;
    u16 key0_event;
    u16 key1_code;
    u16 key1_event;
    u16 key2_code;
    u16 key2_event;
    u16 key3_code;
    u16 key3_event;
};

extern struct kpc_msg g_kpc_msg_store;/* for kpc blt at */

static u32 atblt_keypad_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[BLT_MAX_AT_RESP_LENGTH] = {0};
    int len;
    strcpy(rsp_str, "^GETKEYVALUE");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "KEYINx-KEYOUTy,KEYINx-KEYOUTy");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)BLT_RESULT_OK;
    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atblt_keypad_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 pressed_num = 0;
    u8 row = 0, col = 0 ;
    ulong rsp[2] = {0};
    int len = 0;
    char str_tmp[BLT_MAX_AT_RESP_LENGTH] = {0};
    char rsp_str[BLT_MAX_AT_RESP_LENGTH] = {0};

    strcpy(rsp_str, "^GETKEYVALUE");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    if (KPC_KEY_EVENT_PRESS == g_kpc_msg_store.key0_event) {
        aos_memset(str_tmp, 0, BLT_MAX_AT_RESP_LENGTH);
        row = GET_ROW(g_kpc_msg_store.key0_code);
        col = GET_COL(g_kpc_msg_store.key0_code);
        sprintf(str_tmp, "%d-%d", row, col);
        strncat(rsp_str, str_tmp, strlen(str_tmp));
        pressed_num++;
        aicx_info(0x1dfaf0, "s", " rsp_str = %s", rsp_str);
    }
    if (KPC_KEY_EVENT_PRESS == g_kpc_msg_store.key1_event) {
        aos_memset(str_tmp, 0, BLT_MAX_AT_RESP_LENGTH);
        row = GET_ROW(g_kpc_msg_store.key1_code);
        col = GET_COL(g_kpc_msg_store.key1_code);
        if (0 < pressed_num) {
            sprintf(str_tmp, ",%d-%d", row, col);
        }
        else {
            sprintf(str_tmp, "%d-%d", row, col);
        }
        strncat(rsp_str, str_tmp, strlen(str_tmp));
        pressed_num++;
        aicx_info(0x1dfaf0, "s", " rsp_str = %s", rsp_str);
    }
    if (KPC_KEY_EVENT_PRESS == g_kpc_msg_store.key2_event) {
        aos_memset(str_tmp, 0, BLT_MAX_AT_RESP_LENGTH);
        row = GET_ROW(g_kpc_msg_store.key2_code);
        col = GET_COL(g_kpc_msg_store.key2_code);
        if (0 < pressed_num) {
            sprintf(str_tmp, ",%d-%d", row, col);
        }
        else {
            sprintf(str_tmp, "%d-%d", row, col);
        }
        strncat(rsp_str, str_tmp, strlen(str_tmp));
        pressed_num++;
        aicx_info(0x1dfaf0, "s", " rsp_str = %s", rsp_str);
    }
    if (KPC_KEY_EVENT_PRESS == g_kpc_msg_store.key3_event) {
        aos_memset(str_tmp, 0, BLT_MAX_AT_RESP_LENGTH);
        row = GET_ROW(g_kpc_msg_store.key3_code);
        col = GET_COL(g_kpc_msg_store.key3_code);
        if (0 < pressed_num) {
            sprintf(str_tmp, ",%d-%d", row, col);
        }
        else {
            sprintf(str_tmp, "%d-%d", row, col);
        }
        strncat(rsp_str, str_tmp, strlen(str_tmp));
        pressed_num++;
        aicx_info(0x1dfaf0, "s", " rsp_str = %s", rsp_str);
    }

    if (0 == pressed_num) {
        sprintf(rsp_str + len, "None");
    }

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)BLT_RESULT_OK;
    return  aic_fill_transmit_cmd_resp(at_req_res_ptr, rsp, 2);
}
#endif

/* *********************************************************************
 * AT^TPID cmd start
 ***********************************************************************/
#ifdef CONFIG_BL6XX3
static u32 atblt_tpid_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[BLT_MAX_AT_RESP_LENGTH] = {0};
    char str_tmp[BLT_MAX_AT_RESP_LENGTH] = {0};
    int len;
    unsigned char chip_id = 0x00;

    strcpy(rsp_str, "^TPID");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';

    /* Get real touch pannel id */
    chip_id = bl_ts_get_chip_id();

    if(chip_id != BTL_FLASH_ID)
    {
        aicx_info(0x1e29d9, "x", "bl_update_fw_for_self_ctp:chip_id = %x\n", chip_id);
        rsp[0] = (ulong)BLT_RESULT_ERR;
        return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    }
    aicx_info(0x1e29d9, "x", "bl_update_fw_for_self_ctp:chip_id = %x\n", chip_id);

    sprintf(str_tmp, "%#X", chip_id);
    strncat(rsp_str, str_tmp, strlen(str_tmp));

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)BLT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}
#endif

#ifdef CONFIG_AUDIO_SUPPORT
/* *********************************************************************
 * AT^AUDIOTEST cmd start
 ***********************************************************************/
static u32 atblt_audio_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[BLT_MAX_AT_RESP_LENGTH] = {0};
    int len;

    strcpy(rsp_str, "^AUDIOTEST");
    len = strlen(rsp_str);
    rsp_str[len++] = ':';
    sprintf(rsp_str + len, "IN,OUT,FREQ");
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)BLT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

u32 tone_check(u32 in_dev, u32 out_dev, u32 freq, u32 *avg1, u32 *avg2);
static u32 atblt_audio_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong line_pointers[2], *paras_arr;
    u32 in_dev, out_dev, freq, avg1, avg2;
    u16 count, out_count;
    int ret = 0;
    char buf[50];

    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^AUDIOTEST", &count);
    if (count != 3) {
        ret = AIC_EPERM;
        goto ret_proc;
    }
    in_dev = atoi((char *)paras_arr[0]);
    out_dev = atoi((char *)paras_arr[1]);
    freq = atoi((char *)paras_arr[2]);

    tone_check(in_dev, out_dev, freq, &avg1, &avg2);

ret_proc:
    if (!ret) {
        sprintf(buf, "%d,%d", avg1, avg2);
        line_pointers[0] = (ulong)buf;
        line_pointers[1] = (ulong)BLT_RESULT_OK;
        out_count = 2;
    } else {
        line_pointers[0] = (ulong)BLT_RESULT_ERR;
        out_count = 1;
    }

    aic_free_paras_mem(paras_arr, count);

    return aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, out_count);
}
#endif
/*
 *  Add debug AT handle here.
 */
const static AT_CMD_HANDLE blt_at_handles[] =
{
#ifdef CONFIG_KPC
    {
        "AT^GETKEYVALUE",//no paramter
        AIC_NULL,
        atblt_keypad_query_cmd,
        atblt_keypad_set_cmd,
        AIC_NULL
    },
#endif
#ifdef CONFIG_BL6XX3
    {
        "AT^TPID",//no paramter
        AIC_NULL,
        atblt_tpid_query_cmd,
        atblt_tpid_query_cmd,
        AIC_NULL
    },
#endif
#ifdef CONFIG_AUDIO_SUPPORT
    {
        "AT^AUDIOTEST",//IN,OUT,FREQ
        AIC_NULL,
        atblt_audio_query_cmd,
        atblt_audio_set_cmd,
        AIC_NULL
    },
#endif
    {
        AIC_NULL
    },
};
int aic_blt_at_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)blt_at_handles);

    return AIC_SUCCESS;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(aic_blt_at_init);


