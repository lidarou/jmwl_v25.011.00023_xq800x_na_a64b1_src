/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __AIC_NET_API_H__
#define __AIC_NET_API_H__

#include <aic_osa.h>
#include <sys/types.h>
#include "nx_api.h"
#ifdef CONFIG_SUBSYS_AP
#include <nx_alte_driver.h>
#endif
#include "aic_net_pdp_mgr.h"
#include "aic_fixed_apns.h"
#include "aic_additional_apns.h"

#ifndef WIN32
#include "nxd_sntp_client.h"
#include "aic_at_parse_utils.h"
#endif
#ifdef CONFIG_NET_IMS_NIC
#include "aic_ims_nic.h"
#endif
#ifdef CONFIG_NX_PPP
#include "nx_ppp.h"
#endif

#ifdef CONFIG_NX_BSD
#include "nxd_bsd.h"
#endif

#ifdef CONFIG_MCX_SUPPORT
#include <aic_mcx.h>
#endif
#include "ip_addr.h"

#ifndef __MCX_CODE
#define __MCX_CODE
#endif


#ifdef CONFIG_NX_RAM_DRIVER
extern VOID  _nx_ram_network_driver(NX_IP_DRIVER *driver_req_ptr);
#endif

#ifdef CONFIG_USBX_NETWORK
#define AIC_USB_LINK_UP   1
#define AIC_USB_LINK_DOWN 0
extern VOID _ux_network_driver_entry(NX_IP_DRIVER *nx_ip_driver);
#endif

#ifdef CONFIG_NET_ALTE_DRIVER
extern void nx_alte_nic_driver_entry(NX_IP_DRIVER *nx_ip_driver);
extern void nx_alte_notify_modem_release();
#endif

#define ANET_IP_TASK_STACK_SIZE SIZE_2K

#define ANET_PACKET_SIZE 1600
#ifndef ANET_MAX_PACKET_NUM
#define ANET_MAX_PACKET_NUM 128
#endif
/* packet pool size */
#define ANET_PACKET_POOL_SIZE ANET_PACKET_SIZE * ANET_MAX_PACKET_NUM

#ifdef CONFIG_NX_BSD
/* Logical FD starting value. */
#define AIC_BSD_SOCKFD_START NX_BSD_SOCKFD_START

/* Maximum number of total sockets available in the BSD layer. Note */
#define AIC_BSD_MAX_SOCKETS NX_BSD_MAX_SOCKETS
#endif

#define ALTE_TASK_HIGH_PRIORITY   3

#define AIC_NET_LOOPBACK_INTERFACE NX_LOOPBACK_INTERFACE

#define ANET_DNS_CACHE_SIZE SIZE_1K

#define AIC_NOTIFY_MODEM_RELEASE 2

#define ANET_IP_TYPE_V4 1
#define ANET_IP_TYPE_V6 2

#define AIC_LTE_AT_SOCKET_MODE               1
#define AIC_D2D_DATA_MODE                    2

struct frag_hdr {
    unsigned char nexthdr;
    unsigned char reserved;
    unsigned short frag_off;
    unsigned int identification;
};

struct iphdr {
    u8 ihl : 4,
       version : 4;
    u8 tos;
    u16 tot_len;
    u16 id;
    u16 frag_off;
    u8 ttl;
    u8 protocol;
    u16 check;
    u32 saddr;
    u32 daddr;
};

struct ipv6hdr {
    u8 priority : 4,
       version : 4;
    u8 flow_lbl[3];

    u16 payload_len;
    u8 nexthdr;
    u8 hop_limit;

    struct in6_addr saddr;
    struct in6_addr daddr;
};

struct tcphdr {
    u16 source;
    u16 dest;
    u32 seq;
    u32 ack_seq;
    u16 res1 : 4,
        doff : 4,
        fin : 1,
        syn : 1,
        rst : 1,
        psh : 1,
        ack : 1,
        urg : 1,
        ece : 1,
        cwr : 1;
    u16 window;
    u16 check;
    u16 urg_ptr;
};

