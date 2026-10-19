
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0be996
#define LOG_MODULE    AMOD_SOUND
#include <aic_type.h>
#include <alog_api.h>
#include <aic_init.h>
#include <i2c.h>

//#include <i2c.h>
//#include <timer.h>

#include "snd_card.h"
#include "snd_dpm.h"
#include "snd_dpm_driver.h"

#define I2CWRNBYTE_CODEC    i2c_write
#define I2CRDNBYTE_CODEC    i2c_read
#define DELAY_MS(ms)        //aic_mdelay(ms)
#define EXT_MCLK            24576000

#define CHECK_CLIENT(c)     \
    if (!c->data)   \
        return AIC_EPERM;

#define ES8374_DACVOLUME    0x11

/**************************************************/
//Revision1.2.0.0
/**************************************************/
#define Slave			0x00
#define Master			0x80
#define NORMAL_I2S		0x00
#define NORMAL_LJ		0x01
#define NORMAL_RJ		0x21
#define NORMAL_DSPA		0x03
#define NORMAL_DSPB		0x23
#define Format_Len16	0x0C
#define Format_Len18	0x08
#define Format_Len20	0x04
#define Format_Len24	0x00
#define Format_Len32	0x10
#define DACChannelL		0x00
#define DACChannelR		0x80
/**************************************************/
#define MSMode 			Slave
#define Ratio 			256//Ratio=MCLK/LRCK
#define Format 			NORMAL_I2S
#define Format_Len		Format_Len16
#define DACChannelSel	DACChannelL
#define ADC_Volume		0//0:0DB -0.5db Step
#define DAC_Volume		0//0:0DB -0.5db Step

#define PLLSet12288		26000//KHZ
/**************************************************/

struct i2c_client es8374_i2c;

#ifdef WIN32
int i2c_open(struct i2c_client *client)
{
    return 0;
}

int i2c_close(struct i2c_client *client)
{
    return 0;
}

int i2c_master_send(struct i2c_client *client, char *buf, u32 len)
{
    return 0;
}
#endif

