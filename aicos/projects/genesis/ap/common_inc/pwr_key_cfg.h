/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PWR_KEY_CFG_H__
#define __PWR_KEY_CFG_H__

#include <aic_type.h>

/*unit is ms, long press shutdown need short than lpr config ,
  shutdown threshold should less than 65535ms*/
#define PWR_LONG_PRESS_SHUTDOWN_THRESHOLD 3000

/* unit is second */
#define PWR_LPR_ENABLED    1
#define PWR_LPR_TIMING     8
#define PWR_LPR_TIMING_MIN 2
#define PWR_LPR_TIMING_MAX 14

#endif /* __PWR_KEY_CFG_H__ */
