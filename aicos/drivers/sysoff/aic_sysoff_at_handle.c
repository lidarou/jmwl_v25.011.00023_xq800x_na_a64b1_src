/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic SYSOFF AT Cmds Parsing Component.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ea448
#define LOG_MODULE    AMOD_DRV

#include <aic_init.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <aic_size.h>
#include <aic_errno.h>
#include "sysoff.h"
#include "fmd.h"
#include <aic_at_resp_handle.h>
#include <timer_inc.h>
#include <aic_sys.h>
#include <aic_boot_mode.h>
#include <subsys_api.h>
#include <aic_cmdline.h>

#define SYSOFF_MAX_AT_RESP_LENGTH        64UL
#define SYSOFF_REPLY_LINE_COUNT_ONCE     2UL

u32 aic_sysoff_autodl_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    u8 result_line0[SYSOFF_MAX_AT_RESP_LENGTH] = {'\0'};
    ulong line_pointers[SYSOFF_REPLY_LINE_COUNT_ONCE];

    u32 handle,size, part_size;
    int ret;

    ret = fmd_open("mbl", &handle);
    if (ret) {
        goto CMD_ERROR;
    }

    ret = fmd_get_partition_size(handle, &part_size);
    if (ret) {
        goto CMD_ERROR;
    }

    ret = fmd_unlock(handle, 0, part_size);
    if (ret) {
        goto CMD_ERROR;
    }

    ret = fmd_get_erase_size(handle, &size);
    if (ret) {
        goto CMD_ERROR;
    }

    ret = fmd_erase(handle, 0, size);
    if (ret) {
        goto CMD_ERROR;
    }

    strcpy(result_line0, "OK");
    /* fill response,two lines. */
    line_pointers[0] = (ulong)&result_line0[0];
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);

    sysoff_emergency_reset();

    return AIC_SUCCESS;

CMD_ERROR:
    snprintf(result_line0, SYSOFF_MAX_AT_RESP_LENGTH,
                 "+CME ERROR: %d", ret);
    line_pointers[0] = (ulong)result_line0;
    aic_fill_transmit_cmd_resp(at_req_res_ptr, line_pointers, 1);
    return ret;
}

u32 aic_sysoff_autodl_gpio_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    sysoff_autodl();
    return AIC_SUCCESS;
}

#ifndef CONFIG_CUSTOMER_CM_AT
/* *********************************************************************
 * AT^SHUTDOWN cmd start
 ***********************************************************************/
static u32 atdbg_shutdown_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    rsp[0] = (ulong)AT_RESULT_OK;

    aic_fill_transmit_cmd_resp(at_req, rsp, 1);

    sysoff_shutdown();
    return AIC_SUCCESS;
}

/* *********************************************************************
 * AT^DPSHUTDOWN cmd start
 ***********************************************************************/
static u32 atdbg_dpshutdown_set_cmd(AT_REQ_RESP *at_req)
{
    int pmic_id;

    pmic_id = aic_cmdline_get_value_int(cl_key_pmic_version);

    if (((pmic_id >> 16) & 0xFFFF) == 0x202) {
        aic_reply_ok_resp(at_req);
        sysoff_deepshutdw();
    } else {
        aic_reply_error_resp(at_req, 100);
    }
    return AIC_SUCCESS;
}


/* *********************************************************************
 * AT+QPOWD cmd start
 ***********************************************************************/
static u32 atdbg_qpowd_test_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[SYSOFF_REPLY_LINE_COUNT_ONCE];
    u8 result_line0[SYSOFF_MAX_AT_RESP_LENGTH];
    u8 result_line1[SYSOFF_MAX_AT_RESP_LENGTH];

    strcpy(result_line0, "+QPOWD:(0,1)");
    strcpy(result_line1, "OK");
    line_pointers[0] = (ulong)result_line0;
    line_pointers[1] = (ulong)result_line1;

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 2);

}

static u32 atdbg_qpowd_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[SYSOFF_REPLY_LINE_COUNT_ONCE], *paras_arr;
    char result_line0[SYSOFF_MAX_AT_RESP_LENGTH];
    u16 cnt,cid;
    int errcode = AIC_SUCCESS;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+QPOWD=",
                                                 &cnt);

    if (paras_arr != AIC_NULL) {
        cid = atoi((u8 *)*paras_arr);
        aic_free_paras_mem(paras_arr, cnt);
        if (cnt == 1 ) {
            if(cid == 1) {//Normal
                aic_reply_ok_resp(at_req);
                aic_transmit_at_cmd_urc(at_req->serial_port_id, "POWER DOWN",
                                        strlen("POWER DOWN"));

                aic_delay(3);
                sysoff_shutdown();
            } else if(cid == 0) {//Immediately
                aic_reply_ok_resp(at_req);
                aic_transmit_at_cmd_urc(at_req->serial_port_id, "POWER DOWN",
                                        strlen("POWER DOWN"));

                aic_delay(3);
                sysoff_emergency_shutdown();
            } else {
                errcode = AIC_EINVAL;
            }
        } else {
            errcode = AIC_EINVAL;
        }
    }else {
        errcode = AIC_EINVAL;
    }

    if (errcode == AIC_EINVAL) {
        snprintf(result_line0, SYSOFF_MAX_AT_RESP_LENGTH,
                 "+CME ERROR: %d", errcode);
        line_pointers[0] = (ulong)result_line0;
    }

    aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);

    return AIC_SUCCESS;
}
#endif

