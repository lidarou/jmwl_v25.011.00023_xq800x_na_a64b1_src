/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  pmu core operate interface implementation.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x114a7c
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <aic_list.h>
#include <pmu_user.h>
#include <pmu_driver.h>
#include <timer_inc.h>
/*
 * variable & macro definition.
 */
static LIST_HEAD(pmu_unit_list);
static LIST_HEAD(pmu_users_list);

aos_mutex_ptr pmu_unit_list_mutex = AOS_NULL;
aos_mutex_ptr pmu_users_list_mutex = AOS_NULL;

/*
 * static local operate interface.
 */
static aos_status pmu_lock(aos_mutex_ptr mutex)
{
    int ret = AOS_SUCCESS;

    if (aos_is_user_thread_context()) {
        ret = aos_get_mutex(mutex, AOS_WAIT_FOREVER);
        AIC_ASSERT(ret == AOS_SUCCESS);
    }

    return ret;
}

static void pmu_unlock(aos_mutex_ptr mutex)
{
    if (aos_is_user_thread_context()) {
        aos_put_mutex(mutex);
    }
}

static struct pmu_unit *pmu_search_unit(char *unit_name)
{
    struct pmu_unit *punit = NULL;
    struct pmu_unit *punit_temp;

    if (!unit_name)
        return punit;

    /*search pmu unit list, lookup tagert node*/
    pmu_lock(pmu_unit_list_mutex);
    list_for_each_entry(punit_temp, &pmu_unit_list, unit_list, struct pmu_unit) {
        if (!strcmp(punit_temp->unit_name, unit_name)) {
            punit = punit_temp;
            break;
        }
    }
    pmu_unlock(pmu_unit_list_mutex);

    return punit;
}


/*
 * pmu interface.
*/
struct pmu_user *pmu_get(char *user_name, char *unit_name)
{
    struct pmu_unit *punit = NULL;
    struct pmu_user *puser = NULL;

    if (!user_name || !unit_name) {
        aicx_warn(0x01b96d, "s", "%s user name or unit name NULL!\n", __func__);
        goto out;
    }

    punit = pmu_search_unit(unit_name);
    if (!punit) {
        aicx_warn(0x0b9ee7, "ss", "%s unregister unit:%s.\n", __func__, unit_name);
        goto out;
    }

    /* create pmu user node*/
    puser = aos_zalloc(sizeof(struct pmu_user));
    if (!puser) {
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        goto out;
    }

    pmu_lock(pmu_users_list_mutex);
    list_head_init(&puser->users_list);
    puser->punit = punit;
    puser->user_name = strdup(user_name);
    list_add_tail(&puser->users_list, &pmu_users_list);
    pmu_unlock(pmu_users_list_mutex);

out:
    return puser;
}

int pmu_put(struct pmu_user *puser)
{
    struct pmu_user *user_op, *user_temp;
    int ret = 0;

    if (puser == NULL) {
        return AIC_ENOSPC;
    }

    pmu_lock(pmu_users_list_mutex);
    list_for_each_entry_safe(user_op, user_temp, &pmu_users_list,
                            users_list, struct pmu_user) {
        if (!strcmp(user_op->user_name, puser->user_name)) {
            list_del(&user_op->users_list);
            ret = aos_free(user_op->user_name);
            if (ret)
                aicx_warn(0x11807c, "s", "%s free user name fail!\n", __func__);

            ret |= aos_free(user_op);
            if (ret)
                aicx_warn(0x085bf7, "s", "%s free user fail!\n", __func__);

            break;
        }
    }
    pmu_unlock(pmu_users_list_mutex);

    return ret;
}

