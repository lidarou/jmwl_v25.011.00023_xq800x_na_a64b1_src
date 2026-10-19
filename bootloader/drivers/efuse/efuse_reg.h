/*****************************************************************************/
/*                                                                           */
/*Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __EFUSE_REG_H__
#define __EFUSE_REG_H__

#include <aic_type.h>
#define REG_BASE_EFUSE                                                          REG_BASE_TOP_EFUSE
#define REG_EFUSE_EFUSE_IP_INFO                                                 (REG_BASE_EFUSE + 0x0)
#define REG_EFUSE_EFUSE_GLB_CTRL                                                (REG_BASE_EFUSE + 0x4)
#define REG_EFUSE_EFUSE_POWER_CTRL                                              (REG_BASE_EFUSE + 0x8)
#define REG_EFUSE_EFUSE_ALL01_CHECK                                             (REG_BASE_EFUSE + 0xc)
#define REG_EFUSE_EFUSE_RW_CTRL                                                 (REG_BASE_EFUSE + 0x10)
#define REG_EFUSE_SIP_BLK_INFO0                                                 (REG_BASE_EFUSE + 0x14)
#define REG_EFUSE_SIP_BLK_INFO1                                                 (REG_BASE_EFUSE + 0x18)
#define REG_EFUSE_SECURITY_BLK_INFO0                                            (REG_BASE_EFUSE + 0x1c)
#define REG_EFUSE_SECURITY_BLK_INFO1                                            (REG_BASE_EFUSE + 0x20)
#define REG_EFUSE_EFUSE_WRITE_DATA                                              (REG_BASE_EFUSE + 0x24)
#define REG_EFUSE_EFUSE_READ_DATA                                               (REG_BASE_EFUSE + 0x28)
#define REG_EFUSE_EFUSE_MAGIC_DATA                                              (REG_BASE_EFUSE + 0x2c)
#define REG_EFUSE_EFUSE_RW_TRIGGER                                              (REG_BASE_EFUSE + 0x30)
#define REG_EFUSE_CPU_MAILBOX                                                   (REG_BASE_EFUSE + 0x34)
#define REG_EFUSE_EFUSE_INT_ENABLE                                              (REG_BASE_EFUSE + 0x38)
#define REG_EFUSE_EFUSE_INT_STATUS_CLR                                          (REG_BASE_EFUSE + 0x3c)
#define REG_EFUSE_EFUSE_INT_STATUS_RAW                                          (REG_BASE_EFUSE + 0x40)
#define REG_EFUSE_EFUSE_INT_STATUS                                              (REG_BASE_EFUSE + 0x44)
#define REG_EFUSE_EFUSE_TIMING_CTRL0                                            (REG_BASE_EFUSE + 0x48)
#define REG_EFUSE_EFUSE_TIMING_CTRL1                                            (REG_BASE_EFUSE + 0x4c)
#define REG_EFUSE_EFUSE_TIMING_CTRL2                                            (REG_BASE_EFUSE + 0x50)
#define REG_EFUSE_EFUSE_GLB_CTRL_SET                                            (REG_BASE_EFUSE + 0x1004)
#define REG_EFUSE_EFUSE_GLB_CTRL_CLR                                            (REG_BASE_EFUSE + 0x2004)

/*REG_EFUSE_EFUSE_IP_INFO*/
#define BIT_EFUSE_IP_MAIN_VERSION(x)                                            (((x) & 0xff) << 8)
#define BIT_EFUSE_IP_SUB_VERSION(x)                                             (((x) & 0xff))

/*REG_EFUSE_EFUSE_GLB_CTRL*/
#define BIT_EFUSE_EFUSE_CACHE_ENABLE                                            BIT(2)
#define BIT_EFUSE_EFUSE_DEFAULT_VALUE                                           BIT(1)
#define BIT_EFUSE_EFUSE_EN                                                      BIT(0)

/*REG_EFUSE_EFUSE_POWER_CTRL*/
#define BIT_EFUSE_PD_VALUE                                                      BIT(1)
#define BIT_EFUSE_PD_AUTO_EN                                                    BIT(0)

/*REG_EFUSE_EFUSE_ALL01_CHECK*/
#define BIT_EFUSE_EFUSE_ALL01_CHECK_TRIGGER                                     BIT(0)

