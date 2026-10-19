/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  bc1.2 detect user demo process.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0b8f5c
#define LOG_MODULE    AMOD_PMIC

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_osa.h>
#include <cache.h>
#include <pmic_auto_reg.h>
#include <pmic_irq.h>
#include <stdlib.h>
#include <irq_chip.h>
#include <pmic_io.h>
#include <tx_api.h>
#include <bcd.h>

void bcd_demo_no_dcd_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_no_dcd_dev2_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_ps2_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_ps2_dev2_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_sdp_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_sdp_dev2_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_cdp_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_dcp_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_unplug_dev1_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo_unplug_dev2_tsk(void *param)
{
    /*bc1.2 type user process, add by user.*/
    AOS_IDXPR(0x01cb61, "sds", "%s(%d) %s\n", __FILE__, __LINE__, __func__);
}

void bcd_demo(void)
{
    int ret;

    struct aic_bc_device *no_dcd_dev1, *no_dcd_dev2,
                         *ps2_dev1, *ps2_dev2,
                         *sdp_dev1, *sdp_dev2,
                         *unplug_dev1, *unplug_dev2,
                         *cdp_dev1, *dcp_dev1;

    no_dcd_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    no_dcd_dev1->bc_type = AIC_BC_TYPE_NO_DCD;
    strcpy(no_dcd_dev1->bcd_name, "no_dcd_dev1");
    ret = aic_bcd_register(no_dcd_dev1, bcd_demo_no_dcd_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    no_dcd_dev2 = aos_zalloc(sizeof(struct aic_bc_device));
    no_dcd_dev2->bc_type = AIC_BC_TYPE_NO_DCD;
    strcpy(no_dcd_dev2->bcd_name, "no_dcd_dev2");
    ret = aic_bcd_register(no_dcd_dev2, bcd_demo_no_dcd_dev2_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    ps2_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    ps2_dev1->bc_type = AIC_BC_TYPE_PS2;
    strcpy(ps2_dev1->bcd_name, "ps2_dev1");
    ret = aic_bcd_register(ps2_dev1, bcd_demo_ps2_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    ps2_dev2 = aos_zalloc(sizeof(struct aic_bc_device));
    ps2_dev2->bc_type = AIC_BC_TYPE_PS2;
    strcpy(ps2_dev2->bcd_name, "ps2_dev2");
    ret = aic_bcd_register(ps2_dev2, bcd_demo_ps2_dev2_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    sdp_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    sdp_dev1->bc_type = AIC_BC_TYPE_SDP;
    strcpy(sdp_dev1->bcd_name, "sdp_dev1");
    ret = aic_bcd_register(sdp_dev1, bcd_demo_sdp_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    sdp_dev2 = aos_zalloc(sizeof(struct aic_bc_device));
    sdp_dev2->bc_type = AIC_BC_TYPE_SDP;
    strcpy(sdp_dev2->bcd_name, "sdp_dev2");
    ret = aic_bcd_register(sdp_dev2, bcd_demo_sdp_dev2_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    cdp_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    cdp_dev1->bc_type = AIC_BC_TYPE_CDP;
    strcpy(cdp_dev1->bcd_name, "cdp_dev1");
    ret = aic_bcd_register(cdp_dev1, bcd_demo_cdp_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    dcp_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    dcp_dev1->bc_type = AIC_BC_TYPE_DCP;
    strcpy(dcp_dev1->bcd_name, "dcp_dev1");
    ret = aic_bcd_register(dcp_dev1, bcd_demo_dcp_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    unplug_dev1 = aos_zalloc(sizeof(struct aic_bc_device));
    unplug_dev1->bc_type = AIC_USB_UNPLUG;
    strcpy(unplug_dev1->bcd_name, "unplug_dev1");
    ret = aic_bcd_register(unplug_dev1, bcd_demo_unplug_dev1_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

    unplug_dev2 = aos_zalloc(sizeof(struct aic_bc_device));
    unplug_dev2->bc_type = AIC_USB_UNPLUG;
    strcpy(unplug_dev2->bcd_name, "unplug_dev2");
    ret = aic_bcd_register(unplug_dev2, bcd_demo_unplug_dev2_tsk, NULL);
    if(ret)
        AOS_IDXPR(0x1aef70, "sds", "%s(%d) %sfail!\n", __FILE__, __LINE__, __func__);

}

/**
 * pmic_bcd_demo() - pmic bcd demo process
 */
int pmic_bcd_demo_init(void)
{
    bcd_demo();

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_L2_INIT_EXPORT(pmic_bcd_demo_init);
