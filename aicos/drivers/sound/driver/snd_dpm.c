/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic interface about DPM for sound device driver,
 *    we use it to manage hardware power up/down dynamically with sequence
 *    during sound play, usally the hardware is audio codec.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x048beb
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include "snd_dpm_driver.h"
#include "snd_ctl.h"
#include "aic_cmd_api.h"

#ifdef WIN32
static int dpm_up_seq[snd_dpm_max] = {
    0,1,2,3,4,5,6,7,8,9,910,11,12,13,14,15,
};

static int dpm_down_seq[snd_dpm_max] = {
    0,1,2,3,4,5,6,7,8,9,910,11,12,13,14,15,
};
#else
/* dpm power sequences - make this per codec in the future */
static int dpm_up_seq[] = {
    [snd_dpm_pre] = 0,
    [snd_dpm_supply] = 1,
    [snd_dpm_micbias] = 2,
    [snd_dpm_aif_in] = 3,
    [snd_dpm_aif_out] = 3,
    [snd_dpm_mic] = 4,
    [snd_dpm_mux] = 5,
    [snd_dpm_virt_mux] = 5,
    [snd_dpm_value_mux] = 5,
    [snd_dpm_dac] = 6,
    [snd_dpm_mixer] = 7,
    [snd_dpm_mixer_named_ctl] = 7,
    [snd_dpm_pga] = 8,
    [snd_dpm_adc] = 9,
    [snd_dpm_out_drv] = 10,
    [snd_dpm_hp] = 10,
    [snd_dpm_spk] = 10,
    [snd_dpm_post] = 11,
};

static int dpm_down_seq[] = {
    [snd_dpm_pre] = 0,
    [snd_dpm_adc] = 1,
    [snd_dpm_hp] = 2,
    [snd_dpm_spk] = 2,
    [snd_dpm_out_drv] = 2,
    [snd_dpm_pga] = 4,
    [snd_dpm_mixer_named_ctl] = 5,
    [snd_dpm_mixer] = 5,
    [snd_dpm_dac] = 6,
    [snd_dpm_mic] = 7,
    [snd_dpm_micbias] = 8,
    [snd_dpm_mux] = 9,
    [snd_dpm_virt_mux] = 9,
    [snd_dpm_value_mux] = 9,
    [snd_dpm_aif_in] = 10,
    [snd_dpm_aif_out] = 10,
    [snd_dpm_supply] = 11,
    [snd_dpm_post] = 12,
};
#endif
unsigned int dpm_reg_read(struct snd_codec *codec, unsigned int reg)
{
    unsigned int ret;

    ret = codec->read(codec, reg);
    aicx_debug(0x09a559, "xx", "read %x => %x\n", reg, ret);

    return ret;
}

unsigned int dpm_reg_write(struct snd_codec *codec,
               unsigned int reg, unsigned int val)
{
    aicx_debug(0x1c0a8f, "xx", "write %x = %x\n", reg, val);
    return codec->write(codec, reg, val);
}

int dpm_reg_test_bits(struct snd_codec *codec, unsigned short reg,
                unsigned int mask, unsigned int value)
{
    int change;
    unsigned int old, new;

    old = dpm_reg_read(codec, reg);
    new = (old & ~mask) | value;
    change = old != new;

    return change;
}

int dpm_reg_update_bits(struct snd_codec *codec, unsigned short reg,
                        unsigned int mask, unsigned int value)
{
    int change;
    unsigned int old, new;
    int ret;

    ret = dpm_reg_read(codec, reg);
    if (ret < 0)
        return ret;

    old = ret;
    new = (old & ~mask) | (value & mask);
    change = old != new;
    if (change) {
        ret = dpm_reg_write(codec, reg, new);
        if (ret < 0)
            return ret;
    }

    return change;
}

static bool dpm_dirty_widget(struct snd_dpm_widget *w)
{
    return !list_empty(&w->dirty);
}

void dpm_mark_dirty(struct snd_dpm_widget *w, const char *reason)
{
    if (!dpm_dirty_widget(w)) {
        aicx_debug(0x18b2c0, "ss", "Marking %s dirty due to %s\n",
                   w->name, reason);
        list_add_tail(&w->dirty, &w->dpm->card->dpm_dirty);
    }
}

/* create a new dpm widget */
static inline struct snd_dpm_widget *dpm_cnew_widget(
    const struct snd_dpm_widget *_widget)
{
    return aos_memdup(_widget, sizeof(*_widget));
}


static int dpm_widget_read(struct snd_dpm_widget *w, int reg)
{
    if (w->codec)
        return dpm_reg_read(w->codec, reg);

    aicx_error(0x1be077, "", "no valid widget read method\n");

    return -1;
}

static int dpm_widget_write(struct snd_dpm_widget *w, int reg, int val)
{
    if (w->codec)
        return dpm_reg_write(w->codec, reg, val);

    aicx_error(0x1ad7e1, "", "no valid widget write method\n");

    return -1;
}

static int dpm_widget_update_bits(struct snd_dpm_widget *w,
    unsigned short reg, unsigned int mask, unsigned int value)
{
    int change;
    unsigned int old, new;
    int ret;

    ret = dpm_widget_read(w, reg);
    if (ret < 0)
        return ret;

    old = ret;
    new = (old & ~mask) | (value & mask);
    change = old != new;
    if (change) {
        ret = dpm_widget_write(w, reg, new);
        if (ret < 0)
            return ret;
    }

    return change;
}

/**
 * snd_dpm_set_bias_level - set the bias level for the system
 * @dpm: DPM context
 * @level: level to configure
 *
 * Configure the bias (power) levels for the SoC audio device.
 *
 * Returns 0 for success else error.
 */
static int snd_dpm_set_bias_level(struct snd_dpm_context *dpm,
                                  enum snd_bias_level level)
{
    int ret = 0;

    //if (card && card->set_bias_level)
    //    ret = card->set_bias_level(card, dpm, level);
    //if (ret != 0)
    //    goto out;

    if (dpm->codec) {
        if (dpm->codec->driver->set_bias_level)
            ret = dpm->codec->driver->set_bias_level(dpm->codec,
                                  level);
    }
    if (ret != 0)
        return ret;

    dpm->bias_level = level;

    //if (card && card->set_bias_level_post)
    //    ret = card->set_bias_level_post(card, dpm, level);

    return ret;
}

/* set up initial codec paths */
static void dpm_set_path_status(struct snd_dpm_widget *w,
                                 struct snd_dpm_path *p, int i)
{
    switch (w->id) {
    case snd_dpm_switch:
    case snd_dpm_mixer:
    case snd_dpm_mixer_named_ctl: {
        int val;
        struct soc_mixer_control *mc = (struct soc_mixer_control *)
            w->kcontrol_news[i].private_value;
        unsigned int reg = mc->reg;
        unsigned int shift = mc->shift;
        int max = mc->max;
        unsigned int mask = (1 << fls(max)) - 1;
        unsigned int invert = mc->invert;

        val = dpm_widget_read(w, reg);
        val = (val >> shift) & mask;

        if ((invert && !val) || (!invert && val))
            p->connect = 1;
        else
            p->connect = 0;
    }
    break;
    case snd_dpm_mux: {
        struct soc_enum *e = (struct soc_enum *)
            w->kcontrol_news[i].private_value;
        int val, item, bitmask;

        for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
            ;
        val = dpm_widget_read(w, e->reg);
        item = (val >> e->shift_l) & (bitmask - 1);

        p->connect = 0;
        for (i = 0; i < e->max; i++) {
            if (!(strcmp(p->name, e->texts[i])) && item == i)
                p->connect = 1;
        }
    }
    break;
    case snd_dpm_virt_mux: {
        struct soc_enum *e = (struct soc_enum *)
            w->kcontrol_news[i].private_value;

        p->connect = 0;
        /* since a virtual mux has no backing registers to
         * decide which path to connect, it will try to match
         * with the first enumeration.  This is to ensure
         * that the default mux choice (the first) will be
         * correctly powered up during initialization.
         */
        if (!strcmp(p->name, e->texts[0]))
            p->connect = 1;
    }
    break;
    case snd_dpm_value_mux: {
        struct soc_enum *e = (struct soc_enum *)
            w->kcontrol_news[i].private_value;
        int val, item;

        val = dpm_widget_read(w, e->reg);
        val = (val >> e->shift_l) & e->mask;
        for (item = 0; item < e->max; item++) {
            if (val == e->values[item])
                break;
        }

        p->connect = 0;
        for (i = 0; i < e->max; i++) {
            if (!(strcmp(p->name, e->texts[i])) && item == i)
                p->connect = 1;
        }
    }
    break;
    /* does not affect routing - always connected */
    case snd_dpm_pga:
    case snd_dpm_out_drv:
    case snd_dpm_output:
    case snd_dpm_adc:
    case snd_dpm_input:
    case snd_dpm_siggen:
    case snd_dpm_dac:
    case snd_dpm_micbias:
    case snd_dpm_vmid:
    case snd_dpm_supply:
    case snd_dpm_aif_in:
    case snd_dpm_aif_out:
    case snd_dpm_hp:
    case snd_dpm_mic:
    case snd_dpm_spk:
    case snd_dpm_line:
        p->connect = 1;
    break;
    /* does affect routing - dynamically connected */
    case snd_dpm_pre:
    case snd_dpm_post:
        p->connect = 0;
    break;
    default:
        break;
    }
}

