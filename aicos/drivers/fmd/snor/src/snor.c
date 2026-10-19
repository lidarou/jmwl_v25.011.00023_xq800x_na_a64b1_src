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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0db19c
#define LOG_MODULE    AMOD_FLASH

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <aic_mcx.h>
#include <aic_osa.h>
#include <cache.h>
#include <irq.h>
#include <core_star.h>
#include <flash_config.h>
#include <flash_mem_config.h>
#include <qspi_inc.h>
#include <snor.h>

/* all kinds timeout defined, unit:us */
#define SNOR_RESET_LATENCY_TIME        (60UL)
#define SNOR_WSR_TIMEOUT_TIME          (50000UL)
#define SNOR_WEL_TIMEOUT_TIME          (50000UL)

#define SNOR_PROG_TIMEOUT_TIME         (5000UL)
#define SNOR_BE_TIMEOUT_TIME           (2400000UL)

#define SNOR_PE_WORK_TIME              (5000UL)
#define SNOR_PE_SUS_IN_CONN_TIME       (200000UL)
#define SNOR_PE_SUS_IN_IDLE_TIME       (10000UL)

#define SNOR_PE_SUS_TO_NEXT_CMD_TIME   (60UL)
#define SNOR_PE_REM_TO_NEXT_SUS_TIME   (100UL)

#define SNOR_RDP_TIME                  (60UL)

#ifdef CONFIG_MCX_SUPPORT
static aos_timer_ptr s_snor_mcx_timer;
static u64 s_snor_start_tick = 0;
static u64 s_snor_end_tick = 0;
static u64 s_snor_work_time = 0;
static u64 s_snor_to_start_tick = 0;
static u32 s_old_priority = 0;
#endif

static struct snor_vendor *s_vendors[] = {
    &g_gigadevice,
    &g_winbond,
    &g_xmc,
    &g_puya,
};

/* #define SNOR_DEBUG */
#ifdef SNOR_DEBUG
#define SNOR_DBG_MAX_POINT_NUM  150

struct snor_dbg_info {
    u64 time;
    u8 type;
    u8 event1;
    u16 event2;
    u64 event3;
};

static volatile struct snor_dbg_info snor_dbg_point[SNOR_DBG_MAX_POINT_NUM];
static u32 snor_dbg_point_ofs = 0;

void snor_dbg_point_record(u8 type, u8 e1, u16 e2, u64 e3)
{
    u64 val;
    u32 i;

    i = snor_dbg_point_ofs++;
    if (snor_dbg_point_ofs >= SNOR_DBG_MAX_POINT_NUM)
        snor_dbg_point_ofs = 0;

    snor_dbg_point[i].type = type;
    snor_dbg_point[i].event1 = e1;
    snor_dbg_point[i].event2 = e2;
    snor_dbg_point[i].event3 = e3;
    val = AIC_READ32(FREERUN_CNT_H);
    if (val)
        val = val << 32;
    val += AIC_READ32(FREERUN_CNT_L);
    snor_dbg_point[i].time = val;
}
#else
#define snor_dbg_point_record(a, b, c, d)
#endif

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

    val = val * 30;

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

static void snor_take_lock(struct snor_dev *snor)
{
    snor->phy_ops->take_lock(snor);
}

static void snor_release_lock(struct snor_dev *snor)
{
    snor->phy_ops->release_lock(snor);
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
    SNOR_ASSERT(ret == AIC_SUCCESS);
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
    SNOR_ASSERT(ret == AIC_SUCCESS);
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
    SNOR_ASSERT(ret == AIC_SUCCESS);

    ret = snor_wait_wel_complete(snor, SNOR_WEL_TIMEOUT_TIME);
    SNOR_ASSERT(ret == AIC_SUCCESS);
}

static void snor_write_sr(struct snor_dev *snor, u8 *sr, u32 len)
{
    int ret;

    snor_write_enable(snor);

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_WRSR, sr, len);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    ret = snor_wait_pew_complete(snor, SNOR_WSR_TIMEOUT_TIME);
    SNOR_ASSERT(ret == AIC_SUCCESS);
}

void snor_write_sr1(struct snor_dev *snor, u8 sr1)
{
    u8 sr1_backup;

    snor_write_sr(snor, &sr1, 1);

    snor_read_sr1(snor, &sr1_backup);
    SNOR_ASSERT(sr1_backup == sr1);
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
    SNOR_ASSERT(sr1_backup == sr[0]);

    snor_read_sr2(snor, &sr[1]);
    SNOR_ASSERT(sr2 == sr[1]);
}

