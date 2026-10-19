/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * Export the basic nvram sync transmit Interface (API).
 */

#ifndef __AIC_NVRAM_SYNC_TRANSMIT_H__
#define __AIC_NVRAM_SYNC_TRANSMIT_H__

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <aic_osa.h>

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
typedef enum _NVRAM_SYNC_CMD{
    NVRAM_CMD_SYNC = 1,
    NVRAM_CMD_GET_DATA,
    NVRAM_MAX_CMD
}NVRAM_SYNC_CMD;

/**---------------------------------------------------------------------------*
 ** Function Prototypes
 **---------------------------------------------------------------------------*/
/**
 * nvram_sync_transmit_init - data sync transmit initialize.
 *
 * Parameter:
 * @sys: the remote system that communicates with the local system.
 *       AIPC_SYS_CP
 *       AIPC_SYS_AP
 * @addr:data sync transmit address.
 * @queue_name: queue name.
 * @queue_num: queue capacity.
 * @queue_ptr: queue ptr
 *
 * Context:
 *
 * Return:
 * @addr:data sync transmit address.
 * @queue_ptrqueue ptr
 */
void nvram_sync_transmit_initialize(u8 sys, addr_t *addr, char *queue_name,
                                    u32 queue_num, aos_queue_ptr *queue_ptr);


/**
 * nvram_sync_transmit_uninitialize - data sync transmit uninitialize.
 *
 * Parameter:
 * @sys: the remote system that communicates with the local system.
 *       AIPC_SYS_CP
 *       AIPC_SYS_AP
 * @addr:data sync transmit address.
 *
 * Context:
 *
 * Return:
 * @addr:data sync transmit address.
 */
void nvram_sync_transmit_uninitialize(u8 sys,addr_t addr);

/**
 * nvram_sync_transmit_recv - recv data from other system.
 *
 * Parameter:
 * @cmd: to store recv cmd
 * @data: to store recv data
 * @queue_ptr: queue ptr
 *
 * Context:
 *
 * Return:
 *
 */
void nvram_sync_transmit_recv(u16 *cmd, u32 *data, aos_queue_ptr queue_ptr);

/**
 * nvram_sync_transmit_send - send data to other system.
 *
 * Parameter:
 * @sys: the remote system that communicates with the local system.
 *       AIPC_SYS_CP
 *       AIPC_SYS_AP
 * @cmd: sync cmd
 * @data: sync data.
 *
 * Context:
 *
 * Return:
 *
 */
void nvram_sync_transmit_send(u8 sys, u16 cmd, u32 data);

/**---------------------------------------------------------------------------*/
#endif /* __AIC_NVRAM_SYNC_TRANSMIT_H__ */

