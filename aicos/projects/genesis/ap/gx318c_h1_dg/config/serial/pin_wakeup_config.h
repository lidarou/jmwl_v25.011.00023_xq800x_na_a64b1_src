/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PIN_WAKEUP_CFG_H__
#define __PIN_WAKEUP_CFG_H__

#define PIN_WAKEUP_DETECT                   GPIO_AON_11  /* Detect wakeup request */
// #define PIN_WAKEUP_INDICATE                 GPIO_AON_5  /* Indicate enter wakeup status */

#define PIN_WAKEUP_LEVEL                    0           /* pin wakeup level */

#define MCU_DEFAULT_SLEEP                   1           /* mcu default sleep or wakeup. 0:wakeup, 1:sleep */

#endif /* __PIN_WAKEUP_CFG_H__ */
