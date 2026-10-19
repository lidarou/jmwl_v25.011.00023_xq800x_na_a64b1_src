/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*****************************************************************************/
/**                                                                          */
/** AIC Component                                                            */
/**                                                                          */
/**   ping                                                                   */
/**                                                                          */
/*****************************************************************************/
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x028f9d
#define LOG_MODULE    AMOD_NET

#include <aic_cmd.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>

#include "nx_ip.h"
#include "nx_api.h"
#include "nx_packet.h"
#include "nxd_bsd.h"
#include "nx_icmpv6.h"
#include "nx_icmpv4.h"

/**
 * struct aic_network_ping - AIC ping management structure.
 * @timer: a timer with a period of 1s is used to trigger the ping operation.
 * @ping_counter: number of times the ping operation needs to be performed.
 * @dst_addr: destination address.
 * @src_addr: specifies the source address.
 * @semaphore: this semaphore is used for synchronous ping operations.
 * @thread_id: task id that executes the ping operation.
 * @ip_head_ptr: points to the default IP instance.
 * @ip_match_ptr: Matched IP instance.
 * @next_hop_address: used for IPv4.
 * @interface_ptr: used for IPv4.
 * @outgoing_address: used for IPv6.
 * @in_progress: indicates whether a ping operation is being performed.
 * @need_stop: whether to cancel the ping operation.
 * @need_reinit: whether reinitialization is required.
 * @use_def_ip: whether to use the default IP instance.
 * @data_ptr: point to send buffer.
 * @data_len: send length.
 * @wait_time: ping timeout, in ms.
 * @send_cnt: Number of pings that have been sent.
 * @recv_cnt: Number of relay that have been recv.
 *
 */
struct aic_network_ping {
    aos_timer_ptr timer;

    u32 ping_counter;

    NXD_ADDRESS dst_addr;
    NXD_ADDRESS src_addr;

    aos_semaphore_ptr semaphore;

    aos_thread_id thread_id;

    NX_IP *ip_head_ptr;
    NX_IP *ip_match_ptr;

    ULONG next_hop_address;
    NX_INTERFACE *interface_ptr;

    NXD_IPV6_ADDRESS *outgoing_address;

    u8 in_progress;
    u8 need_stop;
    u8 need_reinit;
    u8 use_def_ip;

    u8 *data_ptr;
    u32 data_len;

    u32 period_time;
    u32 wait_time;

    u32 send_cnt;
    u32 recv_cnt;
};

static struct aic_network_ping aic_net_ping_cfg = {0};

/* The timer callback that uses the semaphore to trigger the ping task */
static void aic_ping_timer_callback(void *input)
{
    struct aic_network_ping *ping_cfg = input;

    ping_cfg->ping_counter--;
    aos_put_semaphore(ping_cfg->semaphore);
}

