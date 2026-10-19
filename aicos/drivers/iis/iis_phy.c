/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * IIS physical functions.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b3b1c
#define LOG_MODULE    AMOD_I2S

#include <aic_io.h>
#include <aic_init.h>
#include <aic_assert.h>
#include <aic_errno.h>
#include <irq.h>
#include <top_glb.h>
#include <dma.h>
#include <aic_clk_map.h>
#include <aic_iot_clk.h>

#include "iis_phy.h"
#include "iis_cfg.h"
#include "iis_compat.h"
#include "snd_card.h"
#include "snd_dpm.h"
#include "snd_dpm_driver.h"
#include "snd_dai.h"
#include "snd_stream.h"
#ifdef CONFIG_SOUND_CONV
#include "snd_data_conv.h"
#endif

#define DIR_PLAY    1
#define DIR_RECORD  2

#define IIS_CTL_STANDARD(ctl)       \
    ctl.control.ws_polar = 0;       \
    ctl.control.ws_mode = 1;        \
    ctl.control.data_ws_del = 1;    \
    ctl.control.data_align = 0;

#define IIS_CTL_L_JUSTIFY(ctl)      \
    ctl.control.ws_polar = 1;       \
    ctl.control.ws_mode = 1;        \
    ctl.control.data_ws_del = 0;    \
    ctl.control.data_align = 0;

#define IIS_CTL_R_JUSTIFY(ctl)      \
    ctl.control.ws_polar = 1;       \
    ctl.control.ws_mode = 1;        \
    ctl.control.data_ws_del = 0;    \
    ctl.control.data_align = 1;

#define IIS_CTL_DSP_MODE(ctl)       \
    ctl.control.ws_polar = 0;       \
    ctl.control.ws_mode = 0;        \
    ctl.control.data_ws_del = 1;    \
    ctl.control.data_align = 0;

#define IIS_CTL_LRPACK(ctl)         \
    ctl.control.lr_pack = 1;

#define IIS_WRITE(reg, val)         iis_write(client, reg, val)
#define IIS_READ(reg)               iis_read(client, reg)
#define IIS_UPDATE(reg, val, mask)  iis_update(client, reg, val, mask)

#define IIS_DAI_TYPE_PCM    1
#define IIS_DAI_TYPE_COMPR  0

#define GET_IIS_SEL_BY_TYPE(type)   type

#define COMPRESS_DAI_FLAG       0x434d5052 /* CMPR */
#define COMPRESS_IN_DAI_FLAG    0x434d5049 /* CMPI */

struct iis_info {
    u32 total_cnt;
    struct iis_cfg *cfg;
};

struct iis_client {
    struct iis_cfg *cfg;
    u32 srate;
    u32 mod_en_cnt;

    u32 opened  : 1;
    u32 wid     : 6;
    u32 chn     : 2;
    u32 wmk_p   : 5;
    u32 wmk_r   : 5;
    u32 dir     : 2;
    u32 ref_tx  : 5;
    u32 ref_rx  : 5;

    u32 control; /* for debug */
};

const char *iis_codec_name[] = {
    "iis0 codec",
    "iis1 codec",
};

struct iis_info g_iis_info;
struct iis_client iis_client[IIS_PORT_NUM];

struct snd_dai_cfg iis_dai_cfg[] = {
    {
        REG_BASE_TOP_IIS0 + REG_TX_ADDR,
        DMA_REQ_TOP_IIS0_TX,
    },
    {
        REG_BASE_TOP_IIS0 + REG_RX_ADDR,
        DMA_REQ_TOP_IIS0_RX,
    },
};

static u32 iis_read(struct iis_client *client, u32 reg)
{
    if (!client)
        return 0;
#ifndef WIN32
    return AIC_RAW_READ32(client->cfg->reg_base + reg);
#else
    return 0;
#endif
}

static void iis_write(struct iis_client *client, u32 reg, u32 val)
{
    if (!client)
        return;
#ifndef WIN32
    AIC_RAW_WRITE32(client->cfg->reg_base + reg, val);
#endif
}

static void iis_update(struct iis_client *client, u32 reg, u32 value, u32 mask)
{
    u32 read_val;

    if (!client)
        return;

    read_val = IIS_READ(reg);
    read_val = (value & mask) | (read_val & ~mask);
    IIS_WRITE(reg, read_val);
}

