/**************************************************************************
**   FileName: CJC8911_Reg.c                                              *
**     Author: tf                                                         *
**  Copyright: Wuhan GuangHuaXin Technology (C)2012. All rights reserved  *
**   HomePage: www.csc-ic.com                                             *
**       Date: 2012-05-08 13:58:09                                        *
**Description: CJC8911 Register configuration and Initial                 *
***************************************************************************/

/*************************************************************************
** Below recommend register value only some opertaion mode:
** 1> LINE Input 1 to ADC mode, slave mode, I2S format output, MCLK=12.288MHz, Fs=48KHz;
** 2> DAC to LOUT1 mode, slave mode, I2S format output, MCLK=12.288MHz, Fs=48KHz;
** 3> LINE Input 1 to LOUT 1 mode, slave mode, I2S format output, MCLK=12.288MHz, Fs=48KHz;
** if you need other operation mode, please look CJC8911 datasheet.
** if you set LINE INPUT 2 or LOUT2, please look R32,R33,R34,R35,R36,R37 Register.
***************************************************************************/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17cad9
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

#include "CJC8911_Reg.h"

struct cjc8911 {
    struct i2c_client client;
    struct pmu_user *power;
    struct pmu_user *vmicbias;

    u32 ref_cnt;
    u32 srate;
    u32 wid;
    u32 chn;
};

struct cjc8911 cjc8911;

unsigned int Delay_ms(unsigned int t)
{
    aic_mdelay(t);

    return 0;
}

static int i2c_init(struct i2c_client *client)
{
    client->addr = 0x1B;
    client->index = 3;

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

    ret = i2c_master_send(&cjc8911.client, buf, 2);
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
    ret = i2c_send_then_read(&cjc8911.client, reg_buf, buf, 1);
    if (ret < 0) {
        aicx_error(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);

        return ret;
    }
    //alog_error("i2c_read: reg=0x%x, 0x%2x, 0x%2x\n", reg, buf[0], buf[1]);

    return buf[0];
}

//*************************************************************************
// CJC8911 Wirte register operation
//
unsigned char CJC8911_WriteReg(unsigned char Reg_Addr, unsigned char Reg_Data)
{
    i2c_write(Reg_Addr, Reg_Data);

    return 0;
}

//*************************************************************************
// CJC8911 Read register operation
//
unsigned short CJC8911_ReadReg(unsigned char Reg_Addr)
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

    aicx_info(0x0212e9, "xxxx", "reg=0x%02x: 0x%02x,0x%02x,0x%03x\n",
              Reg_Addr, Data_H, Data_L, ((Data_H << 8) | Data_L));

    return ((Data_H << 8) | Data_L);
}

//*************************************************************************
// CJC8911 Register Initial Value
//
struct CJC8911_reg{
    unsigned char reg_index;
    unsigned char reg_value;
    };

//*************************************************************************
// LINE Input to ADC mode, slave mode, I2S format output, MCLK=12.288MHz, BCLK =3.072MHz, Fs=48KHz;
//
static struct CJC8911_reg    LINPUT1_TO_ADC[] = {
    {CJC8911_R0_LEFT_INPUT_VOLUME,       0x17},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W
    {CJC8911_R7_AUDIO_INTERFACE,         0x0A},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x00},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
    {CJC8911_R11_RIGHT_DAC_VOLUME,       0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,          0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,           0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,        0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,    0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,            0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x00},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                    0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,                0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_H,            0xE0},   // Power management2 and DAC left power down; R/W;
};

#define CJC8911_LINPUT1_TO_ADC_REG_NUM    ARRAY_SIZE(LINPUT1_TO_ADC)

//*************************************************************************
// DAC to LOUT1 mode, slave mode, I2S format input, MCLK=12.288MHz, BCLK =3.072MHz, Fs=48KHz;
//
static struct CJC8911_reg    DAC_TO_LOUT1[] = {
    {CJC8911_R0_LEFT_INPUT_VOLUME,       0x17},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W

