
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x04170b
#define LOG_MODULE    AMOD_FOTA

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_at_process_common.h>
#include <aic_at_cmd_handle.h>
#include <connmgr_api.h>
#include <sysoff.h>
#include <sys_mem_api.h>

#include "fota.h"
#include "fota_util.h"
#include <slp_inc.h>

#define FOTA_DATA_MODE_TIMEOUT 600

const char *atfota_ind =    "+IND:";
const char *atfota_prefix = "FOTA";

const char *atfota_st_none =       "NONE";
const char *atfota_st_httpstart =  "HTTPSTART";
const char *atfota_st_dl =         "DOWNLOADING";
const char *atfota_st_httpend =    "HTTPEND";
const char *atfota_st_check =      "CHECK";
const char *atfota_st_update =     "UPDATING";
const char *atfota_st_ext_update = "EXT\",\"UPDATING";
const char *atfota_st_rfs_update = "ROMFS\",\"UPDATING";
const char *atfota_st_end =        "END";
const char *atfota_st_cert_fail =  "CERT DL FAIL";
const char *atfota_st_dl_fail =    "DL FAIL";
const char *atfota_st_reset =      "Diff file downloaded! Reset to upgrade!";
const char *atfota_st_ver_no_match = "VER NO MATCH";

#ifndef CONFIG_FOTA_BY_UART
const char *atfota_rsp_test =  "+FOTA=<url>,<port>,<user_name>,<pwd>,(0,1),(0,5-100),(0,5-100)";
#else
const char *atfota_rsp_test =  "+FOTA=<cmd>,<fw_size>,<sub_package_size>,<mode>,<upd_step>";
#endif
const char *atfota_rsp_ok =      "OK";
const char *atfota_rsp_err =     "ERROR";
const char *atfota_rsp_query =   "+FOTA:STATE ";
const char *atfota_rsp_send_ok = "SEND OK";
const char *atfota_rsp_finish =  "FINISHED";

#define AT_FOTACRT          "AT+FOTACRT=<size>"
#define STR_FOTACRT_START   "FOTACRT START"

struct atfota_ctx {
    struct fota_input in;
    u32 crt_len_recv;
};

#ifdef CONFIG_FOTA_BY_MCU
struct atfota_data_info {
    struct FOTA_RAMDATA_INFO ramdata;
    u32 recv_len;
};
#endif

#ifdef CONFIG_FOTA_BY_UART
struct atfota_uart_ctx {
    u32 ota_cmd_id;
    u32 ota_fw_size;
    u32 ota_package_size;
    u32 ota_recv_size;
};

struct atfota_uart_ctx uart_ctx = {0};
#endif

#ifdef CONFIG_FOTA_BY_MCU
struct atfota_data_info atfota_data;
#endif
struct atfota_ctx atfota;

static int atfota_urc(const char *st, int count, u32 para1,
                      u32 para2, u32 para3)
{
    char urc[FOTA_IND_LEN];

    if (count == 0)
        sprintf(urc, "%s\"%s\",\"%s\"",
                atfota_ind, atfota_prefix, st);
    else if (count == 1)
        sprintf(urc, "%s\"%s\",\"%s\",%d",
                atfota_ind, atfota_prefix, st, para1);
    else if (count == 2)
        sprintf(urc, "%s\"%s\",\"%s\",%d,%d",
                atfota_ind, atfota_prefix, st, para1, para2);
    else if (count == 3)
        sprintf(urc, "%s\"%s\",\"%s\",%d,%d,%d",
                atfota_ind, atfota_prefix, st, para1, para2, para3);
    else
        return 0;
#ifndef CONFIG_FOTA_BY_MCU
    return aic_transmit_at_cmd_urc(conn_get_uart_at_conid, urc, strlen(urc));
#else
    FLOIX(0x11a77f, "ds", "atfota_urc:port_id=%d, str is %s\n.", fota_get_input_param()->serial_port_id, urc);
    return aic_transmit_at_cmd_urc(fota_get_input_param()->serial_port_id, urc, strlen(urc));
#endif
}

