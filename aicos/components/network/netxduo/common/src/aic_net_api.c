
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x11506c
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <aipc_api.h>

#include "aic_net_api.h"
#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_tcp.h"
#include "nx_packet.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"
#include "nx_icmpv4.h"
#include "nx_icmpv6.h"

#ifdef CONFIG_NX_DNS
#include "nxd_dns.h"
#endif

#ifdef CONFIG_NX_NAT
#include "nx_nat.h"
#endif

#ifdef CONFIG_NET_PCAP_DUMP
#include "aic_network_pcap.h"
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
#include <aic_ipv6_forward.h>
#endif

#ifdef CONFIG_NET_SHARE_COMMON
#include <aic_net_share_common.h>
#endif

#include "aic_general_at_handle.h"

static IP_LINK_DRIVER aic_ip_driver_entry[] = {
#ifdef CONFIG_NET_ALTE_DRIVER
    nx_alte_nic_driver_entry,
#endif
#ifdef CONFIG_USB_NET_SHARE
    _ux_network_driver_entry,
#endif
#ifdef CONFIG_NET_IMS_NIC
    aic_ims_nic_driver_entry,
#endif
#ifdef CONFIG_NX_RAM_DRIVER
    _nx_ram_network_driver,
#endif
#ifdef CONFIG_PPP_DIAL
    nx_ppp_driver,
#endif
};

#if (CONFIG_PDP_MGR) && (CONFIG_NX_DNS)
static struct aic_packet_pool anet_dns_packet_pool = {0};
static NX_DNS *anet_dns_client;
static DNS_RESOLVE_CFG dns_resolve_cfg;
#ifdef NX_DNS_CACHE_ENABLE
static u8 *anet_dns_cache;
#endif
#endif

#ifdef CONFIG_PDP_MGR
extern struct aic_network_ip_instance alte_ip;
#endif

#ifdef CONFIG_IMS_NET_MGR
extern struct aic_network_ip_instance aims_ip;
#endif

#ifdef CONFIG_NET_SEMI_TETHER
struct aic_network_ip_instance usb_semi_tether_ip;
#endif

#ifdef CONFIG_NX_PPP
extern AIC_PPP_ECHO_CFG aic_ppp_echo_cfg;
#endif

#ifdef CONFIG_SUBSYS_AP
static void aic_network_monitor_keepalive(NX_TCP_SOCKET *socket_ptr, int is_recv)
{
    int ret;
    AMSG_T msg;

    if (aic_get_reg_mode() != AT_REG_LOW_POWER &&
        aic_get_reg_mode() != AT_REG_CORE_NET_LOW_POWER)
        return;

    aicx_debug(0x165ee9, "d", "[KEEPALIVE] recv a tcp keepalive ack %d\r\n", is_recv);
#ifdef CONFIG_NET_ALTE_DRIVER
    if (!is_recv && aic_get_reg_mode() == AT_REG_CORE_NET_LOW_POWER)
        nx_alte_notify_modem_release();
#endif

    if (is_recv && (aic_get_reg_mode() == AT_REG_LOW_POWER ||
                    aic_get_reg_mode() == AT_REG_CORE_NET_LOW_POWER)) {
        amsg_fill_msg(&msg, AMOD_EL2, AIC_NOTIFY_MODEM_RELEASE,
                      AIC_NOTIFY_MODEM_RELEASE);
        ret = amsg_send_msg(AIC_SYS_CP, &msg, 0);
        if (ret)
            aicx_warn(0x0ca2be, "x", "[KEEPALIVE] notify modem release failed 0x%x\r\n", ret);
    }
}
#endif

