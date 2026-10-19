/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The alog idx api do file.
 */

/*
 * trace id format:
 * 0xfe + format header + format parameters + 0xef.
 */

#define ALOG_INTERRUPT_DECLARE AOS_INTERRUPT_DECLARE
#define ALOG_INTERRUPT_DISABLE AOS_INTERRUPT_DISABLE
#define ALOG_INTERRUPT_RESTORE AOS_INTERRUPT_RESTORE


#define AIDX_HEAD       0xfe
#define AIDX_TAIL       0xef

/* head and tail len. */
#define ADX_HT_LEN      2
#define ADX_FIX_LEN     (ADX_HT_LEN + sizeof(AIDX_FMT_T))
#define AIDX_NOFMT_ID   0xFFFFFFFF

/*
 * AIDX_FMT_TYPE: trace need format by tools.
 * AIDX_NOFMT_TYPE: trace no need format by tools.
 * the paramters is context of the trace.
 */
#define AIDX_NOFMT_TYPE 0x00
#define AIDX_FMT_BASE   0x01

enum {
    AIDX_FMT_NORMAL = AIDX_FMT_BASE,
    AIDX_FMT_BITSTR
};

/*
 * AIDX_MAX_P_LEN: the max parameters len, if the len of paramets
 *                 exceeds it will be truncated.
 * AIDX_MIN_BUF_SIZE: The minimun size of the trace request.
 */
#define AIDX_MAX_P_LEN      236
#define AIDX_MIN_BUF_SIZE   (ADX_FIX_LEN + AIDX_MAX_P_LEN)

#ifdef CONFIG_LOG_FILE_LINE
#define AIDX_NOFMT_SET(idx, module, length) do { \
        idx->file_id = AIDX_NOFMT_ID; \
        idx->fmt_id = AIDX_NOFMT_ID; \
        idx->type = AIDX_NOFMT_TYPE; \
        idx->len = (length); \
        idx->mod = (module); \
        idx->line = 0; \
        idx->time = alog_get_aon_timestamp(); \
    } while(0)

#define AIDX_FMT_SET(idx, module, file, line_num, fmtid, fmttype, length) do { \
        idx->file_id = (file); \
        idx->fmt_id = (fmtid); \
        idx->type = (fmttype); \
        idx->len = (length); \
        idx->mod = (module); \
        idx->line = (u16)(line_num); \
        idx->time = alog_get_aon_timestamp(); \
    } while(0)
#else
#define AIDX_NOFMT_SET(idx, module, length) do { \
        idx->fmt_id = AIDX_NOFMT_ID; \
        idx->type = AIDX_NOFMT_TYPE; \
        idx->len = (length); \
        idx->mod = (module); \
        idx->time = alog_get_aon_timestamp(); \
    } while(0)

#define AIDX_FMT_SET(idx, module, file, line_num, fmtid, fmttype, length) do { \
        idx->fmt_id = (fmtid); \
        idx->type = (fmttype); \
        idx->len = (length); \
        idx->mod = (module); \
        idx->time = alog_get_aon_timestamp(); \
    } while(0)
#endif
/*
 * INT_CASES define:
 * 'm' and 'M' is expand by aic, is menas bitmaps of a bitmap string.
 * can be used with '%S'.
 * 'm' means all bitmaps use a same bitmap string. Such as:
 * alog_bitstr("str1 is bits(0x%m), str2 is bits(0x%m), str3 is bits(0x%m)" \
 *             ", [bitsstr(0x%S)]",
 *             0x11, 0x22, 0x33,
               "|str0|str1|str2|str3|str4|str5|str6|str7");
 * after scan will like below:
 * aidx_bitstr(AIC_THIS_FILE, 0xxxxx, "mmmS",
               0x11, 0x22, 0x33,
               0xxxxx);
 * 'M' means evry bitmaps use a bitmap string. Such as
 * alog_bitstr("str1 is BITS(0x%M), str2 is BITS(0x%M), str3 is BITS(0x%M)" \
 *           "[bitsstr(0x%S)], [bitsstr(0x%S)], [bitsstr(0x%S)],
 *           0x11, 0x22, 0x33,
             "|str0|str1|str2|str3|str4|str5|str6|str7",
             "|stra|strb|strc|strd|stre|strf|strg|strh",
             "|str0|str1|str2|str3|str4|str5|str6|str7");
 * after scan will like below:
 * aidx_info(AIC_THIS_FILE, 0xxxxx, "BBBSSS",
             0x11, 0x22, 0x33,
             0xxxxx, 0xxxxx, 0xxxxx);
 * bellow case is not normal case.
 *   case 'b': \
 *   case 'B': \
 */

