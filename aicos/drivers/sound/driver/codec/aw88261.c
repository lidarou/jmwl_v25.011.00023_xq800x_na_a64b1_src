
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0aa666
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
#include <limits.h>
#include <i2c.h>
#include <pmu_user.h>
#include <audio/snd_config.h>

#include "snd_card.h"
#include "snd_dpm_driver.h"
#include "aw88261.h"

#define CHECK_REG(r)    \
    if ((r > AW_REG_DACCFG4 && r < AW_REG_DACST) || (r > AW_REG_PVDD && r < AW_REG_BSTCTL1) || (r > AW_REG_BSTCTL2)) {  \
        alog_error("wrong codec reg = 0AW_REG_DACSTx%x\n", r);    \
        return 0;           \
    }
#define AW88261_MCLK_FREQ            24576000
#define AW88261_I2C_SLAVE_ADDR       0x34
#define AW88261_I2C_ID               3

#define CHECK_CLIENT(c)     \
    if (!c->data)   \
        return AIC_EPERM;

struct aw_ctx {
    struct i2c_client client;
    struct snd_codec *codec;
    u32 srate;
    u32 brate;
    u16 wid;
    u16 chn;
};

struct aw_ctx aw_ctx;

static int aw88261_i2c_init(struct i2c_client *client)
{
    client->addr = AW88261_I2C_SLAVE_ADDR;
    client->index = AW88261_I2C_ID;

    return i2c_open(client);
}

static int aw88261_send_then_read(struct i2c_client *client, u8 *reg ,u8 *buf)
{
    int ret;

    struct i2c_msg msg[2] = {
       {
        .addr = client->addr,
        .flags = 0,
        .len = 1,
        .buf = reg,
       }, {
         .addr = client->addr,
        .flags = I2C_M_RD,
        .len = 2,
        .buf = buf,
       }
    };

    CHECK_CLIENT(client)
    ret = i2c_transfer(client, &msg[0], 2);
    if (ret < 0)
        aicx_error(0x15bb9c, "d", "codec read failed, ret=%d\n", ret);

    return ret;
}

static int aw88261_i2c_write(struct i2c_client *client,
                             unsigned int reg, u32 value)
{
    u8 buf[3];
    int ret;

    SNX_INFO(0x1702f5, "xx", "aw88261: r=0x%x, v=0x%x\n", reg, value);

    CHECK_CLIENT(client)
    buf[0] = reg;
    buf[1] = (value >> 8) & 0xff;
    buf[2] = value & 0xff;

    ret = i2c_master_send(client, buf, 3);
    if (ret < 0) {
        SNX_ERROR(0x068418, "xxd", "i2c_write failed, reg=0x%02x, value=0x%02x, ret=%d\n",
                  reg, value, ret);
        return ret;
    }

    return 0;
}

static u32 aw88261_i2c_read(struct i2c_client *client, u32 reg)
{
    u8 buf[4];
    u8 reg_buf[2];
    int ret;

    CHECK_CLIENT(client)

    reg_buf[0] = reg;
    buf[0] = 0;
    ret = aw88261_send_then_read(client, reg_buf, (u8 *)buf);
    if (ret < 0) {
        SNX_ERROR(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);

        return ret;
    }

    return ((u32)buf[0] << 8) | buf[1];
}

u32 aw88261_show(u32 reg)
{
    u32 i = 0;

    if (reg != (u32)(-1)) {
        CONS_OUT("0x%02x 0x%04x\n", reg, aw88261_i2c_read(&aw_ctx.client, reg));
        return 0;
    }
    for (i = 0; i < AW88261_REG_NUM; i++)
        CONS_OUT("0x%02x 0x%04x\n", i, aw88261_i2c_read(&aw_ctx.client, i));

    for (i = 0x20; i <= 0x23; i++)
        CONS_OUT("0x%02x 0x%04x\n", i, aw88261_i2c_read(&aw_ctx.client, i));

    for (i = 0x60; i <= 0x61; i++)
        CONS_OUT("0x%02x 0x%04x\n", i, aw88261_i2c_read(&aw_ctx.client, i));

    return 0;
}

static int awr(int argc, char **argv)
{
    u32 reg = 0, val = 0;

    if (argc == 0)
        aw88261_show((u32)(-1));
    else if (argc == 1)
        aw88261_show(atoi(argv[0]));
    else {
        reg = atoi(argv[0]);
        val = atoi(argv[1]);
        aw88261_i2c_write(&aw_ctx.client, reg, val);
    }

    return 0;
}
CONSOLE_EXPORT_CMD(awr, awr, aw codec read write);

static u32 aw88261_read(struct snd_codec *codec, u32 reg)
{
    struct aw_ctx *ctx =
        (struct aw_ctx *)snd_card_get_codec_data(codec);

    CHECK_REG(reg)

    return aw88261_i2c_read(&ctx->client, reg);
}

static int aw88261_write(struct snd_codec *codec, u32 reg, u32 val)
{
    struct aw_ctx *ctx =
        (struct aw_ctx *)snd_card_get_codec_data(codec);

    CHECK_REG(reg)

    return aw88261_i2c_write(&ctx->client, reg, val);
}

u16 get_srate(void)
{
    u16 srate;

    switch(aw_ctx.srate) {
    case 8000:
        srate = 0;
        break;
    case 11000:
        srate = 1;
        break;
    case 12000:
        srate = 2;
        break;
    case 16000:
        srate = 3;
        break;
    case 22000:
        srate = 4;
        break;
    case 24000:
        srate = 5;
        break;
    case 32000:
        srate = 6;
        break;
    case 44000:
        srate = 7;
        break;
    case 48000:
        srate = 8;
        break;
    case 96000:
        srate = 9;
        break;
    default:
        srate = 3; /* 16000 */
        break;
    }

    return srate;
}

int aw88261_set_params(struct snd_dpm_context *dpm, u32 wid, u32 chn, u32 srate)
{
    aw_ctx.srate = srate;
    aw_ctx.wid = wid;
    aw_ctx.chn = chn;

    return 0;
}

void aw88261_open()
{
    struct i2c_client *client = &aw_ctx.client;
    u16 val;

    val = BIT(6); /* i2s en */
    aw88261_i2c_write(client, AW_REG_SYSCTL, val);

    val = get_srate(); /* 16bit, 2chn */
    aw88261_i2c_write(client, AW_REG_I2SCTL1, val);

    return;
}

void aw88261_close()
{
    struct i2c_client *client = &aw_ctx.client;

    aw88261_i2c_write(client, AW_REG_SYSCTL, 0);

    return;
}

int aw88261_stream_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        /* TODO: add ref count */
        aw88261_open();
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        aw88261_close();
        break;
    default:
        break;
    }

    return 0;
}

static int aw88261_probe(struct snd_codec *codec)
{
    snd_card_set_codec_data(codec, (u32)&aw_ctx);
    aw_ctx.codec = codec;

    return 0;
}

static struct snd_codec_driver aw88261_codec_driver = {
    .probe = aw88261_probe,
    .controls = NULL,
    .num_controls = 0,
    .dpm_widgets = NULL,
    .num_dpm_widgets = 0,
    .dpm_routes = NULL,
    .num_dpm_routes = 0,
    .read = aw88261_read,
    .write = aw88261_write,
    .set_params = aw88261_set_params,
    .stream_event = aw88261_stream_event,
};

int aw88261_codec_init(void)
{
    aw88261_i2c_init(&aw_ctx.client);

    return snd_card_register_codec("aw88261_codec", &aw88261_codec_driver);
}
AIC_NORMAL_DRV_P1_INIT_EXPORT(aw88261_codec_init);


