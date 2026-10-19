/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the giga flash device infomation.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02e54d
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_size.h>
#include <snor.h>

#define GIGA_CLK_80M                (80000000)
#define GIGA_CLK_108M               (108000000)
#define GIGA_CLK_133M               (133000000)

#define DUMMY_4_CLKS_FREQ_80M       0x0
#define DUMMY_6_CLKS_FREQ_108M      0x2
#define DUMMY_8_CLKS_FREQ_133M      0x3

/* gd25le128e */
#define GD25LE128E_DUMMY_4_CLKS_FREQ_80M        0x0
#define GD25LE128E_DUMMY_6_CLKS_FREQ_108M       0x1
#define GD25LE128E_DUMMY_8_CLKS_FREQ_133M       0x2

#define DUMMY_4_CLOCKS              4
#define DUMMY_6_CLOCKS              6
#define DUMMY_8_CLOCKS              8

/* Status Register 1 bits. */
#define GIGA_SR_BP0         BIT(2)  /* Block protect 0 */
#define GIGA_SR_BP1         BIT(3)  /* Block protect 1 */
#define GIGA_SR_BP2         BIT(4)  /* Block protect 2 */
#define GIGA_SR_TB          BIT(5)  /* Top/Bottom protect */
#define GIGA_BP_MASK        (GIGA_SR_BP2 | GIGA_SR_BP1 | GIGA_SR_BP0)
#define GIGA_SR_BP_SHIFT     2

/* Status Register 2 bits. */
#define GIGA_SR2_QUAD_EN    BIT(1)
#define GIGA_SR2_LB1        BIT(3)  /* Security Register Lock Bit 1 */
#define GIGA_SR2_LB2        BIT(4)  /* Security Register Lock Bit 2 */
#define GIGA_SR2_LB3        BIT(5)  /* Security Register Lock Bit 3 */

/* flash commands */
#define GIGA_CMD_ESECR         0x44    /* Erase Security registers */
#define GIGA_CMD_PSECR         0x42    /* Program Security registers */
#define GIGA_CMD_RSECR         0x48    /* Read Security registers */
#define GIGA_CMD_ENTERQPI      0x38    /* Enter QPI mode */
#define GIGA_CMD_SET_PARAM     0xC0    /* Set read parameters */
#define GIGA_CMD_EXITQPI       0xFF    /* Exit QPI mode */

static void giga_get_locked_range(struct snor_dev *snor, u8 sr, u32 *ofs,
                                    u32 *len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val = (sr & GIGA_BP_MASK) >> GIGA_SR_BP_SHIFT;

    if (!val) {
        /* No protection */
        *ofs = 0;
        *len = 0;
        return;
    }

    *len = lock_density << (val - 1);

    if (*len > size)
        *len = size;

    *ofs = 0;
}

static int giga_lock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    giga_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if ((ofs >= locked_offs) && ((ofs + len) <= (locked_offs + locked_len)))
        return AIC_SUCCESS;

    if ((ofs > (locked_offs + locked_len)) || ((ofs + len) < locked_offs))
        return AIC_EINVAL;

    lock_len = ofs + len;

    if (lock_len >= size) {
        val = GIGA_BP_MASK;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            GIGA_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(GIGA_BP_MASK | GIGA_SR_TB)) | (val | GIGA_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static int giga_unlock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    giga_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if (ofs >= (locked_offs + locked_len))
        return AIC_SUCCESS;

    lock_len = ofs;

    if (lock_len == 0) {
        val = 0;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            GIGA_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(GIGA_BP_MASK | GIGA_SR_TB)) | (val | GIGA_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static boolean giga_is_locked(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 locked_offs, locked_len, ofs_max, locked_max;
    u8 sr;

    snor_read_sr1(snor, &sr);

    giga_get_locked_range(snor, sr, &locked_offs, &locked_len);

    ofs_max = ofs + len;
    locked_max = locked_offs + locked_len;

    return ((ofs >= locked_offs) && (ofs_max <= locked_max));
}

static void giga_quad_enable(struct snor_dev *snor)
{
    u8 sr;

    snor_read_sr2(snor, &sr);
    if (sr & GIGA_SR2_QUAD_EN)
        return;

    sr |= GIGA_SR2_QUAD_EN;
    snor_write_sr2(snor, sr);
}

static void giga_qpi_enter(struct snor_dev *snor)
{
    int ret;
    u8 dummy_val;

    SNOR_ASSERT(snor->work_mode == SNOR_SPI_MODE);

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    ret = snor->phy_ops->write_reg(snor, GIGA_CMD_ENTERQPI, NULL, 0);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    /* delay 1ms make sure flash truely in QPI mode. */
    snor_udelay(1UL);

    snor->reg_stp = SNOR_STP_4_0_0_0_4;
    dummy_val = (snor->info->dummy_val) << 4;
    ret = snor->phy_ops->write_reg(snor, GIGA_CMD_SET_PARAM, &dummy_val, 1);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    snor->work_mode = SNOR_QPI_MODE;
}

static struct snor_dev_ops s_giga_dev_ops = {
    .quad_enable = giga_quad_enable,
    .qpi_enter = giga_qpi_enter,
    .lock = giga_lock,
    .unlock = giga_unlock,
    .is_locked = giga_is_locked,
};

static struct snor_erase_info s_giga_erase[] = {
    {SNOR_CMD_SE_4K, SIZE_4K},
    {SNOR_CMD_BE_32K, SIZE_32K},
    {SNOR_CMD_BE_64K, SIZE_64K},
};

static struct snor_dev_info s_giga_devs[] = {
    {0x6015, 0, 0, SIZE_2M, SIZE_4K, SIZE_256B, 3, GIGA_CLK_133M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, DUMMY_8_CLKS_FREQ_133M), SIZE_64K,
             s_giga_erase, ARRAY_SIZE(s_giga_erase),
             &s_giga_dev_ops}, /* gd25le16e */
    {0x6016, 0, 0, SIZE_4M, SIZE_4K, SIZE_256B, 3, GIGA_CLK_133M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, DUMMY_8_CLKS_FREQ_133M), SIZE_64K,
             s_giga_erase, ARRAY_SIZE(s_giga_erase),
             &s_giga_dev_ops}, /* gd25le32e */
    {0x6017, 0, 0, SIZE_8M, SIZE_4K, SIZE_256B, 3, GIGA_CLK_133M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, DUMMY_8_CLKS_FREQ_133M), SIZE_128K,
             s_giga_erase, ARRAY_SIZE(s_giga_erase),
             &s_giga_dev_ops}, /* gd25le64e */
    {0x6018, 0, 0, SIZE_16M, SIZE_4K, SIZE_256B, 3, GIGA_CLK_133M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, GD25LE128E_DUMMY_8_CLKS_FREQ_133M),
             SIZE_256K, s_giga_erase, ARRAY_SIZE(s_giga_erase),
             &s_giga_dev_ops}, /* gd25le128e */
};

static struct snor_dev_info* giga_get_dev_info(struct snor_dev *snor, u16 did)
{
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_giga_devs); i++) {
        if (s_giga_devs[i].did == did)
            return &s_giga_devs[i];
    }

    return NULL;
}

struct snor_vendor g_gigadevice = {
    .mid = 0xc8,
    .get_dev_info = giga_get_dev_info,
};
