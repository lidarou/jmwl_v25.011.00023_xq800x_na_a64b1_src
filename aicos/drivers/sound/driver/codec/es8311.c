/*
 * es8311.c  --  ES8311/ES8312 ALSA SoC Audio Codec
 *
 * Version 1.0 2021.1.12
 *
 * Copyright (C) 2018 Everest Semiconductor Co., Ltd
 *
 * Authors:  David Yang(yangxiaohua@everest-semi.com)
 *
 *
 * Based on es8374.c by David Yang(yangxiaohua@everest-semi.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x091cd4
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
#include <reg_base.h>
#include <top_glb.h>
#include <limits.h>
#include <i2c.h>
#include <pmu_user.h>
#include <timer_inc.h>
#include <mha.h>
#include <audio/snd_config.h>

#include "snd_card.h"
#include "snd_dpm_driver.h"
#include "snd_ext_pa.h"

#include "es8311.h"
#include <pmic_io.h>

#define SND_SOC_NOPM                    -1

#define snd_soc_component_write         dpm_reg_write
#define snd_soc_component_update_bits   dpm_reg_update_bits
#define snd_soc_component_read32        dpm_reg_read

#define snd_soc_component_get_drvdata   snd_card_get_codec_data
#define snd_soc_component_set_drvdata   snd_card_set_codec_data

#define printk  SND_INFO
#define dev_err SND_ERROR
#define pr_err  SND_ERROR

#define devm_kzalloc    snd_zalloc
#define kfree           snd_free
#define msleep          aos_sleep

enum snd_soc_bias_level {
    SND_SOC_BIAS_OFF = 0,
    SND_SOC_BIAS_STANDBY = 1,
    SND_SOC_BIAS_PREPARE = 2,
    SND_SOC_BIAS_ON = 3,
};

#define CHECK_CLIENT(c)     \
    if (!c->data)   \
        return AIC_EPERM;

#define CHECK_REG(r)    \
        if (r > ES8311_MAX_REGISTER) {  \
            alog_error("wrong codec reg = 0x%x\n", r);    \
            return 0;           \
        }

struct sp_config {
    u8 spc, mmcc, spfs;
    u32 srate;
    u8 lrcdiv;
    u8 sclkdiv;
};

/* codec private data */

struct    es8311_private {
    struct snd_codec *codec;
    struct i2c_client client;
    u32 mclk;
    struct pmu_user *vmicbias;
    struct pmu_user *power;
    bool ext_power_on;
    bool is_low_domain;
    bool sclkinv;
    bool mclkinv;
    bool dmic_enable;
    bool is_dac_on;
    bool is_bias_on;
    u8 adc_volume;
    u8 dac_volume;
};

struct es8311_private *es8311_data;
struct snd_codec *es8311_codec;

static int amic_event(struct snd_dpm_widget *w,
                      struct snd_kcontrol *kcontrol, int event);
static int dac_event(struct snd_dpm_widget *w,
                      struct snd_kcontrol *kcontrol, int event);

static const DECLARE_TLV_DB_SCALE(vdac_tlv,
                -9550, 50, 1);
static const DECLARE_TLV_DB_SCALE(vadc_tlv,
                -9550, 50, 1);
static const DECLARE_TLV_DB_SCALE(mic_pga_tlv,
                0, 300, 1);
static const DECLARE_TLV_DB_SCALE(adc_scale_tlv,
                0, 600, 0);
static const DECLARE_TLV_DB_SCALE(alc_winsize_tlv,
                0, 25, 0);
static const DECLARE_TLV_DB_SCALE(alc_maxlevel_tlv,
                -3600, 200, 0);
static const DECLARE_TLV_DB_SCALE(alc_minlevel_tlv,
                -3600, 200, 0);
static const DECLARE_TLV_DB_SCALE(alc_noisegate_tlv,
                -9600, 600, 0);
static const DECLARE_TLV_DB_SCALE(alc_noisegate_winsize_tlv,
                2048, 2048, 0);
static const DECLARE_TLV_DB_SCALE(alc_automute_gain_tlv,
                0, -400, 0);
static const DECLARE_TLV_DB_SCALE(adc_ramprate_tlv,
                0, 25, 0);

static const char * const dmic_type_txt[] = {
    "dmic at high level",
    "dmic at low level"
};
static const struct soc_enum dmic_type =
SOC_ENUM_SINGLE(ES8311_ADC_REG15, 0, 1, dmic_type_txt);

static const char * const automute_type_txt[] = {
    "automute disabled",
    "automute enable"
};
static const struct soc_enum alc_automute_type =
SOC_ENUM_SINGLE(ES8311_ADC_REG18, 6, 1, automute_type_txt);

static const char * const dacdsm_mute_type_txt[] = {
    "mute to 8",
    "mute to 7/9"
};
static const struct soc_enum dacdsm_mute_type =
SOC_ENUM_SINGLE(ES8311_DAC_REG31, 7, 1, dacdsm_mute_type_txt);

static const char * const aec_type_txt[] = {
    "adc left, adc right",
    "adc left, null right",
    "null left, adc right",
    "null left, null right",
    "dac left, adc right",
    "adc left, dac right",
    "dac left, dac right",
    "N/A"
};
static const struct soc_enum aec_type =
SOC_ENUM_SINGLE(ES8311_GPIO_REG44, 4, 7, aec_type_txt);

static const char * const adc2dac_sel_txt[] = {
    "disable",
    "adc data to dac",
};
static const struct soc_enum adc2dac_sel =
SOC_ENUM_SINGLE(ES8311_GPIO_REG44, 7, 1, adc2dac_sel_txt);