    /* from 0a -> 02, 16bit wid */
    {CJC8911_R7_AUDIO_INTERFACE,         0x02},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x00},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
  {CJC8911_R11_RIGHT_DAC_VOLUME,         0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,            0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,             0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,          0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,      0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,              0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x00},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                  0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,               0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_H,            0xE0},   // Power management2 and DAC left power down; R/W;
};

static struct CJC8911_reg    DAC_TO_LOUT1_16K[] = {
    {CJC8911_R0_LEFT_INPUT_VOLUME,       0x3a},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W

    /* from 0a -> 02, 16bit wid */
    {CJC8911_R7_AUDIO_INTERFACE,         0x02},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x54},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
  {CJC8911_R11_RIGHT_DAC_VOLUME,         0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,            0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,             0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,          0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,      0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,              0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x20},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                  0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,               0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_H,            0xE0},   // Power management2 and DAC left power down; R/W;
};


static struct CJC8911_reg    LINPUT1_TO_LOUT1[] = {
};

#define CJC8911_LINPUT1_TO_LOUT1_REG_NUM    ARRAY_SIZE(LINPUT1_TO_LOUT1)

//*************************************************************************
// LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK = 12MHz, Fs=8KHz;
//
static struct CJC8911_reg    LINPUT2_ADC_TO_DAC_LOUT2_8[] = {
     {CJC8911_R0_LEFT_INPUT_VOLUME,       0x17},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W
    {CJC8911_R7_AUDIO_INTERFACE,         0x0A},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x00},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
  {CJC8911_R11_RIGHT_DAC_VOLUME,         0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,          0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,           0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,        0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,             0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,            0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x00},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                   0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,                0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_H,            0xE0},   // Power management2 and DAC left power down; R/W;
};

#define CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_8_REG_NUM    ARRAY_SIZE(LINPUT2_ADC_TO_DAC_LOUT2_8)

//*************************************************************************
// LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK = 12MHz, Fs=32KHz;
//
static struct CJC8911_reg    LINPUT2_ADC_TO_DAC_LOUT2_32[] = {
      {CJC8911_R0_LEFT_INPUT_VOLUME,       0x17},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W
    {CJC8911_R7_AUDIO_INTERFACE,         0x0A},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x00},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
  {CJC8911_R11_RIGHT_DAC_VOLUME,         0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,             0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,             0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,             0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,         0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,             0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x00},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                    0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,                0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_H,            0xE0},   // Power management2 and DAC left power down; R/W;
};

#define CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_32_REG_NUM    ARRAY_SIZE(LINPUT2_ADC_TO_DAC_LOUT2_32)

//*************************************************************************
// LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK = 12MHz, Fs=44.1KHz;
//
static struct CJC8911_reg    LINPUT2_ADC_TO_DAC_LOUT2_44[] = {
      {CJC8911_R0_LEFT_INPUT_VOLUME,      0x17},   // Audio input left channel volume; R/W

    {CJC8911_R2_LOUT1_VOLUME,            0x79},   // Audio output letf channel1 volume; R/W

    {CJC8911_R5_ADC_DAC_CONTROL,         0x00},   // ADC and DAC CONTROL; R/W
    {CJC8911_R7_AUDIO_INTERFACE,         0x0A},   // Digital Audio interface format; R/W
    {CJC8911_R8_SAMPLE_RATE,             0x00},   // Clock and Sample rate control; R/W
    {CJC8911_R10_LEFT_DAC_VOLUME,        0xff},   // Left channel DAC digital volume; R/W
  {CJC8911_R11_RIGHT_DAC_VOLUME,         0xff},   // right channel DAC digital volume; R/W

