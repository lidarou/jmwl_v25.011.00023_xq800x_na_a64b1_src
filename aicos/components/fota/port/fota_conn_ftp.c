/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is FTP type network access for FOTA download.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x048551
#define LOG_MODULE    AMOD_FOTA

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_size.h>

#include <aic_at_cmd_handle.h>
#include <aic_ftp_client.h>
#include <aic_http_comm.h>

#include "fota_util.h"

#define FOTA_NET_WAIT_TIME      5000
#define FOTA_FTP_URL_MAX        128

static u32 g_fota_ftp_handle;
static char g_fota_ftp_url[32];
static char g_fota_ftp_dir[128];
static char g_fota_ftp_name[32];
static long g_fota_file_size;

/* test for the FTP Client get method. */
#ifdef CONFIG_FOTA_CMIOT_CMD
static int fota_ftp_connect(char *url, u32 port, u8 *crt, u32 crt_len,
                            char *user_name, char *password,
                            u32 start, u32 end, u32 timeout, u32 ssl_id)
#else
static int fota_ftp_connect(char *url, u32 port, u8 *crt, u32 crt_len,
                            char *user_name, char *password,
                            u32 start, u32 end)
#endif
{
    u32 handle, status;
    u8 *p, *p2, *p3 = 0;

    if (!url || !user_name || !password) {
        FLOEX(0x01acc6, "", "url or user name or password is null\n");
        return AIC_EPERM;
    }

    if (strlen(url) > FOTA_FTP_URL_MAX) {
        FLOEX(0x10d751, "", "url is too long\n");
        return AIC_EPERM;
    }

    if ((p = strstr(url + 6, "/")) != 0) {
        /* url got */
        strncpy(g_fota_ftp_url, url, p - (u8 *)url);
    } else {
        FLOGE("no file name1\n");
        return AIC_EPERM;
    }

    p2 = p;
    if ((p = strstr(p + 1, "/")) != 0) {
        p3 = p;
        while((p = strstr(p + 1, "/")) != 0)
            p3 = p;
        /* now p3 is the last '\' */
        /* dir got */
        strncpy(g_fota_ftp_dir, p2, p3 - p2);
        strcpy(g_fota_ftp_name, p3 + 1);
    } else {
        strcpy(g_fota_ftp_dir,"/");
        strcpy(g_fota_ftp_name, p2 + 1);
    }

    FLOGE("fota url=%s, dir=%s, name=%s\n", g_fota_ftp_url, g_fota_ftp_dir, g_fota_ftp_name);

    handle = aic_ftp_client_create(FOTA_CLIENT);
    if (!handle) {
        FLOEX(0x0fd8e2, "", "Error in FTP client create\n");
        return AIC_EPIPE;
    }

#ifdef CONFIG_FOTA_CMIOT_CMD
    status = aic_ftp_client_connect(handle, url, user_name, password, port,
                                    timeout);
else
    status = aic_ftp_client_connect(handle, url, user_name, password, port,
                                    FOTA_NET_WAIT_TIME);
#endif

    /* Check status.  */
    if (status != NX_SUCCESS) {
        aic_ftp_client_delete(handle, FOTA_NET_WAIT_TIME);
        FLOEX(0x1da653, "x", "ftp connect error: 0x%x\n", status);
        return AIC_EPIPE;
    }
    {
#ifdef CONFIG_FOTA_CMIOT_CMD
        g_fota_file_size = aic_ftp_client_file_size_get(handle, g_fota_ftp_dir,
                                                        g_fota_ftp_name,
                                                        timeout);
#else
        g_fota_file_size = aic_ftp_client_file_size_get(handle, g_fota_ftp_dir,
                                                        g_fota_ftp_name,
                                                        FOTA_NET_WAIT_TIME);
#endif
        FLOGE("fota file size: %d\n", g_fota_file_size);
    }

#ifdef CONFIG_FOTA_CMIOT_CMD
    status = aic_ftp_client_download_cmd_send(handle, g_fota_ftp_dir, g_fota_ftp_name,
                                              timeout);
#else
    status = aic_ftp_client_download_cmd_send(handle, g_fota_ftp_dir, g_fota_ftp_name,
                                              FOTA_NET_WAIT_TIME);
#endif
    if (status != NX_SUCCESS) {
#ifdef CONFIG_FOTA_CMIOT_CMD
        aic_ftp_client_delete(handle, timeout);
#else
        aic_ftp_client_delete(handle, FOTA_NET_WAIT_TIME);
#endif
        FLOEX(0x182c19, "x", "ftp dl cmd send error: 0x%x\n", status);
        return AIC_EPIPE;
    }

    g_fota_ftp_handle = handle;

    return 0;
}

/**
 *  return:
 *       < 0: error code
 *       >=0: data size
 */
#ifdef CONFIG_FOTA_CMIOT_CMD
static int fota_ftp_get_data(u8 *buff, u32 *bytes, u32 timeout)
#else
static int fota_ftp_get_data(u8 *buff, u32 *bytes)
#endif
{
    LONG lbytes;

#ifdef CONFIG_FOTA_CMIOT_CMD
    lbytes = aic_ftp_client_file_data_get(g_fota_ftp_handle,
                                          buff, *bytes,
                                          timeout);
#else
    lbytes = aic_ftp_client_file_data_get(g_fota_ftp_handle,
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

static u32 fota_ftp_get_total_size(void)
{
    return g_fota_file_size;
#if 0
    long ret;

    ret = aic_ftp_client_file_size_get(g_fota_ftp_handle, g_fota_ftp_dir, g_fota_ftp_name,
                                       FOTA_NET_WAIT_TIME);
    if (ret < 0) {
        FLOEX(0x09dd55, "d", "get file size failed, ret=%d\n", ret);
        return 0;
    }

    return (u32)ret;
#endif
}

int fota_ftp_disconnect(void)
{
    u32 status;

    /* Clear out the HTTP client when we are done. */
#ifdef CONFIG_FOTA_CMIOT_CMD
    status = aic_ftp_client_delete(g_fota_ftp_handle, timeout);
#else
    status = aic_ftp_client_delete(g_fota_ftp_handle, FOTA_NET_WAIT_TIME);
#endif
    if (status != NX_SUCCESS) {
        FLOEX(0x13027e, "x", "error in FTP client delete: 0x%x\n", status);
        return AIC_EACCES;
    }

    return 0;
}

struct FOTA_CONN_OBJ fota_conn_ftp = {
    .type = FOTA_CONN_FTP,
    .connect = fota_ftp_connect,
    .get_data = fota_ftp_get_data,
    .disconnect = fota_ftp_disconnect,
    .get_total_size = fota_ftp_get_total_size,
};

int fota_ftp_init(void)
{
    fota_conn_register(&fota_conn_ftp);

    return 0;
}
DIS_CHARGE_COMP_P1_INIT_EXPORT(fota_ftp_init);