static const char * const mclk_sel_txt[] = {
    "from mclk pin",
    "from bclk",
};
static const struct soc_enum mclk_src =
SOC_ENUM_SINGLE(ES8311_CLK_MANAGER_REG01, 7, 1, mclk_sel_txt);

/*
 * es8311 Controls
 */
static const struct snd_kcontrol_new es8311_snd_controls[] = {
    SOC_SINGLE_TLV("MIC PGA GAIN", ES8311_SYSTEM_REG14,
            0, 10, 0, mic_pga_tlv),
    SOC_SINGLE_TLV("ADC SCALE", ES8311_ADC_REG16,
            0, 7, 0, adc_scale_tlv),
    SOC_ENUM("DMIC TYPE", dmic_type),
    SOC_SINGLE_TLV("ADC RAMP RATE", ES8311_ADC_REG15,
            4, 15, 0, adc_ramprate_tlv),
    SOC_SINGLE("ADC SDP MUTE", ES8311_SDPOUT_REG0A, 6, 1, 1),
    SOC_SINGLE("ADC INVERTED", ES8311_ADC_REG16, 4, 1, 0),
    SOC_SINGLE("ADC SYNC", ES8311_ADC_REG16, 5, 1, 1),
    SOC_SINGLE("ADC RAM CLR", ES8311_ADC_REG16, 3, 1, 0),
    SOC_SINGLE_TLV("ADC VOLUME", ES8311_ADC_REG17,
            0, 255, 0, vadc_tlv),
    SOC_SINGLE("ALC ENABLE", ES8311_ADC_REG18, 7, 1, 0),
    SOC_ENUM("ALC AUTOMUTE TYPE", alc_automute_type),
    SOC_SINGLE_TLV("ALC WIN SIZE", ES8311_ADC_REG18,
            0, 15, 0, alc_winsize_tlv),
    SOC_SINGLE_TLV("ALC MAX LEVEL", ES8311_ADC_REG19,
            4, 15, 0, alc_maxlevel_tlv),
    SOC_SINGLE_TLV("ALC MIN LEVEL", ES8311_ADC_REG19,
            0, 15, 0, alc_minlevel_tlv),
    SOC_SINGLE_TLV("ALC AUTOMUTE WINSIZE", ES8311_ADC_REG1A,
            4, 15, 0, alc_noisegate_winsize_tlv),
    SOC_SINGLE_TLV("ALC AUTOMUTE GATE THRESHOLD", ES8311_ADC_REG1A,
            0, 15, 0, alc_noisegate_tlv),
    SOC_SINGLE_TLV("ALC AUTOMUTE VOLUME", ES8311_ADC_REG1B,
            5, 7, 0, alc_automute_gain_tlv),
    SOC_SINGLE("ADC FS MODE", ES8311_CLK_MANAGER_REG03, 6, 1, 0),
    SOC_SINGLE("ADC OSR", ES8311_CLK_MANAGER_REG03, 0, 63, 0),
    SOC_SINGLE("DAC SDP MUTE", ES8311_SDPIN_REG09, 6, 1, 0),
    SOC_SINGLE("DAC DEM  MUTE", ES8311_DAC_REG31, 5, 1, 0),
    SOC_SINGLE("DAC INVERT", ES8311_DAC_REG31, 4, 1, 0),
    SOC_SINGLE("DAC RAM CLR", ES8311_DAC_REG31, 3, 1, 0),
    SOC_ENUM("DAC DSM MUTE", dacdsm_mute_type),
    SOC_SINGLE("DAC OFFSET", ES8311_DAC_REG33, 0, 255, 0),
    SOC_SINGLE_TLV("DAC VOLUME", ES8311_DAC_REG32,
            0, 255, 0, vdac_tlv),
    SOC_SINGLE("DRC ENABLE", ES8311_DAC_REG34, 7, 1, 0),
    SOC_SINGLE_TLV("DRC WIN SIZE",    ES8311_DAC_REG34,
            0, 15, 0, alc_winsize_tlv),
    SOC_SINGLE_TLV("DRC MAX LEVEL",    ES8311_DAC_REG35,
            4, 15, 0, alc_maxlevel_tlv),
    SOC_SINGLE_TLV("DRC MIN LEVEL",    ES8311_DAC_REG35,
            0, 15, 0, alc_minlevel_tlv),
    SOC_SINGLE_TLV("DAC RAMP RATE",    ES8311_DAC_REG37,
            4, 15, 0, adc_ramprate_tlv),
    SOC_SINGLE("DAC OSR", ES8311_CLK_MANAGER_REG04, 0, 127, 0),
    SOC_ENUM("AEC MODE", aec_type),
    SOC_ENUM("ADC DATA TO DAC TEST MODE", adc2dac_sel),
    SOC_SINGLE("MCLK INVERT", ES8311_CLK_MANAGER_REG01, 6, 1, 0),
    SOC_SINGLE("BCLK INVERT", ES8311_CLK_MANAGER_REG06, 5, 1, 0),
    SOC_ENUM("MCLK SOURCE", mclk_src),
};

/*
 * DAPM Controls
 */
static const char * const es8311_dmic_mux_txt[] = {
    "DMIC DISABLE",
    "DMIC ENABLE"
};
static const unsigned int es8311_dmic_mux_values[] = {
    0, 1
};
static const struct soc_enum es8311_dmic_mux_enum =
    SOC_VALUE_ENUM_SINGLE(ES8311_SYSTEM_REG14, 6, 1,
        ARRAY_SIZE(es8311_dmic_mux_txt),
        es8311_dmic_mux_txt,
        es8311_dmic_mux_values);
static const struct snd_kcontrol_new es8311_dmic_mux_controls =
    CTL_DPM_ENUM("DMIC ROUTE", es8311_dmic_mux_enum);

