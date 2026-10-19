/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x12fb7c
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_utility.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_endpoint_destroy(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint)
{
    UX_INTERRUPT_SAVE_AREA

    UX_DCD_AIC_ED *ed;
    ULONG   ep, dir;

    /* Keep the physical endpoint address in the endpoint container.  */
    ed = (UX_DCD_AIC_ED *) endpoint->ux_slave_endpoint_ed;

    ep = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress;
    dir = !!(ep & UX_ENDPOINT_DIRECTION);

    UX_DISABLE
    usb_release_endpoint(ep & 0x0f, dir);
    /* We can free this endpoint. */
    ed->ux_dcd_aic_ed_status = UX_DCD_AIC_ED_STATUS_UNUSED;
    ed->ux_dcd_aic_ed_state = UX_DCD_AIC_ED_STATE_IDLE;
    if (!(ep & 0x0f))
        usb_release_endpoint(0, 1);
    UX_RESTORE

    /* This function never fails. */
    return(UX_SUCCESS);
}
