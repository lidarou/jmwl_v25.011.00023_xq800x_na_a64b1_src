/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pmu operate interface demo.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x149f6f
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>

#include <pmu_user.h>

int pmu_demo(void)
{
    struct pmu_user *puser;
    int ret, voltage;

    puser = pmu_get("pmu_demo_app", "AVDDCAM2V8");
    if (!puser) {
        ret = AIC_ENXIO;
        aicx_warn(0x09b1f3, "s", "%s get pmu operate handler fail!\n", __func__);
        goto out;
    }

    ret = pmu_enable(puser);
    if (ret) {
        aicx_warn(0x06e33d, "s", "%s enable fail!\n", __func__);
        goto out;
    }

    ret = pmu_voltage_set(puser, 2500000, 10000);
    if (ret) {
        aicx_warn(0x132284, "s", "%s set voltage fail!\n", __func__);
        goto out;
    }

    ret = pmu_voltage_get(puser, &voltage);
    if (ret) {
        aicx_warn(0x0e40c7, "s", "%s get voltage fail!\n", __func__);
        goto out;
    }

    ret = pmu_disable(puser);
    if (ret) {
        aicx_warn(0x036995, "s", "%s disable fail!\n", __func__);
        goto out;
    }

    pmu_put(puser);

out:
    return AIC_SUCCESS;
}
AIC_DRV_L2_INIT_EXPORT(pmu_demo);
