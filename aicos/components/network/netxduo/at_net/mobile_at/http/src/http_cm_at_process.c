/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to HTTP.
 *  This is the handler of AT commands relating to http,
 *  such as setting url.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1acc40
#define LOG_MODULE    AMOD_NET

#include <aic_init.h>
#include <aic_osa.h>
#include <aic_osa_ex.h>
#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <strings.h>
#include <string.h>
#include <aic_size.h>
#include <http_cm_at_process.h>
#include <cm_at_common.h>
#include <aic_at_mtls.h>
#include <connmgr_api.h>
#include <aic_at_tcpip_common.h>
#include <aic_at_interact_cp.h>
#include <aic_net_pdp_mgr.h>
#include <aic_at_tcpip_common.h>

#ifdef CONFIG_FS
#include <fs_api.h>
#endif

#ifdef CONFIG_CRYPTO_MBEDTLS_2
#define M_HTTP_TASK_SIZE  SIZE_4K
#else
#define M_HTTP_TASK_SIZE  (SIZE_2K + SIZE_1K)
#endif
#if CONFIG_GX_SSL_AT || CONFIG_CM_SSL_AT
extern struct mbedtls_config msslconf_list[SSL_CTX_ID_MAX];
#endif
static MOBILE_HTTP_PARAS m_http_para[MOBILE_HTTP_MAX_CLIENT_NUM];
static aos_thread_id m_http_event_t;
static aos_queue_ptr m_http_q;
#ifdef CONFIG_FS
static aos_thread_id m_http_dl_t;
static aos_queue_ptr m_http_dl_q;
#endif

static AT_CMD_HANDLE m_mqtt_at_cmds[] =
{
    {HTTP_CFG_CMD, mobile_http_para_at_test_cmd, mobile_http_para_at_query_cmd,
     mobile_http_para_at_set_cmd, AIC_NULL},

    {HTTP_CREATE_CMD, mobile_http_client_create_at_test_cmd, AIC_NULL,
     mobile_http_client_create_at_set_cmd, AIC_NULL},

    {HTTP_HEADER_CMD, mobile_http_header_at_test_cmd, AIC_NULL,
     mobile_http_header_at_set_cmd, AIC_NULL},

    {HTTP_CONTENT_CMD, mobile_http_content_at_test_cmd, AIC_NULL,
     mobile_http_content_at_set_cmd, AIC_NULL},

    {HTTP_REQUEST_CMD, mobile_http_request_at_test_cmd, AIC_NULL,
     mobile_http_request_at_set_cmd, AIC_NULL},

    {MHTTP_READ_CMD, mobile_http_read_data_at_test_cmd, AIC_NULL,
     mobile_http_read_data_at_set_cmd, AIC_NULL},

    {MHTTP_DEL_CMD, mobile_http_delete_at_test_cmd, AIC_NULL,
     mobile_http_delete_at_set_cmd, AIC_NULL},

    {HTTP_TERM_CMD, mobile_http_term_at_test_cmd, AIC_NULL,
     mobile_http_term_at_set_cmd, AIC_NULL},

#ifdef CONFIG_FS
    {HTTP_DLFILE_CMD, mobile_http_download_file_at_test_cmd, AIC_NULL,
     mobile_http_download_file_at_set_cmd, AIC_NULL},
#endif

    {AIC_NULL},
};

/* Define http paras maps. */
static MOBILE_HTTP_AT_PARAS_MAP mobile_http_paras_map[] =
{
    {"ssl",        MOBILE_HTTP_AT_SSL,       3},
    {"cid",        MOBILE_HTTP_AT_CID,       3},
    {"header",     MOBILE_HTTP_AT_HEADER,    6},
    {"cached",     MOBILE_HTTP_AT_CACHED,    6},
    {"timeout",    MOBILE_HTTP_AT_TIMEOUT,   7},
    {"chunked",    MOBILE_HTTP_AT_CHUNKED,   7},
    {"encoding",   MOBILE_HTTP_AT_ENCODING,  8},
    {"fragment",   MOBILE_HTTP_AT_FRAGMENT,  8},
    {"urlencode",  MOBILE_HTTP_AT_URLENCODE, 9},
};

static u32 MOBILE_HTTP_AT_PARAS_MAP_SIZE = sizeof(mobile_http_paras_map) /
                                           sizeof(MOBILE_HTTP_AT_PARAS_MAP);

static void mobile_http_local_path_mem_release(u8 httpid)
{
    if (m_http_para[httpid].local_path) {
        aos_free(m_http_para[httpid].local_path);
        m_http_para[httpid].local_path = AIC_NULL;
    }
}

static void mobile_http_content_mem_release(u8 httpid)
{
    if (m_http_para[httpid].s_content.data) {
        aos_free(m_http_para[httpid].s_content.data);
        m_http_para[httpid].s_content.data = AIC_NULL;
    }

    aos_memset(&m_http_para[httpid].s_content, '\0',
               sizeof(MOBILE_HTTP_HEADER_CONTENT_INFO));
}

static u32 mobile_http_content_mem_alloc(u8 httpid)
{
    if (!m_http_para[httpid].s_content.data)
        m_http_para[httpid].s_content.data =
            aos_zalloc_relaxed(3 * MOBILE_HTTP_MAX_CONTENT_DATA_LEN + 1);

    if (!m_http_para[httpid].s_content.data)
        return MOBILE_HTTP_MEMORY_ALLOCATION_FAILED;
    else
        return AIC_SUCCESS;
}

static void mobile_http_specific_header_mem_release(u8 httpid)
{
    if (m_http_para[httpid].s_header.data) {
        aos_free(m_http_para[httpid].s_header.data);
        m_http_para[httpid].s_header.data = AIC_NULL;
    }

    aos_memset(&m_http_para[httpid].s_header, '\0',
               sizeof(MOBILE_HTTP_HEADER_CONTENT_INFO));
}

static void mobile_http_specific_header_mem_alloc(u8 httpid)
{
    /* MOBILE_HTTP_MAX_HEADER_LEN + \r\n + \0 */
    if (!m_http_para[httpid].s_header.data)
        m_http_para[httpid].s_header.data =
            aos_zalloc(MOBILE_HTTP_MAX_HEADER_LEN + 2 + 1);
}

static void mobile_http_specific_header_key_value_mem_release(u8 httpid)
{
    int i;

    for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
        if (m_http_para[httpid].sp_k_v[i].key) {
            aos_free(m_http_para[httpid].sp_k_v[i].key);
            m_http_para[httpid].sp_k_v[i].key = AIC_NULL;
        }

        if (m_http_para[httpid].sp_k_v[i].value) {
            aos_free(m_http_para[httpid].sp_k_v[i].value);
            m_http_para[httpid].sp_k_v[i].value = AIC_NULL;
        }
    }
}

static void mobile_http_common_header_key_value_mem_release(u8 httpid)
{
    int i;

    for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
        if (m_http_para[httpid].comm_k_v[i].key) {
            aos_free(m_http_para[httpid].comm_k_v[i].key);
            m_http_para[httpid].comm_k_v[i].key = AIC_NULL;
        }

        if (m_http_para[httpid].comm_k_v[i].value) {
            aos_free(m_http_para[httpid].comm_k_v[i].value);
            m_http_para[httpid].comm_k_v[i].value = AIC_NULL;
        }
    }

    return;
}

static void mobile_http_common_header_mem_release(u8 httpid)
{

    if (m_http_para[httpid].header.data) {
        aos_free(m_http_para[httpid].header.data);
        m_http_para[httpid].header.data = AIC_NULL;
    }

    if (m_http_para[httpid].header.has_data)
        m_http_para[httpid].header.has_data = 0;

    return;
}

static void mobile_http_common_header_mem_alloc(u8 httpid)
{
    if (!m_http_para[httpid].header.data)
        m_http_para[httpid].header.data =
            aos_zalloc(MOBILE_HTTP_MAX_HEADER_LEN + 4); /* MOBILE_HTTP_MAX_HEADER_LEN + \r\n + \0 */

    return;
}

static void mobile_http_read_data_mem_release(u8 httpid)
{
    if (m_http_para[httpid].read_data_info.read_data_buf) {
        aos_free(m_http_para[httpid].read_data_info.read_data_buf);
        m_http_para[httpid].read_data_info.read_data_buf = AIC_NULL;
    }

    if (m_http_para[httpid].read_data_info.read_header_buf) {
        aos_free(m_http_para[httpid].read_data_info.read_header_buf);
        m_http_para[httpid].read_data_info.read_header_buf = AIC_NULL;
    }

    return;
}

static void mobile_http_read_data_mem_alloc(u8 httpid)
{
    if (m_http_para[httpid].read_data_info.read_data_buf)
        aos_free(m_http_para[httpid].read_data_info.read_data_buf);

    m_http_para[httpid].read_data_info.read_data_buf =
                aos_zalloc(m_http_para[httpid].read_data_info.to_read_len +
                           MOBILE_HTTP_URC_HEADER_LEN);

    if (m_http_para[httpid].read_data_info.read_header_buf)
        aos_free(m_http_para[httpid].read_data_info.read_header_buf);

    m_http_para[httpid].read_data_info.read_header_buf =
                aos_zalloc(m_http_para[httpid].read_data_info.to_read_len +
                           MOBILE_HTTP_URC_HEADER_LEN);

    return;
}

static void mobile_http_cached_data_mem_release(u8 httpid)
{
    if (m_http_para[httpid].cached_data_info.cached_data) {
        aos_free(m_http_para[httpid].cached_data_info.cached_data);
        m_http_para[httpid].cached_data_info.cached_data = AIC_NULL;
    }

    if (m_http_para[httpid].cached_data_info.cached_header) {
        aos_free(m_http_para[httpid].cached_data_info.cached_header);
        m_http_para[httpid].cached_data_info.cached_header = AIC_NULL;
    }

    return;
}

static void mobile_http_cached_data_mem_alloc(u8 httpid)
{

    if (m_http_para[httpid].cached_data_info.cache_len == 0)
        m_http_para[httpid].cached_data_info.cache_len = MOBILE_HTTP_DEFAULT_CACHE_LEN;

    if (m_http_para[httpid].cached_data_info.cached_data)
        aos_free(m_http_para[httpid].cached_data_info.cached_data);

    m_http_para[httpid].cached_data_info.cached_data =
        aos_zalloc(m_http_para[httpid].cached_data_info.cache_len + 1);

    if (m_http_para[httpid].cached_data_info.cached_header)
        aos_free(m_http_para[httpid].cached_data_info.cached_header);

    m_http_para[httpid].cached_data_info.cached_header =
        aos_zalloc( m_http_para[httpid].cached_data_info.cache_len + 1);

    return;
}

static void mobile_http_response_data_mem_release(u8 httpid)
{
    if (m_http_para[httpid].res_data.res_data_buf) {
        aos_free(m_http_para[httpid].res_data.res_data_buf);
        m_http_para[httpid].res_data.res_data_buf = AIC_NULL;
    }

    return;
}

static void mobile_http_response_hex_data_mem_release(u8 httpid)
{
    if (m_http_para[httpid].res_data.res_hex_buf) {
        aos_free(m_http_para[httpid].res_data.res_hex_buf);
        m_http_para[httpid].res_data.res_hex_buf = AIC_NULL;
    }

    return;
}

static void mobile_http_connect_info_mem_release(u8 httpid)
{
    if (m_http_para[httpid].connect_info.url) {
        aos_free(m_http_para[httpid].connect_info.url);
        m_http_para[httpid].connect_info.url = AIC_NULL;
    }

    if (m_http_para[httpid].connect_info.host) {
        aos_free(m_http_para[httpid].connect_info.host);
        m_http_para[httpid].connect_info.host = AIC_NULL;
    }

    if (m_http_para[httpid].connect_info.resource) {
        aos_free(m_http_para[httpid].connect_info.resource);
        m_http_para[httpid].connect_info.resource = AIC_NULL;
    }

    aos_memset(&m_http_para[httpid].connect_info, '\0',
               sizeof(MOBILE_HTTP_CONNECT_INFO));

    return;
}

static void mobile_http_read_data_info_reset(u8 httpid)
{
    mobile_http_read_data_mem_release(httpid);

    aos_memset(&m_http_para[httpid].read_data_info, '\0',
               sizeof(MOBILE_HTTP_READ_DATA_INFO));
    return;
}

static void mobile_http_res_data_info_reset(u8 httpid)
{
    mobile_http_response_data_mem_release(httpid);
    mobile_http_response_hex_data_mem_release(httpid);

    aos_memset(&m_http_para[httpid].res_data, '\0',
               sizeof(MOBILE_HTTP_RESPONSE_URC_DATA_INFO));

    return;
}

static void moble_http_default_paras_set(u8 httpid)
{
    m_http_para[httpid].conn_timeout = MOBILE_HTTP_DEFAULT_CONNECT_TIMEOUT;
    m_http_para[httpid].input_timeout = MOBILE_HTTP_DEFAULT_INPUT_TIMEOUT;
    m_http_para[httpid].cached_data_info.cache_len = MOBILE_HTTP_DEFAULT_CACHE_LEN;
    m_http_para[httpid].cid = MOBILE_HTTP_INVALID_ID;
    m_http_para[httpid].s_content.eof = MOBILE_HTTP_INVALID_ID;
    m_http_para[httpid].is_term = 0;

    return;
}

static void mobile_http_paras_reset(u8 httpid)
{
    /* release the alloca buffer. */
    mobile_http_read_data_mem_release(httpid);
    mobile_http_cached_data_mem_release(httpid);
    mobile_http_response_data_mem_release(httpid);
    mobile_http_response_hex_data_mem_release(httpid);
    mobile_http_connect_info_mem_release(httpid);
    mobile_http_specific_header_mem_release(httpid);
    mobile_http_specific_header_key_value_mem_release(httpid);
    mobile_http_common_header_key_value_mem_release(httpid);
    mobile_http_common_header_mem_release(httpid);
    mobile_http_content_mem_release(httpid);
    mobile_http_local_path_mem_release(httpid);

    aos_memset(&m_http_para[httpid], '\0', sizeof(MOBILE_HTTP_PARAS));
    moble_http_default_paras_set(httpid);
    m_http_para[httpid].httpid = MOBILE_HTTP_INVALID_ID;

    return;
}

/* Note: dst_data_len is an IN/OUT parameter.
 * it represents the maximum dst data buffer size as INPUT and the final formatted length as OUTPUT */
static int mobile_http_input_data_format_process(u8 httpid, u8 *dst_data,
                                                 u8 *src_data, u32 src_data_len,
                                                 u32 *dst_data_len)
{
    int status = 0;
    u8 *temp_buf = AIC_NULL;
    u32 max_dest_buf_size = *dst_data_len;

    if (max_dest_buf_size == 0)
        return 0;

    if (!dst_data)
        return 1;

    switch(m_http_para[httpid].input_format) {
    case MOBILE_HTTP_ASCII:
        if (src_data_len > max_dest_buf_size)
            src_data_len = max_dest_buf_size;
        aos_memcpy(dst_data, src_data, src_data_len);
        *dst_data_len = src_data_len;
        break;
    case MOBILE_HTTP_HEX:
        if (src_data_len > max_dest_buf_size)
            src_data_len = max_dest_buf_size;
        status = aic_hexstring_to_char(dst_data, src_data, src_data_len * 2);
        *dst_data_len = src_data_len;
        break;
    case MOBILE_HTTP_ESCAPE:
        temp_buf = aos_zalloc(src_data_len + 4);        /* 4 bytes more to avoid data overflow */
        if (!temp_buf) {
            status = AIC_ENOMEM;
            break;
        }
        aic_escape_data_normal(temp_buf, src_data, src_data_len,
                               &max_dest_buf_size);
        if (*dst_data_len < max_dest_buf_size) {
            max_dest_buf_size = *dst_data_len;
        } else {
            *dst_data_len = max_dest_buf_size;
        }
        aos_memcpy(dst_data, temp_buf, max_dest_buf_size);
        aos_free(temp_buf);
        break;
    default:
        break;
    }

    return status;
}

static u8 mobile_http_ssl_enable_check(u8 *host)
{
    u8 *url_head;
    const u8 *flag = "/";
    char *next = AIC_NULL;
    u8 *temp;

    temp = aos_zalloc(MOBILE_HTTP_MAX_HOST_LEN + 1);
    strncpy(temp, host, MOBILE_HTTP_MAX_HOST_LEN);

    url_head = strtok_r(temp, flag, &next);
    aos_free(temp);

    if (strncasecmp(url_head, "https:", sizeof("https:")) == 0)
        return MOBILE_HTTP_SSL_ENABLE;
    else if (strncasecmp(url_head, "http:", sizeof("http:")) == 0)
        return MOBILE_HTTP_SSL_DISABLE;
    else {
        return MOBILE_HTTP_SSL_ENABLE;
    }
}

static void mobile_http_server_port_find(u8 *host, u8 httpid, u8 ssl_enable)
{
    u8 *found;
    u8 *temp;
    u8 *check_v6 = AIC_NULL;
    u8 offset_len = 0;
    char *flag = "/";
    char *next = AIC_NULL;
    char *http_url_head = AIC_NULL;
    u32 no_http_header = 0;
    u32 http_header_offset = 0;

    temp = aos_zalloc(MOBILE_HTTP_MAX_HOST_LEN + 1);
    strncpy(temp, host, MOBILE_HTTP_MAX_HOST_LEN);

    http_url_head = strtok_r(temp, flag, &next);
    if (http_url_head == AIC_NULL) {
        aos_free(temp);
        return;
    }

    /* http host input is string, can use strncpy */
    if (ssl_enable)
        m_http_para[httpid].connect_info.port = NX_WEB_HTTPS_SERVER_PORT;
    else
        m_http_para[httpid].connect_info.port = NX_WEB_HTTP_SERVER_PORT;

    if (strlen(http_url_head) == strlen(host)) {
        /* no http header and resource and port, such as 192.168.0.1:80 */
        found = strchr(http_url_head, ':');
        if (!found) {
            aos_free(temp);
            return;
        }
    }

    if (strncasecmp(http_url_head, "http", 4) != 0){
        /*no http header such as 192.168.0.1/doc/test.txt */
        no_http_header = 1;
    } else {
        /*http_url_head is http: or https: , need add //, so add 2 char */
        http_header_offset = strlen(http_url_head) + 2;
    }

    strncpy(temp, host, MOBILE_HTTP_MAX_HOST_LEN);
    if (host[http_header_offset] == '[') {
        aos_memset(m_http_para[httpid].connect_info.host, '\0',
                   MOBILE_HTTP_MAX_HOST_LEN);

        strncpy(m_http_para[httpid].connect_info.host,
                temp + http_header_offset, MOBILE_HTTP_MAX_HOST_LEN);

        aos_free(temp);
        return;
    }

    if (no_http_header)
        found = strchr(temp, ':');
    else
        found = strchr(temp + http_header_offset, ':');

    if (found) {
        if (no_http_header)
            offset_len = strlen(found);
        else
            offset_len = strlen(found) + http_header_offset;
        found++;

        check_v6 = strchr(found, ':');
        if (check_v6) {
            //alog_debug("mobile http may be ipv6 addr, not a port");
            aos_memset(m_http_para[httpid].connect_info.host, '\0',
                       MOBILE_HTTP_MAX_HOST_LEN);
            if (no_http_header)
                strncpy(m_http_para[httpid].connect_info.host,
                        temp, strlen(temp));
            else
                strncpy(m_http_para[httpid].connect_info.host,
                        temp + http_header_offset,
                        (strlen(temp) - http_header_offset));
            aos_free(temp);
            return;
        } else
            m_http_para[httpid].connect_info.port = atoi(found);
    }

    aos_memset(m_http_para[httpid].connect_info.host, '\0',
               MOBILE_HTTP_MAX_HOST_LEN);

    if (no_http_header)
        strncpy(m_http_para[httpid].connect_info.host,
                temp, strlen(temp) - offset_len);
    else
        strncpy(m_http_para[httpid].connect_info.host,
                temp + http_header_offset, strlen(temp) - offset_len);

    strncpy(temp, m_http_para[httpid].connect_info.host, MOBILE_HTTP_MAX_HOST_LEN);

    /*if host is 192.168.1.1/abc/ef need to dump /abc/ef */
    found = strtok_r(temp, flag, &next);
    if (found) {
        aos_memset(m_http_para[httpid].connect_info.host, '\0', MOBILE_HTTP_MAX_HOST_LEN);
        strncpy(m_http_para[httpid].connect_info.host, found, MOBILE_HTTP_MAX_HOST_LEN);
    }

    aos_free(temp);
    return;
}

