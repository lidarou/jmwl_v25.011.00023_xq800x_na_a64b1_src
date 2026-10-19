/******************************************************************************
 *
 *  Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.
 *
 *****************************************************************************/

/******************************************************************************
 *
 *  DESCRIPTION:
 *
 *     This file is aw87390 driver file.
 *
 *****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e4609
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

#include "snd_card.h"
#include "snd_dpm_driver.h"
#include "snd_ext_pa.h"
#include "aw87390.h"

#define AW_CODEC_NAME   "aw codec"

#define REG_AW_ID       0x00
#define REG_AW_SYSCTRL  0x01
#define REG_AW_MODECTRL 0x02
#define REG_AW_CPOVP    0x03
#define REG_AW_CPP      0x04
#define REG_AW_GAIN     0x05
#define REG_AW_AGC3_PO  0x06
#define REG_AW_AGC3     0x07
#define REG_AW_AGC2_PO  0x08
#define REG_AW_AGC2     0x09
#define REG_AW_AGC1     0x0A
#define REG_AW_MAX      REG_AW_AGC1
#define REG_AW_VTH      0x63

#define CHECK_AW_REG(reg)   \
    if (reg > REG_AW_MAX && reg != REG_AW_VTH)  \
        return AIC_EPERM;

#define CHECK_CLIENT(c)     \
    if (!c && !c->data) \
        return AIC_EPERM;

struct i2c_client g_aw_i2c_client;

#ifdef CONFIG_AW87390_PRIMARY_CODEC
static const DECLARE_TLV_DB_SCALE(ovp_volt,
                6000, 250, 0);
static const DECLARE_TLV_DB_SCALE(pa_gain,
                0, 1500, 0);
static const DECLARE_TLV_DB_SCALE(agc3,
                100, 100, 0);
static const DECLARE_TLV_DB_SCALE(agc2,
                400, 200, 0);

/* aw87390 control for setting param */
static const struct snd_kcontrol_new aw87390_controls[] = {
    SOC_SINGLE_TLV("aw ovp volt", REG_AW_CPOVP,
            0, 0x0a, 0, ovp_volt),
    SOC_SINGLE_TLV("aw pa gain", REG_AW_GAIN,
            0, 0x12, 0, pa_gain),
    SOC_SINGLE_TLV("agc3", REG_AW_AGC3_PO,
            0, 0x0f, 0, agc3),
    SOC_SINGLE("agc3 rt", REG_AW_AGC3, 5, 7, 1),
    SOC_SINGLE("agc3 at", REG_AW_AGC3, 2, 7, 1),
    SOC_SINGLE("agc3 fast", REG_AW_AGC3, 0, 3, 1),
    SOC_SINGLE_TLV("agc2", REG_AW_AGC2_PO,
            0, 0x09, 0, agc2),
    SOC_SINGLE("agc2 at", REG_AW_AGC2, 2, 7, 1),
    SOC_SINGLE("agc2 fast", REG_AW_AGC2, 0, 3, 1),
    SOC_SINGLE("agc1 at", REG_AW_AGC1, 1, 3, 1),
    SOC_SINGLE("pd agc1", REG_AW_AGC1, 0, 1, 1),
};
#endif

static int aw_i2c_send_then_read(struct i2c_client *client, u8 *reg,
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
        aicx_error(0x1738c5, "d", "aw PW read failed, ret=%d\n", ret);

    return ret;
}

static int aw_i2c_write(struct i2c_client *client,
                        unsigned int reg, unsigned int value)
{
    u8 buf[2];
    int ret;

    aicx_info(0x12d2be, "xx", "aw: r=0x%x, v=0x%x\n",
               reg, value);

    CHECK_CLIENT(client)
    buf[0] = reg;
    buf[1] = value;

    ret = i2c_master_send(client, buf, 2);
    if (ret < 0) {
        aicx_error(0x1b2429, "xxd", "aw i2c_write failed, reg=0x%02x, value=0x%02x, ret=%d\n",
                   reg, value, ret);
        return ret;
    }

    return 0;
}

