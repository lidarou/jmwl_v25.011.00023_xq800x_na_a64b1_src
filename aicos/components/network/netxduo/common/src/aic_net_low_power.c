
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x03c5a8
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>

#include "aic_net_api.h"
#include "nx_api.h"
#include "nx_ip.h"

#include "timer_inc.h"
#include "slp_inc.h"
#include "nx_nd_cache.h"
#include "nx_tcp.h"

#include "user_tracex_api.h"

#define AIC_NETWORK_DELTA_THREADHOLD 10000

#define AIC_FAST_TIMER_TYPE   1
#define AIC_NORMAL_TIMER_TYPE 2

#define AIC_NETWORK_FAST_WAKE_EVENT   BIT(0)
#define AIC_NETWORK_NORMAL_WAKE_EVENT BIT(1)

static int aic_network_suspend_check_fast_icmpv6(struct aic_network_ip_instance *
                                                 ip_instance)
{
    int i;
    NX_IP *ip_ptr = ip_instance->ip_ptr;
    struct NXD_IPV6_ADDRESS_STRUCT *interface_ipv6_address;

    if (ip_ptr->nx_ip_icmpv6_packet_process) {
        /* Proceed with DAD check only if ICMPv6 is enabled. */
        for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
            interface_ipv6_address =
                ip_ptr->nx_ip_interface[i].nxd_interface_ipv6_address_list_head;

            while (interface_ipv6_address) {
                if (interface_ipv6_address->nxd_ipv6_address_DupAddrDetectTransmit ||
                    interface_ipv6_address->nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_TENTATIVE) {
                    aicx_debug(0x0c217e, "sdd", "[LOW_POWER] %s status %d have DAD %d to send\r\n",
                              ip_ptr->nx_ip_interface[i].nx_interface_name,
                              interface_ipv6_address->nxd_ipv6_address_state,
                              interface_ipv6_address->nxd_ipv6_address_DupAddrDetectTransmit);
                    return AIC_EBUSY;
                }

                interface_ipv6_address = interface_ipv6_address->nxd_ipv6_address_next;
            }
        }
    }

    return AIC_SUCCESS;
}

static int aic_network_suspend_check_fast_nd_cache(struct aic_network_ip_instance *
                                                   ip_instance)
{
    int i;
    NX_IP *ip_ptr = ip_instance->ip_ptr;

    /* Loop through all entries, and invalidate the ones that are timed out. */
    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++) {

        /* Check the entry is valid. */
        if (ip_ptr->nx_ipv6_nd_cache[i].nx_nd_cache_nd_status ==
            ND_CACHE_STATE_INVALID)
            continue;

        /* Is this entry being checked for neighbor discovery? */
        if ((ip_ptr->nx_ipv6_nd_cache[i].nx_nd_cache_nd_status !=
             ND_CACHE_STATE_PROBE) &&
            (ip_ptr->nx_ipv6_nd_cache[i].nx_nd_cache_nd_status !=
             ND_CACHE_STATE_INCOMPLETE))
            continue;

        aicx_debug(0x0631ee, "", "[LOW_POWER] have nd cache to check\r\n");
        return AIC_EBUSY;
    }

    return AIC_SUCCESS;
}

