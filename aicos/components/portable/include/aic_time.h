/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_TIME_H__
#define __AIC_TIME_H__

#include <time.h>

void aic_msec_to_abstime(long msec, struct timespec *abstime);
void aic_msec_to_time(long msec, struct timespec *timeout);

#endif /* __AIC_TIME_H__ */
