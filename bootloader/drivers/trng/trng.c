#include <aic_io.h>
#include <assert.h>
#include <reg_base.h>
#include <ckm_en_glb_top.h>

#include "trng_rf.h"

#define REG_BASE_TRNG                   REG_BASE_TOP_TRNG

/* TRNG_PROT_REG magic number */
#define TRNG_PROT_MAGIC_NUMBER           (0x54524e47)

#define TRNG_SAMPLE_DIV           0x18b1
#define TRNG_ACCUMULATE_NUM       0x1

/* determine discard how many trng data when enable esro, unit is 128bits */
#define TRNG_STARTUP_NUM          0x3

int trng_init(void)
{
    u32 val;

    /* enable trng global eb */
    AIC_WRITE32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB, BIT_CKM_EN_GLB_TOP_TOP_TRNG_EB);

    /* write protect magic number */
    AIC_WRITE32(REG_TRNG_PROT_REG, TRNG_PROT_MAGIC_NUMBER);

    /* set trng number discard num, uint is 128bits */
    AIC_WRITE32(REG_TRNG_STARTUP_CTRL, BIT_TRNG_STARTUP_NUM(TRNG_STARTUP_NUM));

    /* configure sample ctrl to asic provided value */
    val = AIC_READ32(REG_TRNG_SAMPLE_CTRL);
    val &= ~(BIT_TRNG_ACCUMULATE_NUM(0xffff) | BIT_TRNG_SAMPLE_DIV(0xffff));
    val |= BIT_TRNG_ACCUMULATE_NUM(TRNG_ACCUMULATE_NUM) | BIT_TRNG_SAMPLE_DIV(TRNG_SAMPLE_DIV);
    AIC_WRITE32(REG_TRNG_SAMPLE_CTRL, val);

    /* trng int enable */
    AIC_WRITE32(REG_TRNG_INT_ENABLE, BIT_TRNG_INT_DATA_READY_EN |
                                     BIT_TRNG_INT_TRNG_FATAL_EN |
                                     BIT_TRNG_INT_ESRO_FATAL_EN);

    return 0;
}

void trng_deinit(void)
{
    /* disable trng int */
    AIC_WRITE32(REG_TRNG_INT_ENABLE, AIC_READ32(REG_TRNG_INT_ENABLE) &
                                     ~(BIT_TRNG_INT_DATA_READY_EN |
                                       BIT_TRNG_INT_TRNG_FATAL_EN |
                                       BIT_TRNG_INT_ESRO_FATAL_EN));

    /* disable trng */
    AIC_WRITE32(REG_CKM_EN_GLB_TOP_TOP_TRNG_EB, ~BIT_CKM_EN_GLB_TOP_TOP_TRNG_EB);
}

int trng_get_data(int *data, int size)
{
    int i;
    u32 val;

    ASSERT((size > 0) && (size <= 4));

    /* enable all 8 SROs */
    AIC_WRITE32(REG_TRNG_ESRO_CTRL, BIT_TRNG_ESRO_ENABLE_CFG(0xff));

    while(1) {
        /* wait trng done */
        val = AIC_READ32(REG_TRNG_INT_STATUS);
        ASSERT(!(val & BIT_TRNG_INT_TRNG_FATAL_STATUS));
        ASSERT(!(val & BIT_TRNG_INT_ESRO_FATAL_STATUS));
        if (val & BIT_TRNG_INT_DATA_READY_STATUS) {
            AIC_WRITE32(REG_TRNG_INT_CLR, BIT_TRNG_INT_DATA_READY_CLR);
            break;
        }
    }

    /* get trng real data */
    for (i = 0; i < size; i++)
        data[i] = AIC_READ32(REG_TRNG_BUFFER_DATA0 + i * 4);

    /* clr the read buffer */
    AIC_WRITE32(REG_TRNG_BUFFER_DATA_CTRL, BIT_TRNG_BUFFER_DATA_READ_DONE);

    /* disable all SROs */
    AIC_WRITE32(REG_TRNG_ESRO_CTRL, BIT_TRNG_ESRO_ENABLE_CFG(0));

    return 0;
}
