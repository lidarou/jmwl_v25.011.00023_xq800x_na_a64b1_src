/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic Handling of AT Cmds Relating to TCPIP.
 *  This is the handler of AT commands relating to TCPIP,
 *  such as establish a tcp connection.etc.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x074af0
#define LOG_MODULE    AMOD_NET

#include <string.h>
#include <aic_size.h>
#include <aic_type.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <aic_errno.h>
#include "aic_at_tcpip_common.h"
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"

#ifdef CONFIG_NET_AT_TCPIP
#include "aic_at_tcpip.h"
extern AIC_SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM];
#endif
#ifdef CONFIG_CUSTOMER_AT
#include "aic_at_qtcpip.h"
#include "aic_at_qtls.h"
extern AIC_SOCKET_LIST sock_list_q[AT_SOCKET_MAX_CONN_NUM_Q];
extern AIC_SOCKET_LIST ssl_list_q[AT_SOCKET_MAX_CONN_NUM_Q];
extern struct at_sock_info sock_info;
#endif
#if defined CONFIG_CM_TCPIP_AT || defined CONFIG_GX_TCPIP_AT
#include "aic_at_mtcpip.h"
extern AIC_SOCKET_LIST sock_list[AT_SOCKET_MAX_CONN_NUM];
extern struct at_dns_cfg dns_cfg;
extern struct at_sock_info sock_info;
#endif

u32 last_errcode = 0;

int aic_at_find_socket_by_linkid(int link_id, int type)
{
#if defined CONFIG_NET_AT_TCPIP || defined NET_AT_MTCPIP
    int i, sock;
    if (type == AT_TCPIP || type == AT_MTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].link_id == link_id) {
                sock = sock_list[i].socket_id;
                if (sock != 0) {
                    return sock;
                }
            }
        }
    }
#endif
#ifdef CONFIG_CUSTOMER_AT
    int i, sock;
    if (type == AT_QTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].link_id == link_id) {
                sock = sock_list_q[i].socket_id;
                if (sock != 0) {
                    return sock;
                }
            }
        }
    }

    if (type == AT_QSSL) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (ssl_list_q[i].link_id == link_id) {
                sock = ssl_list_q[i].socket_id;
                if (sock != 0) {
                    return sock;
                }
            }
        }
    }
#endif

    return 0;
}

int aic_at_find_linkid_by_socket(int socket, int type)
{
#if defined CONFIG_NET_AT_TCPIP || defined NET_AT_MTCPIP
    int i, link_id;
    if (type == AT_TCPIP || type == AT_MTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].socket_id == socket) {
                link_id = sock_list[i].link_id;
                return link_id;
            }
        }
    }
#endif
#ifdef CONFIG_CUSTOMER_AT
    int i, link_id;
    if (type == AT_QTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].socket_id == socket) {
                link_id = sock_list_q[i].link_id;
                return link_id;
            }
        }
    }

    if (type == AT_QSSL) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (ssl_list_q[i].socket_id == socket) {
                link_id = ssl_list_q[i].link_id;
                return link_id;
            }
        }
    }
#endif

    return AIC_SUCCESS;
}

int aic_at_find_empty_socket(int type)
{
#if defined CONFIG_NET_AT_TCPIP || defined NET_AT_MTCPIP
    int i;
    if (type == AT_TCPIP || type == AT_MTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].socket_id == 0) {
                return i;
            }
        }
    }
#endif
#ifdef CONFIG_CUSTOMER_AT
    int i;
    if (type == AT_QTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].socket_id == 0) {
                return i;
            }
        }
    }

    if (type == AT_QSSL) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (ssl_list_q[i].socket_id == 0) {
                return i;
            }
        }
    }
#endif
    return AIC_EPERM;
}

int aic_at_find_index_by_socket(int socket, int type)
{
#if defined CONFIG_NET_AT_TCPIP || defined NET_AT_MTCPIP
    int i;
    if (type == AT_TCPIP || type == AT_MTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].socket_id == socket) {
                return i;
            }
        }
    }
#endif
#ifdef CONFIG_CUSTOMER_AT
    int i;
    if (type == AT_QTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].socket_id == socket) {
                return i;
            }
        }
    }

    if (type == AT_QSSL) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (ssl_list_q[i].socket_id == socket) {
                return i;
            }
        }
    }
#endif
    return AIC_EPERM;
}

int aic_at_find_index_by_linkid(int link_id, int type)
{
#if defined CONFIG_NET_AT_TCPIP || defined NET_AT_MTCPIP
    int i;
    if (type == AT_TCPIP || type == AT_MTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
            if (sock_list[i].link_id == link_id) {
                return i;
            }
        }
    }
