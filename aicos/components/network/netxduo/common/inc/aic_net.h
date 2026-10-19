/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_NET_H__
#define __AIC_NET_H__

#include <aic_osa.h>

#ifdef CONFIG_NX_BSD
#include "nxd_bsd.h"
#endif

#ifdef CONFIG_AT_PROC
#include "aic_at_process_common.h"
#endif

#ifdef CONFIG_NET_TLS_API
#include "aic_tls_api.h"
#endif

#ifdef CONFIG_NET_HTTP_COMM
#include "aic_http_comm.h"
#endif

#ifdef CONFIG_NET_HTTPS
#include "aic_https_client.h"
#endif

#ifdef CONFIG_NET_HTTP
#include "aic_http_client.h"
#endif

#endif
