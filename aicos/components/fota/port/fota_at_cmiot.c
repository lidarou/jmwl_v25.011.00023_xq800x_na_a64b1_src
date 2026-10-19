
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1458bf
#define LOG_MODULE    AMOD_FOTA

#include <aic_type.h>
#ifdef CONFIG_FOTA_CMIOT_CMD
#include <string.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_at_process_common.h>
#include <connmgr_api.h>
#include <aic_boot_mode.h>
#include <sysoff.h>
#include "fota_util.h"
#include "fota.h"

const char *fota_upgrade_cmiot = "+MFWUPGRADE: ";
const char *fota_rsp_ok =        "OK";
const char *fota_rsp_err =       "ERROR";

#define FOTA_MAX_PARA_RES_LEN    288

enum fota_mfwcfg_items {
    FOTA_MFWCFG_URL,
    FOTA_MFWCFG_AUTH,
    FOTA_MFWCFG_TIMEOUT,
    FOTA_MFWCFG_MODE,
    FOTA_MFWCFG_PROGIND,
    FOTA_MFWCFG_SSL,
    FOTA_MFWCFG_PKGSIZE,
    FOTA_MFWCFG_ITEM_NUM
};

enum fota_mfw_err_code {
    FOTA_AT_ERR_PARAM_ERR = 5000,
    FOTA_AT_ERR_NOT_SUPPORT,
    FOTA_AT_ERR_PROTO_ERR,
    FOTA_AT_ERR_UNKNOWN,
};