static int aic_network_suspend_check_fast_tcp(struct aic_network_ip_instance *
                                              ip_instance)
{
    AOS_INTERRUPT_DECLARE
    NX_IP *ip_ptr = ip_instance->ip_ptr;
    NX_TCP_SOCKET *socket_ptr;
    ULONG          sockets;
    ULONG          timer_rate;

    /* Pickup this timer's periodic rate.  */
    timer_rate = _nx_tcp_fast_timer_rate;

    AOS_INTERRUPT_DISABLE
    /* Pickup the number of created TCP sockets.  */
    sockets = ip_ptr->nx_ip_tcp_created_sockets_count;

    /* Pickup the first socket.  */
    socket_ptr = ip_ptr->nx_ip_tcp_created_sockets_ptr;

    /* Loop through the created sockets. */
    while (sockets--) {
        /* Determine if the socket is in an established or disconnect state and
           has delayed sending an ACK from a previous receive packet event. */
        if ((socket_ptr->nx_tcp_socket_state >= NX_TCP_ESTABLISHED) &&
            ((socket_ptr->nx_tcp_socket_rx_sequence !=
              socket_ptr->nx_tcp_socket_rx_sequence_acked) ||
             (socket_ptr->nx_tcp_socket_rx_window_last_sent <
              socket_ptr->nx_tcp_socket_rx_window_current))) {
            AOS_INTERRUPT_RESTORE
            aicx_debug(0x1c4f33, "", "[LOW_POWER] has delayed sending an ACK\r\n");
            return AIC_EBUSY;
        }

        /* Move to the next TCP socket.  */
        socket_ptr =  socket_ptr->nx_tcp_socket_created_next;
    }

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

static int aic_network_suspend_check_fast_tcp_timeout(struct aic_network_ip_instance *
                                                      ip_instance)
{
    NX_IP *ip_ptr = ip_instance->ip_ptr;
    NX_TCP_SOCKET *socket_ptr;
    ULONG          sockets;
    ULONG          min_timeout = 0, timeout = 0;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    /* Pickup the number of created TCP sockets.  */
    sockets = ip_ptr->nx_ip_tcp_created_sockets_count;

    /* Pickup the first socket.  */
    socket_ptr = ip_ptr->nx_ip_tcp_created_sockets_ptr;

    /* Loop through the created sockets. */
    while (sockets--) {
        timeout = 0;
        /* Determine if a timeout is active. */
        if (socket_ptr->nx_tcp_socket_timeout || socket_ptr->nx_tcp_socket_linger_timeout) {
            aicx_debug(0x17c9db, "d", "[LOW_POWER] tcp socket timeout %d\r\n",
                      socket_ptr->nx_tcp_socket_timeout);
            if (socket_ptr->nx_tcp_socket_timeout)
                timeout = socket_ptr->nx_tcp_socket_timeout;

            if (socket_ptr->nx_tcp_socket_linger_timeout) {
                if (timeout && timeout > socket_ptr->nx_tcp_socket_linger_timeout)
                    timeout = socket_ptr->nx_tcp_socket_linger_timeout;
                else if (!timeout)
                    timeout = socket_ptr->nx_tcp_socket_linger_timeout;
            }

            if (!timeout)
                AIC_ASSERT(FALSE);

            if (!min_timeout || (timeout < min_timeout))
                min_timeout = timeout;
        }

        /* Move to the next TCP socket.  */
        socket_ptr =  socket_ptr->nx_tcp_socket_created_next;
    }
    AOS_INTERRUPT_RESTORE

    return min_timeout;
}


static int aic_network_suspend_check_tcp(struct aic_network_ip_instance *
                                         ip_instance)
{
#ifdef NX_ENABLE_TCP_KEEPALIVE
    NX_TCP_SOCKET *socket_ptr;
    UINT keepalive_enabled = NX_FALSE;
    NX_IP *ip_ptr = ip_instance->ip_ptr;
    ULONG sockets, min_keepalive_timeout = 0;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    /* Pickup the number of created TCP sockets. */
    sockets = ip_ptr->nx_ip_tcp_created_sockets_count;

    /* Pickup the first socket. */
    socket_ptr = ip_ptr->nx_ip_tcp_created_sockets_ptr;

    /* Get the keepalive status of the current socket. */
    if (socket_ptr)
        keepalive_enabled = socket_ptr->nx_tcp_socket_keepalive_enabled;

    /* Loop through the created sockets. */
    while ((sockets--) && socket_ptr) {
        /* Is keep alive enabled on this socket? */
        if (keepalive_enabled) {
            /* Yes; Check for the socket having a TCP Keepalive timer active. */
            if ((socket_ptr->nx_tcp_socket_state == NX_TCP_ESTABLISHED) &&
                (socket_ptr->nx_tcp_socket_keepalive_timeout)) {
                if (min_keepalive_timeout < socket_ptr->nx_tcp_socket_keepalive_timeout) {
                    min_keepalive_timeout = socket_ptr->nx_tcp_socket_keepalive_timeout;
                    aicx_debug(0x12a4ee, "d", "[LOW_POWER] tcp keepalive timeout %d\n",
                              min_keepalive_timeout);
                }
            }
        }

        /* Move to the next TCP socket.  */
        socket_ptr = socket_ptr->nx_tcp_socket_created_next;

        /* Get the socket's keep alive status. */
        keepalive_enabled = socket_ptr->nx_tcp_socket_keepalive_enabled;
    }
    AOS_INTERRUPT_RESTORE

    return min_keepalive_timeout;
#else
    return 0;
#endif
}

static int aic_ipv6_router_solicitation_check(struct aic_network_ip_instance *ip_instance)
{
    u32 i;
    NX_IP *ip_ptr = ip_instance->ip_ptr;

    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        if (ip_ptr->nx_ip_interface[i].nx_interface_valid == NX_TRUE) {
            if (ip_ptr->nx_ip_interface[i].nx_ipv6_rtr_solicitation_count != 0 &&
                ip_ptr ->nx_ip_interface[i].nxd_interface_ipv6_address_list_head) {
                aicx_debug(0x0e5917, "sdd", "[LOW_POWER] interface %s have rs send count %d timer %d\n",
                          ip_ptr->nx_ip_interface[i].nx_interface_name,
                          ip_ptr->nx_ip_interface[i].nx_ipv6_rtr_solicitation_count,
                          ip_ptr->nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer);
                return AIC_EBUSY;
            }
        }
    }

    return AIC_SUCCESS;
}

