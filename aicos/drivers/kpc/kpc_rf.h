
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-07-11 09:57:49
 */

#ifndef __KPC_RF_H__
#define __KPC_RF_H__

#define REG_KPC_KPC_IP_INFO                                                     (REG_BASE_KPC + 0x0)
#define REG_KPC_KPC_GLB_CTRL                                                    (REG_BASE_KPC + 0x4)
#define REG_KPC_KPC_MTX_EN                                                      (REG_BASE_KPC + 0x8)
#define REG_KPC_KPC_INT_EN                                                      (REG_BASE_KPC + 0xc)
#define REG_KPC_KPC_KEY_INT_STATUS_RAW                                          (REG_BASE_KPC + 0x10)
#define REG_KPC_KPC_KEY_INT_STATUS                                              (REG_BASE_KPC + 0x14)
#define REG_KPC_KPC_INT_CLR                                                     (REG_BASE_KPC + 0x18)
#define REG_KPC_KPC_POLARITY                                                    (REG_BASE_KPC + 0x1c)
#define REG_KPC_KPC_DEBOUNCE_CNT                                                (REG_BASE_KPC + 0x20)
#define REG_KPC_KPC_LONGKEY_CNT                                                 (REG_BASE_KPC + 0x24)
#define REG_KPC_KPC_AUTO_SLP_CNT                                                (REG_BASE_KPC + 0x28)
#define REG_KPC_KPC_CLK_DIV_CNT                                                 (REG_BASE_KPC + 0x2c)
#define REG_KPC_KPC_KEY_MATRIX                                                  (REG_BASE_KPC + 0x30)
#define REG_KPC_KPC_AUTO_SLP_STATUS                                             (REG_BASE_KPC + 0x34)
#define REG_KPC_KPC_DEBUG_STATUS0                                               (REG_BASE_KPC + 0x38)
#define REG_KPC_KPC_DEBUG_STATUS1                                               (REG_BASE_KPC + 0x3c)
#define REG_KPC_KPC_DEBUG_STATUS2                                               (REG_BASE_KPC + 0x40)
#define REG_KPC_KPC_GLB_CTRL_SET                                                (REG_BASE_KPC + 0x1004)
#define REG_KPC_KPC_MTX_EN_SET                                                  (REG_BASE_KPC + 0x1008)
#define REG_KPC_KPC_INT_EN_SET                                                  (REG_BASE_KPC + 0x100c)
#define REG_KPC_KPC_INT_CLR_SET                                                 (REG_BASE_KPC + 0x1018)
#define REG_KPC_KPC_POLARITY_SET                                                (REG_BASE_KPC + 0x101c)
#define REG_KPC_KPC_DEBOUNCE_CNT_SET                                            (REG_BASE_KPC + 0x1020)
#define REG_KPC_KPC_LONGKEY_CNT_SET                                             (REG_BASE_KPC + 0x1024)
#define REG_KPC_KPC_AUTO_SLP_CNT_SET                                            (REG_BASE_KPC + 0x1028)
#define REG_KPC_KPC_GLB_CTRL_CLR                                                (REG_BASE_KPC + 0x2004)
#define REG_KPC_KPC_MTX_EN_CLR                                                  (REG_BASE_KPC + 0x2008)
#define REG_KPC_KPC_INT_EN_CLR                                                  (REG_BASE_KPC + 0x200c)
#define REG_KPC_KPC_INT_CLR_CLR                                                 (REG_BASE_KPC + 0x2018)
#define REG_KPC_KPC_POLARITY_CLR                                                (REG_BASE_KPC + 0x201c)
#define REG_KPC_KPC_DEBOUNCE_CNT_CLR                                            (REG_BASE_KPC + 0x2020)
#define REG_KPC_KPC_LONGKEY_CNT_CLR                                             (REG_BASE_KPC + 0x2024)
#define REG_KPC_KPC_AUTO_SLP_CNT_CLR                                            (REG_BASE_KPC + 0x2028)

/*REG_KPC_KPC_IP_INFO*/
#define BIT_KPC_IP_MAIN_VERSION(x)                                              (((x) & 0xff) << 8)
#define BIT_KPC_IP_SUB_VERSION(x)                                               (((x) & 0xff))

/*REG_KPC_KPC_GLB_CTRL*/
#define BIT_KPC_KPC_LONGKEY_EN                                                  BIT(2)
#define BIT_KPC_KPC_AUTO_SLP_EN                                                 BIT(1)
#define BIT_KPC_KPC_EN                                                          BIT(0)

/*REG_KPC_KPC_MTX_EN*/
#define BIT_KPC_KPC_ROW_EN(x)                                                   (((x) & 0xff) << 8)
#define BIT_KPC_KPC_COL_EN(x)                                                   (((x) & 0xff))

/*REG_KPC_KPC_INT_EN*/
#define BIT_KPC_KPC_GHOSTKEY_INT_EN                                             BIT(3)
#define BIT_KPC_KPC_LONG_INT_EN                                                 BIT(2)
#define BIT_KPC_KPC_RELEASE_INT_EN                                              BIT(1)
#define BIT_KPC_KPC_PRESS_INT_EN                                                BIT(0)