    {CJC8911_R12_BASS_CONTROL,           0x0f},   // BASS control; R/W
    {CJC8911_R13_TREBLE_CONTROL,         0x0f},   // Treble control; R/W
    {CJC8911_R16_3D_CONTROL,             0x00},   // 3D control; R/W
     {CJC8911_R17_ALC1_CONTROL,             0x7b},   // ALC1 control; R/W
    {CJC8911_R18_ALC2_CONTROL,             0x00},   // ACL2 control; R/W
       {CJC8911_R19_ALC3_CONTROL,             0x32},   // ALC3 control; R/W
     {CJC8911_R20_NOISE_GATE_CONTROL,          0x00},   // noise gate; R/W
{CJC8911_R21_LEFT_ADC_VOLUME,               0xc3},   // Left ADC digital volume; R/W

    {CJC8911_R23_ADDITIONAL_CONTROL1,    0x00},   // Additional control 1; R/W
    {CJC8911_R24_ADDITIONAL_CONTROL2,    0x04},   // Additional control 2; R/W
    {CJC8911_R27_ADDITIONAL_CONTROL3,    0x00},   // Additional control 3; R/W

    {CJC8911_R31_ADC_INPUT_MODE,         0x00},   // ADC input mode; R/W

    {CJC8911_R32_ADCL_SIGNAL_PATH,       0x00},   //  ADC signal path control; R/W
    {CJC8911_R33_MIC,                    0x0a},   // micR/W

    {CJC8911_R34_AUX,                    0x0a},   // aux R/W
    {CJC8911_R35_LEFT_OUT_MIX2_H,        0x00},   //  Left out Mix (2) ; R/W
    {CJC8911_R36_RIGHT_OUT_MIX1_L,       0x80},   //  Right out Mix (1; R/W
    {CJC8911_R37_ADC_PDN,                0x00},   // Adc_pdn sel; R/W


    {CJC8911_R43_LOW_POWER_PLAYBACK,     0x00},   // Low power playback; R/W

    {CJC8911_R25_PWR_MGMT1_H,            0xE8},   // Power management1 and VMIDSEL; R/W
    {CJC8911_R26_PWR_MGMT2_L,            0xE0},   // Power management2 and DAC left power down; R/W;
};

#define CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_44_REG_NUM    ARRAY_SIZE(LINPUT2_ADC_TO_DAC_LOUT2_44)

//*************************************************************************
// CJC8911 LINE Input 1 to ADC mode, slave mode, I2S format output, MCLK=12.288MHz, BCLK =3.072MHz, Fs=48KHz;
//
void CJC8911_LINPUT1_TO_ADC(void)
{
    int i;
    for (i = 0; i < CJC8911_LINPUT1_TO_ADC_REG_NUM; i++)
    {
        CJC8911_WriteReg(LINPUT1_TO_ADC[i].reg_index, LINPUT1_TO_ADC[i].reg_value);
    }
}

#define CJC8911_DAC_TO_LOUT1_REG_NUM    ARRAY_SIZE(DAC_TO_LOUT1)
//*************************************************************************
// CJC8911 DAC to LOUT1, slave mode, I2S format output, MCLK=12.288MHz, BCLK =3.072MHz, Fs=48KHz;
//
void CJC8911_DAC_TO_LOUT1(void)
{
    int i;
    for (i = 0; i < CJC8911_DAC_TO_LOUT1_REG_NUM; i++)
    {
        CJC8911_WriteReg(DAC_TO_LOUT1[i].reg_index, DAC_TO_LOUT1[i].reg_value);
    }
}

void CJC8911_DAC_TO_LOUT1_16K(void)
{
    int i;
    for (i = 0; i < CJC8911_DAC_TO_LOUT1_REG_NUM; i++)
    {
        CJC8911_WriteReg(DAC_TO_LOUT1_16K[i].reg_index, DAC_TO_LOUT1_16K[i].reg_value);
    }
}

//*************************************************************************
// LINE Input 1 to LOUT 1 mode, slave mode, I2S format output, MCLK=12.288MHz, BCLK =3.072MHz, Fs=48KHz;
//
void CJC8911_LINPUT1_TO_LOUT1(void)
{
    int i;
    for (i = 0; i < CJC8911_LINPUT1_TO_LOUT1_REG_NUM; i++)
    {
        CJC8911_WriteReg(LINPUT1_TO_LOUT1[i].reg_index, LINPUT1_TO_LOUT1[i].reg_value);
    }
}

