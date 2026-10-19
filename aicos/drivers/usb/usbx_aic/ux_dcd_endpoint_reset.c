/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13b9c6
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_endpoint_reset(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint)
{
    UX_DCD_AIC_ED *ed;

    /* Get the physical endpoint address in the endpoint container.  */
    ed = (UX_DCD_AIC_ED *)endpoint->ux_slave_endpoint_ed;

    /* Set the status of the endpoint to not stalled.  */
    ed->ux_dcd_aic_ed_status &= ~UX_DCD_AIC_ED_STATUS_STALLED;

    /* Set the state of the endpoint to IDLE.  */
    ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;

    /* Clear STALL condition.  */
    usb_endpoint_unstall(dcd_aic->usb_dev, endpoint);

    /* This function never fails.  */
    return(UX_SUCCESS);

}