/**
 * aic_create_ip_instance() - Create IP Instance.
 * @aic_ip_ptr: AIC IP instance.
 *
 * CALLS
 *    nx_packet_pool_create: Create packet pool.
 *    nx_ip_create: Create netxduo ip instance.
 *
 * CALLED BY
 *    Application
 *
 * Create a netxduo ip instance. If the packet_pool member in the struct
 * aic_network_ip_instance structure is NULL, the packet pool will be created by
 * itself in this function.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_create_ip_instance(struct aic_network_ip_instance *aic_ip_ptr)
{
    int ret = AIC_SUCCESS;

    if (!aic_ip_ptr)
       return AIC_EINVAL;

    if (aic_ip_ptr->if_type >= AIC_MAX_NETWORK_LINK)
        return AIC_ENODEV;

    if (!aic_ip_ptr->ip_ptr) {
        aic_ip_ptr->ip_ptr = aos_malloc(sizeof(NX_IP));
        if (!aic_ip_ptr->ip_ptr) {
            return AIC_ENOMEM;
        }

        memset(aic_ip_ptr->ip_ptr, 0, sizeof(NX_IP));

        aic_ip_ptr->ip_owner = TRUE;
    }

    aic_ip_ptr->task_stack = aos_malloc(aic_ip_ptr->task_stack_size);
    if (!aic_ip_ptr->task_stack) {
        goto OUT_ERR1;
    }

    memset(aic_ip_ptr->task_stack, 0, aic_ip_ptr->task_stack_size);

    if (!aic_ip_ptr->packet_pool) {
        aic_ip_ptr->packet_pool = aos_malloc(sizeof(NX_PACKET_POOL));
        if (!aic_ip_ptr->packet_pool) {
            ret = AIC_ENOMEM;
            goto OUT_ERR2;
        }

        memset(aic_ip_ptr->packet_pool, 0, sizeof(NX_PACKET_POOL));

        ret = nx_packet_pool_create(aic_ip_ptr->packet_pool,
                                    aic_ip_ptr->packet_pool_name,
                                    aic_ip_ptr->per_packet_size,
                                    aic_ip_ptr->packet_pool_buf,
                                    aic_ip_ptr->packet_pool_size);
        if (ret != NX_SUCCESS) {
            goto OUT_ERR4;
        }

        aic_ip_ptr->packet_owner = 1;
    } else {
        aic_ip_ptr->packet_owner = 0;
    }

    ret = nx_ip_create(aic_ip_ptr->ip_ptr , aic_ip_ptr->ip_name,
                       aic_ip_ptr->default_ip_address,
                       aic_ip_ptr->default_net_mask, aic_ip_ptr->packet_pool,
                       aic_ip_driver_entry[aic_ip_ptr->if_type],
                       aic_ip_ptr->task_stack,
                       aic_ip_ptr->task_stack_size, aic_ip_ptr->pri);
    if (ret != NX_SUCCESS) {
        goto OUT_ERR5;
    }

#ifdef CONFIG_NET_LOW_POWER
    aic_ip_ptr->ip_ptr->nx_ip_resume = aic_network_resume;
    aic_ip_ptr->ip_ptr->nx_ip_suspend = aic_network_suspend;
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
    if(!aic_boot_mode_is_basic()) {
        aic_ip_ptr->ip_ptr->nx_ipv6_packet_router = aic_ipv6_packet_router;
        aic_ip_ptr->ip_ptr->nx_ipv6_forward_table_timeout = aic_ipv6_forward_table_timeout;
        aic_ip_ptr->ip_ptr->ipv6_forward_release_packet = aic_ipv6_forward_release_packet;
    }
#endif

#ifdef CONFIG_NET_SHARE_COMMON
    if(!aic_boot_mode_is_basic())
        aic_ip_ptr->ip_ptr->get_wan_intf_index = aic_get_wan_intf_index;
#endif

    if (aic_ip_ptr->bsd_owner) {
        if (!aic_ip_ptr->bsd_task_stack_size) {
            ret =  AIC_ENOMEM;
            goto OUT_ERR5;
        }

        aic_ip_ptr->bsd_task_stack = aos_malloc(aic_ip_ptr->bsd_task_stack_size);
        if (!aic_ip_ptr->bsd_task_stack) {
            ret =  AIC_ENOMEM;
            goto OUT_ERR5;
        }

        memset(aic_ip_ptr->bsd_task_stack, 0, aic_ip_ptr->bsd_task_stack_size);

        ret = bsd_initialize(aic_ip_ptr->ip_ptr, aic_ip_ptr->packet_pool,
                             aic_ip_ptr->bsd_task_stack,
                             aic_ip_ptr->bsd_task_stack_size,
                             aic_ip_ptr->bsd_task_pri);
        if (ret != NX_SUCCESS) {
            goto OUT_ERR6;
        }

    }

#ifdef NX_ENABLE_DUAL_PACKET_POOL
    if (!aic_ip_ptr->auxiliary_packet_pool) {
        aic_ip_ptr->auxiliary_packet_pool = aos_malloc(sizeof(NX_PACKET_POOL));
        if (aic_ip_ptr->auxiliary_packet_pool) {
            memset(aic_ip_ptr->auxiliary_packet_pool, 0,
                   sizeof(NX_PACKET_POOL));

            ret = nx_packet_pool_create(aic_ip_ptr->auxiliary_packet_pool,
                                        "auxiliary packet pool",
                                        ANET_AUX_PACKET_SIZE,
                                        NULL, ANET_AUX_PACKET_POOL_SIZE);
            if (!ret) {
                nx_ip_auxiliary_packet_pool_set(aic_ip_ptr->ip_ptr,
                                                aic_ip_ptr->auxiliary_packet_pool);
            } else {
                aos_free(aic_ip_ptr->auxiliary_packet_pool);
            }
        }
    }
#endif

#ifdef NX_ENABLE_MULTIPACKET_POOL
    if (!aic_ip_ptr->multipacket_pool) {
        aic_ip_ptr->multipacket_pool = aos_malloc(sizeof(NX_PACKET_POOL));
        if (aic_ip_ptr->multipacket_pool) {
            memset(aic_ip_ptr->multipacket_pool, 0,
                   sizeof(NX_PACKET_POOL));

            ret = nx_packet_pool_create(aic_ip_ptr->multipacket_pool,
                                        "multipacket pool",
                                        ANET_MULTIPACKET_SIZE,
                                        NULL, ANET_MULTIPACKET_POOL_SIZE);
            if (!ret) {
                nx_ip_multipacket_pool_set(aic_ip_ptr->ip_ptr,
                                           aic_ip_ptr->multipacket_pool);
            }
        }
    }
#endif

#ifdef CONFIG_SUBSYS_AP
#ifdef NX_ENABLE_TCP_KEEPALIVE
    aic_ip_ptr->ip_ptr->nx_ip_tcp_keepalive_ack_notify = aic_network_monitor_keepalive;
#endif
#endif

    aic_ip_ptr->ip_ptr->timer_active = TRUE;
    aic_ip_ptr->ip_ptr->fast_timer_active = TRUE;

    return AIC_SUCCESS;
OUT_ERR6:
    aos_free(aic_ip_ptr->bsd_task_stack);
    aic_ip_ptr->bsd_task_stack = NULL;
OUT_ERR5:
    nx_packet_pool_delete(aic_ip_ptr->packet_pool);
OUT_ERR4:
    if (aic_ip_ptr->packet_owner)
        aic_ip_ptr->packet_pool_buf = NULL;
    if (aic_ip_ptr->packet_owner) {
        aos_free(aic_ip_ptr->packet_pool);
        aic_ip_ptr->packet_pool = NULL;
    }
OUT_ERR2:
    aos_free(aic_ip_ptr->task_stack);
    aic_ip_ptr->task_stack = NULL;
OUT_ERR1:
    if (aic_ip_ptr->ip_owner)
        aos_free(aic_ip_ptr->ip_ptr);
    aic_ip_ptr->ip_ptr = NULL;

    aic_ip_ptr->packet_owner = 0;
    aic_ip_ptr->bsd_owner = 0;
    aic_ip_ptr->ip_owner = 0;

    return ret;
}

/**
 * aic_delete_ip_instance() - Delete IP Instance.
 * @aic_ip_ptr: AIC IP instance.
 *
 * CALLED BY
 *    Application
 *
 * Delete a netxduo ip instance. and release the associated memory.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_delete_ip_instance(struct aic_network_ip_instance *aic_ip_ptr)
{
    if (aic_ip_ptr->bsd_owner) {
        return AIC_EBUSY;
    }

    if (aic_ip_ptr->ip_ptr)
        nx_ip_delete(aic_ip_ptr->ip_ptr);

    if (aic_ip_ptr->packet_owner) {
        nx_packet_pool_delete(aic_ip_ptr->packet_pool);
        if (aic_ip_ptr->packet_pool_buf) {
            aic_ip_ptr->packet_pool_buf = NULL;
        }
        if (aic_ip_ptr->packet_pool) {
            aos_free(aic_ip_ptr->packet_pool);
            aic_ip_ptr->packet_pool = NULL;
        }
    }

#ifdef NX_ENABLE_DUAL_PACKET_POOL
    if (aic_ip_ptr->auxiliary_packet_pool) {
        nx_packet_pool_delete(aic_ip_ptr->auxiliary_packet_pool);
        if (aic_ip_ptr->auxiliary_packet_pool) {
            aos_free(aic_ip_ptr->auxiliary_packet_pool);
            aic_ip_ptr->auxiliary_packet_pool = NULL;
        }
    }
#endif
#ifdef NX_ENABLE_MULTIPACKET_POOL
    if (aic_ip_ptr->multipacket_pool) {
        nx_packet_pool_delete(aic_ip_ptr->multipacket_pool);
        if (aic_ip_ptr->multipacket_pool) {
            aos_free(aic_ip_ptr->multipacket_pool);
            aic_ip_ptr->multipacket_pool = NULL;
        }
    }
#endif

#ifndef CONFIG_DISABLE_IPV4_ARP
    if (aic_ip_ptr->arp_cache_mem) {
        aos_free(aic_ip_ptr->arp_cache_mem);
        aic_ip_ptr->arp_cache_mem = NULL;
    }
#endif

    if (aic_ip_ptr->task_stack) {
        aos_free(aic_ip_ptr->task_stack);
        aic_ip_ptr->task_stack = NULL;
    }

    if (aic_ip_ptr->bsd_task_stack) {
        aos_free(aic_ip_ptr->bsd_task_stack);
        aic_ip_ptr->bsd_task_stack = NULL;
    }

    if (aic_ip_ptr->ip_ptr) {
        aos_free(aic_ip_ptr->ip_ptr);
        aic_ip_ptr->ip_ptr = NULL;
    }

    return AIC_SUCCESS;
}

#ifdef CONFIG_NX_PPP
/**
 * aic_create_ppp() - Create PPP.
 * @aic_ppp: AIC PPP structure.
 *
 * CALLS
 *    nx_packet_pool_create: Create packet pool.
 *    nx_ppp_create: Create nx ppp.
 *
 * CALLED BY
 *    Application
 *
 * Create a nx ppp. If the packet_pool member in the struct aic_network_ppp is
 * NULL, the packet pool will be created by itself in this function.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_create_ppp(struct aic_network_ppp *aic_ppp)
{
    int ret = AIC_SUCCESS;

    if (!aic_ppp || aic_ppp->nx_ppp || aic_ppp->stack_memory_ptr) {
        aicx_error(0x092d9a, "", "aic ppp may be created\n");
        return AIC_EBUSY;
    }

    if (!aic_ppp->nx_ip_ptr) {
        aic_ppp->nx_ip_ptr = aos_malloc(sizeof(NX_IP));
        memset(aic_ppp->nx_ip_ptr, 0, sizeof(NX_IP));
        if (!aic_ppp->nx_ip_ptr) {
            aicx_error(0x05ac35, "", "alloc NX_PPP IP mem fail\n");
            return AIC_ENOMEM;
        }

        aic_ppp->ip_owner = TRUE;
    }

    aic_ppp->nx_ppp = aos_malloc(sizeof(NX_PPP));
    memset(aic_ppp->nx_ppp, 0, sizeof(NX_PPP));
    if (!aic_ppp->nx_ppp) {
        aicx_error(0x1e5836, "", "alloc NX_PPP mem fail\n");
        if (aic_ppp->ip_owner)
            aos_free(aic_ppp->nx_ip_ptr);
        return AIC_ENOMEM;
    }

    aic_ppp->stack_memory_ptr = aos_malloc(aic_ppp->stack_size);
    memset(aic_ppp->stack_memory_ptr, 0, aic_ppp->stack_size);
    if (!aic_ppp->stack_memory_ptr) {
        aicx_error(0x1dfd80, "", "alloc ppp stack_memory_ptr fail\n");
        goto OUT_ERR1;
    }

    if (!aic_ppp->packet_pool) {
        aic_ppp->packet_pool = aos_malloc(sizeof(NX_PACKET_POOL));
        memset(aic_ppp->packet_pool, 0, sizeof(NX_PACKET_POOL));
        if (!aic_ppp->packet_pool) {
            aicx_error(0x115b93, "s", "%s alloc packet pool structure fail\n",
                       aic_ppp->packet_pool_name);
            ret = AIC_ENOMEM;
            goto OUT_ERR2;
        }

        ret = nx_packet_pool_create(aic_ppp->packet_pool,
                                    aic_ppp->packet_pool_name,
                                    aic_ppp->per_packet_size,
                                    aic_ppp->packet_pool_buf,
                                    aic_ppp->packet_pool_size);
        if (ret != NX_SUCCESS) {
            aicx_error(0x1e67ba, "s", "%s packet pool create fail\n",
                       aic_ppp->packet_pool_name);
            goto OUT_ERR4;
        }

        aic_ppp->packet_owner = 1;
    } else {
        aic_ppp->packet_owner = 0;
    }

    ret = nx_ppp_create(aic_ppp->nx_ppp, aic_ppp->nx_ppp_name,
                        aic_ppp->nx_ip_ptr, aic_ppp->stack_memory_ptr,
                        aic_ppp->stack_size, aic_ppp->thread_priority,
                        aic_ppp->packet_pool,
                        aic_ppp->ppp_non_ppp_packet_handler,
                        aic_ppp->ppp_byte_send);
    if (ret) {
        aicx_error(0x123328, "xxxxx", "[API] nx ppp create fail 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                   ret, aic_ppp->nx_ppp->nx_ppp_id, (u32)aic_ppp->nx_ip_ptr,
                   (u32)aic_ppp->stack_memory_ptr, (u32)aic_ppp->packet_pool);
        goto OUT_ERR5;
    }

    return ret;

OUT_ERR5:
    if (aic_ppp->packet_owner)
        nx_packet_pool_delete(aic_ppp->packet_pool);

OUT_ERR4:
    if (aic_ppp->packet_owner) {
        aic_ppp->packet_pool_buf = NULL;
    }

    if (aic_ppp->packet_owner) {
        aos_free(aic_ppp->packet_pool);
        aic_ppp->packet_pool = NULL;
    }
OUT_ERR2:
    aos_free(aic_ppp->stack_memory_ptr);
    aic_ppp->stack_memory_ptr = NULL;
OUT_ERR1:
    aos_free(aic_ppp->nx_ppp);
    aic_ppp->nx_ppp = NULL;

    if (aic_ppp->ip_owner)
        aos_free(aic_ppp->nx_ip_ptr);

    aic_ppp->packet_owner = 0;
    aic_ppp->ip_owner = 0;

    return ret;
}

/**
 * aic_delete_ppp() - Delete nx ppp.
 * @aic_ppp: AIC PPP.
 *
 * CALLED BY
 *    Application
 *
 * Delete a netxduo PPP. and release the associated memory.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_delete_ppp(struct aic_network_ppp *aic_ppp)
{
    int ret;

    if (aic_ppp->packet_owner) {
        nx_packet_pool_delete(aic_ppp->packet_pool);
        if (aic_ppp->packet_pool)
            aos_free(aic_ppp->packet_pool);
    }

    if (aic_ppp->stack_memory_ptr)
        aos_free(aic_ppp->stack_memory_ptr);

    ret = nx_ppp_delete(aic_ppp->nx_ppp);
    if (ret) {
        aicx_error(0x07d34e, "x", "nx ppp delete fail 0x%x\n", ret);
        return ret;
    }

    aos_free(aic_ppp->nx_ppp);

    return AIC_SUCCESS;
}

void aic_set_ppp_echo_cfg(AIC_PPP_ECHO_CFG *echo_cfg)
{
    aic_ppp_echo_cfg.echo_enabled = echo_cfg->echo_enabled;
    if (!echo_cfg->echo_enabled)
        return;

    if (echo_cfg->echo_interval > 0)
        aic_ppp_echo_cfg.echo_interval = echo_cfg->echo_interval;

    if (echo_cfg->retry_times > 0)
        aic_ppp_echo_cfg.retry_times = echo_cfg->retry_times;

    if (echo_cfg->timeout_one_shot > 0)
        aic_ppp_echo_cfg.timeout_one_shot = echo_cfg->timeout_one_shot;
}
#endif

/**
 * aic_create_packet_pool() - Create packet pool.
 * @aic_pool: AIC packet pool pointer.
 *
 * CALLED BY
 *    Application
 *
 * Create a packet pool, and apply for the required memory from the heap
 * interval.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_create_packet_pool(struct aic_packet_pool *aic_pool)
{
    u32 ret;

    aic_pool->packet_pool = aos_malloc(sizeof(NX_PACKET_POOL));
    if (!aic_pool->packet_pool) {
        return AIC_ENOMEM;
    }

    memset(aic_pool->packet_pool, 0, sizeof(NX_PACKET_POOL));

    ret = nx_packet_pool_create(aic_pool->packet_pool,
                                aic_pool->packet_pool_name,
                                aic_pool->per_packet_size,
                                aic_pool->packet_pool_buf,
                                aic_pool->packet_pool_size);
    if (ret != NX_SUCCESS) {
        aos_free(aic_pool->packet_pool);
        return AIC_ENOMEM;
    }

    return AIC_SUCCESS;
}

/**
 * aic_delete_packet_pool() - Delete packet pool.
 * @aic_pool: AIC packet pool pointer.
 *
 * CALLED BY
 *    Application
 *
 * Create a packet pool, and release the associated memory.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_delete_packet_pool(struct aic_packet_pool *aic_pool)
{
    int ret = AIC_SUCCESS;

    if (aic_pool->packet_pool) {
        ret = nx_packet_pool_delete(aic_pool->packet_pool);
        if (ret != NX_SUCCESS) {
            return ret;
        }
        aos_free(aic_pool->packet_pool);
    }

    return ret;
}

#ifdef CONFIG_NX_NAT
/**
 * aic_nat_create() - Create NAT.
 * @nat_device: NX_NAT_DEVICE pointer.
 * @aic_ip_ptr: AIC ip instance pointer.
 * @global_interface_index: interface index.
 *
 * CALLS:
 *    nx_nat_create
 *
 * CALLED BY:
 *    Application
 *
 * Create NAT, and apply for the required memory from the heap
 * interval.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_nat_create(void *nat_device, struct aic_network_ip_instance *aic_ip_ptr,
                   u32 global_interface_index)
{
    int ret;

    if (!nat_device || !aic_ip_ptr || !aic_ip_ptr->nat_cache_mem_size ||
        !aic_ip_ptr->ip_ptr ||
        aic_ip_ptr->nat_cache_mem[global_interface_index]) {
        return AIC_EINVAL;
    }

    aic_ip_ptr->nat_cache_mem[global_interface_index] =
        aos_malloc(aic_ip_ptr->nat_cache_mem_size);
    if (!aic_ip_ptr->nat_cache_mem[global_interface_index]) {
        return AIC_ENOMEM;
    }

    memset(aic_ip_ptr->nat_cache_mem[global_interface_index], 0,
           aic_ip_ptr->nat_cache_mem_size);

    ret = nx_nat_create(nat_device, aic_ip_ptr->ip_ptr, global_interface_index,
                        aic_ip_ptr->nat_cache_mem[global_interface_index],
                        aic_ip_ptr->nat_cache_mem_size);
    if (ret != NX_SUCCESS) {
        aos_free(aic_ip_ptr->nat_cache_mem[global_interface_index]);
        aic_ip_ptr->nat_cache_mem[global_interface_index] = NULL;
        return ret;
    }

    return AIC_SUCCESS;
}

/**
 * aic_nat_delete() - Delete NAT.
 * @nat_device: NX_NAT_DEVICE pointer.
 * @aic_ip_ptr: AIC ip instance pointer.
 *
 * CALLS:
 *    nx_nat_delete
 *
 * CALLED BY:
 *    Application
 *
 * Delete NAT, and release the associated memory.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
void aic_nat_delete(void *nat_dev, struct aic_network_ip_instance *aic_ip_ptr)
{
    NX_NAT_DEVICE *nat_dev_ptr = nat_dev;

    if (aic_ip_ptr->nat_cache_mem[nat_dev_ptr->nx_nat_global_interface_index]) {
        aos_free(aic_ip_ptr->nat_cache_mem[nat_dev_ptr->nx_nat_global_interface_index]);
        aic_ip_ptr->nat_cache_mem[nat_dev_ptr->nx_nat_global_interface_index] = NULL;
    }

    nx_nat_delete(nat_dev_ptr->nx_nat_global_interface_index);
}
#endif

#ifndef CONFIG_DISABLE_IPV4_ARP
/**
 * aic_enable_arp() - Enable arp mechanism for IP instance.
 * @aic_ip_ptr: AIC ip instance pointer.
 *
 * CALLS:
 *    nx_arp_enable
 *
 * CALLED BY:
 *    Application
 *
 * Enable arp mechanism for IP instance, and apply for the required memory
 * from the heap interval.
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_enable_arp(struct aic_network_ip_instance *aic_ip_ptr)
{
    int ret;

    if (!aic_ip_ptr || !aic_ip_ptr->arp_cache_mem_size || !aic_ip_ptr->ip_ptr) {
        return AIC_EINVAL;
    }

    aic_ip_ptr->arp_cache_mem = aos_malloc(aic_ip_ptr->arp_cache_mem_size);
    if (!aic_ip_ptr->arp_cache_mem) {
        return AIC_ENOMEM;
    }

    memset(aic_ip_ptr->arp_cache_mem, 0,
           sizeof(aic_ip_ptr->arp_cache_mem_size));

    ret = nx_arp_enable(aic_ip_ptr->ip_ptr,
                        aic_ip_ptr->arp_cache_mem,
                        aic_ip_ptr->arp_cache_mem_size);
    if (ret != NX_SUCCESS) {
        return ret;
    }

    return AIC_SUCCESS;
}

int aic_register_arp_age_notify(struct aic_network_ip_instance *aic_ip_ptr,
                                void (*nx_ip_arp_age_notify)(NX_INTERFACE *nx_interface, ULONG ip_address))
{
    if (!aic_ip_ptr || !aic_ip_ptr->arp_cache_mem_size || !aic_ip_ptr->ip_ptr) {
        return AIC_EINVAL;
    }

    aic_ip_ptr->ip_ptr->nx_ip_arp_age_notify = nx_ip_arp_age_notify;

    return AIC_SUCCESS;
}

/**
 * aic_free_arp_cache() - Free arp cache mem.
 * @aic_ip_ptr: AIC ip instance pointer.
 *
 * CALLS:
 *
 * CALLED BY:
 *    Application
 *
 * Return: void.
 *
 */
