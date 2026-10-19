/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-06-20 10:06:22
 */

#ifndef __THERM_RF_H__
#define __THERM_RF_H__

#define REG_BASE_THERM                                                          (REG_BASE_TOP_THMC)
#define REG_THERM_THERM_WORK_MODE                                               (REG_BASE_THERM + 0x0)
#define REG_THERM_THERM_CALIB_TEMP                                              (REG_BASE_THERM + 0x4)
#define REG_THERM_THERM_CALIB_RPT                                               (REG_BASE_THERM + 0x8)
#define REG_THERM_THERM_DETECT_FACTOR                                           (REG_BASE_THERM + 0xc)
#define REG_THERM_THERM_DETECT_CFG                                              (REG_BASE_THERM + 0x10)
#define REG_THERM_THERM_DETECT_RPT                                              (REG_BASE_THERM + 0x14)
#define REG_THERM_THERM_STAT_CFG                                                (REG_BASE_THERM + 0x18)
#define REG_THERM_THERM_STAT_RPT                                                (REG_BASE_THERM + 0x1c)
#define REG_THERM_THERM_AUTO_DETECT_CFG                                         (REG_BASE_THERM + 0x20)
#define REG_THERM_THERM_HIGH_TEMP_ALARM0                                        (REG_BASE_THERM + 0x24)
#define REG_THERM_THERM_HIGH_TEMP_ALARM1                                        (REG_BASE_THERM + 0x28)
#define REG_THERM_THERM_LOW_TEMP_ALARM                                          (REG_BASE_THERM + 0x2c)
#define REG_THERM_THERM_AUTO_DETECT_RPT                                         (REG_BASE_THERM + 0x30)
#define REG_THERM_THERM_INT_RAW                                                 (REG_BASE_THERM + 0x34)
#define REG_THERM_THERM_INT_STA                                                 (REG_BASE_THERM + 0x38)
#define REG_THERM_THERM_INT_EN                                                  (REG_BASE_THERM + 0x3c)
#define REG_THERM_THERM_INT_CLR                                                 (REG_BASE_THERM + 0x40)
#define REG_THERM_THERM_LOOOP_EN_CFG                                            (REG_BASE_THERM + 0x44)
#define REG_THERM_THERM_LOOP_DATA_CFG                                           (REG_BASE_THERM + 0x48)

/*REG_THERM_THERM_WORK_MODE*/
#define BIT_THERM_THERM_CALIB_EN                                                BIT(2)
#define BIT_THERM_THERM_DETECT_EN                                               BIT(1)
#define BIT_THERM_THERM_AUTO_DETECT_EN                                          BIT(0)

/*REG_THERM_THERM_CALIB_TEMP*/
#define BIT_THERM_THERM_CALIB_TEMP(x)                                           (((x) & 0xfff) << 16)

/*REG_THERM_THERM_CALIB_RPT*/
#define BIT_THERM_THERM_CALIB_FACTOR(x)                                         (((x) & 0xfff))

/*REG_THERM_THERM_DETECT_FACTOR*/
#define BIT_THERM_THERM_DETECT_FACTOR(x)                                        (((x) & 0xfff) << 16)

/*REG_THERM_THERM_DETECT_CFG*/
#define BIT_THERM_THERM_DETECT_OFFSET(x)                                        (((x) & 0xfff))

/*REG_THERM_THERM_DETECT_RPT*/
#define BIT_THERM_THERM_DETECT_TEMP(x)                                          (((x) & 0xfff))

/*REG_THERM_THERM_STAT_CFG*/
#define BIT_THERM_THERM_STAT_CNT(x)                                             (((x) & 0xffff))

/*REG_THERM_THERM_STAT_RPT*/
#define BIT_THERM_THERM_STAT_DATA(x)                                            (((x) & 0xffff))

/*REG_THERM_THERM_AUTO_DETECT_CFG*/
#define BIT_THERM_THERM_AUTO_DETECT_GAP(x)                                      (((x) & 0xffffffff))

/*REG_THERM_THERM_HIGH_TEMP_ALARM0*/
#define BIT_THERM_THERM_ALARM0_THRES(x)                                         (((x) & 0xfff))

/*REG_THERM_THERM_HIGH_TEMP_ALARM1*/
#define BIT_THERM_THERM_ALARM1_THRES(x)                                         (((x) & 0xfff))

/*REG_THERM_THERM_LOW_TEMP_ALARM*/
#define BIT_THERM_THERM_ALARM2_THRES(x)                                         (((x) & 0xfff))

/*REG_THERM_THERM_AUTO_DETECT_RPT*/
#define BIT_THERM_THERM_AUTO_DETECT_TEMP(x)                                     (((x) & 0xfff))

/*REG_THERM_THERM_INT_RAW*/
#define BIT_THERM_THERM_INT_RAW(x)                                              (((x) & 0x1f))

/*REG_THERM_THERM_INT_STA*/
#define BIT_THERM_THERM_INT_STA(x)                                              (((x) & 0x1f))

/*REG_THERM_THERM_INT_EN*/
#define BIT_THERM_THERM_INT_EN(x)                                               (((x) & 0x1f))

/*REG_THERM_THERM_INT_CLR*/
#define BIT_THERM_THERM_INT_CLR(x)                                              (((x) & 0x1f))

/*REG_THERM_THERM_LOOOP_EN_CFG*/
#define BIT_THERM_THERM_LOOPBACK_EN                                             BIT(0)

/*REG_THERM_THERM_LOOP_DATA_CFG*/
#define BIT_THERM_THERM_LOOPBACK_DATA(x)                                        (((x) & 0xffff))

#endif /*__THERM_RF_H__*/
