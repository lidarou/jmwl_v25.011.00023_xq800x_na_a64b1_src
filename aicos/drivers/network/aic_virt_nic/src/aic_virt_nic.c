/**************************************************************************/
/*                                                                        */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** Network Virtual NIC                                                   */
/**                                                                       */
/**   Network Virtual NIC Driver for NETX 5.3 and above.                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x068483
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <cmux.h>
#include <cmux_cfg.h>
#include "nx_api.h"
#include "nx_packet.h"
#include <aic_size.h>
#include "aic_net_api.h"
#include "aic_packet_management.h"

#include "aic_virt_nic.h"

struct aic_virt_network_driver aic_virt_dev;

int aic_virt_nic_send(NX_PACKET *packet_ptr)
{
    size_t size;
    struct aic_virt_network_driver *dev = &aic_virt_dev;

    aicx_info(0x0bfada, "d", "[VIRT_NIC] send net packet len %d\n",
              packet_ptr->nx_packet_length);
    size = aic_cmux_write(dev->cmux_handle, packet_ptr->nx_packet_prepend_ptr,
                          packet_ptr->nx_packet_length, 1000);
    if (size != packet_ptr->nx_packet_length)
        aicx_warn(0x08c371, "d", "[VIRT_NIC] cmux write fail %d\n", size);

    nx_packet_release(packet_ptr);

    return AIC_SUCCESS;
}

static int aic_virt_nic_recv(void *p)
{
    int ret;
    size_t read_size;
    static u8 data[1500];
    NX_PACKET *packet_ptr;
    NX_IP_DRIVER driver_request;
    NX_INTERFACE *interface_ptr;
    struct aic_network_ip_instance *aic_ip;
    struct aic_virt_network_driver *dev = p;

    read_size = aic_cmux_read(dev->cmux_handle, data, 1500, AOS_WAIT_FOREVER);
    if (read_size < 20)
        return AIC_EINVAL;
    aicx_info(0x10b8d8, "d", "[FORWARD] recv %d from cmux\n", read_size);

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_warn(0x0472c2, "", "[VIRT_NIC] get ip instance fail\n");
        return AIC_ENODEV;
    }

    interface_ptr = aic_ip->ip_ptr->nx_ip_gateway_interface;
    if (!interface_ptr) {
        aicx_warn(0x0bc61e, "", "[VIRT_NIC] interface not ready\n");
        return AIC_ENODEV;
    }

    ret = aic_network_alloc_usb_packet(&packet_ptr, read_size);
    if (ret != AIC_SUCCESS) {
        aicx_warn(0x18e93c, "x", "[VIRT_NIC] alloc packet fail 0x%x\n", ret);
        return AIC_ENOMEM;
    }

    ret = nx_packet_data_append(packet_ptr, data, read_size,
                                aic_ip->ip_ptr->nx_ip_default_packet_pool,
                                AOS_WAIT_FOREVER);
    if (ret != AIC_SUCCESS) {
        aicx_warn(0x161279, "x", "[VIRT_NIC] append packet fail 0x%x\n", ret);
        nx_packet_release(packet_ptr);
        return AIC_ENOMEM;
    }

    driver_request.nx_ip_driver_ptr = aic_ip->ip_ptr;
    driver_request.nx_ip_driver_packet = packet_ptr;
    driver_request.nx_ip_driver_interface = aic_ip->ip_ptr->nx_ip_gateway_interface;
    driver_request.nx_ip_driver_return_ptr = NULL;
    driver_request.nx_ip_driver_command = NX_LINK_PACKET_SEND;

    if (!driver_request.nx_ip_driver_interface->nx_interface_link_up) {
        nx_packet_release(packet_ptr);
        return AIC_EACCES;
    }

    packet_ptr->nx_packet_ip_interface = driver_request.nx_ip_driver_interface;
    (driver_request.nx_ip_driver_interface->nx_interface_link_driver_entry)(&driver_request);
    if (driver_request.nx_ip_driver_status) {
        nx_packet_release(packet_ptr);
        return driver_request.nx_ip_driver_status;
    }

    return AIC_SUCCESS;
}

static void aic_virt_recv_task(void *p)
{
    while (1) {
        aic_virt_nic_recv(p);
    }
}

int aic_virt_nic_init(void)
{
    int status= AIC_SUCCESS;

    memset(&aic_virt_dev, 0, sizeof(aic_virt_dev));

    aic_virt_dev.recv_thread_id =
        aos_create_thread("net-virt-recv", aic_virt_recv_task,
                          (void *)&aic_virt_dev, SIZE_2K, 0, NULL,
                          VIRT_NIC_TASK_PRIORITY,
                          AOS_AUTO_START);
    if (aic_virt_dev.recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1aca12, "", "[driver] create virt recv task fail\n");
        return AIC_EPERM;
    }

    aic_virt_dev.cmux_handle = aic_cmux_open(AIC_CMUX_DATA_COM);

    return status;
}

AIC_APP_P2_INIT_EXPORT(aic_virt_nic_init);

