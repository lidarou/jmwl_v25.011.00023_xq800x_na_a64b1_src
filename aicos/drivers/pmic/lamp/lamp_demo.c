/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *  lamp operate interface demo.
 *
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x03ead5
#define LOG_MODULE    AMOD_PMIC

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <lamp.h>
#include <timer_inc.h>

void lamp_demo_task(void *input)
{
    int ret;
    u32 lamp_current_uA;

    while(1) {
        lamp_current_uA = 0;
        ret = lamp_enabled(true);
        if (ret)
            aicx_warn(0x1382a1, "", "lamp operate fail!\n");
        ret = lamp_is_enabled();
        if (ret)
            aicx_warn(0x0c90be, "", "lamp open\n");
        else
            aicx_warn(0x16b691, "", "lamp colse\n");
        aos_sleep(1000);
        ret = lamp_enabled(false);
        if (ret)
            aicx_warn(0x1382a1, "", "lamp operate fail!\n");

        ret = lamp_set_current(60000);
        if (ret)
            aicx_warn(0x1382a1, "", "lamp operate fail!\n");

        ret = lamp_get_current(&lamp_current_uA);
        if (ret)
            aicx_warn(0x1382a1, "", "lamp operate fail!\n");
        else
            aicx_warn(0x13dce4, "d", "lamp total current:%d uA!\n", lamp_current_uA);

        ret = lamp_is_enabled();
        if (ret)
            aicx_warn(0x0c90be, "", "lamp open\n");
        else
            aicx_warn(0x16b691, "", "lamp colse\n");

        aos_sleep(1000);

    }
}

static int lamp_demo_init(void)
{
    aos_create_thread("lamp_demo", lamp_demo_task, AOS_NULL, 1024, 8,
                      "lamp_demo", 16, AOS_AUTO_START);
    return 0;
}
AIC_NORMAL_APP_L1_INIT_EXPORT(lamp_demo_init);
