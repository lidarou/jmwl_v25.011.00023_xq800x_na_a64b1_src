/**************************************************************************/
/*                                                                        */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   AIC Packet Management                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13cec9
#define LOG_MODULE    AMOD_NET

#include <aic_errno.h>
#include <aic_osa.h>
#include <nx_api.h>
#include <nx_packet.h>

#ifdef CONFIG_NET_ALTE_DRIVER
#include "nx_alte_driver.h"
#endif
#include "aipc_api.h"
#include "aic_init.h"
#include "aic_net_api.h"
#include "aic_net_pdp_mgr.h"
#include "aic_packet_management.h"

static struct aic_packet_manage aic_pkt_mg = {0};

/**
 * aic_packet_init - Init NX_PACKET list.
 *
 * @packet_num: Number of NX_PACKET to be pre-allocated.
 *
 * Initialize the NX_PACKET link list to improve the efficiency of apply and
 * release the NX_PACKET structure.
 */
int aic_packet_init(int packet_num)
{
    int i;
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_packet_manage *pkt_mg = &aic_pkt_mg;

    pkt_mg->mem_pool_start = aos_dma_malloc(sizeof(NX_PACKET) * packet_num);
    memset(pkt_mg->mem_pool_start, 0, sizeof(NX_PACKET) * packet_num);
    if (!pkt_mg->mem_pool_start) {
        aicx_error(0x190fb9, "", "aic_packet_init: alloc aic packet mem pool fail\n");
        return AIC_ENOBUFS;
    }

    AOS_INTERRUPT_DISABLE
    for (i = 0; i < packet_num; i++) {
         packet_ptr = (NX_PACKET *)(pkt_mg->mem_pool_start +
                                    sizeof(NX_PACKET) * i);
         packet_ptr->nx_packet_next = pkt_mg->aic_packet_free_list;
         pkt_mg->aic_packet_free_list = packet_ptr;
    }

    pkt_mg->mem_pool_end = pkt_mg->mem_pool_start +
        sizeof(NX_PACKET) * packet_num;
    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

/**
 * aic_free_packet_pool - Free NX_PACKET pool.
 *
 * Free space used by the pre-allocated NX_PACKET structure.
 */
void aic_free_packet_pool(void)
{
    struct aic_packet_manage *pkt_mg = &aic_pkt_mg;
    AOS_INTERRUPT_DECLARE

    aos_dma_free(pkt_mg->mem_pool_start);
    AOS_INTERRUPT_DISABLE
    memset(pkt_mg, 0, sizeof(aic_pkt_mg));
    AOS_INTERRUPT_RESTORE
}

/**
 * aic_alloc_packet - Alloc NX_PACKET structure.
 *
 * @return: NX_PACKET structure pointer.
 *
 * The NX_PACKET is first fetched from the pre-allocated NX_PACKET link list,
 * or alloc from heap if the list is empty.
 */
NX_PACKET *aic_alloc_packet(void)
{
    NX_PACKET *packet_ptr = NULL;
    struct aic_packet_manage *pkt_mg = &aic_pkt_mg;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (pkt_mg->aic_packet_free_list) {
        packet_ptr = pkt_mg->aic_packet_free_list;
        pkt_mg->aic_packet_free_list = packet_ptr->nx_packet_next;
        packet_ptr->nx_packet_next = NULL;
        packet_ptr->nx_packet_queue_next = NULL;
#ifdef NX_PACKET_HEADER_PAD
        packet_ptr->nx_packet_packet_pad[0] = 0;
#endif
        AOS_INTERRUPT_RESTORE
    } else {
        AOS_INTERRUPT_RESTORE
#ifdef CONFIG_MEM_MODE_TLSF
        packet_ptr = aos_zalloc_relaxed(sizeof(NX_PACKET));
#else
        packet_ptr = aos_zalloc(sizeof(NX_PACKET));
#endif
        pkt_mg->alloc_heap_cnt++;
    }

    if (packet_ptr)
        packet_ptr->nx_packet_reserved = AIC_PACKET_MANAGE;
    else
        aicx_info(0x09e52a, "", "aic alloc packet fail\n");

    return packet_ptr;
}

void *aic_ppp_alloc_mem(u32 size, u8 *mem_type)
{
    void *addr = NULL;

#ifdef CONFIG_NET_ALTE_DRIVER
    if (!nx_alte_check_cached_share_mem()) {
        addr = nx_alte_alloc_block(size, 0);
        *mem_type = AIC_PACKET_SHARE_MEM;
    }
#endif

    if (addr)
        return addr;

    *mem_type = AIC_PACKET_DYNAMIC_MEM;
    addr = aos_dma_malloc(size);

    return addr;
}

void aic_ppp_release_mem(u8 *addr, u8 mem_type, u32 size)
{
    if (mem_type == AIC_PACKET_SHARE_MEM) {
#ifdef CONFIG_NET_ALTE_DRIVER
        nx_alte_release_block(addr, size);
#else
        aicx_error(0x144bc3, "x", "ppp release buf 0x%x fail\n", (u32)addr);
#endif
    } else {
        aos_dma_free(addr);
    }
}

#ifndef WIN32
#ifdef CONFIG_SUBSYS_AP
/**
 * aic_network_alloc_usb_packet - Apply the packet for the usb network card.
 *
 * @packet_ptr: Store the applied NX_PACKET pointer.
 * @packet_len: Length of packet.
 *
 * Return: Success is AIC_SUCCESS, other values are failure.
 */
UINT aic_network_alloc_usb_packet(NX_PACKET **packet_ptr, ULONG packet_len)
{
    u8 mem_type, retry_cnt = 0;
    void *mem_ptr = NULL;
    struct aic_network_ip_instance *aic_ip_ptr;

CHECK_AGAIN:
    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        aos_sleep(AIC_USB_NETWORK_MEM_TIEMOUT);
        retry_cnt++;

        if (retry_cnt > 3)
            return AIC_ECANCELED;
        else
            goto CHECK_AGAIN;
    }

#ifdef CONFIG_NET_ALTE_DRIVER
    if (!nx_alte_check_cached_share_mem()) {
        mem_ptr = nx_alte_alloc_block(packet_len + AIC_USB_NETWORK_OFFSET, 0);
        if (mem_ptr)
            mem_type = AIC_PACKET_SHARE_MEM;
    }
#endif

    if (!mem_ptr) {
        mem_type = AIC_PACKET_DYNAMIC_MEM;
        mem_ptr = aos_dma_malloc(packet_len + AIC_USB_NETWORK_OFFSET);
    }

    if (!mem_ptr) {
        aicx_error(0x078cce, "d", "[USB_NET] aic usb alloc heap mem %d fail\r\n", packet_len);
        return AIC_ENOMEM;
    }

    *packet_ptr = aic_packet_build(mem_ptr, packet_len + AIC_USB_NETWORK_OFFSET,
                                   AIC_USB_NETWORK_OFFSET,
                                   aic_ip_ptr->ip_ptr->nx_ip_default_packet_pool,
                                   mem_type);
    if (!*packet_ptr) {
        if (mem_type == AIC_PACKET_SHARE_MEM) {
#ifdef CONFIG_NET_ALTE_DRIVER
            nx_alte_release_block(mem_ptr, packet_len + AIC_USB_NETWORK_OFFSET);
#else
            aicx_error(0x0191a4, "x", "[USB_NET] release buf 0x%x fail\n", (u32)mem_ptr);
#endif
        } else {
            aos_free(mem_ptr);
        }

        aicx_error(0x114f7f, "", "[USB_NET] aic usb alloc packet pointer fail\r\n");
        return AIC_ENOMEM;
    }

    aicx_debug(0x0c98c4, "xx", "[USB] packet 0x%x mem start 0x%x\r\n",
               *packet_ptr, (*packet_ptr)->nx_packet_data_start);

    return AIC_SUCCESS;
}
#endif
#endif