/* connect mux widget to its interconnecting audio paths */
static int dpm_connect_mux(struct snd_dpm_context *dpm,
    struct snd_dpm_widget *src, struct snd_dpm_widget *dest,
    struct snd_dpm_path *path, const char *control_name,
    const struct snd_kcontrol_new *kcontrol)
{
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    int i;

    for (i = 0; i < e->max; i++) {
        if (!(strcmp(control_name, e->texts[i]))) {
            list_add(&path->list, &dpm->card->paths);
            list_add(&path->list_sink, &dest->sources);
            list_add(&path->list_source, &src->sinks);
            path->name = (char*)e->texts[i];
            dpm_set_path_status(dest, path, 0);
            return 0;
        }
    }

    return AIC_ENODEV;
}

/* connect mixer widget to its interconnecting audio paths */
static int dpm_connect_mixer(struct snd_dpm_context *dpm,
    struct snd_dpm_widget *src, struct snd_dpm_widget *dest,
    struct snd_dpm_path *path, const char *control_name)
{
    int i;

    /* search for mixer kcontrol */
    for (i = 0; i < dest->num_kcontrols; i++) {
        if (!strcmp(control_name, (char *)dest->kcontrol_news[i].name)) {
            list_add(&path->list, &dpm->card->paths);
            list_add(&path->list_sink, &dest->sources);
            list_add(&path->list_source, &src->sinks);
            path->name = (char *)dest->kcontrol_news[i].name;
            dpm_set_path_status(dest, path, i);
            return 0;
        }
    }
    return AIC_ENODEV;
}

static int dpm_is_shared_kcontrol(struct snd_dpm_context *dpm,
    struct snd_dpm_widget *kcontrolw,
    const struct snd_kcontrol_new *kcontrol_new,
    struct snd_kcontrol **kcontrol)
{
    struct snd_dpm_widget *w;
    int i;

    *kcontrol = NULL;

    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget) {
        if (w == kcontrolw || w->dpm != kcontrolw->dpm)
            continue;
        for (i = 0; i < w->num_kcontrols; i++) {
            if (&w->kcontrol_news[i] == kcontrol_new) {
                if (w->kcontrols)
                    *kcontrol = w->kcontrols[i];
                return 1;
            }
        }
    }

    return 0;
}

/* create new dpm mixer control */
static int dpm_new_mixer(struct snd_dpm_widget *w)
{
    /* struct snd_dpm_context *dpm = w->dpm; */
    int i, ret = 0;
    size_t name_len, prefix_len;
    struct snd_dpm_path *path;
    const char *prefix;
    struct snd_dpm_widget_list *wlist;
    size_t wlistsize;


    prefix = NULL;

    if (prefix)
        prefix_len = strlen(prefix) + 1;
    else
        prefix_len = 0;

    /* add kcontrol */
    for (i = 0; i < w->num_kcontrols; i++) {

        /* match name */
        list_for_each_entry(path, &w->sources, list_sink, struct snd_dpm_path) {

            /* mixer/mux paths name must match control name */
            if (path->name != (char *)w->kcontrol_news[i].name)
                continue;

            if (w->kcontrols[i]) {
                path->kcontrol = w->kcontrols[i];
                continue;
            }

            wlistsize = sizeof(struct snd_dpm_widget_list);
            wlist = snd_zalloc(wlistsize);
            if (wlist == NULL) {
                aicx_error(0x12fcb9, "s", "asoc: can't allocate widget list for %s\n",
                           w->name);
                return AIC_ENOMEM;
            }
            wlist->num_widgets = 1;
            wlist->widgets[0] = w;

            /* add dpm control with long name.
             * for dpm_mixer this is the concatenation of the
             * mixer and kcontrol name.
             * for dpm_mixer_named_ctl this is simply the
             * kcontrol name.
             */
            name_len = strlen((char *)w->kcontrol_news[i].name) + 1;
            if (w->id != snd_dpm_mixer_named_ctl)
                name_len += 1 + strlen(w->name);

            path->long_name = snd_malloc(name_len);

            if (path->long_name == NULL) {
                snd_free(wlist);
                return AIC_ENOMEM;
            }

            switch (w->id) {
            default:
                /* The control will get a prefix from
                 * the control creation process but
                 * we're also using the same prefix
                 * for widgets so cut the prefix off
                 * the front of the widget name.
                 */
                snprintf(path->long_name, name_len, "%s %s",
                     w->name + prefix_len,
                     w->kcontrol_news[i].name);
                break;
            case snd_dpm_mixer_named_ctl:
                snprintf(path->long_name, name_len, "%s",
                     w->kcontrol_news[i].name);
                break;
            }

            path->long_name[name_len - 1] = '\0';

            path->kcontrol = snd_ctl_cnew(&w->kcontrol_news[i],
                              wlist, path->long_name,
                              prefix);
            ret = snd_ctl_add(path->kcontrol);
            if (ret < 0) {
                aicx_error(0x1ce339, "sd", "asoc: failed to add dpm kcontrol %s: %d\n",
                           path->long_name, ret);
                snd_free(wlist);
                snd_free(path->long_name);
                path->long_name = NULL;
                return ret;
            }
            w->kcontrols[i] = path->kcontrol;
        }
    }
    return ret;
}

/* create new dpm mux control */
static int dpm_new_mux(struct snd_dpm_widget *w)
{
    struct snd_dpm_context *dpm = w->dpm;
    struct snd_dpm_path *path = NULL;
    struct snd_kcontrol *kcontrol;
    const char *prefix;
    size_t prefix_len;
    int ret;
    struct snd_dpm_widget_list *wlist;
    int shared, wlistentries;
    size_t wlistsize;
    char *name;

    if (w->num_kcontrols != 1) {
        aicx_error(0x174de6, "s", "asoc: mux %s has incorrect number of controls\n",
                   w->name);
        return AIC_EINVAL;
    }

    shared = dpm_is_shared_kcontrol(dpm, w, &w->kcontrol_news[0],
                     &kcontrol);
    if (kcontrol) {
        wlist = kcontrol->private_data;
        wlistentries = wlist->num_widgets + 1;
    } else {
        wlist = NULL;
        wlistentries = 1;
    }
    wlistsize = sizeof(struct snd_dpm_widget_list);

    if (!wlist)
        wlist = snd_zalloc(wlistsize);
    if (wlist == NULL) {
        aicx_error(0x12fcb9, "s", "asoc: can't allocate widget list for %s\n", w->name);
        return AIC_ENOMEM;
    }
    wlist->num_widgets = wlistentries;
    wlist->widgets[wlistentries - 1] = w;

    if (!kcontrol) {
            prefix = NULL;

        if (shared) {
            name = (char *)w->kcontrol_news[0].name;
            prefix_len = 0;
        } else {
            name = w->name;
            if (prefix)
                prefix_len = strlen(prefix) + 1;
            else
                prefix_len = 0;
        }

        /*
         * The control will get a prefix from the control creation
         * process but we're also using the same prefix for widgets so
         * cut the prefix off the front of the widget name.
         */
        kcontrol = snd_ctl_cnew(&w->kcontrol_news[0], wlist,
                    name + prefix_len, prefix);
        ret = snd_ctl_add(kcontrol);
        if (ret < 0) {
            aicx_error(0x06d672, "sd", "failed to add kcontrol %s: %d\n", w->name, ret);
            snd_free(wlist);
            return ret;
        }
    }

    kcontrol->private_data = wlist;

    w->kcontrols[0] = kcontrol;

    list_for_each_entry(path, &w->sources, list_sink, struct snd_dpm_path)
        path->kcontrol = kcontrol;

    return 0;
}

/* create new dpm volume control */
static int dpm_new_pga(struct snd_dpm_widget *w)
{
    if (w->num_kcontrols)
        aicx_error(0x19d36b, "s", "asoc: PGA controls not supported: '%s'\n", w->name);

    return 0;
}

/* reset 'walked' bit for each dpm path */
static inline void dpm_clear_walk(struct snd_dpm_context *dpm)
{
    struct snd_dpm_path *p;

    list_for_each_entry(p, &dpm->card->paths, list, struct snd_dpm_path)
        p->walked = 0;
}

/* We implement power down on suspend by checking the power state of
 * the ALSA card - when we are suspending the ALSA state for the card
 * is set to D3.
 */
static int snd_dpm_suspend_check(struct snd_dpm_widget *widget)
{
#ifdef DPM_SUSPEND_CHECK
    int level = snd_power_get_state(widget->dpm->card->snd_card);

    switch (level) {
    case SNDRV_CTL_POWER_D3hot:
    case SNDRV_CTL_POWER_D3cold:
        if (widget->ignore_suspend)
            dev_dbg(widget->dpm->dev, "%s ignoring suspend\n",
                widget->name);
        return widget->ignore_suspend;
    default:
        return 1;
    }
#else
    return 1;
#endif
}

/*
 * Recursively check for a completed path to an active or physically connected
 * output widget. Returns number of complete paths.
 */
