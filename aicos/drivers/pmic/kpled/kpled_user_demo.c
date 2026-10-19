/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  kpled operate interface demo.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15ad9d
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <pmu_user.h>
#include <pmu_driver.h>

int kpled_demo(void)
{
    struct pmu_user *puser;
#ifdef CONFIG_TSC
    struct pmu_user *padc_user;
#endif
    int ret;

    puser = pmu_get("kpled_user_demo", "kpled");
    if (!puser) {
        ret = AIC_ENXIO;
        aicx_warn(0x09b1f3, "s", "%s get pmu operate handler fail!\n", __func__);
        goto out;
    }
    if (pmu_is_enabled(puser))
        aicx_warn(0x0e055c, "s", "%s is enabled!!!\n", puser->punit->unit_name);
    else
        aicx_warn(0x182106, "s", "%s is not enabled!\n", puser->punit->unit_name);

    ret = pmu_enable(puser);
    if (ret) {
        aicx_warn(0x06e33d, "s", "%s enable fail!\n", __func__);
        goto out;
    }

    ret = pmu_current_set(puser, 80000, 5000);
    if (ret) {
        aicx_warn(0x132284, "s", "%s set voltage fail!\n", __func__);
        goto out;
    }

#ifdef CONFIG_TSC
    padc_user = pmu_get("user_demo", "adc");
    if (!padc_user) {
        ret = AIC_ENXIO;
        aicx_warn(0x0f8060, "s", "%s get adc pmu operate handler fail!\n", __func__);
        goto out;
    }

    ret = pmu_enable(padc_user);
    if (ret) {
        aicx_warn(0x059007, "", "backlight enable adc fail!\n");
        goto out;
    }

    pmu_put(padc_user);
#endif
/*
    int current, voltage;
    struct pmu_user *buck0_user;

    ret = pmu_current_get(puser, &current);
    if (ret) {
        alog_warn("%s get current fail!\n", __func__);
        goto out;
    }
    alog_warn("%s current:%d\n", puser->punit->unit_name, current);

    if (pmu_is_enabled(puser))
        alog_warn("%s is enabled!!!\n", puser->punit->unit_name);
    else
        alog_warn("%s is not enabled!\n", puser->punit->unit_name);

    buck0_user = pmu_get("pmu_demo", "VDDCORE");
    if (buck0_user) {
        ret = pmu_voltage_set(buck0_user, 900000, 10000);
        if (ret) {
            alog_warn("%s set voltage fail!\n", __func__);
            goto out;
        }

        ret = pmu_voltage_get(buck0_user, &voltage);
        if (ret) {
            alog_warn("%s set voltage fail!\n", __func__);
            goto out;
        }

        if (voltage == 900000)
            alog_warn("voltage set get success!!!\n");
        else
            alog_warn("voltage set:900000mV get:%d failed!!!\n", voltage);
    }

    if (pmu_is_enabled(buck0_user))
        alog_warn("%s is enabled!!!\n", buck0_user->punit->unit_name);
    else
        alog_warn("%s is not enabled!\n", buck0_user->punit->unit_name);


    ret = pmu_disable(puser);
    if (ret) {
        alog_warn("%s disable fail!\n", __func__);
        goto out;
    }

    pmu_put(puser);
*/
out:
    return AIC_SUCCESS;
}
AIC_NORMAL_DRV_L2_INIT_EXPORT(kpled_demo);
