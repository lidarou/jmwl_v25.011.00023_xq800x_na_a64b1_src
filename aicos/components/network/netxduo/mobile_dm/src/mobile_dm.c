/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Support Network Extended Functions
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x046a56
#define LOG_MODULE    AMOD_NET

#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_version.h>
#include <aic_md_api.h>
#include <aic_size.h>
#include <strings.h>
#include <string.h>
#include <sn.h>
#include <aic_md_id.h>
#include <aic_at_process_common.h>
#include <aic_https_client.h>
#include <aic_http_comm.h>
#include <rtc.h>
#include "mobile_dm.h"
#include "mbedtls/base64.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"

static aos_thread_id thread_mobile_dm_service;
static aos_event_ptr mobile_dm_event;
static u32 mobile_dm_pdp_active_count = 0;
static aos_timer_ptr mobile_dm_keepalive_timer;
static aos_timer_ptr mobile_dm_report_timer;

static u32 mobile_dm_handle;
static MOBILE_DM_RULE_CONFIG rule_config = {1440, 8, 1440, 10, 3};
static MOBILE_DM_ADDRESS_CONFIG address_config;

static MOBILE_DM_DEVICE_INFO device_info[MOBILE_DM_DEVICE_INFO_COUNT];
static MOBILE_DM_ENCRYP_DEVICE_INFO e_device_info[MOBILE_DM_DEVICE_INFO_COUNT];

static u8 *mobile_dm_encryp_device_info_buf; /* save encrypt device info */
static u8 *mobile_dm_config_post_buf; /* used for get config */
static u8 *mobile_dm_post_buf; /* used for register or keepalive */
static u8 *mobile_dm_config_res_buf; /* save response from server */

static bool mobile_dm_cfg_enable = true; /* test switch for config */

static MOBILE_DM_FIELD_CONFIG_MAP mobile_dm_field_config_maps[] =
{
    {"imsi",                1},
    {"mac",                 1},
    {"rom",                 1},
    {"ram",                 1},
    {"cpu",                 1},
    {"sysVersion",          1},
    {"softwareVer",         1},
    {"softwareName",        1},
    {"volte",               1},
    {"netType",             1},
    {"phoneNumber",         1},
    {"routerMac",           1},
    {"bluetoothMac",        1},
    {"sn",                  1},
    {"gpu",                 1},
    {"board",               1},
    {"resolution",          1},
    {"batteryCapacity",     1},
    {"screenSize",          1},
    {"networkStatus",       1},
    {"wearingStatus",       1},
    {"appInfo",             1},
    {"imsi2",               1},
};

static u32 MOBILE_DM_FIELD_CONFIG_MAP_SIZE = sizeof(mobile_dm_field_config_maps) /
                                             sizeof(MOBILE_DM_FIELD_CONFIG_MAP);

static MOBILE_DM_CFG mobile_dm_cfg = {0};

static int mobile_dm_configuration_get(u8 *url);
static int mobile_dm_device_register(u8 *url);
static int mobile_dm_heart_beat_send(u8 *url);

static void mobile_dm_config_res_mem_alloc(void)
{
    if (!mobile_dm_config_res_buf)
        mobile_dm_config_res_buf = aos_zalloc(MOBILE_DM_MAX_RESP_BUF_SIZE);

    return;
}

static void mobile_dm_config_res_mem_release(void)
{
    if (mobile_dm_config_res_buf) {
        aos_free(mobile_dm_config_res_buf);
        mobile_dm_config_res_buf = AIC_NULL;
    }

    return;
}

static void mobile_dm_encryp_device_info_mem_alloc(void)
{
    if (!mobile_dm_encryp_device_info_buf)
        mobile_dm_encryp_device_info_buf =
            aos_zalloc(MOBILE_DM_MAX_DEVICE_INFO_SIZE);

    return;
}

static void mobile_dm_encryp_device_info_mem_release(void)
{
    if (mobile_dm_encryp_device_info_buf) {
        aos_free(mobile_dm_encryp_device_info_buf);
        mobile_dm_encryp_device_info_buf = AIC_NULL;
    }

    return;
}

static void mobile_dm_config_post_mem_alloc(void)
{
    if (!mobile_dm_config_post_buf)
        mobile_dm_config_post_buf = aos_zalloc(MOBILE_DM_MAX_CONFIG_POST_SIZE);
    else
        aos_memset(mobile_dm_config_post_buf, 0, MOBILE_DM_MAX_CONFIG_POST_SIZE);

    return;
}

static void mobile_dm_config_post_mem_release(void)
{
    if (mobile_dm_config_post_buf) {
        aos_free(mobile_dm_config_post_buf);
        mobile_dm_config_post_buf = AIC_NULL;
    }

    return;
}

static void mobile_dm_post_mem_alloc(void)
{
    if (!mobile_dm_post_buf)
        mobile_dm_post_buf = aos_zalloc(MOBILE_DM_MAX_POST_SIZE);
    else
        aos_memset(mobile_dm_post_buf, 0, MOBILE_DM_MAX_POST_SIZE);

    return;
}

static void mobile_dm_post_mem_release(void)
{
    if (mobile_dm_post_buf) {
        aos_free(mobile_dm_post_buf);
        mobile_dm_post_buf = AIC_NULL;
    }

    return;
}

static void mobile_dm_encryp_mem_alloc(void)
{
    u32 i;

    for (i = 0; i < MOBILE_DM_DEVICE_INFO_COUNT; i++) {
        if (!e_device_info[i].encryp_data) {
            e_device_info[i].encryp_data = aos_zalloc(MOBILE_DM_ENCRYP_DATA_LEN);
            memset(e_device_info[i].encryp_data, '\0', MOBILE_DM_ENCRYP_DATA_LEN);
        }
    }

    return;
}

static void mobile_dm_encryp_mem_release(void)
{
    u32 i;

    for (i = 0; i < MOBILE_DM_DEVICE_INFO_COUNT; i++) {
        if (e_device_info[i].encryp_data) {
            aos_free(e_device_info[i].encryp_data);
            e_device_info[i].encryp_data = AIC_NULL;
        }
    }

    return;
}

static void mobile_dm_device_info_mem_alloc(void)
{
    u32 i;

    for (i = 0; i < MOBILE_DM_DEVICE_INFO_COUNT; i++) {
        if (!device_info[i].info)
            device_info[i].info = aos_zalloc(MOBILE_DM_DEVICE_INFO_LEN);
    }

    return;
}

static void mobile_dm_device_info_mem_release(void)
{
    u32 i;

    for (i = 0; i < MOBILE_DM_DEVICE_INFO_COUNT; i++) {
        if (device_info[i].info) {
            aos_free(device_info[i].info);
            device_info[i].info = AIC_NULL;
        }
    }

    return;
}