static int is_connected_output_ep(struct snd_dpm_widget *widget)
{
    struct snd_dpm_path *path;
    int con = 0;

    if (widget->outputs >= 0)
        return widget->outputs;

    if (widget->id == snd_dpm_supply)
        return 0;

    switch (widget->id) {
    case snd_dpm_adc:
    case snd_dpm_aif_out:
        if (widget->active) {
            widget->outputs = snd_dpm_suspend_check(widget);
            return widget->outputs;
        }
    default:
        break;
    }

    if (widget->connected) {
        /* connected pin ? */
        if (widget->id == snd_dpm_output && !widget->ext) {
            widget->outputs = snd_dpm_suspend_check(widget);
            return widget->outputs;
        }

        /* connected jack or spk ? */
        if (widget->id == snd_dpm_hp ||
            widget->id == snd_dpm_spk ||
            (widget->id == snd_dpm_line &&
             !list_empty(&widget->sources))) {
            widget->outputs = snd_dpm_suspend_check(widget);
            return widget->outputs;
        }
    }

    list_for_each_entry(path, &widget->sinks, list_source, struct snd_dpm_path) {

        if (path->weak)
            continue;

        if (path->walked)
            continue;

        if (path->sink && path->connect) {
            path->walked = 1;
            con += is_connected_output_ep(path->sink);
        }
    }

    widget->outputs = con;

    return con;
}

/*
 * Recursively check for a completed path to an active or physically connected
 * input widget. Returns number of complete paths.
 */
static int is_connected_input_ep(struct snd_dpm_widget *widget)
{
    struct snd_dpm_path *path;
    int con = 0;

    if (widget->inputs >= 0)
        return widget->inputs;

    if (widget->id == snd_dpm_supply)
        return 0;

    /* active stream ? */
    switch (widget->id) {
    case snd_dpm_dac:
    case snd_dpm_aif_in:
        if (widget->active) {
            widget->inputs = snd_dpm_suspend_check(widget);
            return widget->inputs;
        }
    default:
        break;
    }

    if (widget->connected) {
        /* connected pin ? */
        if (widget->id == snd_dpm_input && !widget->ext) {
            widget->inputs = snd_dpm_suspend_check(widget);
            return widget->inputs;
        }

        /* connected VMID/Bias for lower pops */
        if (widget->id == snd_dpm_vmid) {
            widget->inputs = snd_dpm_suspend_check(widget);
            return widget->inputs;
        }

        /* connected jack ? */
        if (widget->id == snd_dpm_mic ||
            (widget->id == snd_dpm_line &&
             !list_empty(&widget->sinks))) {
            widget->inputs = snd_dpm_suspend_check(widget);
            return widget->inputs;
        }

        /* signal generator */
        if (widget->id == snd_dpm_siggen) {
            widget->inputs = snd_dpm_suspend_check(widget);
            return widget->inputs;
        }
    }

    list_for_each_entry(path, &widget->sources, list_sink, struct snd_dpm_path) {

        if (path->weak)
            continue;

        if (path->walked)
            continue;

        if (path->source && path->connect) {
            path->walked = 1;
            con += is_connected_input_ep(path->source);
        }
    }

    widget->inputs = con;

    return con;
}

/*
 * Handler for generic register modifier widget.
 */
int dpm_reg_event(struct snd_dpm_widget *w,
           struct snd_kcontrol *kcontrol, int event)
{
    unsigned int val;

    if (SND_DPM_EVENT_ON(event))
        val = w->on_val;
    else
        val = w->off_val;

    dpm_widget_update_bits(w, -(w->reg + 1),
                w->mask << w->shift, val << w->shift);

    return 0;
}

static int dpm_widget_power_check(struct snd_dpm_widget *w)
{
    if (w->power_checked)
        return w->new_power;

    if (w->force)
        w->new_power = 1;
    else
        w->new_power = w->power_check(w);

    w->power_checked = TRUE;

    return w->new_power;
}

/* Generic check to see if a widget should be powered.
 */
static int dpm_generic_check_power(struct snd_dpm_widget *w)
{
    int in, out;

    in = is_connected_input_ep(w);
    dpm_clear_walk(w->dpm);
    out = is_connected_output_ep(w);
    dpm_clear_walk(w->dpm);
    return out != 0 && in != 0;
}

/* Check to see if an ADC has power */
static int dpm_adc_check_power(struct snd_dpm_widget *w)
{
    int in;

    if (w->active) {
        in = is_connected_input_ep(w);
        dpm_clear_walk(w->dpm);
        return in != 0;
    } else {
        return dpm_generic_check_power(w);
    }
}

/* Check to see if a DAC has power */
static int dpm_dac_check_power(struct snd_dpm_widget *w)
{
    int out;

    if (w->active) {
        out = is_connected_output_ep(w);
        dpm_clear_walk(w->dpm);
        return out != 0;
    } else {
        return dpm_generic_check_power(w);
    }
}

/* Check to see if a power supply is needed */
static int dpm_supply_check_power(struct snd_dpm_widget *w)
{
    struct snd_dpm_path *path;

    /* Check if one of our outputs is connected */
    list_for_each_entry(path, &w->sinks, list_source, struct snd_dpm_path) {
        if (path->weak)
            continue;

        if (path->connected &&
            !path->connected(path->source, path->sink))
            continue;

        if (!path->sink)
            continue;

        if (dpm_widget_power_check(path->sink))
            return 1;
    }

    dpm_clear_walk(w->dpm);

    return 0;
}

static int dpm_always_on_check_power(struct snd_dpm_widget *w)
{
    return 1;
}

static int dpm_seq_compare(struct snd_dpm_widget *a,
                struct snd_dpm_widget *b,
                bool power_up)
{
    int *sort;

    if (power_up)
        sort = dpm_up_seq;
    else
        sort = dpm_down_seq;

    if (sort[a->id] != sort[b->id])
        return sort[a->id] - sort[b->id];
    if (a->subseq != b->subseq) {
        if (power_up)
            return a->subseq - b->subseq;
        else
            return b->subseq - a->subseq;
    }
    if (a->reg != b->reg)
        return a->reg - b->reg;
    if (a->dpm != b->dpm)
        return (unsigned long)a->dpm - (unsigned long)b->dpm;

    return 0;
}

/* Insert a widget in order into a DPM power sequence. */
static void dpm_seq_insert(struct snd_dpm_widget *new_widget,
                struct list_head *list,
                bool power_up)
{
    struct snd_dpm_widget *w;

    list_for_each_entry(w, list, power_list, struct snd_dpm_widget)
        if (dpm_seq_compare(new_widget, w, power_up) < 0) {
            list_add_tail(&new_widget->power_list, &w->power_list);
            return;
        }

    list_add_tail(&new_widget->power_list, list);
}

static void dpm_seq_check_event(struct snd_dpm_context *dpm,
                 struct snd_dpm_widget *w, int event)
{
    const char *ev_name;
    int power, ret;

    switch (event) {
    case SND_DPM_PRE_PMU:
        ev_name = "PRE_PMU";
        power = 1;
        break;
    case SND_DPM_POST_PMU:
        ev_name = "POST_PMU";
        power = 1;
        break;
    case SND_DPM_PRE_PMD:
        ev_name = "PRE_PMD";
        power = 0;
        break;
    case SND_DPM_POST_PMD:
        ev_name = "POST_PMD";
        power = 0;
        break;
    default:
        BUG(0);
        return;
    }

    if (w->power != power)
        return;

    if (w->event && (w->event_flags & event)) {
        ret = w->event(w, NULL, event);
        if (ret < 0)
            aicx_error(0x1797ef, "ssd", "%s: %s event failed: %d\n", ev_name, w->name, ret);
    }
}

/* Apply the coalesced changes from a DPM sequence */
static void dpm_seq_run_coalesced(struct snd_dpm_context *dpm,
                   struct list_head *pending)
{
    struct snd_dpm_widget *w;
    int reg, power;
    unsigned int value = 0;
    unsigned int mask = 0;
    unsigned int cur_mask;

    /* Dennis: Change code write style, compiler do not support */
#if 0
    reg = list_first_entry(pending, struct snd_dpm_widget,
                   power_list)->reg;
#else
    w = list_first_entry(pending, struct snd_dpm_widget, power_list);
    reg = w->reg;
#endif
    list_for_each_entry(w, pending, power_list, struct snd_dpm_widget) {
        cur_mask = 1 << w->shift;
        /* BUG_ON(reg != w->reg); */

        if (w->invert)
            power = !w->power;
        else
            power = w->power;

        mask |= cur_mask;
        if (power)
            value |= cur_mask;

        /* Check for events */
        dpm_seq_check_event(dpm, w, SND_DPM_PRE_PMU);
        dpm_seq_check_event(dpm, w, SND_DPM_PRE_PMD);
    }

    if (reg >= 0) {
        /* Any widget will do, they should all be updating the
         * same register.
         */
        w = list_first_entry(pending, struct snd_dpm_widget,
                     power_list);

        dpm_widget_update_bits(w, reg, mask, value);
    }

    list_for_each_entry(w, pending, power_list, struct snd_dpm_widget) {
        dpm_seq_check_event(dpm, w, SND_DPM_POST_PMU);
        dpm_seq_check_event(dpm, w, SND_DPM_POST_PMD);
    }
}

/* Apply a DPM power sequence.
 *
 * We walk over a pre-sorted list of widgets to apply power to.  In
 * order to minimise the number of writes to the device required
 * multiple widgets will be updated in a single write where possible.
 * Currently anything that requires more than a single write is not
 * handled.
 */
static void dpm_seq_run(struct snd_dpm_context *dpm,
             struct list_head *list, int event, bool power_up)
{
    struct snd_dpm_widget *w, *n;
    //LIST_HEAD(pending);
    struct list_head pending;
    int cur_sort = -1;
    int cur_subseq = -1;
    int cur_reg = SND_REG_NONE;
    struct snd_dpm_context *cur_dpm = NULL;
    int ret, i;
    int *sort;