/*REG_EFUSE_EFUSE_RW_CTRL*/
#define BIT_EFUSE_EFUSE_WRITE_EN                                                BIT(8)
#define BIT_EFUSE_EFUSE_ADDR(x)                                                 (((x) & 0xff))

/*REG_EFUSE_SIP_BLK_INFO0*/
#define BIT_EFUSE_SIP_BLK_INFO0(x)                                              (((x) & 0xffffffff))

/*REG_EFUSE_SIP_BLK_INFO1*/
#define BIT_EFUSE_SIP_BLK_INFO1(x)                                              (((x) & 0xffffffff))

/*REG_EFUSE_SECURITY_BLK_INFO0*/
#define BIT_EFUSE_SECURITY_BLK_INFO0(x)                                         (((x) & 0xffffffff))

/*REG_EFUSE_SECURITY_BLK_INFO1*/
#define BIT_EFUSE_SECURITY_BLK_INFO1(x)                                         (((x) & 0xffffffff))

/*REG_EFUSE_EFUSE_WRITE_DATA*/
#define BIT_EFUSE_EFUSE_WDATA(x)                                                (((x) & 0xffffffff))

/*REG_EFUSE_EFUSE_READ_DATA*/
#define BIT_EFUSE_EFUSE_RDATA(x)                                                (((x) & 0xffffffff))

/*REG_EFUSE_EFUSE_MAGIC_DATA*/
#define BIT_EFUSE_EFUSE_MAGIC_DATA_WR(x)                                        (((x) & 0xffffffff))

/*REG_EFUSE_EFUSE_RW_TRIGGER*/
#define BIT_EFUSE_EFUSE_BONDING_VLD                                             BIT(1)
#define BIT_EFUSE_EFUSE_RW_TRIGGER                                              BIT(0)

/*REG_EFUSE_CPU_MAILBOX*/
#define BIT_EFUSE_CPU2_EFUSE_USED                                               BIT(3)
#define BIT_EFUSE_CPU2_EFUSE_REQUEST                                            BIT(2)
#define BIT_EFUSE_CPU1_EFUSE_USED                                               BIT(1)
#define BIT_EFUSE_CPU1_EFUSE_REQUEST                                            BIT(0)

/*REG_EFUSE_EFUSE_INT_ENABLE*/
#define BIT_EFUSE_INT_EN_EXCEPTION                                              BIT(3)
#define BIT_EFUSE_INT_EN_ALL01_CHECK                                            BIT(2)
#define BIT_EFUSE_INT_EN_WRITE_FINISH                                           BIT(1)
#define BIT_EFUSE_INT_EN_READ_FINISH                                            BIT(0)

/*REG_EFUSE_EFUSE_INT_STATUS_CLR*/
#define BIT_EFUSE_INT_STATUS_EXCEPTION_CLR                                      BIT(3)
#define BIT_EFUSE_INT_STATUS_ALL01_CHECK_FINISH_CLR                             BIT(2)
#define BIT_EFUSE_INT_STATUS_WRITE_FINISH_CLR                                   BIT(1)
#define BIT_EFUSE_INT_STATUS_READ_FINISH_CLR                                    BIT(0)

/*REG_EFUSE_EFUSE_INT_STATUS_RAW*/
#define BIT_EFUSE_INT_STATUS_APB_WRITE_WITH_WRONG_MAGIC_DATA_RAW                BIT(6)
#define BIT_EFUSE_INT_STATUS_APB_WRITE_LOCKED_BLOCK_RAW                         BIT(5)
#define BIT_EFUSE_INT_STATUS_APB_ACCESS_SECURITY_ILLEGAL_RAW                    BIT(4)
#define BIT_EFUSE_INT_STATUS_APB_ACCESS_SIP_ILLEGAL_RAW                         BIT(3)
#define BIT_EFUSE_INT_STATUS_ALL01_CHECK_FINISH_RAW                             BIT(2)
#define BIT_EFUSE_INT_STATUS_WRITE_FINISH_RAW                                   BIT(1)
#define BIT_EFUSE_INT_STATUS_READ_FINISH_RAW                                    BIT(0)

