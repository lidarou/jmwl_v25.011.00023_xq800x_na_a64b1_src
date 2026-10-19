/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x00f005
#define LOG_MODULE    AMOD_DRV

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <irq.h>
#include <irq_chip.h>

#include <trng_cfg.h>

#include "trng_phy.h"

#define TRNG_GET_DATA_TIMEOUT       1000

extern aos_mutex_ptr trng_mutex_ptr;
extern aos_mutex_ptr drbg_mutex_ptr;
extern aos_event_ptr trng_event_ptr;

int trng_get_data(int *data, int size)
{
    u32 trng_event_flags;
    int ret;

    ret = aos_get_mutex(trng_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AIC_SUCCESS);

    /* enable all trng ints */
    trng_int_ctrl(TRUE);

    /* enable all 8 SROs */
    trng_esro_ctrl(0xff);

    ret = aos_get_event(trng_event_ptr, TRNG_DATA_VALID_FLAG,
                        AOS_AND_CLEAR, &trng_event_flags,
                        TRNG_GET_DATA_TIMEOUT);
    AIC_ASSERT(ret == AIC_SUCCESS);

    trng_read_data(data, size);

    /* disable all 8 SROs */
    trng_esro_ctrl(0);

    aos_put_mutex(trng_mutex_ptr);

    return AIC_SUCCESS;
}

int drbg_get_data(int *data, int size, int *seed, int seed_size)
{
    u32 drbg_event_flags;
    int ret;

    ret = aos_get_mutex(drbg_mutex_ptr, AOS_WAIT_FOREVER);
    AIC_ASSERT(ret == AIC_SUCCESS);

    if (seed != AIC_NULL)
        drbg_swseed(seed, seed_size);

    /* enable drbg int */
    drbg_int_ctrl(TRUE);

    /* enable drbg */
    drbg_enable_ctrl(TRUE);

    //drbg_read_data_done();

    ret = aos_get_event(trng_event_ptr, DRBG_DATA_VALID_FLAG,
                            AOS_AND_CLEAR, &drbg_event_flags,
                            TRNG_GET_DATA_TIMEOUT);
    AIC_ASSERT(ret == AIC_SUCCESS);

    drbg_read_data(data, size);

    /* disable drbg */
    drbg_enable_ctrl(FALSE);

    aos_put_mutex(drbg_mutex_ptr);

    return AIC_SUCCESS;
}

int trng_init(void)
{
    int ret;

    /* create a mutex for trng */
    trng_mutex_ptr = aos_create_mutex("tgm");
    AIC_ASSERT(trng_mutex_ptr);

    /* create a mutex for drbg */
    drbg_mutex_ptr = aos_create_mutex("dgm");
    AIC_ASSERT(drbg_mutex_ptr);

    /* create event for trng and drbg */
    trng_event_ptr = aos_create_event("tge");
    AIC_ASSERT(trng_event_ptr);

    /* enable trng global eb */
    AIC_WRITE32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB, BIT_CKM_EN_GLB_TOP_TOP_TRNG_EB);

    /* write protect magic number */
    trng_write_prot_magic();

    /* set trng number discard num, uint is 128bits */
    trng_startup_num(TRNG_STARTUP_NUM);

    /* configure sample ctrl to asic provided value */
    trng_sample_ctrl(TRNG_ACCUMULATE_NUM, TRNG_SAMPLE_DIV);

    ret = register_irq(IRQ_TOP_TRNG, trng_irq_handler, NULL, NULL, NULL);
    AIC_ASSERT(ret == AIC_SUCCESS);

    return 0;
}
AIC_DRV_P2_INIT_EXPORT(trng_init);