static int mobie_dm_imei_get(char *p_sim_imei)
{
    AT_RESP_FROM_CP at_cimi_resp = {0};
    ulong              *line_arr = NULL;

    /* check param */
    if (NULL == p_sim_imei)
        return -1;

    /* malloc */
    at_cimi_resp.resp_buf = aos_zalloc(64);
    if (NULL == at_cimi_resp.resp_buf)
        return -1;

    at_cimi_resp.buf_len = 64;
    /* send AT+CIMI command,respnse: */
    aic_send_cmd_and_wait("AT+GSN=1", &at_cimi_resp);

    /* judge network is error */
    if(strstr(at_cimi_resp.resp_buf, "+CME ERROR")) {
        aicx_error(0x0eb5f1, "", "mobile dm get imei fail!\r\n");
        if(NULL != at_cimi_resp.resp_buf) {
            aos_free(at_cimi_resp.resp_buf);
            at_cimi_resp.resp_buf = NULL;
        }
        return -1;
    }
    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(at_cimi_resp.resp_buf,
                                           at_cimi_resp.resp_line_cnt);

    /*imsi max length is 15, offset is length of(+GSN: ) 6 bytes*/
    strncpy(p_sim_imei, (char *)line_arr[0] + 6, strlen((char *)line_arr[0]));
    aicx_info(0x0db31f, "s", "mobile dm imei = %s.\r\n", p_sim_imei);

    /* free params */
    aic_free_paras_mem(line_arr, at_cimi_resp.resp_line_cnt);
    if(NULL != at_cimi_resp.resp_buf) {
        aos_free(at_cimi_resp.resp_buf);
        at_cimi_resp.resp_buf = NULL;
    }

    return 0;
}

static int mobie_dm_imsi_get(char *p_sim_imsi)
{
    AT_RESP_FROM_CP at_cimi_resp = {0};
    ulong              *line_arr = NULL;

    /* check param */
    if (NULL == p_sim_imsi)
        return -1;

    /* malloc */
    at_cimi_resp.resp_buf = aos_zalloc(64);
    if (NULL == at_cimi_resp.resp_buf)
        return -1;

    at_cimi_resp.buf_len = 64;
    /* send AT+CIMI command,respnse: */
    aic_send_cmd_and_wait("at+cimi", &at_cimi_resp);

    /* judge network is error */
    if(strstr(at_cimi_resp.resp_buf, "+CME ERROR")) {
        aicx_error(0x154ac2, "", "mobile dm get imsi fail!\r\n");
        if(NULL != at_cimi_resp.resp_buf) {
            aos_free(at_cimi_resp.resp_buf);
            at_cimi_resp.resp_buf = NULL;
        }
        return -1;
    }
    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(at_cimi_resp.resp_buf,
                                           at_cimi_resp.resp_line_cnt);

    /*imsi max length is 15 */
    strncpy(p_sim_imsi, (char *)line_arr[0], strlen((char *)line_arr[0]));
    aicx_info(0x043f2d, "s", "mobile dm imsi = %s.\r\n", p_sim_imsi);

    /* free params */
    aic_free_paras_mem(line_arr, at_cimi_resp.resp_line_cnt);
    if(NULL != at_cimi_resp.resp_buf) {
        aos_free(at_cimi_resp.resp_buf);
        at_cimi_resp.resp_buf = NULL;
    }

    return 0;
}

static int mobie_dm_phone_num_get(char *phonenum)
{
    AT_RESP_FROM_CP phonenum_resp = {0};
    ulong           *line_arr = NULL;
    u8 *found;

    /* check param */
    if (NULL == phonenum)
        return -1;

    /* malloc */
    phonenum_resp.resp_buf = aos_zalloc(64);
    if (NULL == phonenum_resp.resp_buf)
        return -1;

    phonenum_resp.buf_len = 64;
    /* send AT+CNUM command,respnse: */
    aic_send_cmd_and_wait("at+cnum", &phonenum_resp);

    /* judge network is error */
    if(strstr(phonenum_resp.resp_buf, "+CME ERROR")) {
        aicx_error(0x16c6e8, "", "mobie dm get phone num fail!\r\n");
        if(NULL != phonenum_resp.resp_buf) {
            aos_free(phonenum_resp.resp_buf);
            phonenum_resp.resp_buf = NULL;
        }
        return -1;
    }
    /* resp split string as line */
    line_arr = aic_get_each_line_from_resp(phonenum_resp.resp_buf,
                                           phonenum_resp.resp_line_cnt);

    /*imsi max length is 14 */
    found = strchr((char *)line_arr[0], ',');
    if (found) {
        found+= 2;
        if (found[0] == '+')
            strncpy(phonenum, found, 14);
        else
            strncpy(phonenum, found, 11);
    }

    aicx_info(0x0c9c48, "s", "mobile dm phone num = %s.\r\n", phonenum);

    /* free params */
    aic_free_paras_mem(line_arr, phonenum_resp.resp_line_cnt);
    if(NULL != phonenum_resp.resp_buf) {
        aos_free(phonenum_resp.resp_buf);
        phonenum_resp.resp_buf = NULL;
    }

    return 0;
}

static int mobile_dm_sha256_encryp(const u8 *src_msg, u8 *dst_msg, u32 *dst_len)
{

    int ret = AIC_SUCCESS;
    const mbedtls_md_info_t *info;
    mbedtls_md_context_t ctx;

    mbedtls_md_init(&ctx);
    info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

    ret = mbedtls_md_setup(&ctx, info, 0);
    if (ret) {
        aicx_error(0x1d1e33, "x", "mobile dm sha256 setup fail(0x%x)\n", ret);
        goto EXIT;
    }

    ret = mbedtls_md_starts(&ctx);
    if (ret) {
        aicx_error(0x1c0a97, "x", "mobile dm sha256 start fail(0x%x)\n", ret);
        goto EXIT;
    }

    ret = mbedtls_md_update(&ctx, src_msg, strlen(src_msg));
    if (ret) {
        aicx_error(0x1a436c, "x", "mobile dm sha256 updata fail(0x%x)\n", ret);
        goto EXIT;
    }

    ret = mbedtls_md_finish(&ctx, dst_msg);
    if (ret) {
        aicx_error(0x14916c, "x", "mobile dm sha256 finsh fail(0x%x)\n", ret);
        goto EXIT;
    }

    *dst_len = mbedtls_md_get_size(info);

EXIT:
    mbedtls_md_free(&ctx);
    return(ret);
}

