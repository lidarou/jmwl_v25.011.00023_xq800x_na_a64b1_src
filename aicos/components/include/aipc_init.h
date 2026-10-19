/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This aipc init header file.
 */
#ifndef __AIPC_INIT_H__
#define __AIPC_INIT_H__

/**
 * aipc_init - will create all the instance of aipc according aipc congfig data.
 */
void aipc_init(void);

/**
 * aipc_deinit - will destoy all the instance of aipc.
 *
 */
void aipc_deinit(void);

/**
 * aipc_reset_sys - Reset a aipc. Just used for reboot a clinet sys.
 *
 */
void aipc_reset_sys(u8 sys);

#endif /* __AIPC_INIT_H__ */

