/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 * alog trace idx api define header file.
 */
#ifndef __ALOG_IDX_API_H__
#define __ALOG_IDX_API_H__

#ifdef CONFIG_LOG_FILE_LINE
#define _FMT_PF_PAM_    u32 file, u32 line, u32 fmt, const char *pf

/* aidx_debug define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_DEBUG)
#define aidx_debug(...)
#else
#define aidx_debug(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_DEBUG, file, __LINE__, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* aidx_info define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define aidx_info(...)
#else
#define aidx_info(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_INFO, file, __LINE__, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* aidx_warn define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_WARN)
#define aidx_warn(...)
#else
#define aidx_warn(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_WARN, file, __LINE__, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_error define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define aidx_error(...)
#else
#define aidx_error(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_ERROR, file, __LINE__, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

#else /* CONFIG_LOG_FILE_LINE */
#define _FMT_PF_PAM_    u32 fmt, const char *pf

/* aidx_debug define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_DEBUG)
#define aidx_debug(...)
#else
#define aidx_debug(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_DEBUG, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* aidx_info define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define aidx_info(...)
#else
#define aidx_info(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_INFO, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* aidx_warn define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_WARN)
#define aidx_warn(...)
#else
#define aidx_warn(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_WARN, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_error define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define aidx_error(...)
#else
#define aidx_error(file, fmt, pf, ...) do { \
        alog_idx_output(LOG_MODULE, ALOG_LV_ERROR, fmt, pf, ##__VA_ARGS__);\
    } while (0)
#endif

#endif /* CONFIG_LOG_FILE_LINE */ 

/* aidx_bitstr define */
#define aidx_bitdbg     aidx_debug
#define aidx_bitstr     aidx_info
#define aidx_bitwarn    aidx_warn
#define aidx_biterror   aidx_error

/**
 * alog_idx_output - output the trace id log.
 *
 * @mod: the id of the module.
 * @level: the log level of this trace.
 * @file: file string id.
 * @line: line number.
 * @fmt: format string id.
 * @pf: parameter format string. the pf format string like "dxsp",
 *      means that the pf has 4 parameters, the style like %d, %x, %s, %p.
 */
void alog_idx_output(u8 mod, u32 level, _FMT_PF_PAM_, ...);

#endif /* __ALOG_IDX_API_H__ */

