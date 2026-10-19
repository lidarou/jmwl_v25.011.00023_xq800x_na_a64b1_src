
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18a8c2
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <aic_assert.h>
#include <aic_type.h>
#include <aic_bitmap.h>
#include <aic_io.h>
#include <aic_size.h>
#include <aic_math.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_cmd_api.h>
#include <pmu_user.h>
#include <timer_inc.h>
#include <mha.h>
#include <audio/snd_config.h>

#include "snd_card.h"
#include "snd_ext_pa.h"

struct ncc_ctx {
    struct pmu_user *power;
    struct pmu_user *io_power;
};

static struct ncc_ctx g_ncc_ctx;

static int ncc_event(struct snd_dpm_context *dpm, int event)
{
    struct ncc_ctx *pctx = &g_ncc_ctx;
    int ret;

    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        pmu_enable(pctx->power);
        pmu_enable(pctx->io_power);

        ret = pmu_voltage_set(pctx->power, 3300000, 10000);
        snd_ext_pa_on();
        SNX_INFO(0x13794b, "d", "ncc on, ret=%d\n", ret);
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        snd_ext_pa_off();
        pmu_disable(pctx->io_power);
        pmu_disable(pctx->power);
        SNX_INFO(0x1c9c40, "", "ncc off\n");
        break;
    default:
        break;
    }

    return 0;
}

static struct snd_codec_driver ncc_driver = {
    .stream_event = ncc_event,
};

static int ncc_init(void)
{
    struct ncc_ctx *pctx = &g_ncc_ctx;
    int ret = -1;

    if (SND_PMU_CODEC)
        pctx->power = pmu_get("ncc power", SND_PMU_CODEC);
    if (SND_PMU_CODEC_IO)
        pctx->io_power = pmu_get("ncc io power", SND_PMU_CODEC_IO);

    ret = snd_card_register_codec("ncc", &ncc_driver);
    SNX_INFO(0x068404, "s", "ncc codec regist %s\n", ret ? "failed" : "OK");

    return ret;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(ncc_init);