void aic_free_arp_cache(struct aic_network_ip_instance *aic_ip_ptr)
{
    if (aic_ip_ptr->arp_cache_mem) {
        aos_free(aic_ip_ptr->arp_cache_mem);
        aic_ip_ptr->arp_cache_mem = NULL;
    }
}
#endif

/**
 * aic_get_ipv6_interface_index() - Get ipv6 address inteface index.
 * @aic_ip_ptr: AIC ip instance pointer.
 * @ip_address: struct aic_network_address pointer.
 * @interface_index: Points to the variable holding the interface index.
 *
 * Get ipv6 address interface index, in host byte order.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_get_ipv6_addr_index(struct aic_network_ip_instance *aic_ip_ptr,
                            void *ip_address, u32 *index)
{
    int i;
    NX_IP *ip_ptr;
    struct aic_network_address *aic_ip_addr =
        (struct aic_network_address *)ip_address;

    if (!aic_ip_ptr || !ip_address || !index) {
        return AIC_EINVAL;
    }

    ip_ptr = aic_ip_ptr->ip_ptr;
    if (!ip_ptr) {
        return AIC_EINVAL;
    }

    if (aic_ip_addr->ip_version != NX_IP_VERSION_V6) {
        return AIC_EINVAL;
    }

    for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
        if ((aic_ip_addr->ip_address.v6[0] ==
            ip_ptr->nx_ipv6_address[i].nxd_ipv6_address[0]) &&
            (aic_ip_addr->ip_address.v6[1] ==
            ip_ptr->nx_ipv6_address[i].nxd_ipv6_address[1]) &&
            (aic_ip_addr->ip_address.v6[2] ==
            ip_ptr->nx_ipv6_address[i].nxd_ipv6_address[2]) &&
            (aic_ip_addr->ip_address.v6[3] ==
            ip_ptr->nx_ipv6_address[i].nxd_ipv6_address[3])) {
            *index = i;;
            return AIC_SUCCESS;
        }
    }

    return AIC_ENODEV;
}

/**
 * aic_init_bsd() - bsd initialize.
 * @aic_ip_ptr: AIC ip instance pointer.
 *
 * Initialize BSD and apply for its required task stack.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
int aic_init_bsd(struct aic_network_ip_instance *aic_ip_ptr)
{
    int ret;

    if (!aic_ip_ptr) {
        return AIC_EINVAL;
    }

    if (!aic_ip_ptr->ip_ptr || !aic_ip_ptr->packet_pool) {
        return AIC_EFAULT;
    }

    if (!aic_ip_ptr->bsd_task_stack_size || aic_ip_ptr->bsd_task_stack) {
        return AIC_ENOMEM;
    }

    aic_ip_ptr->bsd_task_stack = aos_malloc(aic_ip_ptr->bsd_task_stack_size);
    if (!aic_ip_ptr->bsd_task_stack) {
        return AIC_ENOMEM;
    }

    memset(aic_ip_ptr->bsd_task_stack, 0, aic_ip_ptr->bsd_task_stack_size);

    ret = bsd_initialize(aic_ip_ptr->ip_ptr, aic_ip_ptr->packet_pool,
                         aic_ip_ptr->bsd_task_stack,
                         aic_ip_ptr->bsd_task_stack_size,
                         aic_ip_ptr->bsd_task_pri);
    if (ret != NX_SUCCESS) {
        return ret;
    }

    aic_ip_ptr->bsd_owner = 1;

    return AIC_SUCCESS;
}

#if (CONFIG_PDP_MGR) && (CONFIG_NX_DNS)
u32 create_dns_pkt_pool(NX_DNS *dns_client_ptr)
{
    u32 status = AIC_SUCCESS;

    anet_dns_packet_pool.per_packet_size = NX_DNS_PACKET_PAYLOAD;
    anet_dns_packet_pool.packet_pool_size = NX_DNS_PACKET_POOL_SIZE;
    anet_dns_packet_pool.packet_pool_name = "anet_dns_packet_pool";
    status = aic_create_packet_pool(&anet_dns_packet_pool);
    if (status) {
        return status;
    }

    /* Use the packet pool created above. */
     status = nx_dns_packet_pool_set(dns_client_ptr,
                                     anet_dns_packet_pool.packet_pool);
     if (status) {
         return status;
     }

    return status;
}