static void iis_printf_all(struct iis_client *client)
{
    int i;
    for (i = 0; i <= REG_TDM_FD_DIR; i = i + 4) {
        aicx_debug(0x074335, "xx", "-0x%2x: %08x\n", i, IIS_READ(i));
    }
}

static void iis_tx_rx_enable(struct iis_client *client, BOOL en, u32 direct)
{
#ifndef IIS_HALF_DUPLEX
    if (en)
        IIS_UPDATE(REG_I2S_CTRL_FDX, FULL_DUPLEX | (1 << direct),
                   FULL_DUPLEX | (1 << direct));
    else
        IIS_UPDATE(REG_I2S_CTRL_FDX, 0, 1 << direct);
#endif
}

static void iis_enalbe(struct iis_client *client, BOOL en)
{
    IIS_UPDATE(REG_I2S_CTRL, en ? I2S_EN : 0, I2S_EN);
    if (en)
        iis_printf_all(client);
}

static void iis_reset(struct iis_client *client)
{
    IIS_WRITE(REG_I2S_CTRL, 0);
    IIS_WRITE(REG_I2S_CTRL_FDX, 0);
}

void iis_mod_enable(struct iis_client *client, bool en)
{
    /* TODO: add lock */
    if (en) {
        if (client->mod_en_cnt == 0) {
            if (CLK_IIS_EN)
                clk_pll_enable(CLK_IIS_EN);
#ifdef CONFIG_CHECK_APLL
            extern void aic_check_apll(void);
            aic_check_apll();
#endif
            CLK_SET_IIS_RATE_COMP(CLK_IIS_PLL, client->cfg->mclk);
            CLK_SET_IIS_PARAENT(CLK_IIS_PLL, CLK_IIS_PARENT);
            AIC_WRITE32(client->cfg->reg_en, client->cfg->bit_en);
            iis_reset(client);
        }
        client->mod_en_cnt++;
    }
    else {
        if (client->mod_en_cnt == 1) {
            if (CLK_IIS_EN)
                clk_pll_disable(CLK_IIS_EN);
            AIC_WRITE32(client->cfg->reg_en, 0);
        }
        if (client->mod_en_cnt)
            client->mod_en_cnt--;
    }
}

void iis_dma_sel(u32 dev_id, u32 type)
{
#ifdef CONFIG_ADSP_MNG
    if (dev_id == 0)
        AIC_WRITE32(REG_IIS0_DMA_SEL, GET_IIS_SEL_BY_TYPE(type));
    else
        AIC_WRITE32(REG_IIS1_DMA_SEL, GET_IIS_SEL_BY_TYPE(type));
#endif
}
static void iis_set_control(struct iis_client *client, u32 control)
{
    IIS_WRITE(REG_I2S_CTRL, control);
}

void iis_set_bitwidth(struct iis_client *client, u32 wid)
{
#ifdef IIS_HALF_DUPLEX
    IIS_WRITE(REG_I2S_SRES, wid - 1);
#else
    /* keep tx and rx same */
    IIS_WRITE(REG_I2S_SRES, wid - 1);
    IIS_WRITE(REG_I2S_SRES_FDR, wid - 1);
#endif
}

void iis_set_srate(struct iis_client *client, u32 sample_rate)
{
    u32 rate;

    if (!client)
        return;

    AIC_ASSERT(sample_rate);
    AIC_ASSERT(client->cfg->mclk);
    AIC_ASSERT(client->wid);
    AIC_ASSERT(client->chn);

    client->srate = sample_rate;
    /* work as 2 channels always */
    rate = client->cfg->mclk / (client->wid * 2 * sample_rate);
    IIS_WRITE(REG_I2S_SRATE, rate);
}

void iis_set_aempty(struct iis_client *client, u32 count)
{
    IIS_WRITE(REG_FIFO_AEMPTY, count);
}

void iis_set_afull(struct iis_client *client, u32 count)
{
#ifdef IIS_HALF_DUPLEX
    IIS_WRITE(REG_FIFO_AFULL, count);
#else
    IIS_WRITE(REG_FIFO_AFULL_FDR, count);
#endif
}

#ifdef IIS_EXTEND

void iis_set_tx_int(struct iis_client *client, BOOL en)
{
    u32 val = INTREQ_MASK | FIFO_EMPTY_MASK | FIFO_AEMPTY_MASK;
    if (en)
        IIS_UPDATE(REG_I2S_CTRL, val, val);
    else
        IIS_UPDATE(REG_I2S_CTRL, 0, val);
}