static u8 mobile_http_idle_httpid_find(void)
{
    int i;

    for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
        if (m_http_para[i].handle == 0) {
            break;
        }
    }

    if (i == MOBILE_HTTP_MAX_CLIENT_NUM)
        i = MOBILE_HTTP_INVALID_ID;

    return i;
}

#ifdef CONFIG_FS
static u8 mobile_http_idle_dl_httpid_find(void)
{
    int i;

    for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
        if (m_http_para[i].handle == 0 && m_http_para[i].downloading == 0) {
            break;
        }
    }

    if (i == MOBILE_HTTP_MAX_CLIENT_NUM)
        i = MOBILE_HTTP_INVALID_ID;

    return i;
}
#endif

static u8 mobile_http_idle_headerid_find(u8 httpid, u8 header_type)
{
    int i = MOBILE_HTTP_MAX_HEADER_NUM;

    if (header_type == MOBILE_HTTP_SP_HEADER_ID) {
        for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
            if (m_http_para[httpid].sp_k_v[i].key == AIC_NULL) {
                break;
            }
        }
    }

    if (header_type == MOBILE_HTTP_COMMON_HEADER_ID) {
        for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
            if (m_http_para[httpid].comm_k_v[i].key == AIC_NULL) {
                break;
            }
        }
    }

    if (i == MOBILE_HTTP_MAX_HEADER_NUM)
        i = MOBILE_HTTP_INVALID_ID;

    return i;
}

static s32 mobile_http_parse_key_value_pair(const u8 *line, size_t len, u8 **key, u8 **value)
{
    s32 ret = -1;
    size_t i = 0;

    /* find colon */
    while (i < len) {
        if (line[i] == ':') {
            size_t key_len = i;
            size_t value_len = len - i - 1;

            if (key_len == 0 || value_len == 0) {
                /* invalid key value pair, ignore it */
                break;
            }
            *key = aos_zalloc(key_len + 1);
            if (!*key) {
                break;
            }
            *value = aos_zalloc(value_len + 1);
            if (!*value) {
                aos_free(*key);
                break;
            }
            strncpy(*key, line, key_len);
            strncpy(*value, &line[i + 1], value_len);
            ret = AIC_SUCCESS;
            break;
        }
        i++;
    }

    return ret;
}

static void mobile_http_comm_header_key_and_value_find(u8 *data, size_t len, u8 httpid)
{
    s32 ret = 0;
    u8 index = MOBILE_HTTP_INVALID_ID;

    if (data) {
        u8 *key = NULL;
        u8 *value = NULL;

        ret = mobile_http_parse_key_value_pair(data, len, &key, &value);

        if (ret == AIC_SUCCESS) {
            /* find slot to fill */
            index = mobile_http_idle_headerid_find(httpid, MOBILE_HTTP_COMMON_HEADER_ID);
            if (index == MOBILE_HTTP_INVALID_ID) {
                return;
            }
            m_http_para[httpid].comm_k_v[index].key = key;
            m_http_para[httpid].comm_k_v[index].value = value;
        }
    }

    return;
}

static void mobile_http_sp_header_key_and_value_find(u8 httpid)
{
    u8 index = MOBILE_HTTP_INVALID_ID;
    size_t len = 0;
    s32 ret = 0;
    const u8 *line_b = AIC_NULL;
    const u8 *line_e = AIC_NULL;
    const u8 *content_end = AIC_NULL;

    /* parse http header only when header input is eof */
    if (m_http_para[httpid].s_header.eof == MOBILE_HTTP_END_DATA_EOF &&
        m_http_para[httpid].s_header.data) {
        line_b = m_http_para[httpid].s_header.data;
        content_end = &m_http_para[httpid].s_header.data[m_http_para[httpid].s_header.actual_len];
        while (line_b && (line_b < content_end)) {
            u8 *key = NULL;
            u8 *value = NULL;

            line_e = strstr(line_b, "\r\n");
            if (line_e) {
                len = (size_t)(line_e - line_b);
            } else {
                len = (size_t)(content_end - line_b);
            }
            ret = mobile_http_parse_key_value_pair(line_b, len, &key, &value);
            if (ret == AIC_SUCCESS) {
                /* find slot to fill */
                index = mobile_http_idle_headerid_find(httpid, MOBILE_HTTP_SP_HEADER_ID);
                if (index == MOBILE_HTTP_INVALID_ID) {
                    break;
                }
                m_http_para[httpid].sp_k_v[index].key = key;
                m_http_para[httpid].sp_k_v[index].value = value;
            }
            if (line_e) {
                line_b = line_e + 2;
            } else {
                break;
            }
        }
    }
}

static u32 mobile_http_paras_set(AT_REQ_RESP *at_ptr)
{
    u8 *tag;
    ulong *paras;
    u16 count = 0;
    s8 httpid = MOBILE_HTTP_INVALID_ID;
    u8 i;
    u8 para_code = 0;
    u32 status = 0;
    u32 out_data_len = 0;
    MOBILE_HTTP_PARAS_TEMP temp_paras = {0};

    if (mobile_specific_cmd_check(at_ptr->at_req))
        paras = mobile_split_paras_to_str_array(at_ptr->at_req, HTTP_CFG_CMD"=",
                                                &count, 3, 3, 0);
    else
        paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_CFG_CMD"=",
                                             &count);

    if (!paras || count < 2)
        goto ERROR_OUT;

    tag = (u8*)paras[0];
    httpid = atoi((u8*)paras[1]);

    /* Determine which para is neet to set. */
    for (i = 0; i < MOBILE_HTTP_AT_PARAS_MAP_SIZE; i++)
    {
        if (strncasecmp(tag, mobile_http_paras_map[i].mobile_http_at_paras_str,
            mobile_http_paras_map[i].paras_len) == 0)
        {
            para_code = mobile_http_paras_map[i].mobile_http_at_paras_code;
            break;
        }
    }

    /* set para */
    if (paras && count > 2) {
        if (httpid < 0 || httpid >= MOBILE_HTTP_MAX_CLIENT_NUM)
            goto ERROR_OUT;

        m_http_para[httpid].httpid = httpid;
    }

    switch(para_code) {
#if CONFIG_NET_HTTPS && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT)
    case MOBILE_HTTP_AT_SSL:
        if (count >= 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            temp_paras.ssl_enable = atoi((u8*)paras[2]);
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.ssl_id = m_http_para[httpid].ssl_id;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, MOBILE_HTTP_MAX_SSL_ID))
                    goto ERROR_OUT;

                temp_paras.ssl_id = atoi((u8*)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3)
            m_http_para[httpid].ssl_enable = temp_paras.ssl_enable;
        if (count >= 4)
            m_http_para[httpid].ssl_id = temp_paras.ssl_id;

        if (m_http_para[httpid].client_ptr) {
            m_http_para[httpid].client_ptr->https_client->auth_type =
                msslconf_list[m_http_para[httpid].ssl_id].seclevel;
        }

        if (m_http_para[httpid].handle != 0 && m_http_para[httpid].client_ptr &&
           m_http_para[httpid].client_ptr->https_client->auth_type != 0) {
            if (msslconf_list[m_http_para[httpid].ssl_id].cacert) {
                aic_https_ca_crt_data_add(m_http_para[httpid].handle,
                    msslconf_list[m_http_para[httpid].ssl_id].cacert,
                    (u16)msslconf_list[m_http_para[httpid].ssl_id].cacert_len,
                    httpid);
            }
        }

#ifdef CONFIG_CRYPTO_MBEDTLS
        if (m_http_para[httpid].client_ptr &&
            m_http_para[httpid].client_ptr->https_client &&
            m_http_para[httpid].client_ptr->https_client->http_mb_cfg)
            m_http_para[httpid].client_ptr->https_client->http_mb_cfg->recv_wait_time =
                                              MOBILE_HTTP_DEFAULT_WAIT_TIME;
#endif
        break;
#endif /* (CONFIG_NET_HTTPS) && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT) */
    case MOBILE_HTTP_AT_CID:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 1, ANET_MAX_PDP_CID))
                goto ERROR_OUT;

            m_http_para[httpid].cid = atoi((u8*)paras[2]);
        } else
            goto ERROR_OUT;

        break;
    case MOBILE_HTTP_AT_HEADER:
        if (count == 3) {
            size_t content_len = strlen((u8 *)paras[2]);
            if (content_len > MOBILE_HTTP_MAX_COMMON_HEADER_LEN || content_len <= 0)
                goto ERROR_OUT;

            if (m_http_para[httpid].input_format == MOBILE_HTTP_HEX) {
                if (content_len & 1) {
                    goto ERROR_OUT;
                }
                m_http_para[httpid].header.input_len = content_len >> 1;
            } else
                m_http_para[httpid].header.input_len = content_len;

            if (m_http_para[httpid].header.offset >= MOBILE_HTTP_MAX_HEADER_LEN) {
                m_http_para[httpid].header.input_len = 0;
                break;
            }
            out_data_len = MOBILE_HTTP_MAX_HEADER_LEN - m_http_para[httpid].header.offset;

            mobile_http_common_header_mem_alloc(httpid);

            status = mobile_http_input_data_format_process(
                httpid, m_http_para[httpid].header.data + m_http_para[httpid].header.offset, (u8 *)paras[2],
                m_http_para[httpid].header.input_len, &out_data_len);
            if (status)
                goto ERROR_OUT;

            m_http_para[httpid].header.has_data = 1;
            mobile_http_comm_header_key_and_value_find(
                m_http_para[httpid].header.data + m_http_para[httpid].header.offset,
                m_http_para[httpid].input_format == MOBILE_HTTP_ESCAPE ? out_data_len
                                                                       : m_http_para[httpid].header.input_len,
                httpid);

            m_http_para[httpid].header.actual_len += out_data_len;
            /* add \r\n to the end */
            m_http_para[httpid].header.data[m_http_para[httpid].header.actual_len] = '\r';
            m_http_para[httpid].header.data[m_http_para[httpid].header.actual_len + 1] = '\n';
            m_http_para[httpid].header.actual_len += 2;

            m_http_para[httpid].header.offset = m_http_para[httpid].header.actual_len;
        } else
            goto ERROR_OUT;
        break;
    case MOBILE_HTTP_AT_CACHED:
        if (count >= 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            temp_paras.cached = atoi((u8 *)paras[2]);
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.cache_len = m_http_para[httpid].cached_data_info.cache_len;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 1, MOBILE_HTTP_MAX_CACHE_LEN))
                    goto ERROR_OUT;

                temp_paras.cache_len = atoi((u8 *)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3)
            m_http_para[httpid].cached = temp_paras.cached;
        if (count >= 4)
            m_http_para[httpid].cached_data_info.cache_len = temp_paras.cache_len;

        break;
    case MOBILE_HTTP_AT_TIMEOUT:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.conn_timeout = m_http_para[httpid].conn_timeout;
            else {
                temp_paras.conn_timeout = atoi((u8 *)paras[2]);

                if (mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_HTTP_CONNECT_TIMEOUT))
                    goto ERROR_OUT;

                temp_paras.conn_timeout = 1000 * temp_paras.conn_timeout;
            }
        }

        if (count >= 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.rsp_timeout = m_http_para[httpid].rsp_timeout;
            else {
                temp_paras.rsp_timeout = atoi((u8 *)paras[3]);
                if (mobile_at_para_range_check((u8 *)paras[3], 0, MOBILE_HTTP_RESPONSE_TIMEOUT))
                    goto ERROR_OUT;

                temp_paras.rsp_timeout = 1000 * temp_paras.rsp_timeout;
            }
        }

        if (count == 5) {
            if (*(u8 *)paras[4] == 0)
                temp_paras.input_timeout = m_http_para[httpid].input_timeout;
            else {
                temp_paras.input_timeout = atoi((u8 *)paras[4]);
                if (mobile_at_para_range_check((u8 *)paras[4], 1, MOBILE_HTTP_INPUT_TIMEOUT))
                    goto ERROR_OUT;

                temp_paras.input_timeout = 1000 * temp_paras.input_timeout;
            }
        }

        if (count > 5)
            goto ERROR_OUT;

        if (count >= 3)
            m_http_para[httpid].conn_timeout = temp_paras.conn_timeout;
        if (count >= 4)
            m_http_para[httpid].rsp_timeout = temp_paras.rsp_timeout;
        if (count >= 5)
            m_http_para[httpid].input_timeout = temp_paras.input_timeout;

        break;
    case MOBILE_HTTP_AT_CHUNKED:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            m_http_para[httpid].chunked_mode = atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;

        break;
    case MOBILE_HTTP_AT_ENCODING:
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.input_format = m_http_para[httpid].input_format;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, 2))
                    goto ERROR_OUT;

                temp_paras.input_format = atoi((u8 *)paras[2]);
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.out_format = m_http_para[httpid].out_format;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, 1))
                    goto ERROR_OUT;

                temp_paras.out_format = atoi((u8 *)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3)
            m_http_para[httpid].input_format = temp_paras.input_format;
        if (count >= 4)
            m_http_para[httpid].out_format = temp_paras.out_format;

        break;
    case MOBILE_HTTP_AT_FRAGMENT:
        if (m_http_para[httpid].has_set_fragment) {
            aic_free_paras_mem(paras, count);
            return MOBILE_HTTP_OPERATION_NOT_ALLOWED;
        }
        if (count >= 3) {
            if (*(u8 *)paras[2] == 0)
                temp_paras.frag_size = m_http_para[httpid].frag_size;
            else {
                if (mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_HTTP_MAX_FRAG_SIZE))
                    goto ERROR_OUT;

                temp_paras.frag_size = atoi((u8 *)paras[2]);
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[3] == 0)
                temp_paras.interval = m_http_para[httpid].interval;
            else {
                if (mobile_at_para_range_check((u8 *)paras[3], 0, MOBILE_HTTP_MAX_INTERVAL_TIME))
                    goto ERROR_OUT;

                temp_paras.interval = atoi((u8 *)paras[3]);
            }
        }

        if (count > 4)
            goto ERROR_OUT;

        if (count >= 3)
            m_http_para[httpid].frag_size = temp_paras.frag_size;
        if (count >= 4)
            m_http_para[httpid].interval = temp_paras.interval;

        break;
    case MOBILE_HTTP_AT_URLENCODE:
        if (count == 3) {
            if (mobile_at_para_range_check((u8 *)paras[2], 0, 1))
                goto ERROR_OUT;

            m_http_para[httpid].urlencode_mode= atoi((u8 *)paras[2]);
        } else
            goto ERROR_OUT;

        break;
    default:
        goto ERROR_OUT;
        break;
    }

    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

static void mobile_http_specific_content_header_set(AT_REQ_RESP *at_ptr, u32 httpid, u8 set_type,
                                                    u32 once_input_len, u32 max_input_len)
{
    int temp_size = 0;
    boolean is_exceeded = AIC_FALSE;
    MOBILE_HTTP_HEADER_CONTENT_INFO *info = AIC_NULL;

    if (set_type == 1)
        info = &m_http_para[httpid].s_content;
    else
        info = &m_http_para[httpid].s_header;


    /* end by ctrl+Z */
    if (info->data && info->end_mob == 1) {
        temp_size = at_ptr->upstream_data_len;

        /* quit composing upon receiving single <ESC> */
        if (temp_size == 1 && at_ptr->at_upstream_data[0] == MOBILE_HTTP_ESC) {

            if (info->offset == 0) {
                if (set_type == 1)
                    mobile_http_content_mem_release(httpid);
                else {
                    /* release s_header when receive ESC */
                    mobile_http_specific_header_mem_release(httpid);
                    mobile_http_specific_header_key_value_mem_release(httpid);
                }
            } else {
                /* roll back to the last confirmed input */
                info->actual_len = info->offset;
            }

            /* enable the next content input */
            info->eof = MOBILE_HTTP_INVALID_ID;
            goto URC_OUT;
        }

        /* trigger sending upon receiving single <CTRL+Z> */
        if (temp_size == 1 && at_ptr->at_upstream_data[0] == MOBILE_HTTP_CTRL_Z) {
            if (set_type == 1) {
                info->offset = info->actual_len;
                /* reset eof only when there was no input at all */
                if (!info->actual_len) {
                    info->eof = MOBILE_HTTP_INVALID_ID;
                }
                goto URC_OUT;
            } else {
                if (info->offset == info->actual_len) {
                    /* No actual input, need to report +CME ERROR: 50 */
                    goto ERROR_EXIT;
                } else {
                    goto URC_OUT;
                }
            }
        }

        /* limit the current input within length of MOBILE_HTTP_MAX_CONTENT_DATA_LEN */
        /* if limits exceeded, goto URC out*/
        if (set_type == 1) {
            if (info->actual_len - info->offset + temp_size >= once_input_len) {
                temp_size = once_input_len - (info->actual_len - info->offset);
                is_exceeded = AIC_TRUE;
            }
            /* moreover, check if the total input exceeds the length limit of 3 * MOBILE_HTTP_MAX_CONTENT_DATA_LEN */
            if (info->actual_len + temp_size >= max_input_len) {
                temp_size = max_input_len - info->actual_len;
                is_exceeded = AIC_TRUE;
            }
        } else {
            if (info->actual_len + temp_size >= once_input_len) {
                //alog_info("mobile http header input overflow. actual_len(%d), temp_size(%d), cut off",
                         // m_http_para[httpid].s_header.actual_len, temp_size);

                temp_size = once_input_len - info->actual_len;
                is_exceeded = AIC_TRUE;
            }
        }

        if (temp_size > 0) {
            aos_memcpy(info->data + info->actual_len,
                       at_ptr->at_upstream_data, temp_size);

            info->actual_len += temp_size;
        }
        if (is_exceeded) {
            if (set_type == 1)
                info->offset = info->actual_len;

            goto URC_OUT;
        }
    }

    /* set http content by length */
    if (info->data && info->input_len > 0 && info->end_mob == 0) {
        u32 inputted_cnt = info->actual_len - info->offset;

        if (inputted_cnt < info->input_len) {
            temp_size = at_ptr->upstream_data_len;

            if (inputted_cnt + temp_size > info->input_len) {
                temp_size = info->input_len - inputted_cnt;
                is_exceeded = AIC_TRUE;
            }

            /* moreover, check if the total input exceeds the length limit of max len */
            if (info->actual_len + temp_size >= max_input_len) {
                temp_size = max_input_len - info->actual_len;
                is_exceeded = AIC_TRUE;
            }

            if (temp_size > 0) {
                aos_memcpy(info->data + info->actual_len,
                           at_ptr->at_upstream_data, temp_size);
                info->actual_len += temp_size;
            }

            if (is_exceeded || (info->actual_len - info->offset == info->input_len)) {
                if (set_type == 1)
                    info->offset = info->actual_len;
                goto URC_OUT;
            }


        } else {
            //alog_debug("mobile http content length enough\n");
            goto URC_OUT;
        }
    }
    return;

ERROR_EXIT:
    if (info->offset == 0 && set_type == 0) {
        mobile_http_specific_header_mem_release(httpid);
        mobile_http_specific_header_key_value_mem_release(httpid);
        info->eof = MOBILE_HTTP_INVALID_ID;
        at_ptr->at_mode = AT_CMD_MODE;
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
        return;
    }

URC_OUT:
    if (info->actual_len != info->offset && set_type == 0) {
        /* insert \r\n to the end of header */
        info->data[info->actual_len] = '\r';
        info->data[info->actual_len + 1] = '\n';
        info->actual_len += 2;
        info->offset = info->actual_len;
        mobile_http_sp_header_key_and_value_find(httpid);
    }
    at_ptr->at_mode = AT_CMD_MODE;
    aic_reply_ok_resp(at_ptr);
    return;
}