#ifdef NX_DNS_CACHE_ENABLE
void dns_cache_full_callback(NX_DNS *dns_client_ptr)
{
    if (dns_client_ptr) {
        aicx_info(0x1766d9, "", "dns cache full,clear it\n");
        aic_clear_dns_cache();
    }
}

void aic_clear_dns_cache(void)
{
    if (anet_dns_cache)
        nx_dns_cache_initialize(anet_dns_client, anet_dns_cache,
                                ANET_DNS_CACHE_SIZE);
}
#endif

void aic_init_user_dns_cfg(void)
{
#ifdef NX_DNS_CACHE_ENABLE
    dns_resolve_cfg.dns_cache_enabled = 1;
#else
    dns_resolve_cfg.dns_cache_enabled = 0;
#endif
}

/* Start to use DNS cache. */
void aic_enable_dns_cache(void)
{
    if (dns_resolve_cfg.dns_cache_enabled)
        return;

    dns_resolve_cfg.dns_cache_enabled = 1;
#ifdef NX_DNS_CACHE_ENABLE
    if ((anet_dns_client && anet_dns_client->nx_dns_id == NX_DNS_ID)) {
        if (!anet_dns_cache) {
            anet_dns_cache = aos_zalloc(ANET_DNS_CACHE_SIZE);
            AIC_ASSERT(anet_dns_cache);
        } else {
            aos_memset(anet_dns_cache, 0, ANET_DNS_CACHE_SIZE);
        }

        if (!nx_dns_cache_initialize(anet_dns_client, anet_dns_cache, ANET_DNS_CACHE_SIZE))
            nx_dns_cache_notify_set(anet_dns_client, dns_cache_full_callback);
    }
#endif
}

/* Stop to use DNS cache. */
void aic_disable_dns_cache(void)
{
    /* Already disabled. */
    if (!dns_resolve_cfg.dns_cache_enabled)
        return;

    dns_resolve_cfg.dns_cache_enabled = 0;
#ifdef NX_DNS_CACHE_ENABLE
    if ((anet_dns_client && anet_dns_client->nx_dns_id == NX_DNS_ID)) {
        if (anet_dns_cache) {
            aic_dns_set_no_cache(anet_dns_client);
            nx_dns_cache_notify_clear(anet_dns_client);
        }
    }
#endif
}

boolean aic_is_dns_cache_enabled(void)
{
    return dns_resolve_cfg.dns_cache_enabled ? AIC_TRUE : AIC_FALSE;
}

/** Get the shortest ttl of dns responses for the host name and ip type.
* host_name: string of host name
* ip_type: ANET_IP_TYPE_V4 or ANET_IP_TYPE_V6
* return the shortest ttl of all; return 0 if error.
*/
ulong aic_get_dns_cache_ttl(u8 *host_name, u8 ip_type)
{
    ulong ttl = 0;
    ulong buffer[10];
    u32 record_count = 0, ret = AIC_SUCCESS, i;
    u8 lookup_type;

    if (ip_type != ANET_IP_TYPE_V4 && ip_type != ANET_IP_TYPE_V6) {
        return 0;
    }

#ifdef NX_DNS_CACHE_ENABLE
    if (ip_type == ANET_IP_TYPE_V4)
        lookup_type = NX_DNS_RR_TYPE_A;
    else if (ip_type == ANET_IP_TYPE_V6)
        lookup_type = NX_DNS_RR_TYPE_AAAA;

    aos_memset(buffer, 0, sizeof(buffer));
    if (anet_dns_client && anet_dns_client->nx_dns_id == NX_DNS_ID &&
        dns_resolve_cfg.dns_cache_enabled) {
        /* Find ttl of the answers in local cache. */
        ret = aic_dns_cache_find_ttl(anet_dns_client, anet_dns_client->nx_dns_cache,
                                     host_name, (USHORT)lookup_type,
                                     (u8 *)buffer, sizeof(buffer), &record_count);
        if (!ret) {
            ttl = buffer[0];
            for (i = 0; i < record_count; i++) {
                /* Return the shortest ttl. */
                if (buffer[i] < ttl)
                    ttl = buffer[i];
            }
        }
    }
#endif

    return ttl;
}

static void save_user_dns_to_wan_net(ulong ipv4_dns1, ulong ipv4_dns2, ulong *ipv6_dns1, ulong *ipv6_dns2)
{
    WAN_NET *wan_net_ptr = AIC_NULL;
    u8 i, ip_type;

    wan_net_ptr = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (!wan_net_ptr)
        return;

    ip_type = aic_get_pdp_ip_type(ANET_DEFAULT_PDP_CID);
    if (ip_type == ANET_DEFAULT_IP_TYPE_V4 || ip_type == ANET_DEFAULT_IP_TYPE_V4V6) {
        if (ipv4_dns1)
            wan_net_ptr->wan_ipv4_dns1_address = ipv4_dns1;

        if (ipv4_dns2)
            wan_net_ptr->wan_ipv4_dns2_address = ipv4_dns2;
    }

    if (ip_type == ANET_DEFAULT_IP_TYPE_V6 || ip_type == ANET_DEFAULT_IP_TYPE_V4V6) {
        if (ipv6_dns1 && aic_is_valid_ipv6_addr(ipv6_dns1)) {
            for (i = 0; i < 4; i++)
                wan_net_ptr->wan_ipv6_dns1_address[i] = ipv6_dns1[i];
        }

        if (ipv6_dns2 && aic_is_valid_ipv6_addr(ipv6_dns2)) {
            for (i = 0; i < 4; i++)
                wan_net_ptr->wan_ipv6_dns2_address[i] = ipv6_dns2[i];
        }
    }
}