static u32 int_cnt = 0;
ISR_RET_E iis_isr_handler(void *param)
{

    struct iis_client *client = (struct iis_client *)param;
    u32 i;
    u32 stat = IIS_READ(REG_I2S_STAT);

    int_cnt++;

    /* TODO: write 0 for test */
    for (i = 0; i < IIS_FIFO_DEPTH - client->wmk_p; i++)
        IIS_WRITE(REG_TX_ADDR, 0);

    IIS_WRITE(REG_I2S_STAT, stat);

    return 0;
}

u32 iis_get_int_cnt(void)
{
    return int_cnt;
}

static void iis_set_rx_int(struct iis_client *client, BOOL en)
{
    u32 val = INTREQ_MASK | FIFO_FULL_MASK | FIFO_AFULL_MASK;
    if (en)
        IIS_UPDATE(REG_I2S_CTRL, val, val);
    else
        IIS_UPDATE(REG_I2S_CTRL, 0, val);
}

static void iis_set_rx_int_fdx(struct iis_client *client, BOOL en)
{
    u32 val = RI2S_MASK | RFIFO_FULL_MASK | RFIFO_AFULL_MASK;
    if (en)
        IIS_UPDATE(REG_I2S_CTRL_FDX, val, val);
    else
        IIS_UPDATE(REG_I2S_CTRL_FDX, 0, val);
}

static void iis_set_control_fdx(struct iis_client *client, u32 control)
{
    iis_cfg.control_fdx = control;
    IIS_WRITE(REG_I2S_CTRL_FDX, control);
}


static void iis_set_bitwidth_fdr(struct iis_client *client, u32 wid)
{
    if (wid > 32)
        wid = 32;
    iis_cfg.wid_fdr = wid;
    IIS_WRITE(REG_I2S_SRES_FDR, wid - 1);
}

static u32 iis_read_state(struct iis_client *client)
{
    return IIS_READ(REG_I2S_STAT);
}

static void iis_clr_state(struct iis_client *client)
{
    IIS_WRITE(REG_I2S_STAT, 0xFFFFFFFF);
}

static u32 iis_read_level(void)
{
    if (iis_cfg.control_fdx)
        return IIS_READ(REG_FIFO_LEVEL_FDR);
    return IIS_READ(REG_FIFO_LEVEL);
}

static u32 iis_read_tx_level(struct iis_client *client)
{
    return IIS_READ(REG_FIFO_LEVEL);
}

static u32 iis_read_rx_level(struct iis_client *client)
{
    if (iis_cfg.control_fdx)
        return IIS_READ(REG_FIFO_LEVEL_FDR);
    return IIS_READ(REG_FIFO_LEVEL);
}

static u32 iis_read_level_fdr(struct iis_client *client)
{
    return IIS_READ(REG_FIFO_LEVEL_FDR);
}

static void iis_set_aempty_fdr(struct iis_client *client, u32 count)
{
    IIS_WRITE(REG_FIFO_AEMPTY_FDR, count);
}

static void iis_set_afull_fdr(u32 count)
{
    IIS_WRITE(REG_FIFO_AFULL_FDR, count);
}

static void iis_enable_loop(struct iis_client *client, BOOL en)
{
    u32 val = 0;

    if (en)
        val = LOOP_L_SWITCH | LOOP_R_SWITCH;
    IIS_UPDATE(REG_I2S_SRES_FDR, val,
               LOOP_L_SWITCH | LOOP_R_SWITCH);
}

static void iis_enable_sdo_mplxer(struct iis_client *client,
                                  BOOL en, int val_l, int val_r)
{
    int mask, val;

    if (en) {
        val = (val_l << 22) | (val_r << 26) | SDO_MULTIPLEXER_SWTITCH;
        mask = GENMASK(22, 24) | GENMASK(26, 28) | SDO_MULTIPLEXER_SWTITCH;
    } else {
        val = 0;
        mask = SDO_MULTIPLEXER_SWTITCH;
    }
    IIS_UPDATE(REG_I2S_SRES_FDR, val, mask);
}

static void iis_enable_sdo_phase(struct iis_client *client, BOOL en_l, BOOL en_r)
{
    int val = 0;

    if (en_l)
        val |= SDO_L_PHASE;
    if (en_r)
        val |= SDO_R_PHASE;
    IIS_UPDATE(REG_I2S_SRES_FDR, val, SDO_L_PHASE | SDO_R_PHASE);
}

