/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  lamp operate interface.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0cd386
#define LOG_MODULE    AMOD_PMIC

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <pmu_user.h>
#include <pmu_driver.h>

#ifdef CONFIG_LAMP_INIT_CURRENT
#define LAMP_INIT_CURRENT CONFIG_LAMP_INIT_CURRENT
#else
#define LAMP_INIT_CURRENT (60000)
#endif

aos_mutex_ptr lamp_mutex = AOS_NULL;
int lamp_units_num = 0;

static LIST_HEAD(lamp_units_list);

struct lamp_unit {
    struct pmu_user *unit;
    struct list_head unit_list; /*current lamp unit*/
};

int lamp_is_enabled(void)
{
    struct lamp_unit *unit_node;
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(lamp_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    list_for_each_entry(unit_node, &lamp_units_list, unit_list, struct lamp_unit) {
        if (!unit_node->unit)
            continue;
        ret |= pmu_is_enabled(unit_node->unit);
    }

    if (aos_is_user_thread_context())
        aos_put_mutex(lamp_mutex);

    return ret;
}

int lamp_enabled(bool enabled)
{
    struct lamp_unit *unit_node;
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(lamp_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    list_for_each_entry(unit_node, &lamp_units_list, unit_list, struct lamp_unit) {
        if (!unit_node->unit)
            continue;

        if (enabled)
            ret |= pmu_enable(unit_node->unit);
        else
            ret |= pmu_disable(unit_node->unit);
    }

    if (aos_is_user_thread_context())
        aos_put_mutex(lamp_mutex);

    return ret;
}

int lamp_set_current(u32 current_uA)
{
    u32 current_each = current_uA / lamp_units_num;
    struct lamp_unit *unit_node;
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(lamp_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    list_for_each_entry(unit_node, &lamp_units_list, unit_list, struct lamp_unit) {
        if (!unit_node->unit)
            continue;

        ret |= pmu_current_set(unit_node->unit, current_each, 800);
    }

    if (aos_is_user_thread_context())
        aos_put_mutex(lamp_mutex);

    return ret;
}

int lamp_get_current(u32 *current_uA)
{
    struct lamp_unit *unit_node;
    u32 each_current_uA;
    int ret = 0;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(lamp_mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    list_for_each_entry(unit_node, &lamp_units_list, unit_list, struct lamp_unit) {
        if (!unit_node->unit)
            continue;

        ret |= pmu_current_get(unit_node->unit, &each_current_uA);
        if (each_current_uA)
            *current_uA += each_current_uA;
    }

    if (aos_is_user_thread_context())
        aos_put_mutex(lamp_mutex);

    return ret;
}

int lamp_unit_register(struct pmu_user *lamp_unit_pmu)
{
    struct lamp_unit *lunit;
    int ret;

    if (!lamp_unit_pmu) {
        aicx_warn(0x106a1d, "s", "%s invaild lamp unit pmu!\n", __func__);
        ret = AIC_ENOSPC;
    }

    /* register lamp unit node into linklist*/
    lunit = aos_zalloc(sizeof(struct lamp_unit));
    if (!lunit) {
        ret = AIC_ENOSPC;
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        return ret;
    }

    list_head_init(&lunit->unit_list);
    lunit->unit = lamp_unit_pmu;
    list_add_tail(&lunit->unit_list, &lamp_units_list);
    lamp_units_num ++;

    return ret;
}

int lamp_hal_init(void)
{
    int ret;

    lamp_mutex = aos_create_mutex("lamp_hal");
    AIC_ASSERT(lamp_mutex);

    ret = lamp_set_current(LAMP_INIT_CURRENT);

    return 0;
}
AIC_DRV_P2_INIT_EXPORT(lamp_hal_init);