static void snor_program_erase_suspend(struct snor_dev *snor)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_0;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_PE_SUSPEND, NULL, 0);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    /* delay specific ticks to make sure flash can handle other
     * command correctly after suspend operation */
    snor_udelay(SNOR_PE_SUS_TO_NEXT_CMD_TIME);

    /* check if erase/program really suppended */
    ret = snor_wait_pew_complete(snor, SNOR_WSR_TIMEOUT_TIME);
    SNOR_ASSERT(ret == AIC_SUCCESS);
}

static void snor_program_erase_resume(struct snor_dev *snor)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_0;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_PE_RESUME, NULL, 0);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    /* delay specific ticks to make sure flash can handle next syspend
     * command correctly after resume operation */
    snor_udelay(SNOR_PE_REM_TO_NEXT_SUS_TIME);
}

#ifdef CONFIG_MCX_SUPPORT
static void snor_improve_thread_pri(u32 *old_priority)
{
    u32 new_priority = 3;

    if (!aos_is_user_thread_context() || !old_priority)
        return;

    aos_change_current_thread_pri(new_priority, old_priority);
}

static void snor_restore_thread_pri(u32 priority)
{
    u32 tmp_priority;

    if (!aos_is_user_thread_context())
        return;

    aos_change_current_thread_pri(priority, &tmp_priority);
}

static void snor_suspend_resume_handle(struct snor_dev *snor, u64 delay_time,
                                          boolean need)
{
    AIC_INT_SAVE_AREA;
    u64 delay_start_time;

    if (need) {
        snor_program_erase_suspend(snor);
        snor_post(snor);
    }

    MCX_RESUME(MCX_WAIT_ASYNC);

    /* record delay start time. */
    delay_start_time = snor_get_utime();

    /* save previous irq status and enable irq directly. */
    AIC_GET_INT_STAUTS
    AIC_INT_ENABLE

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);

    /* suspend specific time for mcx task execution, during this time
     * ap can handle interrupts and run other tasks. */
    while ((snor_get_utime() - delay_start_time) < delay_time) {}

    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

#ifdef CONFIG_MCX_SUPPORT
    /* don't give up cpu, loop for CP ACK. */
    MCX_SUSPEND(MCX_WAIT_SYNC);
#endif

    /* restore previous irq status. */
    AIC_INT_RESTORE

    if (need) {
        snor_prepare(snor);
        snor_program_erase_resume(snor);
    }
}
#endif

static boolean snor_is_pending_interrupts_exist(void)
{
    return ((SCB->ICSR & SCB_ICSR_VECTPENDING_Msk) >> SCB_ICSR_VECTPENDING_Pos);
}

static void snor_pending_interrupts_handle(struct snor_dev *snor)
{
    u32 int_save;

    snor_program_erase_suspend(snor);
    snor_post(snor);

    int_save = __get_PRIMASK();
    enable_interrupts();

    /* handle pending interrupts */
    __asm__ volatile ("isb 0xF":::"memory");

    restore_interrupts(int_save);

    snor_prepare(snor);
    snor_program_erase_resume(snor);
}