static void iis_set_side_dvc(struct iis_client *client, u32 dvc_l, u32 dvc_r)
{
    u32 mask, val;

    val = (dvc_l << 0) | (dvc_r << 16);
    mask = GENMASK(0, 8) | GENMASK(16, 24);
    IIS_UPDATE(REG_SIDE_CFG, val, mask);
}

static void iis_set_side_hpf(struct iis_client *client, u32 coff_l, u32 coff_r)
{
    u32 mask, val;

    val = (coff_l << 9) | (coff_r << 25);
    mask = GENMASK(9, 12) | GENMASK(25, 28);
    IIS_UPDATE(REG_SIDE_CFG, val, mask);
}

static void iis_enable_side_hpf(struct iis_client *client, BOOL en)
{
    u32 val = 0;

    if (en)
        val = HPF_SWITCH_L | HPF_SWITCH_R;
    IIS_UPDATE(REG_SIDE_CFG, val,
               HPF_SWITCH_L | HPF_SWITCH_R);
}

static void iis_enable_sidetone(struct iis_client *client, BOOL en)
{
    u32 val = 0;

    if(en)
        val = (0x3 << 14) | (0x3 << 30);
    IIS_UPDATE(REG_SIDE_CFG, val,
               GENMASK(14, 15) | GENMASK(30, 31));
}
#endif

void iis_fill_zero(struct iis_client *client)
{
    int i;

    for (i = 0; i < IIS_FIFO_DEPTH; i++)
        IIS_WRITE(REG_TX_ADDR, 0);
}

int iis_open(struct snd_dai *dai)
{
    struct iis_client *client;
    int i;

    if (!dai) {
        aicx_error(0x06ac9b, "", "iis open failed, invalide param\n");
        return AIC_EPERM;
    }

    for (i = 0; i < g_iis_info.total_cnt; i++) {
        if (dai->dev_id == g_iis_info.cfg[i].id) {
            client = &iis_client[i];
            client->chn = 2;
            client->wid = IIS_DEFAULT_WID;
            client->wmk_p = IIS_AEMPTY_LEVEL;
            client->wmk_r = IIS_AFULL_LEVEL;
            client->srate = IIS_DEFAULT_SRATE;

            dai->private = (u32)client;
            iis_mod_enable(client, TRUE);
            aicx_info(0x0c4b8e, "ddd", "iis open, mod_en_cnt=%d, ref_tx=%d, ref_tx=%d\n",
                      client->mod_en_cnt, client->ref_tx, client->ref_rx);

            break;
        }
    }

    return 0;
}

static int iis_set_param(struct snd_dai *dai, u32 width, u32 chn, u32 srate)
{
    struct iis_client *client;

    if (!dai)
        return AIC_EPERM;

    /* TODO: check param */

    client = (struct iis_client *)dai->private;
    client->wid = width;
    client->chn = chn;
#ifdef CONFIG_SOUND_CONV
    if (chn == 1) {
        client->srate = srate << (pcm_conv_multi() - 1);
        client->chn = 2;
    } else if (chn == 2) {
        client->srate = srate << pcm_conv_multi();
    } else
        AICX_ASSERT_IFX(0, 0x147ca4, "d", "chn must be 1 or 2, chn=%d\n", chn);
#else
#ifdef CONFIG_MUSIC_FIX_48K
    if (dai->cfg == COMPRESS_DAI_FLAG) {
        client->srate = 48000;
        client->chn = 2;
    } else
#endif
#ifdef CONFIG_RECORD_FIX_16K
    if (dai->cfg == COMPRESS_IN_DAI_FLAG) {
        client->srate = 16000;
        client->chn = 1;
    } else
#endif
    client->srate = srate;
#endif

    return AIC_SUCCESS;
}