struct udphdr {
    u16 source;
    u16 dest;
    u16 len;
    u16 check;
};

struct icmphdr {
    unsigned char  type;
    unsigned char  code;
    unsigned short checksum;
    union {
        struct {
            unsigned short id;
            unsigned short sequence;
        } echo;
        unsigned int gateway;
        struct {
            unsigned short unused;
            unsigned short mtu;
        } frag;
        unsigned char reserved[4];
    } un;
};

struct icmp6hdr {

    unsigned char  icmp6_type;
    unsigned char  icmp6_code;
    unsigned short icmp6_cksum;

    union {
        unsigned int   un_data32[1];
        unsigned short un_data16[2];
        unsigned char  un_data8[4];

        struct icmpv6_echo {
            unsigned short identifier;
            unsigned short sequence;
        } u_echo;

        struct icmpv6_nd_advt {
            unsigned int reserved : 5,
                         override :1,
                         solicited : 1,
                         router : 1,
                         reserved2 : 24;
        } u_nd_advt;

        struct icmpv6_nd_ra {
            unsigned char hop_limit;
            unsigned char reserved : 3,
                          router_pref : 2,
                          home_agent : 1,
                          other : 1,
                          managed : 1;

            unsigned short rt_lifetime;
        } u_nd_ra;

    } icmp6_dataun;

#define icmp6_identifier        icmp6_dataun.u_echo.identifier
#define icmp6_sequence          icmp6_dataun.u_echo.sequence
#define icmp6_pointer           icmp6_dataun.un_data32[0]
#define icmp6_mtu               icmp6_dataun.un_data32[0]
#define icmp6_unused            icmp6_dataun.un_data32[0]
#define icmp6_maxdelay          icmp6_dataun.un_data16[0]
#define icmp6_datagram_len      icmp6_dataun.un_data8[0]
#define icmp6_router            icmp6_dataun.u_nd_advt.router
#define icmp6_solicited         icmp6_dataun.u_nd_advt.solicited
#define icmp6_override          icmp6_dataun.u_nd_advt.override
#define icmp6_ndiscreserved     icmp6_dataun.u_nd_advt.reserved
#define icmp6_hop_limit         icmp6_dataun.u_nd_ra.hop_limit
#define icmp6_addrconf_managed  icmp6_dataun.u_nd_ra.managed
#define icmp6_addrconf_other    icmp6_dataun.u_nd_ra.other
#define icmp6_rt_lifetime       icmp6_dataun.u_nd_ra.rt_lifetime
#define icmp6_router_pref       icmp6_dataun.u_nd_ra.router_pref
};

enum aic_interface_type {
    AIC_PDP_INTERFACE,
    AIC_PPP_INTERFACE,
    AIC_USB_RNDIS_INTERFACE,
    AIC_USB_ECM_INTERFACE,
    AIC_USB_NCM_INTERFACE,
    AIC_USB_SEMI_TETHER_INTERFACE,
    AIC_NET_INTERFACE_CNT,
};

#define PDP_ONE_TO_ONE_MODE    0
#define PDP_ONE_TO_MULTI_MODE  1
#define AIC_GET_WAN_NET_FAIL 2

typedef enum aic_net_interface_type {
#ifdef CONFIG_NET_ALTE_DRIVER
    AIC_MODEM_NETWORK_LINK,
#endif
#ifdef CONFIG_USBX_NETWORK
    AIC_USB_NETWORK_LINK,
#endif
#ifdef CONFIG_NET_IMS_NIC
    AIC_IMSBR_NETWORK_LINK,
#endif
#ifdef CONFIG_NX_RAM_DRIVER
    AIC_RAM_NETWORK_LINK,
#endif
#ifdef CONFIG_NX_PPP
    AIC_PPP_NETWORK_LINK,
#endif
    AIC_MAX_NETWORK_LINK
} AIC_NET_HWIF_TYPE_E;

typedef enum ipv6_addr_type {
    IPV6_LOCAL_ADDR,
    IPV6_GLOBAL_ADDR
} IPV6_ADDR_TYPE_E;