static int i2c_init(void)
{
    es8374_i2c.addr = 0x10;
    es8374_i2c.index = 1;

    return i2c_open(&es8374_i2c);
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

static int i2c_write(unsigned int reg, unsigned int value)
{
    u8 buf[2];
    int ret;

    CHECK_CLIENT((&es8374_i2c))
    buf[0] = reg;
    buf[1] = value;

    ret = i2c_master_send(&es8374_i2c, buf, 2);
    if (ret < 0) {
        aicx_error(0x107c88, "xxd", "i2c_write failed, reg=0x%x, value=0x%x, ret=%d\n",
                   reg, value, ret);
        return ret;
    }

    return 0;
}

static unsigned int i2c_read(unsigned int reg)
{
    u8 buf[2];
    u8 reg_buf[2];
    int ret;

    CHECK_CLIENT((&es8374_i2c))

    /* aic_mdelay(20); */
    reg_buf[0] = reg;
    buf[0] = 0;
    ret = i2c_send_then_read(&es8374_i2c, reg_buf, buf, 1);
    if (ret < 0) {
        aicx_error(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);
        return ret;
    }

    return (u32)buf[0];
}

void ES8374_Codec(void)
{
	I2CWRNBYTE_CODEC(0x00,0x3F);	//IC Rst start
	I2CWRNBYTE_CODEC(0x00,0x03);	//IC Rst stop

	if(Ratio == 1536)//Ratio=MCLK/LRCK=153612M288-8K
	{
		I2CWRNBYTE_CODEC(0x01,0x7F);	//IC clk on
		I2CWRNBYTE_CODEC(0x02,0x00);
		I2CWRNBYTE_CODEC(0x03,0x20);
		I2CWRNBYTE_CODEC(0x04,0x00);
		I2CWRNBYTE_CODEC(0x05,0x66);	//clk div set
		I2CWRNBYTE_CODEC(0x06,0x06);
		I2CWRNBYTE_CODEC(0x07,0x00);	//LRCK=MCLK/6/256
	}
	if(Ratio == 768)//Ratio=MCLK/LRCK=76812M288-16K24M576-32K6M144-8K;
	{
		I2CWRNBYTE_CODEC(0x01,0x7F);	//IC clk on
		I2CWRNBYTE_CODEC(0x02,0x00);
		I2CWRNBYTE_CODEC(0x03,0x20);
		I2CWRNBYTE_CODEC(0x04,0x00);
		I2CWRNBYTE_CODEC(0x05,0x33);	//clk div set
		I2CWRNBYTE_CODEC(0x06,0x03);
		I2CWRNBYTE_CODEC(0x00,0x00);	//LRCK=MCLK/768
	}
	if(Ratio == 512)//Ratio=MCLK/LRCK=51212M288-24K24M576-48K8M192-16K
	{
		I2CWRNBYTE_CODEC(0x01,0x7F);	//IC clk on
		I2CWRNBYTE_CODEC(0x02,0x00);
		I2CWRNBYTE_CODEC(0x03,0x20);
		I2CWRNBYTE_CODEC(0x04,0x00);
		I2CWRNBYTE_CODEC(0x05,0x22);	//clk div set
		I2CWRNBYTE_CODEC(0x06,0x02);
		I2CWRNBYTE_CODEC(0x00,0x00);	//LRCK=MCLK/2/256
	}
	if(Ratio == 256)//Ratio=MCLK/LRCK=25612M288-48K4M096-16K; 2M048-8K
	{
		I2CWRNBYTE_CODEC(0x01,0x7F);	//IC clk on
		I2CWRNBYTE_CODEC(0x02,0x00);
		I2CWRNBYTE_CODEC(0x03,0x20);
		I2CWRNBYTE_CODEC(0x04,0x00);
		I2CWRNBYTE_CODEC(0x05,0x11);	//clk div set
		I2CWRNBYTE_CODEC(0x06,0x01);
		I2CWRNBYTE_CODEC(0x00,0x00);	//LRCK=MCLK/256
	}

	I2CWRNBYTE_CODEC(0x10,Format_Len16+NORMAL_I2S);	//16B-IISģʽ
	I2CWRNBYTE_CODEC(0x11,Format_Len16+NORMAL_I2S);	//16B-IISģʽ

	if(PLLSet12288 == 26000)//13M12M288 Ratio12M288/LRCKѡ,ͨGPIO2PLLMCLKȷϲ
	{
		I2CWRNBYTE_CODEC(0x01,0xFF);	//26MڲDIV213000
		I2CWRNBYTE_CODEC(0x6F,0xA0);	//pll set:mode enable
		I2CWRNBYTE_CODEC(0x72,0x41);	//pll set:mode set
		I2CWRNBYTE_CODEC(0x09,0x01);	//pll set:reset on ,set start
		I2CWRNBYTE_CODEC(0x0C,0x17);	//pll set:k
		I2CWRNBYTE_CODEC(0x0D,0xA3);	//pll set:k
		I2CWRNBYTE_CODEC(0x0E,0x2F);	//pll set:k
		I2CWRNBYTE_CODEC(0x0A,0x88);	//pll set:
		I2CWRNBYTE_CODEC(0x0B,0x07);	//pll set:n
		I2CWRNBYTE_CODEC(0x09,0x41);	//pll set:reset off ,set stop
		I2CWRNBYTE_CODEC(0x02,0x08);	//pll set:use pll
	}
	if(PLLSet12288 == 13000)//13M12M288 Ratio12M288/LRCKѡ,ͨGPIO2PLLMCLKȷϲ
	{
		I2CWRNBYTE_CODEC(0x6F,0xA0);	//pll set:mode enable
		I2CWRNBYTE_CODEC(0x72,0x41);	//pll set:mode set
		I2CWRNBYTE_CODEC(0x09,0x01);	//pll set:reset on ,set start
		I2CWRNBYTE_CODEC(0x0C,0x17);	//pll set:k
		I2CWRNBYTE_CODEC(0x0D,0xA3);	//pll set:k
		I2CWRNBYTE_CODEC(0x0E,0x2F);	//pll set:k
		I2CWRNBYTE_CODEC(0x0A,0x88);	//pll set:
		I2CWRNBYTE_CODEC(0x0B,0x07);	//pll set:n
		I2CWRNBYTE_CODEC(0x09,0x41);	//pll set:reset off ,set stop
		I2CWRNBYTE_CODEC(0x02,0x08);	//pll set:use pll
	}
	if(PLLSet12288 == 12000)//12M12M288 Ratio12M288/LRCKѡ,ͨGPIO2PLLMCLKȷϲ
	{
		I2CWRNBYTE_CODEC(0x6F,0xA0);	//pll set:mode enable
		I2CWRNBYTE_CODEC(0x72,0x41);	//pll set:mode set
		I2CWRNBYTE_CODEC(0x09,0x01);	//pll set:reset on ,set start
		I2CWRNBYTE_CODEC(0x0C,0x08);	//pll set:k
		I2CWRNBYTE_CODEC(0x0D,0x13);	//pll set:k
		I2CWRNBYTE_CODEC(0x0E,0xE0);	//pll set:k
		I2CWRNBYTE_CODEC(0x0A,0x88);	//pll set:
		I2CWRNBYTE_CODEC(0x0B,0x08);	//pll set:n
		I2CWRNBYTE_CODEC(0x09,0x41);	//pll set:reset off ,set stop
		I2CWRNBYTE_CODEC(0x02,0x08);	//pll set:use pll
	}

	I2CWRNBYTE_CODEC(0x0F,MSMode+0x04);	//SCLK=MCLK/4


	I2CWRNBYTE_CODEC(0x24,0x08);	//adc set
	I2CWRNBYTE_CODEC(0x36,0x00);	//dac set
	I2CWRNBYTE_CODEC(0x12,0x30);	//timming set
	I2CWRNBYTE_CODEC(0x13,0x20);	//timming set
	I2CWRNBYTE_CODEC(0x21,0x50);	//adc set:	SEL LIN1 CH+PGAGAIN=0DB
	I2CWRNBYTE_CODEC(0x22,0xFF);	//adc set:	PGA GAIN=0DB
	I2CWRNBYTE_CODEC(0x21,0x10);	//adc set:	SEL LIN1 CH+PGAGAIN=0DB
	I2CWRNBYTE_CODEC(0x00,0x80);	// IC START
	DELAY_MS(30);  					//DELAY_MS
	I2CWRNBYTE_CODEC(0x14,0x8A);	// IC START
	I2CWRNBYTE_CODEC(0x15,0x40);	// IC START
	I2CWRNBYTE_CODEC(0x1A,0xA0);	// monoout set
	I2CWRNBYTE_CODEC(0x1B,0x19);//5V 0DB
	I2CWRNBYTE_CODEC(0x1C,0x90);
	I2CWRNBYTE_CODEC(0x1D,0x2B);//5V 0DB 8R=>1W
	I2CWRNBYTE_CODEC(0x1F,0x00);	// spk set
	I2CWRNBYTE_CODEC(0x1E,0xA0);	// spk on
	I2CWRNBYTE_CODEC(0x28,0x00);	// alc set
	I2CWRNBYTE_CODEC(0x25,ADC_Volume);	// ADCVOLUME  on
	I2CWRNBYTE_CODEC(0x38,DAC_Volume);	// DACVOLUMEL on
	I2CWRNBYTE_CODEC(0x37,DACChannelSel+0x00);	// dac set
	I2CWRNBYTE_CODEC(0x6D,0x60);	//SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT
}

void ES8374_StandBy(void)//--ES8374_Resume(void)//ָ
{
	I2CWRNBYTE_CODEC(0x38,0xC0);
	I2CWRNBYTE_CODEC(0x25,0xC0);
	I2CWRNBYTE_CODEC(0x15,0x00);
	I2CWRNBYTE_CODEC(0x28,0x1C);
	I2CWRNBYTE_CODEC(0x36,0x20);
	I2CWRNBYTE_CODEC(0x37,0x20);
	I2CWRNBYTE_CODEC(0x6D,0x00);
	I2CWRNBYTE_CODEC(0x09,0x80);
	I2CWRNBYTE_CODEC(0x1A,0x08);
	I2CWRNBYTE_CODEC(0x1E,0x20);
	DELAY_MS(5);
	I2CWRNBYTE_CODEC(0x1C,0x10);
	I2CWRNBYTE_CODEC(0x1D,0x10);
	I2CWRNBYTE_CODEC(0x1E,0x40);
	I2CWRNBYTE_CODEC(0x24,0x20);
	I2CWRNBYTE_CODEC(0x22,0x00);
	I2CWRNBYTE_CODEC(0x21,0xC0);
	I2CWRNBYTE_CODEC(0x15,0xFF);
	I2CWRNBYTE_CODEC(0x14,0x16);
	I2CWRNBYTE_CODEC(0x01,0x03);
}

void ES8374_Resume(void)//ָ(δµ)--ES8374_StandBy(void)
{
	I2CWRNBYTE_CODEC(0x01,0x7F);
	I2CWRNBYTE_CODEC(0x14,0x8A);
	I2CWRNBYTE_CODEC(0x15,0x40);
	I2CWRNBYTE_CODEC(0x21,0x10);
	I2CWRNBYTE_CODEC(0x22,0xFF);
	I2CWRNBYTE_CODEC(0x24,0x08);
	I2CWRNBYTE_CODEC(0x24,0xA0);
	I2CWRNBYTE_CODEC(0x1D,0x2B);
	I2CWRNBYTE_CODEC(0x1C,0x90);
	I2CWRNBYTE_CODEC(0x1A,0xA0);
	I2CWRNBYTE_CODEC(0x09,0x41);
	I2CWRNBYTE_CODEC(0x6D,0x60);
	I2CWRNBYTE_CODEC(0x37,0x00);
	I2CWRNBYTE_CODEC(0x36,0x00);
	I2CWRNBYTE_CODEC(0x28,0x00);
	I2CWRNBYTE_CODEC(0x38,0x00);
	I2CWRNBYTE_CODEC(0x25,0x00);

}
void ES8374_ADC(void)//ES_KM2
{
	I2CWRNBYTE_CODEC(0x00,0x3F);
	DELAY_MS(1);
	I2CWRNBYTE_CODEC(0x00,0x03);
	I2CWRNBYTE_CODEC(0x01,0x7A);
	I2CWRNBYTE_CODEC(0x05,0x11);
	I2CWRNBYTE_CODEC(0x24,0x08);
	I2CWRNBYTE_CODEC(0x12,0x40);
	I2CWRNBYTE_CODEC(0x13,0x40);
	I2CWRNBYTE_CODEC(0x22,0xFF);
	I2CWRNBYTE_CODEC(0x21,0x10);
	I2CWRNBYTE_CODEC(0x00,0x81);
	DELAY_MS(50);
	I2CWRNBYTE_CODEC(0x15,0x62);
	I2CWRNBYTE_CODEC(0x16,0x1F);
	I2CWRNBYTE_CODEC(0x17,0x43);
	I2CWRNBYTE_CODEC(0x14,0x8A);
	I2CWRNBYTE_CODEC(0x28,0x00);
	I2CWRNBYTE_CODEC(0x25,0x00);

}

void ES8374_DAC(void)//ES_KM3
{
	I2CWRNBYTE_CODEC(0x00,0x3F);
	DELAY_MS(1);
	I2CWRNBYTE_CODEC(0x00,0x03);
	I2CWRNBYTE_CODEC(0x01,0x7F);
	I2CWRNBYTE_CODEC(0x05,0x11);
	I2CWRNBYTE_CODEC(0x36,0x00);
	I2CWRNBYTE_CODEC(0x12,0x40);
	I2CWRNBYTE_CODEC(0x13,0x40);
	I2CWRNBYTE_CODEC(0x00,0x80);
	DELAY_MS(50);
	I2CWRNBYTE_CODEC(0x15,0x40);
	I2CWRNBYTE_CODEC(0x14,0x8A);
	I2CWRNBYTE_CODEC(0x1A,0xA0);
	I2CWRNBYTE_CODEC(0x17,0xA8);
	I2CWRNBYTE_CODEC(0x1B,0x19);//5V 0DB
	I2CWRNBYTE_CODEC(0x1C,0x90);
	I2CWRNBYTE_CODEC(0x1D,0x2B);//5V 0DB 8R=>1W
	I2CWRNBYTE_CODEC(0x1F,0x00);
	I2CWRNBYTE_CODEC(0x1E,0xA0);
	I2CWRNBYTE_CODEC(0x28,0x80);
	I2CWRNBYTE_CODEC(0x26,0x2C);
	I2CWRNBYTE_CODEC(0x38,0x00);
	I2CWRNBYTE_CODEC(0x37,0x00);

}

void ES8374_VOLINC(void)//ES_KM5
{
    unsigned int RDDATA;
	RDDATA=I2CRDNBYTE_CODEC(ES8374_DACVOLUME);			//ES8374 volĴ0X11
	DELAY_MS(1);
	if(RDDATA == 0)	  //0XC0
	{
	}
	else		  //slave
	{
	RDDATA--;
	}
	I2CWRNBYTE_CODEC(ES8374_DACVOLUME, RDDATA);			//дES8374 volĴ0X11

}

void ES8374_VOLDEC(void)//ES_KM6
{
    unsigned int RDDATA;
	RDDATA=I2CRDNBYTE_CODEC(ES8374_DACVOLUME);			//ES8374 volĴ0X11
	DELAY_MS(1);

	if(RDDATA == 192)	  //master mode
	{
	}
	else		  //slave
	{
	RDDATA++;
	}

	I2CWRNBYTE_CODEC(ES8374_DACVOLUME, RDDATA);			//дES8374 volĴ0X11
}

void ES8374_DMIC(void)//ES_KM8
{
	I2CWRNBYTE_CODEC(0x00,0x3F);
	DELAY_MS(1);
	I2CWRNBYTE_CODEC(0x00,0x03);
	I2CWRNBYTE_CODEC(0x01,0x7F);
	I2CWRNBYTE_CODEC(0x05,0x11);
	I2CWRNBYTE_CODEC(0x24,0x8A);
	I2CWRNBYTE_CODEC(0x36,0x00);
	I2CWRNBYTE_CODEC(0x12,0x40);
	I2CWRNBYTE_CODEC(0x13,0x40);
	I2CWRNBYTE_CODEC(0x21,0x50);
	I2CWRNBYTE_CODEC(0x22,0x00);
	I2CWRNBYTE_CODEC(0x21,0x10);
	I2CWRNBYTE_CODEC(0x00,0x80);
	DELAY_MS(50);
	I2CWRNBYTE_CODEC(0x15,0x40);
	I2CWRNBYTE_CODEC(0x14,0x8A);
	I2CWRNBYTE_CODEC(0x1A,0xB0);
	I2CWRNBYTE_CODEC(0x1B,0x19);//5V 0DB
	I2CWRNBYTE_CODEC(0x1C,0x90);
	I2CWRNBYTE_CODEC(0x1D,0x2B);//5V 0DB 8R=>1W
	I2CWRNBYTE_CODEC(0x1F,0x00);
	I2CWRNBYTE_CODEC(0x1E,0xA0);
	I2CWRNBYTE_CODEC(0x25,0x00);
	I2CWRNBYTE_CODEC(0x26,0x50);
	I2CWRNBYTE_CODEC(0x28,0x80);
	I2CWRNBYTE_CODEC(0x38,0x00);
	I2CWRNBYTE_CODEC(0x37,0x00);
	I2CWRNBYTE_CODEC(0x6D,0x40);
}
//************************Parameter Setting************************//
//----------EVEREST EQ Mode Setting : ES8374 EQ ADCSingle Mode
//----------LRCK Frequency  : 16000
//----------EQ1 Mode        : EQ1_NC
//----------EQ1 Frequency   : 4.000000e-01~4
//----------EQ2 Mode        : EQ2_NC
//----------EQ2 Frequency   : 4.000000e-01~4
void ES8374_EQ_FileName(void)
{
  I2CWRNBYTE_CODEC(0x36,0x20);//adc mute
  I2CWRNBYTE_CODEC(0x45,0x04);
  I2CWRNBYTE_CODEC(0x46,0x64);
  I2CWRNBYTE_CODEC(0x47,0x79);
  I2CWRNBYTE_CODEC(0x48,0xFF);
  I2CWRNBYTE_CODEC(0x49,0x1F);
  I2CWRNBYTE_CODEC(0x4A,0xF7);
  I2CWRNBYTE_CODEC(0x4B,0xFD);
  I2CWRNBYTE_CODEC(0x4C,0xFF);
  I2CWRNBYTE_CODEC(0x4D,0x25);
  I2CWRNBYTE_CODEC(0x4E,0x6C);
  I2CWRNBYTE_CODEC(0x4F,0x39);
  I2CWRNBYTE_CODEC(0x50,0xFF);
  I2CWRNBYTE_CODEC(0x51,0x00);
  I2CWRNBYTE_CODEC(0x52,0x19);
  I2CWRNBYTE_CODEC(0x53,0x94);
  I2CWRNBYTE_CODEC(0x54,0xED);
  I2CWRNBYTE_CODEC(0x55,0x22);
  I2CWRNBYTE_CODEC(0x56,0x4A);
  I2CWRNBYTE_CODEC(0x57,0x9F);
  I2CWRNBYTE_CODEC(0x58,0xAE);
  I2CWRNBYTE_CODEC(0x59,0x04);
  I2CWRNBYTE_CODEC(0x5A,0x64);
  I2CWRNBYTE_CODEC(0x5B,0x79);
  I2CWRNBYTE_CODEC(0x5C,0xFF);
  I2CWRNBYTE_CODEC(0x5D,0x1F);
  I2CWRNBYTE_CODEC(0x5E,0xF7);
  I2CWRNBYTE_CODEC(0x5F,0xFD);
  I2CWRNBYTE_CODEC(0x60,0xFF);
  I2CWRNBYTE_CODEC(0x61,0x25);
  I2CWRNBYTE_CODEC(0x62,0x6C);
  I2CWRNBYTE_CODEC(0x63,0x39);
  I2CWRNBYTE_CODEC(0x64,0xFF);
  I2CWRNBYTE_CODEC(0x65,0x00);
  I2CWRNBYTE_CODEC(0x66,0x19);
  I2CWRNBYTE_CODEC(0x67,0x94);
  I2CWRNBYTE_CODEC(0x68,0xED);
  I2CWRNBYTE_CODEC(0x69,0x22);
  I2CWRNBYTE_CODEC(0x6A,0x4A);
  I2CWRNBYTE_CODEC(0x6B,0x9F);
  I2CWRNBYTE_CODEC(0x6C,0xAE);
  I2CWRNBYTE_CODEC(0x2D,0x85);
  I2CWRNBYTE_CODEC(0x36,0x00);//adc unmute
}
/**************************************************************************ES8374 END*******************************/
/**************************************************************************ES8374 END*******************************/

/**************************************************************************ES END***********************************/
/**************************************************************************ES END***********************************/



/********************************************************************************************************************/
/********************************************************************************************************************/
/********************************************************************************************************************/
#include <aic_assert.h>

#include "..\iis\iis.h"

#define BCLK_PAIR_COUNT     13
#define VALID_BCLK_DIV_MAX  18

const u8 bclk_pair[BCLK_PAIR_COUNT][2] = {
    { 19, 20 },
    { 20, 22 },
    { 21, 24 },
    { 22, 25 },
    { 23, 30 },
    { 24, 32 },
    { 25, 33 },
    { 26, 34 },
    { 27, 36 },
    { 28, 44 },
    { 29, 48 },
    { 30, 66 },
    { 31, 72 },
};

struct {
    u32 mclk;
    u32 srate;
    u32 bit_width;
    u32 chn;
    u32 dac_vol;
    u32 adc_vol;
} es8374_config;

union iis_control es8374_control;
static u32 g_8374_open_cnt;

void es8374_set_control(u32 val)
{
    es8374_control.value = val;
}

void es8374_set_mclk(u32 mclk)
{
    es8374_config.mclk = mclk;
}

void es8374_set_srate(u32 srate)
{
    es8374_config.srate = srate;
}

void es8374_set_width(u32 bit_wid)
{
    es8374_config.bit_width = bit_wid;
}

void es8374_set_chn(u32 chn)
{
    /* keep to be 2, mono work as lrpack in cpu side */
    es8374_config.chn = 2;
}

void es8374_reset(void)
{
    I2CWRNBYTE_CODEC(0x00,0x3F);
    DELAY_MS(5);
    I2CWRNBYTE_CODEC(0x00,0x03);

    return;
}

void es8374_config_clk(void)
{
    u32 ratio;

    I2CWRNBYTE_CODEC(0x01, 0x7F); /* IC clk on */
    I2CWRNBYTE_CODEC(0x02, 0x00); /* all is default */
    I2CWRNBYTE_CODEC(0x03, 0x20); /* ADC OSR */
    I2CWRNBYTE_CODEC(0x04, 0x00); /* Filter counter */

    ratio = es8374_config.mclk / es8374_config.srate;
    aicx_debug(0x0bcdc5, "ddd", "mclk=%d, srate=%d, ratio=%d\n",
               es8374_config.mclk, es8374_config.srate, ratio);
    /* Ratio=MCLK/LRCK=1536: 12M288-8K; */
    if(ratio == 1536)
    {
        I2CWRNBYTE_CODEC(0x05, 0x66); /* clk div set, seemed for OSR only */
        I2CWRNBYTE_CODEC(0x06, 0x06); /* reg06 and reg07 is LRCLK_DIV(Ratio) */
        I2CWRNBYTE_CODEC(0x07, 0x00); /* LRCK=MCLK/6/256 */
    }

    /* Ratio=MCLK/LRCK=768; 12M288-16K; 24M576-32K; 6M144-8K; */
    else if(ratio == 768)
    {
        I2CWRNBYTE_CODEC(0x05, 0x33);
        I2CWRNBYTE_CODEC(0x06, 0x03);
        I2CWRNBYTE_CODEC(0x07, 0x00); /* LRCK=MCLK/768 */
    }

    /* Ratio=MCLK/LRCK=512; 12M288-24K; 24M576-48K; 8M192-16K */
    else if(ratio == 512)
    {
        I2CWRNBYTE_CODEC(0x05, 0x22);
        I2CWRNBYTE_CODEC(0x06, 0x02);
        I2CWRNBYTE_CODEC(0x07, 0x00); /* LRCK=MCLK/2/256 */
    }

    /* Ratio=MCLK/LRCK=256; 12M288-48K; 4M096-16K; 2M048-8K */
    else if(ratio == 256)
    {
        I2CWRNBYTE_CODEC(0x05, 0x11);
        I2CWRNBYTE_CODEC(0x06, 0x01);
        I2CWRNBYTE_CODEC(0x07, 0x00); /* LRCK=MCLK/256 */
    }

    else
        aicx_error(0x0f6da8, "", "cannot set right clock ratio!\n");
}

void es8374_dac_vol(u8 vol)
{
    I2CWRNBYTE_CODEC(0x38, vol);
}

void es8374_adc_vol(u32 vol)
{
    I2CWRNBYTE_CODEC(0x25, vol);
}

u8 get_bclk_div(u8 div)
{
    int i;

    if (div <= 18)
        return div;
    for (i = 0; i < BCLK_PAIR_COUNT; i++)
        if(bclk_pair[i][1] == div)
            return bclk_pair[i][0];

    //AIC_ASSERT(0);

    return bclk_pair[BCLK_PAIR_COUNT - 1][0];
}

static void es8374_printf_reg(void)
{
    int i;

    for (i = 0; i <= 0x3f; i++)
        aicx_info(0x11ab44, "xx", "8374 reg 0x%2x: 0x%2x\n", i, I2CRDNBYTE_CODEC(i));
}

void es8374_open(void)
{
    u32 chn, bclk, bclk_div;
    u8 cfg, mode;

    g_8374_open_cnt++;
    if (g_8374_open_cnt > 1)
        return;

    i2c_init();

    es8374_config.bit_width = 16;

    es8374_reset();
    es8374_config_clk();

    /* set SDP left to out */
    I2CWRNBYTE_CODEC(0x36, 0x00);

    if (es8374_config.bit_width == 16)
        cfg = Format_Len16;
    else if (es8374_config.bit_width == 32)
        cfg = Format_Len32;
    else
        return;

    /* IIS mode */
    if (es8374_config.chn == 2) {
        cfg += NORMAL_I2S;
    } else
        cfg += NORMAL_DSPA;

    I2CWRNBYTE_CODEC(0x10, cfg);
    I2CWRNBYTE_CODEC(0x11, cfg);

    mode = MSMode;

    if (es8374_config.chn == 2)
        chn = 2;
    else
        chn = 1;
    bclk = es8374_config.srate * chn * es8374_config.bit_width;
    bclk_div = es8374_config.mclk / bclk;
    bclk_div = get_bclk_div(bclk_div);
    mode += bclk_div;
    I2CWRNBYTE_CODEC(0x0F, mode);

    I2CWRNBYTE_CODEC(0x24, 0x08); /* adc set */
    //I2CWRNBYTE_CODEC(0x36, 0x00); /* dac set */
    I2CWRNBYTE_CODEC(0x12, 0x30); /* timming set */
    I2CWRNBYTE_CODEC(0x13, 0x20); /* timming set */
    I2CWRNBYTE_CODEC(0x21, 0x50); /* adc set: SEL LIN1 CH+PGAGAIN=0DB */
    I2CWRNBYTE_CODEC(0x22, 0x06); /* adc set: PGA GAIN=0DB(0xFF), 18DB(0x06) */
    I2CWRNBYTE_CODEC(0x21, 0x10); /* adc set: SEL LIN1 CH+PGAGAIN=0DB */
    I2CWRNBYTE_CODEC(0x00, 0x80); /* IC START */
    DELAY_MS(30);
    I2CWRNBYTE_CODEC(0x14, 0x8A); /* IC START */
    I2CWRNBYTE_CODEC(0x15, 0x40); /* IC START */
    I2CWRNBYTE_CODEC(0x1A, 0xA0); /* monoout set */
    I2CWRNBYTE_CODEC(0x1B, 0x19); /* 5V 0DB */
    I2CWRNBYTE_CODEC(0x1C, 0x88); /* 0x90 */
    I2CWRNBYTE_CODEC(0x1D, 0x2B); /* 5V 0DB 8R=>1W */
    I2CWRNBYTE_CODEC(0x1F, 0x00); /* spk set */
    I2CWRNBYTE_CODEC(0x1E, 0xA0); /* spk on */
    I2CWRNBYTE_CODEC(0x28, 0x00); /* alc set */
    I2CWRNBYTE_CODEC(0x25, ADC_Volume); /* ADCVOLUME  on */
    I2CWRNBYTE_CODEC(0x38, DAC_Volume); /* DACVOLUMEL on */
    I2CWRNBYTE_CODEC(0x37, DACChannelSel + 0x00); /* dac set */
    I2CWRNBYTE_CODEC(0x6D, 0x60); /* GPIO1=DMIC CLK OUT + GPIO2=PLL CLK OUT */
    es8374_printf_reg();
}

void es8374_close(void)
{
    if (g_8374_open_cnt > 0)
        g_8374_open_cnt--;
    if (g_8374_open_cnt)
        return;

    ES8374_StandBy();
    i2c_close(&es8374_i2c);
    return;
}

int es8374_set_params(struct snd_dpm_context *dpm, u32 wid, u32 chn, u32 srate)
{
    es8374_set_srate(srate);
    es8374_set_width(wid);
    es8374_set_chn(chn);

    return 0;
}

u32 es8374_codec_read(struct snd_codec *codec, u32 reg)
{
    return 0;
}

int es8374_codec_write(struct snd_codec *codec, u32 reg, u32 val)
{
    return 0;
}


int es8374_stream_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        /* TODO: add ref count */
        es8374_open();
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        es8374_close();
        break;
    default:
        break;
    }

    return 0;
}

