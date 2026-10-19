/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1657bf
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_endpoint_stall(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint)
{
    UX_DCD_AIC_ED *ed;

    ed = (UX_DCD_AIC_ED *) endpoint->ux_slave_endpoint_ed;
    ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;

    usb_endpoint_stall(dcd_aic->usb_dev, endpoint);

    return(UX_SUCCESS);
}
