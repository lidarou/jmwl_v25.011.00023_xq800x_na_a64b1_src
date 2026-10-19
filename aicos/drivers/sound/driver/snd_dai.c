

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0d650b
#define LOG_MODULE    AMOD_SOUND
#include <aic_init.h>
#include <aic_errno.h>
#include <alog_api.h>
#include <aic_list.h>

#include "snd_dai.h"

struct dai_mng {
    struct list_head list;
};

struct dai_mng g_snd_dai;

int snd_dai_register(struct snd_dai *dai)
{
    list_add(&dai->list, &g_snd_dai.list);

    return 0;
}

struct snd_dai_cfg *snd_dai_get_cfg(struct snd_dai *dai)
{
    return (struct snd_dai_cfg *)dai->cfg;
}

/* type: reserve now */
struct snd_dai *snd_dai_get(char *stream_name, u32 type, u32 reserve)
{
    struct snd_dai *dai;

    if (!stream_name) {
        aicx_error(0x198dd4, "", "get dai error, NULL name\n");
        return 0;
    }

    list_for_each_entry(dai, &g_snd_dai.list, list, struct snd_dai) {
        if (strcmp(stream_name, dai->stream_name) == 0) {
            aicx_debug(0x0f8e13, "s", "snd dai getten, name=%s\n", dai->name);
            return dai;
        }
    }
    aicx_error(0x18ddce, "s", "snd dai not found, stream=%s\n", stream_name);

    return NULL;
}

int snd_dai_init(void)
{
    list_head_init(&g_snd_dai.list);
    return 0;
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(snd_dai_init);