/**
 * aic_free_packet - free NX_PACKET structure space.
 *
 * @return: AIC_SUCCESS is returned on success, if AIC_EPERM is returned, it
 * fail and need to be release use the NETXDUO NX_PACKET management mechanism.
 *
 * Release NX_PACKET structure space.
 */
int aic_free_packet(NX_PACKET *packet_ptr)
{
    AOS_INTERRUPT_DECLARE
    struct aic_packet_manage *pkt_mg = &aic_pkt_mg;

        aicx_debug(0x12eb63, "xx", "[FREE] packet 0x%x mem 0x%x\r\n",
                   packet_ptr, packet_ptr->nx_packet_data_start);
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    if (!(packet_ptr->nx_packet_reserved & AIC_PACKET_MANAGE) ||
        (packet_ptr->nx_packet_reserved > 0xF))
        return AIC_EPERM;

    if (packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM)
#ifdef CONFIG_NET_ALTE_DRIVER
        nx_alte_release_block(packet_ptr->nx_packet_data_start,
                              packet_ptr->nx_packet_append_ptr -
                              packet_ptr->nx_packet_data_start);
#else
        aicx_error(0x154b50, "x", "aic packet mem type is err 0x%x\n",
                   packet_ptr->nx_packet_reserved);
#endif
    else if (packet_ptr->nx_packet_reserved & AIC_PACKET_POOL_MEM)
        aos_free_mem(packet_ptr->nx_packet_data_start);
    else if (packet_ptr->nx_packet_reserved & AIC_PACKET_DYNAMIC_MEM)
        aos_dma_free(packet_ptr->nx_packet_data_start);
    else
        aicx_error(0x154b50, "x", "aic packet mem type is err 0x%x\n",
                   packet_ptr->nx_packet_reserved);

    AOS_INTERRUPT_DISABLE
    packet_ptr->nx_packet_union_next.nx_packet_tcp_queue_next =
        (NX_PACKET *)NX_PACKET_FREE;
    if (((ulong)packet_ptr >= (ulong)pkt_mg->mem_pool_start) &&
        ((ulong)packet_ptr < (ulong)pkt_mg->mem_pool_end)) {
        packet_ptr->nx_packet_next = pkt_mg->aic_packet_free_list;
        packet_ptr->nx_packet_queue_next = NULL;
        pkt_mg->aic_packet_free_list = packet_ptr;
        AOS_INTERRUPT_RESTORE
    } else {
        AOS_INTERRUPT_RESTORE
        aos_free(packet_ptr);
    }

    return AIC_SUCCESS;
}