static unsigned int aw_i2c_read(struct i2c_client *client, unsigned int reg)
{
    u8 buf[2];
    u8 reg_buf[2];
    int ret;

    CHECK_CLIENT(client)

    reg_buf[0] = reg;
    buf[0] = 0;
    ret = aw_i2c_send_then_read(client, reg_buf, buf, 1);
    if (ret < 0) {
        aicx_error(0x0a3d5f, "dd", "i2c_read failed, reg=%d, ret=%d\n", reg, ret);

        return 0;
    }

    aicx_info(0x12d2be, "xx", "aw: r=0x%x, v=0x%x\n",
               reg, (u32)buf[0]);

    return (u32)buf[0];
}

/* for external module */
int aw_write(unsigned int reg, unsigned int value)
{
    CHECK_AW_REG(reg)

    return aw_i2c_write(&g_aw_i2c_client, reg, value);
}

/* for external module */
unsigned int aw_read(unsigned int reg)
{
    CHECK_AW_REG(reg)

    return aw_i2c_read(&g_aw_i2c_client, reg);
}

#ifdef CONFIG_AW87390_PRIMARY_CODEC
static u32 aw_codec_read(struct snd_codec *codec, u32 reg)
{
    struct i2c_client *client =
        (struct i2c_client *)snd_card_get_codec_data(codec);

    CHECK_AW_REG(reg)
    CHECK_CLIENT(client)

    return aw_i2c_read(client, reg);
}

static int aw_codec_write(struct snd_codec *codec, u32 reg, u32 val)
{
    struct i2c_client *client =
        (struct i2c_client *)snd_card_get_codec_data(codec);

    CHECK_AW_REG(reg)
    CHECK_CLIENT(client)

    return aw_i2c_write(client, reg, val);
}
#endif

static int aw_pa_on(u32 data)
{
    struct i2c_client *client = (struct i2c_client *)data;

    CHECK_CLIENT(client)

    return aw_i2c_write(client, 1, 7);
}

static int aw_pa_off(u32 data)
{
    struct i2c_client *client = (struct i2c_client *)data;

    CHECK_CLIENT(client)

    return aw_i2c_write(client, 1, 6);
}

#ifdef CONFIG_AW87390_PRIMARY_CODEC
static int aw_pa_event(struct snd_dpm_context *dpm, int event)
{
    switch (event) {
    case SND_DPM_STREAM_START:
    case SND_DPM_STREAM_RESUME:
        /* TODO: add ref count */
        aw_pa_on(dpm->codec->private);
        break;
    case SND_DPM_STREAM_STOP:
    case SND_DPM_STREAM_PAUSE_PUSH:
        /* TODO: add ref count */
        aw_pa_off(dpm->codec->private);
        break;
    default:
        break;
    }

    return 0;
}


static struct snd_codec_driver aw_codec_driver = {
    .controls = aw87390_controls,
    .num_controls = ARRAY_SIZE(aw87390_controls),
    .read = aw_codec_read,
    .write = aw_codec_write,
    .stream_event = aw_pa_event,
};
#endif

static int aw_i2c_init(void)
{
    g_aw_i2c_client.addr = AW_I2C_SLAVE_ADDR;
    g_aw_i2c_client.index = AW_I2C_ID;

    if (!i2c_open(&g_aw_i2c_client))
        aw_write(2, 0x1c);

#ifdef CONFIG_AW87390_PRIMARY_CODEC
    snd_card_register_codec(AW_CODEC_NAME, &aw_codec_driver);
    snd_card_set_codec_data_by_name(AW_CODEC_NAME,
                                    (u32)&g_aw_i2c_client);
#else
    snd_ext_pa_reg(aw_pa_on, aw_pa_off, (u32)&g_aw_i2c_client);
    /* snd_card_register_aux_codec(AW_CODEC_NAME, &aw_codec_driver); */
#endif

    return 0;
}
AIC_NORMAL_DRV_L1_INIT_EXPORT(aw_i2c_init);


