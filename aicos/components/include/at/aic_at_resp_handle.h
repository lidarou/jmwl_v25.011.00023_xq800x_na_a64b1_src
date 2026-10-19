/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis handling AT response.
 */

#ifndef AIC_AT_RESP_HANDLE_H
#define AIC_AT_RESP_HANDLE_H

#include "aic_at_process_common.h"

/* 3 seconds */
#define AT_CP_CMD_DEFAULT_TIMEOUT 3000
/* 6 seconds for ATD dial,not PPP dial. */
#define AT_TIMEOUT_SIX_SECONDS 6000
/* 10 seconds for AT+CFUN=0. */
#define AT_TIMEOUT_TEN_SECONDS 10000
/* 12 seconds. */
#define AT_TIMEOUT_TWELVE_SECONDS 12000
/* 15 seconds. */
#define AT_TIMEOUT_FIFTEEN_SECONDS 15000
/* 20 seconds. */
#define AT_TIMEOUT_TWENTY_SECONDS 20000
/* 30 seconds,timeout for time consuming cmds handled by CP */
#define AT_TIMEOUT_HALF_MINUTE 30000
/* 60 seconds */
#define AT_TIMEOUT_ONE_MINUTE 60000

#define AT_MAX_RESP_HANDLES 30

#define AT_RESULT_OK "OK"
#define AT_RESULT_CONNECT "CONNECT"
#define AT_SIMPLE_RESULT_ERROR "ERROR"
#define AT_VERBOSE_RESULT_ERROR "+CME ERROR"
#define AT_RESULT_NO_CARRIER "NO CARRIER"

#define AT_NO_RESP_HANDLE_INDEX -1

#define AT_TIMEOUT_RETRY_ONE_TIME 1
#define AT_TIMIEOUT_NO_RETRY 0

/* Define global functions. */
u32 aic_set_resp_handler(AT_REQ_RESP *at_req_resp);
u32 aic_set_timeout_handler(AT_REQ_RESP *at_req_resp);
void aic_net_at_resp_init(void);
//u8 get_retry_count(u8 *at_req);

#endif /* AIC_AT_RESP_HANDLE_H */
