/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05deea
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_type.h>
#include <usb/usb_phy_cfg.h>

/* PHY configuration register offset */
#define USBPHY_XTLSEL          0x0
#define USBPHY_WRITE_PROT           0x04
#define EFUSE2XCFGI                 0x58
#define USBPHY_LOCK_KEY_WORD        0xAA55FF00
#define USBPHY_CALI_PARA(v)     ((v) & 0xffff)
#define USBPHY_XCFGI1           0x10
#define USBPHY_XCFGI2           0x0c
#define USBPHY_XCFGI3           0x08
#define USBPHY_EFUSE2XCFGI1_B2 (0x01 << 2)
#define USBPHY_EFUSE2XCFGI1_B4_B6 (0x07 << 4)
#define USBPHY_EFUSE2XCFGI1_B13_B15 (0x07 << 13)
#define USBPHY_EFUSE2XCFGI1_B24_B25 (0x03 << 24)
#define USBPHY_EFUSE2XCFGI1_B29_B31 (0x07 << 29)
#define USBPHY_EFUSE2XCFGI2_B34_B37 (0x0f << 2)

#define USBPHY_EFUSE2XCFGI1_B2_MASK (0x01)
#define USBPHY_EFUSE2XCFGI1_B4_B6_MASK (0x07 << 1)
#define USBPHY_EFUSE2XCFGI1_B13_B15_MASK (0x07 << 4)
#define USBPHY_EFUSE2XCFGI1_B24_B25_MASK (0x03 << 7)
#define USBPHY_EFUSE2XCFGI1_B29_B31_MASK (0x07 << 9)
#define USBPHY_EFUSE2XCFGI2_B34_B37_MASK (0x0f << 12)

#define XCFGI2EFUSE_EXTRACT(v, w) (\
    (((v) & USBPHY_EFUSE2XCFGI1_B2) >> 2) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B4_B6) >> 4) << 1) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B13_B15) >> 13) << 4) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B24_B25) >> 24) << 7) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B29_B31) >> 29) << 9) | \
    ((((w) & USBPHY_EFUSE2XCFGI2_B34_B37) >> 2) << 12) \
    )

#define EFUSE2XCFGI1_FILL(v) (\
    ((v) & USBPHY_EFUSE2XCFGI1_B2_MASK) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B4_B6_MASK) >> 1) << 4) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B13_B15_MASK) >> 4) << 13) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B24_B25_MASK) >> 7) << 24) | \
    ((((v) & USBPHY_EFUSE2XCFGI1_B29_B31_MASK) >> 9) << 29) \
    )

#define EFUSE2XCFGI2_FILL(v) (\
    ((((v) & USBPHY_EFUSE2XCFGI2_B34_B37_MASK) >> 12) << 2) \
    )

void usbphy_cfg_lock(void)
{
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_WRITE_PROT, 0);
}

void usbphy_cfg_unlock(void)
{
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_WRITE_PROT, USBPHY_LOCK_KEY_WORD);
}

void usbphy_cfg_xcfgi_read(void)
{
    uphy.xcfgi[0] = AIC_READ32(USBPHY_CFG_BASE + EFUSE2XCFGI);
    uphy.xcfgi[1] = AIC_READ32(USBPHY_CFG_BASE + USBPHY_XCFGI1);
    uphy.xcfgi[2] = AIC_READ32(USBPHY_CFG_BASE + USBPHY_XCFGI2);
    uphy.xcfgi[3] = AIC_READ32(USBPHY_CFG_BASE + USBPHY_XCFGI3);
    uphy.xcfgi[1] = uphy.xcfgi[1] | EFUSE2XCFGI1_FILL(uphy.xcfgi[0]);
    uphy.xcfgi[2] = uphy.xcfgi[2] | EFUSE2XCFGI2_FILL(uphy.xcfgi[0]);
}

void usbphy_cfg_xcfgi_write(void)
{
    usbphy_cfg_unlock();
    AIC_WRITE32(USBPHY_CFG_BASE + EFUSE2XCFGI, uphy.xcfgi[0]);
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_XCFGI1, uphy.xcfgi[1]);
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_XCFGI2, uphy.xcfgi[2]);
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_XCFGI3, uphy.xcfgi[3]);
    usbphy_cfg_lock();
}

void usbphy_cfg_efuse_extract(void)
{
    uphy.xcfgi[0] = XCFGI2EFUSE_EXTRACT(uphy.xcfgi[1], uphy.xcfgi[2]);
}

u32 *usbphy_cfg_get_xcfgi(void)
{
    return uphy.xcfgi;
}

void usbphy_ref_clock_swicth(void)
{
    if (!uphy.ref_clk_switch)
        return;

    usbphy_cfg_unlock();
    AIC_WRITE32(USBPHY_CFG_BASE + USBPHY_XTLSEL, uphy.clk_select);
    usbphy_cfg_lock();
}

int usbphy_cfg_init(void)
{
    if (uphy.cfg_en) {
        usbphy_cfg_efuse_extract();
        usbphy_cfg_xcfgi_write();
    }
    usbphy_cfg_xcfgi_read();

    return AIC_SUCCESS;
}
DIS_BASIC_DRV_P2_INIT_EXPORT(usbphy_cfg_init);
