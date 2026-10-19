/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0784f9
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_init.h>
#include <alog_api.h>
#include <cache.h>
#include <usb_componets.h>
#include <stdint.h>
#include <usb/cdc_acm_cfg.h>
#include <ux_device_stack.h>

#ifdef CONFIG_USB_DEBUG
#define USB_PRINTF    alog_error
#else
#define USB_PRINTF(...)
#endif

size_t usb_cdc_acm_read(void *handle, void *buf, size_t length)
{
    struct usb_cdc_acm *uca = (struct usb_cdc_acm *)handle;
    uint32_t actual_length;
    uint32_t ux_status;

    if (!uca->status)
        return 0;

    ux_status = ux_device_class_cdc_acm_read(uca->cdc_acm, buf, length,
                        &actual_length);
    if (ux_status) {
        aicx_error(0x1acdbd, "sd", "%s usb_serial_read error ret =%d\n", uca->name, ux_status);
        return 0;
    }

    return actual_length;
}

size_t usb_cdc_acm_write(void *handle, const void *buf, size_t length)
{
    struct usb_cdc_acm *uca = (struct usb_cdc_acm *)handle;
    uint32_t actual_length;
    uint32_t ux_status;

    if (!uca->status)
        return 0;

    ux_status = ux_device_class_cdc_acm_write(uca->cdc_acm, (UCHAR *)buf, length,
                        &actual_length);
    if (ux_status) {
        aicx_error(0x08c7f6, "sd", "%s usb_serial_write error ret = %d\n", uca->name, ux_status);
        return 0;
    }

    return actual_length;
}

enum vcom_event usb_cdc_acm_get_state(void *handle)
{
    struct usb_cdc_acm *uca = (struct usb_cdc_acm *)handle;

    return uca->status;
}

void usb_cdc_acm_stop_transmission(UX_SLAVE_CLASS_CDC_ACM *cdc_acm)
{
    UX_SLAVE_INTERFACE          *interface;
    UX_SLAVE_ENDPOINT           *endpoint_in;
    UX_SLAVE_ENDPOINT           *endpoint_out;

    /* We need the interface to the class.  */
    interface =  cdc_acm->ux_slave_class_cdc_acm_interface;
    if (!interface)
        return;

    /* Locate the endpoints.  */
    endpoint_in =  interface->ux_slave_interface_first_endpoint;
    if (!endpoint_in)
        return;
    /* Check the endpoint direction, if IN we have the correct endpoint. */
    if ((endpoint_in->ux_slave_endpoint_descriptor.bEndpointAddress &
        UX_ENDPOINT_DIRECTION) != UX_ENDPOINT_IN) {
        /* Wrong direction, we found the OUT endpoint first. */
        endpoint_out =  endpoint_in;
        /* So the next endpoint has to be the IN endpoint. */
        endpoint_in =  endpoint_out->ux_slave_endpoint_next_endpoint;
    } else {
        /* We found the endpoint IN first, so next endpoint is OUT. */
        endpoint_out =  endpoint_in->ux_slave_endpoint_next_endpoint;
    }

        /* Terminate the transactions pending on the endpoints. */
    _ux_device_stack_transfer_all_request_abort(endpoint_in, UX_TRANSFER_BUS_RESET);
    _ux_device_stack_transfer_all_request_abort(endpoint_out, UX_TRANSFER_BUS_RESET);
}

void *usb_cdc_acm_open(int port, CDC_ACM_CB func)
{
    if (port >= USB_CDC_ACM_INSTANCE_MAX)
        return NULL;

    usb_cdc_acm[port].cb = func;
    return (void*)&usb_cdc_acm[port];
}

void usb_cdc_acm_close(void *handle)
{
    struct usb_cdc_acm *uca = (struct usb_cdc_acm *)handle;

    if (!uca)
        return;

    uca->cb = NULL;
    if (uca->cdc_acm)
        usb_cdc_acm_stop_transmission(uca->cdc_acm);

}

