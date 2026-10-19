/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef USB_PHY_H
#define USB_PHY_H

#include <aic_type.h>

enum source_clk_select {
    SOURCE_CLK_10M,
    SOURCE_CLK_12M,
    SOURCE_CLK_25M,
    SOURCE_CLK_30M,
    SOURCE_CLK_19_2M,
    SOURCE_CLK_24M,
    SOURCE_CLK_27M,
    SOURCE_CLK_40M,
};

struct usbphy {
    u32 xcfgi[4];
    u32 cfg_en;
    enum source_clk_select clk_select;
    u32 ref_clk_switch;
};

u32 *usbphy_cfg_get_xcfgi(void);
void usbphy_cfg_xcfgi_read(void);
void usbphy_cfg_xcfgi_write(void);
void usbphy_cfg_efuse_extract(void);
void usbphy_ref_clock_swicth(void);

#endif /* USB_PHY_H */