/*REG_EFUSE_EFUSE_INT_STATUS*/
#define BIT_EFUSE_INT_STATUS_APB_WRITE_WITH_WRONG_MAGIC_DATA                    BIT(24)
#define BIT_EFUSE_INT_STATUS_APB_WRITE_LOCKED_BLOCK                             BIT(23)
#define BIT_EFUSE_INT_STATUS_APB_ACCESS_SECURITY_ILLEGAL                        BIT(22)
#define BIT_EFUSE_INT_STATUS_APB_ACCESS_SIP_ILLEGAL                             BIT(21)
#define BIT_EFUSE_ALL01_CHECK_ERROR_ADDR_ECC(x)                                 (((x) & 0xff) << 13)
#define BIT_EFUSE_ALL01_CHECK_ERROR_ADDR_RAW(x)                                 (((x) & 0xff) << 5)
#define BIT_EFUSE_INT_STATUS_ALL01_CHECK_PASS_ECC                               BIT(4)
#define BIT_EFUSE_INT_STATUS_ALL01_CHECK_PASS_RAW                               BIT(3)
#define BIT_EFUSE_INT_STATUS_ALL01_CHECK_FINISH                                 BIT(2)
#define BIT_EFUSE_INT_STATUS_WRITE_FINISH                                       BIT(1)
#define BIT_EFUSE_INT_STATUS_READ_FINISH                                        BIT(0)

/*REG_EFUSE_EFUSE_TIMING_CTRL0*/
#define BIT_EFUSE_TIME_HP_PS_CS(x)                                              (((x) & 0x1f) << 24)
#define BIT_EFUSE_TIME_HP_CS(x)                                                 (((x) & 0x3) << 22)
#define BIT_EFUSE_TIME_HP_A(x)                                                  (((x) & 0x7) << 19)
#define BIT_EFUSE_TIME_SUP_A(x)                                                 (((x) & 0x7) << 16)
#define BIT_EFUSE_TIME_SUP_CS(x)                                                (((x) & 0x3) << 14)
#define BIT_EFUSE_TIME_SUP_PS_CS(x)                                             (((x) & 0x1f) << 9)
#define BIT_EFUSE_TIME_SUP_PD_PS(x)                                             (((x) & 0x1ff))

/*REG_EFUSE_EFUSE_TIMING_CTRL1*/
#define BIT_EFUSE_TIME_HR_CS(x)                                                 (((x) & 0x3) << 27)
#define BIT_EFUSE_TIME_HR_A(x)                                                  (((x) & 0x7) << 24)
#define BIT_EFUSE_TIME_STROBE_RD(x)                                             (((x) & 0x7f) << 17)
#define BIT_EFUSE_TIME_SUR_A(x)                                                 (((x) & 0x7) << 14)
#define BIT_EFUSE_TIME_SUR_CS(x)                                                (((x) & 0x1f) << 9)
#define BIT_EFUSE_TIME_SUR_PD_CS(x)                                             (((x) & 0x1ff))

/*REG_EFUSE_EFUSE_TIMING_CTRL2*/
#define BIT_EFUSE_TIME_STROBE_PGM(x)                                            (((x) & 0xfff) << 13)
#define BIT_EFUSE_TIME_HR_PD_CS(x)                                              (((x) & 0x7) << 10)
#define BIT_EFUSE_TIME_SQ(x)                                                    (((x) & 0x7f) << 3)
#define BIT_EFUSE_TIME_HP_PD_PS(x)                                              (((x) & 0x7))

/*REG_EFUSE_EFUSE_GLB_CTRL_SET*/
#define BIT_EFUSE_EFUSE_CACHE_ENABLE                                            BIT(2)
#define BIT_EFUSE_EFUSE_DEFAULT_VALUE                                           BIT(1)
#define BIT_EFUSE_EFUSE_EN                                                      BIT(0)

/*REG_EFUSE_EFUSE_GLB_CTRL_CLR*/
#define BIT_EFUSE_EFUSE_CACHE_ENABLE                                            BIT(2)
#define BIT_EFUSE_EFUSE_DEFAULT_VALUE                                           BIT(1)
#define BIT_EFUSE_EFUSE_EN                                                      BIT(0)

#endif /*__EFUSE_REG_H__*/