#ifdef CONFIG_ALOG_BITMAP_STRING
#define INT_CASES \
         'c': \
    case 'C': \
    case 'd': \
    case 'i': \
    case 'o': \
    case 'u': \
    case 'x': \
    case 'X': \
    case 'n': \
    case 'p': \
    case 'm': \
    case 'M'

/*
 * STR_CASES define:
 * 'S' is expand by aic, is menas a bitmap string,
 * like "|bit0 str | bit1  str | bit2 str | bitn str",
 * only output a stringid process like %d.
 * Can be used with '%m' or ''%M'
 */
#define BITSTR_CASES \
         'S'
#else
#define INT_CASES \
         'c': \
    case 'C': \
    case 'd': \
    case 'i': \
    case 'o': \
    case 'u': \
    case 'x': \
    case 'X': \
    case 'n': \
    case 'p'
#endif

/*
 * FP_CASES define:
 */
#define FP_CASES \
         'a': \
    case 'A': \
    case 'e': \
    case 'E': \
    case 'f': \
    case 'F': \
    case 'g': \
    case 'G'

/*
 * STR_CASES define:
 */
#define STR_CASES \
         's'

/*
 * LENMOD_CASES define:
 */
#define LENMOD_CASES \
         'h': \
    case 'l': \
    case 'z': \
    case 'L'

typedef enum {
    len_mod_int = 0,
    len_mod_long,
    len_mod_long_long,
    len_mod_long_dobule
} len_mod_type;

#ifdef CONFIG_SYS_LDR_4BYTE_ALIGN
/*
 * GET_A_COMM_TYPE_PARAM define:
 */
#define GET_A_COMM_TYPE_PARAM(type, ap, p, end) \
do { \
    type v = va_arg((ap), type); \
    volatile u8 *dst = (volatile u8 *)(p); \
    volatile u8 *src = (volatile u8 *)(&v); \
    int i, cnt; \
    \
    cnt = sizeof(type) / sizeof(*dst); \
    if ((p) + sizeof(type) < (end)) { \
        for (i = 0;  i < cnt; i++) {\
            *dst++ = *src++;\
        } \
        (p) += sizeof(type); \
    } \
} while (0)

#define GET_A_TYPE_PARAM                        GET_A_COMM_TYPE_PARAM
#define GET_A_64BIT_TYPE_PARAM                  GET_A_COMM_TYPE_PARAM
#define GET_A_TYGET_A_BIG_TYPE_PARAMPE_PARAM    GET_A_COMM_TYPE_PARAM
#define GET_A_BIG_TYPE_PARAM                    GET_A_COMM_TYPE_PARAM
#else
/*
 * GET_A_TYPE_PARAM define:
 */
#define GET_A_TYPE_PARAM(type, ap, p, end) \
do { \
    type v = va_arg((ap), type); \
    type *v_p = (type *)(p); \
    \
    if ((p) + sizeof(type) < (end)) {\
        *v_p = v;\
        (p) += sizeof(type);\
    } \
} while (0)

/*
 * GET_A_64BIT_TYPE_PARAM define:
 */
#define GET_A_64BIT_TYPE_PARAM(type, ap, p, end) \
do { \
    type v = va_arg((ap), type); \
    volatile u32 *dst = (volatile u32 *)(p); \
    volatile u32 *src = (volatile u32 *)(&v); \
    \
    if ((p) + sizeof(type) < (end)) {\
        *dst++ = *src++;\
        *dst = *src;\
        (p) += sizeof(type);\
    } \
} while (0)

/*
 * GET_A_BIG_TYPE_PARAM define:
 */
#define GET_A_BIG_TYPE_PARAM(type, ap, p, end) \
do { \
    type v = va_arg((ap), type); \
    volatile u32 *dst = (volatile u32 *)(p); \
    volatile u32 *src = (volatile u32 *)(&v); \
    int i, cnt; \
    \
    cnt = sizeof(type) / sizeof(*dst); \
    if ((p) + sizeof(type) < (end)) { \
        for (i = 0;  i < cnt; i++) {\
            *dst++ = *src++;\
        } \
        (p) += sizeof(type); \
    } \
} while (0)
#endif

