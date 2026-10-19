/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *  All aic sys define in here.
 */
#ifndef __AIC_SYS_H__
#define __AIC_SYS_H__

/**
 * AIC system defines. It contains all the system of aic system.
 */
enum {
    /* application processor system */
    AIC_SYS_AP = 0,
    /* comunication processor system */
    AIC_SYS_CP,
#ifdef CONFIG_AIC_CHIP_GENESIS
    /* audio dsp processor system */
    AIC_SYS_ADSP,
#endif
    /* the number of all the system */
    AIC_SYS_NR
};

#if defined (CONFIG_SUBSYS_AP)
#define AIC_THIS_SYS    AIC_SYS_AP
#elif defined (CONFIG_SUBSYS_CP)
#define AIC_THIS_SYS    AIC_SYS_CP
#else
#define AIC_THIS_SYS    AIC_SYS_AP
#endif

#define AIC_SYS_AUTO    0xFF

#endif /* __AIC_SYS_H__ */