    if (power_up)
        sort = dpm_up_seq;
    else
        sort = dpm_down_seq;

    list_head_init(&pending);
    list_for_each_entry_safe(w, n, list, power_list, struct snd_dpm_widget) {
        ret = 0;

        /* Do we need to apply any queued changes? */
        if (sort[w->id] != cur_sort || w->reg != cur_reg ||
            w->dpm != cur_dpm || w->subseq != cur_subseq) {
            if (!list_empty(&pending))
                dpm_seq_run_coalesced(cur_dpm, &pending);

            if (cur_dpm && cur_dpm->seq_notifier) {
                for (i = 0; i < ARRAY_SIZE(dpm_up_seq); i++)
                    if (sort[i] == cur_sort)
                        cur_dpm->seq_notifier(cur_dpm,
                                       i,
                                       cur_subseq);
            }

            list_head_init(&pending);
            cur_sort = -1;
            cur_subseq = INT_MIN;
            cur_reg = SND_REG_NONE;
            cur_dpm = NULL;
        }

        switch (w->id) {
        case snd_dpm_pre:
            if (!w->event)
                list_for_each_entry_safe_continue(w, n, list, power_list, struct snd_dpm_widget);

            if (event == SND_DPM_STREAM_START)
                ret = w->event(w,
                           NULL, SND_DPM_PRE_PMU);
            else if (event == SND_DPM_STREAM_STOP)
                ret = w->event(w,
                           NULL, SND_DPM_PRE_PMD);
            break;

        case snd_dpm_post:
            if (!w->event)
                list_for_each_entry_safe_continue(w, n, list,
                                  power_list, struct snd_dpm_widget);

            if (event == SND_DPM_STREAM_START)
                ret = w->event(w,
                           NULL, SND_DPM_POST_PMU);
            else if (event == SND_DPM_STREAM_STOP)
                ret = w->event(w,
                           NULL, SND_DPM_POST_PMD);
            break;

        default:
            /* Queue it up for application */
            cur_sort = sort[w->id];
            cur_subseq = w->subseq;
            cur_reg = w->reg;
            cur_dpm = w->dpm;
            list_move(&w->power_list, &pending);
            break;
        }

        if (ret < 0)
            aicx_error(0x17c221, "d", "Failed to apply widget power: %d\n", ret);
    }

    if (!list_empty(&pending))
        dpm_seq_run_coalesced(cur_dpm, &pending);

    if (cur_dpm && cur_dpm->seq_notifier) {
        for (i = 0; i < ARRAY_SIZE(dpm_up_seq); i++)
            if (sort[i] == cur_sort)
                cur_dpm->seq_notifier(cur_dpm,
                               i, cur_subseq);
    }
}

static void dpm_widget_update(struct snd_dpm_context *dpm)
{
    struct snd_dpm_update *update = dpm->update;
    struct snd_dpm_widget *w;
    int ret;

    if (!update)
        return;

    w = update->widget;

    if (w->event &&
        (w->event_flags & SND_DPM_PRE_REG)) {
        ret = w->event(w, update->kcontrol, SND_DPM_PRE_REG);
        if (ret != 0)
            aicx_error(0x148b4a, "sd", "%s DPM pre-event failed: %d\n", w->name, ret);
    }

    ret = dpm_reg_update_bits(w->codec, update->reg, update->mask,
                  update->val);
    if (ret < 0)
        aicx_error(0x0fbf44, "sd", "%s DPM update failed: %d\n", w->name, ret);

    if (w->event &&
        (w->event_flags & SND_DPM_POST_REG)) {
        ret = w->event(w, update->kcontrol, SND_DPM_POST_REG);
        if (ret != 0)
            aicx_error(0x07cef3, "sd", "%s DPM post-event failed: %d\n", w->name, ret);
    }
}

/* Async callback run prior to DPM sequences - brings to _PREPARE if
 * they're changing state.
 */
static void dpm_pre_sequence_async(void *data, u32 cookie)
{
    struct snd_dpm_context *d = data;
    int ret;

    /* If we're off and we're not supposed to be go into STANDBY */
    if (d->bias_level == SND_BIAS_OFF &&
        d->target_bias_level != SND_BIAS_OFF) {
        if (d->dev)
            ;//pm_runtime_get_sync(d->dev);

        ret = snd_dpm_set_bias_level(d, SND_BIAS_STANDBY);
        if (ret != 0)
            aicx_error(0x11137a, "d", "Failed to turn on bias: %d\n", ret);
    }

    /* Prepare for a STADDBY->ON or ON->STANDBY transition */
    if (d->bias_level != d->target_bias_level) {
        ret = snd_dpm_set_bias_level(d, SND_BIAS_PREPARE);
        if (ret != 0)
            aicx_error(0x153eb1, "d", "Failed to prepare bias: %d\n", ret);
    }
}

/* Async callback run prior to DPM sequences - brings to their final
 * state.
 */
static void dpm_post_sequence_async(void *data, u32 cookie)
{
    struct snd_dpm_context *d = data;
    int ret;

    /* If we just powered the last thing off drop to standby bias */
    if (d->bias_level == SND_BIAS_PREPARE &&
        (d->target_bias_level == SND_BIAS_STANDBY ||
         d->target_bias_level == SND_BIAS_OFF)) {
        ret = snd_dpm_set_bias_level(d, SND_BIAS_STANDBY);
        if (ret != 0)
            aicx_error(0x08768f, "d", "Failed to apply standby bias: %d\n", ret);
    }

    /* If we're in standby and can support bias off then do that */
    if (d->bias_level == SND_BIAS_STANDBY &&
        d->target_bias_level == SND_BIAS_OFF) {
        ret = snd_dpm_set_bias_level(d, SND_BIAS_OFF);
        if (ret != 0)
            aicx_error(0x020c82, "d", "Failed to turn off bias: %d\n", ret);

        if (d->dev)
            ;//pm_runtime_put_sync(d->dev);
    }

    /* If we just powered up then move to active bias */
    if (d->bias_level == SND_BIAS_PREPARE &&
        d->target_bias_level == SND_BIAS_ON) {
        ret = snd_dpm_set_bias_level(d, SND_BIAS_ON);
        if (ret != 0)
            aicx_error(0x1a3177, "d", "Failed to apply active bias: %d\n", ret);
    }
}

static void dpm_widget_set_peer_power(struct snd_dpm_widget *peer,
                       bool power, bool connect)
{
    /* If a connection is being made or broken then that update
     * will have marked the peer dirty, otherwise the widgets are
     * not connected and this update has no impact. */
    if (!connect)
        return;

    /* If the peer is already in the state we're moving to then we
     * won't have an impact on it. */
    if (power != peer->power)
        dpm_mark_dirty(peer, "peer state change");
}

static void dpm_widget_set_power(struct snd_dpm_widget *w, bool power,
                  struct list_head *up_list,
                  struct list_head *down_list)
{
    struct snd_dpm_path *path;

    if (w->power == power)
        return;

    /* If we changed our power state perhaps our neigbours changed
     * also.
     */
    list_for_each_entry(path, &w->sources, list_sink, struct snd_dpm_path) {
        if (path->source) {
            dpm_widget_set_peer_power(path->source, power,
                           path->connect);
        }
    }
    switch (w->id) {
    case snd_dpm_supply:
        /* Supplies can't affect their outputs, only their inputs */
        break;
    default:
        list_for_each_entry(path, &w->sinks, list_source, struct snd_dpm_path) {
            if (path->sink) {
                dpm_widget_set_peer_power(path->sink, power,
                               path->connect);
            }
        }
        break;
    }

    if (power)
        dpm_seq_insert(w, up_list, TRUE);
    else
        dpm_seq_insert(w, down_list, FALSE);

    w->power = power;
}

static void dpm_power_one_widget(struct snd_dpm_widget *w,
                  struct list_head *up_list,
                  struct list_head *down_list)
{
    int power;

    switch (w->id) {
    case snd_dpm_pre:
        dpm_seq_insert(w, down_list, FALSE);
        break;
    case snd_dpm_post:
        dpm_seq_insert(w, up_list, TRUE);
        break;

    default:
        power = dpm_widget_power_check(w);

        dpm_widget_set_power(w, power, up_list, down_list);
        break;
    }
}

/*
 * Scan each dpm widget for complete audio path.
 * A complete path is a route that has valid endpoints i.e.:-
 *
 *  o DAC to output pin.
 *  o Input Pin to ADC.
 *  o Input pin to Output pin (bypass, sidetone)
 *  o DAC to ADC (loopback).
 */
