/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18caec
#define LOG_MODULE    AMOD_USB

#include <aic_osa.h>
#include <aic_io.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_at_process_common.h>
#include <stdint.h>
#include <slp_inc.h>
#include <ux_api.h>
#include <usb_service.h>
#include <usb_phy.h>
#include <dvfs_inc.h>

#define USB_SERVICE_STACK_SIZE 2048
#define USB_SERVICE_PRI        10
#define USB_SERVICE_QUEUE_CNT  10
#define USB_SERVICE_ENUM_TIMEOUT    16000
struct usb_service uservice;

void usb_service_virtual_plugin(void)
{
    u32 uevent = USB_PLUG_IN;

    aicx_info(0x0958b2, "", "virtual usb plugin\n");
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}

void usb_service_virtual_plugout(void)
{
    u32 uevent = USB_PLUG_OUT;
    aicx_info(0x0ed0ae, "", "virtual usb plugout\n");
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}

u32 usb_service_open_query_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    char buffer[128] = {'\0'};
    ulong rsp = (ulong)buffer;
    u32 *xcfgi;

    xcfgi = usbphy_cfg_get_xcfgi();
    usbphy_cfg_xcfgi_read();
    snprintf(buffer, sizeof(buffer), "xcfgi: bit0~bit70: \n 0x%08x, 0x%08x, 0x%08x",
             xcfgi[1], xcfgi[2], xcfgi[3]);

    aic_fill_transmit_cmd_resp(at_req_res_ptr, &rsp, 1);

    return AIC_SUCCESS;
}

u32 usb_service_open_set_cmd(AT_REQ_RESP *at_req_res_ptr)
{
    ulong *paras_arr, rsp;
    u16 count, i;
    u32 *xcfgi;

    xcfgi = usbphy_cfg_get_xcfgi();
    rsp = (ulong)"OK";
    paras_arr = aic_split_paras_to_str_array(at_req_res_ptr->at_req,
                                             "AT^USBXCFGI=", &count);
    if (!paras_arr || count != 3) {
        rsp = (ulong)"ERROR";
        goto AT_REPLY;
    }

    for (i = 0; i < 3; i++ )
         xcfgi[i + 1] = strtoul((char *)paras_arr[i], NULL, 16);
    usbphy_cfg_efuse_extract();
    usbphy_cfg_xcfgi_write();
AT_REPLY:
    aic_free_paras_mem(paras_arr, count);
    return aic_fill_transmit_cmd_resp(at_req_res_ptr, &rsp, 1);
}

static AT_CMD_HANDLE uservice_at_handles[] = {
    {"AT^USBXCFGI", AIC_NULL, usb_service_open_query_cmd,
                usb_service_open_set_cmd, AIC_NULL},
    {AIC_NULL},
};

void usb_service_register_instance(UEC class,
                            struct usb_class_instance instance)
{
    uservice.instance[class].init = instance.init;
    uservice.instance[class].uninit = instance.uninit;
    uservice.instance[class].status = instance.status;
    uservice.instance[class].name = instance.name;
}

int usb_service_scene_vaild_check(UEC combo)
{
    if (uservice.instance[combo].status)
        return AIC_SUCCESS;

    return AIC_ENXIO;
}

static int usb_service_class_register_init(UEC combo)
{
    int ret = 0;

    ret = uservice.instance[combo].init();
    if (ret)
        return ret;

    if (combo == LOG_AT_DIAG)
        return AIC_SUCCESS;

    if (!uservice.instance[LOG_AT_DIAG].status)
       return AIC_SUCCESS;

    ret = uservice.instance[LOG_AT_DIAG].init();
    if (ret)
        return ret;

    return AIC_SUCCESS;
}

static int usb_service_class_register_uninit(UEC combo)
{
    int ret = 0;

    ret = uservice.instance[combo].uninit();
    if (ret)
        return ret;

    if (combo == LOG_AT_DIAG)
        return AIC_SUCCESS;

    if (!uservice.instance[LOG_AT_DIAG].status)
       return AIC_SUCCESS;

    ret = uservice.instance[LOG_AT_DIAG].uninit();
    if (ret)
        return ret;

    return AIC_SUCCESS;
}

int usb_service_enum_start(UEC combo)
{
    int ret;

    if (combo == UNKNOW_FUNC)
        return AIC_SUCCESS;
    usbx_descriptor_build(combo);
    ret = usbx_device_stack_init();
    if (ret)
        return ret;
    ret = usb_service_class_register_init(combo);
    if (ret)
       usbx_uninit();

    return ret;
}