u32 aic_add_dns_servers(WAN_NET *wan_net_ptr)
{
    u32 status = AIC_SUCCESS;
    u8 i, pdp_cid;
    NXD_ADDRESS dns_ipv6_server;

    if (!anet_dns_client || !wan_net_ptr) {
        return status;
    }

    pdp_cid = wan_net_ptr->pdp_cid;
    if (wan_net_ptr->wan_ipv4_dns1_address != 0) {
        status = aic_add_dns_server_by_cid(anet_dns_client, pdp_cid,
                                           wan_net_ptr->wan_ipv4_dns1_address,
                                           AIC_NULL);
        if (status && status != NX_DNS_DUPLICATE_ENTRY)
            aicx_error(0x0c1cc8, "sx", "%s:add v4 server error 0x%x\n", __func__, status);
    }

    if (wan_net_ptr->wan_ipv4_dns2_address != 0) {
        aic_add_dns_server_by_cid(anet_dns_client, pdp_cid,
                                  wan_net_ptr->wan_ipv4_dns2_address, AIC_NULL);
    }

    /** Add an IPv6 DNS servers to the DNS client if the PDP has got
     * a global IPv6 address.
     */
    if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_dns1_address)) {
        dns_ipv6_server.nxd_ip_version = NX_IP_VERSION_V6;
        for (i = 0; i < 4; i++) {
            dns_ipv6_server.nxd_ip_address.v6[i] =
                wan_net_ptr->wan_ipv6_dns1_address[i];
        }

        status = aic_add_dns_server_by_cid(anet_dns_client, pdp_cid,
                                           0, &dns_ipv6_server);
        if (status && status != NX_DNS_DUPLICATE_ENTRY)
            aicx_error(0x167060, "sx", "%s:add v6 server error 0x%x\n", __func__, status);
    }

    if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_dns2_address)) {
        dns_ipv6_server.nxd_ip_version = NX_IP_VERSION_V6;
        for (i = 0; i < 4; i++) {
            dns_ipv6_server.nxd_ip_address.v6[i] =
                wan_net_ptr->wan_ipv6_dns2_address[i];
        }
        aic_add_dns_server_by_cid(anet_dns_client, pdp_cid, 0, &dns_ipv6_server);
    }

    return status;
}

/**
 * aic_init_dns_client() - Init DNS client to get ip addresses by host name.
 * It should be called before calling getaddrinfo().
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_init_dns_client(void)
{
    s32 status;
    WAN_NET *wan_net_ptr;
    struct aic_network_ip_instance *pdp_ip_instance;

    if (anet_dns_client && anet_dns_client->nx_dns_id == NX_DNS_ID) {
        return AIC_SUCCESS;
    } else {
        pdp_ip_instance = aic_get_pdp_ip_instance();
        if (!pdp_ip_instance->ip_ptr) {
            return ANET_ERROR_NO_IP_INSTANCE;
        }

        /* Create a DNS client instance. */
        if (!anet_dns_client) {
            anet_dns_client = aos_zalloc(sizeof(NX_DNS));
            AIC_ASSERT(anet_dns_client);
        }

        status = nx_dns_create(anet_dns_client,
                               (NX_IP *)pdp_ip_instance->ip_ptr,
                               "ANET_DNS_Client");

        if (status) {
            return status;
        }

#ifdef NX_DNS_CACHE_ENABLE
        if (dns_resolve_cfg.dns_cache_enabled) {
            /* Initialize the DNS cache. */
            if (!anet_dns_cache) {
                anet_dns_cache = aos_zalloc_relaxed(ANET_DNS_CACHE_SIZE);
                AIC_ASSERT(anet_dns_cache);
                status = nx_dns_cache_initialize(anet_dns_client, anet_dns_cache,
                                                 ANET_DNS_CACHE_SIZE);
                if (!status)
                    /* Register the notify callback of full cache. */
                    nx_dns_cache_notify_set(anet_dns_client, dns_cache_full_callback);
            }
        }
#endif

#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
        /** Create the packet pool for the DNS Client to send packets.
         * Then it won't be created when nx_dns_create is called.
         */
        if (!anet_dns_packet_pool.packet_pool) {
            status = create_dns_pkt_pool(anet_dns_client);
            if (status)
                return status;
        }
#endif

        wan_net_ptr = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
        if (wan_net_ptr) {
            /* Check if user set dns servers. */
            save_user_dns_to_wan_net(dns_resolve_cfg.ipv4_dns_address1,
                                     dns_resolve_cfg.ipv4_dns_address2,
                                     dns_resolve_cfg.ipv6_dns_address1,
                                     dns_resolve_cfg.ipv6_dns_address2);
            aic_add_dns_servers(wan_net_ptr);
        }
    }

    return AIC_SUCCESS;
}

/** Pdp may be deactivated and activated again,then should update dns servers,
 * if DNS client has already created.
 */
void aic_update_dns_servers(WAN_NET *wan_net_ptr)
{
    if (anet_dns_client && wan_net_ptr) {
        /* Check if user set dns servers. */
        if (wan_net_ptr->pdp_cid == ANET_DEFAULT_PDP_CID)
            save_user_dns_to_wan_net(dns_resolve_cfg.ipv4_dns_address1,
                                     dns_resolve_cfg.ipv4_dns_address2,
                                     dns_resolve_cfg.ipv6_dns_address1,
                                     dns_resolve_cfg.ipv6_dns_address2);

        aic_remove_dns_server_by_cid(anet_dns_client, wan_net_ptr->pdp_cid);
        aic_add_dns_servers(wan_net_ptr);
    }
}

void *aic_get_dns_client(void)
{
    return anet_dns_client;
}

/* Delete DNS client and free all memory malloced. */
void aic_delete_dns_client(void)
{
#ifdef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    aic_delete_packet_pool(&anet_dns_packet_pool);
    aos_memset((u8 *)&anet_dns_packet_pool, 0, sizeof(struct aic_packet_pool));
#endif

#ifdef NX_DNS_CACHE_ENABLE
    if (anet_dns_cache) {
        aos_free(anet_dns_cache);
        anet_dns_cache = AIC_NULL;
    }
#endif

    if (anet_dns_client) {
        nx_dns_delete(anet_dns_client);
        aos_free(anet_dns_client);
        anet_dns_client = AIC_NULL;
    }
    aicx_debug(0x1b6989, "s", "%s:delete dns client\n", __func__);
}

/* Add for users to set certain dns servers.
* ip_type: ANET_IP_TYPE_V4 or ANET_IP_TYPE_V6
*/
int aic_set_dns_server_ip(u8 ip_type, u8 *dns_server1, u8 *dns_server2)
{
    u8 ip_num[4] = {0, 0, 0, 0};
    WAN_NET *wan_net_ptr = AIC_NULL;
    u8 i;
    boolean dns_set_flag = AIC_FALSE;

    if (dns_server1)
        aicx_info(0x06c1ce, "s", "dns sever1 set by user %s\n", dns_server1);

    if (ip_type == ANET_IP_TYPE_V4) {
        if (dns_server1 && strlen(dns_server1) > 0) {
            dns_set_flag = AIC_TRUE;
            get_numeric_ipv4_addr(ip_num, dns_server1);
            dns_resolve_cfg.ipv4_dns_address1 =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
        }

        aos_memset(ip_num, 0, sizeof(ip_num));
        if (dns_server2 && strlen(dns_server2) > 0) {
            dns_set_flag = AIC_TRUE;
            get_numeric_ipv4_addr(ip_num, dns_server2);
            dns_resolve_cfg.ipv4_dns_address2 =
                IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);
        }

        if (dns_set_flag)
            goto UPDATE_DNS;
    } else if (ip_type == ANET_IP_TYPE_V6) {
        if (dns_server1 && strlen(dns_server1) > 0) {
            dns_set_flag = AIC_TRUE;
            inet_pton(AF_INET6, dns_server1, dns_resolve_cfg.ipv6_dns_address1);
            for (i = 0; i < 4; i++)
                NX_CHANGE_ULONG_ENDIAN(dns_resolve_cfg.ipv6_dns_address1[i]);
        }

        if (dns_server2 && strlen(dns_server2) > 0) {
            dns_set_flag = AIC_TRUE;
            inet_pton(AF_INET6, dns_server2, dns_resolve_cfg.ipv6_dns_address2);
            for (i = 0; i < 4; i++)
                NX_CHANGE_ULONG_ENDIAN(dns_resolve_cfg.ipv6_dns_address2[i]);
        }

        if (dns_set_flag)
            goto UPDATE_DNS;
    } else {
        return ANET_ERROR_INVALID_IP_TYPE;
    }

UPDATE_DNS:
    wan_net_ptr = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    /* Save them in WAN_NET. */
    if (wan_net_ptr) {
        save_user_dns_to_wan_net(dns_resolve_cfg.ipv4_dns_address1,
                                 dns_resolve_cfg.ipv4_dns_address2,
                                 dns_resolve_cfg.ipv6_dns_address1,
                                 dns_resolve_cfg.ipv6_dns_address2);
        aic_update_dns_servers(wan_net_ptr);
    }

    return AIC_SUCCESS;
}

/* Set the timeout for one try in seconds and total retry count. */
void aic_set_dns_timeout(u8 timeout_once_seconds, u8 max_retry_cnt)
{
    dns_resolve_cfg.timeout_one_shot = timeout_once_seconds * 1000;
    dns_resolve_cfg.max_retry_times = max_retry_cnt;
}