static void mobile_http_specific_header_set(AT_REQ_RESP *at_ptr, u32 httpid)
{
    mobile_http_specific_content_header_set(at_ptr, httpid, 0, 4096, 4096);

}

static void mobile_http_specific_content_set(AT_REQ_RESP *at_ptr, u32 httpid)
{
    mobile_http_specific_content_header_set(at_ptr, httpid, 1, 4096, 3*4096);

}

static void mobile_http_resource_set(AT_REQ_RESP *at_ptr, u32 httpid)
{
    u32 temp_size = 0;
    u32 last_char = 0;

    /* end by ctrl+Z */
    if (m_http_para[httpid].connect_info.resource &&
        m_http_para[httpid].connect_info.end_mob == 1) {

        temp_size = at_ptr->upstream_data_len;
        if (m_http_para[httpid].connect_info.actual_len + temp_size >
            MOBILE_HTTP_MAX_HEADER_LEN) {
            temp_size = MOBILE_HTTP_MAX_HEADER_LEN -
                        m_http_para[httpid].connect_info.actual_len;
        }

        if (temp_size > 0) {
            aos_memcpy(m_http_para[httpid].connect_info.resource +
                       m_http_para[httpid].connect_info.actual_len,
                       at_ptr->at_upstream_data, temp_size);

            m_http_para[httpid].connect_info.actual_len += temp_size;
            last_char = m_http_para[httpid].connect_info.actual_len - 1;
        }

        if (m_http_para[httpid].connect_info.resource[last_char] ==
            MOBILE_HTTP_ESC) {
            aos_memset(m_http_para[httpid].connect_info.resource, '\0',
                       MOBILE_HTTP_MAX_RESOURCE_LEN);
            m_http_para[httpid].ready = 0;
            m_http_para[httpid].connect_info.actual_len = 0;
            aic_reply_ok_resp(at_ptr);
            at_ptr->at_mode = AT_CMD_MODE;
            return;
        }

        if (m_http_para[httpid].connect_info.resource[last_char] ==
            MOBILE_HTTP_CTRL_Z) {
            m_http_para[httpid].connect_info.resource[last_char] = '\0';
            at_ptr->at_mode = AT_CMD_MODE;
            //alog_debug("mobile http request can be send\n");
            goto URC_OUT;
        }
    }

    /* set http header by length */
    if (m_http_para[httpid].connect_info.resource &&
        m_http_para[httpid].connect_info.resource_len > 0 &&
        m_http_para[httpid].connect_info.end_mob == 0) {

        if (m_http_para[httpid].connect_info.actual_len <
            m_http_para[httpid].connect_info.resource_len) {
            temp_size = at_ptr->upstream_data_len;

            if (m_http_para[httpid].connect_info.actual_len + temp_size >
                m_http_para[httpid].connect_info.resource_len) {
                temp_size = m_http_para[httpid].connect_info.resource_len -
                            m_http_para[httpid].connect_info.actual_len;
            }

            if (temp_size != 0) {
                strncpy(m_http_para[httpid].connect_info.resource +
                        m_http_para[httpid].connect_info.actual_len,
                        at_ptr->at_upstream_data, temp_size);
                m_http_para[httpid].connect_info.actual_len += temp_size;
            }

            if (m_http_para[httpid].connect_info.actual_len >=
                m_http_para[httpid].connect_info.resource_len) {
                at_ptr->at_mode = AT_CMD_MODE;
                goto URC_OUT;
            }
        } else {
            //alog_debug("mobile http resource len enough, request can be send\n");
            goto URC_OUT;
        }
    }

    return;
URC_OUT:
    m_http_para[httpid].ready = 1;
    m_http_para[httpid].connect_info.actual_len = 0;
    aic_reply_ok_resp(at_ptr);
    return;
}

static u32 mobile_http_header_transmit(AT_REQ_RESP *at_ptr, u8 *protocol,
                                       u8 *tag, u32 value1, u32 value2,
                                       u8 *str1)
{
    u8 *result;
    ulong line_p[5];
    u32 malloc_len = 0;

    malloc_len = MOBILE_HTTP_MAX_HEADER_LEN + 32;
    result = aos_zalloc(malloc_len);

    if (tag)
        snprintf(result, malloc_len, "+%s: \"%s\"", protocol, tag);
    if (strlen(result) < malloc_len -1)
        snprintf(result + strlen(result), malloc_len, ",%d", value1);
    if (strlen(result) < malloc_len -1)
        snprintf(result + strlen(result), malloc_len, ",%d", value2);
    if (str1 && strlen(result) < malloc_len -1)
        snprintf(result + strlen(result), malloc_len, ",%s", str1);

    line_p[0] = (ulong)(result);
    aic_fill_transmit_cmd_resp(at_ptr, line_p, 1);

    aos_free(result);
    result = AIC_NULL;
    return AIC_SUCCESS;
}

static u32 mobile_http_paras_value_transmit(AT_REQ_RESP *at_ptr, u8 httpid,
                                             u8 *tag, u8 print_all)
{
    /* http header need \r\n at the end. so the header len need + 2 */
    if (print_all || strncasecmp(tag, "header", strlen(tag)) == 0) {
        if (m_http_para[httpid].header.has_data)
            mobile_http_header_transmit(at_ptr, HTTP_CFG_RESP, "header", httpid,
            m_http_para[httpid].header.actual_len,
            m_http_para[httpid].header.data);

        if (print_all && !m_http_para[httpid].header.has_data)
            mobile_http_header_transmit(at_ptr, HTTP_CFG_RESP, "header", httpid,
            0, AIC_NULL);
    }

    if (print_all || strncasecmp(tag, "chunked", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "chunked", httpid,
                                   m_http_para[httpid].chunked_mode,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "cached", strlen(tag)) == 0) {
        if (m_http_para[httpid].cached == 1)
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "cached", httpid,
                        m_http_para[httpid].cached,
                        m_http_para[httpid].cached_data_info.cache_len,
                        MOBILE_RESERVED_VALUE);
        else
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "cached", httpid,
                        m_http_para[httpid].cached, MOBILE_RESERVED_VALUE,
                        MOBILE_RESERVED_VALUE);
    }

    if (print_all || strncasecmp(tag, "timeout", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "timeout", httpid,
                                   m_http_para[httpid].conn_timeout / 1000,
                                   m_http_para[httpid].rsp_timeout / 1000,
                                   m_http_para[httpid].input_timeout / 1000);

    if (print_all || strncasecmp(tag, "encoding", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "encoding", httpid,
                                   m_http_para[httpid].input_format,
                                   m_http_para[httpid].out_format,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "ssl", strlen(tag)) == 0) {
        if (m_http_para[httpid].ssl_enable > 0)
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "ssl", httpid,
                                       m_http_para[httpid].ssl_enable,
                                       m_http_para[httpid].ssl_id,
                                       MOBILE_RESERVED_VALUE);
        else
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "ssl", httpid,
                                       m_http_para[httpid].ssl_enable,
                                       MOBILE_RESERVED_VALUE,
                                       MOBILE_RESERVED_VALUE);
    }

    if (print_all || strncasecmp(tag, "cid", strlen(tag)) == 0) {
        if (m_http_para[httpid].cid == MOBILE_HTTP_INVALID_ID)
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "cid", httpid,
                                               MOBILE_RESERVED_VALUE,
                                               MOBILE_RESERVED_VALUE,
                                               MOBILE_RESERVED_VALUE);
        else
            transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "cid", httpid,
                                       m_http_para[httpid].cid,
                                       MOBILE_RESERVED_VALUE,
                                       MOBILE_RESERVED_VALUE);
    }

    if (print_all || strncasecmp(tag, "fragment", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "fragment", httpid,
                                   m_http_para[httpid].frag_size,
                                   m_http_para[httpid].interval,
                                   MOBILE_RESERVED_VALUE);

    if (print_all || strncasecmp(tag, "urlencode", strlen(tag)) == 0)
        transmit_mobile_at_integer(at_ptr, HTTP_CFG_RESP, "urlencode", httpid,
                                   m_http_para[httpid].urlencode_mode,
                                   MOBILE_RESERVED_VALUE,
                                   MOBILE_RESERVED_VALUE);

    return AIC_SUCCESS;
}

static u32 mobile_http_dns_process(u8 cid, u8 httpid)
{
    struct addrinfo hints = {'\0'};
    int status = 0;
    struct addrinfo *res;
    u8 *v6_str;
    u8 dns_prio = 0;
    u8 pdp_type = 0;

    dns_prio = aic_get_dns_prio();
    pdp_type = aic_get_pdp_ip_type(cid);

    //alog_debug("mobile_http_dns_process cid is %d ;type is %d; dns_prio is %d\n",
               //cid, pdp_type, dns_prio);

    if (dns_prio == MOBILE_DNS_PRIO_V4) {
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V4 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6)
            hints.ai_family = AF_INET;
        else if (ANET_DEFAULT_IP_TYPE_V6)
            hints.ai_family = AF_INET6;
    } else if (dns_prio == MOBILE_DNS_PRIO_V6) {
        if (pdp_type == ANET_DEFAULT_IP_TYPE_V6 ||
            pdp_type == ANET_DEFAULT_IP_TYPE_V4V6)
            hints.ai_family = AF_INET6;
        else if (ANET_DEFAULT_IP_TYPE_V4)
            hints.ai_family = AF_INET;
    }

    if (mobile_http_ipv4_addr_check(m_http_para[httpid].connect_info.host))
        hints.ai_family = AF_INET;

    if (strncmp(m_http_para[httpid].connect_info.host, "[", 1) == 0) {
        v6_str = (u8 *)aos_zalloc(128);
        hints.ai_family = AF_INET6;

        strncpy(v6_str, m_http_para[httpid].connect_info.host + 1, 128);
        v6_str[strlen(v6_str) - 1] = '\0';
        //alog_debug("v6_str is %s\n", v6_str);
        status = getaddrinfo(v6_str, NULL, &hints, &res);
        aos_free(v6_str);
        v6_str = AIC_NULL;
    } else {
        aicx_error(0x0e9518, "s", "mobile http host is %s\n", m_http_para[httpid].connect_info.host);
        status = getaddrinfo(m_http_para[httpid].connect_info.host, NULL,
                             &hints, &res);
    }

    if (status == 0 && res) {
        http_server_ip_set(&m_http_para[httpid].connect_info.server_ip, res);
    } else if (status) {
        hints.ai_family = AF_UNSPEC;

        status = getaddrinfo(m_http_para[httpid].connect_info.host, NULL,
                             &hints, &res);
        if (status == 0 && res) {
            http_server_ip_set(&m_http_para[httpid].connect_info.server_ip, res);
        } else {
            return MOBILE_HTTP_URC_URL_PARSING_FAILED;
        }
    }

    if (res)
        freeaddrinfo(res);

    if (cid >= 1 && cid <= ANET_MAX_PDP_CID) {
        status = aic_http_bind_source_ip(m_http_para[httpid].client_ptr, cid,
                                         &m_http_para[httpid].connect_info.server_ip,
                                         MOBILE_HTTP_DEFAULT_WAIT_TIME);
        if (status != AIC_SUCCESS) {
            return MOBILE_HTTP_URC_URL_PARSING_FAILED;
        }
    }

    return AIC_SUCCESS;
}

static void mobile_http_header_compare(u8 httpid)
{
    int i, j;

    for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
        for(j = 0; j < MOBILE_HTTP_MAX_HEADER_NUM; j++) {
            if (m_http_para[httpid].comm_k_v[i].key &&
                m_http_para[httpid].sp_k_v[j].key) {

                if (strncmp(m_http_para[httpid].comm_k_v[i].key,
                    m_http_para[httpid].sp_k_v[j].key,
                    strlen(m_http_para[httpid].comm_k_v[i].key)) == 0) {
                    m_http_para[httpid].comm_k_v[i].is_same = 1;
                } else {
                    m_http_para[httpid].comm_k_v[i].is_same = 0;
                }
            }
        }
    }

    return;
}

static u32 mobile_http_custom_header_add(NX_WEB_HTTP_CLIENT *client_ptr,
                                         u8 httpid)
{
    u32 status = AIC_SUCCESS;
    int i;

    mobile_http_header_compare(httpid);

    for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
        if (m_http_para[httpid].sp_k_v[i].key &&
            m_http_para[httpid].sp_k_v[i].value) {
            status = aic_http_client_specific_header_add(client_ptr,
                     m_http_para[httpid].sp_k_v[i].key,
                     m_http_para[httpid].sp_k_v[i].value,
                     MOBILE_HTTP_DEFAULT_WAIT_TIME);
        }
    }

    for (i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
        if (m_http_para[httpid].comm_k_v[i].key &&
            m_http_para[httpid].comm_k_v[i].value &&
            !m_http_para[httpid].comm_k_v[i].is_same) {
            status = aic_http_client_specific_header_add(client_ptr,
                     m_http_para[httpid].comm_k_v[i].key,
                     m_http_para[httpid].comm_k_v[i].value,
                     MOBILE_HTTP_DEFAULT_WAIT_TIME);
        }
    }

    return status;
}

static u32 mobile_http_request_send(u8 httpid)
{
    u32 status;
    AIC_HTTPS_CLIENT_INSTANCE *client_ptr;
    u8 *temp;

    client_ptr = (AIC_HTTPS_CLIENT_INSTANCE *)m_http_para[httpid].handle;
    if (aic_http_input_paras_check(client_ptr))
        return AIC_HTTPS_PTR_ERROR;

    client_ptr->https_client->nx_web_http_client_connect_port =
                                        m_http_para[httpid].connect_info.port;

    aic_http_host_check(client_ptr, m_http_para[httpid].connect_info.host,
                        MOBILE_HTTP_DEFAULT_WAIT_TIME);
    if (m_http_para[httpid].cid == MOBILE_HTTP_INVALID_ID)
        status = mobile_http_dns_process(1, httpid);
    else
        status = mobile_http_dns_process(m_http_para[httpid].cid, httpid);

    if (status)
        return status;

#ifdef CONFIG_NET_HTTPS
    if (m_http_para[httpid].ssl_enable && client_ptr->https_client->http_mb_cfg)
        status = nx_web_http_client_secure_connect(client_ptr->https_client,
                 &m_http_para[httpid].connect_info.server_ip,
                 m_http_para[httpid].connect_info.port, https_tls_setup,
                 MOBILE_HTTP_DEFAULT_WAIT_TIME);
    else
#endif
        status = nx_web_http_client_connect(client_ptr->https_client,
                 &m_http_para[httpid].connect_info.server_ip,
                 m_http_para[httpid].connect_info.port,
                 MOBILE_HTTP_DEFAULT_WAIT_TIME);
    if (status != NX_SUCCESS)
        return status;

    if (m_http_para[httpid].urlencode_mode) {
        temp = aos_zalloc(m_http_para[httpid].connect_info.resource_len + 1);
        aos_memcpy(temp, m_http_para[httpid].connect_info.resource,
                   m_http_para[httpid].connect_info.resource_len);

        aos_memset(m_http_para[httpid].connect_info.resource, '\0',
                   m_http_para[httpid].connect_info.resource_len);

        mobile_http_url_escape(temp, m_http_para[httpid].connect_info.resource_len,
                               m_http_para[httpid].connect_info.resource);
        aos_free(temp);
    }

    aicx_error(0x1ac352, "s", "mobile http resource is %s\n",
               m_http_para[httpid].connect_info.resource);

    /* Initialize HTTP request. */
    if (m_http_para[httpid].method == MOBILE_HTTP_METHOD_PUT ||
        m_http_para[httpid].method == MOBILE_HTTP_METHOD_POST) {
        status = nx_web_http_client_request_initialize(client_ptr->https_client,
                             m_http_para[httpid].method,
                             m_http_para[httpid].connect_info.resource,
                             m_http_para[httpid].connect_info.host,
                             m_http_para[httpid].s_content.actual_len,
                             m_http_para[httpid].chunked_mode,
                             AIC_NULL, AIC_NULL, MOBILE_HTTP_DEFAULT_WAIT_TIME);

            if (m_http_para[httpid].s_content.actual_len == 0)
                aic_http_client_specific_header_add(client_ptr->https_client,
                      "Content-Length", "0", MOBILE_HTTP_DEFAULT_WAIT_TIME);
        } else {
            status = nx_web_http_client_request_initialize(client_ptr->https_client,
                             m_http_para[httpid].method,
                             m_http_para[httpid].connect_info.resource,
                             m_http_para[httpid].connect_info.host,
                             0, m_http_para[httpid].chunked_mode,
                             AIC_NULL, AIC_NULL, MOBILE_HTTP_DEFAULT_WAIT_TIME);
        }

    if (status != NX_SUCCESS)
        return status;

    status = mobile_http_custom_header_add(client_ptr->https_client, httpid);
    if (status)
        return status;

    status = nx_web_http_client_request_send(client_ptr->https_client,
                                             MOBILE_HTTP_DEFAULT_WAIT_TIME);

    return status;
}

