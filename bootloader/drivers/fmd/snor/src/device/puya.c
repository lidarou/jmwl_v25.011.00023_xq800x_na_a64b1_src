/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the puya flash device infomation.
 */

#include <aic_errno.h>
#include <aic_size.h>
#include <snor.h>

#define PUYA_CLK_80M                (80000000)
#define PUYA_CLK_85M                (85000000)
#define PUYA_CLK_104M               (104000000)
#define PUYA_CLK_120M               (120000000)

/* p25q64slf */
#define P25Q64SLF_DUMMY_4_CLKS_FREQ_70M     0x1
#define P25Q64SLF_DUMMY_6_CLKS_FREQ_85M     0x2
#define P25Q64SLF_DUMMY_8_CLKS_FREQ_104M    0x3

/* p25q32le */
#define P25Q32LE_DUMMY_4_CLKS_FREQ_80M      0x1
#define P25Q32LE_DUMMY_6_CLKS_FREQ_104M     0x2

/* p25q32sh */
#define P25Q32SH_DUMMY_4_CLKS_FREQ_80M      0x1
#define P25Q32SH_DUMMY_6_CLKS_FREQ_104M     0x2
#define P25Q32SH_DUMMY_8_CLKS_FREQ_120M     0x3

/* p25q16sl */
#define P25Q16SL_DUMMY_4_CLKS_FREQ_55M      0x1
#define P25Q16SL_DUMMY_6_CLKS_FREQ_70M      0x2
#define P25Q16SL_DUMMY_8_CLKS_FREQ_85M      0x3

/* p25q32sl */
#define P25Q32SL_DUMMY_4_CLKS_FREQ_55M      0x1
#define P25Q32SL_DUMMY_6_CLKS_FREQ_70M      0x2
#define P25Q32SL_DUMMY_8_CLKS_FREQ_85M      0x3

#define DUMMY_4_CLOCKS              4
#define DUMMY_6_CLOCKS              6
#define DUMMY_8_CLOCKS              8

/* Status Register 1 bits. */
#define PUYA_SR_BP0         BIT(2)  /* Block protect 0 */
#define PUYA_SR_BP1         BIT(3)  /* Block protect 1 */
#define PUYA_SR_BP2         BIT(4)  /* Block protect 2 */
#define PUYA_SR_TB          BIT(5)  /* Top/Bottom protect */
#define PUYA_BP_MASK        (PUYA_SR_BP2 | PUYA_SR_BP1 | PUYA_SR_BP0)
#define PUYA_SR_BP_SHIFT     2

/* Status Register 2 bits. */
#define PUYA_SR2_QUAD_EN    BIT(1)
#define PUYA_SR2_LB1        BIT(3)  /* Security Register Lock Bit 1 */
#define PUYA_SR2_LB2        BIT(4)  /* Security Register Lock Bit 2 */
#define PUYA_SR2_LB3        BIT(5)  /* Security Register Lock Bit 3 */

/* flash commands */
#define PUYA_CMD_ESECR         0x44    /* Erase Security registers */
#define PUYA_CMD_PSECR         0x42    /* Program Security registers */
#define PUYA_CMD_RSECR         0x48    /* Read Security registers */
#define PUYA_CMD_ENTERQPI      0x38    /* Enter QPI mode */
#define PUYA_CMD_SET_PARAM     0xC0    /* Set read parameters */
#define PUYA_CMD_EXITQPI       0xFF    /* Exit QPI mode */
#define PUYA_CMD_SFDP          0x5A    /* Read SFDP mode */

static void puya_get_locked_range(struct snor_dev *snor, u8 sr, u32 *ofs,
                                    u32 *len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val = (sr & PUYA_BP_MASK) >> PUYA_SR_BP_SHIFT;

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

static int puya_lock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    puya_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if ((ofs >= locked_offs) && ((ofs + len) <= (locked_offs + locked_len)))
        return AIC_SUCCESS;

    if ((ofs > (locked_offs + locked_len)) || ((ofs + len) < locked_offs))
        return AIC_EINVAL;

    lock_len = ofs + len;

    if (lock_len >= size) {
        val = PUYA_BP_MASK;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            PUYA_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(PUYA_BP_MASK | PUYA_SR_TB)) | (val | PUYA_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static int puya_unlock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    puya_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if (ofs >= (locked_offs + locked_len))
        return AIC_SUCCESS;

    lock_len = ofs;

    if (lock_len == 0) {
        val = 0;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            PUYA_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(PUYA_BP_MASK | PUYA_SR_TB)) | (val | PUYA_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static boolean puya_is_locked(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 locked_offs, locked_len, ofs_max, locked_max;
    u8 sr;

    snor_read_sr1(snor, &sr);

    puya_get_locked_range(snor, sr, &locked_offs, &locked_len);

    ofs_max = ofs + len;
    locked_max = locked_offs + locked_len;

    return ((ofs >= locked_offs) && (ofs_max <= locked_max));
}

static void puya_quad_enable(struct snor_dev *snor)
{
    u8 sr;

    snor_read_sr2(snor, &sr);
    if (sr & PUYA_SR2_QUAD_EN)
        return;

    sr |= PUYA_SR2_QUAD_EN;
    snor_write_sr2(snor, sr);
}

static void puya_qpi_enter(struct snor_dev *snor)
{
    int ret;
    u8 dummy_val;

    SNOR_ASSERT_INFO((snor->work_mode == SNOR_SPI_MODE), "wrong work mode\n");

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    ret = snor->phy_ops->write_reg(snor, PUYA_CMD_ENTERQPI, NULL, 0);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "enter QPI mode failed(%d)\n", ret);

    /* delay 1ms make sure flash truely in QPI mode. */
    snor_udelay(1UL);

    snor->reg_stp = SNOR_STP_4_0_0_0_4;
    dummy_val = (snor->info->dummy_val) << 4;
    ret = snor->phy_ops->write_reg(snor, PUYA_CMD_SET_PARAM, &dummy_val, 1);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "set dummy clocks failed(%d)\n", ret);

    snor->work_mode = SNOR_QPI_MODE;
}

static void puya_get_voltage_range(struct snor_dev *snor, u16 *max_vol, u16 *min_vol)
{
    int ret;
    u32 addr, val;

    addr = 0x60;
    snor->reg_stp = SNOR_STP_1_1_0_1_1;
    snor->read_dummy = 8;
    ret = snor->phy_ops->read_reg(snor, PUYA_CMD_SFDP, (u8 *)&val, addr, 4);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "Read SFDP error(%d)\n", ret);

    *max_vol = val & 0xFFFF;
    *min_vol = (val >> 16) & 0xFFFF;
}

