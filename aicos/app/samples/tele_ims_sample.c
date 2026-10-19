/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file register ims automatically
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08fc85

/*********************
 *      INCLUDES
 *********************/
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#ifndef WIN32
#include <strings.h>
#endif
#include "aic_at_process_common.h"
#include "aic_at_parse_utils.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

static bool g_tele_ims_state = false;

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void tele_ims_register(void)
{
    u8 cmd_str[20] = {'\0'};
    s32 ret = 0;
    ulong at_req_resp = 0;

    snprintf(cmd_str, sizeof(cmd_str), "at+imsreg=1");
    ret = aic_assemble_and_handle_at_cmd(cmd_str,
                                         &at_req_resp,
                                         AT_FROM_INTERNAL,
                                         AT_NO_SERIAL_PORT_ID,
                                         AT_RETRY_COUNT0);
    alog_debug("[%s] ret = %d.\r\n", __func__, ret);
}

static void tele_handle_at_urc_cb(u8 *urc_str)
{
    u8 *p_dst_str = NULL;
    u16 string_len = 0;

    string_len = strlen(urc_str) -4;
    p_dst_str = (u8 *)aos_zalloc(string_len + 1);
    if (NULL == p_dst_str) {
        alog_error("[%s] malloc fail.\r\n", __func__);
        return;
    }
    aic_delete_start_end_symbols(p_dst_str, urc_str, string_len);

    if (0 == strlen(p_dst_str)) {
        alog_info("[%s] urc length is 0.\r\n", __func__, strlen(p_dst_str));
        aos_free(p_dst_str);
        return;
    }

    /* 网络注册的主动上报消息 */
    /* 获取+CEREG:后面的参数. */
    if (strcasestr(p_dst_str, "+CEREG: ")) {
        if (9 <= strlen(p_dst_str)) {
            /* 第一个参数代表LTE的驻网状态，值为1代表注册到本地网络，值为5代表注册到漫游网络 */
            if (('1' == p_dst_str[8]) || ('5' == p_dst_str[8])) {
                if (true == g_tele_ims_state) {
                    /* 如果当前ims已经注册成功，则不需要再次注册 */
                    alog_info("[%s] ims has registered.\r\n", __func__);
                } else {
                    /* 发起IMS注册 */
                    alog_info("[%s] Lte attach success and send at command to register ims.\r\n", __func__);
                    tele_ims_register();
                }
            } else {
                alog_info("[%s] Lte attach fail.\r\n", __func__);
            }
        } else {
            alog_error("[%s] cereg urc length(%d) is error.\r\n", __func__, strlen(p_dst_str));
        }
    } else if (strcasestr(p_dst_str, "+CIREGU: ")) {
        /* 获取+CIREGU:后面的参数. */
        if (10 <= strlen(p_dst_str)) {
            /* 该URC仅有一个参数，代表IMS的注册状态，值为1代表IMS注册成功 */
            if ('1' == p_dst_str[9]) {
                alog_info("[%s] ims register success.\r\n", __func__);
                g_tele_ims_state = true;
            } else {
                /* IMS注册失败，再次发起注册 */
                alog_info("[%s] ims register fail.\r\n", __func__);
                tele_ims_register();
                g_tele_ims_state = false;
            }
        } else {
            alog_error("[%s] cireg urc length(%d) is error.\r\n", __func__, strlen(p_dst_str));
        }
    } else {
        alog_debug("[%s] No need to care about other urc message.\r\n", __func__);
    }

    aos_free(p_dst_str);
}

static int tele_ims_init(void)
{
    aic_register_cp_urc_callback(tele_handle_at_urc_cb);

    return AIC_SUCCESS;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
/**
* tele_ims_get_state - 获取ims状态是否注册成功的接口.
* Return: true--ims注册成功，false--ims注册失败.
*/
bool tele_ims_get_state(void)
{
    return g_tele_ims_state;
}

AIC_NORMAL_APP_P2_INIT_EXPORT(tele_ims_init);