/* use src addr or dst addr to match the appropriate IP instance */
static void aic_ping_find_ip_instance(u32 dst_addr)
{
    int flag = FALSE;
    u32 ret;
    NX_IP *ip_ptr;
    struct aic_network_ping *ping_cfg = &aic_net_ping_cfg;
    int i;

    ip_ptr = _nx_ip_created_ptr;

    if (ip_ptr) {
        if (ping_cfg->src_addr.nxd_ip_version == NX_IP_VERSION_V4) {
            for (i = 0; i < NX_MAX_IP_INTERFACES; i++) {
                ping_cfg->interface_ptr = &(ip_ptr->nx_ip_interface[i]);

                if ((ping_cfg->interface_ptr->nx_interface_valid) &&
                    (ping_cfg->interface_ptr->nx_interface_link_up) &&
                    ((ping_cfg->interface_ptr->nx_interface_ip_address) ==
                     ping_cfg->src_addr.nxd_ip_address.v4)) {
                    ping_cfg->ip_match_ptr = ip_ptr;
                    ret = _nx_ip_route_find(ip_ptr,
                                            ping_cfg->src_addr.nxd_ip_address.v4,
                                            &ping_cfg->interface_ptr,
                                            &ping_cfg->next_hop_address);
                    if (!ret) {
                        flag = TRUE;
                    } else {
                        CONS_OUT("ping match ip instance fail by src addr 0x%x\n",
                                 ret);
                        ping_cfg->use_def_ip = TRUE;
                    }
                    CONS_OUT("ping match ip instance success by src addr\n");
                }
            }
        } else if (ping_cfg->src_addr.nxd_ip_version == NX_IP_VERSION_V6) {
            for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
                /* Compare the address. */
                if ((ip_ptr->nx_ipv6_address[i].nxd_ipv6_address_valid) &&
                    (CHECK_IPV6_ADDRESSES_SAME(ip_ptr->nx_ipv6_address[i].nxd_ipv6_address,
                                               ping_cfg->src_addr.nxd_ip_address.v6))) {
                     /* Found a proper address. */
                    ping_cfg->outgoing_address = &(ip_ptr->nx_ipv6_address[i]);
                    ping_cfg->ip_match_ptr = ip_ptr;

                    flag = TRUE;
                    CONS_OUT("ping match ip instance success by src ipv6 addr\n");
                }
            }

            if (!flag) {
                CONS_OUT("ping match ip instance fail by src ipv6 addr\n");
                ping_cfg->outgoing_address = &(ip_ptr->nx_ipv6_address[0]);
                ping_cfg->use_def_ip = TRUE;
            }
        } else {
            if (ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V4) {
                ret = _nx_ip_route_find(ip_ptr,
                                        ping_cfg->dst_addr.nxd_ip_address.v4,
                                        &ping_cfg->interface_ptr,
                                        &ping_cfg->next_hop_address);
                if (!ret) {
                    ping_cfg->ip_match_ptr = ip_ptr;
                    flag = TRUE;
                }
            } else if (ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V6) {
                ret = _nxd_ipv6_interface_find(ip_ptr,
                                               ping_cfg->dst_addr.nxd_ip_address.v6,
                                               &ping_cfg->outgoing_address,
                                               NX_NULL);
                if (!ret) {
                    ping_cfg->ip_match_ptr = ip_ptr;
                    flag = TRUE;
                }
            } else {
                CONS_OUT("IP addr version is err 0x%x\n",
                         ping_cfg->dst_addr.nxd_ip_version);
                return;
            }
        }
    }

    if (!flag || ping_cfg->use_def_ip) {
        ping_cfg->use_def_ip = TRUE;
        ping_cfg->ip_match_ptr = ping_cfg->ip_head_ptr;
    }
}

/* After the ping operation, this interface is called to reinitialize the
 * params.
 */
static void aic_ping_end(struct aic_network_ping *ping_cfg)
{
    CONS_OUT("Sent: %d received: %d loss: %d\n", ping_cfg->send_cnt,
             ping_cfg->recv_cnt,
             ping_cfg->send_cnt - ping_cfg->recv_cnt);
    ping_cfg->in_progress = FALSE;
    ping_cfg->data_len = 32;
    ping_cfg->ping_counter = 20;
    ping_cfg->need_stop = FALSE;
    memset(&ping_cfg->src_addr, 0, sizeof(NXD_ADDRESS));
    ping_cfg->wait_time = 1000;
    ping_cfg->period_time = 1000;
    ping_cfg->send_cnt = 0;
    ping_cfg->recv_cnt = 0;
    ping_cfg->ip_match_ptr = NULL;
    ping_cfg->interface_ptr = NULL;
    ping_cfg->next_hop_address = 0;
    ping_cfg->outgoing_address = NULL;
    ping_cfg->use_def_ip = FALSE;
    if (ping_cfg->data_ptr) {
        aos_free(ping_cfg->data_ptr);
        ping_cfg->data_ptr = NULL;
    }
}