#ifdef CONFIG_MCX_SUPPORT
static int snor_check_pe_status(struct snor_dev *snor, u64 timeout)
{
    u64 start_tick, work_start_tick, work_end_tick, work_time, suspend_time;
    u64 interrupt_start_tick, interrupt_end_tick, interrupt_work_time;
    u8 sr;

    start_tick = snor_get_utime();
    work_start_tick = s_snor_start_tick;
    work_time = suspend_time = 0;
    interrupt_work_time = 0;

    while (1) {
        snor_read_sr1(snor, &sr);
        if (!(sr & SR_WIP))
            return AIC_SUCCESS;

        /* check if there is interrupt need to be handled. */
        interrupt_start_tick = snor_get_utime();
        if (snor_is_pending_interrupts_exist())
            snor_pending_interrupts_handle(snor);
        interrupt_end_tick = snor_get_utime();
        interrupt_work_time += (interrupt_end_tick - interrupt_start_tick);
#ifndef CONFIG_CHIP_FPGA
        if ((interrupt_end_tick - interrupt_start_tick) >=
            (SNOR_PE_WORK_TIME * 2)) {
            snor_dbg_point_record(0xC0, 0, 0,
                                  (interrupt_end_tick - interrupt_start_tick));
            return AIC_ENOTSUP;
        }
#endif

        work_end_tick = snor_get_utime();
        if ((work_end_tick - start_tick) >=
                            (timeout + suspend_time + interrupt_work_time)) {
            snor_dbg_point_record(0xC1, 0, 0, interrupt_work_time);
            snor_dbg_point_record(0xC2, 0, 0, suspend_time);
            snor_dbg_point_record(0xC3, 0, 0, (work_end_tick - start_tick));
            break;
        }

        work_time = (work_end_tick - work_start_tick);

        /* work for sepecific time */
        if (work_time >= SNOR_PE_WORK_TIME) {
            u64 delay_time;

            snor_dbg_point_record(0xC4, 0, 0, work_time);

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            snor_suspend_resume_handle(snor, delay_time, TRUE);

            work_start_tick = snor_get_utime();
            s_snor_start_tick = work_start_tick;
            suspend_time += (work_start_tick - work_end_tick);
        }
    }

    return AIC_ETIMEDOUT;
}
#else
static int snor_check_pe_status(struct snor_dev *snor, u64 timeout)
{
    u64 work_start_tick, work_end_tick;
    u64 interrupt_start_tick, interrupt_end_tick, interrupt_work_time;
    u8 sr;

    work_start_tick = snor_get_utime();
    interrupt_work_time = 0;

    while (1) {
        snor_read_sr1(snor, &sr);
        if (!(sr & SR_WIP))
            return AIC_SUCCESS;

        /* check if there is interrupt need to be handled. */
        interrupt_start_tick = snor_get_utime();
        if (snor_is_pending_interrupts_exist())
            snor_pending_interrupts_handle(snor);
        interrupt_end_tick = snor_get_utime();
        interrupt_work_time += (interrupt_end_tick - interrupt_start_tick);

        work_end_tick = snor_get_utime();
        if ((work_end_tick - work_start_tick) >=
                            (timeout + interrupt_work_time))
            break;
    }

    return AIC_ETIMEDOUT;
}
#endif

static void snor_erase_single_block(struct snor_dev *snor, u32 addr)
{
    int ret;

    snor->erase_stp = SNOR_STP_1_1_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->erase_stp = SNOR_STP_4_4_0_0_0;

    ret = snor->phy_ops->erase(snor, addr);
    SNOR_ASSERT(ret == AIC_SUCCESS);
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

    snor_take_lock(snor);

#ifdef CONFIG_MCX_SUPPORT
    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

    SNOR_INT_DISABLE;

    snor_dbg_point_record(MCX_DEBUG_ERASE_ENTER, 0, 0, len);

    if (mcx_is_eb()) {
        if (s_snor_to_start_tick != 0) {
            u64 delay_time, delay_start_time, delta_time;

            delta_time = snor_get_utime() - s_snor_to_start_tick;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            if (delta_time < delay_time) {
                delay_time -= delta_time;

                /* record delay start time. */
                delay_start_time = snor_get_utime();

                SNOR_INT_RESTORE;

                snor_restore_thread_pri(s_old_priority);

                /* suspend specific time for mcx task execution, during this
                 * time ap can handle interrupts and run other tasks. */
                while ((snor_get_utime() - delay_start_time) < delay_time) {}

                snor_improve_thread_pri(&s_old_priority);

                SNOR_INT_DISABLE;
            }

            s_snor_to_start_tick = 0;
        }

        if (s_snor_start_tick == 0) {
            SNOR_INT_RESTORE;
            MCX_SUSPEND(MCX_WAIT_SYNC);
            SNOR_INT_DISABLE;

            s_snor_start_tick = snor_get_utime();
        } else {
            /* deactive notify timer */
            aos_deactivate_timer(s_snor_mcx_timer);
        }

        snor_dbg_point_record(0xD0, 0, 0, s_snor_start_tick);
    }
#else
    SNOR_INT_DISABLE;
#endif

    while (len) {
        erase = snor_find_matched_erase_cmd(tab, addr, len);
        SNOR_ASSERT(erase != NULL);

        snor->erase_cmd = erase->cmd;
        snor->erase_size = erase->size;

        snor_prepare(snor);

        snor_write_enable(snor);

        snor_erase_single_block(snor, addr);

#ifdef CONFIG_MCX_SUPPORT
        if (mcx_is_eb())
            ret = snor_check_pe_status(snor, SNOR_BE_TIMEOUT_TIME);
        else
            ret = snor_wait_pew_complete(snor, SNOR_BE_TIMEOUT_TIME);
#else
        ret = snor_check_pe_status(snor, SNOR_BE_TIMEOUT_TIME);
#endif

        snor_post(snor);

        SNOR_ASSERT(ret == AIC_SUCCESS);

        addr += erase->size;
        len -= erase->size;
    }

#ifdef CONFIG_MCX_SUPPORT
    if (mcx_is_eb() && s_snor_start_tick != 0) {
        s_snor_end_tick = snor_get_utime();
        s_snor_work_time = s_snor_end_tick - s_snor_start_tick;

        /* work time compensation 1ms for boundary processing. */
        if ((s_snor_work_time + 1000) >= SNOR_PE_WORK_TIME) {
            u64 delay_time, delay_start_time;

            s_snor_start_tick = 0;
            s_snor_end_tick = 0;
            s_snor_work_time = 0;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            MCX_RESUME(MCX_WAIT_ASYNC);

            /* record delay start time. */
            delay_start_time = snor_get_utime();

            SNOR_INT_RESTORE;

            snor_restore_thread_pri(s_old_priority);

            /* suspend specific time for mcx task execution, during this time
             * ap can handle interrupts and run other tasks. */
            while ((snor_get_utime() - delay_start_time) < delay_time) {}

            snor_improve_thread_pri(&s_old_priority);

            SNOR_INT_DISABLE;
        } else {
            u32 timeout = (SNOR_PE_WORK_TIME - s_snor_work_time) / 1000;

            /* active notify timer */
            aos_change_timer(s_snor_mcx_timer, timeout, AOS_NO_PERIOD);
            snor_dbg_point_record(0xD2, 0, 0, timeout);
        }

        snor_dbg_point_record(0xD3, 0, 0, s_snor_end_tick);
    }

    snor_dbg_point_record(MCX_DEBUG_ERASE_EXIT, 0, 0, s_snor_start_tick);

    SNOR_INT_RESTORE;

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);
#else
    SNOR_INT_RESTORE;