static u8 mobile_http_using_httpid_find(NX_WEB_HTTP_CLIENT *http_client)
{
    u8 i;

    for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
        if (m_http_para[i].client_ptr &&
            m_http_para[i].client_ptr->https_client == http_client)
            break;
    }

    return i;
}

static void mobile_http_header_res_urc_cb(NX_WEB_HTTP_CLIENT *http_client,
                                          char* data, u32 header_len)
{
    u8 *res_buf = AIC_NULL;
    u8 *res_hex_buf = AIC_NULL;
    u8 httpid = MOBILE_HTTP_INVALID_ID;
    u8 res_header[MOBILE_HTTP_URC_HEADER_LEN] = {'\0'};
    u32 offset = 0;
    u32 frag_size = 0;

    httpid = mobile_http_using_httpid_find(http_client);
    if (httpid >= MOBILE_HTTP_MAX_CLIENT_NUM) {
        return;
    }

    /* cm does not need \r\n\r\n at the end. delete 2 char */
    if (header_len > 4) {
        if (strncmp(&data[header_len - 4], "\r\n\r\n", 4) == 0)
            header_len -= 2;
    }

    aos_memcpy(m_http_para[httpid].client_ptr->status_code, data + 9, 3);

    /* cached mode */
    if (m_http_para[httpid].cached && m_http_para[httpid].cached_data_info.cached_header) {
        if (header_len > m_http_para[httpid].cached_data_info.cache_len) {
            m_http_para[httpid].cached_data_info.header_len = header_len;
            m_http_para[httpid].read_data_info.unread_header_len = 0;
            return;
        }

        aos_memcpy(m_http_para[httpid].cached_data_info.cached_header,
                   data, header_len);
        m_http_para[httpid].cached_data_info.header_len = header_len;
        m_http_para[httpid].read_data_info.unread_header_len = header_len;
    } else {
        /* urc mode */
#ifdef AIC_HTTP_REDIR
        if (http_client->redir_flag == 1)
            return;
#endif

        if (m_http_para[httpid].out_format == MOBILE_HTTP_HEX) {
            res_buf = aos_zalloc(header_len * 2 + MOBILE_HTTP_URC_HEADER_LEN);
            res_hex_buf = aos_zalloc(header_len * 2 + 2);

            frag_size = m_http_para[httpid].frag_size;
            aic_print_data_to_hex(data, header_len, res_hex_buf);

            if (frag_size == 0) {
                snprintf(res_buf, header_len * 2 + MOBILE_HTTP_URC_HEADER_LEN,
                         HTTP_URC_STR"\"header\",%d,%s,%d,%s", httpid,
                         m_http_para[httpid].client_ptr->status_code,
                         header_len, res_hex_buf);

                aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res_buf,
                                        strlen(res_buf));
            } else {
                do {
                    aos_memset(res_buf, '\0', header_len * 2 + MOBILE_HTTP_URC_HEADER_LEN);

                    if (m_http_para[httpid].is_term)
                        break;

                    if (header_len * 2 - offset < frag_size)
                        frag_size = header_len - (offset >> 1);

                    snprintf(res_header, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_URC_STR"\"header\",%d,%s,%d,", httpid,
                             m_http_para[httpid].client_ptr->status_code,
                             frag_size);

                    aos_memcpy(res_buf, res_header, strlen(res_header));
                    aos_memcpy(res_buf + strlen(res_header),
                               res_hex_buf + offset, 2 * frag_size);

                    aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res_buf,
                                            strlen(res_buf));

                    offset += (2 * frag_size);

                    if (m_http_para[httpid].interval)
                        aos_sleep(m_http_para[httpid].interval);

                } while (offset < header_len * 2);
            }
        } else {
            res_buf = aos_zalloc(header_len + MOBILE_HTTP_URC_HEADER_LEN);
            frag_size = m_http_para[httpid].frag_size;

            if (frag_size == 0) {
                snprintf(res_header, MOBILE_HTTP_URC_HEADER_LEN,
                         HTTP_URC_STR"\"header\",%d,%s,%d,", httpid,
                         m_http_para[httpid].client_ptr->status_code, header_len);

                aos_memcpy(res_buf, res_header, strlen(res_header));
                aos_memcpy(res_buf + strlen(res_header), data, header_len);

                aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res_buf,
                                        strlen(res_buf));
            } else {
                do {
                    aos_memset(res_buf, '\0', header_len + MOBILE_HTTP_URC_HEADER_LEN);
                    if (m_http_para[httpid].is_term)
                        break;

                    if (frag_size == 0 || header_len - offset < frag_size)
                        frag_size = header_len - offset;

                    snprintf(res_header, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_URC_STR"\"header\",%d,%s,%d,", httpid,
                             m_http_para[httpid].client_ptr->status_code,
                             frag_size);

                    aos_memcpy(res_buf, res_header, strlen(res_header));
                    aos_memcpy(res_buf + strlen(res_header), data + offset,
                               frag_size);

                    aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res_buf,
                                            strlen(res_buf));

                    offset += frag_size;

                    if (m_http_para[httpid].interval)
                        aos_sleep(m_http_para[httpid].interval);

                } while (offset < header_len);
            }
        }

        aos_free(res_buf);
        if (res_hex_buf)
            aos_free(res_hex_buf);
    }

    return;
}

void mobile_http_content_range_get_cb(NX_WEB_HTTP_CLIENT *http_client,
                                      char *field_name, u32 field_name_len,
                                      char *field_value, u32 field_value_len)
{
    u8 name[100] = {'\0'};
    u8 value[100] = {'\0'};
    u8 httpid;
    u8 *found;

    httpid = mobile_http_using_httpid_find(http_client);
    if (httpid >= MOBILE_HTTP_MAX_CLIENT_NUM) {
        return;
    }

    if (field_name_len >= sizeof(name))
        return;

    aos_memcpy(name, field_name, field_name_len);
    if (strncasecmp(name, "Content-Range", 13) == 0) {
        aos_memcpy(value, field_value, field_value_len);

        found = strchr(value, '/');
        if (found) {
            found++;
            m_http_para[httpid].content_range = atoi(found);
        }
    }

    return;
}

#ifdef CONFIG_FS
static void mobile_http_response_save(u32 urc_port, u8 *local_path, u8 httpid,
                                      u8 eof, u32 progind)
{
    long data_read;
    u8 res[MOBILE_HTTP_URC_HEADER_LEN] = {'\0'};
    int fd = 0;
    int status = 0;
    int w_size = 0;
    slong total_size = 0;
    slong dl_size = 0;
    u32 percent = 0;
    ulong progind_size = 0;
    u8 chunked_mode = 0;
    u8 no_content_len = 0;
    u32 file_write_count = 0;
    u32 dl_count = 0;
    u8 *temp_data_buf = AIC_NULL;
    u32 offset = 0;

    m_http_para[httpid].cached_data_info.actual_content_len = 0;
    temp_data_buf = aos_zalloc_relaxed(SIZE_8K + SIZE_2K);
    if (!temp_data_buf)
        return;

    fd = open(local_path, FS_O_WRONLY | FS_O_CREATE);
    if (fd < 0) {
        status = fd;
        goto DL_ERROR;
    }

    if (eof == 0) {
       status = ftruncate(fd, 0);
       if (status < 0)
            goto FD_ERROR;
    } else if (eof == 1) {
        status = lseek(fd, 0, SEEK_END);
        if (status < 0)
            goto FD_ERROR;
    }

    do {
#ifdef AIC_HTTP_REDIR
        data_read = aic_https_client_redir_response_data_get(
                    m_http_para[httpid].handle, temp_data_buf + offset,
                    MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN, MOBILE_HTTP_DL_TIMEOUT);
#else
        data_read = aic_https_client_response_data_get(
                    m_http_para[httpid].handle,
                    temp_data_buf + offset, MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                    MOBILE_HTTP_DL_TIMEOUT);
#endif
        //alog_info("mobile http get dl data_read is %d\n", data_read);
        dl_count++;
        if (data_read <= 0 && data_read != AIC_HTTPS_CHUNKED_MODE_DATA_GET_DONE) {
            break;
        } else {
            if (data_read == AIC_HTTPS_CHUNKED_MODE_DATA_GET_DONE) {
                if (offset && temp_data_buf) {
                    w_size = write(fd, temp_data_buf, offset);
                    if (w_size < 0) {
                        status = w_size;
                        goto FD_ERROR;
                    } else {
                        status = lseek(fd, 0, SEEK_END);
                        if (status < 0)
                            goto FD_ERROR;
                    }
                    offset = 0;
                }
                goto CHUNKED_END;
            }

            /* only first dl time need to get total size */
            if (dl_count == 1) {
                chunked_mode = aic_http_client_chunked_mode_check(m_http_para[httpid].handle);
                if (chunked_mode)
                    no_content_len = 1;

                if (no_content_len)
                    total_size = 0;
                else
                    total_size = aic_https_client_total_receive_bytes_get(
                                 m_http_para[httpid].handle);
            }

            offset += data_read;
            dl_size += data_read;
            progind_size += data_read;

            if (offset >= SIZE_8K || (dl_size != 0 && dl_size == total_size)) {
                w_size = write(fd, temp_data_buf, offset);
                if (w_size < 0) {
                    status = w_size;
                    goto FD_ERROR;
                } else {
                    status = lseek(fd, 0, SEEK_END);
                    if (status < 0)
                        goto FD_ERROR;
                }
                file_write_count++;
                offset = 0;
            }

CHUNKED_END:
            chunked_mode = aic_http_client_chunked_mode_check(m_http_para[httpid].handle);
            if (progind) {
                if (no_content_len) {
                    if (chunked_mode) {
                        percent = 0;
                    }
                    else
                        percent = 100;
                } else {
                    percent = dl_size * 100 / total_size;
                }

                if (m_http_para[httpid].content_range > 0 &&
                    (progind_size >= progind || percent == 100)) {
                    if (percent == 100) {
                        status = close(fd);
                        if (status < 0)
                            goto DL_ERROR;
                        else
                            fd = 0;
                    }

                    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_DL_FILE_RESP"%ld,%ld,%d,%ld", dl_size,
                             total_size, percent,
                             m_http_para[httpid].content_range);
                    progind_size = 0;
                    aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
                    if (file_write_count > 10) {
                        //sleep , let wdt get chance to run.
                        aos_sleep(20);
                        file_write_count = 0;
                    }
                } else if (m_http_para[httpid].content_range == 0 &&
                           (progind_size >= progind || percent == 100)) {
                        /* close fd immediately when download success */
                    if (percent == 100) {
                        status = close(fd);
                        if (status < 0)
                            goto DL_ERROR;
                        else
                            fd = 0;
                    }

                    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_DL_FILE_RESP"%ld,%ld,%d", dl_size,
                             total_size, percent);
                    progind_size = 0;
                    aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
                    if (file_write_count > 10) {
                        //sleep , let wdt get chance to run.
                        aos_sleep(20);
                        file_write_count = 0;
                    }
                }
            }
        }
    } while (fd > 0 && (dl_size < total_size || chunked_mode));

    if (!progind) {
        if (m_http_para[httpid].dl_mod == 1) {
            snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                     HTTP_URC_STR"\"download\",%ld,%ld", dl_size, total_size);
        } else {
            if (total_size > 0) {
                if (m_http_para[httpid].content_range > 0)
                    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_DL_FILE_RESP"%ld,%ld,%ld,%ld",
                             dl_size, total_size, dl_size * 100 / total_size,
                             m_http_para[httpid].content_range);
                else
                    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                             HTTP_DL_FILE_RESP"%ld,%ld,%ld",
                             dl_size, total_size, dl_size * 100 / total_size);
            }
        }
        aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
    }

    aos_free(temp_data_buf);
    if (fd > 0)
        close(fd);
    return;

FD_ERROR:
    if (fd > 0)
        close(fd);

DL_ERROR:
    aicx_error(0x06de19, "x", "m_http dl file fail(0x%x)\n", status);
    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_URC_STR"\"err\",%d,%d",
             httpid, MOBILE_HTTP_URC_WRITE_FILE_FAILED);

    aic_transmit_at_cmd_urc(urc_port, res, strlen(res));

    aos_free(temp_data_buf);
    return;
}
#endif

static void mobile_http_resp_cached_process (u8 httpid)
{
    long data_read = 0;
    u8 *res = AIC_NULL;
    u8 *drop_buf = AIC_NULL;

    res = aos_zalloc(64);
    m_http_para[httpid].cached_data_info.actual_content_len = 0;
    mobile_http_cached_data_mem_alloc(httpid);

    //to get content len and header len. no need redir.
    data_read = aic_https_client_response_data_get(m_http_para[httpid].handle,
                m_http_para[httpid].cached_data_info.cached_data,
                1, MOBILE_HTTP_DEFAULT_WAIT_TIME);

    if (data_read <= 0) {
        goto ERROR_OUT;
    } else {
        m_http_para[httpid].cached_data_info.actual_content_len += data_read;

        m_http_para[httpid].cached_data_info.content_len =
            m_http_para[httpid].client_ptr->https_client->nx_web_http_client_total_receive_bytes;
    }

    // no enough cached buff
    if (m_http_para[httpid].cached_data_info.content_len +
        m_http_para[httpid].cached_data_info.header_len >
        m_http_para[httpid].cached_data_info.cache_len) {

        drop_buf = aos_zalloc(MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN);

        //get remaining content data and drop them.no need redir.
        do {
            data_read = aic_https_client_response_data_get(m_http_para[httpid].handle,
                        drop_buf, MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                        MOBILE_HTTP_DEFAULT_WAIT_TIME);
        } while (data_read > 0);

        aos_free(drop_buf);
        snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_URC_STR"\"err\",%d,%d", httpid,
                 MOBILE_HTTP_URC_NO_CACHE_SPACE);

        if (m_http_para[httpid].cached_data_info.cached_header) {
            aos_free(m_http_para[httpid].cached_data_info.cached_header);
            m_http_para[httpid].cached_data_info.cached_header = AIC_NULL;
        }
        m_http_para[httpid].cached_data_info.header_len = 0;
        m_http_para[httpid].read_data_info.unread_header_len = 0;

        if(m_http_para[httpid].cached_data_info.cached_data) {
            aos_free(m_http_para[httpid].cached_data_info.cached_data);
            m_http_para[httpid].cached_data_info.cached_data = AIC_NULL;
        }
        m_http_para[httpid].cached_data_info.content_len = 0;
        m_http_para[httpid].cached_data_info.actual_content_len = 0;

        goto OUT;
    }

    //to get remaining content data
    do {
#ifdef AIC_HTTP_REDIR
        data_read = aic_https_client_redir_response_data_get(m_http_para[httpid].handle,
                    m_http_para[httpid].cached_data_info.cached_data +
                    m_http_para[httpid].cached_data_info.actual_content_len,
                    MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                    MOBILE_HTTP_DL_TIMEOUT);
#else
        data_read = aic_https_client_response_data_get(m_http_para[httpid].handle,
                    m_http_para[httpid].cached_data_info.cached_data +
                    m_http_para[httpid].cached_data_info.actual_content_len,
                    MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                    MOBILE_HTTP_DL_TIMEOUT);
#endif

        if (data_read <= 0) {
            snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                     HTTP_URC_STR"\"recv\",%d,%s,%d,%d", httpid,
                     m_http_para[httpid].client_ptr->status_code,
                     m_http_para[httpid].cached_data_info.header_len,
                     m_http_para[httpid].cached_data_info.content_len);
            goto OUT;
        } else {
            m_http_para[httpid].cached_data_info.actual_content_len += data_read;
            m_http_para[httpid].read_data_info.unread_content_len =
                m_http_para[httpid].cached_data_info.actual_content_len;

            snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                     HTTP_URC_STR"\"recv\",%d,%s,%d,%d", httpid,
                     m_http_para[httpid].client_ptr->status_code,
                     m_http_para[httpid].cached_data_info.header_len,
                     m_http_para[httpid].cached_data_info.content_len);
        }
    } while (m_http_para[httpid].cached_data_info.actual_content_len <
             m_http_para[httpid].cached_data_info.cache_len);

OUT:
    aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res, strlen(res));

    aos_free(res);
    return;

ERROR_OUT:
    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_URC_STR"\"err\",%d,%d",
             httpid, MOBILE_HTTP_URC_RESPONSE_TIMEOUT);

    aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res, strlen(res));

    aos_free(res);
    return;
}