static void aic_ping_task(void *param)
{
    char ip_buff[32];
    NXD_ADDRESS addr;
    u32 ret, val, first_time, delta_time;
    NX_PACKET *my_packet = NULL;
    NX_IPV4_HEADER *ipv4_header;
    struct aic_network_ping *ping_cfg = param;

    while (1) {
        ret = aos_get_semaphore(ping_cfg->semaphore, AOS_WAIT_FOREVER);
        if (ret) {
            aicx_error(0x15914e, "x", "aic ping get semaphore fail 0x%x\n", ret);
            continue;
        }

        if (ping_cfg->need_stop) {
            aic_ping_end(ping_cfg);
            aos_deactivate_timer(ping_cfg->timer);
            continue;
        }

        if (ping_cfg->need_reinit) {
            if (ping_cfg->data_ptr)
                aos_free(ping_cfg->data_ptr);
            ping_cfg->data_ptr = aos_malloc(ping_cfg->data_len);
            aic_ping_find_ip_instance(ping_cfg->dst_addr.nxd_ip_address.v4);
            ping_cfg->need_reinit = FALSE;
        }

        if (!ping_cfg->in_progress)
            continue;

        if (!ping_cfg->ping_counter) {
            aos_deactivate_timer(ping_cfg->timer);
        }

        ping_cfg->send_cnt++;
        first_time = aos_get_tickcount();

        if (!ping_cfg->use_def_ip &&
            ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V4 &&
            ping_cfg->ip_match_ptr) {
            ret = _nx_icmp_interface_ping(ping_cfg->ip_match_ptr,
                                          ping_cfg->dst_addr.nxd_ip_address.v4,
                                          ping_cfg->interface_ptr,
                                          ping_cfg->next_hop_address,
                                          ping_cfg->data_ptr,
                                          ping_cfg->data_len,
                                          &my_packet, ping_cfg->wait_time);
        } else if (!ping_cfg->use_def_ip &&
                   ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V6 &&
                   ping_cfg->ip_match_ptr) {
            ret = _nx_icmp_interface_ping6(ping_cfg->ip_match_ptr,
                                           &ping_cfg->dst_addr,
                                           ping_cfg->data_ptr,
                                           ping_cfg->data_len,
                                           ping_cfg->outgoing_address,
                                           &my_packet, ping_cfg->wait_time);
        } else {
            ret = nxd_icmp_ping(ping_cfg->ip_head_ptr, &ping_cfg->dst_addr,
                                ping_cfg->data_ptr, ping_cfg->data_len,
                                &my_packet, ping_cfg->wait_time);
        }

        if ((ret != NX_SUCCESS) || (my_packet == NX_NULL)) {
            CONS_OUT("aic ping fail 0x%x 0x%x, no response\n", ret,
                     (u32)my_packet);
            if (!ping_cfg->ping_counter)
                aic_ping_end(ping_cfg);

            if (my_packet)
                nx_packet_release(my_packet);

            continue;
        }

        delta_time = aos_get_tickcount() - first_time;
        ping_cfg->recv_cnt++;
        if (ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V4) {
            ipv4_header = (NX_IPV4_HEADER *)my_packet->nx_packet_ip_header;
            val = ipv4_header->nx_ip_header_word_2;
            CONS_OUT("Come from %d.%d.%d.%d' reply: byte=%d time=%dms ttl=%d\n",
                     ping_cfg->dst_addr.nxd_ip_address.v4 >> 24,
                     (ping_cfg->dst_addr.nxd_ip_address.v4 >> 16) & 0xFF,
                     (ping_cfg->dst_addr.nxd_ip_address.v4 >> 8) & 0xFF,
                     (ping_cfg->dst_addr.nxd_ip_address.v4) & 0xFF,
                     my_packet->nx_packet_length, delta_time, val >> 24);
        } else if (ping_cfg->dst_addr.nxd_ip_version == NX_IP_VERSION_V6) {
            memcpy(addr.nxd_ip_address.v6, ping_cfg->dst_addr.nxd_ip_address.v6,
                   16);
            NX_CHANGE_ULONG_ENDIAN(addr.nxd_ip_address.v6[0]);
            NX_CHANGE_ULONG_ENDIAN(addr.nxd_ip_address.v6[1]);
            NX_CHANGE_ULONG_ENDIAN(addr.nxd_ip_address.v6[2]);
            NX_CHANGE_ULONG_ENDIAN(addr.nxd_ip_address.v6[3]);
            inet_ntop(AF_INET6, addr.nxd_ip_address.v6, ip_buff,
                      sizeof(ip_buff));
            CONS_OUT("Come from %s' reply: time=%dms\n", ip_buff, delta_time);
        }

        nx_packet_release(my_packet);
        if (!ping_cfg->ping_counter)
            aic_ping_end(ping_cfg);
    }
}