static const char * const es8311_adc_sdp_mux_txt[] = {

    "FROM EQUALIZER",
    "FROM ADC OUT",
};
static const unsigned int es8311_adc_sdp_mux_values[] = {
    0, 1
};
static const struct soc_enum es8311_adc_sdp_mux_enum =
    SOC_VALUE_ENUM_SINGLE(ES8311_ADC_REG1C, 6, 1,
        ARRAY_SIZE(es8311_adc_sdp_mux_txt),
        es8311_adc_sdp_mux_txt,
        es8311_adc_sdp_mux_values);
static const struct snd_kcontrol_new es8311_adc_sdp_mux_controls =
    CTL_DPM_ENUM("ADC SDP ROUTE", es8311_adc_sdp_mux_enum);

/*
* DAC data  soure
*/
static const char * const es8311_dac_data_mux_txt[] = {
    "SELECT SDP LEFT DATA",
    "SELECT SDP RIGHT DATA",
};
static const unsigned int  es8311_dac_data_mux_values[] = {
    0, 1
};
static const struct soc_enum  es8311_dac_data_mux_enum =
    SOC_VALUE_ENUM_SINGLE(ES8311_SDPIN_REG09, 7, 1,
        ARRAY_SIZE(es8311_dac_data_mux_txt),
        es8311_dac_data_mux_txt,
        es8311_dac_data_mux_values);
static const struct snd_kcontrol_new  es8311_dac_data_mux_controls =
    CTL_DPM_ENUM("DAC SDP ROUTE", es8311_dac_data_mux_enum);

static const struct snd_dpm_widget es8311_dapm_widgets[] = {
    /* Input*/
    SND_DPM_INPUT("DMIC"),
    SND_DPM_MIC("AMIC", amic_event),

    SND_DPM_PGA("INPUT PGA", ES8311_SYSTEM_REG0E,
            6, 1, NULL, 0),
    /* ADCs */
    SND_DPM_ADC("MONO ADC", NULL, ES8311_SYSTEM_REG0E, 5, 1),
    /* Dmic MUX */
    SND_DPM_MUX("DMIC MUX", SND_SOC_NOPM, 0, 0,
            &es8311_dmic_mux_controls),
    /* sdp MUX */
    SND_DPM_MUX("SDP OUT MUX", SND_SOC_NOPM, 0, 0,
            &es8311_adc_sdp_mux_controls),
    /* Digital Interface */
    SND_DPM_AIF_OUT("I2S OUT", "dpm_c0",  1,
            SND_SOC_NOPM, 0, 0),
    SND_DPM_AIF_OUT("I2S OUT COMPR", "compress_ipc_c",  1,
            SND_SOC_NOPM, 0, 0),
    /* Render path    */
    SND_DPM_AIF_IN("I2S IN", "dpm_p0", 0,
            SND_SOC_NOPM, 0, 0),
    SND_DPM_AIF_IN("I2S IN COMPR", "compress_ipc_p", 0,
            SND_SOC_NOPM, 0, 0),
    /*DACs SDP DATA SRC MUX */
    SND_DPM_MUX("DAC SDP SRC MUX", SND_SOC_NOPM, 0, 0,
            &es8311_dac_data_mux_controls),
    SND_DPM_PGA("DAC SDP SRC", SND_SOC_NOPM,
            6, 0, NULL, 0),
    SND_DPM_DAC_E("MONO DAC", NULL, SND_SOC_NOPM, 0, 0, dac_event,
                  SND_DPM_PRE_PMD | SND_DPM_POST_PMU),
    //SND_DPM_DAC("MONO DAC COMPR", "compress_ipc_p", SND_SOC_NOPM, 0, 0),

    /* Output Lines */
    SND_DPM_OUTPUT("DIFFERENTIAL OUT"),

};


static const struct snd_dpm_route es8311_dapm_routes[] = {
    /* record route map */
    {"INPUT PGA", NULL, "AMIC"},
    {"MONO ADC", NULL, "INPUT PGA"},
    {"DMIC MUX", "DMIC DISABLE", "MONO ADC"},
    {"DMIC MUX", "DMIC ENABLE", "DMIC"},
    {"SDP OUT MUX", "FROM ADC OUT", "DMIC MUX"},
    {"SDP OUT MUX", "FROM EQUALIZER", "DMIC MUX"},
    {"I2S OUT", NULL, "SDP OUT MUX"},
    {"I2S OUT COMPR", NULL, "SDP OUT MUX"},

    /* playback route map */
    {"DAC SDP SRC", NULL, "I2S IN"},
    {"DAC SDP SRC", NULL, "I2S IN COMPR"},
    {"DAC SDP SRC MUX", "SELECT SDP LEFT DATA", "DAC SDP SRC"},
    {"DAC SDP SRC MUX", "SELECT SDP RIGHT DATA", "DAC SDP SRC"},
    {"MONO DAC", NULL, "DAC SDP SRC MUX"},
    {"DIFFERENTIAL OUT", NULL, "MONO DAC"},

    /* NOTE: side tone, move to IIS route */
    /*
    {"DAC SDP SRC", NULL, "side_tone_out"},
    {"side_tone_in", NULL, "SDP OUT MUX"},
    */
};

struct _coeff_div {
    u32 mclk;       /* mclk frequency */
    u32 rate;       /* sample rate */
    u8 prediv;      /* the pre divider with range from 1 to 8 */
    u8 premulti;    /* the pre multiplier with x1, x2, x4 and x8 selection */
    u8 adcdiv;      /* adcclk divider */
    u8 dacdiv;      /* dacclk divider */
    u8 fsmode;      /* double speed or single speed, =0, ss, =1, ds */
    u8 lrck_h;      /* adclrck divider and daclrck divider */
    u8 lrck_l;
    u8 bclkdiv;     /* sclk divider */
    u8 adcosr;      /* adc osr */
    u8 dacosr;      /* dac osr */
};


