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
#ifndef __ALOG_DEFAULT_CFG_H__
#define __ALOG_DEFAULT_CFG_H__

/**
 * s_buf_cfg - Contain the buffer manage data of alog trace and alog message.
 */
static const ALOG_BUF_CFG_T s_buf_cfg[] =
{
    {8,  4, SIZE_4K, 1},
    {10,  2, SIZE_4K, 1}
};

/**
 * s_alog_cfg - Contain the dst system and the brearer id of the log system.
 */
static const ALOG_CFG_T s_alog_cfg = {
    AIC_SYS_AP, AIPC_BRR_DEBUG
};

/**
 * s_inputter_cfg - receive debug brr(simulator ap recv, dst sys is cp).
 */
static const ALBRR_CFG_T s_inputter_cfg[] = {
    {AIC_SYS_CP, AIPC_BRR_DEBUG},
};

#define LOG_DEV     ALOP_MAKE_OUTDEV(AIC_IODEV_FILE, AIC_COM0)
#define DIAG_DEV    ALOP_MAKE_OUTDEV(AIC_IODEV_UART, AIC_COM2)
#define LOG_SWITCH  (ALOG_TRACE_SW | ALOG_MSG_SW | ALOG_DATA_SW)

/**
 * s_debug_cfg - Contain the non-volatile configuration of the debug system.
 */
AIC_DEBUG s_debug_cfg = {
    ALOG_NV_MAGIC, /* magic */
    0, /* sysdump */
    0, /* autoreset */
    0, /* debug_gate */
    0, /* sleep_mode */
    0, /* status_light */
    /* aic at */
    {
        115200
    },
	/* aic_diag */
    {
        DIAG_DEV,
        0,0,
        2000000
    },
	/* aic_log */
    {
        LOG_DEV,
        LOG_SWITCH,
        0,
        0,
        2000000,
        {0},
        {0}
    },
    /* reserve */
    {0}
};

#endif /* __ALOG_DEFAULT_CFG_H__ */

