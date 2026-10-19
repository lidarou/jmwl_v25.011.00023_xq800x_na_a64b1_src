/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PPP_CFG_H__
#define __PPP_CFG_H__

#include <aic_auto_reg.h>
#include <aic_reg_base.h>
#include <cru_mcu.h>
#include <irq.h>

#define PPP_ENCODE_IRQ_NUM                        18
#define PPP_DECODE_IRQ_NUM                        17

#define PPP_CLK_125M                              0
#define PPP_CLK_26M                               1

#define PPP_DECODE_DEFAULT_CLK                    PPP_CLK_125M
#define PPP_ENCODE_DEFAULT_CLK                    PPP_CLK_26M

#define PPP_DECODE_CORE_CLK                       REG_CRU_MCU_CLK_MCU_PPP_D_CFG
#define PPP_ENCODE_CORE_CLK                       REG_CRU_MCU_CLK_MCU_PPP_E_CFG

#define PPP_ENCODE_IP_POWER                       REG_MCU_GLB_PPP_E_EB
#define PPP_DECODE_IP_POWER                       REG_MCU_GLB_PPP_D_EB

#define PPP_ENCODE_REG_START                      REG_BASE_AP_PPP_ENCODER
#define PPP_DECODE_REG_START                      REG_BASE_AP_PPP_DECODER

#endif /* __GPIO_BOARD_CFG_H__ */