int pmu_voltage_set(struct pmu_user *puser, u32 voltage_uV, u32 tolerance_uV)
{
    struct pmu_unit *punit;
    int ret = 0;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;


    if (punit->pinfo->pops->set_voltage == NULL) {
        return AIC_EIO;
    }

    pmu_lock(punit->unit_mutex);
    ret = punit->pinfo->pops->set_voltage(punit, voltage_uV, tolerance_uV);
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_voltage_get(struct pmu_user *puser, u32 *voltage_uV)
{
    struct pmu_unit *punit;
    int ret = 0;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    if (!punit->pinfo->pops->get_voltage) {
        return AIC_EIO;
    }

    pmu_lock(punit->unit_mutex);
    ret = punit->pinfo->pops->get_voltage(punit, voltage_uV);
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_current_set(struct pmu_user *puser, u32 current_uA, u32 tolerance_uA)
{
    struct pmu_unit *punit;
    int ret = 0;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    if (punit->pinfo->pops->set_current == NULL) {
        return AIC_EIO;
    }

    pmu_lock(punit->unit_mutex);
    ret = punit->pinfo->pops->set_current(punit, current_uA, tolerance_uA);
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_current_get(struct pmu_user *puser, u32 *current_uA)
{
    struct pmu_unit *punit;
    int ret = 0;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    if (!punit->pinfo->pops->get_current) {
        return AIC_EIO;
    }

    pmu_lock(punit->unit_mutex);
    ret = punit->pinfo->pops->get_current(punit, current_uA);
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_is_enabled(struct pmu_user *puser)
{
    struct pmu_unit *punit;
    int ret = 0;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    pmu_lock(punit->unit_mutex);
    ret = punit->pinfo->pops->is_enabled(punit);
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_enable(struct pmu_user *puser)
{
    struct pmu_unit *punit;
    int ret;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    pmu_lock(punit->unit_mutex);

    /*ensure parent unit alread enabled.*/
    if (punit->enable_depth == 0 && punit->parent_unit) {
        ret = pmu_enable(punit->parent_unit);
        if (ret)
            goto fail;
    }

    if (punit->enable_depth == 0) {
        ret = punit->pinfo->pops->is_enabled(punit);;
        if (ret == 0) {
            ret = punit->pinfo->pops->enable(punit);
            if (ret)
                goto fail;
        } else if (ret < 0) {
            aicx_warn(0x088be1, "", "pmu enable fail!\n", __func__);
            goto fail;
        }
    }

    punit->enable_depth++;
    pmu_unlock(punit->unit_mutex);

    return AIC_SUCCESS;

fail:
    if (punit->enable_depth == 0 && punit->parent_unit)
        pmu_disable(punit->parent_unit);

    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_reset(struct pmu_user *puser)
{
    struct pmu_unit *punit;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    pmu_lock(punit->unit_mutex);
    punit->pinfo->pops->disable(punit);
    aic_mdelay(10);
    punit->pinfo->pops->enable(punit);
    pmu_unlock(punit->unit_mutex);

    return 0;
}

int pmu_disable(struct pmu_user *puser)
{
    struct pmu_unit *punit;
    int ret = AIC_SUCCESS;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    pmu_lock(punit->unit_mutex);

    if (punit->enable_depth > 1) {
        punit->enable_depth--;
    } else if (punit->enable_depth == 1) {

        if (punit->pinfo->always_on) {
            aicx_warn(0x123d31, "ss", "%s pmu %s always on, can't disable!\n",
                        __func__, punit->unit_name);
            goto out;
        }

        ret = punit->pinfo->pops->disable(punit);
        if (ret) {
            aicx_warn(0x1665f9, "ss", "%s fail, %s phy disable fail.\n",
                        __func__, punit->unit_name);
            goto out;
        }

        punit->enable_depth = 0;
        if (punit->parent_unit) {
            ret = pmu_disable(punit->parent_unit);
            if (ret)
                aicx_warn(0x112e6b, "sss", "%s, %s parent %s pmu disable fail!\n",
                         __func__, punit->unit_name, punit->parent_unit->punit->unit_name);
        }
    } else {
        aicx_warn(0x016b81, "ss", "%s fail, unbalance disable, pmu unit %s already disable.\n",
                    __func__, punit->unit_name);
        ret = AIC_ESRCH;
    }

out:
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_force_disable(struct pmu_user *puser)
{
    struct pmu_unit *punit;
    int ret;

    if (puser == NULL) {
        aicx_warn(0x019801, "s", "%s operate handler is NULL\n", __func__);
        return AIC_EIO;
    }

    punit = puser->punit;

    pmu_lock(punit->unit_mutex);

    if (punit->pinfo->always_on) {
        aicx_warn(0x123d31, "ss", "%s pmu %s always on, can't disable!\n",
                    __func__, punit->unit_name);
        ret = AIC_EIO;
        goto out;
    }

    ret = punit->pinfo->pops->disable(punit);
    if (ret) {
        aicx_warn(0x1665f9, "ss", "%s fail, %s phy disable fail.\n",
                    __func__, punit->unit_name);
        goto out;
    }

    punit->enable_depth = 0;

    if (punit->parent_unit) {
        ret = pmu_disable(punit->parent_unit);
        if (ret)
            aicx_warn(0x112e6b, "sss", "%s, %s parent %s pmu disable fail!\n",
                     __func__, punit->unit_name, punit->parent_unit->punit->unit_name);
    }

out:
    pmu_unlock(punit->unit_mutex);

    return ret;
}

int pmu_register(struct pmu_info* pinfo, struct pmu_io *pio)
{
    struct pmu_unit *unit_cur, *unit_node;
    struct pmu_user *unit_parent = NULL;
    int ret = AIC_SUCCESS;
    char *name;

    /*pmu register unit legal judgement start*/
    if (!pinfo || !pinfo->pops) {
        ret = AIC_ENODEV;
        goto out;
    }

    if (!pinfo->unit_name) {
        ret = AIC_EAGAIN;
        goto out;
    }

    if (!pinfo->pops->enable || !pinfo->pops->disable ||
        !pinfo->pops->is_enabled) {
        ret = AIC_EAGAIN;
        goto out;
    }
    /* pmu register unit legal judgement end */

    /* register pmu node into linklist*/
    unit_cur = aos_zalloc(sizeof(struct pmu_unit));
    if (!unit_cur) {
        ret = AIC_ENOSPC;
        aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
        goto out;
    }

    list_head_init(&unit_cur->unit_list);
    list_head_init(&unit_cur->child_list_head);

    /* pmu unit members initialize.*/
    unit_cur->unit_name = strdup(pinfo->unit_name);
    if (!unit_cur->unit_name) {
        ret = AIC_ENOSPC;
        aos_free(unit_cur);
        goto out;
    }

    unit_cur->pinfo = pinfo;
    unit_cur->pmu_io = pio;
    unit_cur->unit_mutex = aos_create_mutex(unit_cur->unit_name);
    AIC_ASSERT(unit_cur->unit_mutex);

    /*pmu parent register, if any*/
    if (strcmp(pinfo->parent_name, "VBAT") != 0) {
        unit_parent = aos_zalloc(sizeof(struct pmu_user));
        if (!unit_parent) {
            ret = AIC_ENOSPC;
            aicx_warn(0x02ad96, "s", "%s not enough space!\n", __func__);
            aos_free(unit_cur);
            aos_free(unit_cur->unit_name);
            goto out;
        }

        list_head_init(&unit_parent->users_list);
        unit_cur ->parent_unit = unit_parent;
        unit_parent->user_name = strdup(pinfo->unit_name);
        list_add_tail(&unit_parent->users_list, &pmu_users_list);
    }

    /* for each pmu unit list find parent & children node */
    list_for_each_entry(unit_node, &pmu_unit_list, unit_list, struct pmu_unit) {
        /*search register pmu children in current unit list.*/
        name = unit_node->pinfo->parent_name;
        if ( (strcmp(name, "VBAT") != 0) && !unit_node->parent_unit->punit) {
            if (strcmp(unit_cur->unit_name, name) == 0) {
                unit_node->parent_unit->punit = unit_cur;
                list_head_init(&unit_node->child);
                list_add_tail(&unit_node->child, &unit_cur->child_list_head);
            }
        }

        /*search register pmu parent in current unit list.*/
        name = unit_cur->pinfo->parent_name;
        if ((strcmp(name, "VBAT") != 0) && !unit_cur->parent_unit->punit) {
            if (strcmp(unit_node->unit_name, name) == 0) {
                unit_cur->parent_unit->punit = unit_node;
                list_head_init(&unit_cur->child);
                list_add_tail(&unit_cur->child, &unit_node->child_list_head);
            }
        }
    }

    list_add_tail(&unit_cur->unit_list, &pmu_unit_list);
out:
    return ret;
}

void pmu_unregister(struct pmu_unit* punit)
{
    if (punit == NULL)
        goto out;

    punit->enable_depth = 0;

    if (punit->parent_unit) {
        pmu_force_disable(punit->parent_unit);
        pmu_put(punit->parent_unit);
    }

    pmu_lock(pmu_unit_list_mutex);
    list_del(&punit->unit_list);
    pmu_unlock(pmu_unit_list_mutex);
out:
    return;
}

static s32 pmu_init(void)
{
    pmu_unit_list_mutex = aos_create_mutex("pmu_unit_list_mutex");
    AIC_ASSERT(pmu_unit_list_mutex);

    pmu_users_list_mutex = aos_create_mutex("pmu_users_list_mutex");
    AIC_ASSERT(pmu_users_list_mutex);

    return AIC_SUCCESS;
}
AIC_EARLY_INIT_EXPORT(pmu_init);

void pmu_show_unit_list(void)
{
    struct pmu_unit *unit_cur;

    list_for_each_entry(unit_cur, &pmu_unit_list, unit_list, struct pmu_unit) {
        aicx_warn(0x09a7b8, "ss", "%s unit_name:%s", __func__, unit_cur->unit_name);
        if (unit_cur->parent_unit && unit_cur->parent_unit->punit)
            aicx_warn(0x1493c1, "ss", " parent_user:%s parent_name:%s\n", unit_cur->parent_unit->user_name, unit_cur->parent_unit->punit->unit_name);
        else
            aicx_warn(0x00ed93, "", " \n");
    }
}

void pmu_show_user_list(void)
{
    struct pmu_user *user_cur;

    list_for_each_entry(user_cur, &pmu_users_list, users_list, struct pmu_user) {
        aicx_warn(0x16543e, "ss", "%s user_name:%s\n", __func__, user_cur->user_name);
    }
}

int pmu_init_complete(void)
{
    const struct pmu_info *pinfo;
    struct pmu_unit *punit;
    int ret;

    pmu_lock(pmu_unit_list_mutex);
    list_for_each_entry(punit, &pmu_unit_list, unit_list, struct pmu_unit) {
        pmu_lock(punit->unit_mutex);
        pinfo = punit->pinfo;

        if (pinfo->bypass)
            goto bypass;

        if (!pinfo->boot_on && !pinfo->always_on) {
        /*disable unused pmu unit*/
            if (pinfo->pops->is_enabled(punit) && !punit->enable_depth) {
                ret = pinfo->pops->disable(punit);
                if (ret)
                    goto bypass;
            }
        }

bypass:
        pmu_unlock(punit->unit_mutex);
    }
    pmu_unlock(pmu_unit_list_mutex);
    return AIC_SUCCESS;
}
AIC_COMP_L1_INIT_EXPORT(pmu_init_complete);

int pmu_voltage_unified_regulation(void)
{
    const struct pmu_info *pinfo;
    struct pmu_unit *punit;
    int ret = AIC_SUCCESS;

    pmu_lock(pmu_unit_list_mutex);
    list_for_each_entry(punit, &pmu_unit_list, unit_list, struct pmu_unit) {
        pmu_lock(punit->unit_mutex);
        pinfo = punit->pinfo;

        if (pinfo->bypass)
            goto bypass;

        if (pinfo->scaling_boot_voltage)
            ret = pinfo->pops->set_voltage(punit, pinfo->scaling_boot_voltage, pinfo->voltage_step_uV);

        /*enable boot-on or always-on units, if not enable*/
        if (pinfo->boot_on || pinfo->always_on)
            if (punit->enable_depth == 0) {
                    ret = pinfo->pops->enable(punit);
                    if (ret)
                        goto bypass;
                    punit->enable_depth++;
            }
bypass:
        pmu_unlock(punit->unit_mutex);
    }
    pmu_unlock(pmu_unit_list_mutex);
    return ret;
}
AIC_DRV_P2_INIT_EXPORT(pmu_voltage_unified_regulation);