/**
 * codec hifi mclk clock divider coefficients
 * Note:
 *      1. prediv and multi used for internal, not for BCLK and LRCK.
 */
static const struct _coeff_div coeff_div[] = {
    //mclk     rate   prediv  mult  adcdiv dacdiv fsmode lrch  lrcl  bckdiv osr
    /* 8k */
    {24576000, 8000 , 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {12288000, 8000 , 0x06, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 8000 , 0x03, 0x02, 0x03, 0x03, 0x00, 0x05, 0xff, 0x18, 0x10, 0x10},
    {16384000, 8000 , 0x08, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 8000 , 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 8000 , 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 8000 , 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 8000 , 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 8000 , 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 8000 , 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 11.025k */
    {11289600, 11025, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 11025, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 11025, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 11025, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 12k */
    {12288000, 12000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 12000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 12000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 12000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 16k */
    {24576000, 16000, 0x06, 0x02, 0x02, 0x01, 0x00, 0x00, 0xff, 0x07, 0x10, 0x40},
    {12288000, 16000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 16000, 0x03, 0x02, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 16000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 16000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 16000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 16000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 16000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 16000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 16000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1024000 , 16000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 22.05k */
    {11289600, 22050, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 22050, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 22050, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 22050, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 24k */
    {24576000, 24000, 0x04, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {12288000, 24000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 24000, 0x03, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 24000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 24000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 24000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 32k */
    {24576000, 32000, 0x06, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {12288000, 32000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 32000, 0x03, 0x04, 0x03, 0x03, 0x00, 0x02, 0xff, 0x0c, 0x10, 0x10},
    {16384000, 32000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 32000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 32000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {4096000 , 32000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2048000 , 32000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 32000, 0x03, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
    {1024000 , 32000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 44.1k */
    {11289600, 44100, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 44100, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 44100, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 44100, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 48k */
    {24576000, 48000, 0x02, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {12288000, 48000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 48000, 0x03, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 48000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
     {3072000 , 48000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 48000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},

    /* 64k */
    {12288000, 64000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 64000, 0x03, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {16384000, 64000, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {8192000 , 64000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 64000, 0x01, 0x04, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {4096000 , 64000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 64000, 0x01, 0x08, 0x03, 0x03, 0x01, 0x01, 0x7f, 0x06, 0x10, 0x10},
    {2048000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0xbf, 0x03, 0x18, 0x18},
    {1024000 , 64000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 88.2k */
    {11289600, 88200, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {5644800 , 88200, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {2822400 , 88200, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1411200 , 88200, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},

    /* 96k */
    {12288000, 96000, 0x01, 0x02, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {18432000, 96000, 0x03, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {6144000 , 96000, 0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {3072000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x00, 0x00, 0xff, 0x04, 0x10, 0x10},
    {1536000 , 96000, 0x01, 0x08, 0x01, 0x01, 0x01, 0x00, 0x7f, 0x02, 0x10, 0x10},
};

static int i2c_init(struct i2c_client *client)
{
    client->addr = ES8311_I2C_SLAVE_ADDR;
    client->index = SND_CODEC_I2C_ID;

    return i2c_open(client);
}

static int i2c_send_then_read(struct i2c_client *client, u8 *reg ,u8 *buf, int num)
{
    int ret;

    struct i2c_msg msg[2] = {
       {
        .addr = client->addr,
        .flags = 0,
        .len = num,
        .buf = reg,
       }, {
         .addr = client->addr,
        .flags = I2C_M_RD,
        .len = num,
        .buf = buf,
       }
    };

    CHECK_CLIENT(client)
    ret = i2c_transfer(client, &msg[0], 2);
    if (ret < 0)
        aicx_error(0x15bb9c, "d", "codec read failed, ret=%d\n", ret);

    return ret;
}

static int es8311_i2c_write(struct i2c_client *client,
                            unsigned int reg, unsigned int value)
{
    u8 buf[2];
    int ret;

    aicx_debug(0x0ee12b, "xx", "8311: r=0x%x, v=0x%x\n",
               reg, value);

    CHECK_CLIENT(client)
    buf[0] = reg;
    buf[1] = value;

    ret = i2c_master_send(client, buf, 2);
    if (ret < 0) {
        aicx_error(0x068418, "xxd", "i2c_write failed, reg=0x%02x, value=0x%02x, ret=%d\n",
                   reg, value, ret);
        return ret;
    }

    return 0;
}

static unsigned int es8311_i2c_read(struct i2c_client *client, unsigned int reg)
{
    u8 buf[2];
    u8 reg_buf[2];
    int ret;

    CHECK_CLIENT(client)

    reg_buf[0] = reg;
    buf[0] = 0;
    ret = i2c_send_then_read(client, reg_buf, buf, 1);
    if (ret < 0) {
        aicx_error(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);
        return ret;
    }

    return (u32)buf[0];
}

static u32 es8311_read(struct snd_codec *codec, u32 reg)
{
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    CHECK_REG(reg)

    return es8311_i2c_read(&es8311->client, reg);
}

static int es8311_write(struct snd_codec *codec, u32 reg, u32 val)
{
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    CHECK_REG(reg)

    return es8311_i2c_write(&es8311->client, reg, val);
}

static inline int get_coeff(int mclk, int rate)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(coeff_div); i++) {
        if (coeff_div[i].rate == rate && coeff_div[i].mclk == mclk)
            return i;
    }

    return AIC_EINVAL;
}

static void es8311_ext_power(BOOL on)
{
    if (!es8311_data || !es8311_data->power)
        return;

    if (on) {
        if (!es8311_data->ext_power_on) {
            pmu_enable(es8311_data->power);
            es8311_data->ext_power_on = TRUE;
            printk("ext power on\n");
            aic_mdelay(2);
        }
    } else {
        if (es8311_data->ext_power_on) {
            pmu_disable(es8311_data->power);
            es8311_data->ext_power_on = FALSE;
        }
    }
}
/*
 * if PLL not be used, use internal clk1 for mclk,otherwise, use internal clk2 for PLL source.
 */
static int es8311_set_dai_sysclk(struct snd_codec *codec, int clk_id,
                                 unsigned int freq, int dir)
{
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    printk("mckl=%d\n", freq);
    switch (freq) {
    case 11289600:
    case 22579200:
    case 12288000:
    case 16384000:
    case 18432000:
    case 24576000:
        es8311->mclk = freq;
        break;
    default:
        es8311->mclk = SND_CODEC_MCLK;
        break;
    }

    return 0;
}

static int es8311_set_dai_fmt(struct snd_codec *codec, unsigned int fmt)
{
    u8 iface = 0;
    u8 adciface = 0;
    u8 daciface = 0;

    printk("Enter into %s()\n", __func__);
    //iface    = snd_soc_component_read32(codec, ES8311_RESET_REG00);
    adciface = snd_soc_component_read32(codec, ES8311_SDPOUT_REG0A);
    daciface = snd_soc_component_read32(codec, ES8311_SDPIN_REG09);

    /* set master/slave audio interface */
    switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
    case SND_SOC_DAIFMT_CBM_CFM:    /* MASTER MODE */
        printk("ES8311 in Master mode\n");
        iface |= 0x40;
        break;
    case SND_SOC_DAIFMT_CBS_CFS:    /* SLAVE MODE */
        printk("ES8311 in Slave mode\n");
        iface &= 0xBF;
        break;
    default:
        return AIC_EINVAL;
    }
    snd_soc_component_write(codec, ES8311_RESET_REG00, iface | 0x80);


    /* interface format */
    switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
    case SND_SOC_DAIFMT_I2S:
        printk("ES8311 in I2S Format\n");
        adciface &= 0xFC;
        daciface &= 0xFC;
        break;
    case SND_SOC_DAIFMT_RIGHT_J:
        return AIC_EINVAL;
    case SND_SOC_DAIFMT_LEFT_J:
        printk("ES8311 in LJ Format\n");
        adciface &= 0xFC;
        daciface &= 0xFC;
        adciface |= 0x01;
        daciface |= 0x01;
        break;
    case SND_SOC_DAIFMT_DSP_A:
        printk("ES8311 in DSP-A Format\n");
        adciface &= 0xDC;
        daciface &= 0xDC;
        adciface |= 0x03;
        daciface |= 0x03;
        break;
    case SND_SOC_DAIFMT_DSP_B:
        printk("ES8311 in DSP-B Format\n");
        adciface &= 0xDC;
        daciface &= 0xDC;
        adciface |= 0x23;
        daciface |= 0x23;
        break;
    default:
        return AIC_EINVAL;
    }

    iface    = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG06);
    /* clock inversion */
    if (((fmt & SND_SOC_DAIFMT_FORMAT_MASK)==SND_SOC_DAIFMT_I2S) ||
        ((fmt & SND_SOC_DAIFMT_FORMAT_MASK)==SND_SOC_DAIFMT_LEFT_J)) {

        switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
        case SND_SOC_DAIFMT_NB_NF:
            iface    &= 0xDF;
            adciface &= 0xDF;
            daciface &= 0xDF;
            break;
        case SND_SOC_DAIFMT_IB_IF:
            iface    |= 0x20;
            adciface |= 0x20;
            daciface |= 0x20;
            break;
        case SND_SOC_DAIFMT_IB_NF:
            iface    |= 0x20;
            adciface &= 0xDF;
            daciface &= 0xDF;
            break;
        case SND_SOC_DAIFMT_NB_IF:
            iface    &= 0xDF;
            adciface |= 0x20;
            daciface |= 0x20;
            break;
        default:
            AIC_ASSERT(0);
        }
    }

    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG06, iface);
    snd_soc_component_write(codec, ES8311_SDPOUT_REG0A, adciface);
    snd_soc_component_write(codec, ES8311_SDPIN_REG09, daciface);

    return 0;
}

static int es8311_set_params(struct snd_dpm_context *dpm,
                             u32 wid, u32 chn, u32 srate)
{
    struct snd_codec *codec = dpm->codec;
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);
    u16 iface;
    int coeff;
    u8 regv, datmp;

    printk("Enter into %s(), wid=%d, chn=%d, srate=%d\n",
           __func__, wid, chn, srate);

    es8311_ext_power(TRUE);
    es8311_set_dai_sysclk(codec, 0, SND_CODEC_MCLK, 0);

    /**
     *  Now codec must be I2S standard mode,
     *  MCU side IIS work in standard mode with LR pack.
     */
    es8311_set_dai_fmt(codec, ES8311_MS_MODE | ES8311_IIS_MODE | ES8311_INV_CFG);

    coeff = get_coeff(es8311->mclk, srate);
    if (coeff < 0) {
        printk("Unable to configure sample rate %dHz with %dHz MCLK\n",
               srate, es8311->mclk);
        return coeff;
    }
    /*
    * set clock parammeters
    */
    if(coeff >= 0) {
        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG02) & 0x07;
        regv |= (coeff_div[coeff].prediv - 1) << 5;
        datmp = 0;
        switch(coeff_div[coeff].premulti)
        {
        case 1:
            datmp = 0;
            break;
        case 2:
            datmp = 1;
            break;
        case 4:
            datmp = 2;
            break;
        case 8:
            datmp = 3;
            break;
        default:
            break;
        }
        regv |= (datmp) << 3;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG05) & 0x00;
        regv |= (coeff_div[coeff].adcdiv -1)<< 4;
        regv |= (coeff_div[coeff].dacdiv -1)<< 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG05, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG03) & 0x80;
        regv |= coeff_div[coeff].fsmode << 6;
        regv |= coeff_div[coeff].adcosr << 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG03, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG04) & 0x80;
        regv |= coeff_div[coeff].dacosr << 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG04, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG07) & 0xf0;
        regv |= coeff_div[coeff].lrck_h << 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG07, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG08) & 0x00;
        regv |= coeff_div[coeff].lrck_l << 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG08, regv);

        regv = snd_soc_component_read32(codec, ES8311_CLK_MANAGER_REG06) & 0xE0;
        regv |= coeff_div[coeff].bclkdiv << 0;
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG06, regv);
    }

    /* set playback */
    {
        iface = snd_soc_component_read32(codec, ES8311_SDPIN_REG09) & 0xE3;
        iface |= 0x0c;
        /* set iface */
        snd_soc_component_write(codec, ES8311_SDPIN_REG09, iface);
    }
    /* set capture */
    {
        iface = snd_soc_component_read32(codec, ES8311_SDPOUT_REG0A) & 0xE3;
        iface |= 0x0c;
        /* set iface */
        snd_soc_component_write(codec, ES8311_SDPOUT_REG0A, iface);
    }

    return 0;
}

static int es8311_set_bias_level(struct snd_codec *codec,
            enum snd_bias_level level)
{
    int regv;
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    printk("Enter into %s(), level = %d\n", __func__, level);
    switch (level) {
    case SND_SOC_BIAS_ON:
        es8311_ext_power(TRUE);

        snd_soc_component_write(codec, ES8311_GP_REG45, 0x00);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x30);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, 0x00);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG03, 0x10);
        snd_soc_component_write(codec, ES8311_ADC_REG16, 0x24);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG04, 0x10);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG05, 0x00);

        snd_soc_component_write(codec, ES8311_SYSTEM_REG0B, 0x00);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG0C, 0x00);
        if (es8311->is_low_domain)
            snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x1F + (3 << 5));
        else
            snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x1F);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG11, 0x7F);
        snd_soc_component_write(codec, ES8311_RESET_REG00, 0x80);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG0D, 0x01);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x3F);
        if(es8311->mclkinv == true) {
            snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                    0x40, 0x40);
        }
        else {
            snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                    0x40, 0x00);
        }
        if(es8311->sclkinv == true) {
            snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                    0x20, 0x20);
        }
        else {
            snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                    0x20, 0x00);
        }
        regv = snd_soc_component_read32(codec, ES8311_SYSTEM_REG14) & 0xCF;
        regv |= 0x1A;
        snd_soc_component_write(codec, ES8311_SYSTEM_REG14, regv);
        if(es8311->dmic_enable == true) {
            snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                    0x40, 0x40);
        }
        else {
            snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                    0x40, 0x00);
        }
        snd_soc_component_write(codec, ES8311_SYSTEM_REG13, 0x10);
        snd_soc_component_write(codec, ES8311_SDPIN_REG09, 0x00);
        snd_soc_component_write(codec, ES8311_SDPOUT_REG0A, 0x00);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG0E, 0x02);
        snd_soc_component_write(codec, ES8311_ADC_REG15, 0x40);
        snd_soc_component_write(codec, ES8311_ADC_REG1B, 0x0A);
        snd_soc_component_write(codec, ES8311_ADC_REG1C, 0x6A);
        snd_soc_component_write(codec, ES8311_DAC_REG37, 0x48);
        snd_soc_component_write(codec, ES8311_ADC_REG17, es8311->adc_volume);
        snd_soc_component_write(codec, ES8311_DAC_REG32, es8311->dac_volume);
        /* open DAC */
        snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x00);
        if (es8311->is_dac_on)
            snd_ext_pa_on();
        es8311->is_bias_on = TRUE;
        break;
    case SND_SOC_BIAS_PREPARE:
        break;
    case SND_SOC_BIAS_STANDBY:
        snd_ext_pa_off();
        snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x02);
        snd_soc_component_write(codec, ES8311_DAC_REG32, 0x00);
        snd_soc_component_write(codec, ES8311_ADC_REG17, 0x00);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG0E, 0xFF);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG0D, 0xFC);
        snd_soc_component_write(codec, ES8311_ADC_REG15, 0x00);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, 0x10);
        snd_soc_component_write(codec, ES8311_DAC_REG37, 0x08);
        snd_soc_component_write(codec, ES8311_RESET_REG00, 0x00);
        snd_soc_component_write(codec, ES8311_RESET_REG00, 0x1F);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x30);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x00);
        snd_soc_component_write(codec, ES8311_GP_REG45, 0x01);
        snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, 0x10);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x63);

        es8311_ext_power(FALSE);
        es8311->is_bias_on = FALSE;

        break;
    case SND_SOC_BIAS_OFF:
        break;
    }

    printk("%s(), exit\n", __func__);

    return 0;
}

