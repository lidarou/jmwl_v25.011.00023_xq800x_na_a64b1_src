/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is HTTP type network access for FOTA download.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b1472
#define LOG_MODULE    AMOD_FOTA

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_size.h>

#include <aic_at_cmd_handle.h>
#include <aic_http_client.h>
#include <aic_http_comm.h>

#include "fota_util.h"

#define FOTA_NET_WAIT_TIME      5000

static u32 g_fota_http_handle;
static void fota_http_response_cb(char *field_name, u32 field_name_length,
                                  char *field_value, u32 field_value_length)
{
    char name[100];
    char value[100];

    if (field_name_length >= sizeof(name) ||
        field_value_length >= sizeof(value))
        return;

    memset(name, 0, sizeof(name));
    memset(value, 0, sizeof(value));

    memcpy(name, field_name, field_name_length);
    memcpy(value, field_value, field_value_length);

    FLOWX(0x16d49c, "sdsd", "Field name %s (%d bytes) Value %s (%d bytes)\n",
          name, field_name_length, value, field_value_length);
}

/* test for the HTTP Client get method. */
#ifdef CONFIG_FOTA_CMIOT_CMD
static int fota_http_connect(char *url, u32 port, u8 *crt, u32 crt_len,
                             char *user_name, char *password,
                             u32 start, u32 end, u32 timeout, u32 ssl_id)
#else
static int fota_http_connect(char *url, u32 port, u8 *crt, u32 crt_len,
                             char *user_name, char *password,
                             u32 start, u32 end)
#endif
{
    u32 handle, status;

    handle = aic_http_client_create(FOTA_CLIENT);
    if (!handle) {
        FLOEX(0x09ec6f, "", "Error in HTTP client create\n");
        return AIC_EPIPE;
    }

    /* Set the header callback routine. */
    aic_https_client_response_header_callback_set(handle,
                                                  fota_http_response_cb);

    /* if no username, we must give 0, otherwise HTTP will connect failed. */
#ifdef CONFIG_FOTA_CMIOT_CMD
    if (user_name[0] == 0 || password[0] == 0)
        status = aic_http_client_range_get_start(handle, url, 0, 0, port,
                                                 start, end,
                                                 timeout);
    else
        status = aic_http_client_range_get_start(handle, url, user_name,
                                                 password, port, start, end,
                                                 timeout);
#else
    if (user_name[0] == 0 || password[0] == 0)
        status = aic_http_client_range_get_start(handle, url, 0, 0, port,
                                                 start, end,
                                                 FOTA_NET_WAIT_TIME);
    else
        status = aic_http_client_range_get_start(handle, url, user_name,
                                                 password, port, start, end,
                                                 FOTA_NET_WAIT_TIME);
#endif
    /* Check status.  */
    if (status != NX_SUCCESS) {
        aic_http_client_delete(handle);
        FLOEX(0x092f63, "x", "Error in http get start: 0x%x\n", status);
        return AIC_EPERM;
    }

    g_fota_http_handle = handle;

    return 0;
}

/**
 *  return:
 *       < 0: error code
 *       >=0: data size
 */
#ifdef CONFIG_FOTA_CMIOT_CMD
static int fota_http_get_data(u8 *buff, u32 *bytes, u32 timeout)
#else
static int fota_http_get_data(u8 *buff, u32 *bytes)
#endif
{
    LONG lbytes;

#ifdef CONFIG_FOTA_CMIOT_CMD
    lbytes = aic_https_client_response_data_get(g_fota_http_handle,
                                                buff, *bytes,
                                                timeout);
#else
    lbytes = aic_https_client_response_data_get(g_fota_http_handle,
                                                buff, *bytes,
                                                FOTA_NET_WAIT_TIME);
#endif
    if (lbytes < 0) {
        FLOEX(0x084803, "d", "get data failed, ret=%d\n", lbytes);
        return lbytes;
    } else {
        *bytes = lbytes;
        return 0;
    }
}

static u32 fota_http_get_total_size(void)
{
    return aic_https_client_total_receive_bytes_get(g_fota_http_handle);
}

int fota_http_disconnect(void)
{
    u32 status;

    /* Clear out the HTTP client when we are done. */
    status = aic_http_client_delete(g_fota_http_handle);
    if (status != NX_SUCCESS) {
        FLOEX(0x14e50d, "x", "error in HTTP client delete: 0x%x\n", status);
        return AIC_EACCES;
    }

    return 0;
}

struct FOTA_CONN_OBJ fota_conn_http = {
    .type = FOTA_CONN_HTTP,
    .connect = fota_http_connect,
    .get_data = fota_http_get_data,
    .disconnect = fota_http_disconnect,
    .get_total_size = fota_http_get_total_size,
};

int fota_http_init(void)
{
    fota_conn_register(&fota_conn_http);

    return 0;
}
DIS_CHARGE_COMP_P1_INIT_EXPORT(fota_http_init);