int usb_service_enum_clear(UEC combo)
{
    int ret;

    if (uservice.usb_combo == UNKNOW_FUNC)
        return AIC_SUCCESS;

    usb_driver_get_mutex();
    ret = ux_device_stack_disconnect();
    if (ret) {
        aicx_error(0x138f8d, "d", "ux_device_stack_disconnect = %d\n", ret);
        usb_driver_put_mutex();
        return ret;
    }
    usb_driver_put_mutex();

    ret = usb_service_class_register_uninit(uservice.usb_combo);
    if (ret) {
        aicx_error(0x03b830, "d", "usb_service_class_register_uninit = %d\n", ret);
        return ret;
    }
    ret = usbx_uninit();

    return ret;
}

/**
 * usb_service_enum_manage - USB enum according to combo parameter.
 *
 * Parameter:
 * @combo: USB enum mode.
 *
 * Return:
 *  return USB enum result.
 */
int usb_service_enum_manage(UEC combo)
{
    int ret;

    if (combo >= COMBO_MAX)
       return AIC_EIO;

    ret = usb_service_scene_vaild_check(combo);
    if (ret)
       return ret;

    if (combo == uservice.usb_combo)
        return AIC_SUCCESS;

    ret = aos_get_mutex(uservice.mutex, AOS_WAIT_FOREVER);
    if (ret)
        return AIC_EINTR;

    if (!uservice.connect) {
        aos_put_mutex(uservice.mutex);
        return AIC_EIO;
    }

    usb_disconnect();
    ret = usb_service_enum_clear(combo);
    if (ret) {
        aos_put_mutex(uservice.mutex);
        return ret;
    }

    ret = usb_service_enum_start(combo);
    if (ret) {
        aos_put_mutex(uservice.mutex);
        return ret;
    }

    if (uservice.usb_combo == UNKNOW_FUNC)
        usb_plugin();
    else
        usb_connect();
    uservice.usb_combo = combo;
    aos_put_mutex(uservice.mutex);

    return AIC_SUCCESS;
}

/**
 * usb_service_get_enum_mode - Get current enum mode.
 *
 * Return:
 *  return current enum mode.
 *  UNKNOW_FUNC,
 *  LOG_AT_DIAG,
 *  RNDIS,
 *  ECM,
 *  MSC,
 */
enum USB_ENUM_COMBO usb_service_get_enum_mode(void)
{
    return uservice.usb_combo;
}

/**
 * usb_service_list_support_func - List usb support function.
 *
 * Return:
 *  return usb support function bitmap.
 *  BIT0:LOG_AT_DIAG, BIT1:RNDIS, BIT2:ECM, BIT3:MSC,
 */
u32 usb_service_list_support_func(void)
{
    u32 func, func_map = 0;

    for (func = LOG_AT_DIAG; func < COMBO_MAX; func++)
        if (!usb_service_scene_vaild_check(func))
            func_map |= (1 << func);

    return func_map;
}

int usb_service_plug_event_listen_register(USB_LISTEN_HANDLER handler)
{
    if (uservice.listener_index >=LISTENER_MAX_NUM)
        return AIC_ENOMEM;
    uservice.listener[uservice.listener_index] = handler;
    uservice.listener_index++;
    return AIC_SUCCESS;
}

void usb_service_hotplug_event_report(u32 event)
{
    int i;

    AIC_ASSERT(uservice.listener_index <= LISTENER_MAX_NUM);
    for (i = 0; i < uservice.listener_index; i++)
        if (uservice.listener[i])
            uservice.listener[i](event);
}

void usb_service_network_status_report(u32 event)
{
    if (uservice.network_listener)
        uservice.network_listener(event);
}

int usb_service_network_status_listen_register(USB_LISTEN_HANDLER handler)
{
    uservice.network_listener = handler;
    return AIC_SUCCESS;
}

BOOL usb_get_connect_status(void)
{
    return uservice.connect;
}

static void usb_service_enum_timeout(void *param)
{
    u32 uevent = USB_PLUG_OUT;;

    aicx_info(0x0e2885, "", "usb enum timeout, change status to plugout\n");
    uservice.timeout_record = 1;
    aos_send_to_queue(uservice.queue, &uevent, TX_NO_WAIT);
}