int es8311_set_tristate(int tristate)
{
    struct snd_codec *codec = es8311_codec;

    printk("Enter into %s(), tristate = %d\n", __func__, tristate);
    if(tristate) {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG07,
            0x30, 0x30);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG07,
            0x30, 0x00);
    }

    return 0;
}

int es8311_mute(int mute)
{
    struct snd_codec *codec = es8311_codec;

    printk("Enter into %s(), mute = %d\n", __func__, mute);
    if (mute) {
        snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x02);
        snd_soc_component_update_bits(codec, ES8311_DAC_REG31, 0x60, 0x60);
        snd_soc_component_write(codec, ES8311_DAC_REG32, 0x00);
        snd_soc_component_write(codec, ES8311_DAC_REG37, 0x08);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_DAC_REG31, 0x60, 0x00);
        snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x00);
    }

    return 0;
}

static int amic_event(struct snd_dpm_widget *w,
                      struct snd_kcontrol *kcontrol, int event)
{
    struct snd_codec *codec = w->codec;
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    if (!es8311->vmicbias)
        return 0;

    switch (event) {
    case SND_DPM_PRE_PMU:
        pmu_enable(es8311->vmicbias);
        printk("amic vmicbias on!\n");
        if (es8311->is_low_domain)
            aos_sleep(20);

        break;
    case SND_DPM_POST_PMD:
        pmu_disable(es8311->vmicbias);
        printk("amic vmicbias off!\n");
        break;
    default:
        printk("amic unknown event=%d\n", event);
        break;
    }

