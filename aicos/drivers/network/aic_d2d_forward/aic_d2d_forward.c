/**************************************************************************/
/*                                                                        */
/*    Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** Network D2D Forward                                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1992be
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
#ifdef CONFIG_D2D_APP
#include <audio_d2d_interface.h>
#endif

#define AIC_D2D_SEND_EVENT    0x1

#define D2D_MAX_CMUX_CHANNEL  8
#define D2D_NIC_SEND_TASK_PRIORITY 8
#define D2D_NIC_RECV_TASK_PRIORITY 8

#define D2D_HEAP_LOW_WATER    20000

#define D2D_CMUX_DATA_CHAN0   AIC_CMUX_VCOM8
#define D2D_CMUX_DATA_CHAN1   AIC_CMUX_VCOM9
#define D2D_CMUX_DATA_CHAN2   AIC_CMUX_VCOM10
#define D2D_CMUX_DATA_CHAN3   AIC_CMUX_VCOM11
#define D2D_CMUX_DATA_CHAN4   AIC_CMUX_VCOM12
#define D2D_CMUX_DATA_CHAN5   AIC_CMUX_VCOM13
#define D2D_CMUX_DATA_CHAN6   AIC_CMUX_VCOM14
#define D2D_CMUX_DATA_CHAN7   AIC_CMUX_VCOM15

#define D2D_CMUX_CHAN0_EVENT  0x1
#define D2D_CMUX_CHAN1_EVENT  0x2
#define D2D_CMUX_CHAN2_EVENT  0x4
#define D2D_CMUX_CHAN3_EVENT  0x8
#define D2D_CMUX_CHAN4_EVENT  0x10
#define D2D_CMUX_CHAN5_EVENT  0x20
#define D2D_CMUX_CHAN6_EVENT  0x40
#define D2D_CMUX_CHAN7_EVENT  0x80
#define D2D_CMUX_ALL_EVENT    0xFF

#define D2D_BIG_BW_MTU        868
#define D2D_LITTLE_BW_MTU     155

struct aic_d2d_network_driver
{
    u32 local_audio;
    u32 loopback;
    void *cmux_handle[D2D_MAX_CMUX_CHANNEL];
    u32 interface_id[8];
    aos_event_ptr d2d_event;
    aos_thread_id recv_thread_id;
    aos_thread_id send_thread_id;

    NX_PACKET *packet_queue_head;
    NX_PACKET *packet_queue_tail;
    u32 packet_queue_cnt;

    u32 ul_err_cnt;
    u32 dl_err_cnt;

    u32 mtu;
};

struct aic_d2d_network_driver aic_d2d_dev;

extern void aic_d2d_data_statistics(u32 ul_bytes, u32 dl_bytes);

static int aic_d2d_get_cmux_channel(u8 link_id)
{
    switch (link_id) {
    case 0:
        return D2D_CMUX_DATA_CHAN0;
    case 2:
        return D2D_CMUX_DATA_CHAN1;
    case 4:
        return D2D_CMUX_DATA_CHAN2;
    case 6:
        return D2D_CMUX_DATA_CHAN3;
    case 8:
        return D2D_CMUX_DATA_CHAN4;
    case 10:
        return D2D_CMUX_DATA_CHAN5;
    case 12:
        return D2D_CMUX_DATA_CHAN6;
    case 14:
        return D2D_CMUX_DATA_CHAN7;
    default:
        return -1;
    }
}
#if 0
static int aic_d2d_get_link_id(u8 chan)
{
    switch (chan) {
    case D2D_CMUX_DATA_CHAN0:
        return 0;
    case D2D_CMUX_DATA_CHAN1:
        return 2;
    case D2D_CMUX_DATA_CHAN2:
        return 4;
    case D2D_CMUX_DATA_CHAN3:
        return 6;
    case D2D_CMUX_DATA_CHAN4:
        return 8;
    case D2D_CMUX_DATA_CHAN5:
        return 10;
    case D2D_CMUX_DATA_CHAN6:
        return 12;
    case D2D_CMUX_DATA_CHAN7:
        return 14;
    default:
        return -1;
    }
}
#endif
int aic_d2d_nic_send(NX_PACKET *packet_ptr, u8 link_id)
{
    char tmp_str[8];
    size_t size;
    int chan;
    struct aic_d2d_network_driver *dev = &aic_d2d_dev;

    chan = aic_d2d_get_cmux_channel(link_id);
    if (chan < 0)
        return -1;

    memset(tmp_str, 0, 8);
    memcpy(tmp_str, packet_ptr->nx_packet_prepend_ptr, 6);
    if (!strcmp("RANDOM", tmp_str) || !strcmp("AICMSG", tmp_str))
        return -1;

    aic_d2d_data_statistics(0, packet_ptr->nx_packet_length);
    aicx_info(0x08e346, "ddd", "[D2D_NIC] link_id %d chan %d send packet len %d\n",
              link_id, chan, packet_ptr->nx_packet_length);

    if (dev->local_audio && (link_id == 0)) {
#ifdef CONFIG_D2D_APP
        size = d2d_write_to_audio(NULL, packet_ptr->nx_packet_prepend_ptr, packet_ptr->nx_packet_length);
        aicx_info(0x06f4e5, "d", "[D2D_NIC] send to audio %d\n", size);
#else
        AIC_ASSERT(FALSE);
#endif
    } else {
        size = aic_cmux_write(dev->cmux_handle[chan - D2D_CMUX_DATA_CHAN0], packet_ptr->nx_packet_prepend_ptr,
                              packet_ptr->nx_packet_length, 1000);
        if (size != packet_ptr->nx_packet_length) {
            aicx_warn(0x028108, "dd", "[D2D_NIC] cmux %d write fail %d\n",
                      aic_d2d_get_cmux_channel(link_id), size);
            dev->dl_err_cnt++;
        }
    }

    nx_packet_release(packet_ptr);

    return AIC_SUCCESS;
}

static void aic_d2d_send_task(void *p)
{
    struct aic_d2d_network_driver *dev = p;
    AOS_INTERRUPT_DECLARE
    NX_PACKET *packet_ptr;
    u32 actual_flags;
    u32 send_size;
    int ret;

    while (1) {
        aos_get_event(dev->d2d_event, AIC_D2D_SEND_EVENT, AOS_OR_CLEAR,
                      &actual_flags, AOS_WAIT_FOREVER);

        while (dev->packet_queue_head) {
            AOS_INTERRUPT_DISABLE
            packet_ptr = dev->packet_queue_head;
            if (!packet_ptr->nx_packet_queue_next) {
                dev->packet_queue_head = NULL;
                dev->packet_queue_tail = NULL;
            } else {
                dev->packet_queue_head = packet_ptr->nx_packet_queue_next;
            }

            dev->packet_queue_cnt--;
            AOS_INTERRUPT_RESTORE

            send_size = packet_ptr->nx_packet_length;
            ret = aic_lte_nic_send(packet_ptr);
            if (!ret)
                aic_d2d_data_statistics(send_size, 0);
            else
                dev->ul_err_cnt++;
        }
    }
}

static void aic_d2d_recv_task(void *p)
{
    AOS_INTERRUPT_DECLARE
    aos_queue_ptr cmux_queue;
    int ret, chan, i;
    size_t read_size, write_size, offset = 0, send_size = 0;
    u32 queue;
    static u8 data[4096];
    NX_PACKET *packet_ptr;
    NX_INTERFACE *interface_ptr;
    struct aic_d2d_network_driver *dev = p;
    struct aic_network_ip_instance *aic_ip = NULL;
    u32 total_heap_size = 0, used_heap_size = 0, free_heap_size = 0;

    while (1) {
        cmux_queue = cmux_conn_get_queue(CMUX_CONN_DATA);
        if (!cmux_queue) {
            aicx_warn(0x1898d4, "", "[D2D_NIC] get queue fail\n");
            continue;
        } else if ((u32)cmux_queue == 0xffffffff) {
            aicx_warn(0x18bec1, "", "[D2D_NIC] get queue fail -1\n");
            aos_sleep(0xFFFFFFFF);
        }

        ret = aos_recv_from_queue(cmux_queue, &queue, AOS_WAIT_FOREVER);
        if (ret != AOS_SUCCESS)
            continue;

        for (i = 0; i < D2D_MAX_CMUX_CHANNEL; i++) {
            if (!(BIT(i + D2D_CMUX_DATA_CHAN0) & queue))
                continue;

            chan = i;
            read_size = aic_cmux_read(dev->cmux_handle[chan], data, 4096,
                                      AOS_WAIT_FOREVER);
            if (read_size <= 0) {
                aicx_info(0x141c16, "d", "[D2D_NIC] chan %d is empty\n", chan);
                continue;
            }

            aicx_info(0x16e792, "ddd", "[D2D_NIC] recv %d from cmux%d interface id %d\n",
                      read_size, chan, dev->interface_id[chan]);
            if (dev->loopback) {
                write_size = aic_cmux_write(dev->cmux_handle[chan], data, read_size, 1000);
                aicx_info(0x0b3bc8, "dd", "[D2D_NIC] write %d to cmux%d\n", write_size, chan);
                continue;
            }

            if (!aic_ip) {
                aic_ip = aic_get_default_ip_instance();
                if (!aic_ip || !aic_ip->ip_ptr) {
                    aicx_warn(0x16d006, "", "[D2D_NIC] get ip instance fail\n");
                    continue;
                }
            }

            if (dev->interface_id[chan] >= 8) {
                aicx_warn(0x07bdb0, "d", "[D2D_NIC] chan %d interface not link up\n", chan);
                continue;
            }

            interface_ptr = &aic_ip->ip_ptr->nx_ip_interface[dev->interface_id[chan]];
            if (!interface_ptr->nx_interface_link_up) {
                aicx_warn(0x17a2e6, "", "[D2D_NIC] interface not ready\n");
                continue;
            }

            offset = 0;
            while (read_size) {
                if (read_size > dev->mtu) {
                    send_size = dev->mtu;
                } else {
                    send_size = read_size;
                }

                ret = aos_get_heap_usage(&total_heap_size, &used_heap_size);
                if (ret) {
                    total_heap_size = 0;
                    used_heap_size = 0;
                    free_heap_size = 0;
                } else {
                    free_heap_size = total_heap_size - used_heap_size;
                }

                if (free_heap_size && free_heap_size < D2D_HEAP_LOW_WATER) {
                    aicx_warn(0x0bcc50, "ddd", "[D2D_NIC] free heap is low water %d total %d used %d\n",
                              free_heap_size, total_heap_size, used_heap_size);
                    dev->ul_err_cnt++;
                    offset += send_size;
                    read_size -= send_size;
                    continue;
                }

                ret = aic_network_alloc_usb_packet(&packet_ptr, send_size);
                if (ret != AIC_SUCCESS) {
                    aicx_warn(0x19507e, "x", "[D2D_NIC] alloc packet fail 0x%x\n", ret);
                    dev->ul_err_cnt++;
                    offset += send_size;
                    read_size -= send_size;
                    continue;
                }

                ret = nx_packet_data_append(packet_ptr, &data[offset], send_size,
                                            aic_ip->ip_ptr->nx_ip_default_packet_pool,
                                            AOS_WAIT_FOREVER);
                if (ret != AIC_SUCCESS) {
                    aicx_warn(0x0affb1, "x", "[D2D_NIC] append packet fail 0x%x\n", ret);
                    dev->ul_err_cnt++;
                    nx_packet_release(packet_ptr);
                    offset += send_size;
                    read_size -= send_size;
                    continue;
                }

                packet_ptr->nx_packet_ip_interface = interface_ptr;
                AOS_INTERRUPT_DISABLE
                if (!dev->packet_queue_head) {
                    dev->packet_queue_head = packet_ptr;
                    dev->packet_queue_tail = packet_ptr;
                } else {
                    dev->packet_queue_tail->nx_packet_queue_next = packet_ptr;
                    dev->packet_queue_tail = packet_ptr;
                }

                dev->packet_queue_cnt++;
                AOS_INTERRUPT_RESTORE

                aos_set_event(dev->d2d_event, AIC_D2D_SEND_EVENT, AOS_OR);
                offset += send_size;
                read_size -= send_size;
            }
        }
    }
}

void aic_d2d_open_cmux(u8 link_id, char *interface_name)
{
    int chan, i;
    struct aic_network_ip_instance *aic_ip = NULL;

    aic_ip = aic_get_default_ip_instance();
    if (!aic_ip || !aic_ip->ip_ptr) {
        aicx_warn(0x16d006, "", "[D2D_NIC] get ip instance fail\n");
        return;
    }

    chan = aic_d2d_get_cmux_channel(link_id);

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (!aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_name)
            continue;

        if (strcmp(interface_name, aic_ip->ip_ptr->nx_ip_interface[i].nx_interface_name) == 0) {
            break;
        }
    }

    aicx_info(0x0e5c5c, "sd", "[D2D_NIC] interface name %s index %d\n", interface_name, i);
    if (i >= NX_MAX_PHYSICAL_INTERFACES)
        app_logi("[D2D_NIC] find interface index fail\n");
    else
        aic_d2d_dev.interface_id[link_id >> 1] = i;
}

void aic_d2d_close_cmux(u8 link_id)
{
    aic_d2d_dev.interface_id[link_id >> 1] = -1;
}

int aic_d2d_get_bw_mode()
{
    if (aic_d2d_dev.mtu == D2D_BIG_BW_MTU)
        return 0;
    else if (aic_d2d_dev.mtu == D2D_LITTLE_BW_MTU)
        return 1;
    else
        return -1;
}

void aic_d2d_set_bw_mode(u8 mode)
{
    aicx_info(0x10d283, "d", "[D2D_NIC] update bw mode %d\n", mode);

    if (!mode)
        aic_d2d_dev.mtu = D2D_BIG_BW_MTU;
    else if (mode == 1)
        aic_d2d_dev.mtu = D2D_LITTLE_BW_MTU;
}

void aic_d2d_setup_local_audio_mode()
{
    aic_d2d_dev.local_audio = TRUE;
}

int aic_d2d_nic_init(void)
{
    u32 chan, i;
    int status= AIC_SUCCESS;

    memset(&aic_d2d_dev, 0, sizeof(aic_d2d_dev));

    aic_d2d_dev.recv_thread_id =
        aos_create_thread("d2d-recv", aic_d2d_recv_task,
                          (void *)&aic_d2d_dev, SIZE_2K, 0, NULL,
                          D2D_NIC_RECV_TASK_PRIORITY,
                          AOS_AUTO_START);
    if (aic_d2d_dev.recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x002902, "", "[driver] create d2d recv task fail\n");
        return AIC_EPERM;
    }

    aic_d2d_dev.send_thread_id =
        aos_create_thread("d2d-send", aic_d2d_send_task,
                          (void *)&aic_d2d_dev, SIZE_2K, 0, NULL,
                          D2D_NIC_SEND_TASK_PRIORITY,
                          AOS_AUTO_START);
    if (aic_d2d_dev.send_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1d3d30, "", "[driver] create d2d send task fail\n");
        return AIC_EPERM;
    }

    aic_d2d_dev.d2d_event = aos_create_event("d2d-event");
    if (!aic_d2d_dev.d2d_event) {
        aicx_error(0x07fed1, "", "[driver] create d2d event fail\n");
        aos_delete_thread(aic_d2d_dev.recv_thread_id);
        aos_delete_thread(aic_d2d_dev.send_thread_id);
        return AIC_EPERM;
    }

    chan = D2D_CMUX_DATA_CHAN0;
    for (i = 0; i < D2D_MAX_CMUX_CHANNEL; i++) {
        aic_d2d_dev.cmux_handle[i] = cmux_conn_port_id_handle(chan + i);
    }

    for (i = 0; i < D2D_MAX_CMUX_CHANNEL; i++) {
        aic_d2d_dev.interface_id[i] = -1;
    }

    aic_d2d_dev.loopback = FALSE;
    aic_d2d_dev.packet_queue_cnt = 0;

    aic_d2d_dev.mtu = D2D_BIG_BW_MTU;
    aicx_info(0x0756a1, "", "[D2D_NIC] init ok\n");

    return status;
}

AIC_APP_P2_INIT_EXPORT(aic_d2d_nic_init);