static int iis_start(struct snd_dai *dai)
{
    struct iis_client *client;
    union iis_control ctl_info;

    if (!dai)
        return AIC_EIO;
    client = (struct iis_client *)dai->private;;

    iis_dma_sel(dai->dev_id, dai->type);

    aicx_info(0x1b8695, "ddd", "iis start, chn=%d, srate=%d, dir=%d\n",
              client->chn, client->srate, client->dir);

    /* TODO: add lock */
    if (!client->ref_tx && !client->ref_rx) {

        /* only enable at the first time */
        ctl_info = client->cfg->ctl_info;
        IIS_CTL_STANDARD(ctl_info)
        if (client->chn == 1) {
            IIS_CTL_LRPACK(ctl_info)
        }

        if (dai->direct == SND_STREAM_PLAYBACK) {
            ctl_info.control.dir_cfg = 1;
            client->ref_tx++;
        } else {
            ctl_info.control.dir_cfg = 0;
            client->ref_rx++;
        }
        /* save for debug */
        client->control = ctl_info.value;
        iis_set_control(client, ctl_info.value);
        iis_set_bitwidth(client, client->wid);
        iis_set_srate(client, client->srate);

        iis_set_aempty(client, client->wmk_p);
        iis_set_afull(client, client->wmk_r);

        client->dir = 1 << dai->direct;
        iis_tx_rx_enable(client, TRUE, client->dir);
        iis_enalbe(client, TRUE);

    } else {
        /* could not set sample rate here for it was alread runned. */
        if ((dai->direct == SND_STREAM_PLAYBACK)) {
            if (client->ref_tx == 0) {
                iis_tx_rx_enable(client, TRUE, DIR_PLAY);
                client->dir |= DIR_PLAY;
            }
            client->ref_tx++;
        }

        if ((dai->direct == SND_STREAM_CAPTURE)) {
            if (client->ref_rx == 0) {
                iis_tx_rx_enable(client, TRUE, DIR_RECORD);
                client->dir |= DIR_RECORD;
            }
            client->ref_rx++;
        }
    }
    /* TODO: unlock */

    aicx_info(0x0e023d, "dddd", "iis start, mod_en_cnt=%d, ref_tx=%d, ref_rx=%d, dir=%d\n",
              client->mod_en_cnt, client->ref_tx, client->ref_rx, client->dir);

    return AIC_SUCCESS;
}

static int iis_stop(struct snd_dai *dai)
{
    struct iis_client *client;

    if (!dai)
        return AIC_EIO;
    client = (struct iis_client *)dai->private;
    if (!client->ref_tx && !client->ref_rx)
        return AIC_SUCCESS;

    if ((dai->direct == SND_STREAM_PLAYBACK)) {
        if (client->ref_tx == 1) {
            iis_tx_rx_enable(client, FALSE, DIR_PLAY);
            client->dir &= ~DIR_PLAY;
        }
        if (client->ref_tx)
            client->ref_tx--;
    }

    if ((dai->direct == SND_STREAM_CAPTURE)) {
        if (client->ref_rx == 1) {
            iis_tx_rx_enable(client, FALSE, DIR_RECORD);
            client->dir &= ~DIR_RECORD;
        }
        if (client->ref_rx)
            client->ref_rx--;
    }

    if (client->ref_tx == 0 && client->ref_rx == 0)
        iis_enalbe(client, FALSE);

    aicx_info(0x08fe70, "dddd", "iis stop, mod_en_cnt=%d, ref_tx=%d, ref_rx=%d, dir=%d\n",
              client->mod_en_cnt, client->ref_tx, client->ref_rx, client->dir);

    return AIC_SUCCESS;
}

static int iis_close(struct snd_dai *dai)
{
    struct iis_client *client;

    if (!dai)
        return AIC_EPERM;
    client = (struct iis_client *)dai->private;
    iis_mod_enable(client, FALSE);

    aicx_info(0x032c7b, "ddd", "iis close, mod_en_cnt=%d, ref_tx=%d, ref_rx=%d\n",
              client->mod_en_cnt, client->ref_tx, client->ref_rx);

    return AIC_SUCCESS;
}

#ifndef WIN32
static const char *const multip_texts[] = {
    "LO", "RO", "ZERO", "L+R", "L-R",
};

static const struct snd_kcontrol_new side_switch_l[] = {
    SOC_SINGLE_EXT("switch", REG_SIDE_CFG, SIDE_SWITCH_L_S, 1, 0,
        snd_dpm_get_volsw, snd_dpm_put_volsw),
};

static const struct snd_kcontrol_new side_switch_r[] = {
    SOC_SINGLE_EXT("switch", REG_SIDE_CFG, SIDE_SWITCH_R_S, 1, 0,
        snd_dpm_get_volsw, snd_dpm_put_volsw)
};

static const struct soc_enum multi_l =
    SOC_ENUM_SINGLE(REG_I2S_SRES_FDR, SDO_L_MULTIPLEXER_S, 4, multip_texts);

