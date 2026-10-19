/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Drivers of soc spinlock.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0e04d2
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_type.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_io.h>
#include <reg_base.h>
#include <top_glb.h>
#include <spinlock.h>
#include <timer_inc.h>
#include <aic_auto_reg.h>

#define MOD_EN_MASK             0x1

#define LOCK_NUM                32
#define REG_BASE                REG_BASE_TOP_SPINLOCK
#define MST_REG(N)              (REG_BASE + 0x4 * (N))
#define LOCK_REG(N)             (REG_BASE + 0x40 + 0x4 *(N))
#define STS_REG(N)              (REG_BASE + 0x200 + 0x4 *(N))
#define STS_MST_SHIFT           0x18
#define STS_MST_MASK            0xff000000
#define STS_LOCK_MASK           BIT_0

#define MST_EN                  BIT_31
#define MST_MASK                0xff
#define UNLOCK_KEY              0x5a5a5a5a

#define MAX_MST_NUM             8
/* only below mast can get spinlock */
#define MSTID_CSTAR             0x1
#define MSTID_CR5               0x20

/* Retry interval if failed to get lock, in ms */
#define RELAX_TIME              5

#define __SPINLOCK_CODE __attribute__((section(".spinlock_code")))

static BOOL __is_spinlock_enabled(void)
{
    if (AIC_READ32(SPINLOCK_EN_REG) & MOD_EN_MASK)
        return AIC_TRUE;
    else
        return AIC_FALSE;
}

static u32 __get_spinlock_sts(LOCK_ID_T lockid)
{
    return AIC_READ32(STS_REG(lockid)) & STS_LOCK_MASK;
}

u32 spinlock_sts(LOCK_ID_T lockid)
{
    return __get_spinlock_sts(lockid);
}

static u32 __get_spinlock_masterid(LOCK_ID_T lockid)
{

    return (AIC_READ32(STS_REG(lockid)) & STS_MST_MASK) >> STS_MST_SHIFT ;
}

u32 spinlock_masterid(LOCK_ID_T lockid)
{

    return __get_spinlock_masterid(lockid);
}

static u32 __try_lock(LOCK_ID_T lockid)
{
    return AIC_READ32(LOCK_REG(lockid));
}

void spinlock_set_master(u32 index, u32 mst_id, bool en)
{
    if (index < MAX_MST_NUM)
        AIC_WRITE32(MST_REG(index), ((en ? 0x80000000:0x0) & MST_EN) | (mst_id & MST_MASK));
    return;
}

/**
 * spinlock_init() - Initialize soc spinlock module
 *
 * Enable spinlock module, return AIC_SUCCESS.
 *
 */
s32 spinlock_init(void)
{
    if (!__is_spinlock_enabled()) {
        AIC_WRITE32(SPINLOCK_EN_REG, MOD_EN_MASK);

        AIC_WRITE32(MST_REG(0), MSTID_CSTAR | MST_EN);
        AIC_WRITE32(MST_REG(1), MSTID_CR5 | MST_EN);
        AIC_WRITE32(MST_REG(2), 0);
        AIC_WRITE32(MST_REG(3), 0);
        AIC_WRITE32(MST_REG(4), 0);
        AIC_WRITE32(MST_REG(5), 0);
        AIC_WRITE32(MST_REG(6), 0);
        AIC_WRITE32(MST_REG(7), 0);
    }
    return AIC_SUCCESS;
}

/**
 * spinlock_lock - Get a lock
 *
 * @lockid: The lock to be gotten.
 * @to_ms: Time out in milliseconds, can be AOS_NO_WAIT/AOS_WAIT_FOREVER and ms.
 *
 * Get a lock , relax cpu a few time if other master has the lock, and continue
 * to try. The cpu relax is implementation by delay and sleep after system initialize
 * finished.
 * WARNNING: DO NOT USE in ISR!
 *
 * Return: AIC_SUCCESS when get a lock; Error number when error occured.
 *
 */
s32 spinlock_lock(LOCK_ID_T lockid, u32 to_ms)
{
    u32 master_id, ret_slp, elapsed_ms = 0;
    s32 ret_val = AIC_SUCCESS;

    for (;;) {
        if (!__try_lock(lockid))
            break;

        master_id = __get_spinlock_masterid(lockid);
        if (to_ms == AOS_NO_WAIT) {
            ret_val = AIC_EBUSY;
            break;
        }
        /* if to_ms == AOS_WAIT_FOREVER, then it will never less than elapsed_ms */
        if (to_ms < elapsed_ms) {
            ret_val = AIC_ETIME;
            break;
        }

        ret_slp = aos_sleep(RELAX_TIME);
        if (ret_slp == AOS_CALLER_ERROR) {
            aic_mdelay(RELAX_TIME);
        }
        else if (ret_slp != AOS_SUCCESS) {
            ret_val = (s32)ret_slp;
            break;
        }
        /* delay or sleep success, increase elapsed time */
        elapsed_ms += RELAX_TIME;
    }

    return ret_val;
}

/**
 * spinlock_lock_poll - Get a lock
 *
 * @lockid: The lock to be gotten.
 * @to_ms: Time out in milliseconds, unit is ms.
 *
 * Return: AIC_SUCCESS when get a lock; Error number when error occured.
 *
 */
s32 __SPINLOCK_CODE spinlock_lock_poll(LOCK_ID_T lockid, u32 to_ms)
{
    u64 cnt_old, cnt_cur;

    cnt_old = aic_get_tick();

    do {
        if (!__try_lock(lockid))
            return AIC_SUCCESS;

        cnt_cur = aic_get_tick();
    } while ((cnt_cur - cnt_old) < to_ms);

    return AIC_ETIMEDOUT;
}

/**
 * spinlock_unlock - Release a lock
 *
 * @lockid: The lock to be released.
 *
 * Release a lock, return none.
 *
 */
void __SPINLOCK_CODE spinlock_unlock(LOCK_ID_T lockid)
{
    AIC_WRITE32(LOCK_REG(lockid), UNLOCK_KEY);
}

/**
 * spinlock_show_sts - Show spinlock status
 *
 * Show whether spinlock module is ON, and show every lock status if it is ON.
 *
 */
void spinlock_show_sts(void)
{
    if (!__is_spinlock_enabled()) {
        aicx_info(0x0f680b, "", "spinlock is disabled!\n");
    } else {
        u32 masterid;
        for (s32 i = 0; i < LOCK_NUM; i++) {
            if (__get_spinlock_sts((LOCK_ID_T)i)) {
               masterid = __get_spinlock_masterid((LOCK_ID_T)i);
               aicx_info(0x08b712, "dd", "spinlock %d is locked by master %d!\n",
                         i, masterid);
            } else {
               aicx_info(0x13e1b4, "d", "spinlock %d is free!\n");
            }
        }
    }
}