#endif

    snor_release_lock(snor);
}

static void snor_read(struct fmd_dev *dev, u32 from, u32 len, u8 *buf)
{
    struct snor_dev *snor = fmd_to_snor(dev);
    int ret;
    SNOR_INT_SAVE;

    if (dev->type != FMD_QSPI_NORFLASH)
        snor_take_lock(snor);

    SNOR_INT_DISABLE;

#ifdef CONFIG_MCX_SUPPORT
    //snor_dbg_point_record(MCX_DEBUG_READ_ENTER, 0, 0, len);
#endif

    while (len) {
        u32 addr = from;

        ret = snor->phy_ops->read(snor, addr, len, buf);

        SNOR_ASSERT(ret > 0);

        buf += ret;
        from += ret;
        len -= ret;

        if (len <= 0)
            break;

        SNOR_INT_RESTORE;
        __asm__ volatile ("isb 0xF":::"memory");
        SNOR_INT_DISABLE;
    }

#ifdef CONFIG_MCX_SUPPORT
    //snor_dbg_point_record(MCX_DEBUG_READ_EXIT, 0, 0, len);
#endif

    SNOR_INT_RESTORE;

    if (dev->type != FMD_QSPI_NORFLASH)
        snor_release_lock(snor);
}

static void snor_write(struct fmd_dev *dev, u32 to, u32 len, u8 *buf)
{
    struct snor_dev *snor = fmd_to_snor(dev);
    u32 i, page_offset, page_remain, page_size = snor->info->page_size;
    int ret;
    SNOR_INT_SAVE;

    snor_take_lock(snor);

#ifdef CONFIG_MCX_SUPPORT
    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

    SNOR_INT_DISABLE;

    snor_dbg_point_record(MCX_DEBUG_WRITE_ENTER, 0, 0, len);

    if (mcx_is_eb()) {
        if (s_snor_to_start_tick != 0) {
            u64 delay_time, delay_start_time, delta_time;

            delta_time = snor_get_utime() - s_snor_to_start_tick;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            if (delta_time < delay_time) {
                delay_time -= delta_time;

                /* record delay start time. */
                delay_start_time = snor_get_utime();

                SNOR_INT_RESTORE;

                snor_restore_thread_pri(s_old_priority);

                /* suspend specific time for mcx task execution, during this
                 * time ap can handle interrupts and run other tasks. */
                while ((snor_get_utime() - delay_start_time) < delay_time) {}

                snor_improve_thread_pri(&s_old_priority);

                SNOR_INT_DISABLE;
            }

            s_snor_to_start_tick = 0;
        }

        if (s_snor_start_tick == 0) {
            SNOR_INT_RESTORE;
            MCX_SUSPEND(MCX_WAIT_SYNC);
            SNOR_INT_DISABLE;

            s_snor_start_tick = snor_get_utime();
        } else {
            /* deactive notify timer */
            aos_deactivate_timer(s_snor_mcx_timer);
        }

        snor_dbg_point_record(0xD0, 0, 0, s_snor_start_tick);
    }
#else
    SNOR_INT_DISABLE;
#endif

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
        SNOR_ASSERT(ret > 0);
        written = ret;

#ifdef CONFIG_MCX_SUPPORT
        if (mcx_is_eb())
            ret = snor_check_pe_status(snor, SNOR_PROG_TIMEOUT_TIME);
        else
            ret = snor_wait_pew_complete(snor, SNOR_PROG_TIMEOUT_TIME);
#else
        ret = snor_check_pe_status(snor, SNOR_PROG_TIMEOUT_TIME);
#endif

        snor_post(snor);

        SNOR_ASSERT(ret == AIC_SUCCESS);

        i += written;

        if (i >= len)
            break;

    } while (1);

