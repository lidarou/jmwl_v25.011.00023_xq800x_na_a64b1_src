
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01956c
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
#include "snd_ext_pa.h"
#include "cjc8910.h"

#define REG_MAX     0x86
#define DELAY_MS    aos_sleep

#define CHECK_CLIENT(c)     \
    if (!c->data)   \
        return AIC_EPERM;

#define CHECK_REG(r)    \
        if (r > REG_MAX) {  \
            alog_error("wrong codec reg = 0x%x\n", r);    \
            return 0;           \
        }


#define CURR_SRATE           (cjc8910_data->srate)

#define I2CWRNBYTE_CODEC    CJC8910_WriteReg
/* codec private data */
struct cjc8910_private {
    struct snd_codec *codec;
    struct i2c_client client;
    struct pmu_user *power;
    struct pmu_user *vmicbias;
    u32 ref_cnt;

//    u32 ratio;
    u32 mclk;
    u32 srate;
//    u8 chn;
//    u8 wid;
    u32 adc_volume;
    u32 dac_volume;
};

struct CJC8910_reg{
    unsigned char reg_index;
    unsigned char reg_value;
};


struct cjc8910_private *cjc8910_data;


int CJC8910_WriteReg(unsigned char reg, unsigned char value);
unsigned short CJC8910_ReadReg(unsigned char reg);



static struct CJC8910_reg    cjc8910_dac_to_out[] = {
#if 0
    {CJC8910_R15_RESET,   0x00},   // reset
    {CJC8910_R0_LEFT_INPUT_VOLUME, 0x3F},
    {CJC8910_R2_LOUT1_VOLUME , 0x79},
    {CJC8910_R5_ADC_DAC_CONTROL , 0x00},   // ADC and DAC CONTROL; R/W
    {CJC8910_R7_AUDIO_INTERFACE , 0x0A},   // Digital Audio interface format; R/W
    {CJC8910_R8_SAMPLE_RATE , 0x00},   // Clock and Sample rate control; R/W
#endif
    {CJC8910_R10_LEFT_DAC_VOLUME , 0xC3},   // Left channel DAC digital volume; R/W
    {CJC8910_R12_BASS_CONTROL , 0x0f},   // BASS control; R/W
    {CJC8910_R13_TREBLE_CONTROL , 0x0f},   // Treble control; R/W
    {CJC8910_R17_ALC1_CONTROL , 0x7B},   // ALC1 control; R/W
    {CJC8910_R18_ALC2_CONTROL , 0x00},   // ACL2 control; R/W
    {CJC8910_R19_ALC3_CONTROL , 0x00},   // ALC3 control; R/W
    {CJC8910_R20_NOISE_GATE_CONTROL , 0x00},   // noise gate; R/W
    {CJC8910_R21_LEFT_ADC_VOLUME , 0xc3},   // Left ADC digital volume; R/W
    {CJC8910_R23_ADDITIONAL_CONTROL1 , 0x00},   // Additional control 1; R/W
    {CJC8910_R24_ADDITIONAL_CONTROL2 , 0x00},   // Additional control 2; R/W
    {CJC8910_R27_ADDITIONAL_CONTROL3 , 0x00},   // Additional control 3; R/W
    {CJC8910_R32_ADCL_SIGNAL_PATH , 0x00},   //  ADC signal path control; R/W
    {CJC8910_R33_MIC , 0x0A},   // micR/W   MIX
    {CJC8910_R34_AUX , 0x0A},   // aux R/W   MIX
    {CJC8910_R35_LEFT_OUT_MIX2_H , 0x00},   //  Left out Mix (2) ; R/W
    {CJC8910_R37_ADC_PDN , 0x08},   // Adc_pdn sel; R/W
    {CJC8910_R67_LOW_POWER_PLAYBACK , 0x08},   // Low power playback; R/W
    {CJC8910_R25_PWR_MGMT1_H , 0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8910_R26_PWR_MGMT2_H , 0x40},   // Power management2 and DAC left power down; R/W;
};

