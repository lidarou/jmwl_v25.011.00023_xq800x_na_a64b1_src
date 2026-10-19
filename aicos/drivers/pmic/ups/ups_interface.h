/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of ups module, it is ups
 *    phy driver header file.
 */

#define GX_UPS_CLK          32

void gx_ups_phy_mode_enter(void);
void gx_ups_phy_mode_exit(void);
void gx_ups_phy_cnt_clr(void);
u32 gx_ups_phy_version(void);
void gx_ups_phy_int_clr(void);
u32 gx_ups_phy_get_int_status(void);
void gx_ups_phy_start_count(void);
void gx_ups_phy_set_comp(u32 cnt);
u32 gx_ups_phy_get_cnt(void);
void gx_ups_phy_set_sw_reg(u32 val);
u32 gx_ups_phy_get_sw_reg(void);
void gx_ups_phy_set_period(void);
void gx_ups_phy_to_rtc(void);
void gx_ups_phy_from_rtc(void);
void gx_ups_phy_init(void);

