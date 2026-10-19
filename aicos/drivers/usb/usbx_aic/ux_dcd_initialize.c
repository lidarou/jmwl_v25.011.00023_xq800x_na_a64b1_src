/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d62e1
#define LOG_MODULE    AMOD_USB

#include <usb_device.h>
#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_initialize(UDC_Device *ud)
{
    UX_DCD_AIC *dcd_aic;
    UX_SLAVE_DCD *dcd;

    dcd =  &_ux_system_slave->ux_system_slave_dcd;
    dcd_aic = _ux_utility_memory_allocate(UX_NO_ALIGN, UX_REGULAR_MEMORY,
                        sizeof(UX_DCD_AIC));
    if(dcd_aic == UX_NULL)
        return(UX_MEMORY_INSUFFICIENT);

    dcd->ux_slave_dcd_controller_hardware = (VOID *) dcd_aic;
    dcd_aic ->ux_dcd_aic_dcd_owner = dcd;

    dcd->ux_slave_dcd_function = _ux_dcd_aic_function;
    dcd_aic->usb_dev = ud;
    dcd->ux_slave_dcd_status = UX_DCD_STATUS_OPERATIONAL;

    return(UX_SUCCESS);
}

