/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Common function for mobile AT.
 *
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19d5db
#define LOG_MODULE    AMOD_NET

#include <aic_errno.h>
#include <aic_at_cmd_handle.h>
#include <aic_at_resp_handle.h>
#include <aic_at_parse_utils.h>
#include <strings.h>
#include <string.h>
#include <cm_at_common.h>

u32 transmit_mobile_at_integer(AT_REQ_RESP *at_ptr, u8 *protocol, u8 *tag,
                               int value1, int value2, int value3, int value4)
{
    u8 *result = AIC_NULL;

    result = aos_zalloc(2 * MOBILE_AT_COMMON_PARA_RES_LEN);
    if (!result)
        return 1;

    if (value1 >= 0)
        snprintf(result, MOBILE_AT_COMMON_SHORT_RES_LEN - 1,
                 "+%s: \"%s\",%d", protocol, tag, value1);

    if (value2 >= 0)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_SHORT_RES_LEN - 1, ",%d", value2);

    if (value3 >= 0)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_SHORT_RES_LEN - 1, ",%d", value3);

    if (value4 >= 0)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_SHORT_RES_LEN - 1, ",%d", value4);

    aic_transmit_at_result(at_ptr->serial_port_id, result, strlen(result));
    aos_free(result);
    return AIC_SUCCESS;
}

u32 transmit_mobile_at_ok_urc(u32 urc_port)
{
    u8 result[5] = {'\0'};

    strncpy(result, AT_RESULT_OK, strlen(AT_RESULT_OK));
    aic_transmit_at_cmd_urc(urc_port, result, strlen(result));

    return AIC_SUCCESS;
}

u32 transmit_mobile_at_cme_urc(u32 urc_port, u32 error_num)
{
    u8 result[32] = {'\0'};

    snprintf(result, 32, "+CME ERROR: %d", error_num);
    aic_transmit_at_cmd_urc(urc_port, result, strlen(result));

    return AIC_SUCCESS;
}

u32 transmit_mobile_at_str(AT_REQ_RESP *at_ptr, u8 *protocol, u8 *tag, u8 *str1,
                           u8 *str2, u8 *str3, u8 *str4)
{
    u8 *result = AIC_NULL;

    result = aos_zalloc(2 * MOBILE_AT_MAX_PARA_RES_LEN);
    if (!result)
        return 1;

    if (tag)
        snprintf(result, MOBILE_AT_COMMON_PARA_RES_LEN, "+%s: \"%s\"", protocol, tag);
    if (str1)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_PARA_RES_LEN, ",%s", str1);
    if (str2)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_PARA_RES_LEN, ",%s", str2);
    if (str3)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_PARA_RES_LEN, ",%s", str3);
    if (str4)
        snprintf(result + strlen(result), MOBILE_AT_COMMON_PARA_RES_LEN, ",%s", str4);

    aic_transmit_at_result(at_ptr->serial_port_id, result, strlen(result));

    aos_free(result);
    return AIC_SUCCESS;
}

u32 transmit_mobile_resp_str(AT_REQ_RESP *at_ptr, u8 *str)
{
    aic_fill_transmit_cmd_resp(at_ptr, (ulong *)(&str), 1);

    return AIC_SUCCESS;
}

bool mobile_is_hex_check(int ch)
{
    if ((ch >= '0' && ch < '9') ||
        (ch >= 'a' && ch <= 'f') ||
        (ch >= 'A' && ch <= 'F'))
        return AIC_TRUE;

    return AIC_FALSE;
}