static void CJC8910_ADC(void)
{
    int i, val = 0;

    I2CWRNBYTE_CODEC(CJC8910_R15_RESET,   0x00);   // reset

    I2CWRNBYTE_CODEC(CJC8910_R0_LEFT_INPUT_VOLUME , 0x3F);   // Audio input left channel volume; R/W
    I2CWRNBYTE_CODEC(CJC8910_R2_LOUT1_VOLUME , 0x79);   // Audio output letf channel1 volume; R/W
    I2CWRNBYTE_CODEC(CJC8910_R5_ADC_DAC_CONTROL , 0x00);   // ADC and DAC CONTROL; R/W
    I2CWRNBYTE_CODEC(CJC8910_R7_AUDIO_INTERFACE , 0x0A);   // Digital Audio interface format; R/W

    if (cjc8910_data->mclk == 12288000) {
        if (cjc8910_data->srate == 16000)
            val = 0x14;
        else if (cjc8910_data->srate == 48000)
            val = 0x0;
    } else if (cjc8910_data->mclk == 24576000) {
        if (cjc8910_data->srate == 16000)
            val = 0x54;
        else if (cjc8910_data->srate == 48000)
            val = 0x40;
    }
    I2CWRNBYTE_CODEC(CJC8910_R8_SAMPLE_RATE , val);   // Clock and Sample rate control; R/W
#if 0
    I2CWRNBYTE_CODEC(CJC8910_R10_LEFT_DAC_VOLUME , 0xC3);   // Left channel DAC digital volume; R/W

    I2CWRNBYTE_CODEC(CJC8910_R12_BASS_CONTROL , 0x0f);   // BASS control; R/W
    I2CWRNBYTE_CODEC(CJC8910_R13_TREBLE_CONTROL , 0x0f);   // Treble control; R/W

    I2CWRNBYTE_CODEC(CJC8910_R17_ALC1_CONTROL , 0x00);   // ALC1 control; R/W
    I2CWRNBYTE_CODEC(CJC8910_R18_ALC2_CONTROL , 0x00);   // ACL2 control; R/W
    I2CWRNBYTE_CODEC(CJC8910_R19_ALC3_CONTROL , 0x00);   // ALC3 control; R/W
    I2CWRNBYTE_CODEC(CJC8910_R20_NOISE_GATE_CONTROL , 0x00);   // noise gate; R/W
    I2CWRNBYTE_CODEC(CJC8910_R21_LEFT_ADC_VOLUME , 0xc3);   // Left ADC digital volume; R/W

    I2CWRNBYTE_CODEC(CJC8910_R23_ADDITIONAL_CONTROL1 , 0x00);   // Additional control 1; R/W
    I2CWRNBYTE_CODEC(CJC8910_R24_ADDITIONAL_CONTROL2 , 0x00);   // Additional control 2; R/W
    I2CWRNBYTE_CODEC(CJC8910_R27_ADDITIONAL_CONTROL3 , 0x00);   // Additional control 3; R/W

    I2CWRNBYTE_CODEC(CJC8910_R32_ADCL_SIGNAL_PATH , 0x00);   //  ADC signal path control; R/W
    I2CWRNBYTE_CODEC(CJC8910_R33_MIC , 0x0A);   // micR/W   MIX

    I2CWRNBYTE_CODEC(CJC8910_R34_AUX , 0x0A);   // aux R/W   MIX
    I2CWRNBYTE_CODEC(CJC8910_R35_LEFT_OUT_MIX2_H , 0x00);   //  Left out Mix (2) ; R/W

    I2CWRNBYTE_CODEC(CJC8910_R37_ADC_PDN , 0x08);   // Adc_pdn sel; R/W

    I2CWRNBYTE_CODEC(CJC8910_R67_LOW_POWER_PLAYBACK , 0x08);   // Low power playback; R/W

    I2CWRNBYTE_CODEC(CJC8910_R25_PWR_MGMT1_H , 0xE8);   // Power management1 and VMIDSEL; R/W
    I2CWRNBYTE_CODEC(CJC8910_R26_PWR_MGMT2_H , 0x40);   // Power management2 and DAC left power down; R/W;
#else
    for(i = 0; i < ARRAY_SIZE(cjc8910_dac_to_out); i++)
    {
        I2CWRNBYTE_CODEC(cjc8910_dac_to_out[i].reg_index, cjc8910_dac_to_out[i].reg_value);
    }
#endif
}

static void CJC8910_off(void)
{
    I2CWRNBYTE_CODEC(CJC8910_R5_ADC_DAC_CONTROL, 8);
    aic_udelay(1000);
    snd_ext_pa_off();
    I2CWRNBYTE_CODEC(CJC8910_R25_PWR_MGMT1_L, 0);
    I2CWRNBYTE_CODEC(CJC8910_R26_PWR_MGMT2_L, 0);
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

    ret = i2c_transfer(client, &msg[0], 2);
    if (ret < 0)
        aicx_error(0x15bb9c, "d", "codec read failed, ret=%d\n", ret);

    return ret;
}