//*************************************************************************
// CJC8911 LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK =12MHz, Fs=8KHz;
//
void CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_8(void)
{
    int i;
    for (i = 0; i < CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_8_REG_NUM; i++)
    {
        CJC8911_WriteReg(LINPUT2_ADC_TO_DAC_LOUT2_8[i].reg_index, LINPUT2_ADC_TO_DAC_LOUT2_8[i].reg_value);
    }
}

//*************************************************************************
// CJC8911 LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK =12MHz, Fs=32KHz;
//
void CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_32(void)
{
    int i;
    for (i = 0; i < CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_32_REG_NUM; i++)
    {
        CJC8911_WriteReg(LINPUT2_ADC_TO_DAC_LOUT2_32[i].reg_index, LINPUT2_ADC_TO_DAC_LOUT2_32[i].reg_value);
    }
}

#define LINPUT2_ADC_TO_DAC_LOUT2_44_REG_NUM     ARRAY_SIZE(LINPUT2_ADC_TO_DAC_LOUT2_44)
//*************************************************************************
// CJC8911 LINE Input 2 to ADC and DAC to LOUT 2 mode, slave mode, I2S format, MCLK=12MHz, BCLK =12MHz, Fs=44.1KHz;
//
void CJC8911_LINPUT2_ADC_TO_DAC_LOUT2_44(void)
{
    int i;
    for (i = 0; i < LINPUT2_ADC_TO_DAC_LOUT2_44_REG_NUM; i++)
    {
        CJC8911_WriteReg(LINPUT2_ADC_TO_DAC_LOUT2_44[i].reg_index, LINPUT2_ADC_TO_DAC_LOUT2_44[i].reg_value);
    }
}

void cjc8911_open(void)
{
    if (cjc8911.ref_cnt) {
        cjc8911.ref_cnt++;
        aicx_error(0x150cb1, "d", "codec already opened, ref_count=%d\n", cjc8911.ref_cnt);

        return;
    }

    if (cjc8911.power)
        pmu_enable(cjc8911.power);

    snd_ext_pa_on();

    if (cjc8911.vmicbias) {
        pmu_enable(cjc8911.vmicbias);
        aicx_debug(0x117436, "", "mic bias enabled\n");
    }
    switch (cjc8911.srate) {
    case 48000:
        aicx_error(0x02755d, "", "codec opened, 48000\n");
        //CJC8911_LINPUT1_TO_ADC();
        CJC8911_DAC_TO_LOUT1();
        break;
    case 16000:
        aicx_error(0x12f4e8, "", "codec opened, 16000\n");
        CJC8911_DAC_TO_LOUT1_16K();
        break;
    default:
        aicx_error(0x0059e1, "d", "codec opened, unknow srate=%d\n", cjc8911.srate);
        break;
    }
    cjc8911.ref_cnt++;
}

void cjc8911_close(void)
{
    if (cjc8911.ref_cnt >= 1) {
        cjc8911.ref_cnt--;
        if (!cjc8911.ref_cnt) {
            aicx_info(0x0ec8b6, "", "codec close\n");

            CJC8911_WriteReg(0xa, 0x08);
            aic_udelay(1000);

            snd_ext_pa_off();
            CJC8911_WriteReg(0x32, 0);
            CJC8911_WriteReg(0x34, 0);

            if (cjc8911.vmicbias)
                pmu_disable(cjc8911.vmicbias);

            /* TODO: close codec here */
            if (cjc8911.power)
                pmu_disable(cjc8911.power);
        }
    }

    return;
}

int cjc8911_set_params(struct snd_dpm_context *dpm, u32 wid, u32 chn, u32 srate)
{
    aicx_info(0x09ed98, "ddd", "cjc8911 srate=%d, chn=%d, wid=%d\n", srate, chn, wid);
    cjc8911.srate = srate;
    cjc8911.wid = wid;
    cjc8911.chn = chn;

    return 0;
}