#ifndef NX_DNS_NAME_MAX
#define NX_DNS_NAME_MAX 255
#endif

#define AIC_MODEM_TDD  1
#define AIC_MODEM_FDD  2

enum aic_network_msg {
    AIC_NETWORK_MODEM_SEND_OK   = 0,
    AIC_NETWORK_MODEM_SEND_FAIL = 1,
    AIC_NETWORK_DUPLEX_MODE     = 2,
    AIC_NETWORK_IMS_FLUSH       = 3,
    AIC_NETWORK_D2D_BW          = 4,
};

#define ANET_AUX_PACKET_SIZE 384
#define ANET_MAX_AUX_PACKET_NUM 192
/* packet pool size */
#define ANET_AUX_PACKET_POOL_SIZE ANET_AUX_PACKET_SIZE * ANET_MAX_AUX_PACKET_NUM

#define ANET_MULTIPACKET_SIZE 128
#define ANET_MAX_MULTIPACKET_NUM 576
/* packet pool size */
#define ANET_MULTIPACKET_POOL_SIZE ANET_MULTIPACKET_SIZE * ANET_MAX_MULTIPACKET_NUM

#define ANET_PACKET_POOL_CACHE_SIZE 45
/**
 * struct aic_network_ip_instance - IP instance management structure.
 * @ip_name: IP Instance name, external filling.
 * @ip_ptr: IP instance pointer, assigned internally by the protocol stack.
 * @pri: IP thread priority.
 * @task_stack: A task will be created in the IP instance, and this pointer
 *              points to the task stack, assigned internally by the protocol
 *              stack.
 * @task_stack_size: The size of the above task stack, external assignment.
 * @packet_pool: The packet pool used by the IP instance, when creating an IP
 *               instance, if it is NULL, it will be automatically
 *               created according to the packet_pool_size; if it it not NULL,
 *               it means that the external value has been assigned, and this
 *               packet pool will be configured to the IP instance.
 * @packet_pool_buf: If packet_pool is null, when creating an IP instance, it
 *                   will apply for memory and assign it to this member
 *                   according to packet_pool_size.
 * @per_packet_size: The size of each packet.
 * @arp_cache_mem: Points to the cache space of the arp mechanism.
 * @arp_cache_mem_size: arp cache space size.
 * @nat_cache_mem: Points to the cache space of the NAT mechanism.
 * @nat_cache_mem_size: NAT cache space size.
 * @if_type: hw interface type.
 * @bsd_task_stack: BSD thread task stack.
 * @bsd_task_stack_size: BSD thread task stack size.
 * @bsd_task_pri: BSD thread priority.
 * @packet_owner: Indicates whether the packet pool is applied for when an IP
 *                instance is created.
 * @bsd_owner: Indicates whether this IP instance is the default for bsd.
 *
 */
struct aic_network_ip_instance {
    NX_IP *ip_ptr;
    void *ip_name;

    u32 default_ip_address;
    u32 default_net_mask;

    u32 pri;
    void *task_stack;
    u32 task_stack_size;

    NX_PACKET_POOL *packet_pool;
#ifdef NX_ENABLE_DUAL_PACKET_POOL
    NX_PACKET_POOL *auxiliary_packet_pool;
#endif
#ifdef NX_ENABLE_MULTIPACKET_POOL
    NX_PACKET_POOL *multipacket_pool;
#endif
    void *packet_pool_buf;
    s8 *packet_pool_name;
    u32 packet_pool_size;
    u32 per_packet_size;

#ifndef CONFIG_DISABLE_IPV4_ARP
    void *arp_cache_mem;
    u32 arp_cache_mem_size;
#endif
    void *nat_cache_mem[NX_MAX_IP_INTERFACES];
    u32 nat_cache_mem_size;

    AIC_NET_HWIF_TYPE_E if_type;

    void *bsd_task_stack;
    u32 bsd_task_stack_size;
    u32 bsd_task_pri;

