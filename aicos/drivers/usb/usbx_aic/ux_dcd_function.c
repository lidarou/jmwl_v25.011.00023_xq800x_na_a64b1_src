/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x03dce1
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_function(UX_SLAVE_DCD *dcd, UINT function,
            VOID *parameter)
{
    UX_DCD_AIC *dcd_aic;
    UINT status = 0U;

    if (dcd->ux_slave_dcd_status == UX_UNUSED) {
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD,
                    UX_SYSTEM_CONTEXT_DCD,
                    UX_CONTROLLER_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer. */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_CONTROLLER_UNKNOWN,
                    0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_CONTROLLER_UNKNOWN);
    }

    dcd_aic = (UX_DCD_AIC *) dcd->ux_slave_dcd_controller_hardware;

    switch (function) {
    case UX_DCD_GET_FRAME_NUMBER:
        status =  _ux_dcd_aic_frame_number_get(dcd_aic,
                        (ULONG *) parameter);
        break;

    case UX_DCD_TRANSFER_REQUEST:
        status = _ux_dcd_aic_transfer_request(dcd_aic,
                    (UX_SLAVE_TRANSFER *) parameter);
        break;

    case UX_DCD_TRANSFER_ABORT:
        status = _ux_dcd_aic_transfer_abort(dcd_aic,
                    (UX_SLAVE_TRANSFER *) parameter);
        break;

    case UX_DCD_CREATE_ENDPOINT:
        status = _ux_dcd_aic_endpoint_create(dcd_aic, parameter);
        break;

    case UX_DCD_DESTROY_ENDPOINT:
        status = _ux_dcd_aic_endpoint_destroy(dcd_aic,
                            parameter);
        break;

    case UX_DCD_RESET_ENDPOINT:
        status = _ux_dcd_aic_endpoint_reset(dcd_aic, parameter);
        break;

    case UX_DCD_STALL_ENDPOINT:
        status =  _ux_dcd_aic_endpoint_stall(dcd_aic, parameter);
        break;

    case UX_DCD_SET_DEVICE_ADDRESS:
        break;

    case UX_DCD_CHANGE_STATE:
        break;

    case UX_DCD_ENDPOINT_STATUS:
        status =  _ux_dcd_aic_endpoint_status(dcd_aic,
                            (ULONG) parameter);
        break;

    default:
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD,
                    UX_SYSTEM_CONTEXT_DCD,
                    UX_FUNCTION_NOT_SUPPORTED);
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR,
                    UX_FUNCTION_NOT_SUPPORTED,
                    0, 0, 0, UX_TRACE_ERRORS, 0, 0)
        status =  UX_FUNCTION_NOT_SUPPORTED;
        break;
    }

    /* Return completion status. */
    return(status);
}