    return 0;
}

static int dac_event(struct snd_dpm_widget *w,
                      struct snd_kcontrol *kcontrol, int event)
{
    struct snd_codec *codec = w->codec;
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    switch (event) {
    case SND_DPM_POST_PMU:
        es8311->is_dac_on = TRUE;
        if (es8311->is_bias_on)
            snd_ext_pa_on();
        break;
    case SND_DPM_PRE_PMD:
        es8311->is_dac_on = FALSE;
        break;
    default:
        printk("dac unknown event=%d\n", event);
        break;
    }

    return 0;
}

static int es8311_suspend(struct snd_codec *codec)
{
    printk("Enter into %s()\n", __func__);
    snd_soc_component_write(codec, ES8311_DAC_REG32, 0x00);
    snd_soc_component_write(codec, ES8311_ADC_REG17, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0E, 0xFF);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x02);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG14, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0D, 0xFA);
    snd_soc_component_write(codec, ES8311_ADC_REG15, 0x00);
    snd_soc_component_write(codec, ES8311_DAC_REG37, 0x08);
    snd_soc_component_write(codec, ES8311_RESET_REG00, 0x00);
    snd_soc_component_write(codec, ES8311_RESET_REG00, 0x1F);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x30);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x00);
    snd_soc_component_write(codec, ES8311_GP_REG45, 0x01);

    return 0;
}

