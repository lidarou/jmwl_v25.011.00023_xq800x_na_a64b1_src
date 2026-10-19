/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/
#ifndef __LPR_H__
#define __LPR_H__
#include <aic_type.h>
/**
 * lpr_config - long press reset function config.
 * @enabled: enable config.
 * @effect_time_s: long press reset effect_time_s.
 * return 0 if config success, else fail.
 */
int lpr_config(BOOL enabled, u32 effect_time_s);

#endif /*__LPR_H__*/