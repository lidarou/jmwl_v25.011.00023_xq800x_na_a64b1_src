/**************************************************************************/
/*                                                                        */
/*  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the structs and apis for handling response of
 *    AT cmds relating to network.
 */

#ifndef AIC_NET_AT_RESP_HANDLE_H
#define AIC_NET_AT_RESP_HANDLE_H

#include "aic_at_cmd_handle.h"

/* Define public functions. */
s32 aic_handle_cops_query_resp(AT_REQ_RESP *at_to_ap);
#ifdef CONFIG_CUSTOMER_CM_AT
s32 aic_handle_ctzr_set_resp(AT_REQ_RESP *at_to_ap);
s32 aic_handle_ctzr_query_resp(AT_REQ_RESP *at_to_ap);
s32 aic_handle_cereg_set_resp(AT_REQ_RESP *at_to_ap);
s32 aic_handle_cereg_query_resp(AT_REQ_RESP *at_to_ap);
s32 aic_handle_cgerep_set_resp(AT_REQ_RESP *at_to_ap);
s32 aic_handle_cgerep_query_resp(AT_REQ_RESP *at_to_ap);
#endif
s32 aic_handle_cfun_exe_resp(AT_REQ_RESP *at_to_cp);

#endif /* AIC_NET_AT_RESP_HANDLE_H */
