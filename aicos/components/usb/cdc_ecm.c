/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07b1d9
#define LOG_MODULE    AMOD_USB

#include <aic_errno.h>
#include <aic_init.h>
#include <cache.h>
#include <usb/ecm_cfg.h>
#include <stdint.h>
#include <usb_componets.h>
#include <ux_api.h>
#include <ux_device_stack.h>
#include <ux_device_class_cdc_ecm.h>

#define USB_CDC_ECM_XMIT_QUEUE_ROOM_SIZE_DEFAULT          32

struct usb_ecm {
    UX_SLAVE_CLASS_CDC_ECM *cdc_ecm;
    int status;
    u32 media_state;
};
struct usb_ecm gx_ecm;


void usb_cdc_ecm_single_connect(void)
{
    if (!gx_ecm.cdc_ecm)
        return;

    gx_ecm.media_state = UX_DEVICE_CLASS_CDC_ECM_MEDIA_CONNECTED;
    _ux_device_class_cdc_ecm_connect(gx_ecm.cdc_ecm, UX_DEVICE_CLASS_CDC_ECM_LINK_STATE_UP);
}

void usb_cdc_ecm_single_disconnect(void)
{
    if (!gx_ecm.cdc_ecm)
        return;

    gx_ecm.media_state = UX_DEVICE_CLASS_CDC_ECM_MEDIA_DISCONNECTED;
    _ux_device_class_cdc_ecm_connect(gx_ecm.cdc_ecm, UX_DEVICE_CLASS_CDC_ECM_LINK_STATE_DOWN);
    _ux_utility_event_flags_set(&gx_ecm.cdc_ecm->ux_slave_class_cdc_ecm_event_flags_group,
        UX_DEVICE_CLASS_CDC_ECM_NEW_DEVICE_STATE_CHANGE_EVENT, UX_OR);
}

u32 usb_cdc_ecm_single_get_state(void)
{
    return gx_ecm.media_state;
}

void usb_cdc_ecm_instance_activate(void *data)
{
    gx_ecm.cdc_ecm = (UX_SLAVE_CLASS_CDC_ECM *)data;
}

void usb_cdc_ecm_instance_deactivate(void *data)
{
    gx_ecm.cdc_ecm = NULL;
}

int usb_cdc_ecm_update_xmit_queue_room_size(u32 size)
{
    if (!gx_ecm.cdc_ecm)
        return AIC_EIO;

    gx_ecm.cdc_ecm->ux_slave_class_cdc_ecm_xmit_queue_room_size = size;
    return AIC_SUCCESS;
}

int usb_cdc_ecm_uninit(void)
{
    uint32_t status;

    if (!gx_ecm.status)
        return AIC_SUCCESS;

    status = _ux_device_stack_class_unregister(
                    _ux_system_slave_class_cdc_ecm_name,
                    ux_device_class_cdc_ecm_entry);
    if (status != UX_SUCCESS) {
        aicx_error(0x0b7dab, "d", "cdc_ecm class unregister. status = %d\n", status);
        return AIC_EIO;
    }

    return AIC_SUCCESS;
}

int usb_cdc_ecm_init(void)
{
    UX_SLAVE_CLASS_CDC_ECM_PARAMETER         parameter;
    uint32_t status;

    /* Set the parameters for callback when insertion/extraction of a CDC device. Set to NULL.*/
    parameter.ux_slave_class_cdc_ecm_instance_activate = usb_cdc_ecm_instance_activate;
    parameter.ux_slave_class_cdc_ecm_instance_deactivate = usb_cdc_ecm_instance_deactivate;
    /* Set extra parameters used by the RNDIS query command with certain OIDs. */
    parameter.ux_slave_class_cdc_ecm_parameter_vendor_id = VENDOR_ID ;
    parameter.ux_slave_class_cdc_ecm_parameter_driver_version = PRODUCT_ID;
    parameter.ux_slave_class_cdc_ecm_parameter_xmit_queue_room_size = USB_CDC_ECM_XMIT_QUEUE_ROOM_SIZE_DEFAULT;
    /* Define a local NODE ID. */
    ux_utility_memory_copy(
        parameter.ux_slave_class_cdc_ecm_parameter_local_node_id,
        ecm_local_node_id, 6);
    /* Define a remote NODE ID. */
    ux_utility_memory_copy(
        parameter.ux_slave_class_cdc_ecm_parameter_remote_node_id,
        ecm_remote_node_id, 6);
    ux_utility_memory_copy(
        parameter.ux_slave_class_cdc_ecm_parameter_vendor_description,
        ECM_VENDOR_DESC, 11);
    /* Initialize the device rndis class. This class owns both interfaces. */
    status = ux_device_stack_class_register(
            _ux_system_slave_class_cdc_ecm_name,
            ux_device_class_cdc_ecm_entry, 1,0, &parameter);
    if (status)
        aicx_error(0x176163, "x", "cdc-ecm ux_device_stack_class_register ret =0x%x.\n", status);

    gx_ecm.status = 1;

    return status;
}

struct usb_class_instance ecm_instance = {
    .init = usb_cdc_ecm_init,
    .uninit =usb_cdc_ecm_uninit,
    .status = 1,
    .name = "usb cdc ecm",
};

int usb_cdc_ecm_register(void)
{
    usb_service_register_instance(ECM, ecm_instance);
    return AIC_SUCCESS;
}
DIS_BASIC_COMP_P1_INIT_EXPORT(usb_cdc_ecm_register);