    bool packet_owner;
    bool bsd_owner;
    bool ip_owner;

    u32 suspend_timestamp;
    u32 delta_timestamp;
    u32 active_event_flag;

    u32 suspend_fast_timestamp;
    u32 delta_fast_timestamp;

    u32 ctcc_auth_mode;
};

/**
 * struct aic_packet_pool - aic packet pool information.
 * @packet_pool: netxduo packet pool, the space of the packet pool structure is
 *               applied from the heap interval.
 * @packet_pool_buf: Pointer to the memory space of the packet pool, apply from
 *                   the heap interval.
 * @packet_pool_size: The size of packet pool.
 * @packet_pool_name: packet pool name.
 * @per_packet_size: The size of each packet.
 *
 */
struct aic_packet_pool {
    NX_PACKET_POOL *packet_pool;
    void *packet_pool_buf;
    u32 packet_pool_size;

    s8 *packet_pool_name;
    u32 per_packet_size;
};

#if (CONFIG_PDP_MGR) && (CONFIG_NX_DNS)
typedef struct DNS_RESOLVE_CFG_STRUCT
{
    ulong ipv4_dns_address1;
    ulong ipv4_dns_address2;
    ulong ipv6_dns_address1[4];
    ulong ipv6_dns_address2[4];
    u32 timeout_one_shot;
    u8 max_retry_times;
    u8 dns_cache_enabled;
} DNS_RESOLVE_CFG;
#endif

#ifdef CONFIG_NX_PPP

/**
 * struct aic_network_ppp - aic ppp instructure.
 * @nx_ppp: need to be initialized to NULL, alloc mem for it when PPP create.
 * @nx_ppp_name: ppp name.
 * @nx_ip_ptr: depends on the ip instance, need to initialize.
 * @stack_memory_ptr: nx ppp task stack memory.
 * @stack_size: nx ppp task stack memory size.
 * @thread_priority: nx ppp task priority.
 * @packet_pool: netxduo packet pool, if this value is NULL when PPP is created,
 *               the space of the packet pool structure will be applied from the
 *               heap interval.
 * @packet_pool_buf: if @packet_pool is NULL when PPP is created, the memory
 *                   space of the packet pool will be applied from the heap
 *                   interval.
 * @packet_pool_size: The size of packet pool.
 * @packet_pool_name: packet pool name.
 * @per_packet_size: The size of each packet.
 * @packet_owner: Indicates whether the packet pool is applied for when PPP is
 *                created.
 * @ppp_non_ppp_packet_handler: handle non ppp packet callback.
 * @ppp_byte_send: ppp send byte callback.
 */
struct aic_network_ppp {
    NX_PPP *nx_ppp;
    s8 *nx_ppp_name;

    NX_IP *nx_ip_ptr;

    void *stack_memory_ptr;
    u32 stack_size;
    u32 thread_priority;

    NX_PACKET_POOL *packet_pool;
    void *packet_pool_buf;
    u32 packet_pool_size;
    u32 per_packet_size;
    s8 *packet_pool_name;

    bool packet_owner;
    bool ip_owner;

    void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr);
    void (*ppp_byte_send)(u8 byte);
};

typedef struct AIC_PPP_ECHO_CFG_STRUCT
{
    u8 echo_enabled;
    u8 timeout_one_shot;
    u8 retry_times;
    u32 echo_interval;
} AIC_PPP_ECHO_CFG;
#endif

/* Define IPv4/v6 Address structure */
struct aic_network_address {
    /* Flag indicating IP address format.  Valid values are: 4 and 6. */
    u32 ip_version;

    /* Union that holds either IPv4 or IPv6 address. */
    union {
        u32 v4;
        u32 v6[4];
    } ip_address;
};

#ifndef WIN32
struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                           terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                           network byte order) for the host, terminated by a null pointer. */
#define h_addr h_addr_list[0] /* for backward compatibility */
};
#endif

typedef void (*IP_LINK_DRIVER)(struct NX_IP_DRIVER_STRUCT *);