/*static bool mobile_is_digit_check(int ch)
{
    if (ch >= '0' && ch < '9')
        return AIC_TRUE;

    return AIC_FALSE;
}

static int mobile_hex_transfer(char ch)
{
    if (mobile_is_digit_check(ch))
        return ch - '0';

    if ((ch >= 'a' && ch <= 'f') ||
        (ch >= 'A' && ch <= 'F')) {
        switch(ch) {
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            break;
        }
    } else {
        return -1;
    }

    return -1;
}

int mobile_hex_to_int(u8 *src_data)
{
    int res =0, i= 0;

    while(src_data[i] != '\0') {
        if (mobile_hex_transfer(src_data[i]) < 0)
            return -1;

        res = res * 16 + mobile_hex_transfer(src_data[i]);
        i++;
    }

    return res;
}

int mobile_octal_to_int(u8 *src_data)
{
    int res =0, i= 0;

    while(src_data[i] != '\0') {
        res = res * 8 + src_data[i] - '0';
        i++;
    }

    return res;
}

int mobile_input_data_char_escape(u8 in)
{
  switch(in) {
    case 'a':
        return 7;
    case 'f':
        return 12;
    case 'n':
        return 10;
    case 'r':
        return 13;
    case 't':
        return 9;
    case 'v':
        return 11;
    case '\\':
        return 92;
    case '\'':
        return 39;
    case '"':
        return 34;
    case '?':
        return 63;
    default:
      break;
  }
  return -1;
}

int mobile_input_data_digit_octal_escape(u8 *src_data, u32 *count)
{
    u8 temp[3] = {'\0'};

    if (atoi(src_data) < 377) {
        *count = 3;
        return mobile_octal_to_int(src_data);
    } else {
        aos_memcpy(temp, src_data, 2);
        if (atoi(temp) < 77) {
            *count = 2;
            return mobile_octal_to_int(temp);
        } else {
            if (src_data[0] < 8) {
                *count = 1;
                return src_data[0];
            } else {
                return -1;
            }
        }
    }

    return -1;
}

int mobile_input_data_digit_hex_escape(u8 *src_data, u32 *count)
{
    int res = -1;

    res = mobile_hex_to_int(src_data);
    if (res > 0) {
        if (mobile_hex_transfer(src_data[1]) < 0) {
            *count = 2;
        } else {
            *count = 3;
        }
    } else {
        *count = 1; //we need to add 'x' as a char
    }

    return res;
}

void mobile_input_data_escape_ext(char *dst_data, u8 *src_data, int inlength,
                                  u32 *outlen)
{
    int count = 0, i = 0;
    int escape_data;
    char temp_octal[4] = {'\0'};
    char temp_hex[3] = {'\0'};
    u32 num;
    u32 escape_len;

    if(inlength < 0)
        return;

    escape_len = inlength;

    while(inlength--) {
        u8 in = src_data[i];
        if(in == '\\') {
            i++;
            in = src_data[i];

            if (in == 'b') {
                //backspace delete the pre char
                count --;
                escape_len -= 3; // \b 2chars and delete 1 char,so -3
                i++;
                continue;
            }

            escape_data = mobile_input_data_char_escape(in);
            if (escape_data > 0) {
                dst_data[count] = escape_data;
                count++;
                i++;
                escape_len --;
            } else {
                if (in == 'x') {
                    aos_memcpy(temp_hex, src_data + i + 1, 2);
                    escape_data = mobile_input_data_digit_hex_escape(temp_hex, &num);
                    if (escape_data > 0) {
                        dst_data[count] = escape_data;
                        count++;
                        i += num;
                        escape_len -= num;
                        inlength -= 2;
                    } else {
                        dst_data[count] = '\\';
                        dst_data[count + 1] = in;
                        count+= 2;
                        i++;
                        //escape_len -= 1;
                    }
                } else if (mobile_is_digit_check(in)) {
                    aos_memcpy(temp_octal, src_data + i, 3);
                    escape_data = mobile_input_data_digit_octal_escape(temp_octal, &num);
                    if (escape_data > 0) {
                        dst_data[count] = escape_data;
                        count++;
                        i+= num;
                        escape_len -= num;
                        inlength -= 3;
                    } else {
                        dst_data[count] = '\\';
                        dst_data[count + 1] = in;
                        count+= 2;
                        i++;
                        //escape_len -= 1;
                    }
                } else {
                    dst_data[count] = '\\';
                    dst_data[count + 1] = in;
                    count+= 2;
                    i++;
                    //escape_len -= 1;
                }
            }
        } else {
            dst_data[count] = in;
            count ++;
            i++;
        }
    }

    *outlen = escape_len;
    return;
}*/

static bool mobile_http_url_isunreserved(u8 in)
{
  switch(in) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o':
    case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E':
    case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O':
    case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '-': case '.': case '_': case '~': case '!':
    case '*': case '`': case '(': case ')': case ';':
    case '/': case '?': case ':': case '@': case '&':
    case '=': case '+': case '$': case ',': case '#':
      return AIC_TRUE;
    default:
      break;
  }
  return AIC_FALSE;
}