static u32 atfota_mfwcfg_set_cmd(AT_REQ_RESP *at_req)
{
    struct fota_input *in = fota_get_input_param();
    char *cfg_items[FOTA_MFWCFG_ITEM_NUM] =
                   {"url", "auth", "timeout", "mode", "progind", "ssl", "pkgsize"};
    u8 result[FOTA_MAX_PARA_RES_LEN] = {'\0'};
    ulong rsp[1];
    ulong *paras_arr;
    u32 ival;
    u16 count;
    int i = 0;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+MFWCFG=", &count);
    FLOIX(0x0d743d, "sd", "fota cmd: %s, count: %d\n", at_req->at_req, count);

    if (count < 1 || count > 3) {
        FLOEX(0x147277, "", "fota: invalid param!\n");
        snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                 FOTA_AT_ERR_PARAM_ERR);
        goto err;
    }

    for (; i < FOTA_MFWCFG_ITEM_NUM; i ++) {
        if (0 != strcmp((char *)paras_arr[0], cfg_items[i]))
            continue;
        else
            break;
    }

    if (i == FOTA_MFWCFG_ITEM_NUM) {
        FLOEX(0x0e97aa, "", "fota: item err!");
        snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                 FOTA_AT_ERR_PARAM_ERR);
        goto err;
    }

    if (count == 1) {
        /* query cmd */
        switch (i) {
            case FOTA_MFWCFG_URL:
                if (strlen(in->http.url) == 0)
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\"", cfg_items[i]);
                else
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",\"%s\"", cfg_items[i],
                            in->http.url);
                break;
            case FOTA_MFWCFG_AUTH:
                if (strlen(in->http.user_name) == 0)
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",,", cfg_items[i]);
                else
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",\"%s\",\"%s\"", cfg_items[i],
                            in->http.user_name, in->http.password);
                break;
            case FOTA_MFWCFG_TIMEOUT:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",%u", cfg_items[i],
                         (in->timeout / 1000));
                break;
            case FOTA_MFWCFG_MODE:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",%u", cfg_items[i],
                         in->mode);
                break;
            case FOTA_MFWCFG_PROGIND:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",%u", cfg_items[i],
                         in->prog_bytes);
                break;
            case FOTA_MFWCFG_SSL:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",%u", cfg_items[i],
                         in->ssl_id);
                break;
            case FOTA_MFWCFG_PKGSIZE:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+MFWCFG: \"%s\",%u", cfg_items[i],
                         in->pkg_size);
                break;
            default:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d", FOTA_AT_ERR_UNKNOWN);
                goto err;
        }

        rsp[0] = (ulong)(&result);
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    }

    if (count == 2 || count == 3) {
        if (0 == in->timeout)
            in->timeout = 60000;
        if (0 == in->pkg_size)
            in->pkg_size = 512;

        /* set cmd */
        switch (i) {
            case FOTA_MFWCFG_URL:
                if (count != 2 || strlen((char *)paras_arr[1]) >= FOTA_URL_LEN) {
                    FLOEX(0x0088eb, "d", "fota: url len err %d\n", strlen((char *)paras_arr[1]));
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                if (strcasestr((char *)paras_arr[1], "http:") != (char *)paras_arr[1] &&
                    strcasestr((char *)paras_arr[1], "https:") != (char *)paras_arr[1] &&
                    strcasestr((char *)paras_arr[1], "ftp:") != (char *)paras_arr[1]) {
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PROTO_ERR);
                    goto err;
                }
                strcpy(in->http.url, (char *)paras_arr[1]);
                break;
            case FOTA_MFWCFG_AUTH:
                if (count != 3) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                if (strlen((char *)paras_arr[1]) >= FOTA_USER_NAME_LEN) {
                    FLOEX(0x11ff83, "d", "fota: name len err, %d\n", strlen((char *)paras_arr[1]));
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                if (strlen((char *)paras_arr[2]) >= FOTA_PWD_LEN) {
                    FLOEX(0x092c5c, "d", "fota: pwd len err, %d\n", strlen((char *)paras_arr[2]));
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                strcpy(in->http.user_name, (char *)paras_arr[1]);
                strcpy(in->http.password, (char *)paras_arr[2]);
                break;
            case FOTA_MFWCFG_TIMEOUT:
                if (count != 2 || !fota_is_digit((char *)paras_arr[1])) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                ival = atoi((char *)paras_arr[1]);
                if (ival < 30 || ival > 180) {
                    FLOEX(0x06ee0d, "d", "fota: timeout param err %d\n", ival);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                in->timeout = ival * 1000;
                break;
            case FOTA_MFWCFG_MODE:
                if (count != 2 || !fota_is_digit((char *)paras_arr[1])) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                ival = atoi((char *)paras_arr[1]);
                if (ival != 0 && ival != 1) {
                    FLOEX(0x117946, "d", "fota: mode param err %d\n", ival);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                if (ival == 1) {
                    FLOEX(0x0e3e90, "", "fota: mode not support !\n");
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_NOT_SUPPORT);
                    goto err;
                }
                in->mode = (u16)ival;
                break;
            case FOTA_MFWCFG_PROGIND:
                if (count != 2 || !fota_is_digit((char *)paras_arr[1])) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                ival = atoi((char *)paras_arr[1]);
                if (ival % 1024 != 0) {
                    FLOEX(0x00984c, "d", "fota: prog bytes param err %d\n", ival);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                in->prog_bytes = ival;
                break;
            case FOTA_MFWCFG_SSL:
                if (count != 2 || !fota_is_digit((char *)paras_arr[1])) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                ival = atoi((char *)paras_arr[1]);
                if (ival > 4) {
                    FLOEX(0x1211cb, "d", "fota: ssl id err %d\n", ival);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                in->ssl_id = ival;
                break;
            case FOTA_MFWCFG_PKGSIZE:
                if (count != 2 || !fota_is_digit((char *)paras_arr[1])) {
                    FLOEX(0x12f359, "d", "fota: para err, %d\n", count);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                ival = atoi((char *)paras_arr[1]);
                if (ival % 512 != 0 || ival > 3072) {
                    FLOEX(0x0dddd1, "d", "fota: pkg size err %d\n", ival);
                    snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d",
                            FOTA_AT_ERR_PARAM_ERR);
                    goto err;
                }
                in->pkg_size = ival;
                break;
            default:
                snprintf(result, FOTA_MAX_PARA_RES_LEN, "+CME ERROR: %d", FOTA_AT_ERR_UNKNOWN);
                goto err;
        }

        if (!in->serial_port_baud) {
            in->serial_port_id = conn_get_at_com_number(conn_get_uart_at_conid);
            in->serial_port_baud = conn_cmd_get_baud_rate(CONN_CMD_AT,
                                                          in->serial_port_id);
        }
        fota_input_para_save(in);
    }

    aic_free_paras_mem(paras_arr, count);
    rsp[0] = (ulong)fota_rsp_ok;
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    return AIC_SUCCESS;
err:
    aic_free_paras_mem(paras_arr, count);
    rsp[0] = (ulong)(&result);
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);

    return AIC_SUCCESS;

}

static u32 atfota_mfwdload_exec_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];

    if (at_req->at_req[strlen("AT+MFWUPGRADE")] != 0x0D &&
        at_req->at_req[strlen("AT+MFWUPGRADE")] != 0x0A) {
        rsp[0] = (ulong)fota_rsp_err;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        return AIC_SUCCESS;
    }

    rsp[0] = (ulong)fota_rsp_ok;
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);

    if (!aic_boot_mode_is_basic()) {
#ifdef CONFIG_COMPLETE_UPGRADE_PKG
        extern void fota_diff_file_check(void);
        fota_diff_file_check();
#endif
    } else
        sysoff_reset_basic();

    return AIC_SUCCESS;
}

/* +FOTA=<url>,<port>,<user_name>,<pwd>,(0,1),(0,5-100),(0,5-100) */
const static AT_CMD_HANDLE fota_cmiot_at_handles[] =
{
    {
        "AT+MFWCFG",
        AIC_NULL,
        AIC_NULL,
        atfota_mfwcfg_set_cmd,
        AIC_NULL,
    },
    {
        "AT+MFWUPGRADE",
        AIC_NULL,
        AIC_NULL,
        AIC_NULL,
        atfota_mfwdload_exec_cmd
    },
    {
        AIC_NULL
    },
};

int atfota_cmiot_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fota_cmiot_at_handles);

    return 0;
}

int fota_urc_cmiot(u8 state, u32 para1)
{
    char urc[FOTA_IND_LEN];

    if (state == UPGRADING)
        sprintf(urc, "%s%d,%d",
                fota_upgrade_cmiot, state, para1);
    else if ((state == UPGRADE_SUCCESS) || (state == UPGRADE_FAIL))
        sprintf(urc, "%s%d",
                fota_upgrade_cmiot, state);
    else
        return 0;

    return aic_transmit_at_cmd_urc(conn_get_uart_at_conid, urc, strlen(urc));
}
#else
int fota_urc_cmiot(u8 state, u32 para1)
{
    return 0;
}
#endif
