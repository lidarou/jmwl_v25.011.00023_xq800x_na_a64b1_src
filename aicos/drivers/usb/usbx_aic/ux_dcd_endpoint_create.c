/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1286a2
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>
#include <usb_device.h>

UINT  _ux_dcd_aic_endpoint_create(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_ENDPOINT *endpoint)
{
    usb_device_endpoint_init_struct_t epInitStruct;
    UX_ENDPOINT_DESCRIPTOR *ep_desc;
    UX_DCD_AIC_ED *ed;
    ULONG index;

    ep_desc = &endpoint->ux_slave_endpoint_descriptor;
    index = ((ep_desc->bEndpointAddress & 0xF) << 1) +
        !!(ep_desc->bEndpointAddress & UX_ENDPOINT_DIRECTION);
    if (index >= UX_MAX_ED)
        return(UX_NO_ED_AVAILABLE);

    ed =  &dcd_aic->ux_dcd_aic_ed[index];
    if (ed->ux_dcd_aic_ed_status & UX_DCD_AIC_ED_STATUS_USED)
        return(UX_NO_ED_AVAILABLE);

    ed->ux_dcd_aic_ed_status |= UX_DCD_AIC_ED_STATUS_USED;

    /* Keep the physical endpoint address in the endpoint
     * container. */
    endpoint->ux_slave_endpoint_ed = (VOID *)ed;

    /* Save the endpoint pointer. */
    ed->ux_dcd_aic_ed_endpoint = endpoint;

    /* And its index. */
    ed->ux_dcd_aic_ed_index = index;

    /* And its direction. */
    ed->ux_dcd_aic_ed_direction = ep_desc->bEndpointAddress &
                    UX_ENDPOINT_DIRECTION;
    epInitStruct.zlt = 1U;
    epInitStruct.transferType = ep_desc->bmAttributes &
                UX_MASK_ENDPOINT_TYPE;
    epInitStruct.interval = ep_desc->bInterval;
    epInitStruct.endpointAddress = ep_desc->bEndpointAddress;
    epInitStruct.maxPacketSize = ep_desc->wMaxPacketSize;
    USB_AIDXPF(0x0e635d, "xx", "endpoint_create = 0x%x, bmAttributes = 0x%x\n",
            ep_desc->bEndpointAddress, epInitStruct.transferType);
    usb_endpoint_create(dcd_aic->usb_dev, &epInitStruct);

    if (!index) {
        epInitStruct.endpointAddress = index | UX_ENDPOINT_DIRECTION;
        usb_endpoint_create(dcd_aic->usb_dev, &epInitStruct);
    }

    return(UX_SUCCESS);
}