void atfota_callback(void *user_data, int event, u32 data1, u32 data2)
{
    switch (event) {
    case FOTA_EVENT_CONNECTED:
        atfota_urc(atfota_st_httpstart, 1, data2, 0, 0);
        break;
    case FOTA_EVENT_DL_PROGRESS:
        atfota_urc(atfota_st_dl, 2, data1, data2, 0);
        break;
    case FOTA_EVENT_DL_DONE:
        atfota_urc(atfota_st_httpend, 2, data1, data2, 0);
        break;
    case FOTA_EVENT_CHECK:
        atfota_urc(atfota_st_check, 1, data1, 0, 0);
        break;
    case FOTA_EVENT_CERT_FAIL:
        atfota_urc(atfota_st_cert_fail, 1, data2, 0, 0);
        break;
    case FOTA_EVENT_DL_FAIL:
        atfota_urc(atfota_st_dl_fail, 1, data2, 0, 0);
        break;
    case FOTA_EVENT_EXT_UPDATE_PROGRESS:
        atfota_urc(atfota_st_ext_update, 1, data1, 0, 0);
        break;
    case FOTA_EVENT_RFS_UPDATE_PROGRESS:
        atfota_urc(atfota_st_rfs_update, 1, data1, 0, 0);
        break;
    case FOTA_EVENT_RESET_BASIC:
        atfota_urc(atfota_st_reset, 0, 0, 0, 0);
        break;
    case FOTA_EVENT_VER_NOT_MATCH:
        atfota_urc(atfota_st_ver_no_match, 1, data2, 0, 0);
        break;
    default:
        break;
    }
}

static u32 atfota_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)atfota_rsp_test;
    rsp[1] = (ulong)atfota_rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atfota_query_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[50];
    u32 state = fota_get_state();

    strcpy(rsp_str, atfota_rsp_query);

    if (state == 0)
        strcat(rsp_str, atfota_st_none);
    else if (state < FOTA_STATE_DONE)
        strcat(rsp_str, atfota_st_dl);
    else
        strcat(rsp_str, atfota_st_update);

    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)atfota_rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static void fota_get_port_id_and_baudrate(struct fota_input *in, AT_REQ_RESP *at_req)
{
#ifndef CONFIG_FOTA_BY_MCU
    u32 conid = conn_get_uart_at_conid;
    if (conid != AIC_ENODEV) {
        in->serial_port_id = conn_get_at_com_number(conid);
        in->serial_port_baud = conn_cmd_get_baud_rate(CONN_CMD_AT,
                                                      in->serial_port_id);
    } else {
        in->serial_port_id = 0xff;
        in->serial_port_baud = 0;
    }
#else
    in->serial_port_id = at_req->serial_port_id;
#endif
}

#ifndef CONFIG_FOTA_BY_UART
static u32 atfota_set_cmd(AT_REQ_RESP *at_req)
{
    struct fota_input *in = &atfota.in;
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    u32 ival;

    paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                             "AT+FOTA=", &count);
    if (!paras_arr || (count != 8 && count != 7)) {
        FLOEX(0x13a54c, "", "malloc fail for splitting\n");
        goto err;
    }

    ival = strlen((char *)paras_arr[0]);
#ifdef CONFIG_FOTA_BY_MCU
    if (ival != 0)
        goto err;
#else
    if ((ival == 0) || (ival >= (FOTA_URL_LEN-1)))
        goto err;
#endif
    strncpy(in->http.url, (char *)paras_arr[0], FOTA_URL_LEN);

    if (!fota_is_digit((char *)paras_arr[1]) ||
        !fota_is_digit((char *)paras_arr[4]) ||
        !fota_is_digit((char *)paras_arr[5]) ||
        !fota_is_digit((char *)paras_arr[6]))
        goto err;

    if (count == 8) {
        if (!fota_is_digit((char *)paras_arr[7]))
            goto err;
    }

    in->http.port = atoi((char *)paras_arr[1]);

    strncpy(in->http.user_name, (char *)paras_arr[2], FOTA_USER_NAME_LEN);
    strncpy(in->http.password, (char *)paras_arr[3], FOTA_PWD_LEN);

    ival = atoi((char *)paras_arr[4]);
    if (ival <= 1)
        in->upgrd_mode = ival;
    else
        goto err;

    ival = atoi((char *)paras_arr[5]);
    if ((ival <= 100 && ival >= 5) || (ival == 0))
        in->dl_urc = ival;
    else
        goto err;

    ival = atoi((char *)paras_arr[6]);
    if ((ival <= 100 && ival >= 5) || (ival == 0))
        in->upgrd_urc = ival;
    else
        goto err;

    if (count == 8) {
        ival = atoi((char *)paras_arr[7]);
        if ((ival == FOTA_UPG_ALL) || (ival == FOTA_UPG_APP_FILE))
            in->upgrd_type = ival;
        else
            goto err;
    } else {
        in->upgrd_type = FOTA_UPG_ALL;
    }

    aic_free_paras_mem(paras_arr, count);

    /* save port id for later response */
    fota_get_port_id_and_baudrate(in, at_req);
    fota_input_para_save(in);

    rsp[0] = (ulong)atfota_rsp_ok;
    aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    sysoff_reset_basic();

