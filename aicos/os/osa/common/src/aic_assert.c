/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file realize the system assert function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0deea1
#define LOG_MODULE    AMOD_AOS

#include <aic_assert.h>
#include <aic_osa.h>
#include <coresight.h>
#include <system_assert.h>
#include <assert_internal.h>
#include <stdarg.h>
#include <stdio.h>
#include <sprintf_nano.h>

#define MAX_ASSERT_INFO_LEN 256
char s_assert_string[MAX_ASSERT_INFO_LEN];
extern struct assert_info s_assert_info;

static void aic_sys_assert_prepare(void)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    /* if assert cnt greater than MAX_COUNT, enter while(1) */
    if (s_assert_info.assert_cnt >= ASSERT_MAX_COUNT)
        while (1);

#ifdef CONFIG_SUBSYS_AP
#ifdef CONFIG_DRIVER_CORESIGHT
    aic_cs_trace_stop();
#endif
#endif
}

static void aic_sys_assert_enter(void)
{
    char *thread_name = aos_get_current_thread_name();
    int len;

    if (!thread_name)
        thread_name = "NULL";

    len = strlen(s_assert_string);
    snprintf_nano(&s_assert_string[len], (MAX_ASSERT_INFO_LEN - 1 - len),
                  "\ncurrent thread: %s!\n", thread_name);

    system_assert(s_assert_string);
}

void aic_sys_assert(char *fmt, ...)
{
    size_t len;
    va_list args;

    aic_sys_assert_prepare();

    len = strlen(s_assert_string);
    va_start(args, fmt);
    len = vsnprintf_nano(&s_assert_string[len], MAX_ASSERT_INFO_LEN - 1, fmt, args);
    va_end(args);

    aic_sys_assert_enter();
}

#ifdef CONFIG_ASSERT_FEWER_PARAM
static  void aic_sys_assert_add_prefix(char *file, u32 line)
{
    int len;

    /* if happen mutiple times assert, not cover the before assert info. */
    len = strlen(s_assert_string);
    snprintf_nano(&s_assert_string[len], (MAX_ASSERT_INFO_LEN - 1 - len),
                  ASSERT_PREFIX, file, line);
}

static  void aic_sys_assert_add_prefix_idx(u32 fileid, u32 line)
{
    int len;

    /* if happen mutiple times assert, not cover the before assert info. */
    len = strlen(s_assert_string);
    snprintf_nano(&s_assert_string[len], (MAX_ASSERT_INFO_LEN - 1 - len),
                  ASSERT_PREFIX_IDX, fileid, line);
}

void aic_sys_assert_ex(char *file, u32 line)
{
    aic_sys_assert_prepare();
    aic_sys_assert_add_prefix(file, line);
    aic_sys_assert_enter();
}

void aic_sys_assert_idx(u32 fileid, u32 line)
{
    aic_sys_assert_prepare();
    aic_sys_assert_add_prefix_idx(fileid, line);
    aic_sys_assert_enter();
}

void aic_sys_assert_info(char *file, u32 line, char *fmt, ...)
{
    size_t len;
    va_list args;

    aic_sys_assert_prepare();
    aic_sys_assert_add_prefix(file, line);

    len = strlen(s_assert_string);
    va_start(args, fmt);
    len = vsnprintf_nano(&s_assert_string[len], MAX_ASSERT_INFO_LEN - 1, fmt, args);
    va_end(args);

    aic_sys_assert_enter();
}

void aic_sys_assert_info_idx(u32 fileid, u32 line, u32 fmtid, const char *pf, ...)
{
    aic_sys_assert_prepare();
    aic_sys_assert_add_prefix_idx(fileid, line);
    aidx_error(fileid, fmtid, pf);

    aic_sys_assert_enter();
}

void aic_sys_assert_info_idx_s(u32 fileid, u32 line, char *fmt, ...)
{
    size_t len;
    va_list args;

    aic_sys_assert_prepare();
    aic_sys_assert_add_prefix_idx(fileid, line);

    len = strlen(s_assert_string);
    va_start(args, fmt);
    len = vsnprintf_nano(&s_assert_string[len], MAX_ASSERT_INFO_LEN - 1, fmt, args);
    va_end(args);

    aic_sys_assert_enter();
}
#endif

