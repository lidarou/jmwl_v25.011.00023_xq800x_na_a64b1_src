/*****************************************************************************/
/*                                                                           */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __BL_PIN_CFG_H__
#define __BL_PIN_CFG_H__

#include <aic_type.h>
#include <pinlist.h>

const u32 pin_bl_cfg[][2] = {
    //Add pin configurations which wanted in bootloader, e.g.
    //{REG_IOMUX_KEYIN4,                    BIT_FUNC_SEL(1)},//SENSOR_SDA,
    //{REG_PG12_PAD_CTRL_KEYIN4,            BIT_PAD_DRV(1)|BIT_PAD_SSPU|BIT_PAD_SLP_MCU|BIT_PAD_SLP_WPU|BIT_PAD_SLP_Z},
};

#endif /* __BL_PIN_CFG_H__ */

