/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the spi nor flash hal driver.
 */

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <cache.h>
#include <flash_config.h>
#include <initcall.h>
#include <qspi_inc.h>
#include <snor.h>

/* all kinds timeout defined, unit:us */
#define SNOR_RESET_LATENCY_TIME        (60UL)
#define SNOR_WSR_TIMEOUT_TIME          (50000UL)
#define SNOR_WEL_TIMEOUT_TIME          (50000UL)

#define SNOR_PROG_TIMEOUT_TIME         (5000UL)
#define SNOR_BE_TIMEOUT_TIME           (2400000UL)
#define SNOR_CE_TIMEOUT_TIME           (50000000UL)

#define SNOR_PE_WORK_TIME              (5000UL)
#define SNOR_PE_SUS_IN_CONN_TIME       (200000UL)
#define SNOR_PE_SUS_IN_IDLE_TIME       (10000UL)

#define SNOR_PE_SUS_TO_NEXT_CMD_TIME   (60UL)
#define SNOR_PE_REM_TO_NEXT_SUS_TIME   (100UL)

#define SNOR_RDP_TIME                  (60UL)

static struct snor_vendor *s_vendors[] = {
    &g_gigadevice,
    &g_winbond,
    &g_xmc,
    &g_puya,
};

/*
 * snor_get_utime - use for us delay scenarios
 * Note that!!!
 * The return usec should be multiple of 30.518us.
 */
u64 snor_get_utime(void)
{
    u64 val;

    val = AIC_READ32(FREERUN_CNT_H);
    if (val)
        val = val << 32;
    val += AIC_READ32(FREERUN_CNT_L);

    val = val * TIMER_FREQ_32K;

    return val;
}

void snor_udelay(u32 usec)
{
    u64 start_time, current_time;

    if (usec < 60)
        usec = 60;

    start_time = snor_get_utime();

    do {
        current_time = snor_get_utime();
    } while ((current_time - start_time) < usec);
}

static void snor_prepare(struct snor_dev *snor)
{
    snor->phy_ops->prepare(snor);
}

static void snor_post(struct snor_dev *snor)
{
    snor->phy_ops->post(snor);
}

/**
 * spi_nor_read_sr1() - Read the Status Register1.
 * @nor:    pointer to 'struct snor_dev'.
 * @sr:     pointer to a buffer where the value of the
 *              Status Register will be written.
 *
 * Return: No return value on success, ASSERT on error.
 */
void snor_read_sr1(struct snor_dev *snor, u8 *sr)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->read_reg(snor, SNOR_CMD_RDSR, sr, 0, 1);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "Read SR1 error(%d)\n", ret);
}

/**
 * spi_nor_read_sr2() - Read the Status Register2 using the
 * SPINOR_OP_RDSR2 (35h) command.
 * @nor:    pointer to 'struct snor_dev'
 * @sr:     pointer to buffer where the value of the
 *          Status Register2 will be written.
 *
 * Return: No return value on success, ASSERT on error.
 */
void snor_read_sr2(struct snor_dev *snor, u8 *sr)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->read_reg(snor, SNOR_CMD_RDSR2, sr, 0, 1);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "Read SR2 error(%d)\n", ret);
}

static int snor_wait_pew_complete(struct snor_dev *snor, u64 timeout)
{
    u64 start_time, current_time;
    u8 sr;

    start_time = snor_get_utime();

    do {
        snor_read_sr1(snor, &sr);
        if (!(sr & SR_WIP))
            return AIC_SUCCESS;

        current_time = snor_get_utime();
    } while ((current_time - start_time) < timeout);

    return AIC_ETIMEDOUT;
}

static int snor_wait_wel_complete(struct snor_dev *snor, u64 timeout)
{
    u64 start_time, current_time;
    u8 sr;

    start_time = snor_get_utime();

    do {
        snor_read_sr1(snor, &sr);
        if (sr & SR_WEL)
            return AIC_SUCCESS;

        current_time = snor_get_utime();
    } while ((current_time - start_time) < timeout);

    return AIC_ETIMEDOUT;
}

static void snor_write_enable(struct snor_dev *snor)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_0;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_WREN, NULL, 0);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "write enable failed(%d)\n", ret);

    ret = snor_wait_wel_complete(snor, SNOR_WEL_TIMEOUT_TIME);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "wait write enable failed(%d)\n", ret);
}

static void snor_write_sr(struct snor_dev *snor, u8 *sr, u32 len)
{
    int ret;

    snor_write_enable(snor);

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_WRSR, sr, len);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "write sr failed(%d)\n", ret);

    ret = snor_wait_pew_complete(snor, SNOR_WSR_TIMEOUT_TIME);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS,
                    "wait write sr complete failed(%d)\n", ret);
}