static int mobile_dm_aes_cbc_encryp(const u8 *key, const u8 *input,
                                    u32 input_len, u8 *output, u32 *output_len)
{
    u32 i;
    int status = 0;
    u32 blk = (input_len + 15) >> 4;
    u8 pad = 16 - input_len % 16;
    mbedtls_aes_context aes_ctx;
    char iv[MOBILE_DM_AES_IV_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00};

    mbedtls_aes_init(&aes_ctx);
    status = mbedtls_aes_setkey_enc(&aes_ctx, key, MOBILE_DM_AES_KEY_BITS);
    if (status) {
        aicx_error(0x1952e4, "x", "mobile dm aes key set fail(0x%x)\n", status);
        return status;
    }

    memcpy(output, input, input_len);
    for (i = input_len; i < input_len + pad; i++)
        output[i] = pad;

    *output_len = 0;
    for (i = 0; i < blk; ++i) {
        status = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, 16, iv,
                                       output + (i * 16), output + (i * 16));
        if (status < 0) {
            aicx_error(0x0dfbf8, "x", "mobile dm aes crypt fail(0x%x)\n", status);
            return status;
        }

        *output_len += 16;
    }

    mbedtls_aes_free(&aes_ctx);

    return AIC_SUCCESS;
}

int mobile_dm_aes_cbc_decryp(const u8 * key, const u8 *input, u32 input_len,
                             u8 *output)
{
    u32 i;
    int status = 0;
    u32 blk = (input_len + 15) >> 4;
    mbedtls_aes_context aes_ctx;
    char iv[MOBILE_DM_AES_IV_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x00, 0x00};

    mbedtls_aes_init(&aes_ctx);
    status = mbedtls_aes_setkey_dec(&aes_ctx, key, MOBILE_DM_AES_KEY_BITS);
    if (status) {
       aicx_error(0x1952e4, "x", "mobile dm aes key set fail(0x%x)\n", status);
       return status;
    }

    for (i = 0; i < blk; ++i) {
        status = mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, 16, iv,
                                       input + (i * 16), output + (i * 16));
        if (status) {
            aicx_error(0x0ab3eb, "x", "mobile dm aes_crypt_cbc fail(0x%x)\n", status);
            return status;
        }
    }

    return AIC_SUCCESS;
}

static u32 mobile_dm_post_request_send(u32 handle, u8 *url, u8 *username,
                                       u8 *password, u32 server_port,
                                       ulong total_bytes, ulong wait_option)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    status = aic_http_action_prepare(handle, url, username, password,
                                     server_port, total_bytes, -1, -1,
                                     AIC_NULL, wait_option);
    if (status)
        return status;

    client_ptr->https_client->nx_web_http_client_connect_port =
                                        client_ptr->login_info.server_port;

    status = nx_web_http_client_secure_connect(client_ptr->https_client,
             &(client_ptr->login_info.server_ip),
             client_ptr->login_info.server_port, https_tls_setup, wait_option);

    if (status != NX_SUCCESS)
        return status;

    /* Initialize HTTP request. */
    status = nx_web_http_client_request_initialize(client_ptr->https_client,
             NX_WEB_HTTP_METHOD_POST, client_ptr->login_info.resource,
             client_ptr->login_info.host, total_bytes, NX_FALSE, username,
             password, wait_option);
    if (status != NX_SUCCESS)
        return status;

    /* Add approproate headers from input data. */
    status = aic_http_client_specific_header_add(client_ptr->https_client,
             MOBILE_DM_HTTPS_HEADER_KEY_1, MOBILE_DM_HTTPS_HEADER_VALUE_1,
             HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x156c8c, "x", "mobile dm http header add fail(0x%x)\n", status);
        return status;
    }

    status = nx_web_http_client_request_send(client_ptr->https_client,
                                             wait_option);
    if (status != NX_SUCCESS)
        return status;

    client_ptr->https_client->nx_web_http_client_state =
    NX_WEB_HTTP_CLIENT_STATE_POST;

    return status;
}

static void mobile_dm_default_device_info_set(void)
{
    int status;
    u8 buf[64] = {'\0'};
    MOBILE_DM_FILED_CFG *field_cfg = &mobile_dm_cfg.mobile_dm_filed_cfg;

    mobile_dm_device_info_mem_alloc();
    mobie_dm_imsi_get(device_info[imsi].info);

    if (mobile_dm_field_config_maps[mac].mobile_md_config_status)
        aos_memcpy(device_info[mac].info, field_cfg->mac, strlen(field_cfg->mac));
    if (mobile_dm_field_config_maps[rom].mobile_md_config_status)
        aos_memcpy(device_info[rom].info, field_cfg->rom, strlen(field_cfg->rom));
    if (mobile_dm_field_config_maps[ram].mobile_md_config_status)
        aos_memcpy(device_info[ram].info, field_cfg->ram, strlen(field_cfg->ram));
    if (mobile_dm_field_config_maps[cpu].mobile_md_config_status)
        aos_memcpy(device_info[cpu].info, field_cfg->cpu, strlen(field_cfg->cpu));
    if (mobile_dm_field_config_maps[sys_ver].mobile_md_config_status)
        aos_memcpy(device_info[sys_ver].info, field_cfg->sys_ver,
                   strlen(field_cfg->sys_ver));
    if (mobile_dm_field_config_maps[soft_ware_ver].mobile_md_config_status)
        aos_memcpy(device_info[soft_ware_ver].info, field_cfg->sw_ver,
                   strlen(field_cfg->sw_ver));
    if (mobile_dm_field_config_maps[soft_ware_name].mobile_md_config_status)
        aos_memcpy(device_info[soft_ware_name].info, field_cfg->sw_name,
                   strlen(field_cfg->sw_name));

    if (mobile_dm_field_config_maps[volte].mobile_md_config_status) {
        sprintf(buf, "%d", field_cfg->volte);
        aicx_info(0x097cbd, "s", "mobile dm: volte status:%s\n", buf);
        aos_memcpy(device_info[volte].info, buf, strlen(buf));
    }
    if (mobile_dm_field_config_maps[net_type].mobile_md_config_status)
        aos_memcpy(device_info[net_type].info, field_cfg->net_type,
                   strlen(field_cfg->net_type));
    if (mobile_dm_field_config_maps[phone_num].mobile_md_config_status)
        mobie_dm_phone_num_get(device_info[phone_num].info);
    if (mobile_dm_field_config_maps[router_mac].mobile_md_config_status)
        aos_memcpy(device_info[router_mac].info, field_cfg->router_mac,
                   strlen(field_cfg->router_mac));
    if (mobile_dm_field_config_maps[bluetooth_mac].mobile_md_config_status)
        aos_memcpy(device_info[bluetooth_mac].info, field_cfg->bt_mac,
                   strlen(field_cfg->bt_mac));

    if (mobile_dm_field_config_maps[sn].mobile_md_config_status) {
        aos_memset(buf, '\0', 64);
        status = aic_md_read(MD_ID_SN0, sizeof(SN_T), buf);
        if (status) {
            aicx_error(0x06e796, "d", "mobile dm sn get fail(%d)\n", status);
            sprintf(buf, "%s", MOBILE_DM_INFO_NOT_SET);
        }
        aos_memcpy(device_info[sn].info, buf, strlen(buf));
    }
    if (mobile_dm_field_config_maps[gpu].mobile_md_config_status)
        aos_memcpy(device_info[gpu].info, field_cfg->gpu, strlen(field_cfg->gpu));
    if (mobile_dm_field_config_maps[board].mobile_md_config_status)
        aos_memcpy(device_info[board].info, field_cfg->board,
                   strlen(field_cfg->board));
    if (mobile_dm_field_config_maps[resolution].mobile_md_config_status)
        aos_memcpy(device_info[resolution].info, field_cfg->resolution,
                   strlen(field_cfg->resolution));
    if (mobile_dm_field_config_maps[battery_capacity].mobile_md_config_status) {
        aos_memset(buf, '\0', 64);
        sprintf(buf, "%dmah", field_cfg->battery_capacity);
        aos_memcpy(device_info[battery_capacity].info, buf, strlen(buf));
    }
    if (mobile_dm_field_config_maps[screen_size].mobile_md_config_status)
        aos_memcpy(device_info[screen_size].info, field_cfg->screen_size,
                   strlen(field_cfg->screen_size));
    if (mobile_dm_field_config_maps[network_status].mobile_md_config_status) {
        aos_memset(buf, '\0', 64);
        sprintf(buf, "%d", field_cfg->network_status);
        aos_memcpy(device_info[network_status].info, buf, strlen(buf));
    }
    if (mobile_dm_field_config_maps[wearing_status].mobile_md_config_status) {
        aos_memset(buf, '\0', 64);
        sprintf(buf, "%d", field_cfg->wearing_status);
        aos_memcpy(device_info[wearing_status].info, buf, strlen(buf));
    }
    if (mobile_dm_field_config_maps[app_info].mobile_md_config_status)
        aos_memcpy(device_info[app_info].info, field_cfg->app_info,
                   strlen(field_cfg->app_info));
    if (mobile_dm_field_config_maps[imsi2].mobile_md_config_status)
        aos_memcpy(device_info[imsi2].info, MOBILE_DM_INFO_NOT_SET,
                   MOBILE_DM_DEVICE_INFO_LEN);

    return;
}

