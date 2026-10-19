/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the system assert Interface.
 */

#ifndef __AIC_ASSERT_DEF_H__
#define __AIC_ASSERT_DEF_H__

/* the str can not be changed, it will be parsed by remote sys. */
#ifdef CONFIG_SUBSYS_AP
#define ASSERT_KEY      "AP ASSERT:"
#else
#define ASSERT_KEY      "CP ASSERT:"
#endif

#ifdef CONFIG_ASSERT_FEWER_PARAM
#define ASSERT_PREFIX_IDX       ASSERT_KEY "at File(0x%06x), Line(%d)!"
#define ASSERT_POS_IDX          AIC_THIS_FILE, __LINE__

#define ASSERT_PREFIX           ASSERT_KEY "at File(%s), Line(%d)!"
#define ASSERT_POS              __FILE__, __LINE__

#ifdef CONFIG_ALOG_TRACE_IDX
#define AIC_ASSERT(n) do { \
    if(!(n)) \
       aic_sys_assert_idx(ASSERT_POS_IDX); \
    } while(0)
#else
#define AIC_ASSERT(n) do { \
    if(!(n)) \
       aic_sys_assert_ex(ASSERT_POS); \
    } while(0)
#endif

#define AIC_ASSERT_INFO(n, fmt, ...) do { \
    if(!(n)) \
        aic_sys_assert_info(ASSERT_POS, fmt, ##__VA_ARGS__); \
    } while(0)

#define AIC_ASSERT_INFX(n,fileid, fmtid, pf, ...) do { \
        if(!(n)) { \
            aic_sys_assert_info_idx(fileid, __LINE__, fmtid, pf, ##__VA_ARGS__); \
        } \
    } while(0)

/* Local macro define */
#ifdef AIC_THIS_FILE
#define AIC_ASSERT_S(cond) do { \
    if(!(cond)) \
       aic_sys_assert_idx(ASSERT_POS_IDX); \
    } while(0)

#define AIC_ASSERT_INFO_S(cond, fmt, ...) do { \
    if(!(cond)) \
        aic_sys_assert_info_idx_s(ASSERT_POS_IDX, fmt, ##__VA_ARGS__); \
    } while(0)
#else
#define AIC_ASSERT_S(cond) do { \
    if(!(cond)) \
       aic_sys_assert_ex(ASSERT_POS); \
    } while(0)

#define AIC_ASSERT_INFO_S(cond, fmt, ...) do { \
    if(!(cond)) \
        aic_sys_assert_info(ASSERT_POS, fmt, ##__VA_ARGS__); \
    } while(0)
#endif
#else /* CONFIG_ASSERT_FEWER_PARAM */
#ifdef CONFIG_FILENAME_TO_ID
#define ASSERT_PREFIX   ASSERT_KEY "at File(0x%06x), Line(%d)!"
#define ASSERT_POS      AIC_THIS_FILE, __LINE__
#else
#define ASSERT_PREFIX   ASSERT_KEY "at File(%s), Line(%d)!"
#define ASSERT_POS    __FILE__, __LINE__
#endif

#define AIC_ASSERT(n) do { \
    if(!(n)) \
       aic_sys_assert(ASSERT_PREFIX, ASSERT_POS); \
    } while(0)

#ifdef CONFIG_ALOG_TRACE_IDX
#ifdef CONFIG_LOG_FILE_LINE
#define AIC_ASSERT_INFX(n, fileid, fmt, pf, ...) do { \
        if(!(n)) { \
            alog_idx_output(AMOD_SYS, ALOG_LV_ERROR, fileid, __LINE__, fmt, pf, ##__VA_ARGS__);\
            aic_sys_assert(ASSERT_PREFIX, ASSERT_POS); \
        } \
    } while(0)
#else
#define AIC_ASSERT_INFX(n, fileid, fmt, pf, ...) do { \
            if(!(n)) { \
                alog_idx_output(AMOD_SYS, ALOG_LV_ERROR, fmt, pf, ##__VA_ARGS__);\
                aic_sys_assert(ASSERT_PREFIX, ASSERT_POS); \
            } \
        } while(0)
#endif /* CONFIG_LOG_FILE_LINE */
#endif /* CONFIG_ALOG_TRACE_IDX */

#define AIC_ASSERT_INFO(n, fmt, ...) do { \
    if(!(n)) \
        aic_sys_assert(ASSERT_PREFIX fmt, ASSERT_POS, ##__VA_ARGS__); \
    } while(0)

/* Local macro define */
#ifdef AIC_THIS_FILE
#define AIC_PREFIX          ASSERT_KEY "at File(0x%06x), Line(%d)!"
#define AIC_ASSERT_POS      AIC_THIS_FILE, __LINE__
#else
#ifdef CONFIG_SUBSYS_AP
#define AIC_PREFIX          ASSERT_KEY "at File(%s), Line(%d)!"
#define AIC_ASSERT_POS      __FILE__, __LINE__
#else
#define AIC_PREFIX          ASSERT_KEY "at File(0x%06x), Line(%d)!"
#define AIC_ASSERT_POS      0xFFFFFFFF, __LINE__
#endif /* CONFIG_SUBSYS_AP */
#endif /* AIC_THIS_FILE */

#define AIC_ASSERT_S(cond) do { \
    if(!(cond)) \
       aic_sys_assert(AIC_PREFIX, AIC_ASSERT_POS); \
    } while(0)

#define AIC_ASSERT_INFO_S(cond, fmt, ...) do { \
    if(!(cond)) \
        aic_sys_assert(AIC_PREFIX fmt, AIC_ASSERT_POS, ##__VA_ARGS__); \
    } while(0)
#endif /* CONFIG_ASSERT_FEWER_PARAM */

#ifdef CONFIG_ALOG_TRACE_IDX
#define AICX_ASSERT_IFX(n, id, xxx, fmt, ...) \
    AIC_ASSERT_INFX(n, AIC_THIS_FILE, id, xxx, ##__VA_ARGS__)
#else
#define AICX_ASSERT_IFX(n, id, xxx, fmt, ...)\
    AIC_ASSERT_INFO(n, fmt, ##__VA_ARGS__)
#endif /* CONFIG_ALOG_TRACE_IDX */

/*
 * CONFIG_DUMMY_CHECK_DEBUG - Can enable it for debug,
 * but will increase the code size.
 */
#define CONFIG_DUMMY_CHECK_DEBUG    0

#if CONFIG_DUMMY_CHECK_DEBUG
#define AIC_NULL_DUMMY_CHECK(ptr)   AIC_ASSERT((ptr))
#define AIC_DUMMY_ASSERT(cond)      AIC_ASSERT((cond))
#else
#define AIC_NULL_DUMMY_CHECK(ptr)
#define AIC_DUMMY_ASSERT(cond)
#endif

#endif/* __AIC_ASSERT_DEF_H__ */

