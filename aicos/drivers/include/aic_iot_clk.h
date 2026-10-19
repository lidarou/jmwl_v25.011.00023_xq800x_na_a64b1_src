
/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the INTERNAL elements used by aic iot clock driver.
 */

#ifndef __AIC_IOT_CLK_H__
#define __AIC_IOT_CLK_H__

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_type.h>

struct clk_gate {
    u32 reg;
    u8 clk_id;

    u8 index;
};

struct clk_pll_gate {
    struct clk_gate gate;
    u8 enable_cnt;
    /* unit is us */
    u8 delay_time;
};

struct clk_parent {
    u8 index;
    u32 rate;
};

struct clk_mux {
    u32 reg;
    u8 clk_id;

    const struct clk_parent *const parents;
    u8 parents_num;
    u8 cur_parent_index;

    u8 mux_width;
    u8 mux_shift;
};

struct clk_div {
    u32 reg;
    u8 clk_id;

    u32 prate;

    u8 div_width;
    u8 div_shift;
};

struct clk_comp {
    u32 reg;
    u8 clk_id;
    const struct clk_parent *const parents;
    u8 parents_num;
    u8 cur_parent_index;

    u8 mux_width;
    u8 mux_shift;
    u8 div_width;
    u8 div_shift;
};

#ifdef CONFIG_AIC_CLK
int clk_set_parent_mux(u8 clk_id, u8 index);
int clk_set_parent_comp(u8 clk_id, u8 index);

int clk_set_rate_comp(u8 clk_id, u32 rate);
int clk_set_rate_div(u8 clk_id, u32 rate);

int clk_enable(u8 clk_id);
int clk_disable(u8 clk_id);
int clk_pll_enable(u8 clk_id);
int clk_pll_disable(u8 clk_id);

u8 clk_get_parent_mux(u8 clk_id);
u8 clk_get_parent_comp(u8 clk_id);

u32 clk_get_rate_comp(u8 clk_id);
u32 clk_get_rate_mux(u8 clk_id);
u32 clk_get_rate_div(u8 clk_id);
#else
/* dumy it for compile */
static inline int clk_set_parent_mux(u8 clk_id, u8 index) {return 0;}
static inline int clk_set_parent_comp(u8 clk_id, u8 index) {return 0;}

static inline int clk_set_rate_comp(u8 clk_id, u32 rate) {return 0;}
static inline int clk_set_rate_div(u8 clk_id, u32 rate) {return 0;}

static inline int clk_enable(u8 clk_id) {return 0;}
static inline int clk_disable(u8 clk_id) {return 0;}
static inline int clk_pll_enable(u8 clk_id) {return 0;}
static inline int clk_pll_disable(u8 clk_id) {return 0;}

static inline u8 clk_get_parent_mux(u8 clk_id) {return 0;}
static inline u8 clk_get_parent_comp(u8 clk_id) {return 0;}

static inline u32 clk_get_rate_comp(u8 clk_id) {return 0;}
static inline u32 clk_get_rate_mux(u8 clk_id) {return 0;}
static inline u32 clk_get_rate_div(u8 clk_id) {return 0;}
#endif /* CONFIG_AIC_CLK */

#endif /* __ACI_IOT_CLK_H__ */
