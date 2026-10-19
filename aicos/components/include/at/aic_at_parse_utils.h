/**************************************************************************/
/*                                                                        */
/* Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  This file defines the macros and apis for parsing AT cmds and responses.
 */

#ifndef AIC_AT_PARSE_UTILS_H
#define AIC_AT_PARSE_UTILS_H

#include <aic_at_process_common.h>

#define AIC_INVALID_TIME_ZONE 100
/* Default time zone is the 8th zone,in quarters of an hour */
#define AIC_DEFAULT_TIME_ZONE_QUARTERS (4 * 8)
#define AIC_DEFAULT_TIME_ZONE 8

/*The max timezone is 48 plus two hours of daylight saving time. */
#define AIC_MIN_TIME_ZONE_QUARTERS -48
#define AIC_MAX_TIME_ZONE_QUARTERS 56

#define AIC_MIN_TIME_ZONE -24
#define AIC_MAX_TIME_ZONE 24

void aic_delete_start_end_symbols(u8 *dst_str, u8 *raw_str, u16 len_str);
u32 aic_parse_civil_time(u8 *date_time, aos_systime *current_time);
void aic_adjust_time_with_time_zone(aos_systime *current_time, s32 time_zone);
boolean aic_is_leap_year(u32 year);
s32 aic_get_current_time_zone(void);
s32 aic_get_time_zone_in_quarters(void);
int aic_set_current_time_zone(s32 time_zone);
int aic_set_time_zone_in_quarters(s32 time_zone);
boolean aic_is_valid_time_zone(s32 time_zone);
boolean aic_is_valid_time_zone_quarters(s32 time_zone);
void aic_convert_numeric_ipv6_to_str(ulong *addr_numeric, u8 *addr_buf, u8 buf_len);
boolean aic_is_powering_off(void);
#ifdef CONFIG_D2D
void aic_at_prepend_msg_head(u8 *dst_ptr, AT_REQ_RESP *at_req_ptr, u32 resp_len);
boolean aic_at_with_msg_head(AT_REQ_RESP *at_req_ptr);
u32 aic_get_urc_msg_type(u8 *urc_ptr, u16 urc_len, u16 *msg_type_ptr);
u32 aic_urc_prepend_msg_head(u8 *dst_ptr, u8 *urc_str, u32 urc_len);
u32 aic_add_urc_msg_info(AT_D2D_URC_MSG_INFO *head_msg_info_ptr);

#define AT_MAX_D2D_URC_CNT 40
/* Just an example. */
#define AT_D2D_URC_REVMSG "+DREVMSG"
#define AT_D2D_URC_RVOIDATA "+DRVOIDATA"
#define AT_D2D_URC_REPDR "+DREPDR"
#define AT_D2D_URC_DATAFC "+DDATAFC"
#define AT_D2D_URC_DSQ "+DSQ"
/* Use invalid msg type if not found. */
#define AT_D2D_URC_INVALID_MSG_TYPE 0xffff

/* The msg type values are determined by the customer.For example: */
enum {
    AT_D2D_URC_REVMSG_MSG_TYPE = 0x0101,
    AT_D2D_URC_RVOIDATA_MSG_TYPE,
    AT_D2D_URC_REPDR_MSG_TYPE,
    AT_D2D_URC_DATAFC_MSG_TYPE,
    AT_D2D_URC_DSQ_MSG_TYPE,
};

#define AT_ERROR_INVALID_MSG_INFO 100
#define AT_ERROR_MSG_INFO_OVERFLOW 101
#endif

#endif