static int aic_network_ping_cmd(int argc, char **argv)
{
    int ret;
    struct addrinfo *res, hints;
    struct aic_network_ping *ping_cfg = &aic_net_ping_cfg;
    int i;
    UNUSED_PARAM(argv);

    CONS_OUT("argc %d\n", argc);
    for (i = 0; i < argc; i++) {
        CONS_OUT("index %d %s\n", i, argv[i]);
        if (i == 0) {
            if (!strcmp(argv[i], "stop")) {
                ping_cfg->need_stop = TRUE;
                aos_put_semaphore(ping_cfg->semaphore);
                return AIC_SUCCESS;
            }

            if (ping_cfg->in_progress) {
                CONS_OUT("aic ping is busy, please stop the ping firstly\n");
                return AIC_EBUSY;
            }

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            ret = getaddrinfo(argv[i], NULL, &hints, &res);
            if (ret != NX_SUCCESS || !res) {
                aicx_error(0x0ae316, "sd", "[PING]: get addr info fail %s ret %d\r\n",
                           (s8 *)argv[i], ret);
                return ret;
            }

            if(res->ai_addr->sa_family == AF_INET) {
                ping_cfg->dst_addr.nxd_ip_version = NX_IP_VERSION_V4;
                aos_memcpy(&ping_cfg->dst_addr.nxd_ip_address.v4,
                           &res->ai_addr->sa_data[2], 4);
                NX_CHANGE_ULONG_ENDIAN(ping_cfg->dst_addr.nxd_ip_address.v4);
                aicx_info(0x13f559, "dddd", "[PING]: get address is v4 %d:%d:%d:%d\r\n",
                          (ping_cfg->dst_addr.nxd_ip_address.v4 >> 24) & 0xFF,
                          (ping_cfg->dst_addr.nxd_ip_address.v4 >> 16) & 0xFF,
                          (ping_cfg->dst_addr.nxd_ip_address.v4 >> 8) & 0xFF,
                          (ping_cfg->dst_addr.nxd_ip_address.v4) & 0xFF);
            } else {
                ping_cfg->dst_addr.nxd_ip_version = NX_IP_VERSION_V6;
                aos_memcpy(ping_cfg->dst_addr.nxd_ip_address.v6,
                           &(((struct sockaddr_in6*)res->ai_addr)->sin6_addr), 16);
                NX_CHANGE_ULONG_ENDIAN(ping_cfg->dst_addr.nxd_ip_address.v6[0]);
                NX_CHANGE_ULONG_ENDIAN(ping_cfg->dst_addr.nxd_ip_address.v6[1]);
                NX_CHANGE_ULONG_ENDIAN(ping_cfg->dst_addr.nxd_ip_address.v6[2]);
                NX_CHANGE_ULONG_ENDIAN(ping_cfg->dst_addr.nxd_ip_address.v6[3]);
                aicx_info(0x0fa546, "xxxxxxxx", "[PING]: get address is v6 %x:%x:%x:%x:%x:%x:%x:%x\r\n",
                          ping_cfg->dst_addr.nxd_ip_address.v6[0] >> 16,
                          ping_cfg->dst_addr.nxd_ip_address.v6[0] & 0xFFFF,
                          ping_cfg->dst_addr.nxd_ip_address.v6[1] >> 16,
                          ping_cfg->dst_addr.nxd_ip_address.v6[1] & 0xFFFF,
                          ping_cfg->dst_addr.nxd_ip_address.v6[2] >> 16,
                          ping_cfg->dst_addr.nxd_ip_address.v6[2] & 0xFFFF,
                          ping_cfg->dst_addr.nxd_ip_address.v6[3] >> 16,
                          ping_cfg->dst_addr.nxd_ip_address.v6[3] & 0xFFFF);
            }

            freeaddrinfo(res);
            continue;
        }

        if (*(argv[i]) == '-' &&
            ((*(argv[i] + 1) >= 65) && (*(argv[i] + 1) <= 121))) {
            if ((i + 1) >= argc) {
                CONS_OUT("aic ping invalid param\n");
                return AIC_EINVAL;
            }

            switch (*(argv[i] + 1)) {
            case 'n':
                ping_cfg->ping_counter = atoi(argv[++i]);
                CONS_OUT("ping counter %d\n", ping_cfg->ping_counter);
                break;
            case 'S':
                ret = inet_pton(AF_INET, argv[i + 1],
                                &ping_cfg->src_addr.nxd_ip_address.v4);
                if (ret == 1) {
                    ping_cfg->src_addr.nxd_ip_version = NX_IP_VERSION_V4;
                    NX_CHANGE_ULONG_ENDIAN(ping_cfg->src_addr.nxd_ip_address.v4);
                    continue;
                } else {
                    ret = inet_pton(AF_INET6, argv[i + 1],
                                    &ping_cfg->src_addr.nxd_ip_address.v6);
                    if (ret == 1) {
                        ping_cfg->src_addr.nxd_ip_version = NX_IP_VERSION_V6;
                    } else {
                        CONS_OUT("get IPv4/IPv6 addr fail 0x%x\n", ret);
                        return AIC_EFAULT;
                    }
                    NX_CHANGE_ULONG_ENDIAN(ping_cfg->src_addr.nxd_ip_address.v6[0]);
                    NX_CHANGE_ULONG_ENDIAN(ping_cfg->src_addr.nxd_ip_address.v6[1]);
                    NX_CHANGE_ULONG_ENDIAN(ping_cfg->src_addr.nxd_ip_address.v6[2]);
                    NX_CHANGE_ULONG_ENDIAN(ping_cfg->src_addr.nxd_ip_address.v6[3]);
                    CONS_OUT("[IPv6] addr 0x%x 0x%x 0x%x 0x%x\n",
                             ping_cfg->src_addr.nxd_ip_address.v6[0],
                             ping_cfg->src_addr.nxd_ip_address.v6[1],
                             ping_cfg->src_addr.nxd_ip_address.v6[2],
                             ping_cfg->src_addr.nxd_ip_address.v6[3]);
                }
                i++;
                break;
            case 'l':
                ping_cfg->data_len = atoi(argv[++i]);
                CONS_OUT("ping size i %d %d %s\n",
                         i, ping_cfg->data_len, argv[i]);
                break;
            case 'w':
                ping_cfg->wait_time = atoi(argv[++i]);
                CONS_OUT("ping wait_time i %d %d %s\n",
                         i, ping_cfg->wait_time, argv[i]);
                break;
            case 'p':
                ping_cfg->period_time = atoi(argv[++i]);
                CONS_OUT("ping period time i %d %d %s\n",
                         i, ping_cfg->period_time, argv[i]);
                break;
            default:
                CONS_OUT("aic ping can't support %s\n", argv[i]);
                return AIC_EPFNOSUPPORT;
            }
        }
    }

    ping_cfg->in_progress = TRUE;
    ping_cfg->need_reinit = TRUE;
    aos_change_timer(ping_cfg->timer, ping_cfg->period_time, AOS_BE_PERIOD);
    aos_activate_timer(ping_cfg->timer);

    return 0;
}
#ifndef WIN32
CONSOLE_EXPORT_CMD(ping, aic_network_ping_cmd, aic network ping);
#endif

int aic_network_ping_init(void)
{
    struct aic_network_ping *ping_cfg = &aic_net_ping_cfg;

    ping_cfg->data_len = 32;
    ping_cfg->ping_counter = 20;
    ping_cfg->wait_time = 1000;
    ping_cfg->period_time = 1000;
    ping_cfg->ip_head_ptr = _nx_ip_created_ptr;
    ping_cfg->timer = aos_create_timer("aic network ping",
                                       aic_ping_timer_callback,
                                       ping_cfg, ping_cfg->period_time,
                                       AOS_NO_ACTIVATE, AOS_BE_PERIOD);

    ping_cfg->semaphore = aos_create_semaphore("aic ping semaphore", 0);

    ping_cfg->thread_id = aos_create_thread("aic ping task", aic_ping_task,
                                            ping_cfg, SIZE_2K, 0, NULL, 15,
                                            AOS_AUTO_START);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_network_ping_init);

