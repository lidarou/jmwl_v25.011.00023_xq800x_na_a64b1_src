/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file audio engine module file, it contains serial debug command,
 *    communication with audio PC tool, and other function such like
 *    debug, dump, etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0147f1
#define LOG_MODULE    AMOD_SOUND

#include "snd_compat.h"
#include "aic_init.h"

#include <dvfs_inc.h>

#ifdef SIMU_SND_ENV
#define AOS_POOL_SIZE  0x4800
static UCHAR s_adpter_pool[AOS_POOL_SIZE];
static TX_BYTE_POOL s_byte_pool;

int aos_adpter_init(void)
{
    UINT ret;

    ret = tx_byte_pool_create(&s_byte_pool, "aipc", s_adpter_pool, AOS_POOL_SIZE);
    AIC_ASSERT(ret == TX_SUCCESS);
    return 0;
}
AIC_NORMAL_EARLY_INIT_EXPORT(aos_adpter_init);

void *aos_malloc(u32 size, const char *func, int line)
{
    static u32 total = 0;
    UINT ret;
    void *addr;
    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    ret = tx_byte_allocate(&s_byte_pool, &addr, size, 0);
    AIC_ASSERT(ret == TX_SUCCESS);
    TX_RESTORE

    total += size;

    aicx_debug(0x0f5e03, "ddpsd", "alloc size=%4d, total=%5d, addr=%p, file=%s, line=%d\n",
               size, total, addr, func, line);
    return addr;
}

void *aos_zmalloc(u32 size, const char *func, int line)
{
    void *addr;

    addr = aos_malloc(size, func, line);
    if (addr)
        memset(addr, 0, size);
    return addr;
}

void aos_free(void *addr, const char *func, int line)
{
    UINT ret;

    aicx_debug(0x07061c, "psd", "free addr=%p, func=%s, line=%d\n", addr, func, line);

    TX_INTERRUPT_SAVE_AREA

    TX_DISABLE
    ret = tx_byte_release(addr);
    AIC_ASSERT(ret == TX_SUCCESS);
    TX_RESTORE
}
#else
void * aos_zmalloc(u32 size)
{
    void *addr;

    addr = aos_malloc(size);
    if (addr)
        memset(addr, 0, size);
    return addr;
}
#endif

void *aos_memdup(const void *src, size_t len)
{
    void *dst = snd_malloc(len);
    if (!dst)
        return NULL;
    memcpy(dst, src, len);
    return dst;
}

int fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

void snd_dvfs_set(u32 level)
{
    static u32 snd_dvfs_ref = 0;

    if (level) {
        aic_dvfs_force(RISE | AUDIO_DMA);
        snd_dvfs_ref++;
    } else {
        if (snd_dvfs_ref) {
            snd_dvfs_ref--;
            if (!snd_dvfs_ref)
                aic_dvfs_force(FALL | AUDIO_DMA);
        }
    }
}
