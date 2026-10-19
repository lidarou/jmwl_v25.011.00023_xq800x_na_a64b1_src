/*                                                                        */
/**************************************************************************/
/*  Copyright 2024 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Implmentation APIs to split parameters of AT commands
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1170a8
#define LOG_MODULE    AMOD_NET

/*******************************************
 *              INCLUDES
 ******************************************/
#include "cm_at_split_paras.h"

/*******************************************
 *              DEFINES
 ******************************************/
#define AT_CM_SPLIT_UNUSED(x) ((void)x)

/*******************************************
 *              TYPEDEFS
 ******************************************/
enum {
    AT_CM_SPLIT_STATE_NONE = 0,
    AT_CM_SPLIT_STATE_QS_FETCH,
    AT_CM_SPLIT_STATE_NONE_QS_FETCH,
    AT_CM_SPLIT_STATE_COMMA,
    AT_CM_SPLIT_STATE_FINISH,
};

enum {
    AT_CM_SPLIT_FETCH_QS_STATE_NONE,
    AT_CM_SPLIT_FETCH_QS_STATE_NORMAL,
    AT_CM_SPLIT_FETCH_QS_STATE_FIN,
    AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE,
    AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_OCT,
    AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_HEX,
};

typedef struct {
    ulong *paras_arr;
    u16 total;
    u16 used;
} AT_CM_SPLIT_PARAS;

static s32 aic_cm_split_paras_add(AT_CM_SPLIT_PARAS *dest, const u8 *str_b, u32 str_len, u8 is_str)
{
    u8 *para_entry = AIC_NULL;

#define AIC_CM_SPLIT_PARAS_STEP 15

    if (!str_b || !dest || (is_str && str_len < 2)) {
        return AIC_EINVAL;
    }

    /* check para table memory first */
    if (!dest->paras_arr) {
        dest->paras_arr = (ulong *)aos_zalloc(AIC_CM_SPLIT_PARAS_STEP * sizeof(ulong *));
        if (!dest->paras_arr) {
            return AIC_ENOMEM;
        }
        dest->total = AIC_CM_SPLIT_PARAS_STEP;
    } else {
        if (dest->used == dest->total) {
            ulong *new_paras_arr = NULL;

            new_paras_arr = (ulong *)aos_zalloc((AIC_CM_SPLIT_PARAS_STEP + dest->total) * sizeof(ulong *));
            if (!new_paras_arr) {
                return AIC_ENOMEM;
            }
            aos_memcpy(new_paras_arr, dest->paras_arr, sizeof(ulong *) * dest->total);
            aos_free(dest->paras_arr);
            dest->paras_arr = new_paras_arr;
            dest->total += AIC_CM_SPLIT_PARAS_STEP;
        }
    }
    /* prepre entry */
    if (is_str) {
        /* -2: cut off qutations, +1: '\0', +1: string flag */
        para_entry = (u8 *)aos_zalloc(str_len - 2 + 1 + 1);
        if (!para_entry) {
            return AIC_ENOMEM;
        }
        /* copy string */
        if (str_len > 2) {
            aos_memcpy(para_entry, str_b + 1, str_len - 2);
        }
        /* set string flag */
        para_entry[str_len - 1] = AIC_TRUE;
    } else {
        /* +1: '\0', +1: string flag */
        para_entry = (u8 *)aos_zalloc(str_len + 1 + 1);
        if (!para_entry) {
            return AIC_ENOMEM;
        }
        /* copy string */
        if (str_len) {
            aos_memcpy(para_entry, str_b, str_len);
        }
        /* set string flag */
        para_entry[str_len + 1] = AIC_FALSE;
    }
    /* attach the new entry to the end of list */
    dest->paras_arr[dest->used] = (ulong)para_entry;
    dest->used++;

    return AIC_SUCCESS;
}

static s32 aic_cm_split_paras_fetch_quotated_string(const u8 *str_begin, size_t str_end, u32 *length, u32 *offset)
{
    s32 ret = AIC_SUCCESS;
    u32 i = 0;
    u32 state = AT_CM_SPLIT_FETCH_QS_STATE_NONE;
    u32 escape_len = 0;

    while (i < str_end) {
        u8 ch = str_begin[i];

        switch (ch) {
        case '\"':
            i++;
            if (state == AT_CM_SPLIT_FETCH_QS_STATE_NONE) {
                state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
            } else if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE) {
                /* [tricky here] even if it is ugly we still have to do this as this is what customer wants */
                if (i == str_end) {
                    state = AT_CM_SPLIT_FETCH_QS_STATE_FIN;
                    goto EXIT;
                } else {
                    state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
                }
            } else {
                state = AT_CM_SPLIT_FETCH_QS_STATE_FIN;
                goto EXIT;
            }
            break;
        case '\\':
            if (state == AT_CM_SPLIT_FETCH_QS_STATE_NORMAL) {
                state = AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE;
            } else if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE) {
                state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
            } else if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_OCT ||
                       state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_HEX) {
                state = AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE;
            }
            i++;
            break;
        default:
            if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE) {
                if (ch >= 0 && ch <= '7') {
                    escape_len = 1;
                    state = AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_OCT;
                } else if (ch == 'x') {
                    escape_len = 0;
                    state = AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_HEX;
                } else if (strchr("abfnrtv'?", ch)) {
                    /* \a \b \f \n \r \t \v \' \? */
                    state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
                } else {
                    state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
                }
            } else if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_OCT) {
                if (escape_len < 3 && ch >= 0 && ch <= '7') {
                    escape_len++;
                } else {
                    escape_len = 0;
                    state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
                }
            } else if (state == AT_CM_SPLIT_FETCH_QS_STATE_ESCAPE_HEX) {
                if (escape_len < 2 && strchr("0123456789abcdefABCDEF", ch)) {
                    escape_len++;
                } else {
                    escape_len = 0;
                    state = AT_CM_SPLIT_FETCH_QS_STATE_NORMAL;
                }
            }
            i++;
            break;
        }
    }

