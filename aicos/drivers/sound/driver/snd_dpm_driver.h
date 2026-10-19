/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines many MACROs, which are used by drivers.
 *    Generrally codec driver use them to define DPM components and controls.
 */

#ifndef __SND_DPM_DRIVER_H__
#define __SND_DPM_DRIVER_H__

#include "snd_dpm.h"
#include "snd_ctl.h"

/* widget has no PM register bit */
#define SND_REG_NONE    -1

/* codec domain */
#define SND_DPM_VMID(wname) \
{   .id = snd_dpm_vmid, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0}

/* platform domain */
#define SND_DPM_SIGGEN(wname) \
{   .id = snd_dpm_siggen, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE }
#define SND_DPM_INPUT(wname) \
{   .id = snd_dpm_input, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE }
#define SND_DPM_OUTPUT(wname) \
{   .id = snd_dpm_output, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE }
#define SND_DPM_MIC(wname, wevent) \
{   .id = snd_dpm_mic, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_PRE_PMU | SND_DPM_POST_PMD}
#define SND_DPM_HP(wname, wevent) \
{   .id = snd_dpm_hp, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_POST_PMU | SND_DPM_PRE_PMD}
#define SND_DPM_SPK(wname, wevent) \
{   .id = snd_dpm_spk, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_POST_PMU | SND_DPM_PRE_PMD}
#define SND_DPM_LINE(wname, wevent) \
{   .id = snd_dpm_line, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_POST_PMU | SND_DPM_PRE_PMD}

/* path domain */
#define SND_DPM_PGA(wname, wreg, wshift, winvert,\
     wcontrols, wncontrols) \
{   .id = snd_dpm_pga, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols}
#define SND_DPM_OUT_DRV(wname, wreg, wshift, winvert,\
     wcontrols, wncontrols) \
{   .id = snd_dpm_out_drv, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols}
#define SND_DPM_MIXER(wname, wreg, wshift, winvert, \
     wcontrols, wncontrols)\
{   .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols}
#define SND_DPM_MIXER_NAMED_CTL(wname, wreg, wshift, winvert, \
     wcontrols, wncontrols)\
{   .id = snd_dpm_mixer_named_ctl, .name = wname, .reg = wreg, \
    .shift = wshift, .invert = winvert, .kcontrol_news = wcontrols, \
    .num_kcontrols = wncontrols}
#define SND_DPM_MICBIAS(wname, wreg, wshift, winvert) \
{   .id = snd_dpm_micbias, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = NULL, .num_kcontrols = 0}
#define SND_DPM_SWITCH(wname, wreg, wshift, winvert, wcontrols) \
{   .id = snd_dpm_switch, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1}
#define SND_DPM_MUX(wname, wreg, wshift, winvert, wcontrols) \
{   .id = snd_dpm_mux, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1}
#define SND_DPM_VIRT_MUX(wname, wreg, wshift, winvert, wcontrols) \
{   .id = snd_dpm_virt_mux, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1}
#define SND_DPM_VALUE_MUX(wname, wreg, wshift, winvert, wcontrols) \
{   .id = snd_dpm_value_mux, .name = wname, .reg = wreg, \
    .shift = wshift, .invert = winvert, .kcontrol_news = wcontrols, \
    .num_kcontrols = 1}

/* Simplified versions of above macros, assuming wncontrols = ARRAY_SIZE(wcontrols) */
#define SND_DPM_PGA_ARRAY(wname, wreg, wshift, winvert,\
     wcontrols) \
{   .id = snd_dpm_pga, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = ARRAY_SIZE(wcontrols)}
#define SND_DPM_MIXER_ARRAY(wname, wreg, wshift, winvert, \
     wcontrols)\
{   .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = ARRAY_SIZE(wcontrols)}
#define SND_DPM_MIXER_NAMED_CTL_ARRAY(wname, wreg, wshift, winvert, \
     wcontrols)\
{   .id = snd_dpm_mixer_named_ctl, .name = wname, .reg = wreg, \
    .shift = wshift, .invert = winvert, .kcontrol_news = wcontrols, \
    .num_kcontrols = ARRAY_SIZE(wcontrols)}

