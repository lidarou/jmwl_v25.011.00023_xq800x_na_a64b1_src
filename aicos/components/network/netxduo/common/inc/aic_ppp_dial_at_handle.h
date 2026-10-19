/**************************************************************************/
/*                                                                        */
/*  Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the apis for handling AT cmds
 *    relating to ppp dial.
 */

#ifndef AIC_PPP_DIAL_AT_HANDLE_H
#define AIC_PPP_DIAL_AT_HANDLE_H

#define ANET_PPP_DIAL_ERROR_RESULT "NO CARRIER"

/* Define global functions. */
u32 aic_handle_pppshare_set_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_pppshare_test_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_pppshare_query_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_atd_exe_cmd(AT_REQ_RESP *at_req_res_ptr);
u32 aic_handle_ath_exe_cmd(AT_REQ_RESP *at_req_resp_ptr);
AT_CMD_HANDLE *aic_get_ppp_dial_cmd_handle(void);

#endif /* AIC_PPP_DIAL_AT_HANDLE_H */
