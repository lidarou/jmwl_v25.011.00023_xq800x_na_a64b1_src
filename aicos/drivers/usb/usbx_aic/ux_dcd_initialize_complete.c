/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0be3c2
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_system.h>
#include <ux_utility.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT _ux_dcd_aic_initialize_complete(VOID)
{
    UX_SLAVE_TRANSFER *transfer_request;
    UX_SLAVE_ENDPOINT *endpoint;
    UX_DCD_AIC *dcd_aic;
    UX_SLAVE_DEVICE *device;
    UCHAR *device_framework;
    ULONG *state_machine;
    UX_SLAVE_DCD *dcd;

    dcd = &_ux_system_slave->ux_system_slave_dcd;

    /* Get the pointer to the AIC DCD.  */
    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    /* Get the pointer to the device.  */
    device =  &_ux_system_slave->ux_system_slave_device;
    state_machine =
        &_ux_system_slave->ux_system_slave_device_dfu_state_machine;
    /* Are we in DFU mode ? If so, check if we are in a Reset mode.  */
    if (*state_machine == UX_SYSTEM_DFU_STATE_APP_DETACH) {
        _ux_system_slave->ux_system_slave_device_framework =
            _ux_system_slave->ux_system_slave_dfu_framework;
        _ux_system_slave->ux_system_slave_device_framework_length =
            _ux_system_slave->ux_system_slave_dfu_framework_length;

    } else {
        *state_machine = UX_SYSTEM_DFU_STATE_APP_IDLE;
        if (_ux_system_slave->ux_system_slave_speed == UX_FULL_SPEED_DEVICE) {
            _ux_system_slave->ux_system_slave_device_framework =
            _ux_system_slave->ux_system_slave_device_framework_full_speed;
            _ux_system_slave->ux_system_slave_device_framework_length =
            _ux_system_slave->ux_system_slave_device_framework_length_full_speed;
        } else {
            _ux_system_slave->ux_system_slave_device_framework =
            _ux_system_slave->ux_system_slave_device_framework_high_speed;
            _ux_system_slave->ux_system_slave_device_framework_length =
            _ux_system_slave->ux_system_slave_device_framework_length_high_speed;
        }
    }

    device_framework = _ux_system_slave->ux_system_slave_device_framework;
    _ux_utility_descriptor_parse(device_framework,
                                _ux_system_device_descriptor_structure,
                                UX_DEVICE_DESCRIPTOR_ENTRIES,
                                (UCHAR *) &device->ux_slave_device_descriptor);

    /* Now we create a transfer request to accept the first SETUP packet
       and get the ball running. First get the address of the endpoint
       transfer request container.  */
    endpoint = &device->ux_slave_device_control_endpoint;
    transfer_request = &endpoint->ux_slave_endpoint_transfer_request;
    transfer_request->ux_slave_transfer_request_timeout =
            UX_MS_TO_TICK(UX_CONTROL_TRANSFER_TIMEOUT);
    transfer_request->ux_slave_transfer_request_current_data_pointer =
            transfer_request->ux_slave_transfer_request_data_pointer;

    /* Update the transfer request endpoint pointer with the
     * default endpoint. */
    transfer_request->ux_slave_transfer_request_endpoint = endpoint;

    /* The control endpoint max packet size needs to be filled manually
     * in its descriptor. */
    endpoint->ux_slave_endpoint_descriptor.wMaxPacketSize =
            device->ux_slave_device_descriptor.bMaxPacketSize0;

    /* On the control endpoint, always expect the maximum. */
    transfer_request->ux_slave_transfer_request_requested_length =
            device->ux_slave_device_descriptor.bMaxPacketSize0;

    /* Attach the control endpoint to the transfer request. */
    transfer_request->ux_slave_transfer_request_endpoint = endpoint;

    /* Create the default control endpoint attached to the device.
       Once this endpoint is enabled, the host can then send a setup packet
       The device controller will receive it and will call the setup function
       module.  */
    dcd->ux_slave_dcd_function(dcd, UX_DCD_CREATE_ENDPOINT, (VOID *)endpoint);

    /* Ensure the control endpoint is properly reset.  */
    endpoint->ux_slave_endpoint_state = UX_ENDPOINT_RESET;
    transfer_request->ux_slave_transfer_request_type =
                        UX_TRANSFER_PHASE_SETUP;
    transfer_request->ux_slave_transfer_request_status =
                        UX_TRANSFER_STATUS_PENDING;
    transfer_request->ux_slave_transfer_request_requested_length =
                                UX_SETUP_SIZE;
    transfer_request->ux_slave_transfer_request_in_transfer_length =
                                UX_SETUP_SIZE;

    /* Reset the number of bytes sent/received. */
    transfer_request->ux_slave_transfer_request_actual_length = 0;

    /* Inform the application if a callback function was programmed. */
    if (_ux_system_slave->ux_system_slave_change_function)
        _ux_system_slave->ux_system_slave_change_function(UX_DEVICE_ATTACHED);

    /* If trace is enabled, insert this event into the trace buffer. */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_DEVICE_STACK_CONNECT, 0, 0, 0, 0,
                UX_TRACE_DEVICE_STACK_EVENTS, 0, 0)

    /* If trace is enabled, register this object.  */
    UX_TRACE_OBJECT_REGISTER(UX_TRACE_DEVICE_OBJECT_TYPE_DEVICE, device,
                0, 0, 0)

    return(UX_SUCCESS);
}
