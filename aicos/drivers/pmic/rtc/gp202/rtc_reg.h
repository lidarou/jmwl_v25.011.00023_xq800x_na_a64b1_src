
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-09-07 15:23:13
 */

#ifndef __RTC_REG_H__
#define __RTC_REG_H__

#define REG_RTC_IP_INFO                                                         (0x0)
#define REG_RTC_GLB_CTRL                                                        (0x4)
#define REG_RTC_EN_CTRL                                                         (0x8)
#define REG_RTC_CFG_TIME                                                        (0xc)
#define REG_RTC_CFG_DAY                                                         (0x10)
#define REG_RTC_UPDATE                                                          (0x14)
#define REG_RTC_TIME_ALARM1                                                     (0x18)
#define REG_RTC_DAY_ALARM1                                                      (0x1c)
#define REG_RTC_ALARM1_CFG_DONE                                                 (0x20)
#define REG_RTC_ALARM1_EN                                                       (0x24)
#define REG_RTC_TIME_ALARM2                                                     (0x28)
#define REG_RTC_DAY_ALARM2                                                      (0x2c)
#define REG_RTC_ALARM2_CFG_DONE                                                 (0x30)
#define REG_RTC_ALARM2_EN                                                       (0x34)
#define REG_RTC_INT_EN                                                          (0x38)
#define REG_RTC_INT_CLR                                                         (0x3c)
#define REG_RTC_RD_TIME                                                         (0x40)
#define REG_RTC_RD_DAY                                                          (0x44)
#define REG_RTC_INT_STS                                                         (0x48)
#define REG_RTC_INT_STS_RAW                                                     (0x4c)

/*REG_RTC_IP_INFO*/
#define BIT_RTC_IP_MAIN_VERSION(x)                                              (((x) & 0xf) << 4)
#define BIT_RTC_IP_SUB_VERSION(x)                                               (((x) & 0xf))

/*REG_RTC_GLB_CTRL*/
#define BIT_RTC_SECOND_CNT_THR(x)                                               (((x) & 0x7fff))

/*REG_RTC_EN_CTRL*/
#define BIT_RTC_CNT_EN                                                          BIT(0)

/*REG_RTC_CFG_TIME*/
#define BIT_RTC_CFG_HOUR(x)                                                     (((x) & 0x1f) << 16)
#define BIT_RTC_CFG_HOUR_OFFSET                                                 16
#define BIT_RTC_CFG_MINUTE(x)                                                   (((x) & 0x3f) << 8)
#define BIT_RTC_CFG_MINUTE_OFFSET                                               8
#define BIT_RTC_CFG_SECOND(x)                                                   (((x) & 0x3f))

/*REG_RTC_CFG_DAY*/
#define BIT_RTC_CFG_DAY(x)                                                      (((x) & 0xffff))

/*REG_RTC_UPDATE*/
#define BIT_RTC_TIME_UPDATE                                                     BIT(0)

/*REG_RTC_TIME_ALARM1*/
#define BIT_RTC_ALARM1_HOUR(x)                                                  ((x) & 0x1f0000)
#define BIT_RTC_ALARM1_HOUR_OFFSET                                              16
#define BIT_RTC_ALARM1_MINUTE(x)                                                ((x) & 0x3f00)
#define BIT_RTC_ALARM1_MINUTE_OFFSET                                            8
#define BIT_RTC_ALARM1_SECOND(x)                                                (((x) & 0x3f))

/*REG_RTC_DAY_ALARM1*/
#define BIT_RTC_ALARM1_DAY(x)                                                   (((x) & 0xffff))

/*REG_RTC_ALARM1_CFG_DONE*/
#define BIT_RTC_ALARM1_CFG_DONE                                                 BIT(0)

/*REG_RTC_ALARM1_EN*/
#define BIT_RTC_ALARM1_EN                                                       BIT(0)

/*REG_RTC_TIME_ALARM2*/
#define BIT_RTC_ALARM2_HOUR(x)                                                  ((x) & 0x1f0000)
#define BIT_RTC_ALARM2_HOUR_OFFSET                                              16
#define BIT_RTC_ALARM2_MINUTE(x)                                                ((x) & 0x3f00)
#define BIT_RTC_ALARM2_MINUTE_OFFSET                                            8
#define BIT_RTC_ALARM2_SECOND(x)                                                (((x) & 0x3f))

/*REG_RTC_DAY_ALARM2*/
#define BIT_RTC_ALARM2_DAY(x)                                                   (((x) & 0xffff))

/*REG_RTC_ALARM2_CFG_DONE*/
#define BIT_RTC_ALARM2_CFG_DONE                                                 BIT(0)

/*REG_RTC_ALARM2_EN*/
#define BIT_RTC_ALARM2_EN                                                       BIT(0)

/*REG_RTC_INT_EN*/
#define BIT_RTC_INTR_EN(x)                                                      (((x) & 0x1ff))

/*REG_RTC_INT_CLR*/
#define BIT_RTC_INTR_CLR(x)                                                     (((x) & 0x1ff))

/*REG_RTC_RSVD_FOR_SW*/
#define BIT_RTC_RSVD_REG_FOR_SW(x)                                              (((x) & 0xffffffff))

/*REG_RTC_RD_TIME*/
#define BIT_RTC_HOUR(x)                                                         ((x) & 0x1f0000)
#define BIT_RTC_HOUR_OFFSET                                                     16
#define BIT_RTC_MINUTE(x)                                                       ((x) & 0x3f00)
#define BIT_RTC_MINUTE_OFFSET                                                   8
#define BIT_RTC_SECOND(x)                                                       (((x) & 0x3f))

/*REG_RTC_RD_DAY*/
#define BIT_RTC_DAY(x)                                                          (((x) & 0xffff))

/*REG_RTC_INT_STS*/
#define BIT_RTC_INTR_STS(x)                                                     (((x) & 0x1ff))

/*REG_RTC_INT_STS_RAW*/
#define BIT_RTC_INTR_STS_RAW(x)                                                 (((x) & 0x1ff))

#endif /*__RTC_REG_H__*/
