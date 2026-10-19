/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the alog module-related data and api
 *    contains all the log apis: alog_fatal, alog_error, alog_warn, alog_info
 *    and alog_debug.
 *    Other module need use log api.
 */
#ifndef __ALOG_API_H__
#define __ALOG_API_H__

#include <aic_type.h>
#include <aic_module.h>

/*
 * Win32 open some log feature in this header file directly.
 * On board they will be open in menuconfig.
 */
#ifdef WIN32
#define CONFIG_ALOG_MSG_SUPPORT
#define CONFIG_ALOG_FRAME_SUPPORT
//#define CONFIG_ALOG_TRACE_IDX
#endif

/**
 * all log level define.
 */
#define ALOG_LV_NONE    0
#define ALOG_LV_ERROR   1
#define ALOG_LV_WARN    2
#define ALOG_LV_INFO    3
#define ALOG_LV_DEBUG   4

/**
 * all log data type define.
 */
enum {
    ALOG_DATA_CAP = 0,
    ALOG_DATA_AUD,
    ALOG_DATA_IQ,
    ALOG_DATA_NORMAL,
    ALOG_DATA_DUMP,
    ALOG_DATA_CNT
};

#define INVALID_DATA_LINK   0xFF

/**
* if the log level is < default static log fiter leve,
* the api alog_xxx will not compile.
* The default static log fiter level is ALOG_LV_INFO.
*/
#ifndef CONFIG_ALOG_STATIC_LEVEL
#define CONFIG_ALOG_STATIC_LEVEL    ALOG_LV_INFO
#endif

/* alog_debug define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_DEBUG)
#define alog_debug(...)
#else
#define alog_debug(fmt, ...) do { \
        alog_output(LOG_MODULE, ALOG_LV_DEBUG, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_info define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define alog_info(...)
#else
#define alog_info(fmt, ...) do { \
        alog_output(LOG_MODULE, ALOG_LV_INFO, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_warn define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_WARN)
#define alog_warn(...)
#else
#define alog_warn(fmt, ...) do { \
        alog_output(LOG_MODULE, ALOG_LV_WARN, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_error define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_ERROR)
#define alog_error(...)
#else
#define alog_error(fmt, ...) do { \
        alog_output(LOG_MODULE, ALOG_LV_ERROR, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

/* alog_print define */
#define alog_print(lv, fmt, ...) do { \
        alog_output(LOG_MODULE, lv, fmt, ##__VA_ARGS__);\
    } while (0)