#ifdef WIN32
static struct snd_codec_driver es8374_codec_driver = {
    /* driver ops */
    NULL, //int (*probe)(struct snd_codec *);
    NULL, //int (*remove)(struct snd_codec *);
    NULL, //int (*suspend)(struct snd_codec *);
    NULL, //int (*resume)(struct snd_codec *);

    /* Default control and setup, added after probe() is run */
    NULL, //const struct snd_kcontrol_new *controls;
    0, //int num_controls;
    NULL, //const struct snd_dpm_widget *dpm_widgets;
    0, //int num_dpm_widgets;
    NULL, //const struct snd_dpm_route *dpm_routes;
    0, //int num_dpm_routes;

    /* codec wide operations */
    NULL, //int (*set_sysclk)(struct snd_codec *codec,
                      //int clk_id, int source, unsigned int freq, int dir);
    NULL, //int (*set_pll)(struct snd_codec *codec, int pll_id, int source,
                   //unsigned int freq_in, unsigned int freq_out);

    /* codec IO */
    NULL, //unsigned int (*read)(struct snd_codec *, unsigned int);
    NULL, //int (*write)(struct snd_codec *, unsigned int, unsigned int);

    /* codec bias level */
    NULL, //int (*set_bias_level)(struct snd_codec *, enum snd_bias_level level);
    0, //bool idle_bias_off;

    NULL, //void (*seq_notifier)(struct snd_dpm_context *, int, int);

    es8374_set_params, //int (*set_params)(struct snd_dpm_context *dpm,
                      //struct snd_stream_params *params);
    /* codec stream completion event */
    es8374_stream_event, //int (*stream_event)(struct snd_dpm_context *dpm, int event);

    /* probe ordering - for components with runtime dependencies */
    0, //int probe_order;
    0, //int remove_order;
};
#else
static struct snd_codec_driver es8374_codec_driver = {
    .read = NULL,
    .write = NULL,
    .controls = NULL,
    .num_controls = 0,
    .dpm_widgets = NULL,
    .num_dpm_widgets = 0,
    .dpm_routes = NULL,
    .num_dpm_routes = 0,
    .read = es8374_codec_read,
    .write = es8374_codec_write,
    .set_params = es8374_set_params,
    .stream_event = es8374_stream_event,
};
#endif

int es8374_codec_init(void)
{
    es8374_config.mclk = EXT_MCLK;
    es8374_config.bit_width = 16;

    return snd_card_register_codec("es8374_codec", &es8374_codec_driver);
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(es8374_codec_init);