#ifdef CONFIG_MCX_SUPPORT
    if (mcx_is_eb() && s_snor_start_tick != 0) {
        s_snor_end_tick = snor_get_utime();
        s_snor_work_time = s_snor_end_tick - s_snor_start_tick;

        /* work time compensation 1ms for boundary processing. */
        if ((s_snor_work_time + 1000) >= SNOR_PE_WORK_TIME) {
            u64 delay_time, delay_start_time;

            s_snor_start_tick = 0;
            s_snor_end_tick = 0;
            s_snor_work_time = 0;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            MCX_RESUME(MCX_WAIT_ASYNC);

            /* record delay start time. */
            delay_start_time = snor_get_utime();

            SNOR_INT_RESTORE;

            snor_restore_thread_pri(s_old_priority);

            /* suspend specific time for mcx task execution, during this
             * time ap can handle interrupts and run other tasks. */
            while ((snor_get_utime() - delay_start_time) < delay_time) {}

            snor_improve_thread_pri(&s_old_priority);

            SNOR_INT_DISABLE;
        } else {
            u32 timeout = (SNOR_PE_WORK_TIME - s_snor_work_time) / 1000;

            /* active notify timer */
            aos_change_timer(s_snor_mcx_timer, timeout, AOS_NO_PERIOD);
            snor_dbg_point_record(0xD2, 0, 0, timeout);
        }

        snor_dbg_point_record(0xD3, 0, 0, s_snor_end_tick);
    }

    snor_dbg_point_record(MCX_DEBUG_WRITE_EXIT, 0, 0, s_snor_start_tick);

    SNOR_INT_RESTORE;

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);
#else
    SNOR_INT_RESTORE;
#endif

    snor_release_lock(snor);
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
    SNOR_ASSERT(ret == AIC_SUCCESS);

    snor_udelay(SNOR_RESET_LATENCY_TIME);

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_RST, NULL, 0);
    SNOR_ASSERT(ret == AIC_SUCCESS);

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
    SNOR_INT_SAVE;

    if (!(snor->info->dev_ops->lock))
        return AIC_ENODEV;

    snor_take_lock(snor);

#ifdef CONFIG_MCX_SUPPORT
    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

    SNOR_INT_DISABLE;

    snor_dbg_point_record(MCX_DEBUG_LOCK_ENTER, 0, 0, len);

    if (mcx_is_eb()) {
        if (s_snor_to_start_tick != 0) {
            u64 delay_time, delay_start_time, delta_time;

            delta_time = snor_get_utime() - s_snor_to_start_tick;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            if (delta_time < delay_time) {
                delay_time -= delta_time;

                /* record delay start time. */
                delay_start_time = snor_get_utime();

                SNOR_INT_RESTORE;

                snor_restore_thread_pri(s_old_priority);

                /* suspend specific time for mcx task execution, during this
                 * time ap can handle interrupts and run other tasks. */
                while ((snor_get_utime() - delay_start_time) < delay_time) {}

                snor_improve_thread_pri(&s_old_priority);

                SNOR_INT_DISABLE;
            }

            s_snor_to_start_tick = 0;
        }

        if (s_snor_start_tick == 0) {
            SNOR_INT_RESTORE;
            MCX_SUSPEND(MCX_WAIT_SYNC);
            SNOR_INT_DISABLE;

            s_snor_start_tick = snor_get_utime();
            snor_dbg_point_record(0xD0, 0, 0, s_snor_start_tick);
        } else {
            /* deactive notify timer */
            aos_deactivate_timer(s_snor_mcx_timer);
        }

        snor_dbg_point_record(0xD1, 0, 0, s_snor_start_tick);
    }
