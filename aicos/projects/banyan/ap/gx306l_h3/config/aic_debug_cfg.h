/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The ap log mudule config file.
 */
#ifndef __ALOG_CUSTOMER_CFG_H__
#define __ALOG_CUSTOMER_CFG_H__

/**
 * s_buf_cfg - Contain the buffer manage data of alog trace and alog message.
 * notify:
 * 1. the offset must < buf_cnt. it means that how many boots stage log blocks
 *    will be kept when the log is cyclically overwritten.
 *
 * 2. the size must be in the size the be configed at cpap_comm_pool_size
 *    in aipc_customer_cfg.h.
 *
 * 3. the buf_cnt must < the blk cnt of the responce size that be configed
 *    at apap_debug_pool_cnt in aipc_customer_cfg.h.
 */
static const ALOG_BUF_CFG_T s_buf_cfg[] =
{
    [ALOG_TYPE_TRACE] = {.buf_cnt = 9, .offset = 5, .size = SIZE_2K, .priority = 250},
};

/**
 * s_alog_cfg - Contain the dst system and the bearer id of the log system.
 */
static const ALOG_CFG_T s_alog_cfg = {
    .sys = AIC_SYS_AP, .brr = AIPC_BRR_DEBUG
};

/**
 * s_inputter_cfg - Contain all the sys debug bearer
 *  associated with the log forworder of this system.
 */
static const ALBRR_CFG_T s_inputter_cfg[] = {
    {AIC_SYS_AP, AIPC_BRR_DEBUG},
    {AIC_SYS_CP, AIPC_BRR_DEBUG},
};

/*
 * Now not use this com configed here,
 * the real used com is configed in connmgr_cfg.h
 */
#ifdef CONFIG_CDC_ACM_INSTANCE
#define LOG_DEV     ALOP_MAKE_OUTDEV(AIC_IODEV_USB, AIC_COM0)
#define DIAG_DEV    ALOP_MAKE_OUTDEV(AIC_IODEV_USB, AIC_COM1)
#else
#define LOG_DEV     ALOP_MAKE_OUTDEV(AIC_IODEV_UART, AIC_COM0)
#define DIAG_DEV    ALOP_MAKE_OUTDEV(AIC_IODEV_UART, AIC_COM4)
#endif
#define LOG_SWITCH  (ALOG_TRACE_SW | ALOG_DATA_SW)

/**
 * s_debug_cfg - Contain the non-volatile configuration of the debug system.
 */
AIC_DEBUG s_debug_cfg = {
    .magic = ALOG_NV_MAGIC,
    .sysdump = 0x7,
    .autoreset = 0x7,
    .debug_gate = 0x00000003,
/**
 * example:
 * deepsleep1-a: .sleep_mode = BIT(1),
 * deepsleep1-b: .sleep_mode = BIT(1) | BIT(3),
 * deepsleep2  : .sleep_mode = BIT(1) | BIT(3) | BIT(2),
 * 
 * deepsleep1-b休眠功耗1.50+mA左右，低电功耗480+uA左右，底电流460+uA。
 * 
 * 选择那种模式要看PIR_OUT的引脚功能决定，使用EIPU功能就用deepsleep1-b
 * 使用GPIO功能则必须用deepsleep2，否则另外两种模式下VDDCORE_2被关闭，
 * 采用GPIO功能的PIR_OUT无法触发唤醒
 */
    .sleep_mode = BIT(1) | BIT(3),
    .aic_diag = {
        .device = DIAG_DEV,
        .baudrate = 2000000
        },
    .aic_log = {
        .device = LOG_DEV,
        .gate = LOG_SWITCH,
        .baudrate = 115200,
        .level_bitmap = {0}
    }
};

#endif /* __ALOG_CUSTOMER_CFG_H__ */