static u32 cjc8911_codec_read(struct snd_codec *codec, u32 reg)
{
    aicx_error(0x1a6b3a, "", "cjc8911 read\n");

    return 0;
}

static int cjc8911_codec_write(struct snd_codec *codec, u32 reg, u32 val)
{
    aicx_error(0x114e61, "", "cjc8911 write\n");

    return 0;
}

static u32 cjc8911_show(u32 reg)
{
    u32 i = 0;

    if (reg != (u32)(-1)) {
        CONS_OUT("0x%02x 0x%03x\n", reg, CJC8911_ReadReg(reg << 1));
        return 0;
    }

    for (i = 0; i <= 0x0d; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8911_ReadReg(i << 1));

    for (i = 0x0f; i <= 0x1b; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8911_ReadReg(i << 1));

    for (i = 0x1f; i <= 0x2a; i++)
        CONS_OUT("0x%02x 0x%03x\n", i, CJC8911_ReadReg(i << 1));

    CONS_OUT("0x%02x 0x%03x\n", 0x43, CJC8911_ReadReg((0x43) << 1));

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
        cjc8911_show((u32)(-1));
    else if (argc == 1)
        cjc8911_show(a2hex(argv[0]));
    else {
        reg = a2hex(argv[0]);
        val = a2hex(argv[1]);
        i2c_write(reg, val);
    }

    return 0;
}
CONSOLE_EXPORT_CMD(cjc_reg, codec_reg, codec reg read_write);

int cjc8911_stream_event(struct snd_dpm_context *dpm, int event)
{
    aicx_error(0x18a53c, "d", "cjc8911 event=%d\n", event);

    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        cjc8911_open();
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        cjc8911_close();
        break;
    default:
        break;
    }

    return 0;
}

static struct snd_codec_driver cjc8911_codec_driver = {
    .read = NULL,
    .write = NULL,
    .controls = NULL,
    .num_controls = 0,
    .dpm_widgets = NULL,
    .num_dpm_widgets = 0,
    .dpm_routes = NULL,
    .num_dpm_routes = 0,
    .read = cjc8911_codec_read,
    .write = cjc8911_codec_write,
    .set_params = cjc8911_set_params,
    .stream_event = cjc8911_stream_event,
};

int cjc8911_codec_init(void)
{
    i2c_init(&cjc8911.client);

#ifdef CONFIG_MHA_SOUND
    struct mha_sound *cfg;
    cfg = mha_get_sound_cfg();
    if (!cfg || !cfg->codec_name || strcmp(cfg->codec_name, "CJC8911")) {
        aicx_error(0x1db194, "", "MHA, not CJC8911\n");
        return AIC_EPERM;
    }
    if (cfg->codec_micbias_name) {
        cjc8911.vmicbias = pmu_get("vmic", (char *)cfg->codec_micbias_name);
        if (cfg->codec_micbias_volt)
            pmu_voltage_set(cjc8911.vmicbias, cfg->codec_micbias_volt, 10000);

    }
    if (cfg->codec_core_pwr_name) {
        cjc8911.power = pmu_get("CJC8911 power", (char *)cfg->codec_core_pwr_name);
        if (cfg->codec_core_pwr_volt)
            pmu_voltage_set(cjc8911.vmicbias, cfg->codec_core_pwr_volt, 10000);
    }
#else
    cjc8911.vmicbias = pmu_get("vmic", SND_PMU_CODEC_MICBIAS);
    cjc8911.power = pmu_get("CJC8911 power", SND_PMU_CODEC);

    if (cjc8911.vmicbias)
        pmu_voltage_set(cjc8911.vmicbias, 2800000, 10000);
#endif

    return snd_card_register_codec("cjc8911_codec", &cjc8911_codec_driver);
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(cjc8911_codec_init);

