/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines some MACROs, by which it can be compatible with
 *    other system without changing code.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0bb5a6
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include "snd_card.h"
#include "snd_dpm_driver.h"


static struct snd_kcontrol *snd_ctl_new_i(struct snd_kcontrol *control,
                                          unsigned int access)
{
    struct snd_kcontrol *kctl;
    unsigned int idx;

    if (!control || !control->count)
        return NULL;

    if (control->count > MAX_CTL_VOLATILE) {
        aicx_error(0x16a2f7, "", "control count is exceeded\n");
        return NULL;
    }

    kctl = snd_zalloc(sizeof(*kctl));
    if (kctl == NULL) {
        aicx_error(0x111fa7, "", "Cannot allocate control instance\n");
        return NULL;
    }
    *kctl = *control;
    for (idx = 0; idx < kctl->count; idx++)
        kctl->vd[idx].access = access;
    return kctl;
}

static struct snd_kcontrol *snd_ctl_new(const struct snd_kcontrol_new *ncontrol,
                                        void *private_data)
{
    struct snd_kcontrol kctl;
    unsigned int access;

    if (!ncontrol || !ncontrol->info)
        return NULL;
    memset(&kctl, 0, sizeof(kctl));
    kctl.id.iface = ncontrol->iface;
    kctl.id.device = ncontrol->device;
    kctl.id.subdevice = ncontrol->subdevice;
    if (ncontrol->name) {
        strncpy(kctl.id.name, ncontrol->name, sizeof(kctl.id.name) - 1);
        if (strcmp(ncontrol->name, kctl.id.name) != 0)
            aicx_info(0x0cf740, "ss", "Control name '%s' truncated to '%s'\n",
                      ncontrol->name, kctl.id.name);
    }
    kctl.id.index = ncontrol->index;
    kctl.count = ncontrol->count ? ncontrol->count : 1;
    access = ncontrol->access == 0 ? SNDRV_CTL_ELEM_ACCESS_READWRITE :
         (ncontrol->access & (SNDRV_CTL_ELEM_ACCESS_READWRITE|
                      SNDRV_CTL_ELEM_ACCESS_INACTIVE|
                      SNDRV_CTL_ELEM_ACCESS_TLV_READWRITE|
                      SNDRV_CTL_ELEM_ACCESS_TLV_COMMAND|
                      SNDRV_CTL_ELEM_ACCESS_TLV_CALLBACK));
    kctl.info = ncontrol->info;
    kctl.get = ncontrol->get;
    kctl.put = ncontrol->put;
    kctl.tlv.p = ncontrol->tlv.p;
    kctl.private_value = ncontrol->private_value;
    kctl.private_data = private_data;
    return snd_ctl_new_i(&kctl, access);
}

int snd_ctl_add(struct snd_kcontrol *kcontrol)
{
    struct snd_card *card = snd_card_get();

    if (!kcontrol)
        return AIC_ENODATA;
    kcontrol->id.numid = ++card->last_numid;
    list_add_tail(&kcontrol->list, &card->ctls);

    return 0;
}

struct snd_kcontrol *snd_ctl_find_id(struct snd_ctl_elem_id *id)
{
    struct snd_card *card = snd_card_get();
    struct snd_kcontrol *kctl;

    if (!id)
        return NULL;

    list_for_each_entry(kctl, &card->ctls, list, struct snd_kcontrol) {
        if (id->numid != 0 && id->numid == kctl->id.numid)
            return kctl;
        if (strncmp(kctl->id.name, id->name, sizeof(kctl->id.name)))
            continue;
        return kctl;
    }

    return NULL;
}

int snd_ctl_read(struct snd_ctl_elem_value *control)
{
    struct snd_kcontrol *kctl;

    kctl = snd_ctl_find_id(&control->id);
    if (kctl == NULL)
        return AIC_ENOENT;

    return kctl->get(kctl, control);
}

int snd_ctl_write(struct snd_ctl_elem_value *control)
{
    struct snd_kcontrol *kctl;
    int result;

    kctl = snd_ctl_find_id(&control->id);
    if (kctl == NULL)
        return AIC_ENOENT;

    result = kctl->put(kctl, control);
    if (result < 0)
        aicx_error(0x042dff, "d", "snd ctl write error: %d\n", result);

    return result;
}

int snd_ctl_get_info(struct snd_ctl_elem_value *control,
                     struct snd_ctl_elem_info *info)
{
    struct snd_kcontrol *kctl;

    int result;

    kctl = snd_ctl_find_id(&control->id);
    if (!kctl)
        return AIC_EINVAL;