static int es8311_resume(struct snd_codec *codec)
{
    struct es8311_private *es8311 =
        (struct es8311_private *)snd_soc_component_get_drvdata(codec);

    printk("Enter into %s()\n", __func__);
    snd_soc_component_write(codec, ES8311_GP_REG45, 0x00);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x30);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, 0x00);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG03, 0x10);
    snd_soc_component_write(codec, ES8311_ADC_REG16, 0x24);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG04, 0x10);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG05, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0B, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0C, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x1F);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG11, 0x7F);
    snd_soc_component_write(codec, ES8311_RESET_REG00, 0x80);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0D, 0x01);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x3F);
    if(es8311->mclkinv == true) {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                0x40, 0x40);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                0x40, 0x00);
    }
    if(es8311->sclkinv == true) {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                0x20, 0x20);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                0x20, 0x00);
    }
    snd_soc_component_write(codec, ES8311_SYSTEM_REG14, 0x1A);
    if(es8311->dmic_enable == true) {
        snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                0x40, 0x40);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                0x40, 0x00);
    }
    snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG13, 0x10);
    snd_soc_component_write(codec, ES8311_SDPIN_REG09, 0x00);
    snd_soc_component_write(codec, ES8311_SDPOUT_REG0A, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0E, 0x02);
    snd_soc_component_write(codec, ES8311_ADC_REG15, 0x40);
    snd_soc_component_write(codec, ES8311_ADC_REG1B, 0x0A);
    snd_soc_component_write(codec, ES8311_ADC_REG1C, 0x6A);
    snd_soc_component_write(codec, ES8311_DAC_REG37, 0x48);
    snd_soc_component_write(codec, ES8311_ADC_REG17, 0xBF);
    snd_soc_component_write(codec, ES8311_DAC_REG32, 0xBF);

    return 0;
}

int es8311_stream_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        es8311_resume(dpm->codec);
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        es8311_suspend(dpm->codec);
        break;
    default:
        break;
    }

    return 0;
}

static int es8311_probe(struct snd_codec *codec)
{
    int ret = 0;
    struct es8311_private *es8311 = es8311_data;

    printk("Enter into %s()\n", __func__);
    snd_soc_component_set_drvdata(codec, (u32)es8311);
    es8311->codec = codec;
    if (codec == NULL) {
        dev_err("Codec device not registered\n");
        return AIC_ENODEV;
    }
    es8311_codec = codec;

    snd_soc_component_write(codec, ES8311_GP_REG45, 0x00);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x30);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG02, 0x00);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG03, 0x10);
    snd_soc_component_write(codec, ES8311_ADC_REG16, 0x24);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG04, 0x10);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG05, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0B, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0C, 0x00);
    if (es8311->is_low_domain)
        snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x1F + (3 << 5));
    else
        snd_soc_component_write(codec, ES8311_SYSTEM_REG10, 0x1F);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG11, 0x7F);
    snd_soc_component_write(codec, ES8311_RESET_REG00, 0x80);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0D, 0x01);
    snd_soc_component_write(codec, ES8311_CLK_MANAGER_REG01, 0x3F);
    if(es8311->mclkinv == true) {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                0x40, 0x40);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG01,
                0x40, 0x00);
    }
    if(es8311->sclkinv == true) {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                0x20, 0x20);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_CLK_MANAGER_REG06,
                0x20, 0x00);
    }
    snd_soc_component_write(codec, ES8311_SYSTEM_REG14, 0x1A);
    if(es8311->dmic_enable == true) {
        snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                0x40, 0x40);
    }
    else {
        snd_soc_component_update_bits(codec, ES8311_SYSTEM_REG14,
                0x40, 0x00);
    }
    snd_soc_component_write(codec, ES8311_SYSTEM_REG12, 0x20);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG13, 0x10);
    snd_soc_component_write(codec, ES8311_SDPIN_REG09, 0x00);
    snd_soc_component_write(codec, ES8311_SDPOUT_REG0A, 0x00);
    snd_soc_component_write(codec, ES8311_SYSTEM_REG0E, 0x02);
    snd_soc_component_write(codec, ES8311_ADC_REG15, 0x40);
    snd_soc_component_write(codec, ES8311_ADC_REG1B, 0x0A);
    snd_soc_component_write(codec, ES8311_ADC_REG1C, 0x6A);
    snd_soc_component_write(codec, ES8311_DAC_REG37, 0x48);
    snd_soc_component_write(codec, ES8311_ADC_REG17, 0xBF);
    snd_soc_component_write(codec, ES8311_DAC_REG32, 0xBF);

    es8311_set_bias_level(codec, SND_SOC_BIAS_STANDBY);
    return ret;
}

