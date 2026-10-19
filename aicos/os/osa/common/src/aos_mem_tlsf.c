
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1463a3
#define LOG_MODULE    AMOD_AOS
#include <aic_osa.h>
#include <aos_config.h>
#include <aos_mem.h>
#include "los_memory.h"


u32 aos_init_tlsf_mem(tlsf_pool_t *pool, addr_t heap_base, size_t heap_size)
{
    u32 ret;
    ret = LOS_MemInit((void *)heap_base, heap_size);
    if (!ret) {
        *pool = (tlsf_pool_t)heap_base;
    }
    return ret;
}

void *tlsf_mem_alloc(tlsf_pool_t pool, u32 size)
{
    return LOS_MemAlloc((void *)pool, size);
}

u32 tlsf_mem_free(tlsf_pool_t pool, void *ptr)
{
    return LOS_MemFree((void *)pool, ptr);
}

u32 tlsf_mem_info_get(tlsf_pool_t pool, aos_heap_size *info)
{
    LOS_MEM_POOL_STATUS poolStatus;

    UINT32 ret = LOS_MemInfoGet((void *)pool, &poolStatus);
    if (ret == LOS_OK) {
        info->total_heap = aos_get_heap_size();
        info->total_used = poolStatus.totalUsedSize;
        info->total_free = poolStatus.totalFreeSize;
        info->max_free_node = poolStatus.maxFreeNodeSize;
#if (LOSCFG_MEM_WATERLINE == 1)
        info->usage_water_line = poolStatus.usageWaterLine;
#else
        info->usage_water_line = 0;
#endif
        return AOS_SUCCESS;
    } else {
        return AOS_ERROR;
    }
}

u32 tlsf_mem_status_get(tlsf_pool_t pool, u32 *total, u32 *used)
{
    return LOS_MemStatusGet(pool, total, used);
}

u32 aos_deinit_tlsf_mem(tlsf_pool_t pool)
{
    return LOS_MemDeInit(pool);
}
