/**************************************************/
//Revision：1.4.1.0.0608
//说明01:Master模式下的NFS格式需要手动调节LRCK分频
//说明02:需要先配置IIS后在配置IIC，如果在没有IIS时钟的情况下配置IIC则需要有IIS
//       后在走下RESET复位操作防止工作异常
/**************************************************/
/*
 * 1. slave address need confirm
 * 2. i2c index and pinmap/crossbar
 * 3. when iis clock on
*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x060f07
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
#include <audio/snd_config.h>
#include "snd_card.h"

#define REG_MAX     0x24
#define DELAY_MS    aos_sleep

#define CHECK_CLIENT(c)     \
    if (!c->data)   \
        return AIC_EPERM;

#define CHECK_REG(r)    \
        if (r > REG_MAX && r != 0xF9 && r != 0xFC && r != 0xF7) {  \
            alog_error("wrong codec reg = 0x%x\n", r);    \
            return 0;           \
        }

#define I2CWRNBYTE_CODEC    es7243l_write
/* codec private data */
struct es7243l_private {
    struct snd_codec *codec;
    struct i2c_client client;

    struct pmu_user *vmicbias;
    struct pmu_user *power;

    u32 ratio;
    u32 mclk;
    u32 srate;
    u8 chn;
    u8 wid;
    u8 adc_volume;
};

struct es7243l_private *es7243l_data;

/***************参数定义***************/
#define CHIPID				0x72430x		//Read 0xFD/FE/FF == 0x72430X，判断IC型号是否为ES7243L
#define STATEconfirm		0xFC			//状态机确认 回读STATEconfirm的寄存值是否为0x20，确认IC正常工作状态

#define NORMAL_I2S			0x00
#define NORMAL_LJ			0x01
#define NORMAL_DSPA			0x03
#define NORMAL_DSPB			0x23
#define Format_Len24		0x00
#define Format_Len20		0x01
#define Format_Len18		0x02
#define Format_Len16		0x03
#define Format_Len32		0x04

#define VDDA_3V3			0x00
#define VDDA_1V8			0x01
#define MCLK_PIN			0x00
#define SCLK_PIN			0x01
/***************参数定义***************/

/***************参数选择***************/
#define Device 				ES7243L			//产品选择是ES7243L,只支持1V8供电
#define CHANNELS_MAX		4				//TDM_NFS模式下配置,多少CH做级联,偶数
#define MSMode_MasterSelOn	0				//产品主从模式选择:默认选择0为SlaveMode,打开为1选择MasterMode
#define Ratio 				(es7243l_data->ratio)				//实际Ratio=MCLK/LRCK比率，需要和实际时钟比例匹配
#define Format 				NORMAL_I2S		//数据格式选择,需要和实际时序匹配
#define Format_Len			Format_Len16	//数据长度选择,需要和实际时序匹配
#define TDMINSelon			0				//TDMIN级联模式选择:默认选择关闭0,打开为1
#define SCLK_DIV			4				//SCLK分频选择:(选择范围1~128),SCLK=MCLK/SCLK_DIV
#define SCLK_INV			0				//默认对齐方式为下降沿,1为上升沿对齐,需要和实际时序匹配
#define MCLK_SOURCE			MCLK_PIN		//是否硬件没接MCLK需要用SCLK当作MCLK
//#define VDDA_VOLTAGE		VDDA_3V3		//模拟电压选择为3V3还是1V8,需要和实际硬件匹配
#define ADC_PGA_GAIN		12				//ADC模拟增益:(选择范围0~14),具体对应关系见相应DS说明
#define ADC_Volume			191				//ADC数字增益:(选择范围0~255),191:0DB,±0.5dB/Step
#define Dmic_Selon 			0				//DMIC选择:默认选择关闭0,打开为1
#define Dmic_GAIN 			0				//DMIC增益:(选择范围0~7),6dB/Step
/***************参数选择***************/

#define ADC_DATASEL        0x2 << 6    //output R_R mode

int es7243l_write(unsigned int reg, unsigned int value);
unsigned int es7243l_read(unsigned int reg);