u32 aic_get_dns_timeout_once(void)
{
    if (dns_resolve_cfg.timeout_one_shot > 0)
        return dns_resolve_cfg.timeout_one_shot;
    else
        return NX_DNS_QUERY_ONCE_TIMEOUT;
}

u8 aic_get_dns_max_retry_times(void)
{
    if (dns_resolve_cfg.max_retry_times > 0)
        return dns_resolve_cfg.max_retry_times;
    else
        return NX_DNS_DEFAULT_RETRY_TIMES;
}
#endif

#ifdef CONFIG_PDP_MGR
/**
 * aic_tcp_client_source_ip_bind() - bind source ip by cid.
 * @socket_ptr: socket ptr.
 * @cid: Context id.
 * @sa_family: source ip type.AF_INET or AF_INET6.
 * @wait_option: Suspension option​.
 *
 * CALLED BY
 *    Application
 *
 * Return: AIC_SUCCESS for success, others for fail.
 *
 */
u32 aic_tcp_client_source_ip_bind(NX_TCP_SOCKET *socket_ptr, u8 cid,
                                  u16 sa_family, ulong wait_option)
{
    WAN_NET *pdp_info = AIC_NULL;
    int if_index, i;
    NXD_IPV6_ADDRESS *dest_v6;

    pdp_info = aic_get_wan_net_per_pdp(cid);
    if (pdp_info == AIC_NULL) {
        return AIC_GET_WAN_NET_FAIL;
    }

    if_index = pdp_info->wan_ifc_index;

    if (sa_family == AF_INET) {
        /* Pick up the ipv4 insterfice */
        if (pdp_info->wan_ipv4_address == 0) {
            return NX_INVALID_INTERFACE;
        }
        socket_ptr->nx_tcp_socket_connect_interface =
        &(pdp_info->wan_ip_instance->nx_ip_interface[if_index]);
    } else if (sa_family == AF_INET6) {
        /* Pick up the ipv6 insterfice */

        for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++) {
            dest_v6 = &(pdp_info->wan_ip_instance->nx_ipv6_address[i]);
            /* Compare the address. */
            if ((dest_v6->nxd_ipv6_address_valid) &&
                (CHECK_IPV6_ADDRESSES_SAME(dest_v6->nxd_ipv6_address,
                                           pdp_info->wan_ipv6_address1))) {
                 /* Found a proper address. */
                 socket_ptr->nx_tcp_socket_connect_interface =
                                            dest_v6->nxd_ipv6_address_attached;

                 socket_ptr->nx_tcp_socket_ipv6_addr = dest_v6;
                 break;
            }
        }

        if (i == NX_MAX_IPV6_ADDRESSES) {
            return NX_INVALID_INTERFACE;
        }
    }

    return AIC_SUCCESS;
}
#endif

int aic_net_check_fragment_packet(NX_PACKET *packet_ptr)
{
    u32 tmp, protocol;
    NX_IPV4_HEADER *ip_header;
    NX_IPV6_HEADER *ipv6_header;
    u16 frag_flag = 0, frag_offset = 0;
    NX_IPV6_HEADER_FRAGMENT_OPTION *ipv6_fragment_option;

    if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) {
        ip_header = (NX_IPV4_HEADER *)packet_ptr->nx_packet_prepend_ptr;
        protocol = ip_header->nx_ip_header_word_2;
        NX_CHANGE_ULONG_ENDIAN(protocol);
        protocol = (protocol & 0xFF0000) >> 16;
        tmp = ip_header->nx_ip_header_word_1;
        NX_CHANGE_ULONG_ENDIAN(tmp);
        frag_flag = (tmp >> 13) & 0x7;
        frag_offset = tmp & 0x1FFF;
        if (((frag_flag & BIT(0)) || frag_offset) && (protocol == IPPROTO_ICMP))
            return TRUE;
    } else if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6) {
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
        protocol = ipv6_header->nx_ip_header_word_1;
        NX_CHANGE_ULONG_ENDIAN(protocol);
        protocol = (protocol & 0x0000FF00) >> 8;
        if (protocol == NX_PROTOCOL_NEXT_HEADER_FRAGMENT) {
            ipv6_fragment_option =
                (NX_IPV6_HEADER_FRAGMENT_OPTION *)(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER));
            protocol = ipv6_fragment_option->nx_ipv6_header_fragment_option_next_header;
            if (protocol == IPPROTO_ICMPV6)
                return TRUE;
        }
    }

    return FALSE;
}

int aic_net_get_trans_params(NX_PACKET *packet_ptr, u8 *protocol,
                             u32 *ip_header_len, u16 *src_port,
                             u16 *dst_port)
{
    u8 proto;
    u8 *ipv4_header;
    u32 val, ip_len, tmp;
    u16 *trans_header_ptr, frag_offset;
    NX_IPV6_HEADER *ipv6_header;

    if (!packet_ptr->nx_packet_ip_header)
        packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;

    if (!packet_ptr->nx_packet_ip_header_length &&
        (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4))
        packet_ptr->nx_packet_ip_header_length =
            (packet_ptr->nx_packet_prepend_ptr[0] & 0x0f) *
            (packet_ptr->nx_packet_prepend_ptr[0] >> 4);
    else if (!packet_ptr->nx_packet_ip_header_length &&
             (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6))
        packet_ptr->nx_packet_ip_header_length = sizeof(NX_IPV6_HEADER);

    if ((packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4) &&
        ((packet_ptr->nx_packet_prepend_ptr[0] & 0xF0) == 0x40)) {
        ipv4_header = packet_ptr->nx_packet_ip_header;
        memcpy(&val, ipv4_header + 8, sizeof(u32));
        NX_CHANGE_ULONG_ENDIAN(val);
        proto = (u8)((val >> 16) & 0xFF);
        //alog_info("val = 0x%x proto 0x%x\n", val, proto);
        memcpy(&val, ipv4_header, sizeof(u32));
        NX_CHANGE_ULONG_ENDIAN(val);
        ip_len = ((val >> 24) & 0xF) * ((val >> 28) & 0xF);
        tmp = ((NX_IPV4_HEADER *)ipv4_header)->nx_ip_header_word_1;
        NX_CHANGE_ULONG_ENDIAN(tmp);
        frag_offset = tmp & NX_IP_FRAGMENT_MASK;
        if (frag_offset) {
            if (protocol)
                *protocol = 0;
            return AIC_SUCCESS;
        }
    } else if (((packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6) &&
               ((packet_ptr->nx_packet_prepend_ptr[0] & 0xF0) == 0x60))) {
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr->nx_packet_prepend_ptr;
        val = ipv6_header->nx_ip_header_word_1;
        NX_CHANGE_ULONG_ENDIAN(val);
        proto = (val >> 8) & 0x000000FF;
        ip_len = sizeof(NX_IPV6_HEADER);
    } else {
        return AIC_EINVAL;
    }

    if (protocol)
        *protocol = proto;

    if (ip_header_len)
        *ip_header_len = ip_len;

    if ((proto != NX_PROTOCOL_TCP) && (proto != NX_PROTOCOL_UDP))
        return AIC_SUCCESS;

    trans_header_ptr = (u16 *)((u32)packet_ptr->nx_packet_ip_header + ip_len);

    if (src_port)
        *src_port = htons(*trans_header_ptr);

    if (dst_port)
        *dst_port = htons(*(trans_header_ptr + 1));

    return AIC_SUCCESS;
}

int aic_socket_register_establish_notify(int sockID,
                                         void (*tcp_establish_notify)(int sock_id, void *pdata),
                                         void *pdata)
{
    return  nx_bsd_socket_register_establish_notify(sockID, tcp_establish_notify, pdata);
}

int aic_socket_unregister_establish_notify(int sockID)
{
    return nx_bsd_socket_unregister_establish_notify(sockID);
}

int aic_tcp_socket_retransmit_configure(int sockID, ULONG max_retries,
                                        ULONG max_timeout)
{
    return nx_bsd_tcp_socket_retransmit_configure(sockID, _nx_tcp_transmit_timer_rate, max_retries,
                                                  NX_TCP_RETRY_SHIFT, max_timeout);
}

int aic_tcp_socket_transmit_max_queue_configure(int sockID, u32 max_queue_depth)
{
    return nx_bsd_tcp_transmit_max_queue_configure(sockID, max_queue_depth);
}

int aic_socket_register_connect_notify(int sockID,
                                       void (*tcp_connect_notify)(int sock_id, u32 status, void *pdata),
                                       void *pdata)
{
    return  nx_bsd_socket_register_connect_notify(sockID, tcp_connect_notify, pdata);
}

int aic_socket_unregister_connect_notify(int sockID)
{
    return nx_bsd_socket_unregister_connect_notify(sockID);
}

int aic_socket_register_receive_notify(int sockID,
                                       void (*tcp_recv_notify)(INT sock_id, void *pdata),
                                       void *pdata)
{
    return nx_bsd_socket_register_receive_notify(sockID, tcp_recv_notify, pdata);
}

int aic_socket_unregister_receive_notify(int sockID)
{
    return nx_bsd_socket_unregister_receive_notify(sockID);
}

int aic_socket_register_tcp_disconnect_notify(int sockID,
                                              void (*tcp_disconnect_notify)(int sock_id, void *pdata, int is_active),
                                              void *pdata)
{
    return nx_bsd_socket_register_tcp_disconnect_notify(sockID,
                                                        tcp_disconnect_notify,
                                                        pdata);
}