static int i2c_write(unsigned int reg, unsigned int value)
{
    u8 buf[2];
    int ret;

    buf[0] = reg;
    buf[1] = value;
    SNX_DBG(0x0085cf, "xx", "cjc8910: r=0x%x, v=0x%x\n", reg, value);

    ret = i2c_master_send(&cjc8910_data->client, buf, 2);
    if (ret < 0) {
        aicx_error(0x107c88, "xxd", "i2c_write failed, reg=0x%x, value=0x%x, ret=%d\n",
                   reg, value, ret);
        return ret;
    }

    return 0;
}

static unsigned char i2c_read(unsigned int reg)
{
    u8 buf[2];
    u8 reg_buf[2];
    int ret;

    /* aic_mdelay(20); */
    reg_buf[0] = reg;
    buf[0] = 0;
    buf[1] = 0;
    ret = i2c_send_then_read(&cjc8910_data->client, reg_buf, buf, 1);
    if (ret < 0) {
        aicx_error(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);

        return ret;
    }
    //alog_error("i2c_read: reg=0x%x, 0x%2x, 0x%2x\n", reg, buf[0], buf[1]);

    return buf[0];
}

//*************************************************************************
// CJC8910 Wirte register operation
//
int CJC8910_WriteReg(unsigned char Reg_Addr, unsigned char Reg_Data)
{
    i2c_write(Reg_Addr, Reg_Data);

    return 0;
}

//*************************************************************************
// CJC8910 Read register operation
//
unsigned short CJC8910_ReadReg(unsigned char Reg_Addr)
{
    unsigned char Data_L = 0, Data_H = 0;

    if ( (Reg_Addr & 0x01) == 0 )
    {
        Data_L = i2c_read(Reg_Addr);
        Data_H = i2c_read(Reg_Addr + 1);
    } else {
        Data_L = i2c_read(Reg_Addr - 1);
        Data_H = i2c_read(Reg_Addr);
    }

    SNX_DBG(0x0212e9, "xxxx", "reg=0x%02x: 0x%02x,0x%02x,0x%03x\n",
            Reg_Addr, Data_H, Data_L, ((Data_H << 8) | Data_L));

    return ((Data_H << 8) | Data_L);
}


static u32 cjc8910_show(u32 reg)
{
    u32 i = 0;

    if (reg != (u32)(-1)) {
        CONS_OUT("0x%02x 0x%03x\n", reg, CJC8910_ReadReg(reg << 1));
        return 0;
    }

    for (i = 0; i <= 0x0d; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8910_ReadReg(i << 1));

    for (i = 0x0f; i <= 0x1b; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8910_ReadReg(i << 1));

    for (i = 0x1f; i <= 0x2a; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8910_ReadReg(i << 1));

    CONS_OUT("0x%02x 0x%03x\n", 0x43, CJC8910_ReadReg((0x43) << 1));

    return 0;
}

static u32 a2hex(char *s)
{
    int v = 0;

    while(*s) {
        if (*s >= '0' && *s <= '9')
            v = (v << 4) | (*s - '0' );
        else if (*s >= 'a' && *s <= 'f')
            v = (v << 4) | (*s - 'a' + 10);
        else if (*s >= 'A' && *s <= 'F')
            v = (v << 4) | (*s - 'A' + 10);
        else
            return v;

        s++;
        continue;
    }

    return v;
}

static int codec_reg(int argc, char **argv)
{
    u32 reg = 0, val = 0;

    if (argc == 0)
        cjc8910_show((u32)(-1));
    else if (argc == 1)
        cjc8910_show(a2hex(argv[0]));
    else {
        reg = a2hex(argv[0]);
        val = a2hex(argv[1]);
        i2c_write(reg, val);
    }

    return 0;
}


CONSOLE_EXPORT_CMD(cjc2_reg, codec_reg, codec reg read_write);

static int cjc8910_set_params(struct snd_dpm_context *dpm, u32 wid,
                              u32 chn, u32 srate)
{
    struct snd_codec *codec = dpm->codec;
    struct cjc8910_private *cjc8910 =
        (struct cjc8910_private *)snd_card_get_codec_data(codec);

    cjc8910->srate = srate;

    return 0;
}

