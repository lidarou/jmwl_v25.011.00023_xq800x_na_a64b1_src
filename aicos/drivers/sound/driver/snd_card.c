/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defined sound card interface, codec driver
 *    can regist to this sound card during boot time.
 *    Normaly it's used for driver module.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c3bd5
#define LOG_MODULE    AMOD_SOUND

#include <aic_init.h>

#include "snd_card.h"
#include "snd_dpm_driver.h"

static struct snd_card aic_snd_card;

struct snd_card *snd_card_get(void)
{
    return &aic_snd_card;
}

int snd_card_add_controls(const struct snd_kcontrol_new *controls,
                          int num_controls, const char *prefix, void *data)
{
    struct snd_kcontrol *ctl;
    int i;

    for (i = 0; i < num_controls; i++) {
        const struct snd_kcontrol_new *control = &controls[i];
        ctl = snd_ctl_cnew(control, data, (char *)control->name, prefix);
        if (!ctl) {
            aicx_error(0x11ba60, "s", "snd_machine: control %s create failed\n", control->name);
            continue;
        }
        snd_ctl_add(ctl);
    }

    return 0;
}

int snd_card_add_widgets_routes(const struct snd_dpm_widget *widget, int wnum,
                               const struct snd_dpm_route *route, int rnum)
{
    struct snd_card *card = snd_card_get();
    int ret = 0;

    if (widget)
        ret = snd_dpm_new_controls(&card->dpm, widget, wnum);
    if (ret) {
        SNX_ERROR(0x0d7228, "d", "snd card add widgets error, ret=%d\n", ret);
        return ret;
    }
    if (route)
        ret = snd_dpm_add_routes(&card->dpm, route, rnum);

    if(ret) {
        SNX_ERROR(0x1adce9, "d", "snd card add routes error, ret=%d\n", ret);
        return ret;
    }

    ret = snd_dpm_new_widgets(&card->dpm);
    if(ret) {
        SNX_ERROR(0x1c9103, "d", "snd card new widgets error, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static int snd_card_init_codec(struct snd_card *card, struct snd_codec *codec)
{
    const struct snd_codec_driver *driver = codec->driver;
    int ret = 0;

    codec->card = card;
    codec->dpm.card = card;

    if (driver->dpm_widgets)
        snd_dpm_new_controls(&codec->dpm, driver->dpm_widgets,
                      driver->num_dpm_widgets);

    codec->dpm.idle_bias_off = driver->idle_bias_off;

    if (driver->probe) {
        ret = driver->probe(codec);
        if (ret < 0) {
            aicx_error(0x0b9f64, "sd", "asoc: failed to probe CODEC %s: %d\n",
                codec->name, ret);
            return ret;
        }
    }

    /** TODO: regarding to open if need */
    if (driver->controls)
        snd_card_add_controls(driver->controls, driver->num_controls, NULL,
                              codec);
    /* */
    if (driver->dpm_routes)
        snd_dpm_add_routes(&codec->dpm, driver->dpm_routes,
                    driver->num_dpm_routes);

    /* add to list at first */
    list_add(&codec->dpm.list, &card->dpm_list);

    /* then can create widgets */
    snd_dpm_new_widgets(&codec->dpm);

    return 0;
}

int snd_card_stream_event(char *stream, int event)
{
    struct snd_card *card = snd_card_get();

    return snd_dpm_stream_event(card->primary_codec, stream, event);
}

int snd_card_set_param(char *stream, u32 wid, u32 chn, u32 srate)
{
    struct snd_card *card = snd_card_get();
    struct snd_codec *codec = card->primary_codec;

    if (codec && codec->driver && codec->driver->set_params)
        return codec->driver->set_params(&codec->dpm, wid, chn, srate);

    return 0;
}

int snd_card_enable_pin(char *name)
{
    return snd_dpm_enable_pin_sync(name);
}

static int snd_card_register_codec_internal(char *name,
            const struct snd_codec_driver *codec_drv, BOOL is_primary)
{
    struct snd_card *card = snd_card_get();
    struct snd_codec *codec;

    if(!name) {
        aicx_error(0x120c5f, "", "codec name is null\n");
        return AIC_EINVAL;
    }
    aicx_debug(0x08acc9, "s", "codec register %s\n", name);

    codec = snd_zalloc(sizeof(struct snd_codec));
    if (codec == NULL)
        return AIC_ENOMEM;

    /**
     * Should create CODEC component name,
     * but we don't malloc memory to save it now, only save name pointer.
     */
    codec->name = name;

    codec->driver = codec_drv;
    codec->write = codec_drv->write;
    codec->read = codec_drv->read;
    codec->dpm.bias_level = SND_BIAS_OFF;

    codec->dpm.codec = codec;
    codec->dpm.seq_notifier = codec_drv->seq_notifier;
    codec->dpm.stream_event = codec_drv->stream_event;
    codec->card = card;
    mutex_init(codec->mutex);
    INIT_LIST_HEAD(&codec->list);
    list_add(&codec->list, &card->codecs);
    snd_card_init_codec(card, codec);
    if (is_primary)
        card->primary_codec = codec;
    /* TODO */
    //snd_dpm_sync(&card->dpm);

    aicx_info(0x128398, "s", "Registered codec '%s'\n", codec->name);

    return 0;
}

int snd_card_register_aux_codec(char *name,
                                const struct snd_codec_driver *codec_drv)
{
    return snd_card_register_codec_internal(name, codec_drv, FALSE);
}


int snd_card_register_codec(char *name,
                            const struct snd_codec_driver *codec_drv)
{
    return snd_card_register_codec_internal(name, codec_drv, TRUE);
}

int snd_card_set_codec_data_by_name(char *name, u32 private_data)
{
    struct snd_codec *codec;
    BOOL found = FALSE;

    list_for_each_entry(codec, &aic_snd_card.codecs, list, struct snd_codec) {
        if (strcmp(codec->name, name) == 0) {
            found = TRUE;
            break;
        }
    }
    if (found)
        codec->private = private_data;

    return 0;
}

int snd_card_set_codec_data(struct snd_codec *codec, u32 private_data)
{
    if (codec)
        codec->private = private_data;

    return 0;
}

u32 snd_card_get_codec_data(struct snd_codec *codec)
{
    return codec->private;
}

static int snd_card_init(void)
{
    struct snd_card *card = snd_card_get();

    INIT_LIST_HEAD(&card->list);
    INIT_LIST_HEAD(&card->widgets);
    INIT_LIST_HEAD(&card->paths);
    INIT_LIST_HEAD(&card->dpm_list);
    INIT_LIST_HEAD(&card->dpm_dirty);
    INIT_LIST_HEAD(&card->ctls);
    INIT_LIST_HEAD(&card->codecs);

    card->last_numid = 0;
    card->dpm.bias_level = SND_BIAS_OFF;
    card->dpm.card = card;

    /* add card dpm to dpm_list */
    list_add(&card->dpm.list, &card->dpm_list);

    return 0;
}
AIC_NORMAL_EARLY_INIT_EXPORT(snd_card_init);