int aic_socket_unregister_tcp_disconnect_notify(int sockID)
{
    return nx_bsd_socket_unregister_tcp_disconnect_notify(sockID);
}

#ifdef CONFIG_SUBSYS_AP
int aic_socket_register_receive_ext_notify(int sockID,
                                       void (*tcp_recv_ext_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata)
{
    return nx_bsd_socket_register_receive_ext_notify(sockID, tcp_recv_ext_notify, pdata);
}

int aic_socket_unregister_receive_ext_notify(int sockID)
{
    return nx_bsd_socket_unregister_receive_ext_notify(sockID);
}

int aic_socket_register_tcp_ack_notify(int sockID,
                                       void (* tcp_ack_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata)
{
    return nx_bsd_socket_register_tcp_ack_notify(sockID, tcp_ack_notify, pdata);
}

int aic_socket_unregister_tcp_ack_notify(int sockID)
{
    return nx_bsd_socket_unregister_tcp_ack_notify(sockID);
}

int aic_socket_register_udp_ack_notify(int sockID,
                                       void (* udp_ack_notify)(INT sock_id, int length, void *pdata),
                                       void *pdata)
{
    return nx_bsd_socket_register_udp_ack_notify(sockID, udp_ack_notify, pdata);
}

int aic_socket_unregister_udp_ack_notify(int sockID)
{
    return nx_bsd_socket_unregister_udp_ack_notify(sockID);
}

int aic_socket_register_udp_drop_notify(int sockID,
                                        void (* udp_drop_notify)(INT sock_id, int length, void *pdata),
                                        void *pdata)
{
    return nx_bsd_socket_register_udp_drop_notify(sockID, udp_drop_notify, pdata);
}

int aic_socket_unregister_udp_drop_notify(int sockID)
{
    return nx_bsd_socket_unregister_udp_drop_notify(sockID);
}

#endif

int aic_socket_setup_window(int sockID, u32 window_size, u32 ack_threshold)
{
    return nx_bsd_socket_setup_window(sockID, window_size, ack_threshold);
}

int aic_socket_send_keepalive(int sockID)
{
    return nx_bsd_socket_send_keepalive(sockID);
}

struct aic_network_ip_instance *aic_get_default_ip_instance(void)
{
#ifdef CONFIG_PDP_MGR
    if (alte_ip.ip_ptr)
        return &alte_ip;
#endif

#ifdef CONFIG_NET_SEMI_TETHER
    if (usb_semi_tether_ip.ip_ptr)
        return &usb_semi_tether_ip;
#endif

#ifdef CONFIG_IMS_NET_MGR
    if (aims_ip.ip_ptr)
        return &aims_ip;
#endif

    return AIC_NULL;
}

#ifdef CONFIG_SUBSYS_AP
/**
 * Returns an entry containing addresses of address family AF_INET
 * for the host with name name.
 * only one address is returned.
 *
 * @param name the hostname to resolve
 * @return an entry containing addresses of address family AF_INET
 *         for the host with name name
 */
struct hostent *aic_gethostbyname(const char *name)
{
    int ret;
    in_addr_t addr;
    struct addrinfo *res;
    struct addrinfo hints;

    /* buffer variables for aic_gethostbyname() */
    static struct hostent s_hostent;
    static char *s_aliases;
    static in_addr_t s_hostent_addr;
    static in_addr_t *s_phostent_addr[2];
    static char s_hostname[NX_DNS_NAME_MAX + 1];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    ret = getaddrinfo(name, NULL, &hints, &res);
    if (ret != NX_SUCCESS || !res) {
        aicx_error(0x156c17, "sd", "get host fail %s ret %d\r\n", name, ret);
        return NULL;
    }

    aos_memcpy(&addr, &res->ai_addr->sa_data[2], 4);

    /* fill hostent */
    s_hostent_addr = addr;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = NULL;
    strncpy(s_hostname, name, NX_DNS_NAME_MAX);
    s_hostname[strlen(name)] = 0;
    s_hostent.h_name = s_hostname;
    s_aliases = NULL;
    s_hostent.h_aliases = &s_aliases;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(in_addr_t);
    s_hostent.h_addr_list = (char **)&s_phostent_addr;
    freeaddrinfo(res);

    return &s_hostent;
}

/**
 * aic_identify_packet_pri - Identify whether the packet is an ACK packet.
 *
 * @packet_ptr: The pointer to the packet that needs to be indentified.
 *
 * Identify whether it is an ACK packet according to whether the length of the
 * data packet is consistent with the length of the IP header plus the TCP
 * header.
 */
bool aic_identify_packet_pri(NX_PACKET *packet_ptr,
                             struct aos_mbuf_st *mbuf)
{
    int ret;
    u32 option_words;
    u16 src_port = 0, dst_port = 0;
    u8 protocol, flag;
    u32 ip_header_len;
    NX_TCP_HEADER *tcp_header_ptr;

    ret = aic_net_get_trans_params(packet_ptr, &protocol, &ip_header_len,
                                   &src_port, &dst_port);
    if (ret)
        return ret;

    if (mbuf) {
        mbuf->ext1 = src_port;
        mbuf->user_num = protocol;
    }

    if (protocol == NX_PROTOCOL_TCP) {
        tcp_header_ptr = (NX_TCP_HEADER *)(packet_ptr->nx_packet_ip_header +
            packet_ptr->nx_packet_ip_header_length);
        option_words = tcp_header_ptr->nx_tcp_header_word_3;
        NX_CHANGE_ULONG_ENDIAN(option_words);

        flag = (option_words >> 16) & 0xFF;
        option_words = (option_words >> 28) - 5;

        if (flag & BIT(1)) {
            if (mbuf)
                mbuf->ext = 255;

            return AIC_TRUE;
        } else if (dst_port == 21) {
            if (mbuf)
                mbuf->ext = 253;

            return AIC_TRUE;
        }

        if ((packet_ptr->nx_packet_length > (packet_ptr->nx_packet_ip_header_length +
             (sizeof(NX_TCP_HEADER) + (option_words << 2)))) ||
            (flag & (BIT(0) | BIT(2))))
            return AIC_FALSE;

        if (mbuf)
            mbuf->ext = 1;

        return AIC_TRUE;
    } else if (protocol == NX_PROTOCOL_UDP && (dst_port == 53 ||
                                               src_port == 53)) {
        if (mbuf)
            mbuf->ext = 250;
        return AIC_TRUE;
    } else if ((protocol == NX_PROTOCOL_ICMP &&
                (*(packet_ptr->nx_packet_prepend_ptr + ip_header_len) == NX_ICMP_ECHO_REQUEST_TYPE ||
                 *(packet_ptr->nx_packet_prepend_ptr + ip_header_len) == NX_ICMP_ECHO_REPLY_TYPE)) ||
               (protocol == NX_PROTOCOL_ICMPV6 &&
                (*(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) == NX_ICMPV6_ECHO_REPLY_TYPE ||
                 *(packet_ptr->nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER)) == NX_ICMPV6_ECHO_REQUEST_TYPE))) {
        if (mbuf) {
            if (*(packet_ptr->nx_packet_append_ptr - 1) == 0xFF &&
                *(packet_ptr->nx_packet_append_ptr - 2) == 0xFF) {
                mbuf->ext = 252;
                aicx_info(0x18af66, "", "[PING] connect check\n");
            } else {
                mbuf->ext = 254;
            }
        }
        return AIC_TRUE;
    } else {
        return AIC_FALSE;
    }
}
#endif

u32 aic_get_mobile_traffic(long *rxpackets, long *rxbytes, long *txpackets,
                           long *txbytes)
{
    struct aic_network_ip_instance *net_ptr;
    NX_IP *ip_ptr;
    NX_INTERFACE *nx_interface = AIC_NULL;
    int i = 0;

    if (rxpackets)
        *rxpackets = 0;
    if (rxbytes)
        *rxbytes = 0;
    if (txpackets)
        *txpackets = 0;
    if (txbytes)
        *txbytes = 0;

    net_ptr = aic_get_default_ip_instance();
    if (!net_ptr)
        return AIC_EFAULT;

    ip_ptr = net_ptr->ip_ptr;
    while (i < NX_MAX_PHYSICAL_INTERFACES) {
        nx_interface = &(ip_ptr->nx_ip_interface[i]);
        if (nx_interface->nx_interface_valid == AIC_FALSE) {
            i++;
            continue;
        }

        if (strncmp(nx_interface->nx_interface_name, ANET_PDP_IFC_NAME, strlen(ANET_PDP_IFC_NAME)) == 0) {
            if (rxpackets)
                *rxpackets += nx_interface->nx_interface_packets_received;

            if (rxbytes)
                *rxbytes += nx_interface->nx_interface_bytes_received;

            if (txpackets)
                *txpackets += nx_interface->nx_interface_packets_sent;

            if (txbytes)
                *txbytes += nx_interface->nx_interface_bytes_sent;
        }

        i++;
    }

    return AIC_SUCCESS;
}

void aic_checksum_reduce(unsigned char* checksum, unsigned char* old_data,
                         int old_data_length)
{
    int i, j;
    int x, old_value;

    i = 0;
    j = 1;

    x = checksum[i] * 256 + checksum[j];
    x = (~x) & 0xFFFF;

    while (old_data_length) {
        old_value = old_data[i] * 256 + old_data[j];

        old_data += 2;

        x -= (old_value & 0xFFFF);

        if (x <= 0)
        {
            x--;
            x &= 0xFFFF;
        }

        old_data_length -= 2;
    }

    x = ~x & 0xFFFF;

    checksum[i] = (unsigned char)(x / 256);
    checksum[j] = (unsigned char)(x & 0xFF);
}

void aic_checksum_add(unsigned char* checksum, unsigned char* new_data,
                      int new_data_length)
{
    int i, j;
    int x, new_value;

    i = 0;
    j = 1;

    x = checksum[i] * 256 + checksum[j];
    x = (~x) & 0xFFFF;

    while (new_data_length) {
        new_value = new_data[i] * 256 + new_data[j];

        new_data += 2;

        x += (new_value & 0xFFFF);

        if (x & 0x10000)
        {
            x++;
            x &= 0xFFFF;
        }

        new_data_length -= 2;
    }

    x = ~x & 0xFFFF;

    checksum[i] = (unsigned char)(x / 256);
    checksum[j] = (unsigned char)(x & 0xFF);
}

void aic_checksum_adjust(unsigned char* checksum, unsigned char* old_data,
                         int old_data_length, unsigned char* new_data,
                         int new_data_length)
{
    int i, j;
    int x, old_value, new_value;

    i = 0;
    j = 1;

    x = checksum[i] * 256 + checksum[j];
    x = (~x) & 0xFFFF;

    while (old_data_length) {
        old_value = old_data[i] * 256 + old_data[j];

        old_data += 2;

        x -= (old_value & 0xFFFF);

        if (x <= 0)
        {
            x--;
            x &= 0xFFFF;
        }

        old_data_length -= 2;
    }

    while (new_data_length) {
        new_value = new_data[i] * 256 + new_data[j];

        new_data += 2;

        x += (new_value & 0xFFFF);

        if (x & 0x10000)
        {
            x++;
            x &= 0xFFFF;
        }

        new_data_length -= 2;
    }

    x = ~x & 0xFFFF;

    checksum[i] = (unsigned char)(x / 256);
    checksum[j] = (unsigned char)(x & 0xFF);
}

int aic_net_check_ctcc_auth()
{
    struct aic_network_ip_instance *aic_ip_ptr;

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr) {
        return 0;
    }

    return aic_ip_ptr->ctcc_auth_mode;
}

