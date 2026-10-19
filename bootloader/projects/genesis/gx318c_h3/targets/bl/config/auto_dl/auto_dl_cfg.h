/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __AUTO_DL_CFG_H__
#define __AUTO_DL_CFG_H__

#include <gpio_inc.h>
#include <reg_base.h>
#include <pinlist.h>

struct auto_download {
    int gpio_num;
    int gpio_pin_base;
    int gpio_func_sel;
};

struct auto_download auto_dl = {
    .gpio_num = GPIO_AON_7,
    .gpio_pin_base = REG_IOMUX_SSP1_CLK,
    .gpio_func_sel = BIT_FUNC_SEL(6),
};

#endif /* __AUTO_DL_CFG_H__ */