/*REG_KPC_KPC_KEY_INT_STATUS_RAW*/
#define BIT_KPC_KPC_GHOSTKEY_INT_STATUS_RAW                                     BIT(16)
#define BIT_KPC_KEY3_INT_STATUS_RAW(x)                                          (((x) & 0x7) << 12)
#define BIT_KPC_KEY2_INT_STATUS_RAW(x)                                          (((x) & 0x7) << 8)
#define BIT_KPC_KEY1_INT_STATUS_RAW(x)                                          (((x) & 0x7) << 4)
#define BIT_KPC_KEY0_INT_STATUS_RAW(x)                                          (((x) & 0x7))

/*REG_KPC_KPC_KEY_INT_STATUS*/
#define BIT_KPC_KPC_GHOSTKEY_INT_STATUS                                         BIT(16)
#define BIT_KPC_KEY3_INT_STATUS(x)                                              (((x) & 0x7) << 12)
#define BIT_KPC_KEY2_INT_STATUS(x)                                              (((x) & 0x7) << 8)
#define BIT_KPC_KEY1_INT_STATUS(x)                                              (((x) & 0x7) << 4)
#define BIT_KPC_KEY0_INT_STATUS(x)                                              (((x) & 0x7))

/*REG_KPC_KPC_INT_CLR*/
#define BIT_KPC_KPC_GHOSTKEY_INT_STATUS_CLR                                     BIT(16)
#define BIT_KPC_KEY3_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 12)
#define BIT_KPC_KEY2_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 8)
#define BIT_KPC_KEY1_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 4)
#define BIT_KPC_KEY0_INT_STATUS_CLR(x)                                          (((x) & 0x7))

/*REG_KPC_KPC_POLARITY*/
#define BIT_KPC_KPC_ROW_COL_POL                                                 BIT(0)

/*REG_KPC_KPC_DEBOUNCE_CNT*/
#define BIT_KPC_KPC_DEBOUNCE_CNT_RELEASE(x)                                     (((x) & 0xffff) << 16)
#define BIT_KPC_KPC_DEBOUNCE_CNT_PRESS(x)                                       (((x) & 0xffff))

/*REG_KPC_KPC_LONGKEY_CNT*/
#define BIT_KPC_KPC_LONGKEY_CNT(x)                                              (((x) & 0xffff))

/*REG_KPC_KPC_AUTO_SLP_CNT*/
#define BIT_KPC_KPC_AUTO_SLP_CNT(x)                                             (((x) & 0xffffffff))

/*REG_KPC_KPC_CLK_DIV_CNT*/
#define BIT_KPC_KPC_CLK_DIV_CNT(x)                                              (((x) & 0xffff))

/*REG_KPC_KPC_KEY_MATRIX*/
#define BIT_KPC_KPC_KEY3_ROW_NUM(x)                                             (((x) & 0x7) << 28)
#define BIT_KPC_KPC_KEY3_COL_NUM(x)                                             (((x) & 0x7) << 24)
#define BIT_KPC_KPC_KEY2_ROW_NUM(x)                                             (((x) & 0x7) << 20)
#define BIT_KPC_KPC_KEY2_COL_NUM(x)                                             (((x) & 0x7) << 16)
#define BIT_KPC_KPC_KEY1_ROW_NUM(x)                                             (((x) & 0x7) << 12)
#define BIT_KPC_KPC_KEY1_COL_NUM(x)                                             (((x) & 0x7) << 8)
#define BIT_KPC_KPC_KEY0_ROW_NUM(x)                                             (((x) & 0x7) << 4)
#define BIT_KPC_KPC_KEY0_COL_NUM(x)                                             (((x) & 0x7))

/*REG_KPC_KPC_AUTO_SLP_STATUS*/
#define BIT_KPC_KPC_AUTO_SLP_STATUS                                             BIT(0)

/*REG_KPC_KPC_DEBUG_STATUS0*/
#define BIT_KPC_KPC_ROW_CNT_SYNC2(x)                                            (((x) & 0x7) << 28)
#define BIT_KPC_KPC_COL_CNT_SYNC2(x)                                            (((x) & 0x7) << 24)
#define BIT_KPC_KPC_LONGKEY_MISSING_CNT(x)                                      (((x) & 0xff) << 16)
#define BIT_KPC_KPC_RELEASE_MISSING_CNT(x)                                      (((x) & 0xff) << 8)
#define BIT_KPC_KPC_PRESS_MISSING_CNT(x)                                        (((x) & 0xff))

/*REG_KPC_KPC_DEBUG_STATUS1*/
#define BIT_KPC_KPC_COL_CNT(x)                                                  (((x) & 0x7) << 28)
#define BIT_KPC_KPC_ROW_CNT(x)                                                  (((x) & 0x7) << 24)
#define BIT_KPC_KEYIN_SYNC2(x)                                                  (((x) & 0xff) << 16)
#define BIT_KPC_KEYIN(x)                                                        (((x) & 0xff) << 8)
#define BIT_KPC_KEYOUT(x)                                                       (((x) & 0xff))