err:
    aic_free_paras_mem(paras_arr, count);
    rsp[0] = (ulong)atfota_rsp_err;
    return aic_fill_transmit_cmd_resp(at_req, rsp, 1);
}
#else
enum {
    FOTA_UART_TEXT_DATA_MODE = 0,
    FOTA_UART_RAW_DATA_MODE
};

#define FOTA_DATA_SAVE(buf, size) do { \
    if (uart_ctx.ota_recv_size + size <= uart_ctx.ota_fw_size \
        && !fota_uart_data_save(buf, size, uart_ctx.ota_fw_size)) { \
        uart_ctx.ota_recv_size += size; \
        rsp[0] = (ulong)atfota_rsp_ok; \
        aic_fill_transmit_cmd_resp(at_req, rsp, 1); \
 \
        if (uart_ctx.ota_fw_size == uart_ctx.ota_recv_size) { \
            FOTA_NOTIFY(FOTA_EVENT_RESET_BASIC, 0, 0); \
            sysoff_reset_basic(); \
        } \
    } else { \
        at_req->at_mode = AT_CMD_MODE; \
        uart_ctx.ota_recv_size = 0; \
        uart_ctx.ota_fw_size = 0; \
        rsp[0] = (ulong)atfota_rsp_err; \
        aic_fill_transmit_cmd_resp(at_req, rsp, 1); \
    } \
} while (0)

#define FOTA_ASYN_DATA_SAVE(buf, size) do { \
    if (uart_ctx.ota_recv_size + size <= uart_ctx.ota_fw_size \
        && !fota_uart_data_save(buf, size, uart_ctx.ota_fw_size)) { \
        uart_ctx.ota_recv_size += size; \
 \
        if (uart_ctx.ota_fw_size == uart_ctx.ota_recv_size) { \
            FOTA_NOTIFY(FOTA_EVENT_RESET_BASIC, 0, 0); \
            sysoff_reset_basic(); \
        } \
    } \
} while (0)

typedef struct fota_pac_data{
    u8 *buffer;
    u32 size;
} fota_pac_data_t;

static void atfota_save_flash_task(void *p)
{
    aos_queue_ptr *queue = (aos_queue_ptr *)p;
    fota_pac_data_t *pac_data;
    aos_status ret;
    int count = 0;

    while ((u32 volatile)uart_ctx.ota_fw_size) {
        ret = aos_recv_from_queue(queue, (void*)&pac_data, AOS_WAIT_FOREVER);
        if (ret == AOS_SUCCESS) {
            FOTA_ASYN_DATA_SAVE(pac_data->buffer, pac_data->size);
            aos_free(pac_data->buffer);
            aos_free(pac_data);
            count++;
            if(count % 10 == 0)
               aos_sleep(10);
        }
    }
}

