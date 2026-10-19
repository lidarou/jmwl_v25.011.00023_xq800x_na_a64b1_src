/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the winbond flash device infomation.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01695b
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_size.h>
#include <snor.h>

#define WINBOND_CLK_104M            (104000000)

#define DUMMY_4_CLOCKS              4
#define DUMMY_6_CLOCKS              6
#define DUMMY_8_CLOCKS              8

/* Status Register 1 bits. */
#define WINBOND_SR_BP0      BIT(2)  /* Block protect 0 */
#define WINBOND_SR_BP1      BIT(3)  /* Block protect 1 */
#define WINBOND_SR_BP2      BIT(4)  /* Block protect 2 */
#define WINBOND_SR_TB       BIT(5)  /* Top/Bottom protect */
#define WINBOND_BP_MASK     (WINBOND_SR_BP2 | WINBOND_SR_BP1 | WINBOND_SR_BP0)
#define WINBOND_SR_BP_SHIFT 2

/* Status Register 2 bits. */
#define WINBOND_SR2_QUAD_EN    BIT(1)
#define WINBOND_SR2_LB1        BIT(3)  /* Security Register Lock Bit 1 */
#define WINBOND_SR2_LB2        BIT(4)  /* Security Register Lock Bit 2 */
#define WINBOND_SR2_LB3        BIT(5)  /* Security Register Lock Bit 3 */

/* flash commands */
#define WINBOND_CMD_ESECR         0x44    /* Erase Security registers */
#define WINBOND_CMD_PSECR         0x42    /* Program Security registers */
#define WINBOND_CMD_RSECR         0x48    /* Read Security registers */

static void winbond_get_locked_range(struct snor_dev *snor, u8 sr, u32 *ofs,
                                     u32 *len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val = (sr & WINBOND_BP_MASK) >> WINBOND_SR_BP_SHIFT;

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

static int winbond_lock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density, size = snor->info->size;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    winbond_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if ((ofs >= locked_offs) && ((ofs + len) <= (locked_offs + locked_len)))
        return AIC_SUCCESS;

    if ((ofs > (locked_offs + locked_len)) || ((ofs + len) < locked_offs))
        return AIC_EINVAL;

    lock_len = ofs + len;

    if (lock_len >= size) {
        val = WINBOND_BP_MASK;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            WINBOND_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(WINBOND_BP_MASK | WINBOND_SR_TB)) | (val | WINBOND_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static int winbond_unlock(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 lock_density = snor->info->lock_density;
    u8 val, sr, sr_new;
    u32 lock_len, locked_offs, locked_len;

    snor_read_sr1(snor, &sr);

    winbond_get_locked_range(snor, sr, &locked_offs, &locked_len);

    if (ofs >= (locked_offs + locked_len))
        return AIC_SUCCESS;

    lock_len = ofs;

    if (lock_len == 0) {
        val = 0;
    } else {
        lock_len = 1 << aic_find_last_bit(lock_len);
        val = (aic_find_last_bit(lock_len/lock_density) + 1) <<
                                                            WINBOND_SR_BP_SHIFT;
    }

    sr_new = (sr & ~(WINBOND_BP_MASK | WINBOND_SR_TB)) | (val | WINBOND_SR_TB);

    if (sr_new == sr)
        return AIC_SUCCESS;

    snor_write_sr1(snor, sr_new);

    return AIC_SUCCESS;
}

static boolean winbond_is_locked(struct snor_dev *snor, u32 ofs, u32 len)
{
    u32 locked_offs, locked_len, ofs_max, locked_max;
    u8 sr;

    snor_read_sr1(snor, &sr);

    winbond_get_locked_range(snor, sr, &locked_offs, &locked_len);

    ofs_max = ofs + len;
    locked_max = locked_offs + locked_len;

    return ((ofs >= locked_offs) && (ofs_max <= locked_max));
}

static void winbond_quad_enable(struct snor_dev *snor)
{
    u8 sr;

    snor_read_sr2(snor, &sr);
    if (sr & WINBOND_SR2_QUAD_EN)
        return;

    sr |= WINBOND_SR2_QUAD_EN;
    snor_write_sr2(snor, sr);
}

static struct snor_dev_ops s_winbond_dev_ops = {
    .quad_enable = winbond_quad_enable,
    .lock = winbond_lock,
    .unlock = winbond_unlock,
    .is_locked = winbond_is_locked,
};

static struct snor_erase_info s_winbond_erase[] = {
    {SNOR_CMD_SE_4K, SIZE_4K},
    {SNOR_CMD_BE_32K, SIZE_32K},
    {SNOR_CMD_BE_64K, SIZE_64K},
};

static struct snor_dev_info s_winbond_devs[] = {
    {0x6015, 0, 0, SIZE_2M, SIZE_4K, SIZE_256B, 3, WINBOND_CLK_104M,
             (SNOR_QUAD_READ | SNOR_QUAD_PP),
             READ_INFO(DUMMY_6_CLOCKS, 0), SIZE_64K,
             s_winbond_erase, ARRAY_SIZE(s_winbond_erase),
             &s_winbond_dev_ops}, /* w25q16jw */
    {0x6016, 0, 0, SIZE_4M, SIZE_4K, SIZE_256B, 3, WINBOND_CLK_104M,
             (SNOR_QUAD_READ | SNOR_QUAD_PP),
             READ_INFO(DUMMY_6_CLOCKS, 0), SIZE_64K,
             s_winbond_erase, ARRAY_SIZE(s_winbond_erase),
             &s_winbond_dev_ops}, /* w25q32jw */
    {0x6017, 0, 0, SIZE_8M, SIZE_4K, SIZE_256B, 3, WINBOND_CLK_104M,
             (SNOR_QUAD_READ | SNOR_QUAD_PP),
             READ_INFO(DUMMY_6_CLOCKS, 0), SIZE_128K,
             s_winbond_erase, ARRAY_SIZE(s_winbond_erase),
             &s_winbond_dev_ops}, /* w25q64jw */
};

static struct snor_dev_info* winbond_get_dev_info(struct snor_dev *snor, u16 did)
{
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_winbond_devs); i++) {
        if (s_winbond_devs[i].did == did)
            return &s_winbond_devs[i];
    }

    return NULL;
}

struct snor_vendor g_winbond = {
    .mid = 0xEF,
    .get_dev_info = winbond_get_dev_info,
};