void mobile_http_url_escape(u8 *url, int inlength, u8 *dest_url)
{
    size_t length;
    int count = 0;

    if(inlength < 0)
        return;

    length = (inlength?(size_t)inlength:strlen(url));

    while(length--) {
        u8 in = *url++; /* treat the characters unsigned */

        if(mobile_http_url_isunreserved(in)) {
            /* append this */
            dest_url[count] = in;
            count++;
        } else {
            /* encode it */
            const char hex[] = "0123456789ABCDEF";
            char out[3]={'%'};
            out[1] = hex[in>>4];
            out[2] = hex[in & 0xf];
            aos_memcpy(dest_url + count, out, 3);
            count+= 3;
        }
    }

    return;
}

ulong *mobile_split_paras_to_str_array(u8 *raw_str, u8 *start_str,
                                       u16 *paras_cnt, u16 max_cnt,
                                       u16 data_index, u8 more_data_flag)
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
    int data_len1 = 0;
    int data_len2 = 0;

    if (!raw_str || !start_str)
        return AIC_NULL;

    if (strlen(raw_str) <= strlen(start_str) || !strcasestr(raw_str, start_str)) {
        *paras_cnt = 0;
        return AIC_NULL;
    }

    raw_str_len = strlen(raw_str);
    if (!is_cr_lf_sign(raw_str[raw_str_len - 1])) {
        *paras_cnt = 0;
        return AIC_NULL;
    } else
        at_end_flag_len ++;

    if (is_cr_lf_sign(raw_str[raw_str_len - 2]))
        at_end_flag_len ++;

    paras_ptr = aos_zalloc(raw_str_len + 1);
    strncpy(paras_ptr, raw_str + strlen(start_str),
            (raw_str_len - at_end_flag_len - strlen(start_str)));
    para_str_len = strlen(paras_ptr);

    work_ptr = paras_ptr;
    do {
        if (data_index && cnt == data_index - 1) {
            if (more_data_flag) {
                found = strstr(work_ptr, "\",\"");
                if (found) {
                found += 2;

                data_len2 = strlen(found);
                /* remove the last ',' */
                data_len1 = strlen(work_ptr) - data_len2 - 1;

                str_pointer[cnt] = aos_zalloc(data_len1 + 1);

                strncpy(str_pointer[cnt], work_ptr, data_len1);
                /* check " " */
                if (str_pointer[cnt][0] == '"') {
                    if (str_pointer[cnt][data_len1 - 1] != '"') {
                        goto SPLIT_ERROR;
                    }
                } else {
                    goto SPLIT_ERROR;
                }

                aos_memset(str_pointer[cnt], '\0', data_len1);
                /* remove "" , length need -2 */
                strncpy(str_pointer[cnt], work_ptr + 1, data_len1 -2);

                cnt++;
                str_pointer[cnt] = aos_zalloc(data_len2 + 1);
                if (!str_pointer[cnt])
                    goto SPLIT_ERROR;

                strncpy(str_pointer[cnt], work_ptr + 1 + data_len1, data_len2);
                if (str_pointer[cnt][0] == '"') {
                    if (str_pointer[cnt][data_len2 - 1] != '"') {
                        goto SPLIT_ERROR;
                    }
                } else {
                    goto SPLIT_ERROR;
                }
                aos_memset(str_pointer[cnt], '\0', data_len2);
                /* remove "" , length need -2 */
                strncpy(str_pointer[cnt], work_ptr + 2 + data_len1, data_len2 - 2);
                cnt++;
                break;

                } else {
                    goto SPLIT_ERROR;
                }
            } else {
                str_pointer[cnt] = aos_zalloc(strlen(work_ptr) + 1);

                if (work_ptr[0] == '"') {
                    if (work_ptr[strlen(work_ptr) - 1] != '"') {
                        goto SPLIT_ERROR;
                    }
                    strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
                } else {
                    if (work_ptr[strlen(work_ptr) - 1] == '"')
                        goto SPLIT_ERROR;
                    else
                        strncpy(str_pointer[cnt], work_ptr, strlen(work_ptr));
                }
                cnt++;
                break;
            }
        }

        found = strchr(work_ptr, ',');
        if (found) {
            offset_len = para_str_len - strlen(found);
            para_str_len = para_str_len - offset_len - 1;
            str_pointer[cnt] = aos_zalloc(offset_len + 1);
            if (!str_pointer[cnt])
                goto SPLIT_ERROR;

            if (work_ptr[0] == '"') {
                if (work_ptr[offset_len - 1] != '"') {
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, offset_len - 2);
            } else {
                if (work_ptr[offset_len - 1] == '"')
                    goto SPLIT_ERROR;
                else
                    strncpy(str_pointer[cnt], work_ptr, offset_len);
            }
            work_ptr = work_ptr + offset_len + 1;
            found++;
            cnt++;
            if (strncmp(found, ",", 1) == 0) {
                /* Two continuous ',',that means there is an empty para. */
                if (more_data_flag)
                    goto SPLIT_ERROR;

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

            if (work_ptr[0] == '"') {
                if (work_ptr[strlen(work_ptr) - 1] != '"') {
                    *paras_cnt = 0;
                    goto SPLIT_ERROR;
                }
                strncpy(str_pointer[cnt], work_ptr + 1, strlen(work_ptr) - 2);
            } else {
                if (work_ptr[strlen(work_ptr) - 1] == '"')
                    goto SPLIT_ERROR;
                else
                    strncpy(str_pointer[cnt], work_ptr, strlen(work_ptr));
            }
            cnt++;
            break;
        }
    } while (cnt < max_cnt && strlen(work_ptr));

    if (cnt > 0) {
        str_arr = (ulong *)aos_malloc(sizeof(ulong) * cnt);
        if (!str_arr) {
            goto SPLIT_ERROR;
        }

        for (i = 0; i < cnt; i++) {
            str_arr[i] = (ulong)str_pointer[i];
            //alog_error("para %d is %s\n", i, str_arr[i]);
        }
    }
    *paras_cnt = cnt;
    aos_free(paras_ptr);
    return str_arr;

SPLIT_ERROR:
    *paras_cnt = 0;

    aos_free(paras_ptr);
    /* cnt need add 1. because the space has been applied,but cnt do not add when split error */
    for (i = 0; i < cnt + 1; i++) {
        if (str_pointer[i]) {
            //alog_error("para %d is %s\n", i, str_pointer[i]);
            aos_free(str_pointer[i]);
        }
    }

    return AIC_NULL;
}