/*
 * GET_A_STR_PARAM define:
 */
#define GET_A_STR_PARAM(ap, p, end) \
do { \
    if ((p) < (end - 1)) { \
        char *str = va_arg((ap), char *); \
        \
        while ( *str && (p) < (end - 1)) {\
            *p++ = *str++;\
        } \
        *p++ = '\0';\
    } \
} while (0)

/*
 * trace id format header define:
 * @len: the len of this format packect, except the head and the tail.
 * @mod: the id of the module.
 * @type: 0 -- don't format it in pc tool, the parameter is the log data.
 *        1 -- need formate it in pc tool.
 * @line: the line number.
 * @time: the trace print time.
 * @file_id: the id of file name string, auto produce by file id tool.
 * @fmt_id:  the id of format string, auto produce by formate id tool.
 */
#ifdef WIN32
#pragma pack(push, 1)
typedef struct AIDX_FMT {
    u16     len;
    u8      mod;
    u8      type;
#ifdef CONFIG_LOG_FILE_LINE
    u16     line;
#endif
    u32     time;
#ifdef CONFIG_LOG_FILE_LINE
    u32     file_id;
#endif
    u32     fmt_id;
} AIDX_FMT_T;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) AIDX_FMT {
    u16     len;
    u8      mod;
    u8      type;
#ifdef CONFIG_LOG_FILE_LINE
    u16     line;
#endif
    u32     time;
#ifdef CONFIG_LOG_FILE_LINE
    u32     file_id;
#endif
    u32     fmt_id;
} AIDX_FMT_T;
#endif

#if defined (CONFIG_SUBSYS_AP) || defined(WIN32)
#define alog_vsnprintf  vsnprintf
#else
#include <sprintf_nano.h>
#define alog_vsnprintf  vsnprintf_nano
#endif

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
PLAT_TCM_CODE void alog_idx_output(u8 mod, u32 level, _FMT_PF_PAM_, ...)
{
    len_mod_type len_mod = len_mod_int;
    int type = AIDX_FMT_NORMAL;
    AOS_INTERRUPT_DECLARE
    size_t p_len = 0;
    bool need_flsuh;
    AIDX_FMT_T *idx;
    ALBF_BLK_T blk;
    const char *arg_f;
    u8 *p, *end;
    va_list ap;

    AOS_INTERRUPT_DISABLE

    /* if it is filtered out or get blk failed, return. */
    if (alog_filter_or_get_blk_failed(&blk, mod, level,
                                      AIDX_MIN_BUF_SIZE, &need_flsuh)) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    /*
     * init the head, idx pointer and end pointer,
     * the end pointer point to the tail.
     */
    p = (u8 *)blk.buf;
    end = p + blk.size - 1;
    *p++ = AIDX_HEAD;
    idx = (AIDX_FMT_T *)p;
    p += sizeof(*idx);

    /* get the fmt params according pf. */
    va_start(ap, pf);
    for (arg_f = pf; *arg_f; arg_f++) {
        switch(*arg_f) {
#ifdef CONFIG_ALOG_BITMAP_STRING
            /* Just has bitstr case, change the type to AIDX_FMT_BITSTR.*/
            case BITSTR_CASES:
                type = AIDX_FMT_BITSTR;
                /* don't break here, do the INT_CASES. */
#endif

            /* char, short and int arg all push to stack 4byte, so all used int type. */
            case INT_CASES:
                if (len_mod == len_mod_long) {
                    GET_A_TYPE_PARAM(long, ap, p, end);
                } else if (len_mod == len_mod_long_long) {
                    GET_A_64BIT_TYPE_PARAM(long long, ap, p, end);
                } else {
                    GET_A_TYPE_PARAM(int, ap, p, end);
                }

                len_mod = len_mod_int;
                break;

            case FP_CASES:
                if (len_mod == len_mod_long_dobule) {
                    GET_A_BIG_TYPE_PARAM(long double, ap, p, end);
                } else {
                    /* 
                     * notify: float' will be promoted to 'double'
                     * when passed through '...'.
                     */
                    GET_A_64BIT_TYPE_PARAM(double, ap, p, end);
                }
                len_mod = len_mod_int;
                break;

            case STR_CASES:
                GET_A_STR_PARAM(ap, p, end);
                len_mod = len_mod_int;
                break;

            case LENMOD_CASES:
                if (*arg_f == 'l') {
                    if (len_mod == len_mod_int)
                        len_mod = len_mod_long;
                    else if (len_mod == len_mod_long)
                        len_mod = len_mod_long_long;
                } else if (*arg_f == 'L') {
                    /* 'L' is long double. */
                    len_mod = len_mod_long_dobule;
                } else {
                    len_mod = len_mod_int;
                }
            break;

            default:
                //AIC_DUMMY_ASSERT(0);
                AIC_ASSERT(0);
                break;
        }
    }
    va_end(ap);

    /* end the idx. */
    *(p) = AIDX_TAIL;
    p_len = p - (u8*)idx;

    AIDX_FMT_SET(idx, mod, file, line, fmt, type, p_len);
    UPDATE_AND_PUT_ABLK(blk, p_len + ADX_HT_LEN);

    AOS_INTERRUPT_RESTORE

    if (need_flsuh)
        albf_mgr_notify_flush_task(s_log_buf);

#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    alog_trace_put_blk_log(&blk);
#endif /* ALOG_TRACE_UART_DIRECT */
}

