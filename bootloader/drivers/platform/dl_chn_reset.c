/*****************************************************************************/
/*                                                                           */
/*         Copyright 2021 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

#if defined(CONFIG_MDL) || defined(CONFIG_BL)
#include <aic_errno.h>
#include <aic_io.h>
#include <dl_ssp_chn.h>
#include <initcall.h>
#include <ssp/ssp_config.h>
#include <timer.h>

static int dl_chn_ssp_reset(void)
{
    struct ssp_ctrl *ctrl = ssp_get_ctrl(DL_SSP_CHANNEL);

    /* reset controller */
    AIC_WRITE32(ctrl->reset_reg, 0x1);
    aic_udelay(100);
    AIC_WRITE32(ctrl->reset_reg, 0x0);

    /* disable controller */
    AIC_WRITE32(ctrl->enable_reg, FALSE);

    return 0;
}
early_init(dl_chn_ssp_reset);
#endif