    result = kctl->info(kctl, info);
    if (!result) {
        strcpy(info->id.name, kctl->id.name);
        info->id.numid = control->id.numid;
    }

    return result;
}

int snd_ctl_get_info_numid(int numid, struct snd_ctl_elem_info *info)
{
    struct snd_card *card = snd_card_get();
    struct snd_kcontrol *kctl;
    int result = 0;

    if (numid == 0)
        return 0;
    list_for_each_entry(kctl, &card->ctls, list, struct snd_kcontrol) {
        if (numid == kctl->id.numid) {
            result = kctl->info(kctl, info);
            break;
        }
    }

    if (!result) {
        strcpy(info->id.name, kctl->id.name);
        info->id.numid = numid;
    }
    return result;
}

int snd_ctl_get_cnt(void)
{
    struct snd_card *card = snd_card_get();

    return card->last_numid;
}

static int snd_ctl_update_bits_locked(struct snd_codec *codec,
                                      unsigned short reg, unsigned int mask,
                                      unsigned int value)
{
    int change;

    mutex_lock(codec->mutex);
    change = dpm_reg_update_bits(codec, reg, mask, value);
    mutex_unlock(codec->mutex);

    return change;
}

struct snd_kcontrol *snd_ctl_cnew(const struct snd_kcontrol_new *_template,
                                  void *data, char *long_name,
                                  const char *prefix)
{
    struct snd_kcontrol_new template;
    struct snd_kcontrol *kcontrol;
    char *name = NULL;
    int name_len;

    memcpy(&template, _template, sizeof(template));
    template.index = 0;

    if (!long_name)
        long_name = template.name;

    if (prefix) {
        name_len = strlen(long_name) + strlen(prefix) + 2;
        name = snd_malloc(name_len);
        if (!name)
            return NULL;

        snprintf(name, name_len, "%s %s", prefix, long_name);

        template.name = name;
    } else {
        template.name = long_name;
    }

    kcontrol = snd_ctl_new(&template, data);

    if(name)
        snd_free(name);

    return kcontrol;
}

static inline bool snd_ctl_volsw_is_stereo(struct soc_mixer_control *mc)
{
    if (mc->reg == mc->rreg && mc->shift == mc->rshift)
        return FALSE;
    /*
     * mc->reg == mc->rreg && mc->shift != mc->rshift, or
     * mc->reg != mc->rreg means that the control is
     * stereo (bits in one register or in two registers)
     */
    return TRUE;
}

int snd_ctl_info_volsw(struct snd_kcontrol *kcontrol,
                       struct snd_ctl_elem_info *uinfo)
{
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    int platform_max;

    if (!mc->platform_max)
        mc->platform_max = mc->max;
    platform_max = mc->platform_max;

    if (platform_max == 1 && !strstr(kcontrol->id.name, " Volume"))
        uinfo->type = SNDRV_CTL_ELEM_TYPE_BOOLEAN;
    else
        uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;

    uinfo->count = snd_ctl_volsw_is_stereo(mc) ? 2 : 1;
    uinfo->value.integer.min = 0;
    uinfo->value.integer.max = platform_max - mc->min;
    return 0;
}

int snd_ctl_info_enum_ext(struct snd_kcontrol *kcontrol,
                          struct snd_ctl_elem_info *uinfo)
{
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;

    uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
    uinfo->count = 1;
    uinfo->value.enumerated.items = e->max;

    if (uinfo->value.enumerated.item > e->max - 1)
        uinfo->value.enumerated.item = e->max - 1;

    /* note: we change it to be a pointer */
    uinfo->value.enumerated.name =
        (char *)e->texts[uinfo->value.enumerated.item];
    /**
    strcpy(uinfo->value.enumerated.name,
        e->texts[uinfo->value.enumerated.item]);
    */

    return 0;
}

int snd_ctl_info_enum_double(struct snd_kcontrol *kcontrol,
                             struct snd_ctl_elem_info *uinfo)
{
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;

    uinfo->type = SNDRV_CTL_ELEM_TYPE_ENUMERATED;
    uinfo->count = e->shift_l == e->shift_r ? 1 : 2;
    uinfo->value.enumerated.items = e->max;

    if (uinfo->value.enumerated.item > e->max - 1)
        uinfo->value.enumerated.item = e->max - 1;

    /* note: we change it to be a pointer */
    uinfo->value.enumerated.name =
        (char *)e->texts[uinfo->value.enumerated.item];
    /*
    strcpy(uinfo->value.enumerated.name,
        e->texts[uinfo->value.enumerated.item]);
    */
    return 0;
}

/**
 * snd_ctl_get_enum_double - enumerated double mixer get callback
 *
 * Callback to get the value of a double enumerated mixer.
 *
 * Returns 0 for success.
 */