static void mobile_dm_rule_config_get(u8 *conf_data)
{
    u8 *found;
    u8 *temp_buf;
    u8 temp_data[4] = {'\0'};

    temp_buf = aos_zalloc(128);

    found = strstr(conf_data, "reportTime");
    if (found) {
        aos_memcpy(temp_buf, found, 128);
        sscanf(temp_buf, "%*[^:]:%[^,]", temp_data);
        rule_config.report_time = atoi(temp_data);
    }

    found = strstr(conf_data, "reportNum");
    if (found) {
        aos_memset(temp_buf, 0, 128);
        aos_memcpy(temp_buf, found, 128);
        sscanf(temp_buf, "%*[^:]:%[^,]", temp_data);
        rule_config.report_num = atoi(temp_data);
    }

    found = strstr(conf_data, "heartBeatTime");
    if (found) {
        aos_memset(temp_buf, 0, 128);
        aos_memcpy(temp_buf, found, 128);
        sscanf(temp_buf, "%*[^:]:%[^,]", temp_data);
        rule_config.heart_beat_time = atoi(temp_data);
    }

    found = strstr(conf_data, "retryInterval");
    if (found) {
        aos_memset(temp_buf, 0, 128);
        aos_memcpy(temp_buf, found, 128);
        sscanf(temp_buf, "%*[^:]:%[^,]", temp_data);
        rule_config.retry_interval = atoi(temp_data);
    }

    found = strstr(conf_data, "retryNum");
    if (found) {
        aos_memset(temp_buf, 0, 128);
        aos_memcpy(temp_buf, found, 128);
        sscanf(temp_buf, "%*[^:]:%[^}]", temp_data);
        rule_config.retry_num = atoi(temp_data);
    }

    aicx_info(0x18b93c, "dd", "mobile dm report time/num:%d/%d\n",
              rule_config.report_time, rule_config.report_num);

    aicx_info(0x171056, "ddd", "mobile dm heartbeat time:%d, retry interval:%d, retry num:%d\n",
              rule_config.heart_beat_time, rule_config.retry_interval,
              rule_config.retry_num);

    aos_free(temp_buf);
    return;
}

static void mobile_dm_address_config_get(u8 *conf_data)
{
    u8 *found;

    found = strstr(conf_data, "https");
    if (found) {
        if (!address_config.address_cfg)
            address_config.address_cfg = aos_zalloc(256);

        aos_memcpy(address_config.address_cfg, found, 256);
        aicx_info(0x043d19, "s", "mobile dm address config.address_cfg is %s\n",
                  address_config.address_cfg);
    } else {
        aicx_info(0x0e246c, "", "mobile dm address config not found\n");
    }

    return;
}

static void mobile_dm_register_device_info_check(u8 *conf_data)
{
    u8 *found;
    u8 flag[5] = {'\0'};
    u32 i;

    for (i = 0; i < MOBILE_DM_FIELD_CONFIG_MAP_SIZE; i++) {
        found = strstr(conf_data, mobile_dm_field_config_maps[i].mobile_md_config_str);
        if (found) {
            sscanf(found, "%*[^:]:%[^,]", flag);
            if (strchr(flag, 'N'))
                mobile_dm_field_config_maps[i].mobile_md_config_status = 0;
        }
    }

    return;
}