static void mobile_http_resp_urc_process(u8 httpid)
{
    long data_read = 0;
    u32 offset_len = 0;
    u8 chunked_mode = 0;
    u8 no_content_len = 0;
    u8 *res = AIC_NULL;
    u32 frag_size = 0;
    u8 out_format = 0;
    u32 status = 0;

    res = aos_zalloc(MOBILE_HTTP_ONE_RESPONSE_HEX_DATA_BUF_LEN +
                     MOBILE_HTTP_URC_HEADER_LEN);

    out_format = m_http_para[httpid].out_format;
    m_http_para[httpid].has_set_fragment = 1;
    do {
        if (m_http_para[httpid].frag_size == 0)
            frag_size = MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN;
        else
            frag_size = m_http_para[httpid].frag_size;

        status =  mobile_mem_alloc(&m_http_para[httpid].res_data.res_data_buf,
                                   (MOBILE_HTTP_MAX_RES_URC_DATA_LEN + MOBILE_HTTP_URC_HEADER_LEN));
        if (status)
            goto OUT;

#ifdef AIC_HTTP_REDIR
        data_read = aic_https_client_redir_response_data_get(
                    m_http_para[httpid].handle,
                    m_http_para[httpid].res_data.res_data_buf,
                    frag_size, MOBILE_HTTP_DL_TIMEOUT);
#else
        data_read = aic_https_client_response_data_get(
                    m_http_para[httpid].handle,
                    m_http_para[httpid].res_data.res_data_buf,
                    frag_size, MOBILE_HTTP_DL_TIMEOUT);
#endif

        chunked_mode = aic_http_client_chunked_mode_check(m_http_para[httpid].handle);
        if (chunked_mode)
            no_content_len = 1;

        if (m_http_para[httpid].method == MOBILE_HTTP_METHOD_HEAD)
            goto OUT;

        if (data_read <= 0) {
            if (no_content_len && m_http_para[httpid].res_data.sum_len > 0 &&
                !m_http_para[httpid].is_term) {
                snprintf(res, MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                         HTTP_URC_STR"\"content\",%d,%d,%d,%d",
                         httpid, 0, m_http_para[httpid].res_data.sum_len, 0);

                aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res,
                                        strlen(res));
            } else if (m_http_para[httpid].client_ptr->status_code[0] == 0 &&
                       m_http_para[httpid].res_data.content_len == 0) {
                snprintf(res, MOBILE_HTTP_ONE_RESPONSE_DATA_BUF_LEN,
                         HTTP_URC_STR"\"err\",%d,%d",
                         httpid, MOBILE_HTTP_URC_DISCONNECTED_ABNORMAL);

                aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port, res,
                                        strlen(res));
            }

            goto OUT;
        } else {
            if (no_content_len)
                m_http_para[httpid].res_data.content_len = 0;
            else
                m_http_para[httpid].res_data.content_len =
                    m_http_para[httpid].client_ptr->https_client->nx_web_http_client_total_receive_bytes;

            if (!m_http_para[httpid].urc_interval &&
                m_http_para[httpid].res_data.content_len > MOBILE_HTTP_URC_FLOWCTRL_VALUE)
                m_http_para[httpid].urc_interval = MOBILE_HTTP_URC_DELAY_VALUE;

            m_http_para[httpid].res_data.sum_len += data_read;
            m_http_para[httpid].res_data.cur_len = data_read;

            if (out_format == MOBILE_HTTP_HEX &&
                strlen(m_http_para[httpid].res_data.res_data_buf)) {

                status = mobile_mem_alloc(&m_http_para[httpid].res_data.res_hex_buf,
                                          (MOBILE_HTTP_ONE_RESPONSE_HEX_DATA_BUF_LEN + MOBILE_HTTP_URC_HEADER_LEN));
                if (status)
                    goto OUT;

                aic_print_data_to_hex(m_http_para[httpid].res_data.res_data_buf,
                                    data_read,
                                    m_http_para[httpid].res_data.res_hex_buf);

                if (strlen(m_http_para[httpid].res_data.res_hex_buf)) {
                    if (conn_get_uart_at_conid == m_http_para[httpid].urc_port) {
                        res[0] = aic_get_cr_sign();
                        res[1] = aic_get_lf_sign();

                        snprintf(res + 2, MOBILE_HTTP_URC_HEADER_LEN,
                                 HTTP_URC_STR"\"content\",%d,%d,%d,%d,",
                                 httpid, m_http_para[httpid].res_data.content_len,
                                 m_http_para[httpid].res_data.sum_len,
                                 m_http_para[httpid].res_data.cur_len);

                        offset_len = strlen(res);

                        aos_memcpy(res + offset_len,
                                   m_http_para[httpid].res_data.res_hex_buf,
                                   data_read * 2);

                        res[data_read * 2 + offset_len] = aic_get_cr_sign();
                        res[data_read * 2 + offset_len + 1] = aic_get_lf_sign();

                        if (m_http_para[httpid].is_term)
                            continue;

                        aic_send_to_at_port(m_http_para[httpid].urc_port,
                                            res, (data_read * 2 + offset_len + 2));

                    } else {
                        snprintf(res, MOBILE_HTTP_ONE_RESPONSE_HEX_DATA_BUF_LEN,
                                 HTTP_URC_STR"\"content\",%d,%d,%d,%d,",
                                 httpid, m_http_para[httpid].res_data.content_len,
                                 m_http_para[httpid].res_data.sum_len,
                                 m_http_para[httpid].res_data.cur_len);

                        offset_len = strlen(res);

                        aos_memcpy(res + offset_len,
                                   m_http_para[httpid].res_data.res_hex_buf,
                                   data_read * 2);

                        if (m_http_para[httpid].is_term)
                            continue;

                        aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port,
                                                res, (offset_len + data_read * 2));
                    }
                }
            } else if (out_format == MOBILE_HTTP_ASCII) {
                if (conn_get_uart_at_conid == m_http_para[httpid].urc_port) {
                    res[0] = aic_get_cr_sign();
                    res[1] = aic_get_lf_sign();

                    snprintf(res + 2, MOBILE_HTTP_MAX_RES_URC_DATA_LEN,
                             HTTP_URC_STR"\"content\",%d,%d,%d,%d,",
                             httpid, m_http_para[httpid].res_data.content_len,
                             m_http_para[httpid].res_data.sum_len,
                             m_http_para[httpid].res_data.cur_len);

                    offset_len = strlen(res);

                    aos_memcpy(res + offset_len,
                               m_http_para[httpid].res_data.res_data_buf,
                               data_read);

                    res[data_read + offset_len] = aic_get_cr_sign();
                    res[data_read + offset_len + 1] = aic_get_lf_sign();

                    if (m_http_para[httpid].is_term)
                        continue;

                    aic_send_to_at_port(m_http_para[httpid].urc_port,
                                        res, (data_read + offset_len + 2));
                } else {
                    snprintf(res, MOBILE_HTTP_MAX_RES_URC_DATA_LEN,
                             HTTP_URC_STR"\"content\",%d,%d,%d,%d,",
                             httpid, m_http_para[httpid].res_data.content_len,
                             m_http_para[httpid].res_data.sum_len,
                             m_http_para[httpid].res_data.cur_len);

                    offset_len = strlen(res);

                    aos_memcpy(res + offset_len,
                               m_http_para[httpid].res_data.res_data_buf,
                               data_read);

                    if (m_http_para[httpid].is_term)
                        continue;

                    aic_transmit_at_cmd_urc(m_http_para[httpid].urc_port,
                                            res, (offset_len + data_read));
                }
            }

            if (m_http_para[httpid].interval)
                aos_sleep(m_http_para[httpid].interval);
            else if (m_http_para[httpid].urc_interval)
                aos_sleep(m_http_para[httpid].urc_interval);
        }
    } while (m_http_para[httpid].res_data.sum_len <
             m_http_para[httpid].res_data.content_len || chunked_mode == 1);

OUT:
    if (res) {
        aos_free(res);
        res = AIC_NULL;
    }
    m_http_para[httpid].has_set_fragment = 0;
    m_http_para[httpid].urc_interval = 0;
    mobile_http_response_hex_data_mem_release(httpid);
    return;
}

static void mobile_http_response_process(u8 httpid, u32 urc_port)
{
    u32 status = AIC_SUCCESS;
    u8 *res = AIC_NULL;

    if ((m_http_para[httpid].method == MOBILE_HTTP_METHOD_PUT ||
        m_http_para[httpid].method == MOBILE_HTTP_METHOD_POST) &&
        m_http_para[httpid].chunked_mode == 0) {

        if (m_http_para[httpid].s_content.data) {
            if(m_http_para[httpid].s_content.actual_len) {
                status = aic_https_client_put_data(m_http_para[httpid].handle,
                         m_http_para[httpid].s_content.data,
                         m_http_para[httpid].s_content.actual_len,
                         MOBILE_HTTP_DEFAULT_WAIT_TIME);
            }

            m_http_para[httpid].s_content.actual_len = 0;
            m_http_para[httpid].s_content.offset = 0;
            if (status) {
                aicx_error(0x1a3855, "x", "m_http put data fail(0x%x)\n", status);

                res = aos_zalloc(MOBILE_HTTP_URC_HEADER_LEN);
                snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                         HTTP_URC_STR"\"err\",%d,%d", httpid,
                         MOBILE_HTTP_URC_RESPONSE_TIMEOUT);

                aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
                aos_free(res);
                goto OUT;
            }
        } else {
            m_http_para[httpid].s_content.actual_len = 0;
            m_http_para[httpid].s_content.offset = 0;
            goto OUT;
        }
    }

#ifdef CONFIG_FS
    /* dl mode */
    if (m_http_para[httpid].dl_mod && m_http_para[httpid].local_path) {
        mobile_http_response_save(urc_port, m_http_para[httpid].local_path,
                                  httpid, 0, 0);
        mobile_http_local_path_mem_release(httpid);
        m_http_para[httpid].dl_mod = 0;
        goto OUT;
    }
#endif

    /* cached mode */
    if (m_http_para[httpid].cached) {
        mobile_http_resp_cached_process(httpid);
    } else {
        /* urc mode. Receive response data from the server. */
        mobile_http_resp_urc_process(httpid);
    }

OUT:
    return;
}

static void mobile_http_response_header_read(AT_REQ_RESP *at_ptr, u8 httpid)
{
    u8 *res;

    res = aos_zalloc(MOBILE_HTTP_URC_HEADER_LEN);

    if (!m_http_para[httpid].cached_data_info.cached_header ||
        strlen(m_http_para[httpid].cached_data_info.cached_header) == 0) {

        snprintf(m_http_para[httpid].read_data_info.read_header_buf,
                 m_http_para[httpid].read_data_info.to_read_len +
                 MOBILE_HTTP_URC_HEADER_LEN, HTTP_READ_RESP"%d,%d,%d,%d", httpid,
                 MOBILE_HTTP_HEADER_DATA, 0, 0);

        transmit_mobile_resp_str(at_ptr,
                                 m_http_para[httpid].read_data_info.read_header_buf);
        aos_free(res);
        return;
    }

    if (m_http_para[httpid].read_data_info.unread_header_len > 0) {
        if (m_http_para[httpid].read_data_info.to_read_len <=
            m_http_para[httpid].read_data_info.unread_header_len) {

            m_http_para[httpid].read_data_info.cur_read_len =
                m_http_para[httpid].read_data_info.to_read_len;
        } else {
            m_http_para[httpid].read_data_info.cur_read_len =
                m_http_para[httpid].read_data_info.unread_header_len;
        }

        m_http_para[httpid].read_data_info.unread_header_len -=
            m_http_para[httpid].read_data_info.cur_read_len;

        snprintf(res, MOBILE_HTTP_URC_HEADER_LEN,
                 HTTP_READ_RESP"%d,%d,%d,%d", httpid,
                 MOBILE_HTTP_HEADER_DATA,
                 m_http_para[httpid].read_data_info.unread_header_len,
                 m_http_para[httpid].read_data_info.cur_read_len);

        /* snprintf can copy \0，so the copy len need +1 */
        snprintf(m_http_para[httpid].read_data_info.read_header_buf,
                 m_http_para[httpid].read_data_info.to_read_len + 1 +
                 strlen(res) + 1, "%s,%s", res,
                 m_http_para[httpid].cached_data_info.cached_header +
                 m_http_para[httpid].read_data_info.has_read_len);

        m_http_para[httpid].read_data_info.has_read_len +=
            m_http_para[httpid].read_data_info.cur_read_len;

        if (m_http_para[httpid].read_data_info.has_read_len >=
            m_http_para[httpid].cached_data_info.header_len)
            m_http_para[httpid].read_data_info.has_read_len = 0;

        transmit_mobile_resp_str(at_ptr,
                                 m_http_para[httpid].read_data_info.read_header_buf);
    }

    if (m_http_para[httpid].read_data_info.unread_header_len == 0) {
        aos_free(m_http_para[httpid].cached_data_info.cached_header);
        m_http_para[httpid].cached_data_info.cached_header = AIC_NULL;
        m_http_para[httpid].cached_data_info.header_len = 0;
    }

    aos_free(res);
    return;
}

static void mobile_http_response_content_read(AT_REQ_RESP *at_ptr, u8 httpid)
{
    u8 res[MOBILE_HTTP_URC_HEADER_LEN] = {'\0'};

    if (!m_http_para[httpid].cached_data_info.cached_data ||
        strlen(m_http_para[httpid].cached_data_info.cached_data) == 0) {
        snprintf(m_http_para[httpid].read_data_info.read_data_buf,
                 m_http_para[httpid].read_data_info.to_read_len +
                 MOBILE_HTTP_URC_HEADER_LEN, HTTP_READ_RESP"%d,%d,%d,%d", httpid,
                 MOBILE_HTTP_CONTENT_DATA, 0, 0);

        transmit_mobile_resp_str(at_ptr,
                                 m_http_para[httpid].read_data_info.read_data_buf);
        return;
    }

     if (m_http_para[httpid].read_data_info.unread_content_len > 0) {

         if (m_http_para[httpid].read_data_info.to_read_len <=
             m_http_para[httpid].read_data_info.unread_content_len) {

             m_http_para[httpid].read_data_info.cur_read_len =
                 m_http_para[httpid].read_data_info.to_read_len;
         } else {
             m_http_para[httpid].read_data_info.cur_read_len =
                 m_http_para[httpid].read_data_info.unread_content_len;
         }

         m_http_para[httpid].read_data_info.unread_content_len -=
             m_http_para[httpid].read_data_info.cur_read_len;

         snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_READ_RESP"%d,%d,%d,%d",
                  httpid, MOBILE_HTTP_CONTENT_DATA,
                  m_http_para[httpid].read_data_info.unread_content_len,
                  m_http_para[httpid].read_data_info.cur_read_len);

         /* snprintf can copy \0，so the copy len need +1 */
         snprintf(m_http_para[httpid].read_data_info.read_data_buf,
                  m_http_para[httpid].read_data_info.to_read_len + 1 +
                  strlen(res) + 1, "%s,%s", res,
                  m_http_para[httpid].cached_data_info.cached_data +
                  m_http_para[httpid].read_data_info.has_read_len);

         m_http_para[httpid].read_data_info.has_read_len +=
             m_http_para[httpid].read_data_info.cur_read_len;

         if (m_http_para[httpid].read_data_info.has_read_len >=
             m_http_para[httpid].cached_data_info.content_len)
             m_http_para[httpid].read_data_info.has_read_len = 0;
     }

    if (m_http_para[httpid].read_data_info.unread_content_len == 0) {
        aos_free(m_http_para[httpid].cached_data_info.cached_data);
        m_http_para[httpid].cached_data_info.cached_data = AIC_NULL;
        m_http_para[httpid].cached_data_info.content_len = 0;
    }

    transmit_mobile_resp_str(at_ptr,
                             m_http_para[httpid].read_data_info.read_data_buf);
    return;
}

static void mobile_http_content_or_header_clean(u8 httpid)
{
    if (!m_http_para[httpid].is_term) {
        mobile_http_specific_header_mem_release(httpid);
        mobile_http_specific_header_key_value_mem_release(httpid);
        mobile_http_content_mem_release(httpid);

        for (int i = 0; i < MOBILE_HTTP_MAX_HEADER_NUM; i++) {
            if (m_http_para[httpid].comm_k_v[i].key) {
                m_http_para[httpid].comm_k_v[i].is_same = 0;
            }
        }

        m_http_para[httpid].ready = 0;
        aos_memset(m_http_para[httpid].connect_info.resource, '\0',
                   MOBILE_HTTP_MAX_RESOURCE_LEN);
        m_http_para[httpid].connect_info.actual_len = 0;
        m_http_para[httpid].connect_info.resource_len = 0;
    }
}

static void mobile_http_chunked_request_send(u8 httpid, u32 urc_port)
{
    u32 status = AIC_SUCCESS;
    u8 res_buf[32] = {'\0'};

    if (m_http_para[httpid].handle == 0) {
        transmit_mobile_at_cme_urc(urc_port, MOBILE_HTTP_CLIENT_NOT_CREATED);
        return;
    }

    if (m_http_para[httpid].ready == 1) {
        mobile_http_read_data_info_reset(httpid);
        mobile_http_res_data_info_reset(httpid);
        if (m_http_para[httpid].cached)
            mobile_http_cached_data_mem_release(httpid);

        if (m_http_para[httpid].ssl_enable == MOBILE_HTTP_SSL_ENABLE &&
            m_http_para[httpid].connect_info.port == 0)
                m_http_para[httpid].connect_info.port = NX_WEB_HTTPS_SERVER_PORT;

        status = mobile_http_request_send(httpid);

        if (status) {
            aicx_error(0x0eba75, "x", "m_http resq send fail(0x%x)\n", status);
            if (status == MOBILE_HTTP_URC_URL_PARSING_FAILED) {
                snprintf(res_buf, 32, HTTP_URC_STR"\"err\",%d,%d", httpid,
                         MOBILE_HTTP_URC_URL_PARSING_FAILED);
                aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
            } else {
                snprintf(res_buf, 32, HTTP_URC_STR"\"err\",%d,%d", httpid,
                         MOBILE_HTTP_URC_RESPONSE_TIMEOUT);
                aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
            }
        } else {
            //alog_debug("mobile http request send is chunked mode\n");

            snprintf(res_buf, 32, HTTP_URC_STR"\"ind\",%d", httpid);
            aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
            /* release s_header and when request send */
            mobile_http_content_or_header_clean(httpid);
            m_http_para[httpid].s_content.chunked_data_input = 1;
        }
    }

    return;
}

static void mobile_http_request_send_and_process(u8 httpid, u32 urc_port)
{
    u32 status;
    u8 res_buf[32] = {'\0'};

    if (m_http_para[httpid].handle == 0) {
        snprintf(res_buf, 32, HTTP_URC_STR"\"err\",%d,%d",
                 httpid, MOBILE_HTTP_URC_RESPONSE_TIMEOUT);

        aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
        return;
    }

    mobile_http_read_data_info_reset(httpid);
    mobile_http_res_data_info_reset(httpid);
    if (m_http_para[httpid].cached)
        mobile_http_cached_data_mem_release(httpid);

    if (m_http_para[httpid].ssl_enable == MOBILE_HTTP_SSL_ENABLE &&
        m_http_para[httpid].connect_info.port == 0)
            m_http_para[httpid].connect_info.port = NX_WEB_HTTPS_SERVER_PORT;

    status = mobile_http_request_send(httpid);
    if (status) {
        m_http_para[httpid].ready = 0;
        aicx_error(0x0eba75, "x", "m_http resq send fail(0x%x)\n", status);

        if (status == MOBILE_HTTP_URC_URL_PARSING_FAILED) {
            snprintf(res_buf, 32, HTTP_URC_STR"\"err\",%d,%d", httpid,
                     MOBILE_HTTP_URC_URL_PARSING_FAILED);
            aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
        } else {
            snprintf(res_buf, 32, HTTP_URC_STR"\"err\",%d,%d", httpid,
                     MOBILE_HTTP_URC_RESPONSE_TIMEOUT);
            aic_transmit_at_cmd_urc(urc_port, res_buf, strlen(res_buf));
        }
    } else {
        /* get data, urc print or save in cache */
        mobile_http_response_process(httpid, urc_port);

        /* release s_header and s_content when request send.Only when request success.
           User send AT+HTTPTERM do not clear */
        mobile_http_content_or_header_clean(httpid);
    }

    return;
}