#else
    SNOR_INT_DISABLE;
#endif

    snor_prepare(snor);

    ret = snor->info->dev_ops->lock(snor, ofs, len);

    snor_post(snor);

#ifdef CONFIG_MCX_SUPPORT
    if (mcx_is_eb() && s_snor_start_tick != 0) {
        s_snor_end_tick = snor_get_utime();
        s_snor_work_time = s_snor_end_tick - s_snor_start_tick;

        /* work time compensation 1ms for boundary processing. */
        if ((s_snor_work_time + 1000) >= SNOR_PE_WORK_TIME) {
            u64 delay_time, delay_start_time;

            s_snor_start_tick = 0;
            s_snor_end_tick = 0;
            s_snor_work_time = 0;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            MCX_RESUME(MCX_WAIT_ASYNC);

            /* record delay start time. */
            delay_start_time = snor_get_utime();

            SNOR_INT_RESTORE;

            snor_restore_thread_pri(s_old_priority);

            /* suspend specific time for mcx task execution, during this
             * time ap can handle interrupts and run other tasks. */
            while ((snor_get_utime() - delay_start_time) < delay_time) {}

            snor_improve_thread_pri(&s_old_priority);

            SNOR_INT_DISABLE;
        } else {
            u32 timeout = (SNOR_PE_WORK_TIME - s_snor_work_time) / 1000;

            /* active notify timer */
            aos_change_timer(s_snor_mcx_timer, timeout, AOS_NO_PERIOD);
            snor_dbg_point_record(0xD2, 0, 0, timeout);
        }

        snor_dbg_point_record(0xD3, 0, 0, s_snor_end_tick);
    }

    snor_dbg_point_record(MCX_DEBUG_LOCK_EXIT, 0, 0, s_snor_start_tick);

    SNOR_INT_RESTORE;

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);
#else
    SNOR_INT_RESTORE;
#endif

    snor_release_lock(snor);

    return ret;
}