/* process the configuration info get from server and ingect device info */
static int mobile_dm_device_info_inject(u8 *conf_data)
{
    MOBILE_DM_ENCRYPT_INFO *key_info = &mobile_dm_cfg.mobile_dm_encrypt_info;
    int ret;
    u8 *aes_cbc_out = 0;
    u8 *sha256_dm_key = 0;
    u32 e_out_len = 0;
    u32 aes_cbc_out_len = 0;
    u32 sha256_out_len = 0;
    u32 i;

    if (MOBILE_DM_FIELD_CONFIG_MAP_SIZE != MOBILE_DM_DEVICE_INFO_COUNT) {
        aicx_error(0x00767c, "", "mobile dm device info count error\n");
        return MOBILE_DM_DEVICE_INFO_PROCESS_FAIL;
    }

    mobile_dm_register_device_info_check(conf_data);

    for (i = 0; i < MOBILE_DM_FIELD_CONFIG_MAP_SIZE; i++)
        aicx_info(0x1bb772, "sd", "mobile dm config_status[%s] is %d\n",
                  mobile_dm_field_config_maps[i].mobile_md_config_str,
                  mobile_dm_field_config_maps[i].mobile_md_config_status);

    mobile_dm_default_device_info_set();
    mobile_dm_encryp_mem_alloc();
    aes_cbc_out = aos_zalloc(64);
    sha256_dm_key = aos_zalloc(32);

    aos_memset(sha256_dm_key, 0, 32);
    ret = mobile_dm_sha256_encryp(key_info->key, sha256_dm_key, &sha256_out_len);
    if (ret) {
        aicx_error(0x188b9c, "d", "mobile dm key sha256 encryp fail(%d)\n", ret);
        return MOBILE_DM_DEVICE_INFO_PROCESS_FAIL;
    }

    for (i = 0; i < MOBILE_DM_FIELD_CONFIG_MAP_SIZE; i++) {
        ret = mobile_dm_aes_cbc_encryp(sha256_dm_key, device_info[i].info,
                                       strlen(device_info[i].info),
                                       aes_cbc_out, &aes_cbc_out_len);
        if (ret) {
            aicx_error(0x1222fc, "d", "mobile dm cbc encryp fail(%d)\n", ret);
            goto ERROR_OUT;
        }

        ret = mbedtls_base64_encode(e_device_info[i].encryp_data,
                                    MOBILE_DM_ENCRYP_DATA_LEN, &e_out_len,
                                    aes_cbc_out, aes_cbc_out_len);
        if (ret) {
            aicx_error(0x1da223, "d", "mobile dm base64 encode fail(%d)\n", ret);
            goto ERROR_OUT;
        }

        aicx_info(0x0d029e, "ssdd", "mobile dm: [%s]: %s, len: %d, %d\n",
                  mobile_dm_field_config_maps[i].mobile_md_config_str,
                  e_device_info[i].encryp_data, aes_cbc_out_len, e_out_len);
    }

    mobile_dm_encryp_device_info_mem_alloc();

    snprintf(mobile_dm_encryp_device_info_buf, MOBILE_DM_MAX_DEVICE_INFO_SIZE,
             MOBILE_DM_FMT, e_device_info[imsi].encryp_data,
             e_device_info[mac].encryp_data,
             e_device_info[rom].encryp_data, e_device_info[ram].encryp_data,
             e_device_info[cpu].encryp_data, e_device_info[sys_ver].encryp_data,
             e_device_info[soft_ware_ver].encryp_data,
             e_device_info[soft_ware_name].encryp_data,
             e_device_info[volte].encryp_data,
             e_device_info[net_type].encryp_data,
             e_device_info[phone_num].encryp_data,
             e_device_info[router_mac].encryp_data,
             e_device_info[bluetooth_mac].encryp_data,
             e_device_info[sn].encryp_data,
             e_device_info[gpu].encryp_data,
             e_device_info[board].encryp_data,
             e_device_info[resolution].encryp_data,
             e_device_info[battery_capacity].encryp_data,
             e_device_info[screen_size].encryp_data,
             e_device_info[network_status].encryp_data,
             e_device_info[wearing_status].encryp_data,
             e_device_info[app_info].encryp_data,
             e_device_info[imsi2].encryp_data);

    aicx_info(0x193599, "d", "mobile dm: encrypt data len: %d\n", strlen(mobile_dm_encryp_device_info_buf));
    return AIC_SUCCESS;

ERROR_OUT:
    mobile_dm_device_info_mem_release();
    mobile_dm_encryp_mem_release();
    mobile_dm_encryp_device_info_mem_release();
    aos_free(aes_cbc_out);
    aos_free(sha256_dm_key);
    return MOBILE_DM_DEVICE_INFO_PROCESS_FAIL;
}

static void mobile_dm_config_post_data_inject(u8 *config_type)
{
    MOBILE_DM_FILED_CFG *field_cfg = &mobile_dm_cfg.mobile_dm_filed_cfg;
    u8 imei[64];

    mobie_dm_imei_get(imei);

    mobile_dm_config_post_mem_alloc();

    snprintf(mobile_dm_config_post_buf, MOBILE_DM_MAX_CONFIG_POST_SIZE,
             MOBILE_DM_ENDPOINT_FMT,field_cfg->brand, field_cfg->model,
             field_cfg->app_key, MOBILE_DM_SDK_VERSION, field_cfg->api_ver,
             MOBILE_DM_API_TYPE, MOBILE_DM_TEMPLATE_ID, imei, MOBILE_DM_IMEI2,
             MOBILE_DM_CONFIG_TYPE, MOBILE_DM_PREFIELD1, MOBILE_DM_PREFIELD2,
             config_type);

    aicx_info(0x00c856, "s", "mobile dm endpoint is %s\n", mobile_dm_config_post_buf);

    return;
}

static void mobile_dm_post_data_inject(u8 *conf_data, u8 *config_type)
{
    MOBILE_DM_FILED_CFG *field_cfg = &mobile_dm_cfg.mobile_dm_filed_cfg;
    u8 imei[64];

    mobie_dm_imei_get(imei);

    mobile_dm_post_mem_alloc();
    mobile_dm_device_info_inject(conf_data);

    snprintf(mobile_dm_post_buf, MOBILE_DM_MAX_POST_SIZE,
             MOBILE_DM_REGISTER_DATA_FMT, field_cfg->brand, field_cfg->model,
             field_cfg->app_key, MOBILE_DM_SDK_VERSION, field_cfg->api_ver,
             MOBILE_DM_API_TYPE, MOBILE_DM_TEMPLATE_ID, imei, MOBILE_DM_IMEI2,
             MOBILE_DM_CONFIG_TYPE, MOBILE_DM_PREFIELD1, MOBILE_DM_PREFIELD2,
             mobile_dm_encryp_device_info_buf);

    aicx_info(0x18543a, "d", "mobile dm post_buf is %d\n", strlen(mobile_dm_post_buf));

    return;
}

static void mobile_dm_keepalive_event_set(void *input)
{
    aos_set_event(mobile_dm_event, MOBILE_DM_KEEP_ALIVE_EVENT, AOS_OR);
    aos_change_timer(mobile_dm_keepalive_timer,
                     (rule_config.heart_beat_time * 60 *1000), AOS_NO_PERIOD);

    return;
}

static void mobile_dm_timer_stop(void)
{
    if (aos_is_timer_created(mobile_dm_keepalive_timer)) {
        aos_deactivate_timer(mobile_dm_keepalive_timer);
        aos_delete_timer(mobile_dm_keepalive_timer);
        mobile_dm_keepalive_timer = AIC_NULL;
    } else {
        aicx_info(0x03d797, "", "mobile dm timer has been stopped \n");
    }

    return;
}