u8 mobile_specific_cmd_check(u8 *cmd)
{
    u8 *found = AIC_NULL;

    found = strstr(cmd, "willpayload");
    if (found)
        return 1;
    else {
        found = strstr(cmd, "header");
        if (found)
            return 1;
        else
            return 0;
    }

    return 0;
}

u32 mobile_at_para_range_check(u8 *data, int min, int max)
{
    if (!aic_is_valid_integer_para(data, strlen(data)))
        return 1;

    if (atoi(data) < min || atoi(data) > max)
        return 1;

    return 0;
}

u32 mobile_mem_alloc(u8 **buff, u32 size)
{
    if (!(*buff)) {
        *buff = aos_zalloc_relaxed(size + 4);
        if (!(*buff))
            return 1;
    } else
        aos_memset(*buff, '\0', size + 4);

    return 0;
}

u8 mobile_http_ipv4_addr_check(u8 *host)
{
    const u8 *flag = ".";
    char *next = AIC_NULL;
    char *buf = AIC_NULL;
    char *num1, *num2, *num3, *num4;
    u8 find_count = 0;

    buf = aos_zalloc(strlen(host) + 1);
    strncpy(buf, host, strlen(host));

    num1 = strtok_r(buf, flag, &next);
    if (num1) {
        if (atoi(num1) <= 255 && atoi(num1) > 0)
            find_count++;
    } else {
        goto NOT_FOUND;
    }

    num2 = strtok_r(AIC_NULL, flag, &next);
    if (num2) {
        if (atoi(num2) <= 255 && atoi(num2) > 0)
            find_count++;
    } else {
        goto NOT_FOUND;
    }

    num3 = strtok_r(AIC_NULL, flag, &next);
    if (num3) {
        if (atoi(num3) <= 255 && atoi(num3) > 0)
            find_count++;
    } else {
        goto NOT_FOUND;
    }

    num4 = next;
    if (num4) {
        if (atoi(num4) <= 255 && atoi(num4) > 0)
            find_count++;
    } else {
        goto NOT_FOUND;
    }

    if (find_count == 4) {
        aos_free(buf);
        return 1;
    } else {
        goto NOT_FOUND;
    }

NOT_FOUND:
    aos_free(buf);
    return 0;
}

