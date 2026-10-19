/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The alog trace do file.
 */

#if defined (CONFIG_SUBSYS_AP) || defined(WIN32)
#define alog_vsnprintf  vsnprintf
#else
#include <sprintf_nano.h>
#define alog_vsnprintf  vsnprintf_nano
#endif

void alog_output(u8 mod, u8 level, const char *fmt, ...)
{
    bool need_flsuh_1 = FALSE, need_flsuh_2 = FALSE;
    AOS_INTERRUPT_DECLARE
    bool try_again = TRUE;
    ALBF_BLK_T blk;
    int len = 0;
    size_t size, left;
    va_list args;

    AOS_INTERRUPT_DISABLE

    if (alog_filter_or_get_blk_failed(&blk, mod, level,
                                      ALOG_BUF_ONCE_MIN, &need_flsuh_1)) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    do {
        /* build the trace head. */
        len = ADD_TICK_AND_MOD_NAME;
        /* tick and module name must < blk.size. */
        AIC_ASSERT(len > 0 && (size_t)len < blk.size);
        size = (size_t)len;
        left = blk.size - size;

        /* buid the trace body. */
        va_start(args, fmt);
        len = alog_vsnprintf((char *)blk.buf + size, left, fmt, args);
        va_end(args);

#ifndef WIN32
        /* On Win32, when left < actual len, will return -1 */
        if (len < 0) {
            /* some error happen, skip this log. */
            AOS_INTERRUPT_RESTORE
            return;
        }
#endif

        /* buffer is enough, no need retry. */
        if ((size_t)len < left) {
            size += (size_t)len;
            break;
        }

        /*
         * len >= left, means only sucess write left - 1,
         * so get a new blk with size + len + 1, than sprintf it again.
         */
        if (s_log_buf && try_again) {
            blk.size = size + len + 1;
            need_flsuh_2 = albf_mgr_get_blk_again(s_log_buf, &blk);

            /* retry format trace only once. */
            try_again = FALSE;
            continue;
        }

        size += left;
        break;
    } while (TRUE);

    UPDATE_AND_PUT_ABLK(blk, size);

    AOS_INTERRUPT_RESTORE

    if (need_flsuh_1 || need_flsuh_2)
        albf_mgr_notify_flush_task(s_log_buf);

#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    alog_trace_put_blk_log(&blk);
#endif /* ALOG_TRACE_UART_DIRECT */
}

#ifdef ALOG_FAST_API_FEATURE
void alog_output_fast(u8 mod, const char *fmt, ...)
{
    AOS_INTERRUPT_DECLARE
    ALBF_BLK_T blk;
    bool need_flsuh;
    int len;
    va_list args;

    AOS_INTERRUPT_DISABLE
    blk.size = ALOG_BUF_ONCE_MIN;
    need_flsuh = albf_mgr_get_blk(s_log_buf, &blk);

    /* buid the trace body. */
    va_start(args, fmt);
    len = alog_vsnprintf((char *)blk.buf, blk.size, fmt, args);
    va_end(args);

    if (len > 0)
        UPDATE_AND_PUT_ABLK(blk, (size_t)len);

    AOS_INTERRUPT_RESTORE

    if (need_flsuh)
        aos_put_semaphore(s_log_buf->sema);
}
#endif

#ifdef CONFIG_ALOG_BITMAP_STRING
#define MAX_BIT_STR_LEN 128

static void alog_bitstr_preprocess_fmt(const char *fmt, char *buf)
{
    const char *src;
    char c, pre = 0;
    u8 i;

    /* process the bitstr fmt. */
    for (src = fmt, i = 0; *src && i < MAX_BIT_STR_LEN; src++, i++) {
        c = *src;
        if (pre == '%') {
            if (c == 'm' || c == 'M')
                c = 'x';
            else if (c == 'S')
                c = 's';
        }
        pre = c;
        buf[i] = c;
    }

    buf[i] = 0;
}

void alog_bitstr_output(u8 mod, u8 lv, const char *fmt, ...)
{
    char fmt_buf[MAX_BIT_STR_LEN + 1];
    AOS_INTERRUPT_DECLARE
    ALBF_BLK_T blk;
    bool need_flsuh;
    va_list args;
    int len;

    if (lv > alog_get_log_level(mod))
        return;

    AOS_INTERRUPT_DISABLE
    blk.size = 512;
    need_flsuh = albf_mgr_get_blk(s_log_buf, &blk);

    /* buid the trace body. */
    alog_bitstr_preprocess_fmt(fmt, fmt_buf);
    va_start(args, fmt);
    len = alog_vsnprintf((char *)blk.buf, blk.size, fmt_buf, args);
    va_end(args);

    if (len > 0)
        UPDATE_AND_PUT_ABLK(blk, (size_t)len);

    AOS_INTERRUPT_RESTORE

    if (need_flsuh)
        aos_put_semaphore(s_log_buf->sema);
}
#endif

