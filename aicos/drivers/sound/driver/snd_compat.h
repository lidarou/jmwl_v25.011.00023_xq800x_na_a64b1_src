/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines some MACROs, by which it can be compatible with
 *    other system without changing code.
 */

#ifndef __SND_COMPAT_H__
#define __SND_COMPAT_H__

/* #define SIMU_SND_ENV */

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_assert.h>
#include <aic_list.h>
#include <aic_errno.h>
#include <slp_inc.h>

#define AIC_ENOTSUPP    200
#define AIC_ERESTARTSYS 201
#define AIC_EBADFD      202

#ifndef INT_MAX
#define INT_MAX    0x7FFFFFFF
#endif
#ifndef INT_MIN
#define INT_MIN    (-INT_MAX - 1)
#endif

#define SND_WAIT_FOREVER (-1)

#ifdef WIN32
#define __func__        __FUNCTION__
#define __attribute__(x)

#endif

#define SND_ENTRY       alog_info("%s entry, line: %d\n", __func__, __LINE__)
#define SND_EXIT        alog_info("%s exit, line: %d\n", __func__, __LINE__)

#define SND_ERROR       alog_error
#define SND_INFO        alog_info
#define SND_DBG         alog_debug

#define SNX_ERROR       aicx_error
#define SNX_INFO        aicx_info
#define SNX_DBG         aicx_debug

#define snd_BUG_ON(x)   AIC_ASSERT(!x)
#define BUG_ON(x)       AIC_ASSERT(!x)
#define BUG             AIC_ASSERT

#define copy_to_user    memcpy
#define copy_from_user  memcpy

#define INIT_LIST_HEAD  list_head_init

#define EVENT_PTR       aos_event_ptr

#define MUTEX_PTR       aos_mutex_ptr
#define mutex_init(x)   x = aos_create_mutex("snd")
#define mutex_lock(x)   aos_get_mutex(x, SND_WAIT_FOREVER)
#define mutex_unlock    aos_put_mutex

#ifdef SIMU_SND_ENV
void *aos_zmalloc(u32 size, const char *func, int line);
void *aos_malloc(u32 size, const char *func, int line);
void aos_free(void *addr, const char *func, int line);

#define snd_zalloc(n) aos_zmalloc(n, __FILE__, __LINE__)
#define snd_malloc(n) aos_malloc(n, __FILE__, __LINE__)
#define snd_free(p) aos_free(p, __FILE__, __LINE__)
#else
void * aos_zmalloc(u32 size);
#define snd_zalloc(n) aos_zmalloc(n)
#define snd_malloc(n) aos_malloc(n)
#define snd_free(p) aos_free(p)
#endif

#define wake_lock       aic_get_wakelock
#define wake_unlock     aic_put_wakelock

void *aos_memdup(const void *src, size_t len);
int fls(int x);
void snd_dvfs_set(u32 level);

#endif /* __SND_COMPAT_H__ */