void snor_write_sr1(struct snor_dev *snor, u8 sr1)
{
    u8 sr1_backup;

    snor_write_sr(snor, &sr1, 1);

    snor_read_sr1(snor, &sr1_backup);
    SNOR_ASSERT_INFO((sr1_backup == sr1), "sr1: check failed\n");
}

void snor_write_sr2(struct snor_dev *snor, u8 sr2)
{
    u8 sr[2];
    u8 sr1_backup;

    /* save the value of the status register 1 */
    snor_read_sr1(snor, &sr[0]);

    /* write value to status registers */
    sr[1] = sr2;
    snor_write_sr(snor, sr, 2);

    /* check SR1 */
    sr1_backup = sr[0];
    snor_read_sr1(snor, &sr[0]);
    SNOR_ASSERT_INFO((sr1_backup == sr[0]), "sr1: check failed\n");

    snor_read_sr2(snor, &sr[1]);
    SNOR_ASSERT_INFO((sr2 == sr[1]), "sr2: check failed\n");
}

static void snor_erase_chip(struct snor_dev *snor)
{
    int ret;

    snor->erase_stp = SNOR_STP_1_0_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->erase_stp = SNOR_STP_4_0_0_0_0;

    snor->erase_cmd = SNOR_CMD_CE;
    ret = snor->phy_ops->erase(snor, 0);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "erase chip error(%d)\n", ret);
}

static void snor_erase_total_chip(struct fmd_dev *fmd)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;
    SNOR_INT_SAVE;

    SNOR_INT_DISABLE;

    snor_prepare(snor);

    snor_write_enable(snor);

    snor_erase_chip(snor);

    ret = snor_wait_pew_complete(snor, SNOR_CE_TIMEOUT_TIME);

    snor_post(snor);

    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "erase total chip error(%d)!\n", ret);

    SNOR_INT_RESTORE;
}

static void snor_erase_single_block(struct snor_dev *snor, u32 addr)
{
    int ret;

    snor->erase_stp = SNOR_STP_1_1_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->erase_stp = SNOR_STP_4_4_0_0_0;

    ret = snor->phy_ops->erase(snor, addr);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "erase block error(%d)\n", ret);
}

static struct snor_erase_cmd *
snor_find_matched_erase_cmd(struct snor_erase_table *tab, u32 addr, u32 len)
{
    struct snor_erase_cmd *erase;
    u32 erase_mask = tab->erase_mask, i;

    while (erase_mask != 0) {
        i = aic_find_last_bit(erase_mask);
        erase = &tab->erase_cmd[i];

        if (erase->size > len) {
            erase_mask &= ~(BIT(i));
            continue;
        }

        if (!(addr & erase->size_mask))
            return erase;

        erase_mask &= ~(BIT(i));
    }

    return NULL;
}

static void snor_erase(struct fmd_dev *fmd, u32 addr, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    struct snor_erase_table *tab = &snor->erase_table;
    struct snor_erase_cmd *erase = NULL;
    int ret;
    SNOR_INT_SAVE;

    SNOR_INT_DISABLE;

    while (len) {
        erase = snor_find_matched_erase_cmd(tab, addr, len);
        SNOR_ASSERT_INFO(erase != NULL, "no erase cmd matched!\n");

        snor->erase_cmd = erase->cmd;
        snor->erase_size = erase->size;

        snor_prepare(snor);

        snor_write_enable(snor);

        snor_erase_single_block(snor, addr);

        ret = snor_wait_pew_complete(snor, SNOR_BE_TIMEOUT_TIME);

        snor_post(snor);

        SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "erase block error(%d)!\n", ret);

        addr += erase->size;
        len -= erase->size;
    }

    SNOR_INT_RESTORE;
}

static void snor_read(struct fmd_dev *dev, u32 from, u32 len, u8 *buf)
{
    struct snor_dev *snor = fmd_to_snor(dev);
    int ret;
    SNOR_INT_SAVE;

    SNOR_INT_DISABLE;

    while (len) {
        u32 addr = from;

        ret = snor->phy_ops->read(snor, addr, len, buf);

        SNOR_ASSERT_INFO((ret > 0), "snor read data error(%d)\n", ret);

        buf += ret;
        from += ret;
        len -= ret;
    }

    SNOR_INT_RESTORE;
}

