/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

#ifndef __USB_PHY_CFG_H__
#define __USB_PHY_CFG_H__

#include <aic_type.h>
#include <top_glb.h>
#include <usb_phy.h>

#define USBPHY_CFG_BASE         REG_TOP_GLB_XTLSEL

struct usbphy uphy = {
    .cfg_en = 0,
    .xcfgi = {
        0x0, /* efuse xcfgi */
        0x0,0x0,0x0, /* xcfgi bit0 ~ bit70 */
    },
};

#endif  /* __USB_PHY_CFG_H__ */