static int es8311_remove(struct snd_codec *codec)
{
    return es8311_set_bias_level(codec, SND_SOC_BIAS_OFF);
}

static struct snd_codec_driver soc_component_dev_es8311 = {
    .probe = es8311_probe,
    .remove = es8311_remove,
    .set_bias_level = es8311_set_bias_level,
    .set_params = es8311_set_params,
    .read = es8311_read,
    .write = es8311_write,

    .controls = es8311_snd_controls,
    .num_controls = ARRAY_SIZE(es8311_snd_controls),
    .dpm_widgets = es8311_dapm_widgets,
    .num_dpm_widgets = ARRAY_SIZE(es8311_dapm_widgets),
    .dpm_routes = es8311_dapm_routes,
    .num_dpm_routes = ARRAY_SIZE(es8311_dapm_routes),
};

u32 es8311_show(u32 reg)
{
    u32 i = 0;

    if (reg != (u32)(-1)) {
        CONS_OUT("0x%02x 0x%02x\n", reg, snd_soc_component_read32(es8311_codec, reg));
        return 0;
    }

    for (i = 0; i <= ES8311_ADC_REG1C; i++)
        CONS_OUT("0x%02x 0x%02x\n", i, snd_soc_component_read32(es8311_codec, i));

    for (i = ES8311_DAC_REG31; i <= ES8311_DAC_REG37; i++)
        CONS_OUT("0x%02x 0x%02x\n", i, snd_soc_component_read32(es8311_codec, i));

    CONS_OUT("0x%02x 0x%02x\n", ES8311_GP_REG45,
             snd_soc_component_read32(es8311_codec, ES8311_GP_REG45));

    return 0;
}

u32 es8311_store(u32 reg, u32 val)
{
    return snd_soc_component_write(es8311_codec, reg, val);
}

static int es8311_i2c_probe(void)
{
    struct es8311_private *es8311;
    int ret = -1;

    printk("Enter into %s\n", __func__);
    es8311 = devm_kzalloc(sizeof(*es8311));
    if (es8311 == NULL)
        return AIC_ENOMEM;

    printk("Enter into %s---1\n", __func__);
    es8311->dmic_enable = SND_CODEC_DMIC_EN;     // dmic interface disabled
    /* the edge of lrck is always at the falling edge of mclk */
    es8311->mclkinv = SND_CODEC_MCLK_INV;
    /* the edge of lrck is always at the falling edge of sclk */
    es8311->sclkinv = SND_CODEC_SCLK_INV;
    es8311->adc_volume = SND_CODEC_DEFALT_ADC_GAIN;
    es8311->dac_volume = SND_CODEC_DEFALT_DAC_GAIN;

#ifdef CONFIG_MHA_SOUND
    struct mha_sound *cfg;
    cfg = mha_get_sound_cfg();
    if (!cfg || !cfg->codec_name || strcmp(cfg->codec_name, "ES8311")) {
        printk("Not es8311\n");
        return AIC_EPERM;
    }
    es8311->vmicbias = pmu_get("vmic", (char *)cfg->codec_micbias_name);
    if (es8311->vmicbias && cfg->codec_micbias_volt)
        pmu_voltage_set(es8311->vmicbias, cfg->codec_micbias_volt, 10000);

    es8311->power = pmu_get("es8311 power", (char *)cfg->codec_core_pwr_name);
    if (es8311->power) {
        if (cfg->codec_core_pwr_volt)
            pmu_voltage_set(es8311->power, cfg->codec_core_pwr_volt, 10000);
    }
    es8311->is_low_domain = cfg->reserve & ES8311_VOLT_LOW_DOMAIN;
#else
    es8311->vmicbias = pmu_get("vmic", SND_PMU_CODEC_MICBIAS);
    if (es8311->vmicbias)
        pmu_voltage_set(es8311->vmicbias, 2800000, 10000);

#ifdef CONFIG_SOUND_CODEC_ES8311_1V8
    es8311->is_low_domain = TRUE;
#endif

    es8311->power = pmu_get("es8311 power", SND_PMU_CODEC);
#endif
    if (es8311->power)
        pmu_enable(es8311->power);

    i2c_init(&es8311->client);

    es8311_data = es8311;
    ret =  snd_card_register_codec("es8311", &soc_component_dev_es8311);
    if (es8311->power)
        pmu_disable(es8311->power);

    if (ret < 0) {
        kfree(es8311);
        es8311_data = 0;
    }
    printk("Exit %s\n", __func__);

    return ret;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(es8311_i2c_probe);

