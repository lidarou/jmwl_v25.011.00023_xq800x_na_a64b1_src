/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c48a7
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_utility.h>
#include <ux_device_stack.h>

UINT _ux_dcd_aic_transfer_request(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_TRANSFER *transfer_request)
{
    UX_SLAVE_ENDPOINT *endpoint;
    ULONG *requested_length;
    UCHAR *data_pointer;
    ULONG  *actual_length;
    ULONG  *request_phase;
    ULONG  *completion_code;
    ULONG *ep_addr;
    UINT status;

    endpoint = transfer_request->ux_slave_transfer_request_endpoint;
    ep_addr = &endpoint->ux_slave_endpoint_descriptor.bEndpointAddress;
    requested_length =
        &transfer_request->ux_slave_transfer_request_requested_length;
    data_pointer =
        transfer_request->ux_slave_transfer_request_data_pointer;
    request_phase =
        &transfer_request->ux_slave_transfer_request_phase;
    completion_code =
        &transfer_request->ux_slave_transfer_request_completion_code;
    actual_length =
        &transfer_request->ux_slave_transfer_request_actual_length;

    if (*request_phase == UX_TRANSFER_PHASE_DATA_OUT) {
       status = usb_EpXDataSend(endpoint, data_pointer, *requested_length);
    } else {
       status = usb_EpXDataReceive(endpoint, data_pointer, *requested_length);
    }
    if (status)
        return(status);

    /* If the endpoint is a Control endpoint, all this is happening
     * under Interrupt and there is nothread to suspend. */
    if (*ep_addr & (UINT)~UX_ENDPOINT_DIRECTION) {
        status = _ux_utility_semaphore_get(
            &transfer_request->ux_slave_transfer_request_semaphore,
            UX_WAIT_FOREVER);
        if (status)
            return(status);

        if (*request_phase == UX_TRANSFER_PHASE_DATA_OUT)
            *actual_length = *requested_length;

        return *completion_code;
    }

    return(UX_SUCCESS);
}