static int snor_unlock(struct fmd_dev *fmd, u32 ofs, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;
    SNOR_INT_SAVE;

    if (!(snor->info->dev_ops->unlock))
        return AIC_ENODEV;

    snor_take_lock(snor);

#ifdef CONFIG_MCX_SUPPORT
    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

    SNOR_INT_DISABLE;

    snor_dbg_point_record(MCX_DEBUG_UNLOCK_ENTER, 0, 0, len);

    if (mcx_is_eb()) {
        if (s_snor_to_start_tick != 0) {
            u64 delay_time, delay_start_time, delta_time;

            delta_time = snor_get_utime() - s_snor_to_start_tick;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            if (delta_time < delay_time) {
                delay_time -= delta_time;

                /* record delay start time. */
                delay_start_time = snor_get_utime();

                SNOR_INT_RESTORE;

                snor_restore_thread_pri(s_old_priority);

                /* suspend specific time for mcx task execution, during this
                 * time ap can handle interrupts and run other tasks. */
                while ((snor_get_utime() - delay_start_time) < delay_time) {}

                snor_improve_thread_pri(&s_old_priority);

                SNOR_INT_DISABLE;
            }

            s_snor_to_start_tick = 0;
        }

        if (s_snor_start_tick == 0) {
            SNOR_INT_RESTORE;
            MCX_SUSPEND(MCX_WAIT_SYNC);
            SNOR_INT_DISABLE;

            s_snor_start_tick = snor_get_utime();
            snor_dbg_point_record(0xD0, 0, 0, s_snor_start_tick);
        } else {
            /* deactive notify timer */
            aos_deactivate_timer(s_snor_mcx_timer);
        }

        snor_dbg_point_record(0xD1, 0, 0, s_snor_start_tick);
    }
#else
    SNOR_INT_DISABLE;
#endif

    snor_prepare(snor);

    ret = snor->info->dev_ops->unlock(snor, ofs, len);

    snor_post(snor);

#ifdef CONFIG_MCX_SUPPORT
    if (mcx_is_eb() && s_snor_start_tick != 0) {
        s_snor_end_tick = snor_get_utime();
        s_snor_work_time = s_snor_end_tick - s_snor_start_tick;

        /* work time compensation 1ms for boundary processing. */
        if ((s_snor_work_time + 1000) >= SNOR_PE_WORK_TIME) {
            u64 delay_time, delay_start_time;

            s_snor_start_tick = 0;
            s_snor_end_tick = 0;
            s_snor_work_time = 0;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            MCX_RESUME(MCX_WAIT_ASYNC);

            /* record delay start time. */
            delay_start_time = snor_get_utime();

            SNOR_INT_RESTORE;

            snor_restore_thread_pri(s_old_priority);

            /* suspend specific time for mcx task execution, during this
             * time ap can handle interrupts and run other tasks. */
            while ((snor_get_utime() - delay_start_time) < delay_time) {}

            snor_improve_thread_pri(&s_old_priority);

            SNOR_INT_DISABLE;
        } else {
            u32 timeout = (SNOR_PE_WORK_TIME - s_snor_work_time) / 1000;

            /* active notify timer */
            aos_change_timer(s_snor_mcx_timer, timeout, AOS_NO_PERIOD);
            snor_dbg_point_record(0xD2, 0, 0, timeout);
        }

        snor_dbg_point_record(0xD3, 0, 0, s_snor_end_tick);
    }

    snor_dbg_point_record(MCX_DEBUG_UNLOCK_EXIT, 0, 0, s_snor_start_tick);

    SNOR_INT_RESTORE;

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);
#else
    SNOR_INT_RESTORE;
#endif

    snor_release_lock(snor);

    return ret;
}

static int snor_is_locked(struct fmd_dev *fmd, u32 ofs, u32 len)
{
    struct snor_dev *snor = fmd_to_snor(fmd);
    int ret;
    SNOR_INT_SAVE;

    if (!(snor->info->dev_ops->is_locked))
        return AIC_ENODEV;

    snor_take_lock(snor);

#ifdef CONFIG_MCX_SUPPORT
    /* improve the priority of the current thread */
    snor_improve_thread_pri(&s_old_priority);

    SNOR_INT_DISABLE;

    snor_dbg_point_record(MCX_DEBUG_IS_LOCK_ENTER, 0, 0, len);

    if (mcx_is_eb()) {
        if (s_snor_to_start_tick != 0) {
            u64 delay_time, delay_start_time, delta_time;

            delta_time = snor_get_utime() - s_snor_to_start_tick;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            if (delta_time < delay_time) {
                delay_time -= delta_time;

                /* record delay start time. */
                delay_start_time = snor_get_utime();

                SNOR_INT_RESTORE;

                snor_restore_thread_pri(s_old_priority);

                /* suspend specific time for mcx task execution, during this
                 * time ap can handle interrupts and run other tasks. */
                while ((snor_get_utime() - delay_start_time) < delay_time) {}

                snor_improve_thread_pri(&s_old_priority);

                SNOR_INT_DISABLE;
            }

            s_snor_to_start_tick = 0;
        }

        if (s_snor_start_tick == 0) {
            SNOR_INT_RESTORE;
            MCX_SUSPEND(MCX_WAIT_SYNC);
            SNOR_INT_DISABLE;

            s_snor_start_tick = snor_get_utime();
            snor_dbg_point_record(0xD0, 0, 0, s_snor_start_tick);
        } else {
            /* deactive notify timer */
            aos_deactivate_timer(s_snor_mcx_timer);
        }

        snor_dbg_point_record(0xD1, 0, 0, s_snor_start_tick);
    }
#else
    SNOR_INT_DISABLE;
#endif

    snor_prepare(snor);

    ret = snor->info->dev_ops->is_locked(snor, ofs, len);

    snor_post(snor);

#ifdef CONFIG_MCX_SUPPORT
    if (mcx_is_eb() && s_snor_start_tick != 0) {
        s_snor_end_tick = snor_get_utime();
        s_snor_work_time = s_snor_end_tick - s_snor_start_tick;

        /* work time compensation 1ms for boundary processing. */
        if ((s_snor_work_time + 1000) >= SNOR_PE_WORK_TIME) {
            u64 delay_time, delay_start_time;

            s_snor_start_tick = 0;
            s_snor_end_tick = 0;
            s_snor_work_time = 0;

            if (MCX_IS_CP_IN_CONNECT())
                delay_time = SNOR_PE_SUS_IN_CONN_TIME;
            else
                delay_time = SNOR_PE_SUS_IN_IDLE_TIME;

            MCX_RESUME(MCX_WAIT_ASYNC);

            /* record delay start time. */
            delay_start_time = snor_get_utime();

            SNOR_INT_RESTORE;

            snor_restore_thread_pri(s_old_priority);

            /* suspend specific time for mcx task execution, during this
             * time ap can handle interrupts and run other tasks. */
            while ((snor_get_utime() - delay_start_time) < delay_time) {}

            snor_improve_thread_pri(&s_old_priority);

            SNOR_INT_DISABLE;
        } else {
            u32 timeout = (SNOR_PE_WORK_TIME - s_snor_work_time) / 1000;

            /* active notify timer */
            aos_change_timer(s_snor_mcx_timer, timeout, AOS_NO_PERIOD);
            snor_dbg_point_record(0xD2, 0, 0, timeout);
        }

        snor_dbg_point_record(0xD3, 0, 0, s_snor_end_tick);
    }

    snor_dbg_point_record(MCX_DEBUG_IS_LOCK_EXIT, 0, 0, s_snor_start_tick);

    SNOR_INT_RESTORE;

    /* restore the priority of the current thread */
    snor_restore_thread_pri(s_old_priority);
#else
    SNOR_INT_RESTORE;
#endif

    snor_release_lock(snor);

    return ret;
}