static int dpm_power_widgets(struct snd_dpm_context *dpm, int event)
{
    struct snd_card *card = dpm->card;
    struct snd_dpm_widget *w;
    struct snd_dpm_context *d;
    struct list_head up_list;
    struct list_head down_list;
    struct list_head async_domain;
    enum snd_bias_level bias;

    list_head_init(&up_list);
    list_head_init(&down_list);
    list_head_init(&async_domain);
    list_for_each_entry(d, &card->dpm_list, list, struct snd_dpm_context) {
        /* now we force set all dapm bias to off/standby. Some dapm has
           no widget, such like external PA, or we cannot shut down all codec.
         */
        /* if (d->n_widgets || d->codec == NULL) */{
            if (d->idle_bias_off)
                d->target_bias_level = SND_BIAS_OFF;
            else
                d->target_bias_level = SND_BIAS_STANDBY;
        }
    }

    //memset(&card->dpm_stats, 0, sizeof(card->dpm_stats));

    list_for_each_entry(w, &card->widgets, list, struct snd_dpm_widget) {
        w->power_checked = FALSE;
        w->inputs = -1;
        w->outputs = -1;
    }

    /* Check which widgets we need to power and store them in
     * lists indicating if they should be powered up or down.  We
     * only check widgets that have been flagged as dirty but note
     * that new widgets may be added to the dirty list while we
     * iterate.
     */
    list_for_each_entry(w, &card->dpm_dirty, dirty, struct snd_dpm_widget) {
        dpm_power_one_widget(w, &up_list, &down_list);
    }

    list_for_each_entry(w, &card->widgets, list, struct snd_dpm_widget) {
        list_del_init(&w->dirty);

        if (w->power) {
            d = w->dpm;

            /* Supplies and micbiases only bring the
             * context up to STANDBY as unless something
             * else is active and passing audio they
             * generally don't require full power.
             */
            switch (w->id) {
            case snd_dpm_supply:
            case snd_dpm_micbias:
                if (d->target_bias_level < SND_BIAS_STANDBY)
                    d->target_bias_level = SND_BIAS_STANDBY;
                break;
            default:
                d->target_bias_level = SND_BIAS_ON;
                break;
            }
        }

    }

    /* If there are no DPM widgets then try to figure out power from the
     * event type.
     */
    if (!dpm->n_widgets) {
        switch (event) {
        case SND_DPM_STREAM_START:
        case SND_DPM_STREAM_RESUME:
            dpm->target_bias_level = SND_BIAS_ON;
            break;
        case SND_DPM_STREAM_STOP:
            //if (dpm->codec && dpm->codec->active)
            //    dpm->target_bias_level = SND_BIAS_ON;
            //else
                dpm->target_bias_level = SND_BIAS_STANDBY;
            break;
        case SND_DPM_STREAM_SUSPEND:
            dpm->target_bias_level = SND_BIAS_STANDBY;
            break;
        case SND_DPM_STREAM_NOP:
            dpm->target_bias_level = dpm->bias_level;
            break;
        default:
            break;
        }
    }

    /* Force all contexts in the card to the same bias state if
     * they're not ground referenced.
     */
    bias = SND_BIAS_OFF;
    list_for_each_entry(d, &card->dpm_list, list, struct snd_dpm_context)
        if (d->target_bias_level > bias)
            bias = d->target_bias_level;
    list_for_each_entry(d, &card->dpm_list, list, struct snd_dpm_context)
        if (!d->idle_bias_off)
            d->target_bias_level = bias;
    /* Run all the bias changes in parallel */
#if 0
    list_for_each_entry(d, &dpm->card->dpm_list, list, struct snd_dpm_context)
        async_schedule_domain(dpm_pre_sequence_async, d,
                    &async_domain);
    async_synchronize_full_domain(&async_domain);
#else
    list_for_each_entry(d, &dpm->card->dpm_list, list, struct snd_dpm_context)
        dpm_pre_sequence_async(d, (u32)&async_domain);
#endif
    /* Power down widgets first; try to avoid amplifying pops. */
    dpm_seq_run(dpm, &down_list, event, FALSE);

    dpm_widget_update(dpm);

    /* Now power up. */
    dpm_seq_run(dpm, &up_list, event, TRUE);
    /* Run all the bias changes in parallel */
#if 0
    list_for_each_entry(d, &dpm->card->dpm_list, list, struct snd_dpm_context)
        async_schedule_domain(dpm_post_sequence_async, d,
                    &async_domain);
    async_synchronize_full_domain(&async_domain);
#else
    list_for_each_entry(d, &dpm->card->dpm_list, list, struct snd_dpm_context)
        dpm_post_sequence_async(d, (u32)&async_domain);
#endif
    return 0;
}

/* test and update the power status of a mux widget */
static int dpm_mux_update_power(struct snd_dpm_widget *widget,
                 struct snd_kcontrol *kcontrol, int change,
                 int mux, struct soc_enum *e)
{
    struct snd_dpm_path *path;
    int found = 0;

    if (widget->id != snd_dpm_mux &&
        widget->id != snd_dpm_virt_mux &&
        widget->id != snd_dpm_value_mux)
        return AIC_ENODEV;

    if (!change)
        return 0;

    /* find dpm widget path assoc with kcontrol */
    list_for_each_entry(path, &widget->dpm->card->paths, list, struct snd_dpm_path) {
        if (path->kcontrol != kcontrol)
            continue;

        if (!path->name || !e->texts[mux])
            continue;

        found = 1;
        /* we now need to match the string in the enum to the path */
        if (!(strcmp(path->name, e->texts[mux]))) {
            path->connect = 1; /* new connection */
            dpm_mark_dirty(path->source, "mux connection");
        } else {
            if (path->connect)
                dpm_mark_dirty(path->source,
                        "mux disconnection");
            path->connect = 0; /* old connection must be powered down */
        }
    }

    if (found) {
        dpm_mark_dirty(widget, "mux change");
        dpm_power_widgets(widget->dpm, SND_DPM_STREAM_NOP);
    }

    return 0;
}

/* test and update the power status of a mixer or switch widget */
static int dpm_mixer_update_power(struct snd_dpm_widget *widget,
                   struct snd_kcontrol *kcontrol, int connect)
{
    struct snd_dpm_path *path;
    int found = 0;

    if (widget->id != snd_dpm_mixer &&
        widget->id != snd_dpm_mixer_named_ctl &&
        widget->id != snd_dpm_switch)
        return AIC_ENODEV;

    /* find dpm widget path assoc with kcontrol */
    list_for_each_entry(path, &widget->dpm->card->paths, list, struct snd_dpm_path) {
        if (path->kcontrol != kcontrol)
            continue;

        /* found, now check type */
        found = 1;
        path->connect = connect;
        dpm_mark_dirty(path->source, "mixer connection");
    }

    if (found) {
        dpm_mark_dirty(widget, "mixer update");
        dpm_power_widgets(widget->dpm, SND_DPM_STREAM_NOP);
    }

    return 0;
}

/* free all dpm widgets and resources */
static void dpm_free_widgets(struct snd_dpm_context *dpm)
{
    struct snd_dpm_widget *w, *next_w;
    struct snd_dpm_path *p, *next_p;

    list_for_each_entry_safe(w, next_w, &dpm->card->widgets, list, struct snd_dpm_widget) {
        if (w->dpm != dpm)
            continue;
        list_del(&w->list);
        /*
         * remove source and sink paths associated to this widget.
         * While removing the path, remove reference to it from both
         * source and sink widgets so that path is removed only once.
         */
        list_for_each_entry_safe(p, next_p, &w->sources, list_sink, struct snd_dpm_path) {
            list_del(&p->list_sink);
            list_del(&p->list_source);
            list_del(&p->list);
            snd_free(p->long_name);
            snd_free(p);
        }
        list_for_each_entry_safe(p, next_p, &w->sinks, list_source, struct snd_dpm_path) {
            list_del(&p->list_sink);
            list_del(&p->list_source);
            list_del(&p->list);
            snd_free(p->long_name);
            snd_free(p);
        }
        snd_free(w->kcontrols);
        snd_free(w->name);
        snd_free(w);
    }
}

static struct snd_dpm_widget *dpm_find_widget(
            struct snd_dpm_context *dpm, const char *pin,
            bool search_other_contexts)
{
    struct snd_dpm_widget *w;
    struct snd_dpm_widget *fallback = NULL;

    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget) {
        if (!strcmp(w->name, pin)) {
            if (w->dpm == dpm)
                return w;
            else
                fallback = w;
        }
    }

    if (search_other_contexts)
        return fallback;

    return NULL;
}

static int snd_dpm_set_pin(struct snd_dpm_context *dpm,
                const char *pin, int status)
{
    struct snd_dpm_widget *w = dpm_find_widget(dpm, pin, TRUE);

    if (!w) {
        aicx_error(0x1117f9, "s", "dpm: unknown pin %s\n", pin);
        return AIC_EINVAL;
    }

    w->connected = status;
    if (status == 0)
        w->force = 0;
    dpm_mark_dirty(w, "pin configuration");

    return 0;
}

/**
 * snd_dpm_sync - scan and power dpm paths
 * @dpm: DPM context
 *
 * Walks all dpm audio paths and powers widgets according to their
 * stream or path usage.
 *
 * Returns 0 for success.
 */
int snd_dpm_sync(struct snd_dpm_context *dpm)
{
    /*
     * Suppress early reports (eg, jacks syncing their state) to avoid
     * silly DPM runs during card startup.
     */
    if (!dpm->card)
        return 0;

    return dpm_power_widgets(dpm, SND_DPM_STREAM_NOP);
}

