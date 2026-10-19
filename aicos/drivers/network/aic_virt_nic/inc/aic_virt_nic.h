/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NETXDUO Component                                                     */
/**                                                                       */
/**   AIC virtual nic for NETX 5.3 and above                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    aic_virt_nic.h                                       PORTABLE C     */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Qingsheng Li,    AICXTEK TECHNOLOGIES CO.,LTD                       */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Sending and Receiving Network DAta based on SPI BUS.                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-14-2023     Qingsheng Li           Initial Version 1.0             */
/*                                                                        */
/**************************************************************************/

#ifndef __AIC_VIRT_NIC_H__
#define __AIC_VIRT_NIC_H__

/* Determine if a C++ compiler is being used. If so, ensure that standard
   C is used to process the API information. */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C. */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_ip.h"
#include "aic_osa.h"

#define VIRT_NIC_TASK_PRIORITY        15

struct aic_virt_network_driver
{
    u32 if_index;
    void *cmux_handle;
    aos_thread_id recv_thread_id;
};

int aic_virt_nic_send(NX_PACKET *packet_ptr);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above. */
#ifdef __cplusplus
}
#endif

#endif