static const struct soc_enum multi_r =
    SOC_ENUM_SINGLE(REG_I2S_SRES_FDR, SDO_R_MULTIPLEXER_S, 4, multip_texts);

/* widget define for iis route */
const struct snd_dpm_widget iis_widget[] = {
    SND_DPM_MIXER("side_l", SND_REG_NONE,
        0, 0, &side_switch_l[0], ARRAY_SIZE(side_switch_l)),

    SND_DPM_MIXER("side_r", SND_REG_NONE,
        0, 0, &side_switch_r[0], ARRAY_SIZE(side_switch_r)),

    SND_DPM_PGA("side_tone_in", SND_REG_NONE,
        0, 0, NULL, 0),

    SND_DPM_PGA("side_tone_out", SND_REG_NONE,
        0, 0, NULL, 0),
};

static const struct snd_dpm_route iis_dapm_routes[] = {
    {"side_l", "switch", "side_tone_in"},
    {"side_tone_out", NULL, "side_l"},

    {"side_r", "switch", "side_tone_in"},
    {"side_tone_out", NULL, "side_r"},

    /* side tone */
    {"DAC SDP SRC", NULL, "side_tone_out"},
    {"side_tone_in", NULL, "SDP OUT MUX"},
};

static const struct snd_kcontrol_new iis_control[] = {
    SOC_SINGLE("sdo_l_phase", REG_I2S_SRES_FDR, SDO_L_PHASE_S, 1, 0),
    SOC_SINGLE("sdo_r_phase", REG_I2S_SRES_FDR, SDO_R_PHASE_S, 1, 0),
    SOC_ENUM("multi_l", multi_l),
    SOC_ENUM("multi_r", multi_r),
    SOC_SINGLE("multi_switch", REG_I2S_SRES_FDR, SDO_MULTIPLEXER_SWTITCH_S, 1, 0),
    SOC_SINGLE("loop_l", REG_I2S_SRES_FDR, LOOP_L_SWITCH_S, 1, 0),
    SOC_SINGLE("loop_r", REG_I2S_SRES_FDR, LOOP_L_SWITCH_S, 1, 0),

    SOC_SINGLE("dvc_coff_l", REG_SIDE_CFG, DVC_COFF_L_S, 511, 0),
    SOC_SINGLE("dvc_coff_r", REG_SIDE_CFG, DVC_COFF_R_S, 511, 0),
    SOC_SINGLE("hpf_coff_l", REG_SIDE_CFG, HPF_COFF_L_S, 15, 0),
    SOC_SINGLE("hpf_coff_r", REG_SIDE_CFG, HPF_COFF_R_S, 15, 0),
    SOC_SINGLE("hpf_switch_l", REG_SIDE_CFG, HPF_SWITCH_L_S, 1, 0),
    SOC_SINGLE("hpf_switch_r", REG_SIDE_CFG, HPF_SWITCH_R_S, 1, 0),
};

#endif

u32 iis_codec_read(struct snd_codec *codec, u32 reg)
{
    struct iis_client *client =
        (struct iis_client *)snd_card_get_codec_data(codec);
    u32 val;

    CHECK_IIS_REG(reg)

    if (!client)
        return 0;

    iis_mod_enable(client, TRUE);
    val = AIC_RAW_READ32(client->cfg->reg_base + reg);
    iis_mod_enable(client, FALSE);

    return val;
}

int iis_codec_write(struct snd_codec *codec, u32 reg, u32 val)
{
    struct iis_client *client =
        (struct iis_client *)snd_card_get_codec_data(codec);

    CHECK_IIS_REG(reg)

    if (!client)
        return 0;

    iis_mod_enable(client, TRUE);
    AIC_RAW_WRITE32(client->cfg->reg_base + reg, val);
    iis_mod_enable(client, FALSE);

    return 0;
}

static struct snd_codec_driver iis_codec_driver = {
    .controls = iis_control,
    .num_controls = ARRAY_SIZE(iis_control),
    .dpm_widgets = iis_widget,
    .num_dpm_widgets = ARRAY_SIZE(iis_widget),
    .dpm_routes = iis_dapm_routes,
    .num_dpm_routes = ARRAY_SIZE(iis_dapm_routes),
    .read = iis_codec_read,
    .write = iis_codec_write,
};

