/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NETXDUO Component                                                     */
/**                                                                       */
/**   ALTE Network Driver for NETX 5.3 and above                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_alte_driver.h                                 PORTABLE C         */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Qingsheng Li,    AICXTEK TECHNOLOGIES CO.,LTD                       */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    ALTE Network driver.                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  11-30-2021     Qingsheng Li           Initial Version 1.0             */
/*                                                                        */
/**************************************************************************/

#ifndef __NX_ALTE_DRIVER_H__
#define __NX_ALTE_DRIVER_H__

/* Determine if a C++ compiler is being used. If so, ensure that standard
   C is used to process the API information. */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C. */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_ip.h"
#include "aic_osa.h"
#include "aipc_api.h"

#define ALTE_NORMAL_PACKET_THRESHOLD 4

#define ALTE_TASK_PRIORITY        9
#ifdef CONFIG_LOW_THROUGHPUT
#define ALTE_RECV_TASK_PRIORITY   5
#else
#define ALTE_RECV_TASK_PRIORITY   9
#endif
#define ALTE_NIC_MAX_INSTANCES    8

#define ALTE_NIC_MAC_HEADER_SIZE  14
#define ALTE_NX_ETHERNET_SIZE     14
#define ALTE_NX_ETHERNET_IP       0x0800
#define ALTE_NX_ETHERNET_IPV6     0x08DD
#define ALTE_NX_ETHERNET_MTU      1514
#define ALTE_BLOCK_OFFSET         24
#define ALTE_BLOCK_TAIL_OFFSET    3

#define ALTE_PACKET_OFFSET        16

#define ALTE_SEND_EVENT             0x01
#define ALTE_SEND_LINK_UP_EVENT     0x02
#define ALTE_PING_EVENT             0x04
#define ALTE_FAST_REBUILD_EVENT     0x08
#define ALTE_NETWORK_ACCESS_EVENT   0x10
#define ALTE_NETWORK_NOTIFY_RELEASE 0x20
#define ALTE_NETWORK_NOTIFY_FLIGHT  0x40
#define ALTE_NETWORK_CHECK_PDP      0x80
#define ALTE_NETWORK_SEND_ONLY      0x100

/* TC event flag */
#define ALTE_SEND_TRAFFIC_CONTROL 0x01
#define ALTE_RECV_TRAFFIC_CONTROL 0x02

#define ALTE_SEND_QUEUE_MAX       44
#define ALTE_SEND_QUEUE_HEAP_THRESHOLD 18

#define ALTE_MAX_TCP_SEND_CNT     50
#define ALTE_MAX_ICMP_SEND_CNT    2
#define ALTE_MAX_OTHERS_SEND_CNT  60

#ifndef CONFIG_LOW_THROUGHPUT
#ifndef ALTE_MAX_HIGH_QUEUE_SIZE
#define ALTE_MAX_HIGH_QUEUE_SIZE   120
#endif
#ifndef ALTE_MAX_NORMAL_QUEUE_SIZE
#define ALTE_MAX_NORMAL_QUEUE_SIZE 90
#endif
#else
#ifndef ALTE_MAX_HIGH_QUEUE_SIZE
#define ALTE_MAX_HIGH_QUEUE_SIZE   100
#endif
#ifndef ALTE_MAX_NORMAL_QUEUE_SIZE
#define ALTE_MAX_NORMAL_QUEUE_SIZE 60
#endif
#endif

#ifndef CONFIG_LOW_THROUGHPUT
#ifndef ALTE_LOW_HIGH_QUEUE_SIZE
#define ALTE_LOW_HIGH_QUEUE_SIZE   50
#endif
#ifndef ALTE_LOW_NORMAL_QUEUE_SIZE
#define ALTE_LOW_NORMAL_QUEUE_SIZE 30
#endif
#else
#ifndef ALTE_LOW_HIGH_QUEUE_SIZE
#define ALTE_LOW_HIGH_QUEUE_SIZE   50
#endif
#ifndef ALTE_LOW_NORMAL_QUEUE_SIZE
#define ALTE_LOW_NORMAL_QUEUE_SIZE 30
#endif
#endif

#define ALTE_MAX_TCP_FLIGHT_CNT     50
#ifndef ALTE_MAX_ICMP_FLIGHT_CNT
#define ALTE_MAX_ICMP_FLIGHT_CNT    10
#endif

#ifndef ALTE_FLIGHT_TIMEOUT
#define ALTE_FLIGHT_TIMEOUT         120000
#endif

#ifndef CONFIG_LOW_THROUGHPUT
#define ALTE_HEAP_LOW_WATER         40000
#else
#define ALTE_HEAP_LOW_WATER         10000
#endif

#define ALTE_NOTIFY_L2_REBUILD    1

#define ALTE_CONNECTIVITY_CHECK_TIMEOUT 1000
#define ALTE_CONNECTIVITY_CHECK_CNT     2

#define ALTE_NETWORK_ACCESS_MAX_CNT     3

#define ALTE_GET_PDP_TYPE_MAX_CNT       3

#define ALTE_TC_SPEED_TIMES       ((1000 / ((NX_IP_PERIODIC_RATE + (NX_IP_FAST_TIMER_RATE - 1)) / NX_IP_FAST_TIMER_RATE)) * 8)