int snd_ctl_get_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int val, bitmask;

    for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
        ;
    val = dpm_reg_read(codec, e->reg);
    ucontrol->value.enumerated.item[0]
        = (val >> e->shift_l) & (bitmask - 1);
    if (e->shift_l != e->shift_r)
        ucontrol->value.enumerated.item[1] =
            (val >> e->shift_r) & (bitmask - 1);

    return 0;
}

/**
 * snd_ctl_put_enum_double - enumerated double mixer put callback
 *
 * Callback to set the value of a double enumerated mixer.
 *
 * Returns 0 for success.
 */
int snd_ctl_put_enum_double(struct snd_kcontrol *kcontrol,
                            struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
    unsigned int val;
    unsigned int mask, bitmask;

    for (bitmask = 1; bitmask < e->max; bitmask <<= 1)
        ;
    if (ucontrol->value.enumerated.item[0] > e->max - 1)
        return AIC_EINVAL;
    val = ucontrol->value.enumerated.item[0] << e->shift_l;
    mask = (bitmask - 1) << e->shift_l;
    if (e->shift_l != e->shift_r) {
        if (ucontrol->value.enumerated.item[1] > e->max - 1)
            return AIC_EINVAL;
        val |= ucontrol->value.enumerated.item[1] << e->shift_r;
        mask |= (bitmask - 1) << e->shift_r;
    }

    return snd_ctl_update_bits_locked(codec, e->reg, mask, val);
}

/**
 * snd_ctl_get_volsw - single mixer get callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to get the value of a single mixer control, or a double mixer
 * control that spans 2 registers.
 *
 * Returns 0 for success.
 */
int snd_ctl_get_volsw(struct snd_kcontrol *kcontrol,
                      struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int reg = mc->reg;
    unsigned int reg2 = mc->rreg;
    unsigned int shift = mc->shift;
    unsigned int rshift = mc->rshift;
    int max = mc->max;
    int min = mc->min;
    unsigned int mask = (1 << fls(max)) - 1;
    unsigned int invert = mc->invert;
    int val;

    val = dpm_reg_read(codec, reg);
    val = (val >> shift) & mask;

    ucontrol->value.integer.value[0] = val - min;
    if (invert)
        ucontrol->value.integer.value[0] =
            max - ucontrol->value.integer.value[0];

    if (snd_ctl_volsw_is_stereo(mc)) {
        if (reg == reg2)
            val = dpm_reg_read(codec, reg);
        else
            val = dpm_reg_read(codec, reg2);

        val = (val >> rshift) & mask;

        ucontrol->value.integer.value[1] = val - min;
        if (invert)
            ucontrol->value.integer.value[1] =
                max - ucontrol->value.integer.value[1];
    }

    return 0;
}

/**
 * snd_ctl_put_volsw - single mixer put callback
 * @kcontrol: mixer control
 * @ucontrol: control element information
 *
 * Callback to set the value of a single mixer control, or a double mixer
 * control that spans 2 registers.
 *
 * Returns 0 for success.
 */
int snd_ctl_put_volsw(struct snd_kcontrol *kcontrol,
                       struct snd_ctl_elem_value *ucontrol)
{
    struct snd_codec *codec = snd_kcontrol_chip(kcontrol);
    struct soc_mixer_control *mc =
        (struct soc_mixer_control *)kcontrol->private_value;
    unsigned int reg = mc->reg;
    unsigned int reg2 = mc->rreg;
    unsigned int shift = mc->shift;
    unsigned int rshift = mc->rshift;
    int max = mc->max;
    int min = mc->min;
    unsigned int mask = (1 << fls(max)) - 1;
    unsigned int invert = mc->invert;
    int err;
    bool type_2r = FALSE;
    unsigned int val2 = 0;
    unsigned int val, val_mask;

    val = ((ucontrol->value.integer.value[0] + min) & mask);
    if (invert)
        val = max - val;
    val_mask = mask << shift;
    val = val << shift;
    if (snd_ctl_volsw_is_stereo(mc)) {
        val2 = ((ucontrol->value.integer.value[1] + min) & mask);
        if (invert)
            val2 = max - val2;
        if (reg == reg2) {
            val_mask |= mask << rshift;
            val |= val2 << rshift;
        } else {
            val2 = val2 << shift;
            type_2r = TRUE;
        }
    }
    err = snd_ctl_update_bits_locked(codec, reg, val_mask, val);
    if (err < 0)
        return err;

    if (type_2r)
        err = snd_ctl_update_bits_locked(codec, reg2, val_mask,
            val2);

    return err;
}

