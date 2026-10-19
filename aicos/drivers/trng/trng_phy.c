/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ebf71
#define LOG_MODULE    AMOD_DRV

#include <irq.h>
#include <irq_chip.h>

#include "trng_phy.h"

/* TRNG_PROT_REG magic number */
#define TRNG_PROT_MAGIC_NUMBER           (0x54524e47)

aos_mutex_ptr trng_mutex_ptr = AIC_NULL;
aos_event_ptr trng_event_ptr = AIC_NULL;
aos_mutex_ptr drbg_mutex_ptr = AIC_NULL;

void trng_int_ctrl(bool enable)
{
    if (enable)
        AIC_WRITE32(REG_TRNG_INT_ENABLE, BIT_TRNG_INT_DATA_READY_EN |
                                         BIT_TRNG_INT_TRNG_FATAL_EN |
                                         BIT_TRNG_INT_ESRO_FATAL_EN);
    else
        AIC_WRITE32(REG_TRNG_INT_CLR, BIT_TRNG_INT_DATA_READY_CLR |
                                      BIT_TRNG_INT_TRNG_FATAL_CLR |
                                      BIT_TRNG_INT_ESRO_FATAL_CLR);
}

void drbg_int_ctrl(bool enable)
{
    if (enable)
        AIC_WRITE32(REG_TRNG_INT_ENABLE, BIT_TRNG_INT_DRBG_READY_EN |
                                         BIT_TRNG_INT_TRNG_FATAL_EN |
                                         BIT_TRNG_INT_ESRO_FATAL_EN);
    else
        AIC_WRITE32(REG_TRNG_INT_CLR, BIT_TRNG_INT_DRBG_READY_CLR |
                                      BIT_TRNG_INT_TRNG_FATAL_CLR |
                                      BIT_TRNG_INT_ESRO_FATAL_CLR);
}

void trng_int_clr(void)
{
    AIC_WRITE32(REG_TRNG_INT_CLR, BIT_TRNG_INT_DATA_READY_CLR);
}

void drbg_int_clr(void)
{
    AIC_WRITE32(REG_TRNG_INT_CLR, BIT_TRNG_INT_DRBG_READY_CLR);
}

void drbg_enable_ctrl(bool enable)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_DRBG_CTRL);
    if (enable)
        val |= BIT_TRNG_DRBG_ENABLE | BIT_TRNG_DRBG_RESEED;
    else
        val &= ~BIT_TRNG_DRBG_ENABLE;
    AIC_WRITE32(REG_TRNG_DRBG_CTRL, val);
}

void trng_enable_ctrl(bool enable)
{
    if (enable)
        AIC_WRITE32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB, AIC_READ32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB) |
                                                    BIT_CKM_EN_GLB_TOP_TOP_TRNG_EB);
    else
        AIC_WRITE32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB, AIC_READ32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB) &
                                                    ~BIT_CKM_EN_GLB_TOP_TOP_TRNG_EB);
}

void trng_write_prot_magic(void)
{
    AIC_WRITE32(REG_TRNG_PROT_REG, TRNG_PROT_MAGIC_NUMBER);
}

void trng_startup_num(u8 startup_num)
{
    AIC_WRITE32(REG_TRNG_STARTUP_CTRL, BIT_TRNG_STARTUP_NUM(startup_num));
}

void trng_sample_ctrl(u32 acc_num, u32 sample_div)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_SAMPLE_CTRL);
    val &= ~(BIT_TRNG_ACCUMULATE_NUM(0xffff) | BIT_TRNG_SAMPLE_DIV(0xffff));
    val |= BIT_TRNG_ACCUMULATE_NUM(acc_num) | BIT_TRNG_SAMPLE_DIV(sample_div);
    AIC_WRITE32(REG_TRNG_SAMPLE_CTRL, val);
}

void trng_esro_ctrl(u32 esro_num)
{
    AIC_WRITE32(REG_TRNG_ESRO_CTRL, BIT_TRNG_ESRO_ENABLE_CFG(esro_num));
}

void trng_reseed(void)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_DRBG_CTRL);
    val &= ~BIT_TRNG_DRBG_AUTO_RESEED;
    val |= BIT_TRNG_DRBG_RESEED;
    AIC_WRITE32(REG_TRNG_DRBG_CTRL, val);
}

void drbg_swseed(int *data, int size)
{
    int i;
    int seed[4] = {0, 0, 0, 0};

    AIC_ASSERT((size > 0) && (size <= 4));

    for(i = 0; i < size; i++)
        seed[i] = data[i];

    for (i = 0; i < 4; i++)
        AIC_WRITE32(REG_TRNG_SEED_DATA0 + i * 4, data[i]);

    trng_reseed();
}

/*void trng_auto_reseed(void)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_DRBG_CTRL);
    val |= BIT_TRNG_DRBG_AUTO_RESEED;
    val &= ~BIT_TRNG_DRBG_RESEED;
    AIC_WRITE32(REG_TRNG_DRBG_CTRL, val);
}*/

void drbg_shift_num_set(u32 shift_num)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_DRBG_CTRL);
    val &= ~BIT_TRNG_DRBG_SHIFT_NUM(0xffff);
    val |= BIT_TRNG_DRBG_SHIFT_NUM(shift_num);
    AIC_WRITE32(REG_TRNG_DRBG_CTRL, val);
}

void drbg_tap_ctrl(u32 tap_val)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_DRBG_CTRL);
    val &= ~BIT_TRNG_DRBG_TAP_CTRL(0x1ff);
    val |= BIT_TRNG_DRBG_TAP_CTRL(tap_val);
    AIC_WRITE32(REG_TRNG_DRBG_CTRL, val);
}

void trng_read_data_done(void)
{
    AIC_WRITE32(REG_TRNG_BUFFER_DATA_CTRL, BIT_TRNG_BUFFER_DATA_READ_DONE);
}

void drbg_read_data_done(void)
{
    trng_read_data_done();
    AIC_WRITE32(REG_TRNG_DRBG_DATA_READ_DONE, BIT_TRNG_DRBG_DATA_READ_DONE);
}

void trng_read_data(int *data, int size)
{
    int i;

    AIC_ASSERT((size > 0) && (size <= 4));

    for (i = 0; i < size; i++)
        data[i] = AIC_READ32(REG_TRNG_BUFFER_DATA0 + i * 4);

    trng_read_data_done();
}

void drbg_read_data(int *data, int size)
{
    int i;

    AIC_ASSERT((size > 0) && (size <= 4));

    for (i = 0; i < size; i++)
        data[i] = AIC_READ32(REG_TRNG_DRBG_DATA0 + i * 4);

    drbg_read_data_done();
}

ISR_RET_E trng_irq_handler(void *param)
{
    u32 val;

    val = AIC_READ32(REG_TRNG_INT_STATUS);

    AIC_ASSERT(!(val & BIT_TRNG_INT_TRNG_FATAL_STATUS));
    AIC_ASSERT(!(val & BIT_TRNG_INT_ESRO_FATAL_STATUS));

    if (val & BIT_TRNG_INT_DATA_READY_STATUS) {
        aos_set_event(trng_event_ptr, TRNG_DATA_VALID_FLAG, AOS_OR);
        trng_int_clr();
    }

    if (val & BIT_TRNG_INT_DRBG_READY_STATUS) {
        aos_set_event(trng_event_ptr, DRBG_DATA_VALID_FLAG, AOS_OR);
        drbg_int_clr();
    }

    return NONE_WORKER;
}