#define ALTE_SEND_TIMEOUT         2000

#define ALTE_MAX_RECV_QUEUE_COUNT 26

#define ALTE_REAL_NETWORK_MODE    0
#define ALTE_LAB_NETWORK_MODE     1
#define ALTE_FLOWCTRL_MODE        2

#define ALTE_ENHANCE_PACKET_PRIORITY  100

#define AIC_NETWORK_ENHANCE_IDLE_TIME_STAGE  0
#define AIC_NETWORK_ENHANCE_RETRY_TIME_STAGE 1

#define AIC_NOTIFY_MODEM_DONGLE  3

enum alte_chan {
    ALTE_NORMAL,
    ALTE_HIGH,
};

/* mbuf */
typedef struct aos_mbuf_st
{
  struct aos_mbuf_st  *next_ptr;
  unsigned char       *data_ptr;
  unsigned short      data_offset;
  unsigned short      data_len;
  unsigned short      user_num;
  unsigned char       ext;
  unsigned char       cached_b:1,
                      free_data_b:1,
                      prio_b:1,
                      ping_b:1,
                      mem_owner:2,  /* 0:AP;1:CP;2:TC */
                      data_type:2;  /* 0:USER APP;1:ims_signaling;2:ims_voice;3:ims_video */
  unsigned int        ext1;
} aos_mbuf;

/**
 * alte_network_driver_cfg - contains all network card config.
 *
 * @nx_nic_ip_instance: IP instance bound to this NIC.
 * @nx_nic_interface_ptr: IP interface bound to this NIC.
 * @s_set: AIPC link channel.
 * @nx_nic_activated: The activation state of the NIC.
 * @netid: Network Card ID.
 * @aipc_recv_handle: AIPC recv brr handle.
 * @aipc_send_handle: AIPC send brr handle.
 * @thread_name: recv task name.
 * @recv_thread_id: recv task id.
 * @ul_current_throughput: current ul rate.
 * @dl_current_throughput: current dl rate.
 * @ul_max_speed: maximum ul speed.
 * @dl_max_speed: maximum dl speed.
 *
 * Include network card id, AIPC parameters, and status information interacting
 * with NETX DUO.
 */
struct alte_network_driver_cfg
{
    /* ip_instance is populated by NetX. */
    NX_IP *nx_nic_ip_instance;

    /* interface_ptr is populated by NetX. */
    NX_INTERFACE *nx_nic_interface_ptr;
    NX_PACKET *send_packet_queue_head;
    NX_PACKET *send_packet_queue_tail;

    NX_PACKET *send_ack_packet_queue_head;
    NX_PACKET *send_ack_packet_queue_tail;

    u32 normal_packet_queue_count;
    u32 high_packet_queue_count;
    u32 packet_queue_count;

    u32 share_packet_queue_count;
    u32 heap_packet_queue_count;
    u32 nx_packet_queue_count;

    u16 nx_nic_activated;
    u16 nx_nic_link_up;

    u8 netid;
    u32 s_set;
};

struct alte_network_driver
{
    u8 dongle_notify_flag;
    u16 pdp_active_flag;
    u8 network_access_retry_cnt;
    u8 network_access_status;

    u32 s_set;
#ifndef AIC_DISABLE_NET_CONNECT_CHECK
    u32 tcp_monitor_cnt;
    u32 icmp_monitor_cnt;
    u32 others_monitor_cnt;
    u64 monitor_timestamp;
    u32 notify_rebuild_cnt;

    u32 flight_tcp_cnt;
    u32 flight_icmp_cnt;

    u64 flight_timestamp;
    u32 last_trigger_fast_rebuild;
#endif
    u32 current_send_cache_size;

    volatile u32 s_link_up_status;

    u32 thread_cnt;
    aos_thread_id recv_thread_id;
    aos_thread_id send_thread_id;
    aos_thread_id ping_thread_id;

    aos_mutex_ptr send_mutex;
    aos_event_ptr send_event;

    ABRR_HANDLE aipc_recv_handle;
    ABRR_HANDLE aipc_send_handle;

    u32 max_send_cache_size;

    aos_timer_ptr enhance_timer;
    u32 enhance_timer_enable;
    u32 enhance_idle_time;
    u32 enhance_retry_time;
    u32 enhance_send_ok;
    u32 enhance_stage;

    u32 work_mode;
    u32 disable_copy;
#ifdef CONFIG_D2D
    u32 d2d_send_timeout;
    u32 enable_d2d_fc;
#endif
    void (*network_abnormal_notify)(u32 abnormal_level);
    u32 modem_duplex_mode;

    struct alte_network_driver_cfg alte_nic_devs[ALTE_NIC_MAX_INSTANCES];
};

void  nx_alte_nic_driver_entry(NX_IP_DRIVER *nx_ip_driver);
int nx_alte_release_block(u8 *addr, u32 size);
bool nx_alte_is_brr_block(u8 *addr);
void *nx_alte_alloc_block(u32 len, int timeout);
void nx_alte_notify_modem_release();
int nx_alte_check_cached_share_mem(void);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above. */
#ifdef __cplusplus
}
#endif

#endif
