
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-04-26 16:12:34
 */

#ifndef __PWM_RF_H__
#define __PWM_RF_H__

#define REG_PWM_PWM_IP_INFO                                                     (0x0)
#define REG_PWM_PWM_GLB_CTRL                                                    (0x4)
#define REG_PWM_PWM_UPDATE                                                      (0x8)
#define REG_PWM_PWM_PREDIV                                                      (0xc)
#define REG_PWM_PWM_MOD                                                         (0x10)
#define REG_PWM_PWM_DUTY                                                        (0x14)
#define REG_PWM_PWM_TONE                                                        (0x18)
#define REG_PWM_PWM_PATTERN                                                     (0x1c)

/*REG_PWM_PWM_IP_INFO*/
#define BIT_PWM_IP_MAIN_VERSION(x)                                              (((x) & 0xff) << 8)
#define BIT_PWM_IP_SUB_VERSION(x)                                               (((x) & 0xff))

/*REG_PWM_PWM_GLB_CTRL*/
#define BIT_PWM_PWM_EN                                                          BIT(0)

/*REG_PWM_PWM_UPDATE*/
#define BIT_PWM_PWM_UPDATE                                                      BIT(0)

/*REG_PWM_PWM_PREDIV*/
#define BIT_PWM_PWM_PREDIV(x)                                                   (((x) & 0xff))

/*REG_PWM_PWM_MOD*/
#define BIT_PWM_PWM_MOD_CNT(x)                                                  (((x) & 0xffff))

/*REG_PWM_PWM_DUTY*/
#define BIT_PWM_PWM_DUTY_CNT(x)                                                 (((x) & 0xffff))

/*REG_PWM_PWM_TONE*/
#define BIT_PWM_PWM_TONE_CNT(x)                                                 (((x) & 0xffff))

/*REG_PWM_PWM_PATTERN*/
#define BIT_PWM_PWM_PATTERN(x)                                                  (((x) & 0xffffffff))

#endif /*__PWM_RF_H__*/