EXIT:
    if (state == AT_CM_SPLIT_FETCH_QS_STATE_FIN) {
        *offset = i;
        *length = i - 2;
    } else {
        ret = AIC_EINVAL;
    }

    return ret;
}

static s32 aic_cm_split_paras_fetch_non_quotated_string(const u8 *str_begin, size_t str_end, u32 *length, u32 *offset)
{
    s32 ret = AIC_SUCCESS;
    u32 i = 0;

    while (i < str_end) {
        u8 ch = str_begin[i];

        if (ch == ',' || ch == '\r' || ch == '\n') {
            break;
        }
        i++;
    }
    *length = i;
    *offset = i;

    return ret;
}

u8 aic_cm_split_para_is_quotated_str(ulong para)
{
    char *str = (char *)para;
    u8 *para_u8 = (u8 *)para;

    if (str) {
        return *(para_u8 + strlen(str) + 1);
    }

    return AIC_FALSE;
}

void aic_cm_free_paras_mem(ulong *ptr, u16 paras_cnt)
{
    u16 i;

    if (!ptr) {
        return;
    }
    for (i = 0; i < paras_cnt; i++) {
        if (ptr[i]) {
            aos_free((void *)(ptr[i]));
        }
    }
    aos_free(ptr);
}

ulong *aic_cm_split_paras_to_str_array(const u8 *raw_str, const u8 *start_str, u16 *paras_cnt, const u8 *format)
{
    size_t at_cmd_len = 0;
    size_t at_prefix_len = 0;
    u8 last_state = AT_CM_SPLIT_STATE_NONE;
    u32 iter = 0;
    u32 para_b = 0, para_e = 0;
    s32 ret = 0;
    AT_CM_SPLIT_PARAS final_paras = {0};

    /* check parameters */
    if (!paras_cnt)
        return AIC_NULL;
    *paras_cnt = 0;
    if (!raw_str || !start_str)
        return AIC_NULL;
    at_cmd_len = strlen((const char *)raw_str);
    at_prefix_len = strlen((const char *)start_str);
    if (at_cmd_len <= at_prefix_len || !strcasestr((const char *)raw_str, (const char *)start_str)) {
        *paras_cnt = 0;
        return AIC_NULL;
    }
    if (at_cmd_len > 2) {
        if (raw_str[at_cmd_len - 2] == '\r' || raw_str[at_cmd_len - 2] == '\n') {
            at_cmd_len -= 2;
        } else if (raw_str[at_cmd_len - 1] == '\r' || raw_str[at_cmd_len - 1] == '\n') {
            at_cmd_len -= 1;
        }
    }
    iter = at_prefix_len;
    para_b = iter;
    para_e = iter;
    while (iter < at_cmd_len + 1) {
        u8 cur_char = raw_str[iter];
        u32 para_len = 0, offset = 0;

        switch (cur_char) {
        case '\"':
            if (last_state != AT_CM_SPLIT_STATE_COMMA && last_state != AT_CM_SPLIT_STATE_NONE) {
                goto ERROR_EXIT;
            }
            ret = aic_cm_split_paras_fetch_quotated_string(raw_str + iter, at_cmd_len - iter, &para_len, &offset);
            if (ret != AIC_SUCCESS)
                goto ERROR_EXIT;
            if (aic_cm_split_paras_add(&final_paras, raw_str + iter, offset, AIC_TRUE)) {
                goto ERROR_EXIT;
            }
            last_state = AT_CM_SPLIT_STATE_QS_FETCH;
            iter += offset;
            break;
        case ',':
            if (last_state == AT_CM_SPLIT_STATE_COMMA || last_state == AT_CM_SPLIT_STATE_NONE) {
                /* empty parameter */
                if (aic_cm_split_paras_add(&final_paras, raw_str + iter, 0, AIC_FALSE)) {
                    goto ERROR_EXIT;
                }
            }
            last_state = AT_CM_SPLIT_STATE_COMMA;
            iter++;
            break;
        case '\r':
        case '\n':
        case '\0': /* picket line, need to handle in case the last character is a comma */
            if (last_state == AT_CM_SPLIT_STATE_COMMA) {
                /* empty parameter */
                if (aic_cm_split_paras_add(&final_paras, raw_str + iter, 0, AIC_FALSE)) {
                    goto ERROR_EXIT;
                }
            }
            last_state = AT_CM_SPLIT_STATE_FINISH;
            iter++;
            break;
        default:
            if (last_state != AT_CM_SPLIT_STATE_COMMA && last_state != AT_CM_SPLIT_STATE_NONE) {
                goto ERROR_EXIT;
            }
            ret = aic_cm_split_paras_fetch_non_quotated_string(raw_str + iter, at_cmd_len - iter, &para_len, &offset);
            if (ret != AIC_SUCCESS)
                goto ERROR_EXIT;
            if (aic_cm_split_paras_add(&final_paras, raw_str + iter, offset, AIC_FALSE)) {
                goto ERROR_EXIT;
            }
            last_state = AT_CM_SPLIT_STATE_NONE_QS_FETCH;
            iter += offset;
            break;
        }
    }
    *paras_cnt = final_paras.used;

    return final_paras.paras_arr;

ERROR_EXIT:
    aic_cm_free_paras_mem(final_paras.paras_arr, final_paras.used);

    return NULL;
}