#endif
#ifdef CONFIG_CUSTOMER_AT
    int i;
    if (type == AT_QTCPIP) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (sock_list_q[i].link_id == link_id) {
                return i;
            }
        }
    }

    if (type == AT_QSSL) {
        for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
            if (ssl_list_q[i].link_id == link_id) {
                return i;
            }
        }
    }
#endif
    return AIC_EPERM;
}

bool aic_get_socket_is_valid(void)
{
#if defined CONFIG_NET_AT_QTCPIP || defined NET_AT_MTCPIP
    int i;
#endif
#ifdef CONFIG_NET_AT_QTCPIP
    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM_Q; i++) {
        if (sock_info.sockid && sock_list_q[i].socket_id == sock_info.sockid) {
            return TRUE;
        }
    }
#endif
#ifdef NET_AT_MTCPIP
    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        if (sock_info.sockid &&
            sock_list[i].socket_id == sock_info.sockid) {
            return TRUE;
        }
    }
#endif

    return FALSE;
}

u32 aic_transmit_resp_normal(AT_REQ_RESP *at_req_resp_ptr, char *resp)
{
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    line_pointers[0] = (ulong)resp;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

#ifdef CONFIG_CUSTOMER_AT
u32 aic_transmit_resp_ok(AT_REQ_RESP *at_req_resp_ptr)
{
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    last_errcode = 0;
    strcpy(result_str, AT_RESULT_OK);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}

u32 aic_transmit_resp_error(AT_REQ_RESP *at_req_resp_ptr, int error)
{
    u8 result_str[AT_RESP_RESULT_MAX] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];

    last_errcode = error;
    snprintf(result_str, sizeof(result_str), "+CME ERROR: %d", error);
    line_pointers[0] = (ulong)result_str;
    aic_fill_transmit_cmd_resp(at_req_resp_ptr, line_pointers, 1);
    return AIC_SUCCESS;
}
#endif

int aic_at_string_to_num(u8 *src)
{
    u32 i, num = 0;

    for (i = 0; i < 4; i++) {
        if (*src >= '0' && *src <= '9')
            num = num * 16 + (*src - '0');
        else if (*src == 'a' || *src == 'A')
            num = num * 16 + 10;
        else if (*src == 'b' || *src == 'B')
            num = num * 16 + 11;
        else if (*src == 'c' || *src == 'C')
            num = num * 16 + 12;
        else if (*src == 'd' || *src == 'D')
            num = num * 16 + 13;
        else if (*src == 'e' || *src == 'E')
            num = num * 16 + 14;
        else if (*src == 'f' || *src == 'F')
            num = num * 16 + 15;
        else if (*src == '\0')
            return num;
        else
            return AIC_EFAULT;

        src++;
    }

    return num;
}

u32 aic_hexstring_to_char(u8 *dest, u8 *src, u32 len)
{
    u8 num = 0;
    int i, j = 0, offset = 0;

    while (len) {
        for (i = 0; i < 2; i++) {
            if (src[j] >= '0' && src[j] <= '9')
                num = num * 16 + (src[j] - '0');
            else if (src[j] == 'a' || src[j] == 'A')
                num = num * 16 + 10;
            else if (src[j] == 'b' || src[j] == 'B')
                num = num * 16 + 11;
            else if (src[j] == 'c' || src[j] == 'C')
                num = num * 16 + 12;
            else if (src[j] == 'd' || src[j] == 'D')
                num = num * 16 + 13;
            else if (src[j] == 'e' || src[j] == 'E')
                num = num * 16 + 14;
            else if (src[j] == 'f' || src[j] == 'F')
                num = num * 16 + 15;
            else
                return AIC_TCPIP_AT_ERR;

            j++;
        }
        dest[offset++] = num;

        num = 0;
        len -= 2;
    }

    return AIC_TCPIP_AT_OK;
}

#if 0
static int aic_data_is_hex(u8 *data)
{
    int num;

    if (data[0] >= '0' && data[0] <= '9')
        num = data[0] - '0';
    else if (data[0] == 'a' || data[0] == 'A')
        num = 10;
    else if (data[0] == 'b' || data[0] == 'B')
        num = 11;
    else if (data[0] == 'c' || data[0] == 'C')
        num = 12;
    else if (data[0] == 'd' || data[0] == 'D')
        num = 13;
    else if (data[0] == 'e' || data[0] == 'E')
        num = 14;
    else if (data[0] == 'f' || data[0] == 'F')
        num = 15;
    else
        num = -1;

    return num;
}