static int cjc8910_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        if (!cjc8910_data->ref_cnt) {
            pmu_enable(cjc8910_data->power);
            pmu_enable(cjc8910_data->vmicbias);
            CJC8910_ADC();
            snd_ext_pa_on();
            SNX_INFO(0x0c4f06, "", "cjc8910 on\n");
        }
        cjc8910_data->ref_cnt++;
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        if (cjc8910_data->ref_cnt > 0) {
            cjc8910_data->ref_cnt--;
            if (!cjc8910_data->ref_cnt) {
                CJC8910_off();
                pmu_disable(cjc8910_data->vmicbias);
                pmu_disable(cjc8910_data->power);
                SNX_INFO(0x06a888, "", "cjc8910 off\n");
            }
        }
        break;
    default:
        break;
    }

    return 0;
}

static int cjc8910_i2c_init(struct i2c_client *client)
{
    client->addr = CJC8910_IIC_ADDRESS;
    client->index = SND_CODEC_I2C_ID;

    return i2c_open(client);
}

static int cjc8910_probe(struct snd_codec *codec)
{
    struct cjc8910_private *cjc8910 = cjc8910_data;

    if (!codec) {
        SNX_ERROR(0x09a61f, "", "cjc8910 device not registered\n");
        return AIC_ENODEV;
    }
    snd_card_set_codec_data(codec, (u32)cjc8910);
    cjc8910->codec = codec;

    return 0;
}

static struct snd_codec_driver cjc8910_driver = {
    .probe = cjc8910_probe,
    .set_params = cjc8910_set_params,
    .stream_event = cjc8910_event,
};



static int cjc8910_init(void)
{
    struct cjc8910_private *cjc8910;
    int ret = -1;

    SNX_INFO(0x13dc00, "", "cjc8910_init enter\n");

    cjc8910 = aos_zmalloc(sizeof(*cjc8910));
    if (!cjc8910) {
        SNX_INFO(0x11cdd1, "", "cjc8910_init no memory\n");
        return AIC_ENOMEM;
    }

    cjc8910->adc_volume = 0;
#ifdef SND_CODEC_MCLK
    cjc8910->mclk = SND_CODEC_MCLK;
#else
    cjc8910->mclk = 24576000;
#endif

#ifdef CONFIG_MHA_SOUND
    struct mha_sound *cfg;
    cfg = mha_get_sound_cfg();
    if (!cfg || !cfg->codec_name || strcmp(cfg->codec_name, "CJC8910")) {
        aicx_error(0x088a45, "", "MHA, not CJC8910\n");
        snd_free(cjc8910);
        return AIC_EPERM;
    }
    if (cfg->codec_micbias_name) {
        cjc8910->vmicbias = pmu_get("vmic", (char *)cfg->codec_micbias_name);
        if (cfg->codec_micbias_volt)
            pmu_voltage_set(cjc8910->vmicbias, cfg->codec_micbias_volt, 10000);
    }
    if (cfg->codec_core_pwr_name) {
        cjc8910->power = pmu_get("8910 power", (char *)cfg->codec_core_pwr_name);
        if (cfg->codec_core_pwr_volt)
            pmu_voltage_set(cjc8910->power, cfg->codec_core_pwr_volt, 10000);
    }
#else
    if (SND_PMU_CODEC_MICBIAS)
        cjc8910->vmicbias = pmu_get("vmic", SND_PMU_CODEC_MICBIAS);
    if (SND_PMU_CODEC)
        cjc8910->power = pmu_get("cjc8910 power", SND_PMU_CODEC);
    if (cjc8910->vmicbias)
        pmu_voltage_set(cjc8910->vmicbias, 2800000, 10000);
#endif
    if (cjc8910->power)
        pmu_enable(cjc8910->power);
    cjc8910->adc_volume = 0;
    cjc8910->dac_volume = 0;

    cjc8910_i2c_init(&cjc8910->client);
    cjc8910_data = cjc8910;

    ret =  snd_card_register_codec("cjc8910", &cjc8910_driver);

    if (cjc8910->power)
        pmu_disable(cjc8910->power);

    SNX_INFO(0x0d1a2b, "dp", "cjc8910_init done ret:%d vmicbias:%p\n", ret, cjc8910->vmicbias);

    return ret;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(cjc8910_init);