static int aic_network_suspend_check_fragment(struct aic_network_ip_instance *
                                              ip_instance)
{
    NX_PACKET *fragment;
    u32 reassembly_time = 0;
    NX_IP *ip_ptr = ip_instance->ip_ptr;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    fragment = ip_ptr->nx_ip_fragment_assembly_head;

    while (fragment) {
        if (!reassembly_time && fragment->nx_packet_reassembly_time)
            reassembly_time = fragment->nx_packet_reassembly_time;

        if (reassembly_time > fragment->nx_packet_reassembly_time)
            reassembly_time = fragment->nx_packet_reassembly_time;
        fragment = fragment->nx_packet_queue_next;
    }

    AOS_INTERRUPT_RESTORE

    return reassembly_time;
}

void aic_network_suspend(void *param)
{
    u32 fast_timer_period;
    struct aic_network_ip_instance *ip_instance;
    u32 timeout = 0, timeout_1 = 0, remain_time = 0, need_timeout;

    USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x1, 0, 0, 0);

    ip_instance = aic_get_default_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr)
        return;

    ip_instance->ip_ptr->nx_ip_low_power_stage = NX_SUSPEND_STAGE;
    fast_timer_period = (NX_IP_PERIODIC_RATE + (NX_IP_FAST_TIMER_RATE - 1)) /
                            NX_IP_FAST_TIMER_RATE;

    if (!aic_network_suspend_check_fast_icmpv6(ip_instance) &&
        !aic_network_suspend_check_fast_nd_cache(ip_instance) &&
        !aic_network_suspend_check_fast_tcp(ip_instance)) {
        timeout = aic_network_suspend_check_fast_tcp_timeout(ip_instance);

        need_timeout = timeout;
        if (!timeout) {
            aos_deactivate_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_fast_periodic_timer));
            ip_instance->ip_ptr->fast_timer_active = FALSE;

            ip_instance->delta_fast_timestamp = 0;
            ip_instance->suspend_fast_timestamp = 0;
        } else {
            ip_instance->suspend_fast_timestamp = aic_get_timestamp();
            aos_get_remaining_time((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_fast_periodic_timer),
                                   &remain_time);

            if (timeout > (fast_timer_period - remain_time))
                timeout = timeout - (fast_timer_period - remain_time);
            else
                timeout = 0;

            ip_instance->delta_fast_timestamp += ((fast_timer_period - remain_time) * 1000);
            aicx_debug(0x1dbd9a, "dd", "[LOW_POWER][SUSPEND] change fast_periodic_timer to %d ms delta_fast_timestamp %d us\r\n",
                      timeout, ip_instance->delta_fast_timestamp);

            if (timeout) {
                ip_instance->ip_ptr->fast_timer_active = FALSE;
                ip_instance->ip_ptr->nx_ip_fast_low_power_period = need_timeout;
                aos_change_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_fast_periodic_timer),
                                 timeout, AOS_NO_PERIOD);
            } else {
                aos_deactivate_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_fast_periodic_timer));
                ip_instance->ip_ptr->fast_timer_active = FALSE;
                ip_instance->active_event_flag |= AIC_NETWORK_FAST_WAKE_EVENT;
                aic_get_wakelock();
            }
        }

        USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x2, timeout,
                                     timeout_1, 0);
    } else {
        ip_instance->suspend_fast_timestamp = 0;
        ip_instance->delta_fast_timestamp = 0;
        USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x3, timeout,
                                     timeout_1, 0);
    }

    if (!aic_ipv6_router_solicitation_check(ip_instance)) {
        timeout = aic_network_suspend_check_tcp(ip_instance);
        timeout_1 = aic_network_suspend_check_fragment(ip_instance);
        if (timeout_1 && ((timeout > timeout_1) || !timeout))
            timeout = timeout_1;

        need_timeout = timeout;
        USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x4, timeout,
                                     timeout_1, 0);
        if (!timeout) {
            aos_deactivate_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_periodic_timer));
            ip_instance->ip_ptr->timer_active = FALSE;

            ip_instance->delta_timestamp = 0;
            ip_instance->suspend_timestamp = 0;
        } else {
            USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x5, timeout, timeout_1, 0);
            ip_instance->suspend_timestamp = aic_get_timestamp();
            aos_get_remaining_time((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_periodic_timer),
                                   &remain_time);

            ip_instance->delta_timestamp += ((NX_IP_PERIODIC_RATE - remain_time) * 1000);
            if ((timeout * NX_IP_PERIODIC_RATE) > (ip_instance->delta_timestamp / 1000))
                timeout = (timeout * NX_IP_PERIODIC_RATE) - (ip_instance->delta_timestamp / 1000);
            else
                timeout = 0;

            aicx_debug(0x195d29, "dd", "[LOW_POWER][SUSPEND] change periodic_timer to %d ms delta_timestamp %d us\r\n",
                      timeout, ip_instance->delta_timestamp);

            if (timeout) {
                ip_instance->ip_ptr->timer_active = FALSE;
                ip_instance->ip_ptr->nx_ip_periodic_low_power_period = need_timeout;
                aos_change_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_periodic_timer),
                                 timeout, AOS_NO_PERIOD);
            } else {
                aos_deactivate_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_periodic_timer));
                ip_instance->ip_ptr->timer_active = FALSE;
                ip_instance->active_event_flag |= AIC_NETWORK_NORMAL_WAKE_EVENT;
                aic_get_wakelock();
            }
        }
    } else {
        ip_instance->suspend_timestamp = 0;
        ip_instance->delta_timestamp = 0;
        USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x7, 0, 0, 0);
    }
}