/**********************************************************************
* UART FOTA AT commands usage:
*   1. at+fota=ota_start,fw_size,package_size,upgrd_mode,upd_step
*   2. at+fota=ota_data,package_seq,data_size,"data"
*   ota_start: 1 for this command;
*   ota_data: 2 for this command, if upgrd_mode is 1, no need this sub command.
*
*   fw_size: total size of OTA diff package
*   package_size: sizeof individual sub-package
*   upgrd_mode: 0: text data mode, need convert the data to hex string.
                1: raw data mode, just send the raw data of the OTA diff package.
*   upd_step: Download notification maximum count,5-100; if 0, do not
*             send download URC;

*   The bellow params is not used in raw data mode.
*
*   package_seq: sub-package sequence index;
*   data_size: size of current sub-packages received;
*   "data": text of hex string;
***********************************************************************/
static u32 atfota_set_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[1];
    ulong *paras_arr;
    u16 count;
    int i = 0;
    u32 uart_data_len = 0;
    u32 part_data_len = 0;
    struct fota_input *in = &atfota.in;
    u8 *buffer;
    aos_status ret;
    static aos_thread_id save_thread = AOS_INVALID_THREAD_ID;
    static aos_queue_ptr save_queue = NULL;

    FLOIX(0x0a31de, "d", "AT+FOTA, at_mode=%d\n", at_req->at_mode);

    /* data mode */
    if (at_req->at_mode == AT_DATA_MODE) {
        static u32 ota_recv_size = 0;

        uart_data_len = at_req->upstream_data_len;
        FLOIX(0x19e3ac, "d", "recv data, data_len=%d\n", uart_data_len);

        if (uart_data_len && ota_recv_size + uart_data_len <= uart_ctx.ota_fw_size) {
            ota_recv_size += uart_data_len;
            fota_pac_data_t *pac_data = aos_malloc(sizeof(*pac_data));
            buffer = aos_malloc(uart_data_len);
            AIC_ASSERT(buffer);
            memcpy(buffer, at_req->at_upstream_data, uart_data_len);
            pac_data->buffer = buffer;
            pac_data->size = uart_data_len;
            ret = aos_send_to_queue(save_queue, (void*)&pac_data, AOS_NO_WAIT);
            AIC_ASSERT(ret == AOS_SUCCESS);
            rsp[0] = (ulong)atfota_rsp_ok;
            aic_fill_transmit_cmd_resp(at_req, rsp, 1);

            if (ota_recv_size == uart_ctx.ota_fw_size)
                at_req->at_mode = AT_CMD_MODE;
        } else {
            if (ota_recv_size + uart_data_len > uart_ctx.ota_fw_size) {
                at_req->at_mode = AT_CMD_MODE;
                ota_recv_size = 0;
                uart_ctx.ota_recv_size = 0;
                uart_ctx.ota_fw_size = 0;
            }
            rsp[0] = (ulong)atfota_rsp_err;
            aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        }
        return 0;
    }

    if (save_queue) {
        aos_delete_queue(save_queue);
        save_thread = 0;
    }
    if (save_thread != AOS_INVALID_THREAD_ID) {
        aos_delete_thread(save_thread);
        save_thread = AOS_INVALID_THREAD_ID;
    }

    paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT+FOTA=", &count);
    FLOIX(0x0d743d, "sd", "fota cmd: %s, count: %d\n", at_req->at_req, count);

    if (count != 4 && count != 5) {
        aic_free_paras_mem(paras_arr, count);
        rsp[0] = (ulong)atfota_rsp_err;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        return 0;
    }
    uart_ctx.ota_cmd_id = atoi((char *)paras_arr[0]);

    if (uart_ctx.ota_cmd_id == 1 && count == 5) {
        uart_ctx.ota_fw_size = atoi((char *)paras_arr[1]);
        uart_ctx.ota_package_size = atoi((char *)paras_arr[2]);
        in->upgrd_mode = atoi((char *)paras_arr[3]);
        in->dl_urc = atoi((char *)paras_arr[4]);
        in->upgrd_urc = in->dl_urc;

        FLOIX(0x090d22, "ddddd", "fota: cmd id:%d, fw size:%d, package size:%d, upgrd mode:%d, dl urc: %d\n",
              uart_ctx.ota_cmd_id, uart_ctx.ota_fw_size, uart_ctx.ota_package_size,
              in->upgrd_mode, in->dl_urc);
        aic_free_paras_mem(paras_arr, count);

        /* save port id for later response */
        fota_get_port_id_and_baudrate(in, at_req);
        fota_ctx.cb = atfota_callback;
        uart_ctx.ota_recv_size = 0;
        fota_uart_data_reset();
        fota_input_para_save(in);

        /* power off cp for write flash faster. */
        if (!aic_boot_mode_is_basic())
            sysoff_stop_cp();

        /* change at to data modem */
        if (in->upgrd_mode == FOTA_UART_RAW_DATA_MODE) {
            at_req->at_mode = AT_DATA_MODE;
            save_queue = aos_create_queue("otasv", sizeof(void*), 200);
            AIC_ASSERT(save_queue);
            save_thread = aos_create_thread_simply("otasv", atfota_save_flash_task,
                                                   (void*)save_queue, 4096, 15);
            AIC_ASSERT(save_thread != AOS_INVALID_THREAD_ID);

            aic_set_data_mode_expire_time("AT+FOTA", FOTA_DATA_MODE_TIMEOUT * 1000);
        }

        rsp[0] = (ulong)atfota_rsp_ok;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    } else if (uart_ctx.ota_cmd_id == 2 && count == 4
             && in->upgrd_mode == FOTA_UART_TEXT_DATA_MODE) {
        if (uart_ctx.ota_fw_size != 0) {
            for (i = 0; i < 3; i++)
                FLOIX(0x0a96d3, "dd", "fota fw_data[%d]=%d\n", i, atoi((char *)paras_arr[i]));
            part_data_len = atoi((char *)paras_arr[i - 1]);
            uart_data_len = strlen((char *)paras_arr[i]);
            FLOIX(0x0ce024, "dd", "fota fw data len: %d, recv len: %d\n", uart_data_len, part_data_len);
            if (part_data_len != uart_data_len) {
                aic_free_paras_mem(paras_arr, count);
                rsp[0] = (ulong)atfota_rsp_err;
                aic_fill_transmit_cmd_resp(at_req, rsp, 1);
                return 0;
            }

            buffer = aos_malloc(uart_data_len);
            AIC_ASSERT(buffer);

            fota_string2hex((char *)paras_arr[i], buffer, uart_data_len);
            uart_data_len = uart_data_len/2;
            aic_free_paras_mem(paras_arr, count);

            FLODX(0x1244bf, "xxxxxxxx", "fota diff[0-8]: %x,%x,%x,%x,%x,%x,%x,%x\n",
                buffer[0], buffer[1], buffer[2], buffer[3],
                buffer[4], buffer[5], buffer[6],buffer[7]);

            FOTA_DATA_SAVE(buffer, uart_data_len);
        } else {
            rsp[0] = (ulong)atfota_rsp_err;
            aic_fill_transmit_cmd_resp(at_req, rsp, 1);
        }
    } else {
        FLOIX(0x015f86, "", "AT cmd error.\n");
        aic_free_paras_mem(paras_arr, count);
        rsp[0] = (ulong)atfota_rsp_err;
        aic_fill_transmit_cmd_resp(at_req, rsp, 1);
    }

    return 0;
}
#endif

