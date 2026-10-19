/******************************************************************************/
/*                                                                            */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.      */
/*                                                                            */
/******************************************************************************/

#ifndef __CM_AT_COMMON_H__
#define __CM_AT_COMMON_H__



#define MOBILE_AT_MAX_PARA_RES_LEN 300
#define MOBILE_AT_COMMON_PARA_RES_LEN 64
#define MOBILE_AT_COMMON_SHORT_RES_LEN 32
#define MOBILE_RESERVED_VALUE -1
#define MOBILE_DNS_PRIO_V4 0
#define MOBILE_DNS_PRIO_V6 1

u32 transmit_mobile_at_ok_urc(u32 urc_port);
u32 transmit_mobile_at_cme_urc(u32 urc_port, u32 error_num);

u32 transmit_mobile_at_integer(AT_REQ_RESP *at_ptr, u8 *protocol, u8 *tag,
                               int value1, int value2, int value3, int vlaue4);

u32 transmit_mobile_at_str(AT_REQ_RESP *at_ptr, u8 *protocol, u8 *tag, u8 *str1,
                           u8 *str2, u8 *str3, u8 *str4);

u32 transmit_mobile_resp_str(AT_REQ_RESP *at_ptr, u8 *str);

bool mobile_is_hex_check(int ch);

void mobile_http_url_escape(u8 *url, int inlength, u8 *dest_url);

ulong *mobile_split_paras_to_str_array(u8 *raw_str, u8 *start_str,
                                       u16 *paras_cnt, u16 max_cnt,
                                       u16 data_index, u8 more_data_flag);

u8 mobile_specific_cmd_check(u8 *cmd);
u32 mobile_at_para_range_check(u8 *data, int min, int max);
u32 mobile_mem_alloc(u8 **buff, u32 size);
u8 mobile_http_ipv4_addr_check(u8 *host);

#endif

