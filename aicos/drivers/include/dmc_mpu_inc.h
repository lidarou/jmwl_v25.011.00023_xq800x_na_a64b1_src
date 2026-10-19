/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of DMC MPU module.
 *   It includes all the dmc mpu common function api.
 */

#ifndef __DMC_MPU_INC_H__
#define __DMC_MPU_INC_H__

typedef enum {
    MPU_WRITE = 0,
    MPU_READ,
    MPU_RW
} MPU_RW_TYPE;

typedef enum {
    MPU_EXCLUDE = 0,
    MPU_INCLUDE
} MPU_ADDR_MODE;

typedef enum {
    MPU_CP = 0,
    MPU_AP,
    MPU_MODEM
} MPU_PORT;

/*
 * gx_mpu_set_monitor_addr
 * This api used to monitor range address
 * @addr_s: the monitor start address
 * @addr_e: the monitor end address
 * @mode: the monitor memory type, include or exclude mode
 * @rw: the monitor operation, read/write or read and write
 */
void gx_mpu_set_monitor_addr(u32 addr_s, u32 addr_e, MPU_ADDR_MODE type, MPU_RW_TYPE op);

/*
 * gx_mpu_set_monitor_port
 * This api used to set monitor port
 * @port: the monitor port
 */
void gx_mpu_set_monitor_port(MPU_PORT port);
#ifdef CONFIG_DRIVER_DMC_MPU
void gx_mpu_mon_suspend(void);
void gx_mpu_mon_resume(void *p);
#else
static inline void gx_mpu_mon_suspend(void){}
static inline void gx_mpu_mon_resume(void *p){}
#endif
#endif /* __DMC_MPU_INC_H__ */

