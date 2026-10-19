/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    Common definitions and interface of soc spinlock.
*/

#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

typedef enum {
    SLOCK_0,
    SLOCK_1,
    SLOCK_2,
    SLOCK_3,
    SLOCK_4,
    SLOCK_5,
} LOCK_ID_T;

/**
 * spinlock_init() - Initialize soc spinlock module
 *
 * Enable spinlock module, return AIC_SUCCESS.
 *
 */
s32 spinlock_init(void);

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
s32 spinlock_lock(LOCK_ID_T lockid, u32 to_ms);

/**
 * spinlock_lock_poll - Get a lock
 *
 * @lockid: The lock to be gotten.
 * @to_ms: Time out in milliseconds, unit is ms.
 *
 * Return: AIC_SUCCESS when get a lock; Error number when error occured.
 *
 */
s32 spinlock_lock_poll(LOCK_ID_T lockid, u32 to_ms);

/**
 * spinlock_unlock - Release a lock
 *
 * @lockid: The lock to be released.
 *
 * Release a lock, return none.
 *
 */
void spinlock_unlock(LOCK_ID_T lockid);

/**
 * spinlock_show_sts - Show spinlock status
 *
 * Show whether spinlock module is ON, and show every lock status if it is ON.
 *
 */
void spinlock_show_sts(void);

u32 spinlock_sts(LOCK_ID_T lockid);

u32 spinlock_masterid(LOCK_ID_T lockid);

void spinlock_set_master(u32 index, u32 mst_id, bool en);

#endif /* __SPINLOCK_H__ */
