/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1629f3
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_init.h>
#include <cache.h>
#include <usb_componets.h>
#include <usb_service.h>
#include <usb/rndis_cfg.h>
#include <stdint.h>
#include <ux_api.h>
#include <ux_device_stack.h>
#include <ux_device_class_rndis.h>

#define USB_RNDIS_XMIT_QUEUE_ROOM_SIZE_DEFAULT          32

struct usb_rndis {
    UX_SLAVE_CLASS_RNDIS *rndis;
    int status;
    u32 media_state;
};
struct usb_rndis gx_rndis;

void usb_rndis_single_connect(void)
{
    if (!gx_rndis.rndis)
        return;

    gx_rndis.media_state = UX_DEVICE_CLASS_RNDIS_MEDIA_CONNECTED;
    _ux_device_class_rndis_msg_indicate_status(gx_rndis.rndis, UX_DEVICE_CLASS_RNDIS_STATUS_MEDIA_CONNECTED);
}

void usb_rndis_single_disconnect(void)
{
    if (!gx_rndis.rndis)
        return;

    gx_rndis.media_state = UX_DEVICE_CLASS_RNDIS_MEDIA_DISCONNECTED;
    _ux_device_class_rndis_msg_indicate_status(gx_rndis.rndis, UX_DEVICE_CLASS_RNDIS_STATUS_MEDIA_DISCONNECT);
    _ux_utility_event_flags_set(&gx_rndis.rndis-> ux_slave_class_rndis_event_flags_group,
        UX_DEVICE_CLASS_RNDIS_NEW_DEVICE_STATE_CHANGE_EVENT, UX_OR);
}

u32 usb_rndis_single_get_state(void)
{
    return gx_rndis.media_state;
}

void usb_rndis_instance_activate(void *data)
{
    gx_rndis.media_state = UX_DEVICE_CLASS_RNDIS_MEDIA_CONNECTED;
    gx_rndis.rndis = (UX_SLAVE_CLASS_RNDIS *)data;
}

void usb_rndis_instance_deactivate(void *data)
{
    gx_rndis.media_state = UX_DEVICE_CLASS_RNDIS_MEDIA_DISCONNECTED;
    gx_rndis.rndis = NULL;
}

int usb_rndis_update_xmit_queue_room_size(u32 size)
{
    if (!gx_rndis.rndis)
        return AIC_EIO;

    gx_rndis.rndis->ux_slave_class_rndis_xmit_queue_room_size = size;
    return AIC_SUCCESS;
}

void usb_rndis_initialize_complete(void)
{
    usb_service_network_status_report(USB_RNDIS);
}

int usb_rndis_uninit(void)
{
    uint32_t status;

    if (!gx_rndis.status)
       return AIC_SUCCESS;

    status = _ux_device_stack_class_unregister(
                    _ux_system_slave_class_rndis_name,
                    ux_device_class_rndis_entry);
    if (status != UX_SUCCESS) {
        aicx_error(0x1d5b80, "d", "rndis class unregister. status = %d\n", status);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

int usb_rndis_init(void)
{
    UX_SLAVE_CLASS_RNDIS_PARAMETER  parameter;
    uint32_t status;

    /* Set the parameters for callback when insertion/extraction of a CDC device. Set to NULL.*/
    parameter.ux_slave_class_rndis_instance_activate = usb_rndis_instance_activate;
    parameter.ux_slave_class_rndis_instance_deactivate = usb_rndis_instance_deactivate;
    parameter.ux_slave_class_rndis_initialize_complete = usb_rndis_initialize_complete;
    /* Set extra parameters used by the RNDIS query command with certain OIDs. */
    parameter.ux_slave_class_rndis_parameter_vendor_id = VENDOR_ID ;
    parameter.ux_slave_class_rndis_parameter_driver_version = PRODUCT_ID;
    parameter.ux_slave_class_rndis_parameter_xmit_queue_room_size = USB_RNDIS_XMIT_QUEUE_ROOM_SIZE_DEFAULT;

    /* Define a local NODE ID. */
    ux_utility_memory_copy(
        parameter.ux_slave_class_rndis_parameter_local_node_id,
        rndis_local_node_id, 6);
    /* Define a remote NODE ID. */
    ux_utility_memory_copy(
        parameter.ux_slave_class_rndis_parameter_remote_node_id,
        rndis_remote_node_id, 6);

    ux_utility_memory_copy(
        parameter.ux_slave_class_rndis_parameter_vendor_description,
        RNDIS_VENDOR_DESC, 13);
    /* Initialize the device rndis class. This class owns both interfaces. */
    status = ux_device_stack_class_register(_ux_system_slave_class_rndis_name,
        ux_device_class_rndis_entry, 1,0, &parameter);
    aicx_error(0x0aad77, "x", "rndis register ret =0x%x.\n", status);
    gx_rndis.status = 1;

    return status;
}

struct usb_class_instance rndis_instance = {
    .init = usb_rndis_init,
    .uninit =usb_rndis_uninit,
    .status = 1,
    .name = "usb rndis",
};

int usb_rndis_register(void)
{
    usb_service_register_instance(RNDIS, rndis_instance);
    return AIC_SUCCESS;
}
DIS_BASIC_COMP_P1_INIT_EXPORT(usb_rndis_register);
