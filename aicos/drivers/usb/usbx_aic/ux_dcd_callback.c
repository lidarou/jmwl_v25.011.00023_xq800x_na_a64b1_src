/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19a2bc
#define LOG_MODULE    AMOD_USB

#define UX_SOURCE_CODE
#define UX_DCD_AIC_SOURCE_CODE


/* Include necessary system files.  */

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>
#include <ux_utility.h>

void _ux_dcd_aic_DataInStageCallback(uint8_t epnum)
{
    uint8_t index = ((epnum & 0xF) << 1) + !!(epnum & UX_ENDPOINT_DIRECTION);
    UX_SLAVE_TRANSFER *transfer_request;
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_aic->ux_dcd_aic_ed[index];

    /* Get the pointer to the transfer request.  */
    transfer_request =
        &(ed ->ux_dcd_aic_ed_endpoint->ux_slave_endpoint_transfer_request);

    if (transfer_request->ux_slave_transfer_request_status == UX_TRANSFER_STATUS_ABORT)
        return;

    /* Update the length of the data sent in previous transaction.  */
    transfer_request->ux_slave_transfer_request_actual_length =
                                       usb_get_actual_length(epnum);

    /* Set the completion code to no error.  */
    transfer_request->ux_slave_transfer_request_completion_code =
                                UX_SUCCESS;

    /* The transfer is completed.  */
    transfer_request->ux_slave_transfer_request_status =
                    UX_TRANSFER_STATUS_COMPLETED;

    /* Non control endpoint operation, use semaphore.  */
    _ux_utility_semaphore_put(
        &transfer_request->ux_slave_transfer_request_semaphore);
}

void _ux_dcd_aic_DataOutStageCallback(uint8_t epnum)
{
    uint8_t index = ((epnum & 0xF) << 1) + !!(epnum & UX_ENDPOINT_DIRECTION);
    UX_SLAVE_TRANSFER *transfer_request;
    UX_DCD_AIC *dcd_aic;
    UX_DCD_AIC_ED *ed;
    UX_SLAVE_DCD *dcd;

    /* Get the pointer to the DCD.  */
    dcd =  &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Fetch the address of the physical endpoint.  */
    ed =  &dcd_aic->ux_dcd_aic_ed[index];

    /* Get the pointer to the transfer request.  */
    transfer_request =
        &(ed->ux_dcd_aic_ed_endpoint->ux_slave_endpoint_transfer_request);

    if (transfer_request->ux_slave_transfer_request_status == UX_TRANSFER_STATUS_ABORT)
        return;

    /* Update the length of the data sent in previous transaction.  */
    transfer_request->ux_slave_transfer_request_actual_length =
                                       usb_get_actual_length(epnum);

    /* Set the completion code to no error.  */
    transfer_request->ux_slave_transfer_request_completion_code =
                                UX_SUCCESS;

    /* The transfer is completed.  */
    transfer_request->ux_slave_transfer_request_status =
                        UX_TRANSFER_STATUS_COMPLETED;

    /* Non control endpoint operation, use semaphore.  */
    _ux_utility_semaphore_put(
        &transfer_request->ux_slave_transfer_request_semaphore);
}