static void mobile_dm_timer_start(void)
{
    if (!aos_is_timer_created(mobile_dm_keepalive_timer)) {
        mobile_dm_keepalive_timer = aos_create_timer("mobile dm keepalive timer",
                                    mobile_dm_keepalive_event_set, AIC_NULL,
                                    (rule_config.heart_beat_time * 60 * 1000),
                                    AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);

        AIC_ASSERT(mobile_dm_keepalive_timer);
    } else {
        aos_change_timer(mobile_dm_keepalive_timer,
                         (rule_config.heart_beat_time * 60 *1000), AOS_NO_PERIOD);
    }

    return;
}

static void mobile_dm_report_timeout_event_set(void *input)
{
    aos_set_event(mobile_dm_event, MOBILE_DM_REPORT_TIMEOUT_EVENT, AOS_OR);
    aos_change_timer(mobile_dm_report_timer,
                     (rule_config.heart_beat_time * 60 *1000), AOS_NO_PERIOD);

    return;
}

static void mobile_dm_report_timer_start(u32 interval)
{
    if (!aos_is_timer_created(mobile_dm_report_timer)) {
        mobile_dm_report_timer = aos_create_timer("mobile dm report timer",
                                    mobile_dm_report_timeout_event_set, AIC_NULL,
                                    interval * 1000, AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);

        AIC_ASSERT(mobile_dm_report_timer);
    }

    return;
}

static void mobile_dm_report_timer_stop(void)
{
    if (aos_is_timer_created(mobile_dm_report_timer)) {
        aos_deactivate_timer(mobile_dm_report_timer);
        aos_delete_timer(mobile_dm_report_timer);
        mobile_dm_report_timer = AIC_NULL;
    } else {
        aicx_info(0x128437, "", "mobile dm report timer has been stopped \n");
    }

    return;
}

static u32 mobile_dm_temp_release(void)
{
    u32 status;

    mobile_dm_config_res_mem_release();
    mobile_dm_post_mem_release();
    mobile_dm_encryp_device_info_mem_release();
    mobile_dm_device_info_mem_release();
    mobile_dm_encryp_mem_release();
    mobile_dm_config_post_mem_release();

    if (mobile_dm_handle == 0) {
        aicx_error(0x1db23a, "", "mobile no dm https client\n");
        return MOBILE_NO_DM_HTTPS_CLIENT;
    }

    status = aic_https_client_delete(mobile_dm_handle);
    if (status) {
        aicx_error(0x090d4a, "x", "mobile dm https client delete fail(0x%x)\n", status);
        return MOBILE_DM_HTTPS_CLIENT_DELETE_FAIL;
    }

    mobile_dm_handle = 0;

    return AIC_SUCCESS;
}

static void mobile_dm_service_entry(void *thread_input)
{
    MOBILE_DM_REPORT_INFO *report_info = &mobile_dm_cfg.mobile_dm_report_info;
    MOBILE_DM_HTTP_INFO *http_info = &mobile_dm_cfg.mobile_dm_http_info;
    struct rtc_time tm;
    u64 cur_time, past_time;
    u32 events, last_events = 0;
    u32 status;
    int ret;
    char url[MOBIIE_DM_HTTPS_URL_LEN] = {0};

    /* read config file */
    status = aic_md_read(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG), &mobile_dm_cfg);
    if (status)
        aicx_error(0x016db2, "d", "mobile dm cfg read fail(%d)\n", status);

    while(1) {
        status = aos_get_event(mobile_dm_event, MOBILE_DM_ANY_EVENT,
                               AOS_OR_CLEAR, &events, AOS_WAIT_FOREVER);

        if (status != AOS_SUCCESS)
            continue;

        if (events & MOBILE_DM_PDP_ACTIVE_EVENT ||
            events & MOBILE_DM_REPORT_TIMEOUT_EVENT ||
            events & MOBILE_DM_KEEP_ALIVE_EVENT) {
            aicx_info(0x143e44, "", "mobile dm report start\n");

            if (events & MOBILE_DM_PDP_ACTIVE_EVENT) {
                if (mobile_dm_pdp_active_count != 1) {
                    aicx_error(0x01053d, "", "mobile dm pdp is not active first time\n");
                    continue;
                }
                /* wait 10s */
                aos_sleep(10000);
                /* clear error retry number */
                report_info->retry_num = 0;
            }

report_retry:
            /* check dynamic report number if larger than reportNum from config file */
            if (report_info->report_num >= report_info->report_num_max) {
                /* check current time */
                aos_get_systime((aos_systime *)&tm);
                cur_time = rtc_tm_to_time64(&tm);
                past_time = cur_time - report_info->report_start_time;
                aicx_info(0x091e5f, "lldlldlld", "mobile dm: cur_time: %lld, report time: %lld, past_time:%lld\n",
                          cur_time, report_info->report_start_time, past_time);
                if (past_time >= (report_info->report_time * 60)) {
                    /* clear report number & retry number*/
                    report_info->report_num = 0;
                    report_info->retry_num = 0;
                    mobile_dm_report_timer_stop();
                    if (events & MOBILE_DM_REPORT_TIMEOUT_EVENT) {
                        if (!(events & MOBILE_DM_KEEP_ALIVE_EVENT) &&
                            last_events & MOBILE_DM_KEEP_ALIVE_EVENT) {
                            last_events = 0;
                            continue;
                        }
                    }
                } else {
                    if (events & MOBILE_DM_PDP_ACTIVE_EVENT ||
                        events & MOBILE_DM_KEEP_ALIVE_EVENT) {
                        /* start a timer to clear the report number & retry number */
                        u32 interval = (report_info->report_time * 60) - past_time;
                        mobile_dm_report_timer_start(interval);
                        aicx_error(0x0231e2, "d", "mobile dm: start a new timer %d to clear the report & retry number\n", interval);
                        last_events = events;
                    } else {
                        aicx_error(0x02647d, "d", "mobile dm: error occur, events: %d\n", events);
                    }

                    continue;
                }
            }

            /* retry error report number >= retry max number, release HTTP */
            if (report_info->retry_num >= report_info->retry_num_max)
                continue;

            aicx_info(0x1932b1, "ddddlld", "mobile dm: report number: %d, max:%d; retry time: %d, max:%d; time:%lld\n",
                      report_info->report_num, report_info->report_num_max,
                      report_info->retry_num, report_info->retry_num_max,
                      report_info->report_start_time);

            /* start HTTP */
            status = mobile_dm_init();
            if (status) {
                report_info->retry_num++;
                if (mobile_dm_cfg_enable)
                    aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG),
                                 &mobile_dm_cfg);
                mobile_dm_stop();
                goto report_retry;
            }

            strcpy(url, http_info->url);
            strcat(url, MOBILE_DM_CONFIG_GET_URL);
            aicx_info(0x0f2a35, "s", "mobile dm: config url:%s\n", url);
            ret = mobile_dm_configuration_get(url);
            if (ret < 0) {
                report_info->retry_num++;
                if (mobile_dm_cfg_enable)
                    aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG),
                                 &mobile_dm_cfg);
                aos_sleep(report_info->retry_interval * 60 * 1000);
                mobile_dm_stop();
                goto report_retry;
            }

            mobile_dm_rule_config_get(mobile_dm_config_res_buf);
            mobile_dm_address_config_get(mobile_dm_config_res_buf);

            if (address_config.address_cfg &&
                strcmp(address_config.address_cfg, http_info->url)) {
                strcpy(http_info->url, address_config.address_cfg);
                if (mobile_dm_cfg_enable)
                    aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG),
                                 &mobile_dm_cfg);
                mobile_dm_stop();
                goto report_retry;
            }

            /* store the report number, clear the report error number */
            if (events & MOBILE_DM_PDP_ACTIVE_EVENT) {
                report_info->report_time = rule_config.report_time;
                report_info->report_num_max = rule_config.report_num;
                report_info->retry_num_max = rule_config.retry_num;
                report_info->retry_interval = rule_config.retry_interval;
                report_info->retry_num = 0;
                report_info->report_num = 0;
            }

            if (report_info->report_num == 0) {
                aos_get_systime((aos_systime *)&tm);
                report_info->report_start_time = rtc_tm_to_time64(&tm);
            }

            strcpy(url, http_info->url);
            if (events & MOBILE_DM_KEEP_ALIVE_EVENT) {
                aicx_info(0x1e3e37, "", "mobile dm heart beat start\n");
                strcat(url, MOBILE_DM_KEEP_ALIVE_URL);
                aicx_info(0x1e7ccf, "s", "mobile dm: keep alive url:%s\n", url);
                ret = mobile_dm_heart_beat_send(url);
            } else {
                aicx_info(0x15edab, "", "mobile dm device_register start\n");
                strcat(url, MOBILE_DM_REGISTER_URL);
                aicx_info(0x11eb36, "s", "mobile dm: reg url:%s\n", url);
                ret = mobile_dm_device_register(url);
            }
            if (ret < 0) {
                report_info->retry_num++;
                if (mobile_dm_cfg_enable)
                    aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG),
                                 &mobile_dm_cfg);
                aos_sleep(report_info->retry_interval * 60 * 1000);
                mobile_dm_stop();
                goto report_retry;
            }

            report_info->report_num++;
            if (mobile_dm_cfg_enable)
                aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG),
                             &mobile_dm_cfg);

            mobile_dm_timer_start();
            mobile_dm_temp_release();

            aicx_info(0x0df80b, "", "mobile dm: this cycle of report over\n");
        }
    }

    return;
}