/* path domain with event - event handler must return 0 for success */
#define SND_DPM_PGA_E(wname, wreg, wshift, winvert, wcontrols, \
    wncontrols, wevent, wflags) \
{   .id = snd_dpm_pga, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_OUT_DRV_E(wname, wreg, wshift, winvert, wcontrols, \
    wncontrols, wevent, wflags) \
{   .id = snd_dpm_out_drv, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_MIXER_E(wname, wreg, wshift, winvert, wcontrols, \
    wncontrols, wevent, wflags) \
{    .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = wncontrols, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_MIXER_NAMED_CTL_E(wname, wreg, wshift, winvert, \
    wcontrols, wncontrols, wevent, wflags) \
{   .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, \
    .num_kcontrols = wncontrols, .event = wevent, .event_flags = wflags}
#define SND_DPM_MICBIAS_E(wname, wreg, wshift, winvert, wevent, wflags) \
{   .id = snd_dpm_micbias, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = NULL, .num_kcontrols = 0, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_SWITCH_E(wname, wreg, wshift, winvert, wcontrols, \
    wevent, wflags) \
{    .id = snd_dpm_switch, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_MUX_E(wname, wreg, wshift, winvert, wcontrols, \
    wevent, wflags) \
{   .id = snd_dpm_mux, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_VIRT_MUX_E(wname, wreg, wshift, winvert, wcontrols, \
    wevent, wflags) \
{    .id = snd_dpm_virt_mux, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = 1, \
    .event = wevent, .event_flags = wflags}

/* additional sequencing control within an event type */
#define SND_DPM_PGA_S(wname, wsubseq, wreg, wshift, winvert, \
    wevent, wflags) \
{   .id = snd_dpm_pga, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .event = wevent, .event_flags = wflags, \
    .subseq = wsubseq}
#define SND_DPM_SUPPLY_S(wname, wsubseq, wreg, wshift, winvert, wevent, \
    wflags)    \
{   .id = snd_dpm_supply, .name = wname, .reg = wreg,    \
    .shift = wshift, .invert = winvert, .event = wevent, \
    .event_flags = wflags, .subseq = wsubseq}

/* Simplified versions of above macros, assuming wncontrols = ARRAY_SIZE(wcontrols) */
#define SND_DPM_PGA_E_ARRAY(wname, wreg, wshift, winvert, wcontrols, \
    wevent, wflags) \
{   .id = snd_dpm_pga, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = ARRAY_SIZE(wcontrols), \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_MIXER_E_ARRAY(wname, wreg, wshift, winvert, wcontrols, \
    wevent, wflags) \
{   .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, .num_kcontrols = ARRAY_SIZE(wcontrols), \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_MIXER_NAMED_CTL_E_ARRAY(wname, wreg, wshift, winvert, \
    wcontrols, wevent, wflags) \
{   .id = snd_dpm_mixer, .name = wname, .reg = wreg, .shift = wshift, \
    .invert = winvert, .kcontrol_news = wcontrols, \
    .num_kcontrols = ARRAY_SIZE(wcontrols), .event = wevent, .event_flags = wflags}

/* events that are pre and post DAPM */
#define SND_DPM_PRE(wname, wevent) \
{   .id = snd_dpm_pre, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_PRE_PMU | SND_DPM_PRE_PMD}
#define SND_DPM_POST(wname, wevent) \
{   .id = snd_dpm_post, .name = wname, .kcontrol_news = NULL, \
    .num_kcontrols = 0, .reg = SND_REG_NONE, .event = wevent, \
    .event_flags = SND_DPM_POST_PMU | SND_DPM_POST_PMD}

/* stream domain */
#define SND_DPM_AIF_IN(wname, stname, wslot, wreg, wshift, winvert) \
{   .id = snd_dpm_aif_in, .name = wname, .sname = stname, \
    .reg = wreg, .shift = wshift, .invert = winvert }
#define SND_DPM_AIF_IN_E(wname, stname, wslot, wreg, wshift, winvert, \
                         wevent, wflags)                \
{   .id = snd_dpm_aif_in, .name = wname, .sname = stname, \
    .reg = wreg, .shift = wshift, .invert = winvert, \
    .event = wevent, .event_flags = wflags }
#define SND_DPM_AIF_OUT(wname, stname, wslot, wreg, wshift, winvert) \
{   .id = snd_dpm_aif_out, .name = wname, .sname = stname, \
    .reg = wreg, .shift = wshift, .invert = winvert }
#define SND_DPM_AIF_OUT_E(wname, stname, wslot, wreg, wshift, winvert, \
                          wevent, wflags)                \
{   .id = snd_dpm_aif_out, .name = wname, .sname = stname, \
    .reg = wreg, .shift = wshift, .invert = winvert, \
    .event = wevent, .event_flags = wflags }
#define SND_DPM_DAC(wname, stname, wreg, wshift, winvert) \
{    .id = snd_dpm_dac, .name = wname, .sname = stname, .reg = wreg, \
    .shift = wshift, .invert = winvert}
#define SND_DPM_DAC_E(wname, stname, wreg, wshift, winvert, \
                      wevent, wflags)                \
{   .id = snd_dpm_dac, .name = wname, .sname = stname, .reg = wreg, \
    .shift = wshift, .invert = winvert, \
    .event = wevent, .event_flags = wflags}
#define SND_DPM_ADC(wname, stname, wreg, wshift, winvert) \
{   .id = snd_dpm_adc, .name = wname, .sname = stname, .reg = wreg, \
    .shift = wshift, .invert = winvert}
#define SND_DPM_ADC_E(wname, stname, wreg, wshift, winvert, \
                      wevent, wflags)                \
{   .id = snd_dpm_adc, .name = wname, .sname = stname, .reg = wreg, \
    .shift = wshift, .invert = winvert, \
    .event = wevent, .event_flags = wflags}

/* generic widgets */
#define SND_DPM_REG(wid, wname, wreg, wshift, wmask, won_val, woff_val) \
{   .id = wid, .name = wname, .kcontrol_news = NULL, .num_kcontrols = 0, \
    .reg = -((wreg) + 1), .shift = wshift, .mask = wmask, \
    .on_val = won_val, .off_val = woff_val, .event = dapm_reg_event, \
    .event_flags = SND_DPM_PRE_PMU | SND_DPM_POST_PMD}
#define SND_DPM_SUPPLY(wname, wreg, wshift, winvert, wevent, wflags) \
{   .id = snd_dpm_supply, .name = wname, .reg = wreg,    \
    .shift = wshift, .invert = winvert, .event = wevent, \
    .event_flags = wflags}

/* dapm kcontrol types */
#define CTL_DPM_SINGLE(xname, reg, shift, max, invert) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_volsw, \
    .get = snd_dpm_get_volsw, .put = snd_dpm_put_volsw, \
    .private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }
