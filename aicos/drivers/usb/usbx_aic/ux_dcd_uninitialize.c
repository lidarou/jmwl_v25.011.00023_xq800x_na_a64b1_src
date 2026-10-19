/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f7640
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_uninitialize(AIC_DEV *ud)
{
    UX_DCD_AIC *dcd_aic;
    UX_SLAVE_DCD *dcd;

    /* Get the pointer to the DCD.  */
    dcd = &_ux_system_slave->ux_system_slave_dcd;

    /* Set the state of the controller to HALTED now.  */
    dcd->ux_slave_dcd_status =  UX_DCD_STATUS_HALTED;

    /* Get controller driver.  */
    dcd_aic = (UX_DCD_AIC *)dcd->ux_slave_dcd_controller_hardware;

    _ux_utility_memory_free(dcd_aic);
    dcd->ux_slave_dcd_controller_hardware = UX_NULL;

    return(UX_SUCCESS);
}