#ifndef WIN32
void aic_time_zone_conversion(aos_systime *local_time,
                                     int time_zone, u32 leap_year)
{
    int hour, lastday = 0;

    aicx_info(0x101a34, "d", "[NET]: time zone is UTC %d\n", time_zone);
    if (time_zone > 0) {
        if (local_time->month == 1 || local_time->month == 3 ||
            local_time->month == 5 || local_time->month == 7 ||
            local_time->month == 8 || local_time->month == 10 ||
            local_time->month == 12) {
            lastday = 31;
        } else if (local_time->month == 4 || local_time->month == 6 ||
                  local_time->month == 9 || local_time->month == 11) {
            lastday = 30;
        } else {
            if(leap_year == NX_TRUE)
                lastday = 29;
            else
                lastday = 28;
        }

        if ((local_time->hour + time_zone) >= 24) {
            local_time->hour = local_time->hour + time_zone - 24;
            local_time->day += 1;
            if (local_time->day > lastday) {
                local_time->day -= lastday;
                local_time->month += 1;
                if (local_time->month > 12) {
                    local_time->month -= 12;
                    local_time->year += 1;
                }
            }
        } else {
            local_time->hour += time_zone;
        }
    } else if(time_zone < 0) {
        hour = local_time->hour + time_zone;
        if (hour < 0) {
            local_time->hour = hour + 24;
            local_time->day -= 1;
            if (local_time->day == 0) {
                local_time->month -= 1;
                if (local_time->month == 1 || local_time->month == 3 ||
                    local_time->month == 5 || local_time->month == 7 ||
                    local_time->month == 8 || local_time->month == 10 ||
                    local_time->month == 0) {
                    lastday = 31;
                } else if (local_time->month == 4 || local_time->month == 6 ||
                           local_time->month == 9 || local_time->month == 11) {
                    lastday = 30;
                } else {
                    if(leap_year == NX_TRUE )
                        lastday = 29;
                    else
                        lastday = 28;
                }

                local_time->day += lastday;
                if (local_time->month == 0) {
                    local_time->day = 31;
                    local_time->month += 12;
                    local_time->year -= 1;
                }
            }
        } else {
            local_time->hour += time_zone;
        }
    }
}

u32 aic_sntp_client_get_time(CHAR *buffer, UINT length, int time_zone)
{
    u32 offset, return_length;
    aos_systime local_time;
    int current_timezone = 0;
    int timezone_diff;

    aos_memset(&local_time, 0, sizeof(aos_systime));
    if (time_zone != 0) {
        aos_get_systime(&local_time);
        current_timezone = aic_get_time_zone_in_quarters();
        timezone_diff = time_zone - current_timezone;
        if (timezone_diff)
            aic_adjust_time_with_time_zone(&local_time, timezone_diff);
    } else {
        aos_get_utctime(&local_time);
    }

    /* Check if we have a long enough buffer. */
    if (length < 5)
        /* Return the error status. */
        return NX_SNTP_INVALID_DATETIME_BUFFER;

    /* Decrease length for terminal zero. */
    length--;

    /* Substitute numeric month to name of the month. */
    if ((local_time.month < JANUARY) || (local_time.month > DECEMBER))
       return NX_SNTP_ERROR_CONVERTING_DATETIME;

    offset = 0;
    return_length = _nx_utility_uint_to_string(local_time.year, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 1)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = '/';

    if (local_time.month < 10)
        buffer[offset++] = '0';
    return_length = _nx_utility_uint_to_string(local_time.month, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 2)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = '/';

    if (local_time.day < 10)
        buffer[offset++] = '0';
    /* Write in the rest of the data as numeric from the Date Time objext. */
    return_length = _nx_utility_uint_to_string(local_time.day, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 2)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ',';

    if (local_time.hour < 10)
        buffer[offset++] = '0';
    return_length = _nx_utility_uint_to_string(local_time.hour, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 1)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ':';
    if (local_time.minute < 10)
        buffer[offset++] = '0';
    return_length = _nx_utility_uint_to_string(local_time.minute, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 1)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }
    buffer[offset++] = ':';
    if (local_time.second < 10)
        buffer[offset++] = '0';
    return_length = _nx_utility_uint_to_string(local_time.second, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;

    if ((return_length == 0) || ((length - offset) < 6)) {
       return NX_SNTP_ERROR_CONVERTING_DATETIME;
    }

    if(time_zone >= 0) {
        buffer[offset++] = '+';
    } else {
        buffer[offset++] = '-';
        time_zone = -time_zone;
    }
    if (time_zone < 10)
        buffer[offset++] = '0';
    return_length = _nx_utility_uint_to_string(time_zone, 10,
                                               &buffer[offset],
                                               length - offset);
    offset += return_length;
    buffer[offset] = '\0';

    return NX_SUCCESS;
}
#endif

#ifdef CONFIG_D2D
u32 _nx_driver_hardware_packet_send(NX_PACKET *packet_ptr)
{
    return aic_lte_nic_send(packet_ptr);
}
#endif

/**
 * aic_mktime_to_seconds - Convert the date and time to seconds.
 * Consider tm_isdst(daylight saving time), modify the tm.tm_hour.
 * tm_mon should be 1~12,if it is 0~11,then 1 should be added.
 */
ulong aic_mktime_to_seconds(aos_systime *systime_ptr)
{
    ulong seconds;
    u32 year, mon, day, hour, min, sec;
    /* The default local time zone is 8. */
    //struct tm *tm_test;
    u8 civil_time[60] = {'\0'};

    year = systime_ptr->year;
    mon = systime_ptr->month;
    day = systime_ptr->day;
    hour = systime_ptr->hour;
    min = systime_ptr->minute;
    sec = systime_ptr->second;

    snprintf(civil_time, sizeof(civil_time), "%d.%d.%d  %d:%d:%d",
             year, mon, day, hour, min, sec);
    aicx_info(0x1db79c, "s", "convert date time: %s\n", civil_time);

    //year -= 30;
    /* 1...12 -> 11,12,1...10 */
    if (0 >= (s32)(mon -= 2)) {
        /* Put Feb last since it has leap day. */
        mon += 12;
        year -= 1;
    }

    seconds = ((((u32) (year/4 - year/100 +year/400 + 367*mon/12 +day) +
               year * 365 - 719499) * 24 + hour) * 60 + min) * 60 + sec;
    aicx_info(0x073b2d, "d", "seconds converted is %d\n", seconds);


#if 0
        //for test
        /** We can use gmtime_r or gmtime to convert seconds to civil time,
         * in order to test if the seconds result is right.
         */
        tm_test = gmtime((time_t *)&seconds);
        /** Maybe the actual year should be tm_year plus 1900 or 1970.
         * And the actual month should be tm_mon plus 1.
         */
        sprintf(civil_time, "%d.%d.%d  %d:%d:%d", tm_test->tm_year,
                tm_test->tm_mon, tm_test->tm_mday, tm_test->tm_hour,
                tm_test->tm_min, tm_test->tm_sec);
        aicx_info(0x053590, "ss", "%s:civil time is %s\n", __func__, civil_time);
#endif

    return seconds;
}