static void mobile_http_content_chunked_data_send(u8 httpid, u32 urc_port)
{
    u32 status;
    u8 *res = AIC_NULL;

    if (m_http_para[httpid].handle == 0) {
        res = aos_zalloc(MOBILE_HTTP_URC_HEADER_LEN);
        snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_URC_STR"\"err\",%d,%d",
                 httpid, MOBILE_HTTP_URC_RESPONSE_TIMEOUT);

        aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
        aos_free(res);

        return;
    }

    status = aic_https_client_request_chunked_packet_set(
             m_http_para[httpid].handle,
             m_http_para[httpid].s_content.actual_len,
             MOBILE_HTTP_DEFAULT_WAIT_TIME);
    if (status)
        goto ERROR_OUT;

    /* Build the post packet. */
    status = nx_packet_data_append(m_http_para[httpid].client_ptr->chunked_data_packet,
             m_http_para[httpid].s_content.data,
             m_http_para[httpid].s_content.actual_len,
             m_http_para[httpid].client_ptr->https_client->nx_web_http_client_packet_pool_ptr,
             MOBILE_HTTP_DEFAULT_WAIT_TIME);

    if (status)
        goto ERROR_OUT;

    status = nx_web_http_client_request_packet_send(
             m_http_para[httpid].client_ptr->https_client,
             m_http_para[httpid].client_ptr->chunked_data_packet,
             0, MOBILE_HTTP_DEFAULT_WAIT_TIME);
    if (status) {
        if (status == NX_WEB_HTTP_NOT_READY ||
            status == NX_WEB_HTTP_AUTHENTICATION_ERROR ||
            status == NX_WEB_HTTP_INCOMPLETE_PUT_ERROR ||
            status == NX_WEB_HTTP_BAD_PACKET_LENGTH)
            aic_http_packet_release(&m_http_para[httpid].client_ptr->chunked_data_packet);

        goto ERROR_OUT;
    } else {
        /* put packet has been send and released, set ptr to null. */
        m_http_para[httpid].client_ptr->chunked_data_packet = AIC_NULL;

        mobile_http_response_process(httpid, urc_port);

        /* release s_header and s_content when request send. Only request success
           User send AT+HTTPTERM do not clear */
        mobile_http_content_or_header_clean(httpid);
    }

    return;

ERROR_OUT:
    res = aos_zalloc(MOBILE_HTTP_URC_HEADER_LEN);
    snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_URC_STR"\"err\",%d,%d",
             httpid, MOBILE_HTTP_URC_RESPONSE_TIMEOUT);

    aic_transmit_at_cmd_urc(urc_port, res, strlen(res));
    aos_free(res);

    return;
}

#ifdef CONFIG_FS
/* asynchronous execution of time-consuming operations */
static void mobile_http_dl_file_entry(MOBILE_HTTP_MSG *mobile_http_msg)
{
    u32 status = AIC_SUCCESS;
    u8 httpid;

    httpid = mobile_http_msg->httpid;
#ifdef CONFIG_NET_HTTPS
    /* https */
    if (mobile_http_msg->ssl_enable &&
        mobile_http_msg->ssl_id != MOBILE_HTTP_INVALID_ID)
        m_http_para[httpid].handle = aic_https_client_create(
                                     MOBILE_HTTP_DL_CLIENT_NAME);
    else
#endif
        m_http_para[httpid].handle = aic_http_client_create(
                                     MOBILE_HTTP_DL_CLIENT_NAME);

    if (m_http_para[httpid].handle == 0) {
        transmit_mobile_at_cme_urc(mobile_http_msg->urc_port,
                                   MOBILE_HTTP_CONNECT_FAILED);
        goto DL_END;
    }

    m_http_para[httpid].client_ptr =
        (AIC_HTTPS_CLIENT_INSTANCE *)(m_http_para[httpid].handle);

#if (CONFIG_NET_HTTPS) && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT)
    if (mobile_http_msg->ssl_enable &&
        mobile_http_msg->ssl_id != MOBILE_HTTP_INVALID_ID &&
        msslconf_list[mobile_http_msg->ssl_id].seclevel &&
        msslconf_list[mobile_http_msg->ssl_id].cacert) {
        status = aic_https_ca_crt_data_add(m_http_para[httpid].handle,
                 msslconf_list[mobile_http_msg->ssl_id].cacert,
                 (u16)msslconf_list[mobile_http_msg->ssl_id].cacert_len,
                 httpid);

#ifdef CONFIG_CRYPTO_MBEDTLS
        if (m_http_para[httpid].client_ptr->https_client->http_mb_cfg)
            m_http_para[httpid].client_ptr->https_client->http_mb_cfg->recv_wait_time =
                                                MOBILE_HTTP_DEFAULT_CONNECT_TIMEOUT;
#endif
        if (status) {
            transmit_mobile_at_cme_urc(mobile_http_msg->urc_port,
                                       MOBILE_HTTP_CONNECT_FAILED);
            goto DL_END;
        }
    }
#endif /* (CONFIG_NET_HTTPS) && (CONFIG_CM_SSL_AT || CONFIG_GX_SSL_AT) */

    /* Set the header callback routine. */
    m_http_para[httpid].client_ptr->https_client->nx_web_http_client_header_callback =
                                                        mobile_http_header_res_urc_cb;

    m_http_para[httpid].client_ptr->https_client->nx_web_http_client_response_callback =
                                            mobile_http_content_range_get_cb;

#ifdef CONFIG_NET_HTTPS
    if (mobile_http_msg->ssl_enable &&
        mobile_http_msg->ssl_id != MOBILE_HTTP_INVALID_ID)
        status = aic_https_client_range_get_start(m_http_para[httpid].handle,
                 m_http_para[httpid].connect_info.url, AIC_NULL, AIC_NULL,
                 m_http_para[httpid].connect_info.port, mobile_http_msg->dl_start,
                 mobile_http_msg->dl_end, MOBILE_HTTP_DEFAULT_CONNECT_TIMEOUT);
    else
#endif
        status = aic_http_client_range_get_start(m_http_para[httpid].handle,
                 m_http_para[httpid].connect_info.url, AIC_NULL, AIC_NULL,
                 m_http_para[httpid].connect_info.port, mobile_http_msg->dl_start,
                 mobile_http_msg->dl_end, MOBILE_HTTP_DEFAULT_CONNECT_TIMEOUT);

    if (status) {
        aicx_error(0x04db01, "x", "m_http get resq send fail(0x%x)", status);
        transmit_mobile_at_cme_urc(mobile_http_msg->urc_port, MOBILE_HTTP_CONNECT_FAILED);
        goto DL_END;
    } else {
        transmit_mobile_at_ok_urc(mobile_http_msg->urc_port);
        mobile_http_response_save(mobile_http_msg->urc_port,
                                  mobile_http_msg->local_path, httpid,
                                  mobile_http_msg->eof, mobile_http_msg->progind);
    }

DL_END:
#ifdef CONFIG_NET_HTTPS
    if (m_http_para[httpid].ssl_enable ||
        m_http_para[httpid].client_ptr->https_client->nx_web_http_client_is_https) {

        if (mobile_http_msg->ssl_id != MOBILE_HTTP_INVALID_ID)
            aic_https_ca_crt_remove(m_http_para[httpid].handle, httpid);

        aic_https_client_delete(m_http_para[httpid].handle);
    } else
#endif
    {
        aic_http_client_delete(m_http_para[httpid].handle);
    }

    aos_free(mobile_http_msg->local_path);
    mobile_http_paras_reset(httpid);

    return;
}
#endif

static void mobile_http_method_process_entry(MOBILE_HTTP_MSG *mobile_http_msg)
{
    mobile_http_request_send_and_process(mobile_http_msg->httpid,
                                         mobile_http_msg->urc_port);
    return;
}

static void mobile_http_chunked_request_send_entry(
                                            MOBILE_HTTP_MSG *mobile_http_msg)
{
    mobile_http_chunked_request_send(mobile_http_msg->httpid,
                                 mobile_http_msg->urc_port);
    return;
}

static void mobile_http_chunked_data_send_entry(MOBILE_HTTP_MSG *mobile_http_msg)
{
    mobile_http_content_chunked_data_send(mobile_http_msg->httpid,
                                          mobile_http_msg->urc_port);
    return;
}

static void mobile_http_client_delete_entry(MOBILE_HTTP_MSG *mobile_http_msg)
{
    u32 status;
    u8 httpid;

    httpid = mobile_http_msg->httpid;
    if (m_http_para[httpid].httpid != httpid ||
        !m_http_para[httpid].client_ptr) {
        return;
    }

    /* avoid executing ssl receive in AT threads.code move from delete at set cmd */
    nx_web_http_client_cleanup(m_http_para[httpid].client_ptr->https_client);

    nx_tcp_socket_disconnect(
        &(m_http_para[httpid].client_ptr->https_client->nx_web_http_client_socket), 500);
    nx_tcp_client_socket_unbind(
        &(m_http_para[httpid].client_ptr->https_client->nx_web_http_client_socket));

    /* Clear out the HTTP(S) client. */
#ifdef CONFIG_NET_HTTPS
    if (m_http_para[httpid].ssl_enable ||
        m_http_para[httpid].client_ptr->https_client->nx_web_http_client_is_https) {
        if (m_http_para[httpid].client_ptr->https_client->auth_type != 0)
            aic_https_ca_crt_remove(m_http_para[httpid].handle, httpid);

        status = aic_https_client_delete(m_http_para[httpid].handle);
    } else
#endif
    {
        status = aic_http_client_delete(m_http_para[httpid].handle);
    }

    mobile_http_paras_reset(httpid);

    return;
}

#ifdef CONFIG_FS
void m_http_dl_msg_process(void *thread_input)
{
    u32 status;
    MOBILE_HTTP_MSG mobile_http_msg;

    while (1) {
        status = aos_recv_from_queue(m_http_dl_q, &mobile_http_msg,
                                     AOS_WAIT_FOREVER);

        if( AOS_QUEUE_EMPTY == status) {
            continue;
        } else {
            switch (mobile_http_msg.msg_id) {
            case MOBILE_HTTP_DL_FILE:
                mobile_http_dl_file_entry(&mobile_http_msg);
                break;
            default:
                break;
            }
        }
    }
}
#endif

void m_http_event_process(void *thread_input)
{
    u32 status;
    MOBILE_HTTP_MSG mobile_http_msg;

    while (1) {
        status = aos_recv_from_queue(m_http_q, &mobile_http_msg,
                                     AOS_WAIT_FOREVER);

        if( AOS_QUEUE_EMPTY == status) {
            continue;
        } else {
            switch (mobile_http_msg.msg_id) {
            case MOBILE_HTTP_METHOD_PROCESS:
                mobile_http_method_process_entry(&mobile_http_msg);
                break;
            case MOBILE_HTTP_CHUNKED_REQUEST_SEND:
                mobile_http_chunked_request_send_entry(&mobile_http_msg);
                break;
            case MOBILE_HTTP_CHUNKED_DATA_SEND:
                mobile_http_chunked_data_send_entry(&mobile_http_msg);
                break;
            case MOBILE_HTTP_CLIENT_DELETE:
                mobile_http_client_delete_entry(&mobile_http_msg);
                break;
            default:
                break;
            }
        }
    }
}

/**
 * Execution command creat HTTP client.
 */
u32 mobile_http_client_create_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count = 0;
    u8 httpid = MOBILE_HTTP_INVALID_ID;
    u8 res[32] = {'\0'};
    u8 ssl_enable = 0;
    u32 status = 0;
    int retry_times = 12;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_CREATE_CMD"=",
                                         &count);

    if (!paras || count != 1) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
        goto ERROR_OUT;
    }

    if (strlen((u8 *)paras[0]) < MOBILE_HTTP_URL_HEADER_LEN) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
        goto ERROR_OUT;
    }

    /* NX_WEB_HTTP_CLIENT_TIMEOUT is 10s, when http client delete,
       the max wait time is NX_WEB_HTTP_CLIENT_TIMEOUT.
       So we need wait 10s here. */
    do {
        httpid = mobile_http_idle_httpid_find();
        if (httpid <= 3)
            break;

        aos_sleep(500);
        //alog_debug("mobile http try to find idle httpid\n");
        retry_times --;
    } while (retry_times > 0);

    if (httpid == MOBILE_HTTP_INVALID_ID) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_NO_IDLE_CLIENT);
        goto ERROR_OUT;
    }

    m_http_para[httpid].httpid = httpid;
    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.url,
                               MOBILE_HTTP_MAX_URL_LEN);
    if (status)
        goto ERROR_OUT;

    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.host,
                               MOBILE_HTTP_MAX_HOST_LEN);
    if (status)
        goto ERROR_OUT;

    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.resource,
                               MOBILE_HTTP_MAX_RESOURCE_LEN);
    if (status)
        goto ERROR_OUT;

    strncpy(m_http_para[httpid].connect_info.host, (u8 *)paras[0],
            MOBILE_HTTP_MAX_HOST_LEN);

    ssl_enable = mobile_http_ssl_enable_check((u8 *)paras[0]);
    mobile_http_server_port_find(m_http_para[httpid].connect_info.host, httpid,
                                 ssl_enable);

#ifdef CONFIG_NET_HTTPS
    if (ssl_enable)
        m_http_para[httpid].handle =
                            aic_https_client_create(MOBILE_HTTPS_CLIENT_NAME);
    else
#endif
        m_http_para[httpid].handle =
                               aic_http_client_create(MOBILE_HTTP_CLIENT_NAME);

    if (m_http_para[httpid].handle == 0) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_CLIENT_NOT_CREATED);
        mobile_http_connect_info_mem_release(httpid);
        goto ERROR_OUT;
    }

    m_http_para[httpid].client_ptr =
        (AIC_HTTPS_CLIENT_INSTANCE *)(m_http_para[httpid].handle);

    /* Set the header callback routine. */
    m_http_para[httpid].client_ptr->https_client->nx_web_http_client_header_callback =
                                                        mobile_http_header_res_urc_cb;

    if (m_http_para[httpid].input_timeout == 0)
        m_http_para[httpid].input_timeout = MOBILE_HTTP_DEFAULT_INPUT_TIMEOUT;

    snprintf(res, 32, HTTP_CREATE_RESP"%d", httpid);
    transmit_mobile_resp_str(at_ptr, res);

    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_client_create_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    aic_reply_ok_resp(at_ptr);

    return AIC_SUCCESS;
}

static u32 mobile_http_input_id_check(u8 *data, int httpid)
{
    if (!aic_is_valid_integer_para(data, strlen(data)))
        return MOBILE_HTTP_PARAMETER_ERROR;

    if (httpid < 0 || httpid >= MOBILE_HTTP_MAX_CLIENT_NUM)
        return MOBILE_HTTP_PARAMETER_ERROR;

    if (m_http_para[httpid].handle == 0 || m_http_para[httpid].downloading)
        return MOBILE_HTTP_CLIENT_NOT_CREATED;

    return AIC_SUCCESS;
}

/**
 * Execution command set the HTTP parameters value.
 */
u32 mobile_http_para_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count = 0;
    s8 httpid = MOBILE_HTTP_INVALID_ID;
    u8 *tag = AIC_NULL;
    u32 i;
    u32 status;

    if (mobile_specific_cmd_check(at_ptr->at_req))
        paras = mobile_split_paras_to_str_array(at_ptr->at_req, HTTP_CFG_CMD"=",
                                                &count, 3, 3, 0);
    else
        paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_CFG_CMD"=",
                                             &count);

    if (!paras || count == 0)
        goto ERROR_OUT;

    /* print all httpid */
    if (count == 1) {
        tag = (u8*)paras[0];

        for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
            if (m_http_para[i].handle != 0)
                mobile_http_paras_value_transmit(at_ptr, i, tag, PRINT_ONE);
        }
        goto SUCESS_OUT;
    }

    if (count >= 2) {
        httpid = atoi((u8*)paras[1]);
        tag = (u8*)paras[0];

        status = mobile_http_input_id_check((u8*)paras[1], httpid);
        if (status == MOBILE_HTTP_PARAMETER_ERROR)
            goto ERROR_OUT;
        else if (status == MOBILE_HTTP_CLIENT_NOT_CREATED) {
            aic_reply_error_resp(at_ptr, MOBILE_HTTP_CLIENT_NOT_CREATED);
            goto FREE_PARA;
        }
    }

    /* print specified httpid */
    if (paras && count == 2) {
        mobile_http_paras_value_transmit(at_ptr, httpid, tag, PRINT_ONE);
        goto SUCESS_OUT;
    }

    /* set paras */
    status = mobile_http_paras_set(at_ptr);
    if (status) {
        if (status == MOBILE_HTTP_OPERATION_NOT_ALLOWED) {
            aic_reply_error_resp(at_ptr, MOBILE_HTTP_OPERATION_NOT_ALLOWED);
            goto FREE_PARA;
        } else
            goto ERROR_OUT;
    }

SUCESS_OUT:
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);

FREE_PARA:
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_para_at_query_cmd(AT_REQ_RESP *at_ptr)
{
    u32 i;
    u8 *tag = "all";

    for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
        if (m_http_para[i].handle)
            mobile_http_paras_value_transmit(at_ptr, i, tag, PRINT_ALL);
    }

    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

u32 mobile_http_para_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "header", "(0-3)", "(0-1024)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "chunked", "(0-3)", "(0,1)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "cached", "(0-3)", "(0,1)",
                           "(1-8192)", AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "timeout", "(0-3)", "(0-180)",
                           "(0-60)", "(1-120)");

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "encoding", "(0-3)", "(0-2)",
                           "(0,1)", AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "ssl", "(0-3)", "(0,1)", "(0-5)",
                           AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "cid", "(0-3)", "(1-5)",
                           AIC_NULL, AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "fragment", "(0-3)", "(0-1024)",
                           "(0-2000)", AIC_NULL);

    transmit_mobile_at_str(at_ptr, HTTP_CFG_RESP, "urlencode", "(0-3)", "(0,1)",
                           AIC_NULL, AIC_NULL);

    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

static u32 mobile_http_input_data_len_check(int *input_data_len,
                                            int actual_data_len, u8 input_format)
{
    if (*input_data_len == 0) {
        if (input_format == MOBILE_HTTP_HEX) {
            if ((actual_data_len & 1) != 0) {
                return 1;
            }

            *input_data_len = actual_data_len >> 1;
        } else
            *input_data_len = actual_data_len;
    } else {
        if (input_format == MOBILE_HTTP_HEX) {
            if (2 * (*input_data_len) != actual_data_len)
                return 1;
        } else {
            if (*input_data_len != actual_data_len)
                return 1;
        }
    }

    return 0;
}

