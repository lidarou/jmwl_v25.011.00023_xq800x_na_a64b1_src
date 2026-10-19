/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x004c0a
#define LOG_MODULE    AMOD_USB

#include <ux_api.h>
#include <ux_dcd_aic.h>
#include <ux_utility.h>
#include <ux_device_stack.h>

UINT  _ux_dcd_aic_transfer_abort(UX_DCD_AIC *dcd_aic,
                UX_SLAVE_TRANSFER *transfer_request)
{
    UX_SLAVE_ENDPOINT *endpoint;
    ULONG   ep, dir;

    /* Get the pointer to the logical endpoint from the transfer request. */
    endpoint = transfer_request ->ux_slave_transfer_request_endpoint;
    ep = endpoint->ux_slave_endpoint_descriptor.bEndpointAddress;
    dir = !!(ep & UX_ENDPOINT_DIRECTION);
    usb_abort_endpoint(ep & 0x0f, dir);

    /* This function never fails. */
    return(UX_SUCCESS);
}