static void snor_unlock_total_chip(struct snor_dev *snor)
{
    int ret;

    if (!(snor->info->dev_ops->unlock))
        SNOR_ASSERT(0);

    snor_prepare(snor);
    ret = snor->info->dev_ops->unlock(snor, 0, snor->info->size);
    snor_post(snor);
    SNOR_ASSERT(ret == AIC_SUCCESS);
}

static void snor_get_id(struct snor_dev *snor, u8 *mid, u16 *did)
{
    u8 buf[3], *ptr;
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_1;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_4;

    ret = snor->phy_ops->read_reg(snor, SNOR_CMD_RDID, buf, 0, 3);
    SNOR_ASSERT(ret == AIC_SUCCESS);

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

    /* initialize flash device */
    snor_dev_init(snor);

    snor_post(snor);

    /* unlock all flash */
    snor_unlock_total_chip(snor);
}

void snor_sleep_resume(struct snor_dev *snor)
{
    int ret;

    snor->reg_stp = SNOR_STP_1_0_0_0_0;
    if (snor->work_mode == SNOR_QPI_MODE)
        snor->reg_stp = SNOR_STP_4_0_0_0_0;

    ret = snor->phy_ops->write_reg(snor, SNOR_CMD_RPD, NULL, 0);
    SNOR_ASSERT(ret == AIC_SUCCESS);

    snor_udelay(SNOR_RDP_TIME);
}

#ifdef CONFIG_MCX_SUPPORT
static void snor_mcx_timer_expiration_func(void *input)
{
    s_snor_start_tick = 0;
    s_snor_end_tick = 0;
    s_snor_work_time = 0;
    MCX_FORCE_RESUME;
    s_snor_to_start_tick = snor_get_utime();
}

int snor_mcx_init(void)
{
    /* snor mcx timer */
    s_snor_mcx_timer = aos_create_timer("smt", snor_mcx_timer_expiration_func,
                                        NULL, SNOR_PE_WORK_TIME/1000,
                                        AOS_NO_ACTIVATE, AOS_TRUE);
    SNOR_ASSERT(s_snor_mcx_timer != AOS_NULL);

    return AIC_SUCCESS;
}
AIC_DRV_P1_INIT_EXPORT(snor_mcx_init);
#endif

void snor_init(void)
{
    struct qspi_cfg cfg = {0};

    cfg.cs_num = QSPI_NOR_DEVICE_NUM;
#if (QSPI_NOR_DEVICE_NUM == 2)
    cfg.dual_xip_mode = TRUE;
    cfg.flash1_size = FLASH1_TOTAL_SIZE;
#endif

    qspi_init(&cfg);
}