void ES7243L_ADC(void)
{
    SNX_INFO(0x0f7bc2, "d", "ES7243L_ADC Ratio:%d\n", Ratio);

    I2CWRNBYTE_CODEC(0x01,0x3A);
    I2CWRNBYTE_CODEC(0x00,0x80);
    I2CWRNBYTE_CODEC(0xF9,0x00);
    I2CWRNBYTE_CODEC(0x04,0x02);
    I2CWRNBYTE_CODEC(0x04,0x01);
    I2CWRNBYTE_CODEC(0xF9,0x01);
    I2CWRNBYTE_CODEC(0x00,0x1E);
    I2CWRNBYTE_CODEC(0x01,0x00);
    I2CWRNBYTE_CODEC(0x0D,ADC_DATASEL | Dmic_GAIN); //Default USE output R_R

    if(Ratio == 64)//Ratio=MCLK/LRCK=64：3M072/48K；1M028/16K; 512K/8K
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x03);
        I2CWRNBYTE_CODEC(0xF9,0x00);
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 256)//Ratio=MCLK/LRCK=256：12M288/48K；4M096/16K; 2M048/8K
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x01);
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 384)//Ratio=MCLK/LRCK=384：12M288/32K；6M144/16K；3M072/8K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0xF9,0x00);
        I2CWRNBYTE_CODEC(0x04,0x22);//384/3*4=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 400)//Ratio=MCLK/LRCK=400：19M2/96K?
    {
        I2CWRNBYTE_CODEC(0x03,0x19);//D25
        I2CWRNBYTE_CODEC(0x04,0x00);//
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 512)//Ratio=MCLK/LRCK=512：12M288/24K；6M144/12K；4M096/8K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x00);//512=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 768)//Ratio=MCLK/LRCK=768：12M288/16K；6M144/8K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x21);//768/3*2=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 800)//Ratio=MCLK/LRCK=800：19M2/48K
    {
        I2CWRNBYTE_CODEC(0x03,0x19);
        I2CWRNBYTE_CODEC(0x04,0x10);//
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 1024)//Ratio=MCLK/LRCK=512：12M288/12K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x10);//1024/2=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 1536)//Ratio=MCLK/LRCK=1536：12M288/8K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x20);//1536/3=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    else if(Ratio == 3072) //aic add for 8K Ratio=MCLK/LRCK=3072；24M576/8K；
    {
        I2CWRNBYTE_CODEC(0x03,0x20);
        I2CWRNBYTE_CODEC(0x04,0x50);//3072/6=512
        I2CWRNBYTE_CODEC(0x05,0x00);
    }
    I2CWRNBYTE_CODEC(0x06,SCLK_DIV - 1);	//SCLK DIV
    I2CWRNBYTE_CODEC(0x07,(Ratio>>8));		//LRCK DIV
    I2CWRNBYTE_CODEC(0x08,(Ratio & 0xFF));	//LRCK DIV
    I2CWRNBYTE_CODEC(0x02,(MCLK_SOURCE<<7) + SCLK_INV);

    I2CWRNBYTE_CODEC(0x09,0xD0);//S1_TIME
    I2CWRNBYTE_CODEC(0x0A,0xD0);//S2_TIME
    I2CWRNBYTE_CODEC(0x0B,0xC0 + Format + (Format_Len<<2));
    I2CWRNBYTE_CODEC(0x0C,0x00 + (TDMINSelon<<3));
    I2CWRNBYTE_CODEC(0x0E,ADC_Volume);

    I2CWRNBYTE_CODEC(0x10,0x38);
    I2CWRNBYTE_CODEC(0x11,0x16);
    I2CWRNBYTE_CODEC(0x14,0x0C);
    I2CWRNBYTE_CODEC(0x15,0x0C);
    I2CWRNBYTE_CODEC(0x17,0x02);
    I2CWRNBYTE_CODEC(0x18,0x26);

    I2CWRNBYTE_CODEC(0x0F,0xF0);
    I2CWRNBYTE_CODEC(0x19,0xAA);
    I2CWRNBYTE_CODEC(0x1F,(Dmic_Selon<<5));

    I2CWRNBYTE_CODEC(0x1A,0xF4);
    I2CWRNBYTE_CODEC(0x1B,0x66);
    I2CWRNBYTE_CODEC(0x1C,0x44);
    I2CWRNBYTE_CODEC(0x1E,0x00);

    I2CWRNBYTE_CODEC(0x20,0x10 + ADC_PGA_GAIN);//PGA 模拟增益:
    I2CWRNBYTE_CODEC(0x21,0x10 + ADC_PGA_GAIN);//PGA 模拟增益:

    I2CWRNBYTE_CODEC(0x00,0x80 + (MSMode_MasterSelOn<<6));//Slave  Mode

    I2CWRNBYTE_CODEC(0x01,0x3A);
    I2CWRNBYTE_CODEC(0x16,0x3F);
    I2CWRNBYTE_CODEC(0x16,0x00);
    DELAY_MS(5);
    I2CWRNBYTE_CODEC(0x0B,Format + (Format_Len<<2));
}