static boolean puya_is_dtr_support(struct snor_dev *snor)
{
    int ret;
    u32 addr, val;

    addr = 0x30;
    snor->reg_stp = SNOR_STP_1_1_0_1_1;
    snor->read_dummy = 8;
    ret = snor->phy_ops->read_reg(snor, PUYA_CMD_SFDP, (u8 *)&val, addr, 4);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "Read SFDP error(%d)\n", ret);

    return (val & (1 << 19));
}

static struct snor_dev_ops s_puya_dev_ops = {
    .quad_enable = puya_quad_enable,
    .qpi_enter = puya_qpi_enter,
    .lock = puya_lock,
    .unlock = puya_unlock,
    .is_locked = puya_is_locked,
};

static struct snor_erase_info s_puya_erase[] = {
    {SNOR_CMD_SE_4K, SIZE_4K},
    {SNOR_CMD_BE_32K, SIZE_32K},
    {SNOR_CMD_BE_64K, SIZE_64K},
};

static struct snor_dev_info s_puya_devs[] = {
    {0x6015, 0x2000, 0x1650, SIZE_2M, SIZE_4K, SIZE_256B, 3, PUYA_CLK_85M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, P25Q16SL_DUMMY_8_CLKS_FREQ_85M),
             SIZE_64K, s_puya_erase, ARRAY_SIZE(s_puya_erase),
             &s_puya_dev_ops}, /* p25q16sl */
    {0x6016, 0x3600, 0x2300, SIZE_4M, SIZE_4K, SIZE_256B, 3, PUYA_CLK_104M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_6_CLOCKS, P25Q32SH_DUMMY_6_CLKS_FREQ_104M),
             SIZE_64K, s_puya_erase, ARRAY_SIZE(s_puya_erase),
             &s_puya_dev_ops}, /* p25q32sh */
    {0x6016, 0x2000, 0x1650, SIZE_4M, SIZE_4K, SIZE_256B, 3, PUYA_CLK_104M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_6_CLOCKS, P25Q32LE_DUMMY_6_CLKS_FREQ_104M),
             SIZE_64K, s_puya_erase, ARRAY_SIZE(s_puya_erase),
             &s_puya_dev_ops}, /* p25q32le */
    {0x6016, 0x0, 0x0, SIZE_4M, SIZE_4K, SIZE_256B, 3, PUYA_CLK_85M, (SNOR_HAS_QPI_MODE | SNOR_HAS_DTR_MODE),
             READ_INFO(DUMMY_8_CLOCKS, P25Q16SL_DUMMY_8_CLKS_FREQ_85M),
             SIZE_64K, s_puya_erase, ARRAY_SIZE(s_puya_erase),
             &s_puya_dev_ops}, /* p25q32sl */
    {0x6017, 0x2000, 0x1650, SIZE_8M, SIZE_4K, SIZE_256B, 3, PUYA_CLK_104M, (SNOR_HAS_QPI_MODE),
             READ_INFO(DUMMY_8_CLOCKS, P25Q64SLF_DUMMY_8_CLKS_FREQ_104M),
             SIZE_128K, s_puya_erase, ARRAY_SIZE(s_puya_erase),
             &s_puya_dev_ops}, /* p25q64slf */
};

static struct snor_dev_info* puya_get_dev_info(struct snor_dev *snor, u16 did)
{
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_puya_devs); i++) {
        if (s_puya_devs[i].did == did) {
            if (did == 0x6016) {
                u16 max_vol, min_vol;

                if (puya_is_dtr_support(snor) &&
                    (s_puya_devs[i].flags & SNOR_HAS_DTR_MODE)) {
                        return &s_puya_devs[i];
                }

                puya_get_voltage_range(snor, &max_vol, &min_vol);

                if ((max_vol == s_puya_devs[i].max_vol) &&
                    (min_vol == s_puya_devs[i].min_vol))
                    return &s_puya_devs[i];
            } else {
                return &s_puya_devs[i];
            }
        }
    }

    return NULL;
}

struct snor_vendor g_puya = {
    .mid = 0x85,
    .get_dev_info = puya_get_dev_info,
};