int aic_get_ipv6_prefix_ifaddr(IPV6_ADDR_TYPE_E type, s8 *ip_name, u8 *addr,
                               u32 addr_len, ulong *prefix_length);

int aic_create_ip_instance(struct aic_network_ip_instance *aic_ip_ptr);

int aic_delete_ip_instance(struct aic_network_ip_instance *aic_ip_ptr);

u32 aic_tcp_client_source_ip_bind(NX_TCP_SOCKET *socket_ptr, u8 cid,
                                  u16 sa_family, ulong wait_option);

int aic_create_packet_pool(struct aic_packet_pool *aic_pool);

int aic_delete_packet_pool(struct aic_packet_pool *aic_pool);

#ifdef CONFIG_NX_PPP
int aic_create_ppp(struct aic_network_ppp *aic_ppp);

int aic_delete_ppp(struct aic_network_ppp *aic_ppp);

void aic_set_ppp_echo_cfg(AIC_PPP_ECHO_CFG *echo_cfg);
#endif

#ifdef CONFIG_NX_NAT
int aic_nat_create(void *nat_device, struct aic_network_ip_instance *aic_ip_ptr,
                   u32 global_interface_index);

void aic_nat_delete(void *nat_dev, struct aic_network_ip_instance *aic_ip_ptr);
void aic_nat_update_win(u32 dl_win, u32 ul_win);

#endif

int aic_enable_arp(struct aic_network_ip_instance *aic_ip_ptr);

void aic_free_arp_cache(struct aic_network_ip_instance *aic_ip_ptr);

int aic_get_ipv6_addr_index(struct aic_network_ip_instance *aic_ip_ptr,
                            void *ip_address, u32 *interface_index);

int aic_init_bsd(struct aic_network_ip_instance *aic_ip_ptr);

#if (CONFIG_PDP_MGR) && (CONFIG_NX_DNS)
u32 aic_init_dns_client(void);

void aic_delete_dns_client(void);

u32 aic_add_dns_servers(WAN_NET *wan_net_ptr);

void aic_update_dns_servers(WAN_NET *wan_net_ptr);

void *aic_get_dns_client(void);

void aic_clear_dns_cache(void);

int aic_set_dns_server_ip(u8 ip_type, u8 *dns_server1, u8 *dns_server2);

void aic_init_user_dns_cfg(void);

void aic_disable_dns_cache(void);

void aic_enable_dns_cache(void);

ulong aic_get_dns_cache_ttl(u8 *host_name, u8 lookups_type);

boolean aic_is_dns_cache_enabled(void);

void aic_set_dns_timeout(u8 timeout_once_seconds, u8 retry_cnt);

u32 aic_get_dns_timeout_once(void);

u8 aic_get_dns_max_retry_times(void);
#endif

#ifdef CONFIG_USBX_NETWORK
void aic_usb_register_link_notify_callback(void (*link_notify_cb)(u32 , u32));
#endif

ulong aic_sntp_client_convert_date_to_seconds(aos_systime *local_time, int tz);

int aic_net_get_trans_params(NX_PACKET *packet_ptr, u8 *protocol,
                             u32 *ip_header_len, u16 *src_port,
                             u16 *dst_port);

int aic_network_traffic_control_disable(u8 cid);

/**
 * aic_network_traffic_control_enable - Set traffic control parameters.
 *
 * @cid: PDP id.
 * @dl_max_speed: Maximum downlink rate, bps.
 * @ul_max_speed: Maximum uplink rate, bps.
 *
 * Use a periodic timer for traffic control, if the PDP corresponding to the CID
 * is deactivated, the traffic control will automatically close, need to re enable.
 */
int aic_network_traffic_control_enable(u8 cid, u32 dl_max_speed,
                                       u32 ul_max_speed);

int aic_network_traffic_control_suspend(void);

int aic_socket_register_establish_notify(int sockID,
                                         void (*tcp_establish_notify)(int sock_id, void *pdata),
                                         void *pdata);

int aic_socket_unregister_establish_notify(int sockID);

