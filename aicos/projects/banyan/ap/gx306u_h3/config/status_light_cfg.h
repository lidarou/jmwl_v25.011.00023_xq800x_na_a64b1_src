/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __STATUS_LIGHT_CFG_H__
#define __STATUS_LIGHT_CFG_H__

#include <aic_type.h>

#define NET_LIGHT_GPO_NUM                       0
#define DEVICE_STATUS_LIGHT_GPIO_NUM            20
#define DEVICE_SLEEP_STATUS_LIGHT_GPO_NUM       1

//#define NET_LIGHT_LED_NAME                      "LED_R"
//#define DEVICE_STATUS_LIGHT_LED_NAME            "LED_G"

#define NET_NOT_REGISTERED_LIGHT_DURATION       50//ms
#define NET_NOT_REGISTERED_LIGHT_OFF_DURATION   950//ms

#define NET_REGISTERED_LIGHT_DURATION           100//ms
#define NET_REGISTERED_LIGHT_OFF_DURATION       1900//ms
#endif /* __STATUS_LIGHT_CFG_H__ */