int aic_free_packet_only(NX_PACKET *packet_ptr)
{
    AOS_INTERRUPT_DECLARE
    struct aic_packet_manage *pkt_mg = &aic_pkt_mg;

    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    if (!(packet_ptr->nx_packet_reserved & AIC_PACKET_MANAGE))
        return AIC_EPERM;

    AOS_INTERRUPT_DISABLE
    packet_ptr->nx_packet_union_next.nx_packet_tcp_queue_next =
        (NX_PACKET *)NX_PACKET_FREE;
    if (((ulong)packet_ptr >= (ulong)pkt_mg->mem_pool_start) &&
        ((ulong)packet_ptr < (ulong)pkt_mg->mem_pool_end)) {
        packet_ptr->nx_packet_next = pkt_mg->aic_packet_free_list;
        pkt_mg->aic_packet_free_list = packet_ptr;
        packet_ptr->nx_packet_queue_next = NULL;
        AOS_INTERRUPT_RESTORE
    } else {
        AOS_INTERRUPT_RESTORE
        aos_free(packet_ptr);
    }

    return AIC_SUCCESS;
}

int aic_packet_manage_init(void)
{
    return aic_packet_init(AIC_PACKET_MAX_NUM);
}

DIS_CHARGE_APP_P1_INIT_EXPORT(aic_packet_manage_init);