static u32 aic_cpon_set_cmd(AT_REQ_RESP *at_req)
{
    ulong line_pointers[SYSOFF_REPLY_LINE_COUNT_ONCE];
    static boolean is_cpon = AIC_FALSE;
    if (aic_boot_mode_is_charge() && !is_cpon) {
        subsys_power_on(AIC_SYS_CP);
        is_cpon = AIC_TRUE;
        line_pointers[0] = (ulong)AT_RESULT_OK;
    } else {
        line_pointers[0] = (ulong)"CP already on";
    }
    aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
    return AIC_SUCCESS;
}

#ifdef CONFIG_CUSTOMER_CM_AT
static u32 atdbg_mpof_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u16 cnt,mode;
    int errcode = AIC_SUCCESS;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT+MPOF=",
                                             &cnt);
    if (paras_arr != AIC_NULL) {
        mode = atoi((u8 *)*paras_arr);

        aic_free_paras_mem(paras_arr, cnt);
        if (cnt == 1 ) {
            if (mode == 1) {//Normal
                aic_reply_ok_resp(at_req);
                aic_transmit_at_cmd_urc(at_req->serial_port_id, "POWER OFF", strlen("POWER OFF"));

                aic_delay(3);
                sysoff_shutdown();
            } else if(mode == 0) {//Immediately
                aic_reply_ok_resp(at_req);
                aic_transmit_at_cmd_urc(at_req->serial_port_id, "POWER OFF", strlen("POWER OFF"));

                aic_delay(3);
                sysoff_emergency_shutdown();
            } else {
                errcode = AIC_EINVAL;
            }
        } else {
            errcode = AIC_EINVAL;
        }
    }else {
        if (cnt ==0) {
            aic_reply_ok_resp(at_req);
            aic_transmit_at_cmd_urc(at_req->serial_port_id, "POWER OFF", strlen("POWER OFF"));

            aic_delay(3);
            sysoff_emergency_shutdown();
            return AIC_SUCCESS;
        }
    }

    if (errcode == AIC_EINVAL)
        aic_reply_error_resp(at_req, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}


static u32 atdbg_mreboot_set_cmd(AT_REQ_RESP *at_req)
{
    ulong *paras_arr;
    u16 cnt,mode;
    int errcode = AIC_SUCCESS;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT+MREBOOT=",
                                             &cnt);

    if(paras_arr != AIC_NULL)
    {
        mode = atoi((u8 *)*paras_arr);

        aic_free_paras_mem(paras_arr, cnt);
        if (cnt == 1 ) {
            if ((mode == 1) || (mode == 0)) {//Hard reboot && Soft reboot
                aic_reply_ok_resp(at_req);
                aic_transmit_at_cmd_urc(at_req->serial_port_id, "REBOOTING", strlen("REBOOTING"));

                aic_delay(3);
                sysoff_reset();
            } else {
                errcode = AIC_EINVAL;
            }
        } else {
            errcode = AIC_EINVAL;
        }
    }
    else
    {
       if(cnt == 0)
       {
            aic_reply_ok_resp(at_req);
            aic_transmit_at_cmd_urc(at_req->serial_port_id, "REBOOTING", strlen("REBOOTING"));

            aic_delay(3);
            sysoff_reset();
            return AIC_SUCCESS;
       }
    }

    if (errcode == AIC_EINVAL)
        aic_reply_error_resp(at_req, AT_ERROR_INVALID_PARAS);

    return AIC_SUCCESS;
}
#endif

static AT_CMD_HANDLE sysoff_at_handles[] = {
    {"AT^AUTODL", AIC_NULL, AIC_NULL, aic_sysoff_autodl_set_cmd, AIC_NULL},
    {"AT+AUTODL", AIC_NULL, AIC_NULL, aic_sysoff_autodl_gpio_set_cmd, AIC_NULL},
#ifndef CONFIG_CUSTOMER_CM_AT
    {"AT^SHUTDOWN", AIC_NULL, AIC_NULL, atdbg_shutdown_set_cmd, AIC_NULL},
    {"AT+QPOWD", atdbg_qpowd_test_cmd, AIC_NULL, atdbg_qpowd_set_cmd, AIC_NULL},
    {"AT+CPOF", AIC_NULL, AIC_NULL, atdbg_shutdown_set_cmd, AIC_NULL},
    {"AT^DPSHUTDW", AIC_NULL, AIC_NULL, atdbg_dpshutdown_set_cmd, AIC_NULL},
#endif
    {"AT^CPON", AIC_NULL, AIC_NULL, aic_cpon_set_cmd,AIC_NULL},
#ifdef CONFIG_CUSTOMER_CM_AT
    {"AT+MPOF", AIC_NULL, AIC_NULL, atdbg_mpof_set_cmd, AIC_NULL},//ZYWL
    {"AT+MREBOOT", AIC_NULL, AIC_NULL, atdbg_mreboot_set_cmd, AIC_NULL},//ZYWL
#endif
    {AIC_NULL},
};

int sysoff_at_cmd_init(void)
{
    aic_add_at_cmd_handle(sysoff_at_handles);
    return AIC_SUCCESS;
}

AIC_COMP_L2_INIT_EXPORT(sysoff_at_cmd_init);