void aic_network_resume(void *param)
{
    u32 fast_timer_period, timer_type = 0;
    struct aic_network_ip_instance *ip_instance;

    ip_instance = aic_get_default_ip_instance();
    if (!ip_instance || !ip_instance->ip_ptr)
        return;

    if (ip_instance->ip_ptr->nx_ip_low_power_stage != NX_SUSPEND_STAGE) {
        aicx_debug(0x01d17e, "d", "[LOW_POWER][RESUME] stage %d\n",
                  ip_instance->ip_ptr->nx_ip_low_power_stage);
        return;
    }

    if (param == &ip_instance->ip_ptr->nx_ip_fast_periodic_timer) {
        timer_type = AIC_FAST_TIMER_TYPE;
        aicx_debug(0x1b7f8d, "", "[LOW_POWER][RESUME] get fast timer type\n");
    } else if (param == &ip_instance->ip_ptr->nx_ip_periodic_timer) {
        timer_type = AIC_NORMAL_TIMER_TYPE;
        aicx_debug(0x070829, "", "[LOW_POWER][RESUME] get normal timer type\n");
    }

    ip_instance->ip_ptr->nx_ip_low_power_stage = NX_RESUME_STAGE;
    fast_timer_period = (NX_IP_PERIODIC_RATE + (NX_IP_FAST_TIMER_RATE - 1)) / NX_IP_FAST_TIMER_RATE;

    USER_TRACE_IN_LINE_INSERT_EX(TX_TRACE_NETWORK_EVENT, 0x6, ip_instance->active_event_flag,
                                 ip_instance->ip_ptr->timer_active,
                                 ip_instance->ip_ptr->fast_timer_active);

    if (!ip_instance->ip_ptr->timer_active) {
        ip_instance->ip_ptr->timer_active = TRUE;
        aos_change_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_periodic_timer),
                         NX_IP_PERIODIC_RATE, AOS_BE_PERIOD);
    }

    if (ip_instance->active_event_flag & AIC_NETWORK_NORMAL_WAKE_EVENT) {
        ip_instance->active_event_flag &= ~AIC_NETWORK_NORMAL_WAKE_EVENT;
        aic_put_wakelock();
    }

    if (ip_instance->suspend_timestamp) {
        ip_instance->delta_timestamp += (aic_get_timestamp() -
                                     ip_instance->suspend_timestamp);
        aicx_debug(0x06b3b0, "d", "[LOW_POWER][RESUME] delta_timestamp %d us\n",
                  ip_instance->delta_timestamp);
    }

    if (ip_instance->delta_timestamp) {
        if (timer_type == AIC_NORMAL_TIMER_TYPE) {
            if (ip_instance->delta_timestamp <
                (ip_instance->ip_ptr->nx_ip_periodic_low_power_period * (1000 * NX_IP_PERIODIC_RATE))) {
                aicx_debug(0x0bde5e, "dd", "[LOW_POWER][RESUME] delta_timestamp %d us periodic_low_power_period %d s\n",
                          ip_instance->delta_timestamp,
                          ip_instance->ip_ptr->nx_ip_periodic_low_power_period);
                ip_instance->delta_timestamp =
                    ip_instance->ip_ptr->nx_ip_periodic_low_power_period * (1000 * NX_IP_PERIODIC_RATE);
            }
        }

        ip_instance->ip_ptr->nx_ip_periodic_low_power_timer_rate +=
            ip_instance->delta_timestamp / 1000 / NX_IP_PERIODIC_RATE;
        if (ip_instance->ip_ptr->nx_ip_periodic_low_power_timer_rate) {
            aicx_debug(0x06fa86, "d", "[LOW_POWER][RESUME] periodic_low_power_timer_rate %d s\n",
                      ip_instance->ip_ptr->nx_ip_periodic_low_power_timer_rate);
            aos_set_event((aos_event_ptr *)&(ip_instance->ip_ptr->nx_ip_events),
                          NX_IP_PERIODIC_LOW_POWER_EVENT, AOS_OR);
        }

        ip_instance->delta_timestamp %= (1000 * NX_IP_PERIODIC_RATE);
    }

    ip_instance->suspend_timestamp = 0;

    if (!ip_instance->ip_ptr->fast_timer_active) {
        ip_instance->ip_ptr->fast_timer_active = TRUE;
        aicx_debug(0x142d78, "", "[LOW_POWER] active fast network timer\r\n");
        aos_change_timer((aos_timer_ptr *)&(ip_instance->ip_ptr->nx_ip_fast_periodic_timer),
                         fast_timer_period, AOS_BE_PERIOD);
    }

    if (ip_instance->active_event_flag & AIC_NETWORK_FAST_WAKE_EVENT) {
        ip_instance->active_event_flag &= ~AIC_NETWORK_FAST_WAKE_EVENT;
        aic_put_wakelock();
    }

    if (ip_instance->suspend_fast_timestamp) {
        ip_instance->delta_fast_timestamp += (aic_get_timestamp() -
                                     ip_instance->suspend_fast_timestamp);
        aicx_debug(0x124340, "d", "[LOW_POWER][RESUME] delta_fast_timestamp %d us\n",
                  ip_instance->delta_fast_timestamp);
    }

    if (ip_instance->delta_fast_timestamp) {
        if (timer_type == AIC_FAST_TIMER_TYPE) {
            if (ip_instance->delta_fast_timestamp <
                (ip_instance->ip_ptr->nx_ip_fast_low_power_period * 1000)) {
                aicx_debug(0x0b8d63, "dd", "[LOW_POWER][RESUME] delta_fast_timestamp %d us fast_low_power_period %d ms\n",
                          ip_instance->delta_fast_timestamp,
                          ip_instance->ip_ptr->nx_ip_fast_low_power_period);
                ip_instance->delta_fast_timestamp = ip_instance->ip_ptr->nx_ip_fast_low_power_period * 1000;
            }
        }

        ip_instance->ip_ptr->nx_ip_fast_low_power_timer_rate +=
            ip_instance->delta_fast_timestamp / 1000;
        if (ip_instance->ip_ptr->nx_ip_fast_low_power_timer_rate) {
            aicx_debug(0x109323, "d", "[LOW_POWER][RESUME] nx_ip_fast_low_power_timer_rate %d ms\n",
                      ip_instance->ip_ptr->nx_ip_fast_low_power_timer_rate);
            aos_set_event((aos_event_ptr *)&(ip_instance->ip_ptr->nx_ip_events),
                          NX_IP_FAST_LOW_POWER_EVENT, AOS_OR);
        }
        ip_instance->delta_fast_timestamp %= 1000;
    }

    ip_instance->suspend_fast_timestamp = 0;
    ip_instance->ip_ptr->nx_ip_fast_low_power_period = 0;
    ip_instance->ip_ptr->nx_ip_periodic_low_power_period = 0;
}

int aic_network_low_power_init(void)
{
    slp_register_sleep_hook_ex(aic_network_suspend, NULL,
                               aic_network_resume, NULL, LV_APP);

    return AIC_SUCCESS;
}
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_network_low_power_init);