static u32 atfota_crt_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)AT_FOTACRT;
    rsp[1] = (ulong)atfota_rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

static u32 atfota_crt_set_cmd(AT_REQ_RESP *at_req)
{
    struct fota_input *in = &atfota.in;
    u32 status = AIC_SUCCESS, new_size;
    u16 count;
    ulong *paras_arr;

    FLOIX(0x062ffb, "d", "atfota_crt set entry, at_mode=%d\n", at_req->at_mode);

    /* cmd mode */
    if (at_req->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FOTACRT=", &count);
        if (!paras_arr || count != 1) {
            FLOEX(0x13a54c, "", "malloc fail for splitting\n");
            return AIC_EPIPE;
        }

        if (in->crt) {
            fota_free(in->crt);
            in->crt = 0;
        }

        in->crt_len = atoi((char *)paras_arr[0]);
        aic_free_paras_mem(paras_arr, count);
        if (!in->crt_len) {
            FLOEX(0x15bb58, "", "fota crt, size is 0!\n");
            return AIC_EINVAL;
        }
        in->crt = fota_malloc(in->crt_len);
        if (in->crt) {
            FLOIX(0x0eec22, "", "fota crt change to data mode\n");
            at_req->at_mode = AT_DATA_MODE;
            atfota.crt_len_recv = 0;
            aic_transmit_at_data(at_req->serial_port_id, STR_FOTACRT_START,
                                 strlen(STR_FOTACRT_START));

            return AIC_SUCCESS;
        } else {
            FLOEX(0x101871, "", "fota crt, no memory!\n");
            return AIC_ENOMEM;
        }
    }

    /* data mode */
    if (at_req->at_mode == AT_DATA_MODE) {
        if (strncmp(at_req->at_upstream_data, "+++", 3) == 0) {
            FLOIX(0x00eac2, "", "fota crt completed, change to cmd mode\n");

            at_req->at_mode = AT_CMD_MODE;
            if (in->crt_len == atfota.crt_len_recv) {
                aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_ok,
                                       strlen(atfota_rsp_ok));
                FLOIX(0x019712, "", "fota crt set success!\n");

                return AIC_SUCCESS;
            } else {
                aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_err,
                                       strlen(atfota_rsp_err));

                FLOIX(0x1dc35f, "", "fota crt set failed!\n");

                return AIC_EAGAIN;
            }
        }
    } else {
        FLOEX(0x0b1685, "", "fota crt wrong at mode\n");
        return AIC_EPERM;
    }

    new_size = at_req->upstream_data_len;
    if (atfota.crt_len_recv + new_size > in->crt_len)
        new_size = in->crt_len - atfota.crt_len_recv;
    if (new_size == 0)
        return 0;
    if (in->crt) {
        memcpy(in->crt + atfota.crt_len_recv, at_req->at_upstream_data,
               new_size);
        atfota.crt_len_recv += new_size;

        FLOIX(0x1619d9, "d", "atfota_crt recv = %d\n", new_size);

        /* all CRT are received, change to CMD mode */
        if (in->crt_len == atfota.crt_len_recv) {
            at_req->at_mode = AT_CMD_MODE;
            aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_ok,
                                    strlen(atfota_rsp_ok));
            FLOIX(0x019712, "", "fota crt set success!\n");

            return AIC_SUCCESS;
        }
    } else {
        in->crt_len = 0;
        atfota.crt_len_recv = 0;
        status = AIC_EPIPE;

        FLOIX(0x194f9e, "", "fota crt lost memory!\n");
    }

    return status;
}