/* alog_bitstr define */
#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_DEBUG)
#define alog_bitdbg(...)
#else
#define alog_bitdbg(fmt, ...) do { \
        alog_bitstr_output(LOG_MODULE, ALOG_LV_DEBUG, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define alog_bitstr(...)
#else
#define alog_bitstr(fmt, ...) do { \
        alog_bitstr_output(LOG_MODULE, ALOG_LV_INFO, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_WARN)
#define alog_bitwarn(...)
#else
#define alog_bitwarn(fmt, ...) do { \
        alog_bitstr_output(LOG_MODULE, ALOG_LV_WARN, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

#if (CONFIG_ALOG_STATIC_LEVEL < ALOG_LV_INFO)
#define alog_biterror(...)
#else
#define alog_biterror(fmt, ...) do { \
        alog_bitstr_output(LOG_MODULE, ALOG_LV_ERROR, fmt, ##__VA_ARGS__);\
    } while (0)
#endif

#ifdef CONFIG_ALOG_TRACE_IDX
#include <alog_idx_api.h>

#define aicx_debug(id, xxx, fmt, ...)       aidx_debug(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define aicx_info(id, xxx, fmt, ...)        aidx_info(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define aicx_warn(id, xxx, fmt, ...)        aidx_warn(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define aicx_error(id, xxx, fmt, ...)       aidx_error(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define aicx_bitstr(id, xxx, fmt, ...)      aidx_bitstr(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define aicx_bitwarn(id, xxx, fmt, ...)     aidx_bitwarn(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#define alcx_biterror(id, xxx, fmt, ...)    aidx_biterror(AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#else
#define aicx_debug(id, xxx, fmt, ...)       alog_debug(fmt, ##__VA_ARGS__)
#define aicx_info(id, xxx, fmt, ...)        alog_info(fmt, ##__VA_ARGS__)
#define aicx_warn(id, xxx, fmt, ...)        alog_warn(fmt, ##__VA_ARGS__)
#define aicx_error(id, xxx, fmt, ...)       alog_error(fmt, ##__VA_ARGS__)
#define aicx_bitstr(id, xxx, fmt, ...)      alog_bitstr(fmt, ##__VA_ARGS__)
#define aicx_bitwarn(id, xxx, fmt, ...)     alog_bitwarn(fmt, ##__VA_ARGS__)
#define aicx_biterror(id, xxx, fmt, ...)    alog_biterror(fmt, ##__VA_ARGS__)
#endif /* CONFIG_ALOG_TRACE_IDX */

/**
 * alog_output - output the trace log.
 *
 * @mod: the id of the module.
 * @level: the log level of this trace
 * @fmt:  format string.
 */
void alog_output(u8 mod, u8 level, const char *fmt, ...);

/**
 * alog_bitstr_output - output ta bitmap string trace..
 *
 * @mod: the id of the module.
 * @level: the log level of this trace
 * @fmt:  format string.
 */
void alog_bitstr_output(u8 mod, u8 lv, const char *fmt, ...);

#ifdef CONFIG_ALOG_MSG_SUPPORT
/**
 * alog_output - output the message log.
 *
 * @message: the message in a specific format.
 */
void alog_message(void *message);
#else
#define alog_message(p)
#endif

/**
 * alog_data_ex - output the data log.
 *
 * @data: the data pointer.
 * @size: the size of the data.
 * @type: the type of the data.
 * @link: the virtual link id of the data.(ALOG_DATA_DUMP, link is the sys)
 * @timeout: when no blk to copy data, max timeout value(ms).
 */
void alog_data_ex(u8 *data, size_t size, u8 type, u8 link, int timeout);
#define alog_data(d, size, type)   alog_data_ex((d), (size), (ALOG_DATA_NORMAL), INVALID_DATA_LINK, AOS_NO_WAIT)
#define audio_data(d, size, link)  alog_data_ex((d), (size), (ALOG_DATA_AUD), (link), AOS_NO_WAIT)
#define alog_dump(sys, d, size)    alog_data_ex((d), (size), (ALOG_DATA_DUMP), (sys), AOS_WAIT_FOREVER)

/**
 * alog_data_finish - finish a data dump.
 *
 * @type: the type of the data.
 * @link: the virtual link id of the data.(ALOG_DATA_DUMP, link is the sys)
 */
void alog_data_finish(u8 type, u8 link);
#define audio_finish(link)  alog_data_finish((ALOG_DATA_AUD), (link))
#define normal_finish()     alog_data_finish((ALOG_DATA_NORMAL), (INVALID_DATA_LINK))

/**
 * alog_dump_start - start a data dump.
 * This api is optinal, if not call pc will create a file with defalt rule.
 * if used this api will create a file with give name and addr like below:
 * name_0x80000000_0x1000.xxxx
 *
 * @sys:  which sys data will be dump.
 * @name: the data dump name in pc tool.
 * @addr: the size of the data.
 * @size: the type of the data.
 */
void alog_dump_start(u8 sys, char *name, u32 addr, u32 size);

/**
 * alog_data_finish - will finish a link dump.
 *
 * @sys:  which sys data will be dump.
 */
#define alog_dump_finish(sys)   alog_data_finish((ALOG_DATA_DUMP), (sys))

/**
 * alog_flush_all_log_in_assert - flush all buffer log after assert.
 *
 * @can only be called after assert.
 */
void alog_flush_all_log_in_assert(void);

/**
 * alog_output_data_in_assert - output data after assert.
 *
 * @can only be called after assert.
 */
void alog_output_data_in_assert(void *addr, size_t size);

#ifdef CONFIG_SUPPORT_ADSP
/**
 * alog_adsp_trace - output the dsp trace log.
 *
 * @buf: the adsp tracebuf.
 * @size: the size of the buf.
 */
void alog_dsp_trace(u8 *buf, size_t size);
#endif

/**
 * alog_set_log_level_no_save
 * dynamic set the log level of the specified mod and will not save in flash.
 *
 * @mod: the data pointer.
 * @level: the size of the data.
 */
void alog_set_log_level_no_save(u8 mod, u8 level);

/** 
 * alog_set_output_dev - set new log output device.
 *
 * @new_dev: the new io device type.
 */
void alog_set_output_dev(u8 new_dev);

#endif /* __ALOG_API_H__ */