void usb_service_enum_timer_active(void)
{
    uservice.timeout_record = 0;
    aos_change_timer(uservice.enum_timer, USB_SERVICE_ENUM_TIMEOUT, AOS_NO_PERIOD);
}

void usb_service_enum_timer_deactive(void)
{
    u32 active_state, remaining_time, reschedule;

    aos_get_timer_info(uservice.enum_timer, AIC_NULL, &active_state,
                       &remaining_time, &reschedule);

    if (active_state)
        aos_deactivate_timer(uservice.enum_timer);
}

void usb_service_hotplug_event(u32 event)
{
    int ret = AIC_SUCCESS;

    switch (event) {
    case USB_PLUG_IN:
        usbphy_ref_clock_swicth();
        if (uservice.connect)
            break;
        aic_get_wakelock();
        aic_dvfs_force(RISE | AP_CMSTAR);
        usbphy_cfg_xcfgi_write();
        usb_clk_enable();
        ret = usb_service_enum_start(uservice.usb_combo);
        if ((uservice.usb_combo != UNKNOW_FUNC) && uservice.usb_combo && ret) {
            uservice.usb_combo = LOG_AT_DIAG;
            ret = usb_service_enum_start(uservice.usb_combo);
        }

        uservice.connect = TRUE;
        if (uservice.usb_combo != UNKNOW_FUNC)
            usb_plugin();
        usb_service_hotplug_event_report(USB_PLUG_IN);
        break;
    case USB_PLUG_OUT :
        if (!uservice.connect)
            break;
        usb_plugout();
        uservice.connect = FALSE;
        ret = usb_service_enum_clear(uservice.usb_combo);
        usb_clk_disable();
        uservice.usb_combo = usbx_get_combo_default();
        usbphy_cfg_xcfgi_read();
        aic_dvfs_force(FALL | AP_CMSTAR);
        aic_put_wakelock();
        usb_service_hotplug_event_report(USB_PLUG_OUT);
        break;
    case USB_PLUG_DEFAULT_IN:
        aic_get_wakelock();
        usb_clk_enable();
        usb_plugout();
        usb_plugin();
        uservice.connect = TRUE;
        usb_service_hotplug_event_report(USB_PLUG_DEFAULT_IN);
        break;
    case USB_PLUG_DEFAULT_OUT:
        usb_clk_enable();
        uservice.connect = FALSE;
        usb_plugout();
        usb_clk_disable();
        aic_put_wakelock();
        usb_service_hotplug_event_report(USB_PLUG_DEFAULT_OUT);
        break;
    default :
        aicx_error(0x1d57c4, "", "usb_service invaild event!!\n");
    }
    AIC_ASSERT(!ret);
}

void usb_service_thread(void *data)
{
    int ret = AIC_SUCCESS;
    u32 uevent;

    if (usb_direct_enum()) {
        usb_service_hotplug_event(USB_PLUG_IN);
        return;
    }

    aic_add_at_cmd_handle(uservice_at_handles);
    aos_queue_ptr queue = (aos_queue_ptr)data;
    while(1) {
        ret = aos_recv_from_queue(queue, &uevent, AOS_WAIT_FOREVER);
        if (ret)
            continue;
        ret = aos_get_mutex(uservice.mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(!ret);
        usb_service_hotplug_event(uevent);
        aos_put_mutex(uservice.mutex);
    }
}

int usb_service_init(void)
{
    uservice.listener_index = 0;
    uservice.connect = FALSE;
    uservice.usb_combo = usbx_get_combo_default();

    uservice.mutex = aos_create_mutex("usb_service");
    AIC_ASSERT(uservice.mutex);

    uservice.enum_timer = aos_create_timer("usb_enum",
                                         usb_service_enum_timeout, NULL,
                                         USB_SERVICE_ENUM_TIMEOUT,
                                         AOS_NO_ACTIVATE, AOS_NO_PERIOD);
    AIC_ASSERT(uservice.enum_timer);

    uservice.queue = aos_create_queue("usb_service queue",
                                   sizeof(u32),
                                   USB_SERVICE_QUEUE_CNT);

    uservice.id = aos_create_thread("usb_service",
                                  usb_service_thread,
                                  uservice.queue, USB_SERVICE_STACK_SIZE,
                                  0, NULL, USB_SERVICE_PRI,
                                  AOS_AUTO_START);
    return AIC_SUCCESS;
}

DIS_BASIC_COMP_P1_INIT_EXPORT(usb_service_init);