static void mobile_dm_service_pdp_active_callback(u8 pdp_cid, u8 pdp_state)
{
    if(pdp_cid != ANET_DEFAULT_PDP_CID || pdp_state != 1) {
        aicx_error(0x13dfc1, "", "mobile dm PDP is not activated\n");
        return;
    }
    mobile_dm_pdp_active_count++;

    aos_set_event(mobile_dm_event, MOBILE_DM_PDP_ACTIVE_EVENT, AOS_OR);

    return;
}

u32 mobile_dm_init()
{
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;

    mobile_dm_handle = aic_https_client_create(MOBILE_DM_HTTPS_CLIENT_NAME);
    if (mobile_dm_handle == 0) {
        aicx_error(0x0e47c8, "", "mobile dm https client create fail\n");
        return MOBILE_DM_HTTPS_CLIENT_CREATE_FAIL;
    }

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)mobile_dm_handle;
    client_ptr->https_client->auth_type = 0;

    return AIC_SUCCESS;
}

u32 mobile_dm_stop(void)
{
    if (address_config.address_cfg) {
        aos_free(address_config.address_cfg);
        address_config.address_cfg = AIC_NULL;
    }

    mobile_dm_report_timer_stop();
    mobile_dm_timer_stop();
    mobile_dm_temp_release();

    return AIC_SUCCESS;
}

void mobile_dm_config_enable(bool enable)
{
    mobile_dm_cfg_enable = enable;
}

void mobile_dm_report_test(void)
{
    if (mobile_dm_event != AOS_NULL) {
        aicx_info(0x0adc30, "", "mobile dm: report test!!\n");
        mobile_dm_pdp_active_count = 1;
        aos_set_event(mobile_dm_event, MOBILE_DM_PDP_ACTIVE_EVENT, AOS_OR);
    }
}

int mobile_dm_config_write(MOBILE_DM_CFG *cfg)
{
    u32 status = 0;

    if (!cfg) {
        aicx_error(0x0e6918, "", "mobile dm write cfg param invalid\n");
        return -1;
    }

    /* read config file */
    status = aic_md_write(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG), cfg);
    if (status) {
        aicx_error(0x1a088b, "d", "mobile dm cfg write fail(%d)\n", status);
        return -1;
    }

    return status;
}

int mobile_dm_config_read(MOBILE_DM_CFG *cfg)
{
    u32 status = 0;

    if (!cfg) {
        aicx_error(0x0fe87a, "", "mobile dm read cfg param invalid\n");
        return -1;
    }

    /* read config file */
    status = aic_md_read(MD_ID_MOBILE_DM, sizeof(MOBILE_DM_CFG), cfg);
    if (status) {
        aicx_error(0x016db2, "d", "mobile dm cfg read fail(%d)\n", status);
        return -1;
    }

    return status;
}