u32 mobile_http_content_or_header_set(AT_REQ_RESP *at_ptr, u8 set_type, u32 max_len)
{
    ulong *paras = AIC_NULL;
    u16 count = 0;
    static int httpid = MOBILE_HTTP_INVALID_ID;
    u8 http_eof = MOBILE_HTTP_INVALID_ID;
    MOBILE_HTTP_MSG mobile_http_msg = {'\0'};
    u32 status = 0;
    u8 *res = AIC_NULL;
    u32 offset_len = 0;
    u32 out_data_len = 0;
    u8 input_format = 0;
    MOBILE_HTTP_HEADER_CONTENT_INFO *info = AIC_NULL;

    /* cmd mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        if (set_type == MOBILE_HTTP_CONTENT_SET)
            paras = mobile_split_paras_to_str_array(at_ptr->at_req, HTTP_CONTENT_CMD"=",
                                                               &count, 4, 4, 0);
        else
            paras = mobile_split_paras_to_str_array(at_ptr->at_req, HTTP_HEADER_CMD"=",
                                                    &count, 4, 4, 0);

        if (!paras || count > 4)
            goto ERROR_OUT;

        if (count > 0) {
           httpid = atoi((u8*)paras[0]);
           status = mobile_http_input_id_check((u8*)paras[0], httpid);
           if (status == MOBILE_HTTP_PARAMETER_ERROR)
               goto ERROR_OUT;
           else if (status == MOBILE_HTTP_CLIENT_NOT_CREATED) {
               aic_reply_error_resp(at_ptr, MOBILE_HTTP_CLIENT_NOT_CREATED);
               goto FREE_PARA;
           }
        }

        if (set_type == MOBILE_HTTP_CONTENT_SET)
            info = &m_http_para[httpid].s_content;
        else
            info = &m_http_para[httpid].s_header;

        if (m_http_para[httpid].chunked_mode && info->chunked_data_input != 1) {
            aic_reply_error_resp(at_ptr, MOBILE_HTTP_OPERATION_NOT_ALLOWED);
            aic_free_paras_mem(paras, count);
            return MOBILE_HTTP_OPERATION_NOT_ALLOWED;
        }

        info->input_len = 0;
        input_format = m_http_para[httpid].input_format;

        /* query specific httpid content */
        if (count == 1) {
            if (!info->data) {
                aic_reply_ok_resp(at_ptr);
            } else if (info->data && *info->data == 0) {
                if (set_type == MOBILE_HTTP_CONTENT_SET)
                    mobile_http_content_mem_release(httpid);
                else {
                    mobile_http_specific_header_mem_release(httpid);
                    mobile_http_specific_header_key_value_mem_release(httpid);
                }

                aic_reply_ok_resp(at_ptr);
            } else {
                res = aos_zalloc_relaxed(info->actual_len + MOBILE_HTTP_URC_HEADER_LEN);
                if (!res) {
                    aic_reply_error_resp(at_ptr, MOBILE_HTTP_MEMORY_ALLOCATION_FAILED);
                } else {
                    if (set_type == MOBILE_HTTP_CONTENT_SET)
                        snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_CONTENT_RESP"%d,%d,",
                                 httpid, info->actual_len);
                    else
                        snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_HEADER_RESP"%d,%d,",
                                 httpid, info->actual_len);

                    offset_len = strlen(res);
                    aos_memcpy(res + offset_len, info->data, info->actual_len);
                    offset_len += info->actual_len;
                    aic_transmit_at_cmd_urc(at_ptr->serial_port_id, res, offset_len);
                    aos_free(res);
                    aic_reply_ok_resp(at_ptr);
                }
            }
        }

        if (count > 1) {
            if(mobile_at_para_range_check((u8 *)paras[1], 0, 2))
                goto ERROR_OUT;

            http_eof = atoi((u8*)paras[1]);
        }

        /* clean data */
        if (http_eof == MOBILE_HTTP_CLEAR_DATA_EOF) {
            if (!m_http_para[httpid].chunked_mode && set_type == MOBILE_HTTP_CONTENT_SET) {
                mobile_http_content_mem_release(httpid);
            } else {
                mobile_http_specific_header_mem_release(httpid);
                mobile_http_specific_header_key_value_mem_release(httpid);
            }

            info->eof = MOBILE_HTTP_INVALID_ID;
            aic_reply_ok_resp(at_ptr);
            goto OUT;
        }

        /* set content by data mode */
        if (count > 1 && count < 4) {
            if (count == 2)
                info->end_mob = 1;

            if (count == 3) {
                if (*(u8*)paras[2]) {
                    if(mobile_at_para_range_check((u8 *)paras[2], 0, max_len))
                        goto ERROR_OUT;

                    info->input_len = atoi((u8*)paras[2]);
                    info->end_mob = 0;

                    if (atoi((u8*)paras[2]) == 0)
                        info->end_mob = 1;
                } else {
                    goto ERROR_OUT;
                }
            }

            if (http_eof != MOBILE_HTTP_CLEAR_DATA_EOF) {
                if (info->data && info->eof == MOBILE_HTTP_END_DATA_EOF) {
                    aic_reply_error_resp(at_ptr, MOBILE_HTTP_OPERATION_NOT_ALLOWED);
                    aic_free_paras_mem(paras, count);
                    return MOBILE_HTTP_OPERATION_NOT_ALLOWED;
                }

                info->eof = http_eof;
                if (set_type == MOBILE_HTTP_CONTENT_SET) {
                    status = mobile_http_content_mem_alloc(httpid);
                    if (status) {
                        aic_reply_error_resp(at_ptr,
                                               MOBILE_HTTP_MEMORY_ALLOCATION_FAILED);
                        at_ptr->at_mode = AT_DATA_MODE;
                        goto ERROR_OUT;
                    }
                } else
                    mobile_http_specific_header_mem_alloc(httpid);

                at_ptr->at_mode = AT_DATA_MODE;
                transmit_mobile_resp_str(at_ptr, ">");

                if (m_http_para[httpid].input_timeout == 0)
                    m_http_para[httpid].input_timeout =
                                    MOBILE_HTTP_DEFAULT_INPUT_TIMEOUT;

                if (set_type == MOBILE_HTTP_CONTENT_SET)
                    aic_set_data_mode_expire_time(HTTP_CONTENT_CMD,
                                                  m_http_para[httpid].input_timeout);
                else
                    aic_set_data_mode_expire_time(HTTP_HEADER_CMD,
                                                  m_http_para[httpid].input_timeout);

                goto OUT;
            }
        }

        if (count == 4) {
            if (*(u8 *)paras[2] == 0)
                goto ERROR_OUT;
            else {
                if(mobile_at_para_range_check((u8 *)paras[2], 0, max_len))
                    goto ERROR_OUT;

                info->input_len = atoi((u8*)paras[2]);
            }

            if (info->data && info->eof == MOBILE_HTTP_END_DATA_EOF) {
                aic_reply_error_resp(at_ptr, MOBILE_HTTP_OPERATION_NOT_ALLOWED);
                aic_free_paras_mem(paras, count);
                return MOBILE_HTTP_OPERATION_NOT_ALLOWED;
            }

            if (set_type == MOBILE_HTTP_CONTENT_SET) {
                status = mobile_http_content_mem_alloc(httpid);
                if (status) {
                    aic_reply_error_resp(at_ptr, MOBILE_HTTP_MEMORY_ALLOCATION_FAILED);
                    at_ptr->at_mode = AT_DATA_MODE;
                    goto ERROR_OUT;
                }
            } else
                mobile_http_specific_header_mem_alloc(httpid);

            status = mobile_http_input_data_len_check(&info->input_len,
                                                      strlen((u8*)paras[3]),
                                                      input_format);
            if (status)
                goto ERROR_OUT;

            if (http_eof != MOBILE_HTTP_CLEAR_DATA_EOF) {
                info->eof = http_eof;
                if (set_type == MOBILE_HTTP_HEADER_SET) {
                    if (info->offset >= max_len)
                        goto OK_EXIT;
                    out_data_len = max_len - info->actual_len;
                } else if (set_type == MOBILE_HTTP_CONTENT_SET) {
                    if (info->offset == 3 * max_len)
                        goto OK_EXIT;
                    out_data_len = (3 * max_len) - info->actual_len;
                }

                status = mobile_http_input_data_format_process(httpid,
                         info->data + info->offset, (u8 *)paras[3],
                         info->input_len, &out_data_len);
                if (status)
                    goto ERROR_OUT;

                info->input_len = out_data_len;
                info->actual_len += info->input_len;
                if (set_type == MOBILE_HTTP_HEADER_SET) {
                    /* insert \r\n to the end of header */
                    info->data[info->actual_len] = '\r';
                    info->data[info->actual_len + 1] = '\n';
                    info->actual_len += 2;
                    mobile_http_sp_header_key_and_value_find(httpid);
                }

                if (info->eof == MOBILE_HTTP_END_DATA_EOF)
                    info->offset = MOBILE_HTTP_END_DATA_EOF;
                else
                    info->offset = info->actual_len;
            }
OK_EXIT:
            aic_reply_ok_resp(at_ptr);
        }
    } else if (at_ptr->at_mode == AT_DATA_MODE) {

        if (set_type == MOBILE_HTTP_CONTENT_SET)
            info = &m_http_para[httpid].s_content;
        else
            info = &m_http_para[httpid].s_header;

        /* data mode */
        if (set_type == MOBILE_HTTP_CONTENT_SET) {
            if (at_ptr->end_data_mode) {
                at_ptr->at_mode = AT_CMD_MODE;
                if (info->data) {
                    if (*info->data == 0) {
                        mobile_http_content_mem_release(httpid);
                    } else {
                        if (!info->actual_len) {
                            /* reset eof only when there was no input at all */
                            info->eof = MOBILE_HTTP_INVALID_ID;
                        } else {
                            info->offset = info->actual_len;
                        }
                    }
                    aic_reply_ok_resp(at_ptr);
                }
            } else
                mobile_http_specific_content_set(at_ptr, httpid);
        } else {
            if (at_ptr->end_data_mode) {
                at_ptr->at_mode = AT_CMD_MODE;
                if (info->offset == info->actual_len) {
                    /* no input this time, need to roll back eof value in case it was set to 0 */
                    info->eof = MOBILE_HTTP_INVALID_ID;
                    goto ERROR_OUT;
                } else {
                    /* insert \r\n to the end of header */
                    info->data[info->actual_len] = '\r';
                    info->data[info->actual_len + 1] = '\n';
                    info->actual_len += 2;
                    info->offset = info->actual_len;
                    mobile_http_sp_header_key_and_value_find(httpid);
                    aic_reply_ok_resp(at_ptr);
                }
            } else
                mobile_http_specific_header_set(at_ptr, httpid);
        }
    }

    if (info && info->data && info->eof == 0 && info->chunked_data_input == 1 &&
        (m_http_para[httpid].method == MOBILE_HTTP_METHOD_PUT ||
        m_http_para[httpid].method == MOBILE_HTTP_METHOD_POST)) {

        mobile_http_msg.httpid = httpid;
        mobile_http_msg.msg_id = MOBILE_HTTP_CHUNKED_DATA_SEND;
        mobile_http_msg.urc_port = at_ptr->serial_port_id;

        aos_send_to_queue(m_http_q, &mobile_http_msg,
                          MOBILE_HTTP_DEFAULT_WAIT_TIME);
    }

OUT:
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    if (httpid >= 0 && httpid < MOBILE_HTTP_MAX_CLIENT_NUM) {
        if (info && info->offset == 0) {
            if (set_type == MOBILE_HTTP_CONTENT_SET)
                mobile_http_content_mem_release(httpid);
            else {
                mobile_http_specific_header_mem_release(httpid);
                mobile_http_specific_header_key_value_mem_release(httpid);
            }
        }
        if (info && info->eof == MOBILE_HTTP_END_DATA_EOF) {
            /* enable the next header input */
            info->eof = MOBILE_HTTP_INVALID_ID;
        }
    }

    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
FREE_PARA:
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

/**
 * Execution command set the HTTP header.
 */
u32 mobile_http_header_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    return mobile_http_content_or_header_set(at_ptr, MOBILE_HTTP_HEADER_SET,
                                             MOBILE_HTTP_MAX_HEADER_LEN);
}

u32 mobile_http_header_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_HEADER_RESP"(0-3),(0-2),(0-4096)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Execution command set the HTTP content.
 */
u32 mobile_http_content_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    return mobile_http_content_or_header_set(at_ptr, MOBILE_HTTP_CONTENT_SET,
                                             MOBILE_HTTP_MAX_CONTENT_DATA_LEN);
}

u32 mobile_http_content_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_CONTENT_RESP"(0-3),(0-2),(0-4096)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Submit the HTTP request.
 * It executes the GET/POST/PUT/DELETE/HEAD operations
 */
u32 mobile_http_request_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras = AIC_NULL;
    u16 count = 0;
    static int httpid = MOBILE_HTTP_INVALID_ID;
    MOBILE_HTTP_MSG mobile_http_msg = {'\0'};
    u32 status = 0;
    u32 out_data_len = 0;
    u8 *found = AIC_NULL;
    u32 offset = 0;

    /* cmd mode */
    if (at_ptr->at_mode == AT_CMD_MODE) {
        paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_REQUEST_CMD"=",
                                             &count);

        if (!paras || count < 2 || count > 5)
            goto ERROR_OUT;

        if (count > 1) {
            if(mobile_at_para_range_check((u8 *)paras[0], 0, 3))
                goto ERROR_OUT;

            if(mobile_at_para_range_check((u8 *)paras[1], MOBILE_HTTP_METHOD_GET,
                                          MOBILE_HTTP_METHOD_HEAD))
                goto ERROR_OUT;

            httpid = atoi((u8*)paras[0]);

            m_http_para[httpid].method = atoi((u8*)paras[1]);
            m_http_para[httpid].urc_port = at_ptr->serial_port_id;
            m_http_para[httpid].is_term = 0;

            if (m_http_para[httpid].handle == 0 || m_http_para[httpid].downloading) {
                aic_reply_error_resp(at_ptr, MOBILE_HTTP_CLIENT_NOT_CREATED);
                aic_free_paras_mem(paras, count);
                m_http_para[httpid].connect_info.actual_len = 0;
                m_http_para[httpid].connect_info.resource_len = 0;
                at_ptr->at_mode = AT_CMD_MODE;
                return MOBILE_HTTP_CLIENT_NOT_CREATED;
            }

            if (m_http_para[httpid].ready == 1) {
                aic_reply_error_resp(at_ptr, MOBILE_HTTP_CLIENT_BUSY);
                aic_free_paras_mem(paras, count);
                m_http_para[httpid].connect_info.actual_len = 0;
                m_http_para[httpid].connect_info.resource_len = 0;
                at_ptr->at_mode = AT_CMD_MODE;
                return MOBILE_HTTP_CLIENT_BUSY;
            }
        }

        aos_memset(m_http_para[httpid].client_ptr->status_code, 0, 4);
        /* set header by data mode */
        if (count > 1 && count < 4) {
            if (count == 2) {
                m_http_para[httpid].connect_info.resource_len = 0;
                m_http_para[httpid].connect_info.end_mob = 1;
            }

            if (count == 3) {
                if (strlen((u8*)paras[2])) {
                    if(mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_HTTP_MAX_RESOURCE_LEN))
                        goto ERROR_OUT;

                    m_http_para[httpid].connect_info.resource_len = atoi((u8*)paras[2]);
                    m_http_para[httpid].connect_info.end_mob = 0;

                    if (atoi((u8*)paras[2]) == 0)
                        m_http_para[httpid].connect_info.end_mob = 1;
                } else {
                    goto ERROR_OUT;
                }
            }

REQUEST_DATA_MODE:
            m_http_para[httpid].urc_port = at_ptr->serial_port_id;
            mobile_http_read_data_info_reset(httpid);

            if (m_http_para[httpid].connect_info.resource)
                aos_memset(m_http_para[httpid].connect_info.resource, '\0',
                           MOBILE_HTTP_MAX_RESOURCE_LEN);

            at_ptr->at_mode = AT_DATA_MODE;
            transmit_mobile_resp_str(at_ptr, ">");

            if (m_http_para[httpid].input_timeout == 0)
                m_http_para[httpid].input_timeout = MOBILE_HTTP_DEFAULT_INPUT_TIMEOUT;

            aic_set_data_mode_expire_time(HTTP_REQUEST_CMD,
                                          m_http_para[httpid].input_timeout);

            goto OUT;
        }

        if (count == 4) {
          if (strlen((u8 *)paras[2]) == 0)
              goto ERROR_OUT;
          else {
              if(mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_HTTP_MAX_RESOURCE_LEN))
                  goto ERROR_OUT;

              m_http_para[httpid].connect_info.resource_len = atoi((u8*)paras[2]);
          }

            status = mobile_http_input_data_len_check(&m_http_para[httpid].connect_info.resource_len,
                                                      strlen((u8*)paras[3]),
                                                      m_http_para[httpid].input_format);
            if (status)
                goto ERROR_OUT;

            out_data_len = MOBILE_HTTP_MAX_RESOURCE_LEN;
            status = mobile_http_input_data_format_process(httpid,
                     m_http_para[httpid].connect_info.resource,
                     (u8 *)paras[3],
                     m_http_para[httpid].connect_info.resource_len, &out_data_len);
            if (status)
                goto ERROR_OUT;

            m_http_para[httpid].ready = 1;
            aic_reply_ok_resp(at_ptr);
            goto SEND;
        }

        if (count == 5) {
            if (*(u8 *)paras[2] == 0 || *(u8 *)paras[4] == 0)
                goto ERROR_OUT;

            status = mobile_mem_alloc(&m_http_para[httpid].local_path,
                                      MOBILE_HTTP_MAX_LOCAL_PATH_LEN);
            if (status)
                goto ERROR_OUT;

            m_http_para[httpid].dl_mod = 1;

            aos_memcpy(m_http_para[httpid].local_path, MOBILE_HTTP_HOME_PATH,
                       strlen(MOBILE_HTTP_HOME_PATH));
            if (strncmp((u8*)paras[4], "/", 1) == 0) {
                found = (u8*)paras[4] + 1;
                do {
                    found = strchr(found, '/');
                    if (found) {
                        found++;
                        offset = strlen(found);
                    } else {
                        if (offset == 0)
                            offset = strlen((u8*)paras[4]); /*no secondary dir */
                        else
                            offset = strlen((u8*)paras[4]) - offset - 1;/* remove the last / , need to -1 */
                        break;
                    }
                } while (found);

                /* only has secondary dir, need to mkdir */
                if (offset != strlen((u8*)paras[4])) {
                    /*local_path used to mkdir*/
                    strncpy(m_http_para[httpid].local_path + strlen(m_http_para[httpid].local_path),
                            (u8*)paras[4], offset);
#ifdef CONFIG_FS
                    status = mkdir(m_http_para[httpid].local_path, FS_O_RDWR);
                    if (status < 0) {
                        aic_reply_error_resp(at_ptr, MOBILE_HTTP_URC_WRITE_FILE_FAILED);
                        goto OUT;
                    }
#else
                goto ERROR_OUT;
#endif
                }

                /* reset local_path used to save file */
                aos_memset(m_http_para[httpid].local_path, '\0', MOBILE_HTTP_MAX_LOCAL_PATH_LEN + 1);
                aos_memcpy(m_http_para[httpid].local_path, MOBILE_HTTP_HOME_PATH,
                           strlen(MOBILE_HTTP_HOME_PATH));
                strncpy(m_http_para[httpid].local_path + strlen(m_http_para[httpid].local_path),
                        (u8*)paras[4], strlen((u8*)paras[4]));
            } else {
                snprintf(m_http_para[httpid].local_path + strlen(m_http_para[httpid].local_path),
                         MOBILE_HTTP_MAX_LOCAL_PATH_LEN, "%s%s",
                         MOBILE_HTTP_DEFAULT_DL_PATH, (u8*)paras[4]);
#ifdef CONFIG_FS
                status = mkdir(MOBILE_HTTP_MKDIR_PATH, FS_O_RDWR);
                if (status < 0) {
                    aic_reply_error_resp(at_ptr, MOBILE_HTTP_URC_WRITE_FILE_FAILED);
                    goto OUT;
                }
#else
            goto ERROR_OUT;
#endif
            }

            if (*(u8 *)paras[3] == 0) {
                m_http_para[httpid].connect_info.resource_len =
                                                        atoi((u8*)paras[2]);
                if (m_http_para[httpid].connect_info.resource_len == 0)
                    m_http_para[httpid].connect_info.end_mob = 1;
                else
                    m_http_para[httpid].connect_info.end_mob = 0;
                goto REQUEST_DATA_MODE;
            }

            if (atoi((u8*)paras[2]) == 0)
                m_http_para[httpid].connect_info.resource_len =
                                                    strlen((u8 *)paras[3]);
            else {
                if (m_http_para[httpid].input_format == MOBILE_HTTP_HEX) {
                    if (2 * atoi((u8*)paras[2]) != strlen((u8 *)paras[3]))
                        goto ERROR_OUT;
                } else {
                    if (atoi((u8*)paras[2]) != strlen((u8 *)paras[3]))
                        goto ERROR_OUT;
                }
                m_http_para[httpid].connect_info.resource_len =
                                                        atoi((u8*)paras[2]);
            }
            out_data_len = MOBILE_HTTP_MAX_RESOURCE_LEN;
            mobile_http_input_data_format_process(httpid,
                    m_http_para[httpid].connect_info.resource,
                    (u8 *)paras[3],
                    m_http_para[httpid].connect_info.resource_len,
                    &out_data_len);

            m_http_para[httpid].ready = 1;
            aic_reply_ok_resp(at_ptr);
            goto SEND;
        }
    } else if (at_ptr->at_mode == AT_DATA_MODE) {
        /* data mode */
        if (at_ptr->end_data_mode) {
            at_ptr->at_mode = AT_CMD_MODE;
            if (m_http_para[httpid].connect_info.resource &&
                strlen(m_http_para[httpid].connect_info.resource)) {
                m_http_para[httpid].ready = 1;
                aic_reply_ok_resp(at_ptr);
                goto SEND;
            } else {
                at_ptr->at_mode = AT_CMD_MODE;
                goto ERROR_OUT;
            }
        }
        mobile_http_resource_set(at_ptr, httpid);
        goto SEND;
    }

