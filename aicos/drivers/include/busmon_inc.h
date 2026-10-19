/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of bus monitor module.
 *   It includes all the bus monitor common function api.
 */

#ifndef __BUSMON_INC_H__
#define __BUSMON_INC_H__

typedef enum  {
    BM_MCU,
    BM_CPUSYS,
    BM_CPSYS,
    BM_MEM_CP,
    BM_MEM_AP,
    BM_MEM_MODEM,
    BM_IP_MAX
} BUSMON_IP;

typedef enum  {
    BM_MCU_S1,
    BM_MCU_S2,
    BM_CPUSYS_S1,
    BM_CPUSYS_S2,
    BM_CPSYS_S1,
    BM_CPSYS_S2,
    BM_CPSYS_S3,
    BM_SEL_NULL = 0xF
} BUSMON_SEL;

typedef enum {
    BUSMON_WRITE = 0,
    BUSMON_READ,
    BUSMON_RW
} BUSMON_RW_TYPE;

typedef enum {
    BUSMON_INCLUDE = 0,
    BUSMON_EXCLUDE
} BUSMON_ADDR_MODE;

struct aic_busmon_board_cfg {
    BUSMON_IP index;
    u32 base;
    u32 glb_base;
    u32 glb_bit;
    u32 mux_sel_base;
    u32 mux_sel_eb;
    u32 irq_no;
};

struct aic_trans_mon_cfg {
    u32 glb_base;
    u32 glb_bit;
};

#if defined CONFIG_DRIVER_BUSMON
/*
 * gx_monitor_cfg_userid
 * This api used to configure monitor user id
 * @ip: the bus monitor ip used to monitor
 * @userid: the master user id
 */
void gx_monitor_cfg_userid(BUSMON_IP ip, u32 userid);

/*
 * gx_monitor_single_addr
 * This api used to monitor single address and set to only
 * one bus monitor ip.
 * @ip: the bus monitor ip used to monitor
 * @sel: the bus monitor could have select config to select slave port
 * @addr: the monitor address
 * @rw: the monitor operation, read/write or read and write
 */
void gx_monitor_single_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr, BUSMON_RW_TYPE rw);

/*
 * gx_monitor_range_addr
 * This api used to monitor a range address and set to only
 * one bus monitor ip. the range address is addr_s ~ addrs_e.
 * @ip: the bus monitor ip used to monitor
 * @sel: the bus monitor could have select config to select slave port
 * @addr_s: the monitor start address
 * @addr_e: the monitor end address
 * @rw: the monitor operation, read/write or read and write
 */
void gx_monitor_range_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, BUSMON_RW_TYPE rw);

/*
 * gx_monitor_exclude_addr
 * This api used to monitor two range address and set to only
 * one bus monitor ip. the range address is 0 ~ addr_s and addrs_e ~ 0xffffff.
 * @ip: the bus monitor ip used to monitor
 * @sel: the bus monitor could have select config to select slave port
 * @addr_s: the monitor exclude start address
 * @addr_e: the monitor exclude end address
 * @rw: the monitor operation, read/write or read and write
 */
void gx_monitor_exclude_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, BUSMON_RW_TYPE rw);

/*
 * gx_monitor_raw_addr
 * This api used to monitor range address
 * @ip: the bus monitor ip used to monitor
 * @sel: the bus monitor could have select config to select slave port
 * @addr_s: the monitor start address
 * @addr_e: the monitor end address
 * @addr_m: the monitor mask address
 * @mode: the monitor memory type, include or exclude mode
 * @rw: the monitor operation, read/write or read and write
 */
void gx_monitor_raw_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, u32 addr_m,
                        BUSMON_ADDR_MODE mode, BUSMON_RW_TYPE rw);

void gx_monitor_suspend(void);

void gx_monitor_resume(void *p);

#else
static inline void gx_monitor_cfg_userid(BUSMON_IP ip, u32 userid){}
static inline void gx_monitor_single_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr, BUSMON_RW_TYPE rw){}

static inline void gx_monitor_range_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, BUSMON_RW_TYPE rw){}
static inline void gx_monitor_exclude_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, BUSMON_RW_TYPE rw){}
static inline void gx_monitor_raw_addr(BUSMON_IP ip, BUSMON_SEL sel, u32 addr_s, u32 addr_e, u32 addr_m,
                        BUSMON_ADDR_MODE mode, BUSMON_RW_TYPE rw){}
static inline void gx_monitor_suspend(void){}
static inline void gx_monitor_resume(void *p){}

#endif

#endif /* __BUSMON_INC_H__ */

