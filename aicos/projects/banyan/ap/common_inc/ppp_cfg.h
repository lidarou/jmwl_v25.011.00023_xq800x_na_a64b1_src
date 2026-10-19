/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __PPP_CFG_H__
#define __PPP_CFG_H__

#include <aic_reg_base.h>
#include <aic_auto_reg.h>
#include <irq_chip.h>
#include <top_glb.h>
#include <aon_glb.h>
#include <mcu_glb.h>
#include <ckm_en_glb_mcu.h>

#define PPP_CLK_FORCE_ON
#define PPPD_DISABLE_DATA_SWAP

#define PPP_ENCODE_IRQ_NUM                        IRQ_PPPE
#define PPP_DECODE_IRQ_NUM                        IRQ_PPPD

#define PPP_CLK_125M                              0
#define PPP_CLK_26M                               1

#define PPP_DECODE_DEFAULT_CLK                    PPP_CLK_125M
#define PPP_ENCODE_DEFAULT_CLK                    PPP_CLK_125M

#define PPP_DECODE_CORE_CLK                       0x42010020
#define PPP_ENCODE_CORE_CLK                       0x42010024

#define PPP_ENCODE_IP_POWER                       REG_CKM_EN_GLB_MCU_MCU_PPPE_EB
#define PPP_DECODE_IP_POWER                       REG_CKM_EN_GLB_MCU_MCU_PPPD_EB

#define PPP_ENCODE_REG_START                      REG_BASE_BLK_MCU_PPP_E
#define PPP_DECODE_REG_START                      REG_BASE_BLK_MCU_PPP_D

#endif /* __GPIO_BOARD_CFG_H__ */