void ES7243L_Standby(void)//待机配置(0UA)--搭配ES7243L_Resume(void)//恢复配置
{
    I2CWRNBYTE_CODEC(0x04,0x02);
    I2CWRNBYTE_CODEC(0x04,0x01);
    I2CWRNBYTE_CODEC(0xF7,0x30);
    I2CWRNBYTE_CODEC(0xF9,0x01);
    I2CWRNBYTE_CODEC(0x16,0xFF);
    I2CWRNBYTE_CODEC(0x17,0x00);
//  I2CWRNBYTE_CODEC(0x17,0x0A);//快速启动，但是有200ua保持电路
    I2CWRNBYTE_CODEC(0x01,0x38);
    I2CWRNBYTE_CODEC(0x20,0x00);
    I2CWRNBYTE_CODEC(0x21,0x00);
    I2CWRNBYTE_CODEC(0x00,0x00);
    I2CWRNBYTE_CODEC(0x00,0x1E);
    I2CWRNBYTE_CODEC(0x01,0x30);
    I2CWRNBYTE_CODEC(0x01,0x00);
}

void ES7243L_Resume(void)//恢复配置(未下电)--搭配ES7243L_Standby(void)
{
    I2CWRNBYTE_CODEC(0x0B,0xc0 + Format + (Format_Len<<2));
    I2CWRNBYTE_CODEC(0xF9,0x00);
    I2CWRNBYTE_CODEC(0x04,0x01);
    I2CWRNBYTE_CODEC(0x17,0x01);
    I2CWRNBYTE_CODEC(0x20,0x10);
    I2CWRNBYTE_CODEC(0x21,0x10);
    I2CWRNBYTE_CODEC(0x00,0x80);
    I2CWRNBYTE_CODEC(0x01,0x3A);
    I2CWRNBYTE_CODEC(0x16,0x3F);
    I2CWRNBYTE_CODEC(0x16,0x00);
    DELAY_MS(5);
    I2CWRNBYTE_CODEC(0x0B,Format + (Format_Len<<2));

}

void ES7243L_Reset(void)//后提供IIS时钟后需要额外配置
{
    I2CWRNBYTE_CODEC(0x00,0x1E);
    I2CWRNBYTE_CODEC(0x00,0x80 + (MSMode_MasterSelOn<<6));//Slave  Mode
}

static int i2c_send_then_read(struct i2c_client *client, u8 *reg,
                              u8 *buf, int num)
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
        SNX_ERROR(0x1e2f32, "d", "es7243l i2c read failed, ret=%d\n", ret);

    return ret;
}

static int i2c_write(struct i2c_client *client,
                     unsigned int reg, unsigned int value)
{
    u8 buf[2];
    int ret;

    SNX_INFO(0x1b7b9e, "xx", "es7243l: r=0x%x, v=0x%x\n", reg, value);

    CHECK_CLIENT(client)
    buf[0] = reg;
    buf[1] = value;

    ret = i2c_master_send(client, buf, 2);
    if (ret < 0) {
        SNX_ERROR(0x0c7a81, "xxd", "es7243l write failed, reg=0x%02x, value=0x%02x, ret=%d\n",
                  reg, value, ret);

        return ret;
    }

    return 0;
}