static int mobile_dm_configuration_get(u8 *url)
{
    u32 status;
    long read_bytes = 0;
    long bytes_copied = 0;
    long bytes_to_copied = 0;
    u8 rsp_code[MOBILE_DM_RESP_CODE_LEN] = {"5555555555"};

    if (mobile_dm_handle == 0) {
        aicx_error(0x1db23a, "", "mobile no dm https client\n");
        return MOBILE_NO_DM_HTTPS_CLIENT;
    }

    mobile_dm_config_post_data_inject(MOBILE_DM_REGIST_TYPE);
    mobile_dm_config_res_mem_alloc();

    status = mobile_dm_post_request_send(mobile_dm_handle, url, AIC_NULL,
                                         AIC_NULL, NX_WEB_HTTPS_SERVER_PORT,
                                         strlen(mobile_dm_config_post_buf),
                                         HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x11cd4c, "x", "mobile dm post request send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = aic_https_client_put_data(mobile_dm_handle,
                                       mobile_dm_config_post_buf,
                                       strlen(mobile_dm_config_post_buf),
                                       HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x060b38, "x", "mobile dm post data send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    bytes_to_copied = MOBILE_DM_MAX_RESP_BUF_SIZE;
    if (mobile_dm_config_res_buf)
        aos_memset(mobile_dm_config_res_buf, 0, MOBILE_DM_MAX_RESP_BUF_SIZE);

    do {
        read_bytes = aic_https_client_response_data_get(mobile_dm_handle,
                     mobile_dm_config_res_buf + bytes_copied, bytes_to_copied,
                     HTTPS_DEFAULT_TIMEOUT);

        if (read_bytes <= 0) {
            aicx_error(0x0211c6, "", "mobile dm config get receive no data from server\n");
            break;
        }
        bytes_copied += read_bytes;
        bytes_to_copied -= read_bytes;
    } while (bytes_to_copied);

    if (bytes_copied <= 0) {
        aicx_error(0x13a8ed, "", "mobile dm get no resp from server\n");
        goto ERROR_OUT;
    }

    aicx_info(0x00bd50, "s", "mobile dm get https resp is %s\n", mobile_dm_config_res_buf);

    aos_memcpy(rsp_code, mobile_dm_config_res_buf, MOBILE_DM_RESP_CODE_LEN);
    if (!strstr(rsp_code, MOBILE_DM_OK)) {
        aicx_error(0x0377b8, "s", "mobile dm get https resp error(%s)\n", rsp_code);
        goto ERROR_OUT;
    }

    mobile_dm_config_post_mem_release();
    return AIC_SUCCESS;

ERROR_OUT:
    mobile_dm_config_res_mem_release();
    mobile_dm_config_post_mem_release();
    return MOBILE_DM_CONFIGURATION_GET_FAIL;
}

static int mobile_dm_device_register(u8 *url)
{
    u32 status;
    long read_bytes = 0;
    u8 res_buf[32] = {'\0'};

    if (mobile_dm_handle == 0) {
        aicx_error(0x1db23a, "", "mobile no dm https client\n");
        return MOBILE_DM_DEVICE_REGISTER_FAIL;
    }

    mobile_dm_post_data_inject(mobile_dm_config_res_buf, MOBILE_DM_REGIST_TYPE);

    status = mobile_dm_post_request_send(mobile_dm_handle, url, AIC_NULL,
                                         AIC_NULL, NX_WEB_HTTPS_SERVER_PORT,
                                         strlen(mobile_dm_post_buf),
                                         HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x035c37, "x", "mobile dm register post request send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = aic_https_client_put_data(mobile_dm_handle,
                                       mobile_dm_post_buf,
                                       strlen(mobile_dm_post_buf),
                                       HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x02cebd, "x", "mobile dm register post data send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    read_bytes = aic_https_client_response_data_get(mobile_dm_handle, res_buf,
                                                    32, HTTPS_DEFAULT_TIMEOUT);
    if (read_bytes <= 0) {
        aicx_error(0x02817c, "", "mobile dm register receive no data from server\n");
        goto ERROR_OUT;
    }

    aicx_info(0x0048e3, "s", "mobile dm device_register resp is %s\n", res_buf);

    if (!strstr(res_buf, MOBILE_DM_OK)) {
        aicx_error(0x0f305a, "s", "mobile dm register get https resp error(%s)\n", res_buf);
        goto ERROR_OUT;
    }

    mobile_dm_post_mem_release();
    return AIC_SUCCESS;

ERROR_OUT:
    mobile_dm_post_mem_release();
    return MOBILE_DM_DEVICE_REGISTER_FAIL;
}

static int mobile_dm_heart_beat_send(u8 *url)
{
    u32 status;
    long read_bytes = 0;
    u8 res_buf[32];

    if (mobile_dm_handle == 0) {
        aicx_error(0x1db23a, "", "mobile no dm https client\n");
        return MOBILE_DM_DEVICE_REGISTER_FAIL;
    }

    mobile_dm_post_data_inject(mobile_dm_config_res_buf,
                               MOBILE_DM_KEEPALIVE_TYPE);

    status = mobile_dm_post_request_send(mobile_dm_handle, url, AIC_NULL,
                                         AIC_NULL, NX_WEB_HTTPS_SERVER_PORT,
                                         strlen(mobile_dm_post_buf),
                                         HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x11cd4c, "x", "mobile dm post request send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    status = aic_https_client_put_data(mobile_dm_handle, mobile_dm_post_buf,
                                       strlen(mobile_dm_post_buf),
                                       HTTPS_DEFAULT_TIMEOUT);
    if (status) {
        aicx_error(0x060b38, "x", "mobile dm post data send fail(0x%x)\n", status);
        goto ERROR_OUT;
    }

    read_bytes = aic_https_client_response_data_get(mobile_dm_handle, res_buf,
                                                    32, HTTPS_DEFAULT_TIMEOUT);
    if (read_bytes <= 0) {
        aicx_error(0x02817c, "", "mobile dm register receive no data from server\n");
        goto ERROR_OUT;
    }

    if (!strstr(res_buf, MOBILE_DM_OK)) {
        aicx_error(0x07e1c6, "s", "mobile dm register https resp error(%s)\n", res_buf);
        goto ERROR_OUT;
    }

    mobile_dm_post_mem_release();
    return AIC_SUCCESS;

ERROR_OUT:
    mobile_dm_post_mem_release();
    return MOBILE_DM_DEVICE_REGISTER_FAIL;
}

static int mobile_dm_service_init(void)
{
    mobile_dm_event = aos_create_event("MOBILE DM Events");
    if (mobile_dm_event == AOS_NULL) {
        aicx_error(0x16989a, "", "mobile dm event create error\n");
        return MOBILE_DM_SERVICE_INIT_FAIL;
    }

    aic_register_pdp_state_change_cb(mobile_dm_service_pdp_active_callback);

    thread_mobile_dm_service = aos_create_thread("thread_mobile_dm_service",
                                                 mobile_dm_service_entry,
                                                 NULL, SIZE_4K, 0, NULL, 10,
                                                 AOS_AUTO_START);

    if (thread_mobile_dm_service == AOS_INVALID_THREAD_ID) {
        aicx_error(0x15b71b, "", "mobile dm service thread create fail\n");
        return MOBILE_DM_SERVICE_INIT_FAIL;
    }

    return AIC_SUCCESS;
}

DIS_CHARGE_APP_P2_INIT_EXPORT(mobile_dm_service_init);
