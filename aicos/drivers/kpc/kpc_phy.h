/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __KPC_H__
#define __KPC_H__

#include <aic_type.h>
#include <reg_base.h>

#include "kpc_rf.h"

#undef REG_BASE_KPC
#define REG_BASE_KPC    REG_BASE_AON_KPC

struct ver_info {
    u8 main_ver;
    u8 sub_ver;
};

struct version_info {
    u8 main_version;
    u8 sub_version;
};

enum key_num {
    KEY0 = 0,
    KEY1,
    KEY2,
    KEY3,
};

static void kpc_phy_get_version(struct ver_info *v_info);
static void kpc_phy_longpress_en(boolean enable);
static void kpc_phy_auto_slp_en(boolean enable);
static void kpc_phy_enable(boolean enable);
static void kpc_phy_row_config(u32 rows);
static void kpc_phy_col_config(u32 cols);
static void kpc_phy_ghostkey_int_enable(boolean enable);
static void kpc_phy_longpress_int_enable(boolean enable);
static void kpc_phy_release_int_enable(boolean enable);
static void kpc_phy_press_int_enable(boolean enable);
static void kpc_phy_polarity(boolean level);
static void kpc_phy_longkey_cnt(u32 count);
static void kpc_phy_auto_slp_cnt(u32 count);
static void kpc_phy_clk_div_cnt(u32 div);
static void kpc_phy_debounce_cnt(u32 press_debounce_cnt, u32 release_debounce_cnt);
static void kpc_phy_int_clr_all(void);
static int kpc_init(void);

#endif /* __KPC_H__ */