#ifdef CONFIG_FOTA_BY_MCU
/**********************************************************************
* FOTA DATA AT commands usage:
*   at+fotadata=start,package_len
*   start: 1 means start to transfer data, 0 means stop transferring;
*   package_len: the total length of diff patch package;
* Return:
*   "OK": means command execute successfully;
*   "SEND OK": means data has been received;
*   "FINISHED": means data finished receiving,
*               (recv len == package total len)
***********************************************************************/
static u32 atfota_data_exec_cmd(AT_REQ_RESP *at_req)
{
    u32 recv_len = 0;
    u32 offset = sizeof(struct FOTA_RAMDATA_INFO);
    u16 count;
    ulong *paras_arr;
    static u32 ram_addr = 0, ram_size = 0;
    static u32 start = 0;

    FLOIX(0x0a489c, "d", "atfota_data set entry, at_mode=%d\n", at_req->at_mode);

    /* cmd mode */
    if (at_req->at_mode == AT_CMD_MODE) {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req,
                                                 "AT+FOTADATA=", &count);
        if (!paras_arr || count != 2) {
            FLOEX(0x13a54c, "", "malloc fail for splitting\n");
            aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_err,
                                    strlen(atfota_rsp_err));
            return AIC_EPIPE;
        }

        start = atoi((char *)paras_arr[0]);
        if (start == 1) {
            atfota_data.ramdata.len = atoi((char *)paras_arr[1]);
            aic_free_paras_mem(paras_arr, count);

            if (!atfota_data.ramdata.len) {
                FLOEX(0x04cef4, "", "fota data, size is 0!\n");
                aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_err,
                                       strlen(atfota_rsp_err));
                return AIC_SUCCESS;
            }

            atfota_data.ramdata.is_ready = 0;
            atfota_data.recv_len = 0;

            /* power off cp for ram release */
            if(!aic_boot_mode_is_basic())
                sysoff_stop_cp();

            sys_get_save_mcu_fota_pkg_area((addr_t *)&ram_addr, (size_t *)&ram_size);
            FLOIX(0x188bf1, "uu", "fota: ram addr: %u, ram size: %u\n", ram_addr, ram_size);

            FLOIX(0x0b9ec4, "d", "fota: change to data mode, prepare to recv %d bytes data\n",
                  atfota_data.ramdata.len);
            aic_get_wakelock();
            aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_ok,
                                strlen(atfota_rsp_ok));

            at_req->at_mode = AT_DATA_MODE;
            aic_set_data_mode_expire_time("AT+FOTADATA", FOTA_DATA_MODE_TIMEOUT * 1000);
            return AIC_SUCCESS;
        } else {
            start = 0;
            aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_ok,
                                   strlen(atfota_rsp_ok));
            aic_put_wakelock();

            FLOIX(0x1de358, "", "fota: stop trans data, change to cmd mode\n");
            return AIC_SUCCESS;
        }
    }

    /* data mode */
    if (at_req->at_mode == AT_DATA_MODE) {
        if (at_req->end_data_mode) {
            start = 0;
            FLOIX(0x01fefd, "", "fota: data data trans timeout!\n");
            at_req->at_mode = AT_CMD_MODE;
            return AIC_SUCCESS;
        }

        recv_len = at_req->upstream_data_len;

        if (recv_len != 0) {
            /* save data to sram */
            if (!ram_addr || ram_size < atfota_data.recv_len) {
                FLOIX(0x0fa8bb, "uuu", "fota: error occur, switch to cmd mode, ram addr: %u, ramsize:%u, recv len:%u\n",
                    ram_addr, ram_size, atfota_data.recv_len);
                at_req->at_mode = AT_CMD_MODE;
                aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_err,
                                    strlen(atfota_rsp_err));
                return AIC_SUCCESS;
            }

            /* write diff patch to ram */
            memcpy((u8 *)ram_addr + atfota_data.recv_len + offset, at_req->at_upstream_data, recv_len);
            atfota_data.recv_len += recv_len;

            if (atfota_data.recv_len == atfota_data.ramdata.len) {
                FLOIX(0x168be3, "", "fota: data trans finished\n");
                atfota_data.ramdata.addr = ram_addr;
                atfota_data.ramdata.is_ready = FOTA_RAM_DATA_READY;

                /* set the raminfo to ram head */
                memcpy((u8 *)ram_addr, (u8 *)&atfota_data.ramdata, offset);

                /* change at channel to cmd mode */
                start = 0;
                at_req->at_mode = AT_CMD_MODE;
                aic_transmit_at_result(at_req->serial_port_id, (u8 *)atfota_rsp_finish,
                                    strlen(atfota_rsp_finish));
            } else {
                aic_transmit_at_data(at_req->serial_port_id, (u8 *)atfota_rsp_send_ok,
                                    strlen(atfota_rsp_send_ok));
            }
        }
        FLOIX(0x1e7e19, "ddd", "fota: recv len: %d, total len: %d, cur recv len: %d\n", atfota_data.recv_len,
              atfota_data.ramdata.len, recv_len);
    }

    return AIC_SUCCESS;
}