static int snd_dpm_add_route(struct snd_dpm_context *dpm,
                  const struct snd_dpm_route *route)
{
    struct snd_dpm_path *path;
    struct snd_dpm_widget *wsource = NULL, *wsink = NULL, *w;
    struct snd_dpm_widget *wtsource = NULL, *wtsink = NULL;
    const char *sink;
    const char *control = route->control;
    const char *source;
    int ret = 0;
#if 0
    char prefixed_sink[80];
    char prefixed_source[80];
    if (dpm->codec && dpm->codec->name_prefix) {
        snprintf(prefixed_sink, sizeof(prefixed_sink), "%s %s",
             dpm->codec->name_prefix, route->sink);
        sink = prefixed_sink;
        snprintf(prefixed_source, sizeof(prefixed_source), "%s %s",
             dpm->codec->name_prefix, route->source);
        source = prefixed_source;
    } else
#endif
        {
        sink = route->sink;
        source = route->source;
    }

    /*
     * find src and dest widgets over all widgets but favor a widget from
     * current DPM context
     */
    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget) {
        if (!wsink && !(strcmp(w->name, sink))) {
            wtsink = w;
            if (w->dpm == dpm)
                wsink = w;
            continue;
        }
        if (!wsource && !(strcmp(w->name, source))) {
            wtsource = w;
            if (w->dpm == dpm)
                wsource = w;
        }
    }
    /* use widget from another DPM context if not found from this */
    if (!wsink)
        wsink = wtsink;
    if (!wsource)
        wsource = wtsource;

    if (wsource == NULL || wsink == NULL)
        return AIC_ENODEV;

    path = snd_zalloc(sizeof(struct snd_dpm_path));
    if (!path)
        return AIC_ENOMEM;

    path->source = wsource;
    path->sink = wsink;
    path->connected = route->connected;
    INIT_LIST_HEAD(&path->list);
    INIT_LIST_HEAD(&path->list_source);
    INIT_LIST_HEAD(&path->list_sink);

    /* check for external widgets */
    if (wsink->id == snd_dpm_input) {
        if (wsource->id == snd_dpm_micbias ||
            wsource->id == snd_dpm_mic ||
            wsource->id == snd_dpm_line ||
            wsource->id == snd_dpm_output)
            wsink->ext = 1;
    }
    if (wsource->id == snd_dpm_output) {
        if (wsink->id == snd_dpm_spk ||
            wsink->id == snd_dpm_hp ||
            wsink->id == snd_dpm_line ||
            wsink->id == snd_dpm_input)
            wsource->ext = 1;
    }

    /* connect static paths */
    if (control == NULL) {
        list_add(&path->list, &dpm->card->paths);
        list_add(&path->list_sink, &wsink->sources);
        list_add(&path->list_source, &wsource->sinks);
        path->connect = 1;
        return 0;
    }

    /* connect dynamic paths */
    switch (wsink->id) {
    case snd_dpm_adc:
    case snd_dpm_dac:
    case snd_dpm_pga:
    case snd_dpm_out_drv:
    case snd_dpm_input:
    case snd_dpm_output:
    case snd_dpm_siggen:
    case snd_dpm_micbias:
    case snd_dpm_vmid:
    case snd_dpm_pre:
    case snd_dpm_post:
    case snd_dpm_supply:
    case snd_dpm_aif_in:
    case snd_dpm_aif_out:
        list_add(&path->list, &dpm->card->paths);
        list_add(&path->list_sink, &wsink->sources);
        list_add(&path->list_source, &wsource->sinks);
        path->connect = 1;
        return 0;
    case snd_dpm_mux:
    case snd_dpm_virt_mux:
    case snd_dpm_value_mux:
        ret = dpm_connect_mux(dpm, wsource, wsink, path, control,
            &wsink->kcontrol_news[0]);
        if (ret != 0)
            goto err;
        break;
    case snd_dpm_switch:
    case snd_dpm_mixer:
    case snd_dpm_mixer_named_ctl:
        ret = dpm_connect_mixer(dpm, wsource, wsink, path, control);
        if (ret != 0)
            goto err;
        break;
    case snd_dpm_hp:
    case snd_dpm_mic:
    case snd_dpm_line:
    case snd_dpm_spk:
        list_add(&path->list, &dpm->card->paths);
        list_add(&path->list_sink, &wsink->sources);
        list_add(&path->list_source, &wsource->sinks);
        path->connect = 0;
        return 0;
    default:
        break;
    }
    return 0;

err:
    aicx_warn(0x032c62, "sss", "asoc: no dpm match for %s --> %s --> %s\n",
              source, control, sink);
    snd_free(path);
    return ret;
}

/**
 * snd_dpm_add_routes - Add routes between DPM widgets
 * @dpm: DPM context
 * @route: audio routes
 * @num: number of routes
 *
 * Connects 2 dpm widgets together via a named audio path. The sink is
 * the widget receiving the audio signal, whilst the source is the sender
 * of the audio signal.
 *
 * Returns 0 for success else error. On error all resources can be freed
 * with a call to xxx_card_free().
 */
int snd_dpm_add_routes(struct snd_dpm_context *dpm,
                const struct snd_dpm_route *route, int num)
{
    int i, ret;

    for (i = 0; i < num; i++) {
        ret = snd_dpm_add_route(dpm, route);
        if (ret < 0) {
            aicx_error(0x03189a, "ss", "Failed to add route %s->%s\n",
                       route->source, route->sink);
            return ret;
        }
        route++;
    }

    return 0;
}

/**
 * snd_dpm_new_widgets - add new dpm widgets
 * @dpm: DPM context
 *
 * Checks the codec for any new dpm widgets and creates them if found.
 *
 * Returns 0 for success.
 */
int snd_dpm_new_widgets(struct snd_dpm_context *dpm)
{
    struct snd_dpm_widget *w;
    unsigned int val;

    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget)
    {
        if (w->new)
            continue;

        if (w->num_kcontrols) {
            w->kcontrols = snd_zalloc(w->num_kcontrols *
                        sizeof(struct snd_kcontrol *));
            if (!w->kcontrols)
                return AIC_ENOMEM;
        }

        switch(w->id) {
        case snd_dpm_switch:
        case snd_dpm_mixer:
        case snd_dpm_mixer_named_ctl:
            dpm_new_mixer(w);
            break;
        case snd_dpm_mux:
        case snd_dpm_virt_mux:
        case snd_dpm_value_mux:
            dpm_new_mux(w);
            break;
        case snd_dpm_pga:
        case snd_dpm_out_drv:
            dpm_new_pga(w);
            break;
        default:
            break;
        }

        /* Read the initial power state from the device */
        if (w->reg >= 0) {
            val = dpm_widget_read(w, w->reg);
            val &= 1 << w->shift;
            if (w->invert)
                val = !val;

            if (val)
                w->power = 1;
        }

        w->new = 1;

        dpm_mark_dirty(w, "new widget");
    }

    dpm_power_widgets(dpm, SND_DPM_STREAM_NOP);
    return 0;
}

#define snd_kcontrol_chip(kcontrol) ((kcontrol)->private_data)
/**
 * snd_dpm_get_volsw - dpm mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a dpm mixer control.
 *
 * Returns 0 for success.
 */
int snd_dpm_get_volsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int reg = mc->reg;
    unsigned int shift = mc->shift;
    unsigned int rshift = mc->rshift;
    int max = mc->max;
    unsigned int invert = mc->invert;
    unsigned int mask = (1 << fls(max)) - 1;

    ucontrol->value.integer.value[0] =
        (dpm_reg_read(widget->codec, reg) >> shift) & mask;
    if (shift != rshift)
        ucontrol->value.integer.value[1] =
            (dpm_reg_read(widget->codec, reg) >> rshift) & mask;
    if (invert) {
        ucontrol->value.integer.value[0] =
            max - ucontrol->value.integer.value[0];
        if (shift != rshift)
            ucontrol->value.integer.value[1] =
                max - ucontrol->value.integer.value[1];
    }

    return 0;
}

/**
 * snd_dpm_put_volsw - dpm mixer set callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a dpm mixer control.
 *
 * Returns 0 for success.
 */
int snd_dpm_put_volsw(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct snd_codec *codec = widget->codec;
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int reg = mc->reg;
    unsigned int shift = mc->shift;
    int max = mc->max;
    unsigned int mask = (1 << fls(max)) - 1;
    unsigned int invert = mc->invert;
    unsigned int val;
    int connect;
    struct snd_dpm_update update;
    int wi;

    val = (ucontrol->value.integer.value[0] & mask);

    if (invert)
        val = max - val;
    mask = mask << shift;
    val = val << shift;

    if (val)
        /* new connection */
        connect = invert ? 0 : 1;
    else
        /* old connection must be powered down */
        connect = invert ? 1 : 0;

    mutex_lock(codec->mutex);

    dpm_reg_test_bits(widget->codec, reg, mask, val);

    /**
     * Dennis: now update anyway, ignore test_bits result.
     *      maybe we don't set register here but set it a little later for
     *      some resean like pop noise or power sequence.
     */
    /* if (change) { */
        for (wi = 0; wi < wlist->num_widgets; wi++) {
            widget = wlist->widgets[wi];

            widget->value = val;

            update.kcontrol = kcontrol;
            update.widget = widget;
            update.reg = reg;
            update.mask = mask;
            update.val = val;
            widget->dpm->update = &update;

            dpm_mixer_update_power(widget, kcontrol, connect);

            widget->dpm->update = NULL;
        }
    /* } */

    mutex_unlock(codec->mutex);
    return 0;
}

/**
 * snd_dpm_get_enum_double - dpm enumerated double mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a dpm enumerated double mixer control.
 *
 * Returns 0 for success.
 */
int snd_dpm_get_enum_double(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int val, bitmask;

    for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
        ;
    val = dpm_reg_read(widget->codec, e->reg);
    ucontrol->value.enumerated.item[0] = (val >> e->shift_l) & (bitmask - 1);
    if (e->shift_l != e->shift_r)
        ucontrol->value.enumerated.item[1] =
            (val >> e->shift_r) & (bitmask - 1);

    return 0;
}

