/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines bcd driver api.
 *    The bcd api will be called by aicos user functions.
 */

#ifndef __BCD_H__
#define __BCD_H__

typedef void (*PMIC_BCD_FUNC)(void *param);

typedef struct pmic_bcd_config {
    PMIC_BCD_FUNC pmic_bcd_f;
    void *uparam;
} PMIC_BCD_CONFIG_T;

typedef enum {
    AIC_BC_TYPE_NO_DCD = 0,
    AIC_BC_TYPE_PS2,
    AIC_BC_TYPE_SDP,
    AIC_BC_TYPE_CDP,
    AIC_BC_TYPE_DCP,
    AIC_USB_UNPLUG,
    AIC_BC_TYPE_DUMMY,
    AIC_BC_TYPE_MAX
} AIC_BC_TYPE;

struct aic_bc_device {
    uint8_t bc_type;
    char bcd_name[30];
};

int aic_is_usb_plugin(void);
int aic_bcd_register(struct aic_bc_device *bc_dev, PMIC_BCD_FUNC bcd_f, void *uparam);
int aic_bcd_unregister(struct aic_bc_device *bc_dev);

#endif /* __BCD_H__ */