static unsigned int i2c_read(struct i2c_client *client, unsigned int reg)
{
    u8 buf[2];
    u8 reg_buf[2];
    int ret;

    CHECK_CLIENT(client)

    reg_buf[0] = reg;
    buf[0] = 0;
    ret = i2c_send_then_read(client, reg_buf, buf, 1);
    if (ret < 0) {
        SNX_ERROR(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);

        return 0;
    }

    SNX_INFO(0x12d2be, "xx", "aw: r=0x%x, v=0x%x\n", reg, (u32)buf[0]);

    return (u32)buf[0];
}

int es7243l_write(unsigned int reg, unsigned int value)
{
    CHECK_REG(reg)

    return i2c_write(&es7243l_data->client, reg, value);
}

unsigned int es7243l_read(unsigned int reg)
{
    CHECK_REG(reg)

    return i2c_read(&es7243l_data->client, reg);
}

static u32 es7243l_show(u32 reg)
{
    u32 i = 0;

    if (reg != (u32)(-1)) {
        CONS_OUT("0x%02x 0x%02x\n", reg, es7243l_read(reg));
        return 0;
    }

    for (i = 0; i <= REG_MAX; i++)
        CONS_OUT("0x%02x 0x%02x\n", i, es7243l_read(i));

    CONS_OUT("0x%02x 0x%02x\n", 0xF9, es7243l_read(0xF9));
    CONS_OUT("0x%02x 0x%02x\n", 0xFC, es7243l_read(0xFC));

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
        es7243l_show((u32)(-1));
    else if (argc == 1)
        es7243l_show(a2hex(argv[0]));
    else {
        reg = a2hex(argv[0]);
        val = a2hex(argv[1]);
        es7243l_write(reg, val);
    }

    return 0;
}
CONSOLE_EXPORT_CMD(codec_reg, codec_reg, codec reg read_write);

static int es7243l_set_params(struct snd_dpm_context *dpm, u32 wid,
                              u32 chn, u32 srate)
{
    struct snd_codec *codec = dpm->codec;
    struct es7243l_private *es7243l =
        (struct es7243l_private *)snd_card_get_codec_data(codec);

    SNX_INFO(0x1a55e9, "sddd", "Enter into %s(), wid=%d, chn=%d, srate=%d\n",
           __func__, wid, chn, srate);

    es7243l->srate = srate;
    es7243l->chn = chn;
    es7243l->wid = wid;
    es7243l->ratio = es7243l->mclk / srate;

    return 0;
}

static int es7243l_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        /* TODO: add ref count */
        pmu_enable(es7243l_data->power);
        pmu_enable(es7243l_data->vmicbias);
        ES7243L_ADC();
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        /* TODO: add ref count */
        ES7243L_Standby();
        pmu_disable(es7243l_data->power);
        pmu_disable(es7243l_data->vmicbias);
        break;
    default:
        break;
    }

    return 0;
}

static int es7243l_i2c_init(struct i2c_client *client)
{
    client->addr = 0x10;
    client->index = 3;

    return i2c_open(client);
}

static int es7243l_probe(struct snd_codec *codec)
{
    struct es7243l_private *es7243l = es7243l_data;

    if (!codec) {
        SNX_ERROR(0x127c50, "", "es7243l device not registered\n");
        return AIC_ENODEV;
    }
    snd_card_set_codec_data(codec, (u32)es7243l);
    es7243l->codec = codec;

    return 0;
}

static struct snd_codec_driver es7243l_driver = {
    .probe = es7243l_probe,
    .set_params = es7243l_set_params,
    .stream_event = es7243l_event,
};

static int es7243l_init(void)
{
    struct es7243l_private *es7243l;
    int ret = -1;

    es7243l = aos_zmalloc(sizeof(*es7243l));
    if (!es7243l)
        return AIC_ENOMEM;

    es7243l->adc_volume = 0;
    es7243l->mclk = 24576000;

    es7243l->vmicbias = pmu_get("vmic", "VDDMIC");
    if (es7243l->vmicbias)
        pmu_voltage_set(es7243l->vmicbias, 2800000, 10000);

    es7243l->power = pmu_get("es7243l power", "VDDCAMIO1V8");
    pmu_enable(es7243l->power);

    es7243l_i2c_init(&es7243l->client);
    es7243l_data = es7243l;

    ret =  snd_card_register_codec("es7243l", &es7243l_driver);

    SNX_INFO(0x1bf050, "", "es7243l_init done\n");

    return ret;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(es7243l_init);