/**
 * snd_dpm_put_enum_double - dpm enumerated double mixer set callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a dpm enumerated double mixer control.
 *
 * Returns 0 for success.
 */
int snd_dpm_put_enum_double(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct snd_codec *codec = widget->codec;
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int val, mux, change;
    unsigned int mask, bitmask;
    struct snd_dpm_update update;
    int wi;

    for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
        ;
    if (ucontrol->value.enumerated.item[0] > e->max - 1)
        return AIC_EINVAL;
    mux = ucontrol->value.enumerated.item[0];
    val = mux << e->shift_l;
    mask = (bitmask - 1) << e->shift_l;
    if (e->shift_l != e->shift_r) {
        if (ucontrol->value.enumerated.item[1] > e->max - 1)
            return AIC_EINVAL;
        val |= ucontrol->value.enumerated.item[1] << e->shift_r;
        mask |= (bitmask - 1) << e->shift_r;
    }

    mutex_lock(codec->mutex);

    change = dpm_reg_test_bits(widget->codec, e->reg, mask, val);
    if (change) {
        for (wi = 0; wi < wlist->num_widgets; wi++) {
            widget = wlist->widgets[wi];

            widget->value = val;

            update.kcontrol = kcontrol;
            update.widget = widget;
            update.reg = e->reg;
            update.mask = mask;
            update.val = val;
            widget->dpm->update = &update;

            dpm_mux_update_power(widget, kcontrol, change, mux, e);

            widget->dpm->update = NULL;
        }
    }

    mutex_unlock(codec->mutex);

    return change;
}

/**
 * snd_dpm_get_enum_virt - Get virtual DPM mux
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Returns 0 for success.
 */
int snd_dpm_get_enum_virt(struct snd_kcontrol *kcontrol,
                   struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];

    ucontrol->value.enumerated.item[0] = widget->value;

    return 0;
}

/**
 * snd_dpm_put_enum_virt - Set virtual DPM mux
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Returns 0 for success.
 */
int snd_dpm_put_enum_virt(struct snd_kcontrol *kcontrol,
                   struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct snd_codec *codec = widget->codec;
    struct soc_enum *e =
        (struct soc_enum *)kcontrol->private_value;
    int change;
    int ret = 0;
    int wi;

    if (ucontrol->value.enumerated.item[0] >= e->max)
        return AIC_EINVAL;

    mutex_lock(codec->mutex);

    change = widget->value != ucontrol->value.enumerated.item[0];
    if (change) {
        for (wi = 0; wi < wlist->num_widgets; wi++) {
            widget = wlist->widgets[wi];

            widget->value = ucontrol->value.enumerated.item[0];

            dpm_mux_update_power(widget, kcontrol, change,
                          widget->value, e);
        }
    }

    mutex_unlock(&codec->mutex);
    return ret;
}

/**
 * snd_dpm_get_value_enum_double - dpm semi enumerated double mixer get
 *                    callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a dpm semi enumerated double mixer control.
 *
 * Semi enumerated mixer: the enumerated items are referred as values. Can be
 * used for handling bitfield coded enumeration for example.
 *
 * Returns 0 for success.
 */
int snd_dpm_get_value_enum_double(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int reg_val, val, mux;

    reg_val = dpm_reg_read(widget->codec, e->reg);
    val = (reg_val >> e->shift_l) & e->mask;
    for (mux = 0; mux < e->max; mux++) {
        if (val == e->values[mux])
            break;
    }
    ucontrol->value.enumerated.item[0] = mux;
    if (e->shift_l != e->shift_r) {
        val = (reg_val >> e->shift_r) & e->mask;
        for (mux = 0; mux < e->max; mux++) {
            if (val == e->values[mux])
                break;
        }
        ucontrol->value.enumerated.item[1] = mux;
    }

    return 0;
}

/**
 * snd_dpm_put_value_enum_double - dpm semi enumerated double mixer set
 *                    callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a dpm semi enumerated double mixer control.
 *
 * Semi enumerated mixer: the enumerated items are referred as values. Can be
 * used for handling bitfield coded enumeration for example.
 *
 * Returns 0 for success.
 */
int snd_dpm_put_value_enum_double(struct snd_kcontrol *kcontrol,
    struct snd_ctl_elem_value *ucontrol)
{
    struct snd_dpm_widget_list *wlist = snd_kcontrol_chip(kcontrol);
    struct snd_dpm_widget *widget = wlist->widgets[0];
    struct snd_codec *codec = widget->codec;
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int val, mux, change;
    unsigned int mask;
    struct snd_dpm_update update;
    int wi;

    if (ucontrol->value.enumerated.item[0] > e->max - 1)
        return AIC_EINVAL;
    mux = ucontrol->value.enumerated.item[0];
    val = e->values[ucontrol->value.enumerated.item[0]] << e->shift_l;
    mask = e->mask << e->shift_l;
    if (e->shift_l != e->shift_r) {
        if (ucontrol->value.enumerated.item[1] > e->max - 1)
            return AIC_EINVAL;
        val |= e->values[ucontrol->value.enumerated.item[1]] << e->shift_r;
        mask |= e->mask << e->shift_r;
    }

    mutex_lock(codec->mutex);

    change = dpm_reg_test_bits(widget->codec, e->reg, mask, val);
    if (change) {
        for (wi = 0; wi < wlist->num_widgets; wi++) {
            widget = wlist->widgets[wi];

            widget->value = val;

            update.kcontrol = kcontrol;
            update.widget = widget;
            update.reg = e->reg;
            update.mask = mask;
            update.val = val;
            widget->dpm->update = &update;

            dpm_mux_update_power(widget, kcontrol, change, mux, e);

            widget->dpm->update = NULL;
        }
    }

    mutex_unlock(codec->mutex);

    return change;
}

/**
 * snd_dpm_info_pin_switch - Info for a pin switch
 *
 * @kcontrol: mixer control
 * @uinfo: control element information
 *
 * Callback to provide information about a pin switch control.
 */
int snd_dpm_info_pin_switch(struct snd_kcontrol *kcontrol,
                 struct snd_ctl_elem_info *uinfo)
{
    uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
    uinfo->count = 1;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = 1;

    return 0;
}

/**
 * snd_dpm_get_pin_switch - Get information for a pin switch
 *
 * @kcontrol: mixer control
 * @ucontrol: Value
 */
int snd_dpm_get_pin_switch(struct snd_kcontrol *kcontrol,
                struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    const char *pin = (const char *)kcontrol->private_value;

    mutex_lock(codec->mutex);

    ucontrol->value.integer.value[0] =
        snd_dpm_get_pin_status(&codec->dpm, pin);

    mutex_unlock(codec->mutex);

    return 0;
}

/**
 * snd_dpm_put_pin_switch - Set information for a pin switch
 *
 * @kcontrol: mixer control
 * @ucontrol: Value
 */
int snd_dpm_put_pin_switch(struct snd_kcontrol *kcontrol,
                struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    const char *pin = (const char *)kcontrol->private_value;

    mutex_lock(codec->mutex);

    if (ucontrol->value.integer.value[0])
        snd_dpm_enable_pin(&codec->dpm, pin);
    else
        snd_dpm_disable_pin(&codec->dpm, pin);

    snd_dpm_sync(&codec->dpm);

    mutex_unlock(codec->mutex);

    return 0;
}

/**
 * snd_dpm_get_pin_status - get audio pin status
 * @dpm: DPM context
 * @pin: audio signal pin endpoint (or start point)
 *
 * Get audio pin status - connected or disconnected.
 *
 * Returns 1 for connected otherwise 0.
 */
int snd_dpm_get_pin_status(struct snd_dpm_context *dpm,
                const char *pin)
{
    struct snd_dpm_widget *w = dpm_find_widget(dpm, pin, TRUE);

    if (w)
        return w->connected;

    return 0;
}

/**
 * snd_dpm_new_control - create new dpm control
 * @dpm: DPM context
 * @widget: widget template
 *
 * Creates a new dpm control based upon the template.
 *
 * Returns 0 for success else error.
 */
int snd_dpm_new_control(struct snd_dpm_context *dpm,
    const struct snd_dpm_widget *widget)
{
    struct snd_dpm_widget *w;
    size_t name_len;

    if ((w = dpm_cnew_widget(widget)) == NULL)
        return AIC_ENOMEM;

    name_len = strlen(widget->name) + 1;
    //if (dpm->codec && dpm->codec->name_prefix)
    //    name_len += 1 + strlen(dpm->codec->name_prefix);
    w->name = snd_malloc(name_len);
    if (w->name == NULL) {
        snd_free(w);
        return AIC_ENOMEM;
    }
    //if (dpm->codec && dpm->codec->name_prefix)
    //    snprintf(w->name, name_len, "%s %s",
    //        dpm->codec->name_prefix, widget->name);
    //else
        snprintf(w->name, name_len, "%s", widget->name);

    switch (w->id) {
    case snd_dpm_switch:
    case snd_dpm_mixer:
    case snd_dpm_mixer_named_ctl:
        w->power_check = dpm_generic_check_power;
        break;
    case snd_dpm_mux:
    case snd_dpm_virt_mux:
    case snd_dpm_value_mux:
        w->power_check = dpm_generic_check_power;
        break;
    case snd_dpm_adc:
    case snd_dpm_aif_out:
        w->power_check = dpm_adc_check_power;
        break;
    case snd_dpm_dac:
    case snd_dpm_aif_in:
        w->power_check = dpm_dac_check_power;
        break;
    case snd_dpm_pga:
    case snd_dpm_out_drv:
    case snd_dpm_input:
    case snd_dpm_output:
    case snd_dpm_micbias:
    case snd_dpm_spk:
    case snd_dpm_hp:
    case snd_dpm_mic:
    case snd_dpm_line:
        w->power_check = dpm_generic_check_power;
        break;
    case snd_dpm_supply:
        w->power_check = dpm_supply_check_power;
        break;
    default:
        w->power_check = dpm_always_on_check_power;
        break;
    }

    dpm->n_widgets++;
    w->dpm = dpm;
    w->codec = dpm->codec;
    INIT_LIST_HEAD(&w->sources);
    INIT_LIST_HEAD(&w->sinks);
    INIT_LIST_HEAD(&w->list);
    INIT_LIST_HEAD(&w->dirty);
    list_add(&w->list, &dpm->card->widgets);

    /* machine layer set ups unconnected pins and insertions */
    w->connected = 1;
    return 0;
}