static void snor_write(struct fmd_dev *dev, u32 to, u32 len, u8 *buf)
{
    struct snor_dev *snor = fmd_to_snor(dev);
    u32 i, page_offset, page_remain, page_size = snor->info->page_size;
    int ret;
    SNOR_INT_SAVE;

    SNOR_INT_DISABLE;

    i = 0;
    do {
        u32 written, addr;

        addr = to + i;
        page_offset = addr & (page_size - 1);
        page_remain = MIN(page_size - page_offset, len - i);
        memcpy(snor->buf, (buf + i), page_remain);

        snor_prepare(snor);

        snor_write_enable(snor);

        ret = snor->phy_ops->write(snor, addr, page_remain, (u8 *)(snor->buf));
        SNOR_ASSERT_INFO((ret > 0), "write data error(%d)\n", ret);
        written = ret;

        ret = snor_wait_pew_complete(snor, SNOR_PROG_TIMEOUT_TIME);

        snor_post(snor);

        SNOR_ASSERT_INFO(ret == AIC_SUCCESS,
                        "wait write complete timeout(%d)\n", ret);

        i += written;

        if (i >= len)
            break;

    } while (1);

    SNOR_INT_RESTORE;
}


static void snor_enter_read(struct fmd_dev *dev)
{
    struct snor_dev *snor = fmd_to_snor(dev);
    snor->phy_ops->enter_read(snor);
}

static void snor_set_default_command(struct snor_dev *snor)
{
    /* read command settings */
    snor->read_cmd = SNOR_CMD_READ_FAST;
    snor->read_stp = SNOR_STP_1_1_0_1_1;
    snor->read_dummy = 8;

    /* page program command settings. */
    snor->write_cmd = SNOR_CMD_PP;
    snor->write_stp = SNOR_STP_1_1_0_0_1;
}

static void snor_set_read_command(struct snor_dev *snor)
{
    if (snor->info->flags & SNOR_HAS_QPI_MODE) {
        snor->read_cmd = SNOR_CMD_READ_FAST;
        snor->read_stp = SNOR_STP_4_4_0_4_4;
        snor->read_dummy = snor->info->dummy_clocks;
    } else if (snor->info->flags & SNOR_QUAD_READ) {
        snor->read_cmd = SNOR_CMD_READ_QUAD_IO;
        snor->read_stp = SNOR_STP_1_4_0_4_4;
        snor->read_dummy = 6;
    }
}

static void snor_set_pp_command(struct snor_dev *snor)
{
    if (snor->info->flags & SNOR_HAS_QPI_MODE) {
        snor->write_cmd = SNOR_CMD_PP;
        snor->write_stp = SNOR_STP_4_4_0_0_4;
    } else if (snor->info->flags & SNOR_QUAD_PP) {
        snor->write_cmd = SNOR_CMD_PP_QUAD;
        snor->write_stp = SNOR_STP_1_1_0_0_4;
    }
}

static void snor_set_erase_table(struct snor_dev *snor)
{
    struct snor_erase_info *erase_info = snor->info->erase_info;
    int num = snor->info->erase_num;
    struct snor_erase_table *tab = &snor->erase_table;
    struct snor_erase_cmd *erase;
    u32 erase_mask = 0, i;

    if (num >= SNOR_ERASE_CMD_MAX)
        SNOR_ASSERT(0);

    for (i = 0; i < num; i++) {
        erase_mask |= BIT(i);

        erase = &tab->erase_cmd[i];
        erase->cmd = erase_info[i].cmd;
        erase->size = erase_info[i].size;
        erase->size_mask = erase->size - 1;
    }

    tab->erase_mask = erase_mask;
}

static void snor_soft_reset(struct snor_dev *snor)
{
    int ret;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_RSTEN, NULL, 0);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "reset enable failed(%d)\n", ret);

    snor_udelay(SNOR_RESET_LATENCY_TIME);

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_RST, NULL, 0);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "reset failed(%d)\n", ret);

    snor_udelay(SNOR_RESET_LATENCY_TIME);
}

static void snor_spi_mode_reset(struct snor_dev *snor)
{
    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    snor_soft_reset(snor);
}

static void snor_qpi_mode_reset(struct snor_dev *snor)
{
    snor->reg_stp = SNOR_STP_4_0_0_0_0;
    snor_soft_reset(snor);
}

static void snor_all_mode_reset(struct snor_dev *snor)
{
    snor_spi_mode_reset(snor);
    snor_qpi_mode_reset(snor);
}

static int snor_lock(struct fmd_dev *fmd, u32 ofs, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;

    if (!(snor->info->dev_ops->lock))
        return AIC_ENODEV;

    snor_prepare(snor);

    ret = snor->info->dev_ops->lock(snor, ofs, len);

    snor_post(snor);

    return ret;
}