static int aic_data_is_octal(u8 *data)
{
    int num;

    if (data[0] >= '0' && data[0] <= '7')
        num = data[0] - '0';
    else
        num = -1;

    return num;
}

u32 aic_escape_data_normal(u8 *buffer, u8 *data, int datalen, int *actual_len)
{
    int num1, num2, num3, len, offset = 0;
    u8 *ptr, *temp_ptr;

    temp_ptr = data;

    ptr = strchr(temp_ptr, '\\');
    if (!ptr) {
        aos_memcpy(buffer, data, datalen);
        *actual_len = datalen;
    } else {
        do {
            len = ptr - temp_ptr;
            aos_memcpy(buffer + offset, temp_ptr, len);
            offset += len;

            if (ptr + 1 == data + datalen) {
                buffer[offset] = 0x5C;
                offset++;
                *actual_len = offset;
                return AIC_SUCCESS;
            }
            switch (*(ptr + 1)) {
            case 'a':
                buffer[offset] = 0x07;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'b':
                //buffer[offset] = 0x08;
                offset--;
                temp_ptr = ptr + 2;
                break;
            case 'f':
                buffer[offset] = 0x0C;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'n':
                buffer[offset] = 0x0A;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'r':
                buffer[offset] = 0x0D;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 't':
                buffer[offset] = 0x09;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'v':
                buffer[offset] = 0x0B;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '\\':
                buffer[offset] = 0x5C;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '\'':
                buffer[offset] = 0x27;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '"':
                buffer[offset] = 0x22;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case '?':
                buffer[offset] = 0x3F;
                offset++;
                temp_ptr = ptr + 2;
                break;
            case 'x':
                num1 = aic_data_is_hex(ptr + 2);
                if (num1 >= 0) {
                    num2 = aic_data_is_hex(ptr + 3);
                    if (num2 >= 0) {
                        buffer[offset] = num1 * 16 + num2;
                        offset++;
                        temp_ptr = ptr + 4;
                    } else {
                        buffer[offset] = num1;
                        offset++;
                        temp_ptr = ptr + 3;
                    }
                } else {
                    buffer[offset] = 0x5C;
                    buffer[offset + 1] = 'x';
                    offset += 2;
                    temp_ptr = ptr + 2;
                }
                break;
            default:
                num1 = aic_data_is_octal(ptr + 1);
                if (num1 >= 0) {
                    num2 = aic_data_is_octal(ptr + 2);
                    if (num2 >= 0) {
                        num3 = aic_data_is_octal(ptr + 3);
                        if (num3 >= 0) {
                            if (num1 >= 0 && num1 < 4) {
                                buffer[offset] = num1 * 8 * 8 + num2 * 8 + num3;
                                offset++;
                                temp_ptr = ptr + 4;
                            } else {
                                buffer[offset] = num1 * 8 + num2;
                                offset++;
                                temp_ptr = ptr + 3;
                            }
                        } else {
                            buffer[offset] = num1 * 8 + num2;
                            offset++;
                            temp_ptr = ptr + 3;
                        }
                    } else {
                        buffer[offset] = num1;
                        offset++;
                        temp_ptr = ptr + 2;
                    }
                }else {
                    buffer[offset] = 0x5C;
                    offset++;
                    temp_ptr = ptr + 1;
                }
                break;
            }

            ptr = strchr(temp_ptr, '\\');
            if (!ptr) {
                len = data + datalen - temp_ptr;
                aos_memcpy(buffer + offset, temp_ptr, len);
                offset += len;
            }
        }while (ptr);

        *actual_len = offset;
    }

    return AIC_SUCCESS;
}
#endif

void aic_print_data_to_hex(u8 *data, u32 length, u8 *out_data)
{
    int i = 0;
    u8 *temp;

    temp = out_data;
    for (i = 0; i < length; i++) {
        sprintf(temp, "%02X", data[i]);

        temp += 2;
    }
}