/**
 * snd_dpm_new_controls - create new dpm controls
 * @dpm: DPM context
 * @widget: widget array
 * @num: number of widgets
 *
 * Creates new DPM controls based upon the templates.
 *
 * Returns 0 for success else error.
 */
int snd_dpm_new_controls(struct snd_dpm_context *dpm,
    const struct snd_dpm_widget *widget,
    int num)
{
    int i, ret;

    for (i = 0; i < num; i++) {
        ret = snd_dpm_new_control(dpm, widget);
        if (ret < 0) {
            aicx_error(0x1a322c, "sd", "ASoC: Failed to create DPM control %s: %d\n",
                       widget->name, ret);
            return ret;
        }
        widget++;
    }
    return 0;
}

static void soc_dpm_stream_event(struct snd_dpm_context *dpm,
    const char *stream, int event)
{
    struct snd_dpm_widget *w;

    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget)
    {
        if (!w->sname || w->dpm != dpm)
            continue;
        aicx_debug(0x0cd3e1, "sssd", "widget %s\n %s stream %s event %d\n",
            w->name, w->sname, stream, event);
        if (strstr(w->sname, stream)) {
            dpm_mark_dirty(w, "stream event");
            switch(event) {
            case SND_DPM_STREAM_START:
                w->active = 1;
                break;
            case SND_DPM_STREAM_STOP:
                w->active = 0;
                break;
            case SND_DPM_STREAM_SUSPEND:
            case SND_DPM_STREAM_RESUME:
            case SND_DPM_STREAM_PAUSE_PUSH:
            case SND_DPM_STREAM_PAUSE_RELEASE:
                break;
            }
        }
    }

    dpm_power_widgets(dpm, event);

    /* do we need to notify any clients that DPM stream is complete */
    if (dpm->stream_event)
        dpm->stream_event(dpm, event);
}

/**
 * snd_dpm_stream_event - send a stream event to the dpm core
 * @rtd: PCM runtime data
 * @stream: stream name
 * @event: stream event
 *
 * Sends a stream event to the dpm core. The core then makes any
 * necessary widget power changes.
 *
 * Returns 0 for success else error.
 */
int snd_dpm_stream_event(struct snd_codec *codec,
    const char *stream, int event)
{
    if (stream == NULL || codec == NULL)
        return 0;

    mutex_lock(codec->mutex);
    soc_dpm_stream_event(&codec->dpm, stream, event);
    mutex_unlock(codec->mutex);

    return 0;
}

/**
 * snd_dpm_enable_pin - enable pin.
 * @dpm: DPM context
 * @pin: pin name
 *
 * Enables input/output pin and its parents or children widgets iff there is
 * a valid audio route and active audio stream.
 * NOTE: snd_dpm_sync() needs to be called after this for DPM to
 * do any widget power switching.
 */
int snd_dpm_enable_pin(struct snd_dpm_context *dpm, const char *pin)
{
    return snd_dpm_set_pin(dpm, pin, 1);
}

/**
 * snd_dpm_force_enable_pin - force a pin to be enabled
 * @dpm: DPM context
 * @pin: pin name
 *
 * Enables input/output pin regardless of any other state.  This is
 * intended for use with microphone bias supplies used in microphone
 * jack detection.
 *
 * NOTE: snd_dpm_sync() needs to be called after this for DPM to
 * do any widget power switching.
 */
int snd_dpm_force_enable_pin(struct snd_dpm_context *dpm,
                  const char *pin)
{
    struct snd_dpm_widget *w = dpm_find_widget(dpm, pin, TRUE);

    if (!w) {
        aicx_error(0x1117f9, "s", "dpm: unknown pin %s\n", pin);
        return AIC_EINVAL;
    }

    w->connected = 1;
    w->force = 1;
    dpm_mark_dirty(w, "force enable");

    return 0;
}

int snd_dpm_enable_pin_sync(char *name)
{
    struct snd_card *card = snd_card_get();
    struct snd_dpm_context *dpm = &card->dpm.codec->dpm;

    if (snd_dpm_force_enable_pin(dpm, name))
        return AIC_EINVAL;

    return snd_dpm_sync(dpm);
}

/**
* snd_dpm_disable_pin - disable pin.
* @dpm: DPM context
* @pin: pin name
*
* Disables input/output pin and its parents or children widgets.
* NOTE: snd_dpm_sync() needs to be called after this for DPM to
* do any widget power switching.
*/
int snd_dpm_disable_pin(struct snd_dpm_context *dpm,
               const char *pin)
{
  return snd_dpm_set_pin(dpm, pin, 0);
}


/**
 * snd_dpm_free - free dpm resources
 * @dpm: DPM context
 *
 * Free all dpm widgets and resources.
 */
void snd_dpm_free(struct snd_dpm_context *dpm)
{
    dpm_free_widgets(dpm);
    list_del(&dpm->list);
}
#if 0
static void soc_dpm_shutdown_codec(struct snd_dpm_context *dpm)
{
    struct snd_dpm_widget *w;
    struct list_head down_list;
    int powerdown = 0;

    list_head_init(&down_list);
    list_for_each_entry(w, &dpm->card->widgets, list, struct snd_dpm_widget) {
        if (w->dpm != dpm)
            continue;
        if (w->power) {
            dpm_seq_insert(w, &down_list, FALSE);
            w->power = 0;
            powerdown = 1;
        }
    }

    /* If there were no widgets to power down we're already in
     * standby.
     */
    if (powerdown) {
        dpm_seq_run(dpm, &down_list, 0, FALSE);
    }
}
/*
 * snd_dpm_shutdown - callback for system shutdown
 */
 void snd_dpm_shutdown(struct snd_card *card)
 {
     struct snd_codec *codec;

     list_for_each_entry(codec, &card->codec_dev_list, list, struct snd_codec) {
         soc_dpm_shutdown_codec(&codec->dpm);
     }
 }
#endif

int snd_dpm_get_widget_status(char *name, char *buf, size_t count)
{
    struct snd_dpm_widget *w;
    int in, out, found;
    int ret;
    struct snd_dpm_path *p = NULL;
    struct snd_card *card = snd_card_get();

    if (!name || !buf || count == 0)
        return AIC_EINVAL;

    found = 0;
    list_for_each_entry(w, &card->widgets, list, struct snd_dpm_widget) {
        if(!strcmp(name, w->name)) {
            found = 1;
            break;
        }
    }
    if (!found)
        return AIC_ENODATA;

    in = is_connected_input_ep(w);
    dpm_clear_walk(w->dpm);
    out = is_connected_output_ep(w);
    dpm_clear_walk(w->dpm);

    ret = snprintf(buf, count, "%s: %s  in %d out %d",
                   w->name, w->power ? "On" : "Off", in, out);

    if (w->reg >= 0)
        ret += snprintf(buf + ret, count - ret,
                        " - R%d(0x%x) bit %d",
                        w->reg, w->reg, w->shift);

    ret += snprintf(buf + ret, count - ret, "\n");

    if (w->sname)
        ret += snprintf(buf + ret, count - ret, " stream %s %s\n",
                        w->sname,
                        w->active ? "active" : "inactive");

    list_for_each_entry(p, &w->sources, list_sink, struct snd_dpm_path) {
        if (p->connected && !p->connected(w, p->sink))
            continue;

        if (p->connect)
            ret += snprintf(buf + ret, count - ret,
                            " in  \"%s\" \"%s\"\n",
                            p->name ? p->name : "static",
                            p->source->name);
    }
    list_for_each_entry(p, &w->sinks, list_source, struct snd_dpm_path) {
        if (p->connected && !p->connected(w, p->sink))
            continue;

        if (p->connect)
            ret += snprintf(buf + ret, count - ret,
                            " out \"%s\" \"%s\"\n",
                            p->name ? p->name : "static",
                            p->sink->name);
    }

    return ret;
}

void snd_dpm_list_all(void)
{
    struct snd_card *card = snd_card_get();
    struct snd_dpm_widget *w;
    int in, out;

    list_for_each_entry(w, &card->widgets, list, struct snd_dpm_widget) {
        in = is_connected_input_ep(w);
        dpm_clear_walk(w->dpm);
        out = is_connected_output_ep(w);
        dpm_clear_walk(w->dpm);
        CONS_OUT("%20s: %s  \tin%2d    out%2d\n",
                 w->name, w->power ? "On" : "Off", in, out);
    }
}