/**
 * alog_output - output the trace log.
 *
 * @mod: the id of the module.
 * @level: the log level of this trace
 * @line: line number.
 * @fmt:  format string.
 */
void alog_output(u8 mod, u8 level, const char *fmt, ...)
{
    bool need_flsuh_1, need_flsuh_2 = FALSE;
    AOS_INTERRUPT_DECLARE
    bool try_again = TRUE;
    size_t data_len, left;
    AIDX_FMT_T *idx;
    ALBF_BLK_T blk;
    va_list args;
    int len = 0;
    u8 *p;

    AOS_INTERRUPT_DISABLE

    /* if it be filtered or get blk failed, return. */
    if (alog_filter_or_get_blk_failed(&blk, mod, level,
                                      ALOG_BUF_ONCE_MIN, &need_flsuh_1)) {
        AOS_INTERRUPT_RESTORE
        return;
    }

    do {
        /* init the idx. */
        p = (u8 *)blk.buf;
        *p++ = AIDX_HEAD;
        idx = (AIDX_FMT_T *)p;
        p += sizeof(*idx);
        left = blk.size - ADX_FIX_LEN;

        /* buid the trace body. */
        va_start(args, fmt);
        len = alog_vsnprintf(p, left, fmt, args);
        va_end(args);

        if (len < 0) {
            /* some error happen, skip this log. */
            AOS_INTERRUPT_RESTORE
            return;
        }

        /* buffer is enough, no need retry. */
        if ((size_t)len < left) {
            data_len = (size_t)len;
            break;
        }

        /*
         * len >= left, means only sucess write left - 1,
         * so get a new blk with ADX_FIX_LEN + len + 1, than sprintf it again.
         */
        if (s_log_buf && try_again) {
            blk.size = ADX_FIX_LEN + len + 1;
            need_flsuh_2 = albf_mgr_get_blk_again(s_log_buf, &blk);

            /* retry format trace only once. */
            try_again = FALSE;
            continue;
        }

        data_len = left;
        break;
    } while (TRUE);

    /* end the idx. */
    *(p + data_len) = AIDX_TAIL;
    AIDX_NOFMT_SET(idx, mod, data_len + sizeof(*idx));
    UPDATE_AND_PUT_ABLK(blk, data_len + ADX_FIX_LEN);

    AOS_INTERRUPT_RESTORE

    if (need_flsuh_1 || need_flsuh_2)
        albf_mgr_notify_flush_task(s_log_buf);

#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    alog_trace_put_blk_log(&blk);
#endif /* ALOG_TRACE_UART_DIRECT */
}

#ifdef ALOG_FAST_API_FEATURE
static inline void albf_mgr_put_blk_fast(ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf = s_log_buf->cur_buf;

    buf->wt_size += blk->size;
    AIC_DUMMY_ASSERT(buf->wt_size <= buf->size);
}

