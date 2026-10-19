/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the software driver of pmic rtc module, it is pmic rtc
 *    phy driver header file.
 */

u32 gx_rtc_pmic_phy_version(void);
u32 gx_rtc_pmic_phy_get_int_status(void);
void gx_rtc_pmic_phy_int_clr(void);
void gx_rtc_pmic_phy_set_int_mode(RTC_INT_MODE mod);
void gx_rtc_pmic_phy_read_time(struct rtc_time *tm);
void gx_rtc_pmic_phy_set_time(struct rtc_time *tm);
void gx_rtc_pmic_phy_alarm_enable(BOOL enable);
void gx_rtc_pmic_phy_read_alarm(struct rtc_time *ta);
void gx_rtc_pmic_phy_set_alarm(struct rtc_time *ta);
void gx_rtc_pmic_phy_del_alarm(void);
void gx_rtc_pmic_phy_init(void);