static u32 atfota_data_test_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];
    char rsp_str[100];

    sprintf(rsp_str, "AT+FOTADATA=%d\n", atfota_data.recv_len);
    rsp[0] = (ulong)rsp_str;
    rsp[1] = (ulong)atfota_rsp_ok;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}
#endif

/* +FOTA=<url>,<port>,<user_name>,<pwd>,(0,1),(0,5-100),(0,5-100) */
const static AT_CMD_HANDLE fota_at_handles[] =
{
    {
        "AT+FOTA",
        atfota_test_cmd,
        atfota_query_cmd,
        atfota_set_cmd,
        AIC_NULL
    },
    {
        "AT+FOTACRT",
        atfota_crt_test_cmd,
        AIC_NULL,
        atfota_crt_set_cmd,
        AIC_NULL
    },
#ifdef CONFIG_FOTA_BY_MCU
    {
        "AT+FOTADATA",
        atfota_data_test_cmd,
        AIC_NULL,
        AIC_NULL,
        atfota_data_exec_cmd
    },
#endif
    {
        AIC_NULL
    },
};

int atfota_init(void)
{
    aic_add_at_cmd_handle((AT_CMD_HANDLE *)fota_at_handles);

    return 0;
}

#ifndef CONFIG_FOTA_CMIOT_CMD
int atfota_callback_init(void)
{
    if(AIC_SUCCESS != fota_notify_register(atfota_callback, 0, FOTA_START_BY_AT))
    {
        FASSERT(0);
        return -1;
    }
    return 0;
}
#endif