static inline bool albf_mgr_start_flush_fast(ALBF_MGR_T *mgr)
{
    u16 next_index;

    /* no log in write buffer, don't flush. */
    if (mgr->cur_buf->wt_size <= mgr->head_size)
        return FALSE;

    /* calculate next_index. */
    next_index = RING_NEXT_INDEX(mgr->wt_index, mgr->buf_cnt);

    /**
     * if the next_index overtake the rd_index,
     * means no empty buffer, need to overlap old buffer.
     */
    if (next_index == mgr->rd_index) {
        mgr->overtake++;
        if (mgr->wait_remote)
            mgr->wait_remote_cnt++;

        /**
         * if offset > 0, must skip it for the boot logs,
         * else move to the next index of the current read index.
         */
        /* if (mgr->offset) {
            if (next_index < mgr->offset)
                next_index = mgr->offset;
        } */

        /*
         * if buffer_busy is 1,  means the current read buffer is not ready,
         * the write index must skip the read index.
         */
        if (mgr->buffer_busy)
            next_index = RING_NEXT_INDEX(mgr->rd_index, mgr->buf_cnt);
    }

    /* update sn to old buffer and write_sn, and sn++. */
    mgr->cur_buf->sn = mgr->sn;
    mgr->write_sn = mgr->sn;
    mgr->sn++;

    /* update write_index, and move the current writer buffer to new buffer. */
    mgr->wt_index = next_index;
    mgr->cur_buf = mgr->bufs + next_index;
    mgr->cur_buf->wt_size = 0;
    mgr->cur_buf->sn = 0;

    return TRUE;
}

static inline bool albf_mgr_get_blk_again_fast(ALBF_MGR_T *mgr, ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf;
    bool need_flush;

    /*  flush cur write buffer. */
    need_flush = albf_mgr_start_flush_fast(mgr);

    /* a new buffer, reserve header size. */
    buf = mgr->cur_buf;
    buf->wt_size = mgr->head_size;
    blk->buf = buf->buf + buf->wt_size;

    /**
     * Regardless of the size of the request, blk->size set to the all buffer
     * size exclude the header. In the blk.size > buf->size - header size case,
     * just throwing away some logs.
     */
    // AIC_DUMMY_ASSERT(blk.size <= buf->size - buf->wt_size);
    blk->size = buf->size - buf->wt_size;

    return need_flush;
}

static inline bool albf_mgr_get_blk_fast(ALBF_MGR_T *mgr, ALBF_BLK_T *blk)
{
    ALOG_BUF_T *buf;
    bool ret = FALSE;
    buf = mgr->cur_buf;

    /* a new buffer, reserve header space. */
    if (buf->wt_size == 0) {
        /* Just put a semaphore to flush task to reset the flush timer. */
        ret = TRUE;
        buf->wt_size = mgr->head_size;
    }

    /* if left space is not enough, flush cur buf and jump to next buf */
    if (buf->wt_size + blk->size > buf->size)
        return albf_mgr_get_blk_again_fast(mgr, blk);

    /*
     * alloc buf, here can't do below code:
     * buf->wt_size += blk.size;
     * the actual size will updata in function albf_mgr_put_blk.
     */
    blk->buf = buf->buf + buf->wt_size;
    blk->size = buf->size - buf->wt_size;

    return ret;
}

PLAT_TCM_CODE void alog_output_fast(u8 mod, const char *fmt, ...)
{
    ALOG_INTERRUPT_DECLARE
    ALBF_BLK_T blk;
    bool need_flsuh;
    AIDX_FMT_T *idx;
    va_list args;
    int len, left;
    u8 *p;

    ALOG_INTERRUPT_DISABLE
    blk.size = ALOG_BUF_ONCE_MIN;
    need_flsuh = albf_mgr_get_blk_fast(s_log_buf, &blk);

    /* init the idx. */
    p = (u8 *)blk.buf;
    *p++ = AIDX_HEAD;
    idx = (AIDX_FMT_T *)p;
    p += sizeof(*idx);
    left = blk.size - ADX_FIX_LEN;

    /* buid the trace body. */
    va_start(args, fmt);
    len = alog_vsnprintf(p, left, fmt, args);
    va_end(args);

    if (len > left)
        len = left;

    if (len > 0) {
        /* end the idx. */
        *(p + len) = AIDX_TAIL;
        AIDX_NOFMT_SET(idx, mod, len + sizeof(*idx));
        blk.size = len + ADX_FIX_LEN;
        albf_mgr_put_blk_fast(&blk);
    }

    ALOG_INTERRUPT_RESTORE

    if (need_flsuh)
        aos_put_semaphore(s_log_buf->sema);
}