#define CTL_DPM_SINGLE_TLV(xname, reg, shift, max, invert, tlv_array) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_volsw, \
    .access = SNDRV_CTL_ELEM_ACCESS_TLV_READ | SNDRV_CTL_ELEM_ACCESS_READWRITE,\
    .tlv.p = (tlv_array), \
    .get = snd_dpm_get_volsw, .put = snd_dpm_put_volsw, \
    .private_value =  SOC_SINGLE_VALUE(reg, shift, max, invert) }
#define CTL_DPM_ENUM(xname, xenum) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_enum_double, \
    .get = snd_dpm_get_enum_double, \
    .put = snd_dpm_put_enum_double, \
    .private_value = (unsigned long)&xenum }
#define CTL_DPM_ENUM_VIRT(xname, xenum)            \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_enum_double, \
    .get = snd_dpm_get_enum_virt, \
    .put = snd_dpm_put_enum_virt, \
    .private_value = (unsigned long)&xenum }
#define CTL_DPM_ENUM_EXT(xname, xenum, xget, xput) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_enum_double, \
    .get = xget, \
    .put = xput, \
    .private_value = (unsigned long)&xenum }
#define CTL_DPM_VALUE_ENUM(xname, xenum) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname, \
    .info = snd_ctl_info_enum_double, \
    .get = snd_dpm_get_value_enum_double, \
    .put = snd_dpm_put_value_enum_double, \
    .private_value = (unsigned long)&xenum }
#define CTL_DPM_PIN_SWITCH(xname) \
{   .iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = xname " Switch", \
    .info = snd_dpm_info_pin_switch, \
    .get = snd_dpm_get_pin_switch, \
    .put = snd_dpm_put_pin_switch, \
    .private_value = (unsigned long)xname }

/* dapm events */
int snd_dpm_stream_event(struct snd_codec *codec,
                         const char *stream, int event);
unsigned int dpm_reg_read(struct snd_codec *codec, unsigned int reg);
unsigned int dpm_reg_write(struct snd_codec *codec,
               unsigned int reg, unsigned int val);
int dpm_reg_update_bits(struct snd_codec *codec, unsigned short reg,
                unsigned int mask, unsigned int value);

#endif /* __SND_DPM_DRIVER_H__ */

