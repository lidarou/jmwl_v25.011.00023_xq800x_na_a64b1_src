/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines all the GPIO WAKEUP MCU APIs exported for other module.
 */

#ifndef __INCLUDE_GPIO_WAKEUP_H__
#define __INCLUDE_GPIO_WAKEUP_H__

void gpio_release_target_mcu(void);
int gpio_wakeup_target_mcu(u32 timeout_ms);
void gpio_wait_target_mcu_wakeup_dir(void);


#endif /* __INCLUDE_GPIO_WAKEUP_H__ */
