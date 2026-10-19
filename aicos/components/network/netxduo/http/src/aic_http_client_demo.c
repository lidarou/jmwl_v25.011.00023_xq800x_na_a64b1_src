/**************************************************************************/
/*                                                                        */
/*Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  HTTP(S) sample code
 */

/* Enable HTTPS */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x06f93a
#define LOG_MODULE    AMOD_NET

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <nx_api.h>
#include <nx_ip.h>
#include <nx_ipv6.h>
#include <nx_web_http_client.h>
#include <nxd_bsd.h>

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_net_pdp_mgr.h>
#include <aic_http_comm.h>
#include <aic_http_client.h>
#include <aic_https_client.h>

#define HTTP_DEMO_CLIENT_NAME "HTTP Demo Client"
#define HTTP_DEMO_URL "http://47.101.171.33/doc/download/10.txt"
#define HTTPS_DEMO_URL "https://47.101.171.33/doc/download/CK8K.txt"
#define HTTP_DEMO_WAIT_TIME (10 * 1000)
#define HTTP_DEMO_SLEEP_TIME (60 *1000)

static u32 http_demo_handle = 0;
static u32 https_demo_handle = 0;
static aos_thread_id http_demo_thread;
static aos_thread_id https_demo_thread;

static void aic_http_demo()
{
    u32 status = 0;
    long lbytes = 0;
    u8 *buff;
    u32 bytes = 1500;
    u32 total_length = 0;
    u32 length_to_be_copied = 0;
    u32 chunked_mode = 0;

    http_demo_handle = aic_http_client_create(HTTP_DEMO_CLIENT_NAME);
    if (http_demo_handle == 0) {
        return;
    }
    aicx_error(0x1132e2, "", "http demo start");

    /*wait pdp active,only for demo test.*/
    aos_sleep(HTTP_DEMO_SLEEP_TIME);

    status = aic_http_client_get_start(http_demo_handle, HTTP_DEMO_URL, NULL,
                                       NULL, 80, HTTP_DEMO_WAIT_TIME);
    if (status != NX_SUCCESS) {
        aic_http_client_delete(http_demo_handle);
        return;
    }

    buff = aos_zalloc(SIZE_16K);

    do {
        lbytes = aic_https_client_response_data_get(http_demo_handle, buff + total_length, bytes,
                                                    HTTP_DEMO_WAIT_TIME);
        aicx_error(0x18afe9, "d", "http get lbytes len is %d", lbytes);
        if (lbytes <= 0) {
            break;
        } else {
            chunked_mode = aic_http_client_chunked_mode_check(http_demo_handle);
            length_to_be_copied = aic_https_client_total_receive_bytes_get(http_demo_handle);
            if (!chunked_mode && length_to_be_copied) {
                total_length += lbytes;
            }
        }

    } while (total_length < length_to_be_copied || chunked_mode);

    aicx_error(0x1e6e26, "s", "http get data is %s", buff);
    aic_http_client_delete(http_demo_handle);

    return;
}

static void aic_https_demo()
{
    u32 status = 0;
    LONG lbytes = 0;
    u8 *buff;
    u32 bytes = 1500;
    u32 total_length = 0;
    u32 length_to_be_copied = 0;
    u32 chunked_mode = 0;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    https_demo_handle = aic_https_client_create(HTTP_DEMO_CLIENT_NAME);
    if (https_demo_handle == 0) {
        return;
    }

    /*for https test,set auth_type to 0, do not need certifica1*/
    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)https_demo_handle;
    client_ptr->https_client->auth_type = 0;

    /*wait pdp active,only for demo test.*/
    aos_sleep(HTTP_DEMO_SLEEP_TIME);

    status = aic_https_client_get_start(https_demo_handle, HTTPS_DEMO_URL, NULL,
                                       NULL, 443, HTTP_DEMO_WAIT_TIME);
    if (status != NX_SUCCESS) {
        aic_https_client_delete(https_demo_handle);
        return;
    }

    buff = aos_zalloc(SIZE_16K);

    do {
        lbytes = aic_https_client_response_data_get(https_demo_handle, buff + total_length, bytes,
                                                    HTTP_DEMO_WAIT_TIME);
        if (lbytes <= 0) {
            break;
        } else {
            chunked_mode = aic_http_client_chunked_mode_check(https_demo_handle);
            length_to_be_copied = aic_https_client_total_receive_bytes_get(https_demo_handle);
            if (!chunked_mode && length_to_be_copied) {
                total_length += lbytes;
            }
        }

    } while (total_length < length_to_be_copied || chunked_mode);

    aicx_error(0x0f0971, "s", "https get data is %s", buff);

    aic_https_client_delete(https_demo_handle);

    return;
}

int http_demo_init(void)
{

    http_demo_thread = aos_create_thread("http_demo_thread", aic_http_demo,
                                       NULL, SIZE_4K, 0, NULL, 15, AOS_AUTO_START);

    AOS_ASSERT(http_demo_thread != AOS_INVALID_THREAD_ID);

    https_demo_thread = aos_create_thread("https_demo_thread", aic_https_demo,
                                       NULL, SIZE_4K, 0, NULL, 15, AOS_AUTO_START);

    AOS_ASSERT(https_demo_thread != AOS_INVALID_THREAD_ID);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(http_demo_init);