PLAT_TCM_CODE void alog_idx_output_fast(u8 mod, u32 fmt, const char *pf, ...)
{
    len_mod_type len_mod = len_mod_int;
    ALOG_INTERRUPT_DECLARE
    size_t p_len = 0;
    bool need_flsuh;
    AIDX_FMT_T *idx;
    ALBF_BLK_T blk;
    const char *arg_f;
    u8 *p, *end;
    va_list ap;

    ALOG_INTERRUPT_DISABLE
    blk.size = AIDX_MIN_BUF_SIZE;
    need_flsuh = albf_mgr_get_blk_fast(s_log_buf, &blk);

    /*
     * init the head, idx pointer and end pointer,
     * the end pointer point to the tail.
     */
    p = (u8 *)blk.buf;
    end = p + blk.size - 1;
    *p++ = AIDX_HEAD;
    idx = (AIDX_FMT_T *)p;
    p += sizeof(*idx);

    /* get the fmt params according pf. */
    va_start(ap, pf);
    for (arg_f = pf; *arg_f; arg_f++) {
        switch(*arg_f) {
            /* char, short and int arg all push to stack 4byte, so all used int type. */
            case INT_CASES:
                if (len_mod == len_mod_long) {
                    GET_A_TYPE_PARAM(long, ap, p, end);
                } else if (len_mod == len_mod_long_long) {
                    GET_A_64BIT_TYPE_PARAM(long long, ap, p, end);
                } else {
                    GET_A_TYPE_PARAM(int, ap, p, end);
                }

                len_mod = len_mod_int;
                break;

            case FP_CASES:
                if (len_mod == len_mod_long_dobule) {
                    GET_A_BIG_TYPE_PARAM(long double, ap, p, end);
                } else {
                    /*
                     * notify: ''float' will be promoted to 'double'
                     * when passed through '...'.
                     */
                    GET_A_64BIT_TYPE_PARAM(double, ap, p, end);
                }
                len_mod = len_mod_int;
                break;

            case STR_CASES:
                GET_A_STR_PARAM(ap, p, end);
                len_mod = len_mod_int;
                break;

            case LENMOD_CASES:
                if (*arg_f == 'l') {
                    if (len_mod == len_mod_int)
                        len_mod = len_mod_long;
                    else if (len_mod == len_mod_long)
                        len_mod = len_mod_long_long;
                } else if (*arg_f == 'L') {
                    /* 'L' is long double. */
                    len_mod = len_mod_long_dobule;
                } else {
                    len_mod = len_mod_int;
                }
            break;

            default:
                //AIC_DUMMY_ASSERT(0);
                AIC_ASSERT(0);
                break;
        }
    }
    va_end(ap);

    /* end the idx. */
    *(p) = AIDX_TAIL;
    p_len = p - (u8*)idx;

    AIDX_FMT_SET(idx, mod, 0, 0, fmt, AIDX_FMT_NORMAL, p_len);
    blk.size = p_len + ADX_HT_LEN;
    albf_mgr_put_blk_fast(&blk);

    ALOG_INTERRUPT_RESTORE

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
                c = 'x';
        }
        pre = c;
        buf[i] = c;
    }

    buf[i] = 0;
}

void alog_bitstr_output(u8 mod, u8 lv, const char *fmt, ...)
{
    ALOG_INTERRUPT_DECLARE
    char fmt_buf[MAX_BIT_STR_LEN + 1];
    ALBF_BLK_T blk;
    bool need_flsuh;
    AIDX_FMT_T *idx;
    va_list args;
    int len, left;
    u8 *p;

    if (lv > alog_get_log_level(mod))
        return;

    ALOG_INTERRUPT_DISABLE
    blk.size = ALOG_BUF_ONCE_MIN;
    need_flsuh = albf_mgr_get_blk(s_log_buf, &blk);

    /* init the idx. */
    p = (u8 *)blk.buf;
    *p++ = AIDX_HEAD;
    idx = (AIDX_FMT_T *)p;
    p += sizeof(*idx);
    left = blk.size - ADX_FIX_LEN;

    /* buid the trace body. */
    alog_bitstr_preprocess_fmt(fmt, fmt_buf);
    va_start(args, fmt);
    len = alog_vsnprintf(p, left, fmt_buf, args);
    va_end(args);

    if (len > left)
        len = left;

    if (len > 0) {
        /* end the idx. */
        *(p + len) = AIDX_TAIL;
        AIDX_NOFMT_SET(idx, mod, len + sizeof(*idx));
        blk.size = len + ADX_FIX_LEN;
        albf_mgr_put_blk(s_log_buf, &blk);
    }

    ALOG_INTERRUPT_RESTORE

    if (need_flsuh)
        aos_put_semaphore(s_log_buf->sema);
}
#endif

