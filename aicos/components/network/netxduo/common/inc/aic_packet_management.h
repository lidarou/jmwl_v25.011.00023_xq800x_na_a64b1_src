#ifndef __AIC_PACKET_MANAGEMENT_H__
#define __AIC_PACKET_MANAGEMENT_H__

#include <aic_osa.h>

#include "nx_api.h"

#define AIC_PACKET_MAX_NUM     16

#define AIC_PACKET_MANAGE      0x1
#define AIC_PACKET_POOL_MEM    0x2
#define AIC_PACKET_DYNAMIC_MEM 0x4
#define AIC_PACKET_SHARE_MEM   0x8

#define AIC_USB_NETWORK_OFFSET 50

#define AIC_USB_NETWORK_MEM_TIEMOUT 50 /* ms */

struct aic_packet_manage {
    u8 *mem_pool_start;
    u8 *mem_pool_end;

    u32 alloc_heap_cnt;
    NX_PACKET *aic_packet_free_list;
};

int aic_packet_init(int packet_num);

void aic_free_packet_pool(void);

NX_PACKET *aic_alloc_packet(void);

int aic_free_packet(NX_PACKET *packet_ptr);

int aic_free_packet_only(NX_PACKET *packet_ptr);

void *aic_ppp_alloc_mem(u32 size, u8 *mem_type);
void aic_ppp_release_mem(u8 *addr, u8 mem_type, u32 size);

NX_PACKET *aic_packet_build(u8 *start, u32 len, u32 offset,
                            void *def_pool, u8 mem_type);
#ifndef WIN32
UINT aic_network_alloc_usb_packet(NX_PACKET **packet_ptr, ULONG packet_len);
#endif

#endif
