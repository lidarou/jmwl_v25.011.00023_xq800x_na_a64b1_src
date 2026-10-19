/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines lamp operate api.
 *    The torch api will be called by aicos user functions.
 */

#ifndef __LAMP_H__
#define __LAMP_H__
#include <pmu_user.h>

/*lamp user interface*/
int lamp_enabled(bool enabled);
int lamp_is_enabled(void);
int lamp_set_current(u32 current_uA);
int lamp_get_current(u32 *current_uA);

/*lamp unit driver register*/
int lamp_unit_register(struct pmu_user *lamp_unit_pmu);
#endif /*__LAMP_H__*/
