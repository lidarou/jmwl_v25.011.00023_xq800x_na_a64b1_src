/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the list api of aic os componets.
 */

#ifndef __AIC_LIST_OS_COMPONENTS_H__
#define __AIC_LIST_OS_COMPONENTS_H__

typedef enum {
    AIC_COMP_THREAD = 0,
    AIC_COMP_EVENT,
    AIC_COMP_MUTEX,
    AIC_COMP_SEMAPHORE,
    AIC_COMP_QUEUE,
    AIC_COMP_TIMER
} AIC_COMPONENTS;

int aic_comp_list(AIC_COMPONENTS components);

#endif /* __AIC_LIST_OS_COMPONENTS_H__ */