SEND:
    if (m_http_para[httpid].ready == 1 &&
        m_http_para[httpid].chunked_mode == 0 &&
        (m_http_para[httpid].method == MOBILE_HTTP_METHOD_PUT ||
        m_http_para[httpid].method == MOBILE_HTTP_METHOD_POST)) {
        if (!m_http_para[httpid].s_content.data) {

            m_http_para[httpid].s_content.data = aos_zalloc(64);
            m_http_para[httpid].s_content.eof = MOBILE_HTTP_END_DATA_EOF;

            m_http_para[httpid].s_content.actual_len = 0;
            mobile_http_msg.httpid = httpid;
            mobile_http_msg.msg_id = MOBILE_HTTP_METHOD_PROCESS;
            mobile_http_msg.urc_port = at_ptr->serial_port_id;

            aos_send_to_queue(m_http_q, &mobile_http_msg,
                              MOBILE_HTTP_DEFAULT_WAIT_TIME);

            goto OUT;
        }
    }

    if (m_http_para[httpid].chunked_mode == 1 &&
        (m_http_para[httpid].method == MOBILE_HTTP_METHOD_PUT ||
        m_http_para[httpid].method == MOBILE_HTTP_METHOD_POST)) {

        mobile_http_msg.httpid = httpid;
        mobile_http_msg.msg_id = MOBILE_HTTP_CHUNKED_REQUEST_SEND;
        mobile_http_msg.urc_port = at_ptr->serial_port_id;

        aos_send_to_queue(m_http_q, &mobile_http_msg,
                          MOBILE_HTTP_DEFAULT_WAIT_TIME);

        goto OUT;
    }

    if (m_http_para[httpid].ready == 1) {
        mobile_http_msg.httpid = httpid;
        mobile_http_msg.msg_id = MOBILE_HTTP_METHOD_PROCESS;
        mobile_http_msg.urc_port = at_ptr->serial_port_id;

        aos_send_to_queue(m_http_q, &mobile_http_msg,
                          MOBILE_HTTP_DEFAULT_WAIT_TIME);

        at_ptr->at_mode = AT_CMD_MODE;
    }

OUT:
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_request_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_REQUEST_RESP"(0-3),(1-5),(0-4096)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Execution command read the HTTP service response.
 */
u32 mobile_http_read_data_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count = 0;
    u8 res[MOBILE_HTTP_URC_HEADER_LEN] = {'\0'};
    s8 httpid = MOBILE_HTTP_INVALID_ID;

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MHTTP_READ_CMD"=",
                                         &count);
    if (!paras || count < 2 || count > 3)
        goto ERROR_OUT;

    if (paras && count > 1) {
        if(mobile_at_para_range_check((u8 *)paras[0], 0, 3))
            goto ERROR_OUT;

        if(mobile_at_para_range_check((u8 *)paras[1], 0, 1))
            goto ERROR_OUT;

        httpid = atoi((u8*)paras[0]);
        m_http_para[httpid].read_data_info.data_type = atoi((u8*)paras[1]);
    }

    if (count == 2) {
        if (m_http_para[httpid].read_data_info.data_type ==
            MOBILE_HTTP_HEADER_DATA)
            snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_READ_RESP"%d,%d,%d",
                     httpid, m_http_para[httpid].read_data_info.data_type,
                     m_http_para[httpid].read_data_info.unread_header_len);

        if (m_http_para[httpid].read_data_info.data_type ==
            MOBILE_HTTP_CONTENT_DATA)
            snprintf(res, MOBILE_HTTP_URC_HEADER_LEN, HTTP_READ_RESP"%d,%d,%d",
                     httpid, m_http_para[httpid].read_data_info.data_type,
                     m_http_para[httpid].read_data_info.unread_content_len);

        transmit_mobile_resp_str(at_ptr, res);
    }

    if (count == 3) {
        if(mobile_at_para_range_check((u8 *)paras[2], 0, MOBILE_HTTP_MAX_CACHE_LEN))
            goto ERROR_OUT;

        if (atoi((u8*)paras[2]) == 0)
            m_http_para[httpid].read_data_info.to_read_len =
                                                    MOBILE_HTTP_MAX_CACHE_LEN;
        else
            m_http_para[httpid].read_data_info.to_read_len = atoi((u8*)paras[2]);

        mobile_http_read_data_mem_alloc(httpid);
        if (m_http_para[httpid].read_data_info.data_type ==
            MOBILE_HTTP_HEADER_DATA)
            mobile_http_response_header_read(at_ptr, httpid);
        else if (m_http_para[httpid].read_data_info.data_type ==
                 MOBILE_HTTP_CONTENT_DATA)
            mobile_http_response_content_read(at_ptr, httpid);
    }
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_read_data_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_READ_RESP"(0-3),(0-1),(0-65535)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Execution command to download file.
 */
#ifdef CONFIG_FS
u32 mobile_http_download_file_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    u8 range[32] = {'\0'};
    ulong *paras;
    u16 count = 0;
    u8 *local_path = AIC_NULL;
    slong start = -1;
    slong end = -1;
    u32 progind = 0;
    const u8 *flag = "-";
    char *next = AIC_NULL;
    u8 *temp;
    u8 eof = 0;
    u8 ssl_enable;
    u8 httpid = MOBILE_HTTP_INVALID_ID;
    u8 ssl_id = MOBILE_HTTP_INVALID_ID;
    MOBILE_HTTP_MSG mobile_http_msg;
    u32 status = 0;
    u8 *found = AIC_NULL;
    u32 offset = 0;

    local_path = aos_zalloc(MOBILE_HTTP_MAX_LOCAL_PATH_LEN + 1);

    paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_DLFILE_CMD"=",
                                         &count);
    if (!paras || count < 2 || count > 6) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
        goto OUT;
    }

    httpid = mobile_http_idle_dl_httpid_find();
    if (httpid == MOBILE_HTTP_INVALID_ID) {
        aic_reply_error_resp(at_ptr, MOBILE_HTTP_NO_IDLE_CLIENT);
        goto OUT;
    }

    m_http_para[httpid].downloading = 1;
    m_http_para[httpid].httpid = httpid;
    m_http_para[httpid].urc_port = at_ptr->serial_port_id;

    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.url,
                               MOBILE_HTTP_MAX_URL_LEN);
    if (status)
        goto ERROR_OUT;

    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.host,
                               MOBILE_HTTP_MAX_HOST_LEN);
    if (status)
        goto ERROR_OUT;

    status =  mobile_mem_alloc(&m_http_para[httpid].connect_info.resource,
                               MOBILE_HTTP_MAX_RESOURCE_LEN);
    if (status)
        goto ERROR_OUT;

    if (paras && count > 1) {
        if (strlen((u8*)paras[1]) == 0)
            goto ERROR_OUT;

        strncpy(m_http_para[httpid].connect_info.url, (u8*)paras[0],
                MOBILE_HTTP_MAX_URL_LEN);

        aos_memcpy(local_path, MOBILE_HTTP_HOME_PATH,
                strlen(MOBILE_HTTP_HOME_PATH));
        if (strncmp((u8*)paras[1], "/", 1) == 0) {
            //strncpy(local_path + strlen(local_path), (u8*)paras[1], strlen((u8*)paras[1]));
            found = (u8*)paras[1] + 1;
            do {
                found = strchr(found, '/');
                if (found) {
                    found++;
                    offset = strlen(found);
                } else {
                    if (offset == 0)
                        offset = strlen((u8*)paras[1]); /*no secondary dir */
                    else
                        offset = strlen((u8*)paras[1]) - offset - 1;/* remove the last / , need to -1 */
                    break;
                }
            } while (found);

            /* only has secondary dir, need to mkdir */
            if (offset != strlen((u8*)paras[1])) {
                /*local_path used to mkdir*/
                strncpy(local_path + strlen(local_path), (u8*)paras[1], offset);
                status = mkdir(local_path, FS_O_RDWR);
                if (status < 0) {
                    aic_reply_error_resp(at_ptr, MOBILE_HTTP_URC_WRITE_FILE_FAILED);
                    goto OUT;
                }
            }

            /* reset local_path used to save file */
            aos_memset(local_path, '\0', MOBILE_HTTP_MAX_LOCAL_PATH_LEN + 1);
            aos_memcpy(local_path, MOBILE_HTTP_HOME_PATH, strlen(MOBILE_HTTP_HOME_PATH));
            strncpy(local_path + strlen(local_path), (u8*)paras[1], strlen((u8*)paras[1]));
        } else {
            snprintf(local_path + strlen(local_path),
                     MOBILE_HTTP_MAX_LOCAL_PATH_LEN, "%s%s",
                     MOBILE_HTTP_DEFAULT_DL_PATH, (u8*)paras[1]);

            status = mkdir(MOBILE_HTTP_MKDIR_PATH, FS_O_RDWR);
            if (status < 0) {
                aic_reply_error_resp(at_ptr, MOBILE_HTTP_URC_WRITE_FILE_FAILED);
                goto OUT;
            }
        }
    }

    if (paras && count > 2) {
        if (!aic_is_valid_integer_para((u8 *)paras[2], strlen((u8 *)paras[2])))
            goto ERROR_OUT;

        progind = atoi((u8 *)paras[2]);
    }

    if (paras && count > 3) {
        strncpy(range, (u8 *)paras[3], 32);
        temp = strtok_r(range + 6, flag, &next);
        if (temp)
            start = atoi(temp);
        if (next)
            end = atoi(next);
    }

    if (paras && count > 4) {
        if (!aic_is_valid_integer_para((u8 *)paras[4], strlen((u8 *)paras[4])))
            goto ERROR_OUT;

        eof = atoi((u8 *)paras[4]);
    }

    if (count == 5) {
        ssl_enable = mobile_http_ssl_enable_check((u8 *)paras[0]);
        if (ssl_enable) {
            aic_reply_error_resp(at_ptr, MOBILE_HTTP_SSL_NOT_ENABLED);
            mobile_http_paras_reset(httpid);
            goto OUT;
        }
    }

    if (paras && count > 5) {
        if(mobile_at_para_range_check((u8 *)paras[5], 0, MOBILE_HTTP_MAX_SSL_ID))
            goto ERROR_OUT;

        ssl_id = atoi((u8 *)paras[5]);
    }

    ssl_enable = mobile_http_ssl_enable_check((u8 *)paras[0]);

    mobile_http_msg.httpid = httpid;
    mobile_http_msg.msg_id = MOBILE_HTTP_DL_FILE;
    mobile_http_msg.ssl_id = ssl_id;
    mobile_http_msg.ssl_enable = ssl_enable;
    mobile_http_msg.dl_start = start;
    mobile_http_msg.dl_end = end;
    mobile_http_msg.urc_port = at_ptr->serial_port_id;
    mobile_http_msg.eof = eof;
    mobile_http_msg.local_path = local_path;
    mobile_http_msg.progind = progind;

    aos_send_to_queue(m_http_dl_q, &mobile_http_msg,
                      MOBILE_HTTP_DEFAULT_WAIT_TIME);

    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
    mobile_http_paras_reset(httpid);
OUT:
    aos_free(local_path);
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_download_file_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}
#endif

/**
 * Execution command delete the http client.
 */
u32 mobile_http_delete_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    ulong *paras;
    u16 count = 0;
    s8 httpid = MOBILE_HTTP_INVALID_ID;
    MOBILE_HTTP_MSG mobile_http_msg = {'\0'};

    paras = aic_split_paras_to_str_array(at_ptr->at_req, MHTTP_DEL_CMD"=",
                                         &count);
    if (!paras || count != 1)
        goto ERROR_OUT;

    if(mobile_at_para_range_check((u8 *)paras[0], 0, 3))
        goto ERROR_OUT;

    httpid = atoi((u8*)paras[0]);

    if (m_http_para[httpid].handle == 0 || m_http_para[httpid].downloading) {
        goto SUCCESS_OUT;
    }

    m_http_para[httpid].client_ptr->https_client->nx_web_http_client_state =
                                                NX_WEB_HTTP_CLIENT_STATE_READY;
    m_http_para[httpid].is_term = 1;

    mobile_http_msg.httpid = httpid;
    mobile_http_msg.msg_id = MOBILE_HTTP_CLIENT_DELETE;

    aos_send_to_queue(m_http_q, &mobile_http_msg,
                      MOBILE_HTTP_DEFAULT_WAIT_TIME);

SUCCESS_OUT:
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_delete_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_DEL_RESP"(0-3)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

/**
 * Execution command terminate the actived HTTP service.
 */
u32 mobile_http_term_at_set_cmd(AT_REQ_RESP *at_ptr)
{
    u32 i;
    ulong *paras;
    u16 count = 0;
    s8 httpid = MOBILE_HTTP_INVALID_ID;

    if (at_ptr->at_req[strlen(HTTP_TERM_CMD)] == 0x0D ||
        at_ptr->at_req[strlen(HTTP_TERM_CMD)] == 0x0A) {

        for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
            if (m_http_para[i].handle != 0 && m_http_para[i].client_ptr) {
                m_http_para[i].is_term = 1;
                m_http_para[i].client_ptr->https_client->nx_web_http_client_state =
                                                NX_WEB_HTTP_CLIENT_STATE_READY;

                nx_web_http_client_cleanup(m_http_para[i].client_ptr->https_client);

                nx_tcp_socket_disconnect(
                    &(m_http_para[i].client_ptr->https_client->nx_web_http_client_socket), 500);
                nx_tcp_client_socket_unbind(
                    &(m_http_para[i].client_ptr->https_client->nx_web_http_client_socket));

                aic_http_packet_release(&m_http_para[i].client_ptr->res_data_packet);
            }
        }
        aic_reply_ok_resp(at_ptr);
        return AIC_SUCCESS;
    } else {
        paras = aic_split_paras_to_str_array(at_ptr->at_req, HTTP_TERM_CMD"=",
                                             &count);
        if (!paras || count != 1)
            goto ERROR_OUT;

        if(mobile_at_para_range_check((u8 *)paras[0], 0, 3))
            goto ERROR_OUT;

        httpid = atoi((u8*)paras[0]);
        if (m_http_para[httpid].handle == 0 || m_http_para[httpid].downloading)
            goto SUCCESS_OUT;

        m_http_para[httpid].client_ptr->https_client->nx_web_http_client_state =
                                            NX_WEB_HTTP_CLIENT_STATE_READY;
        m_http_para[httpid].is_term = 1;
        nx_web_http_client_cleanup(m_http_para[httpid].client_ptr->https_client);

        nx_tcp_socket_disconnect(
            &(m_http_para[httpid].client_ptr->https_client->nx_web_http_client_socket), 500);
        nx_tcp_client_socket_unbind(
            &(m_http_para[httpid].client_ptr->https_client->nx_web_http_client_socket));

        aic_http_packet_release(&m_http_para[httpid].client_ptr->res_data_packet);
    }

SUCCESS_OUT:
    aic_reply_ok_resp(at_ptr);
    aic_free_paras_mem(paras, count);
    return AIC_SUCCESS;

ERROR_OUT:
    aic_reply_error_resp(at_ptr, MOBILE_HTTP_PARAMETER_ERROR);
    aic_free_paras_mem(paras, count);
    return MOBILE_HTTP_PARAMETER_ERROR;
}

u32 mobile_http_term_at_test_cmd(AT_REQ_RESP *at_ptr)
{
    transmit_mobile_resp_str(at_ptr, HTTP_TERM_RESP"(0-3)");
    aic_reply_ok_resp(at_ptr);
    return AIC_SUCCESS;
}

int mobile_http_init(void)
{
    int i;

#ifdef CONFIG_FS
    m_http_dl_q = aos_create_queue(MOBILE_HTTP_DL_QUEUE_NAME,
                                            sizeof(MOBILE_HTTP_MSG),
                                            MOBILE_HTTP_QUEUE_NUM);
    AOS_ASSERT(m_http_dl_q);

    m_http_dl_t = aos_create_thread("m_http_dl_t",m_http_dl_msg_process,
                                    NULL, M_HTTP_TASK_SIZE, 0, NULL,
                                    15, AOS_AUTO_START);

    AOS_ASSERT(m_http_dl_t != AOS_INVALID_THREAD_ID);
#endif
    m_http_q = aos_create_queue(MOBILE_HTTP_QUEUE_NAME, sizeof(MOBILE_HTTP_MSG),
                                MOBILE_HTTP_QUEUE_NUM);
    AOS_ASSERT(m_http_q);

    m_http_event_t = aos_create_thread("m_http_event_t", m_http_event_process,
                                       NULL, M_HTTP_TASK_SIZE, 0,
                                       NULL, 15, AOS_AUTO_START);

    AOS_ASSERT(m_http_event_t != AOS_INVALID_THREAD_ID);

    /* set default paras */
    for (i = 0; i < MOBILE_HTTP_MAX_CLIENT_NUM; i++) {
        moble_http_default_paras_set(i);
    }

    aic_add_at_cmd_handle(&m_mqtt_at_cmds[0]);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(mobile_http_init);

