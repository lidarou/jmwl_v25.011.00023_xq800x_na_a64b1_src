/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_AT_SNTP_H__
#define __AIC_AT_SNTP_H__

#include <stdlib.h>
#include <string.h>

#include <nx_api.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#ifdef CONFIG_NET_SNTP
#include <nxd_sntp_client.h>
#endif
#include <nx_ip.h>
#include <aic_errno.h>

#include "aic_at_tcpip_common.h"
#include "aic_net_api.h"
#include "aic_at_parse_utils.h"
#include "aic_at_resp_handle.h"

#define SNTPTIME_SIZE 128
#define SNTP_TIMEZONE_MIN   (-12)
#define SNTP_TIMEZONE_MAX    12

#endif

