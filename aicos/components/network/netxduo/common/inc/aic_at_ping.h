/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_PING_H__
#define __AIC_AT_PING_H__

#include <stdlib.h>
#include <string.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#include <nx_ip.h>
#include <aic_errno.h>
#include "aic_net_api.h"
#include "aic_at_process_common.h"

#define PING_PROCESS_EVENT  0x1

typedef struct aic_at_ping_cfg{
    u8 host[255];
    int cnt;
    int cid;
    int ping_interval;
    int ping_timeout;
    int ping_size;
    int ping_comid;
} AT_PING_CFG;

struct aic_ping_result{
    u32 send_cnt;
    u32 recv_cnt;
    u32 shortest_time;
    u32 longest_time;
    u32 average_time;
};

u32 aic_at_ping_set_cmd_handle(AT_REQ_RESP *at_req_resp_ptr);

#endif