/*REG_KPC_KPC_DEBUG_STATUS2*/
#define BIT_KPC_KEY3_FSM_STATUS(x)                                              (((x) & 0xf) << 12)
#define BIT_KPC_KEY2_FSM_STATUS(x)                                              (((x) & 0xf) << 8)
#define BIT_KPC_KEY1_FSM_STATUS(x)                                              (((x) & 0xf) << 4)
#define BIT_KPC_KEY0_FSM_STATUS(x)                                              (((x) & 0xf))

/*REG_KPC_KPC_GLB_CTRL_SET*/
#define BIT_KPC_KPC_LONGKEY_EN                                                  BIT(2)
#define BIT_KPC_KPC_AUTO_SLP_EN                                                 BIT(1)
#define BIT_KPC_KPC_EN                                                          BIT(0)

/*REG_KPC_KPC_MTX_EN_SET*/
#define BIT_KPC_KPC_ROW_EN(x)                                                   (((x) & 0xff) << 8)
#define BIT_KPC_KPC_COL_EN(x)                                                   (((x) & 0xff))

/*REG_KPC_KPC_INT_EN_SET*/
#define BIT_KPC_KPC_GHOSTKEY_INT_EN                                             BIT(3)
#define BIT_KPC_KPC_LONG_INT_EN                                                 BIT(2)
#define BIT_KPC_KPC_RELEASE_INT_EN                                              BIT(1)
#define BIT_KPC_KPC_PRESS_INT_EN                                                BIT(0)

/*REG_KPC_KPC_INT_CLR_SET*/
#define BIT_KPC_KPC_GHOSTKEY_INT_STATUS_CLR                                     BIT(16)
#define BIT_KPC_KEY3_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 12)
#define BIT_KPC_KEY2_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 8)
#define BIT_KPC_KEY1_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 4)
#define BIT_KPC_KEY0_INT_STATUS_CLR(x)                                          (((x) & 0x7))

/*REG_KPC_KPC_POLARITY_SET*/
#define BIT_KPC_KPC_ROW_COL_POL                                                 BIT(0)

/*REG_KPC_KPC_DEBOUNCE_CNT_SET*/
#define BIT_KPC_KPC_DEBOUNCE_CNT_RELEASE(x)                                     (((x) & 0xffff) << 16)
#define BIT_KPC_KPC_DEBOUNCE_CNT_PRESS(x)                                       (((x) & 0xffff))

/*REG_KPC_KPC_LONGKEY_CNT_SET*/
#define BIT_KPC_KPC_LONGKEY_CNT(x)                                              (((x) & 0xffff))

/*REG_KPC_KPC_AUTO_SLP_CNT_SET*/
#define BIT_KPC_KPC_AUTO_SLP_CNT(x)                                             (((x) & 0xffffffff))

/*REG_KPC_KPC_GLB_CTRL_CLR*/
#define BIT_KPC_KPC_LONGKEY_EN                                                  BIT(2)
#define BIT_KPC_KPC_AUTO_SLP_EN                                                 BIT(1)
#define BIT_KPC_KPC_EN                                                          BIT(0)

/*REG_KPC_KPC_MTX_EN_CLR*/
#define BIT_KPC_KPC_ROW_EN(x)                                                   (((x) & 0xff) << 8)
#define BIT_KPC_KPC_COL_EN(x)                                                   (((x) & 0xff))

/*REG_KPC_KPC_INT_EN_CLR*/
#define BIT_KPC_KPC_GHOSTKEY_INT_EN                                             BIT(3)
#define BIT_KPC_KPC_LONG_INT_EN                                                 BIT(2)
#define BIT_KPC_KPC_RELEASE_INT_EN                                              BIT(1)
#define BIT_KPC_KPC_PRESS_INT_EN                                                BIT(0)

/*REG_KPC_KPC_INT_CLR_CLR*/
#define BIT_KPC_KPC_GHOSTKEY_INT_STATUS_CLR                                     BIT(16)
#define BIT_KPC_KEY3_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 12)
#define BIT_KPC_KEY2_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 8)
#define BIT_KPC_KEY1_INT_STATUS_CLR(x)                                          (((x) & 0x7) << 4)
#define BIT_KPC_KEY0_INT_STATUS_CLR(x)                                          (((x) & 0x7))

/*REG_KPC_KPC_POLARITY_CLR*/
#define BIT_KPC_KPC_ROW_COL_POL                                                 BIT(0)

/*REG_KPC_KPC_DEBOUNCE_CNT_CLR*/
#define BIT_KPC_KPC_DEBOUNCE_CNT_RELEASE(x)                                     (((x) & 0xffff) << 16)
#define BIT_KPC_KPC_DEBOUNCE_CNT_PRESS(x)                                       (((x) & 0xffff))

/*REG_KPC_KPC_LONGKEY_CNT_CLR*/
#define BIT_KPC_KPC_LONGKEY_CNT(x)                                              (((x) & 0xffff))

/*REG_KPC_KPC_AUTO_SLP_CNT_CLR*/
#define BIT_KPC_KPC_AUTO_SLP_CNT(x)                                             (((x) & 0xffffffff))

#endif /*__KPC_RF_H__*/