int aic_socket_register_connect_notify(int sockID,
                                       void (*tcp_connect_notify)(int sock_id, u32 status, void *pdata),
                                       void *pdata);

int aic_socket_unregister_connect_notify(int sockID);

int aic_socket_register_receive_notify(int sockID,
                                       void (*tcp_recv_notify)(INT sock_id, void *pdata),
                                       void *pdata);
int aic_socket_unregister_receive_notify(int sockID);

int aic_socket_register_tcp_disconnect_notify(int sockID,
                                              void (*tcp_disconnect_notify)(int sock_id, void *pdata, int is_active),
                                              void *pdata);

int aic_socket_unregister_tcp_disconnect_notify(int sockID);

#ifdef CONFIG_SUBSYS_AP
int aic_socket_register_receive_ext_notify(int sockID,
                                       void (*tcp_recv_ext_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata);
int aic_socket_unregister_receive_ext_notify(int sockID);
int aic_socket_register_tcp_ack_notify(int sockID,
                                       void (* tcp_ack_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata);
int aic_socket_unregister_tcp_ack_notify(int sockID);
int aic_socket_register_udp_ack_notify(int sockID,
                                       void (*udp_ack_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata);
int aic_socket_unregister_udp_ack_notify(int sockID);
int aic_socket_register_udp_drop_notify(int sockID,
                                       void (* udp_drop_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata);
int aic_socket_unregister_udp_drop_notify(int sockID);
#endif

int aic_socket_setup_window(int sockID, u32 window_size, UINT ack_threshold);

int aic_socket_send_keepalive(int sockID);

struct aic_network_ip_instance *aic_get_default_ip_instance(void);
#ifdef CONFIG_SUBSYS_AP
struct hostent *aic_gethostbyname(const char *name);
#endif
int aic_register_arp_age_notify(struct aic_network_ip_instance *aic_ip_ptr,
                                void (*nx_ip_arp_age_notify)(NX_INTERFACE *nx_interface, ULONG ip_address));
#ifdef CONFIG_SUBSYS_AP
bool aic_identify_packet_pri(NX_PACKET *packet_ptr,
                             struct aos_mbuf_st *mbuf);
#endif

u32 aic_get_mobile_traffic(long *rxpackets, long *rxbytes, long *txpackets,
                           long *txbytes);

#ifdef CONFIG_NET_ALTE_DRIVER
void aic_network_setup_enhance_low_power(u32 enable, u32 idle_time,
                                         u32 retry_time);
void aic_network_register_abnormal_notify(void (*cb)(u32));
void aic_network_trigger_rebuild(void);
void aic_set_lte_nic_work_mode(u32 mode);
#endif

#ifdef CONFIG_NET_LOW_POWER
void aic_network_suspend(void *param);
void aic_network_resume(void *param);
#endif

int aic_tcp_socket_retransmit_configure(int sockID, ULONG max_retries,
                                        ULONG max_timeout);
int aic_tcp_socket_transmit_max_queue_configure(int sockID, u32 max_queue_depth);

void aic_checksum_adjust(unsigned char* checksum, unsigned char* old_data,
                         int old_data_length, unsigned char* new_data,
                         int new_data_length);
void aic_checksum_add(unsigned char* checksum, unsigned char* new_data,
                      int new_data_length);
void aic_checksum_reduce(unsigned char* checksum, unsigned char* old_data,
                         int old_data_length);
int aic_net_check_fragment_packet(NX_PACKET *packet_ptr);

int aic_net_check_ctcc_auth();
#ifndef WIN32
u32 aic_sntp_client_get_time(CHAR * buffer, UINT length, int time_zone);
#endif
#ifdef CONFIG_D2D
void aic_cfg_d2d_send_timeout(u32 timeout);
int aic_lte_nic_send(NX_PACKET *packet_ptr);
int aic_d2d_send(u8 *buff, u32 size);
void aic_d2d_setup_local_audio_mode();
#endif
ulong aic_mktime_to_seconds(aos_systime *tm_ptr);

#endif