struct usb_cdc_acm *
usb_cdc_acm_2_vcom(UX_SLAVE_CLASS_CDC_ACM *cdc_acm)
{
    UX_SLAVE_INTERFACE *cdc_interface;
    uint32_t if_num, i;

    cdc_interface = cdc_acm->ux_slave_class_cdc_acm_interface;
    if_num = cdc_interface->ux_slave_interface_descriptor.bInterfaceNumber;

    for (i = 0; i < USB_CDC_ACM_INSTANCE_MAX; i++)
        if (usb_cdc_acm[i].if_num == if_num)
                    break;

    if (i == USB_CDC_ACM_INSTANCE_MAX)
            return NULL;

    usb_cdc_acm[i].cdc_acm = cdc_acm;
    return &usb_cdc_acm[i];
}

void usb_cdc_acm_instance_activate(void *cdc_acm)
{
    struct usb_cdc_acm *uca;

    uca = usb_cdc_acm_2_vcom((UX_SLAVE_CLASS_CDC_ACM *)cdc_acm);
    if (!uca) {
        aicx_error(0x0e59fa, "", "do not find register usb_cdc_acm.\r\n");
        return;
    }
    aicx_error(0x04ce0b, "s", "cdc %s activate\n", uca->name);
    uca->status = VCOM_ACTIVE;
    if (uca->cb)
       uca->cb(uca->index, VCOM_ACTIVE);
}

void usb_cdc_acm_instance_deactivate(void *cdc_acm)
{
    struct usb_cdc_acm *uca;

    uca = usb_cdc_acm_2_vcom((UX_SLAVE_CLASS_CDC_ACM *)cdc_acm);
    if (!uca) {
        aicx_error(0x025a6f, "", "deactivate can not find register usb cdc acm.\r\n");
        return;
    }

    aicx_error(0x0cee76, "s", "cdc %s deactivate\n", uca->name);
    if (uca->status == VCOM_DEACTIVE)
        return;
    uca->status = VCOM_DEACTIVE;
    if (uca->cb)
       uca->cb(uca->index, VCOM_DEACTIVE);
}

int usb_cdc_acm_uninit(void)
{
    uint32_t status;
    int i;

    for (i = 0; i< USB_CDC_ACM_INSTANCE_MAX; i++) {
        if (usb_cdc_acm[i].cb)
           usb_cdc_acm[i].cb(usb_cdc_acm[i].index, VCOM_DEACTIVE);
        status = _ux_device_stack_class_unregister(
                    _ux_system_slave_class_cdc_acm_name,
                    ux_device_class_cdc_acm_entry);
        if (status != UX_SUCCESS) {
            aicx_error(0x05eb2b, "d", "cdc class unregister. status = %d\n", status);
            return AIC_EIO;
        }
    }
    return AIC_SUCCESS;
}

int usb_cdc_acm_init(void)
{
    UX_SLAVE_CLASS_CDC_ACM_PARAMETER  parameter;
    uint32_t status;
    int i;

    /* Set the parameters for callback when insertion/extraction
     * of a Data Pump device. */
    parameter.ux_slave_class_cdc_acm_instance_activate =
        usb_cdc_acm_instance_activate;
    parameter.ux_slave_class_cdc_acm_instance_deactivate =
        usb_cdc_acm_instance_deactivate;
    parameter.ux_slave_class_cdc_acm_parameter_change = NULL;

    /* Initialize the device dpump class. The class is
     * connected with interface 0 */
    for (i = 0; i < USB_CDC_ACM_INSTANCE_MAX; i++) {
        status = _ux_device_stack_class_register(
                    _ux_system_slave_class_cdc_acm_name,
                    ux_device_class_cdc_acm_entry, 1,
                    usb_cdc_acm[i].if_num, &parameter);
        if (status != UX_SUCCESS) {
            aicx_error(0x1c2ea3, "dd", "stack_class_register cdc%d. status = %d\n", i, status);
            return AIC_EIO;
        }
        aicx_error(0x1a2cbe, "s", "%s register success\n", usb_cdc_acm[i].name);
    }

    return AIC_SUCCESS;
}

struct usb_class_instance cdc_acm_instance = {
    .init = usb_cdc_acm_init,
    .uninit =usb_cdc_acm_uninit,
    .status = 1,
    .name = "usb cdc acm",
};

int usb_cdc_acm_register(void)
{
    usb_service_register_instance(LOG_AT_DIAG, cdc_acm_instance);
    return AIC_SUCCESS;
}
DIS_BASIC_COMP_P1_INIT_EXPORT(usb_cdc_acm_register);