#if defined CONFIG_CUSTOMER_CM_AT || defined CONFIG_GX_AT_V2
ulong *aic_at_get_para_array(u8 *raw_str, u8 *start_str, u16 *paras_cnt,
                              u16 max_cnt, u16 data_index)
{
    u16 raw_str_len, para_str_len;
    u8 cnt = 0, i;
    ulong *str_arr = AIC_NULL;
    u8 *str_pointer[30] = {0};
    u8 *paras_ptr;
    u8 *work_ptr;
    u8 *found;
    int offset_len = 0;
    u8 at_end_flag_len = 0;

    if (!raw_str || !start_str)
        return AIC_NULL;

    if (strlen(raw_str) <= strlen(start_str) || !strcasestr(raw_str, start_str)) {
        aicx_info(0x08d8b8, "", "[TCPIP] split paras error, no paras\n");
        *paras_cnt = 0;
        return AIC_NULL;
    }

    raw_str_len = strlen(raw_str);
    if (!is_cr_lf_sign(raw_str[raw_str_len - 1])) {
        aicx_info(0x15a31d, "", "[TCPIP] not fount the AT cmd end\n");
        *paras_cnt = 0;
        return AIC_NULL;
    } else
        at_end_flag_len++;

    if (is_cr_lf_sign(raw_str[raw_str_len - 2]))
        at_end_flag_len++;

    paras_ptr = aos_zalloc_relaxed(raw_str_len + 1);
    if (!paras_ptr) {
        *paras_cnt = 0;
        return AIC_NULL;
    }

    strncpy(paras_ptr, raw_str + strlen(start_str),
            (raw_str_len - at_end_flag_len - strlen(start_str)));
    para_str_len = strlen(paras_ptr);

    work_ptr = paras_ptr;
    do {
        if (data_index && cnt == data_index - 1) {
            str_pointer[cnt] = aos_zalloc(strlen(work_ptr) + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[strlen(work_ptr) - 1] != '"') {
                    aicx_error(0x1a303d, "", "[TCPIP]  not fount other quotation in last data\n");
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
            } else {
                if (work_ptr[strlen(work_ptr) - 1] == '"')
                    goto SPLIT_ERROR;

                strncpy(str_pointer[cnt], work_ptr, strlen(work_ptr));
            }
            cnt++;
            break;
        }

        found = strchr(work_ptr, ',');
        if (found) {
            offset_len = para_str_len - strlen(found);
            para_str_len = para_str_len - offset_len - 1;

            str_pointer[cnt] = aos_zalloc(offset_len + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            //parameter with quotes("), remove commas(,)
            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[offset_len - 1] != '"') {
                    aicx_error(0x1cf2ec, "", "[TCPIP]  not fount other quotation\n");
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, offset_len - 2);
            } else {
                strncpy(str_pointer[cnt], work_ptr, offset_len);
            }

            work_ptr = work_ptr + offset_len + 1;
            found++;
            cnt++;
            if (strncmp(found, ",", 1) == 0) {
                /* Two continuous ',',that means there is an empty para. */
                str_pointer[cnt] = aos_zalloc(2);
                if (!str_pointer[cnt])
                    goto SPLIT_ERROR;
                strncpy((u8 *)str_pointer[cnt], "", 1);
                cnt++;
                found++;
                para_str_len--;
                work_ptr++;
            }
        } else {
            str_pointer[cnt] = aos_zalloc(strlen(work_ptr) + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            if (work_ptr[0] == '"') {
                //the quotation marks(") should be included both the begin & end
                if (work_ptr[strlen(work_ptr) - 1] != '"') {
                    aicx_info(0x020e1b, "", "[tcpip] not fount other quotation 333\n");
                    goto SPLIT_ERROR;
                }

                strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
            } else {
                strncpy(str_pointer[cnt], work_ptr, strlen(work_ptr));
            }

            cnt++;
            break;
        }
    } while (cnt < max_cnt);

    if (cnt > 0) {
        str_arr = (ulong *)aos_malloc(sizeof(ulong) * cnt);
        if (!str_arr) {
            aicx_error(0x1e180c, "", "[tcpip]:malloc str_arr fail\n");
            goto SPLIT_ERROR;
        }

        for (i = 0; i < cnt; i++) {
            str_arr[i] = (ulong)str_pointer[i];
            aicx_info(0x0f9d99, "ds", "[TCPIP] str_arr[%d] is %s\n", i, str_arr[i]);
        }
    }
    *paras_cnt = cnt;
    aos_free(paras_ptr);

    return str_arr;

SPLIT_ERROR:
    *paras_cnt = 0;

    aos_free(paras_ptr);
    for (i = 0; i < cnt + 1; i++) {
        if (str_pointer[i])
            aos_free(str_pointer[i]);
    }

    return AIC_NULL;
}
#endif

boolean aic_is_transparent_mode(void)
{
    boolean result = FALSE;
#ifdef NET_AT_MTCPIP
    int i;

    for (i = 0; i < AT_SOCKET_MAX_CONN_NUM; i++) {
        if (sock_list[i].mode == 1)
            result = TRUE;
    }
#endif

    return result;
}

int aic_get_dns_prio(void)
{
#ifdef NET_AT_MTCPIP
    return dns_cfg.dns_prio;
#endif
    return 0;
}


