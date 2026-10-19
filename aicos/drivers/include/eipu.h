/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the head software driver of eipu module.
 *   It includes all the eipu common function api.
*/

#ifndef __EIPU_H__
#define __EIPU_H__
#include <timer_inc.h>

#define BUF_SIZE 10
#ifdef CONFIG_EIPU_R1P0
#define EIPU_CHN_NUM  12
#else
#define EIPU_CHN_NUM  9
#endif
typedef enum _eipu_func_type_e
{
    EIPU_LATCH = 0,
    EIPU_EDGE_32K,
    EIPU_EDGE,
    EIPU_DBNC,
} EIPU_FUNC_TYPE_E;

#ifdef CONFIG_EIPU_R1P0
typedef enum _eipu_mode_type_e
{
#ifdef CONFIG_SUBSYS_CP
    CP_MODE = 0x0,
#else
    NOSEC_MODE = 0x0,
#endif
    MAX_MODE,
} EIPU_MODE_TYPE_E;
#else
typedef enum _eipu_mode_type_e
{
#ifdef CONFIG_SUBSYS_CP
    CP_MODE = 0x0,
#else
    NOSEC_MODE = 0x0,
    ND_MODE,
#endif
    MAX_MODE,
} EIPU_MODE_TYPE_E;
#endif

typedef enum _eipu_work_mode_e
{
    BOTH_WK_MODE = 0x0,
    SLEEP_WK_MODE,
} EIPU_WORK_MODE_E;

typedef void (*EIPU_CALLBACK)(u32 id, u32 state, u32 pol);

typedef struct _eipu_chan_t
{
    EIPU_FUNC_TYPE_E type;
    EIPU_CALLBACK callback;
    EIPU_WORK_MODE_E wk_mode;
} EIPU_CHAN_T;

typedef struct _eipu_pin_t
{
    u32 eiu_pinnum;
    u32 eiu_ctrlnum;
} EIPU_PIN_T;

typedef enum _eipu_trg_type_e
{
    LOW_LEVEL = 0x0,
    HIGH_LEVEL,
    RISING_EDGE,
    FALLING_EDGE,
    BOTH_EDGE
} EIPU_TRG_TYPE_E;

typedef struct _save_tick_t {
    u32 state;
    u32 bit_index;
    u32 value;
    u32 eipu_intmode;
    u32 eipu_both;
    u32 eipu_pol;
    u32 tick;
} SAVE_TICK_T;

typedef struct _save_info_t {
    SAVE_TICK_T tick_buf[BUF_SIZE];
    u32 save_idx;
    u32 save_hidx;
    u32 loop_count;
    u32 case_index;
} SAVE_INFO_T;

#ifdef CONFIG_EIPU_R1P0
typedef enum _eipu_chan_e
{
    EXTINT_UART2 = 8,
    EXTINT_UART1 = 9,
    EXTINT_CP    = 10,
    EXTINT_UART5 = 11,
} EIPU_CHAN_E;
#else
#define EXTINT_CP 8
#endif

extern SAVE_INFO_T sa_info;

#define SAVE_TICK(a,b,c,d,e,f) \
                sa_info.tick_buf[sa_info.save_idx].state        = a;      \
                sa_info.tick_buf[sa_info.save_idx].bit_index    = b;      \
                sa_info.tick_buf[sa_info.save_idx].value        = c;      \
                sa_info.tick_buf[sa_info.save_idx].eipu_intmode = d;      \
                sa_info.tick_buf[sa_info.save_idx].eipu_both    = e;      \
                sa_info.tick_buf[sa_info.save_idx].eipu_pol     = f;      \
                sa_info.tick_buf[sa_info.save_idx].tick         = tmr_get_frcnt();      \
                if (sa_info.save_idx < BUF_SIZE - 1)                      \
                    sa_info.save_idx++;                                   \
                else {                                                    \
                    sa_info.save_idx = 0;                                 \
                    sa_info.save_hidx++;                                  \
                }                                                         \

int EIPU_UnRegCallback(u32 chan_id, EIPU_FUNC_TYPE_E eipu_type, EIPU_MODE_TYPE_E sec_mode);
int EIPU_RegCallback(u32 chan_id,
                     EIPU_FUNC_TYPE_E eipu_type,
                     EIPU_CALLBACK callback,
                     EIPU_TRG_TYPE_E trg_type_e,
                     u32 dbnc_cnt_ms,
                     EIPU_MODE_TYPE_E sec_mode);
int EIPU_RegCallback_Ex(u32 chan_id,
                     EIPU_FUNC_TYPE_E eipu_type,
                     EIPU_CALLBACK callback,
                     EIPU_TRG_TYPE_E trg_type_e,
                     u32 dbnc_cnt_ms,
                     EIPU_MODE_TYPE_E sec_mode,
                     EIPU_WORK_MODE_E work_mode);
int EIPU_Reset(void);
void EIPU_IntDisAll(void);
void EIPU_SetPol(EIPU_MODE_TYPE_E sec_mode,EIPU_FUNC_TYPE_E eipu_type,u32 chn_id,u32 pol_val);
void EIPU_SetSel(u32 chn_id,u32 chn_sel);
u32 EIPU_GetBase(EIPU_MODE_TYPE_E sec_mode);
u32 EIPU_GetIndata(EIPU_MODE_TYPE_E sec_mode,EIPU_FUNC_TYPE_E eipu_type,u32 chn_id);
#ifdef CONFIG_SUBSYS_AP
u32 EIPU_GetIndata_Ex(u32 pin_idx);
#endif
#endif /* __EIPU_H__ */