static void iis_init_cfg(void)
{
    int i;

    /* g_iis_info.total_cnt = sizeof(iis_cfgs) / sizeof(struct iis_cfg); */
    g_iis_info.total_cnt = 1;
    g_iis_info.cfg = &iis_cfgs[0];

    for (i = 0; i < ARRAY_SIZE(iis_client) && i < g_iis_info.total_cnt; i++)
        iis_client[i].cfg = &g_iis_info.cfg[i];
}

#ifdef WIN32
struct snd_dai_ops iis_dai_ops = {
    iis_open,
    iis_close,
    iis_set_param,
    iis_start,
    iis_stop,
};
#else
struct snd_dai_ops iis_dai_ops = {
    .open = iis_open,
    .close = iis_close,
    .start = iis_start,
    .stop = iis_stop,
    .set_params = iis_set_param,
};
#endif

#ifdef WIN32
struct snd_dai iis_dai[] = {
    {
        {NULL, NULL},   //struct list_head list;
        "iis play0",    //char *name;
        "pcm_p0",       //char *stream_name;
        1,              //u32 type;
        0,              //u32 dev_id;
        0,              //u32 direct;
        &iis_dai_ops,   //struct snd_dai_ops *ops;
        0,              //u32 private;
    },
};
#else
struct snd_dai iis_dai[] = {
    {
        .name = "iis dai play0",
        .stream_name = "pcm_p0",
        .type = IIS_DAI_TYPE_PCM,
        .dev_id = 0, /* iis id 0 */
        .direct = SND_STREAM_PLAYBACK,
        .ops = &iis_dai_ops,
        .cfg = (u32)&iis_dai_cfg[0],
    },
    {
        .name = "iis dai record",
        .stream_name = "pcm_c0",
        .type = IIS_DAI_TYPE_PCM,
        .dev_id  = 0,
        .direct = SND_STREAM_CAPTURE,
        .ops = &iis_dai_ops,
        .cfg = (u32)&iis_dai_cfg[1],
    },
    {
        .name = "iis dai compress play0",
        .stream_name = "ipc_compr_playback0",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_PLAYBACK,
        .ops = &iis_dai_ops,
        .cfg = COMPRESS_DAI_FLAG,
    },
    {
        .name = "iis dai compress record",
        .stream_name = "ipc_compr_capture",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_CAPTURE,
        .ops = &iis_dai_ops,
        .cfg = COMPRESS_IN_DAI_FLAG,
    },
    {
        .name = "iis dai compress play1",
        .stream_name = "ipc_compr_playback1",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_PLAYBACK,
        .ops = &iis_dai_ops,
        .cfg = COMPRESS_DAI_FLAG,
    },
    {
        .name = "iis dai voice dl",
        .stream_name = "voice_dl",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_PLAYBACK,
        .ops = &iis_dai_ops,
        .cfg = 0,
    },
    {
        .name = "iis dai voice ul",
        .stream_name = "voice_ul",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_CAPTURE,
        .ops = &iis_dai_ops,
        .cfg = 0,
    },
    {
        .name = "iis dai voip ul",
        .stream_name = "voip_ul",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_CAPTURE,
        .ops = &iis_dai_ops,
        .cfg = COMPRESS_IN_DAI_FLAG,
    },
    {
        .name = "iis dai voip dl",
        .stream_name = "voip_dl",
        .type = IIS_DAI_TYPE_COMPR,
        .dev_id  = 0,
        .direct = SND_STREAM_PLAYBACK,
        .ops = &iis_dai_ops,
        .cfg = COMPRESS_IN_DAI_FLAG,
    },
};
#endif

int iis_init(void)
{
    int i;
    u32 dev_id;

    iis_init_cfg();

    for (i = 0; i < sizeof(iis_dai) / sizeof(struct snd_dai); i++ ) {
        INIT_LIST_HEAD(&iis_dai[i].list);
        snd_dai_register(&iis_dai[i]);

    }

    for (i = 0; i < g_iis_info.total_cnt; i++) {
        dev_id = g_iis_info.cfg[i].id;
        if (dev_id < ARRAY_SIZE(iis_codec_name)) {
            snd_card_register_aux_codec((char *)iis_codec_name[dev_id],
                                        &iis_codec_driver);
            snd_card_set_codec_data_by_name((char *)iis_codec_name[dev_id],
                                            (u32)&iis_client[i]);
        }
    }

    return 0;
}

AIC_NORMAL_DRV_P2_INIT_EXPORT(iis_init);