static int snor_unlock(struct fmd_dev *fmd, u32 ofs, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;

    if (!(snor->info->dev_ops->unlock))
        return AIC_ENODEV;

    snor_prepare(snor);

    ret = snor->info->dev_ops->unlock(snor, ofs, len);

    snor_post(snor);

    return ret;
}

static int snor_is_locked(struct fmd_dev *fmd, u32 ofs, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;

    if (!(snor->info->dev_ops->is_locked))
        return AIC_ENODEV;

    snor_prepare(snor);

    ret = snor->info->dev_ops->is_locked(snor, ofs, len);

    snor_post(snor);

    return ret;
}

static void snor_unlock_total_chip(struct snor_dev *snor)
{
    int ret;

    ret = snor_unlock(&snor->fmd, 0, snor->info->size);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS,
                    "unlock the entire flash failed\n");
}

static void snor_get_id(struct snor_dev *snor, u8 *mid, u16 *did)
{
    u8 buf[3], *ptr;
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->read_reg(snor, SNOR_CMD_RDID, buf, 0, 3);
    SNOR_ASSERT_INFO(ret == AIC_SUCCESS, "reading JEDEC id error(%d)\n", ret);

    *mid = buf[0];
    ptr = (u8 *)did;
    ptr[0] = buf[2];
    ptr[1] = buf[1];
}

static struct snor_dev_info* snor_get_dev_info(struct snor_dev *snor)
{
    struct snor_dev_info *info;
    u8 mid;
    u16 did;
    u32 i;

    snor_get_id(snor, &mid, &did);

    for (i = 0; i < ARRAY_SIZE(s_vendors); i++) {
        if (mid == s_vendors[i]->mid) {
            info = s_vendors[i]->get_dev_info(snor, did);
            if (info) {
                snor->mid = mid;
                return info;
            }
        }
    }

    return NULL;
}

static u32 snor_get_flash_base(struct fmd_dev *fmd)
{
    struct snor_dev *snor = fmd_to_snor(fmd);

    return snor->phy_ops->get_flash_base(snor);
}

static void snor_dev_init(struct snor_dev *snor)
{
    struct fmd_dev *fmd = &snor->fmd;

    if (!(snor->info->dev_ops))
        return;

    /* enable Quad bit */
    if (snor->info->dev_ops->quad_enable)
        snor->info->dev_ops->quad_enable(snor);

    /* enter QPI mode */
    if (snor->info->dev_ops->qpi_enter)
        snor->info->dev_ops->qpi_enter(snor);

    /* register lock operations */
    fmd->_lock = snor_lock;
    fmd->_unlock = snor_unlock;
    fmd->_is_locked = snor_is_locked;
}

int snor_probe(struct snor_dev *snor)
{
    struct snor_dev_info *info = NULL;
    int ret = AIC_SUCCESS;

    /* set default read/write command. */
    snor_set_default_command(snor);

    /* reset flash */
    snor_all_mode_reset(snor);

    /* get flash info */
    info = snor_get_dev_info(snor);
    if (!info)
        ret = AIC_ENODEV;

    return ret;
}

void snor_setup(struct snor_dev *snor)
{
    struct snor_dev_info *info = NULL;
    struct fmd_dev *fmd = &snor->fmd;

    snor_prepare(snor);

    /* get flash info */
    info = snor_get_dev_info(snor);
    SNOR_ASSERT(info != NULL);

    snor->info = info;

    /* set flash command based on flash_info */
    snor_set_read_command(snor);
    snor_set_pp_command(snor);
    snor_set_erase_table(snor);

    fmd->priv = snor;
    fmd->devno = snor->cs;
    fmd->type = FMD_QSPI_NORFLASH;
    fmd->size = snor->info->size;
    fmd->erasesize = snor->info->min_erase_size;
    fmd->_erase = snor_erase;
    fmd->_read = snor_read;
    fmd->_write = snor_write;
    fmd->_get_mem_base = snor_get_flash_base;
    fmd->_enter_read = snor_enter_read;
    fmd->_erase_total_chip = snor_erase_total_chip;

    /* initialize flash device */
    snor_dev_init(snor);

    /* unlock all flash */
    snor_unlock_total_chip(snor);

    snor_post(snor);
}

int snor_init(void)
{
    struct qspi_cfg cfg = {0};

    cfg.cs_num = QSPI_NOR_DEVICE_NUM;
#if (QSPI_NOR_DEVICE_NUM == 2)
    cfg.dual_xip_mode = TRUE;
#endif

    qspi_init(&cfg);

    return AIC_SUCCESS;
}
driver_init(snor_init);
