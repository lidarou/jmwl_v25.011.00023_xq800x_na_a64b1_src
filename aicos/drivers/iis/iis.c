/*****************************************************************************/
/*                                                                           */
/*   Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of IIS module.
 *   It includes all the IIS CV cases. The IIS CV case configurations
 *   will be configued by aic_iis_config.h in project dir.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17cd90
#define LOG_MODULE    AMOD_I2S

#include <aic_case.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_printf.h>
#include <aic_math.h>
#include <iis_case_cfg.h>
#include <timer.h>

#include "iis.h"
#include "iis_phy.h"

#define RECEIVE_BUFF_LEN    (512 * 1024 + 64)
#define DUPLEX_COUNT        (8000 * 5)

#if (IIS_FIFO_DEPTH == 32)
#define IIS_INT_WMK         14
#else
#define IIS_INT_WMK         8
#endif

#define IIS_BIT_WID_16      16
#define IIS_BIT_WID_32      32
#define DEFAULT_WID         IIS_BIT_WID_16

#define IIS_SRATE_48K       48000
#define IIS_SRATE_32K       32000
#define IIS_SRATE_16K       16000
#define IIS_SRATE_8K        8000

#define SHORT_TEST              0

#define RING_8K                 ((short *)ring8)
#define RING_16K                ((short *)ring16)
#define RING_48K                ((short *)ring48)

#if SHORT_TEST
#define RING_8K_SAMPLES         8000
#define RING_16K_SAMPLES        16000
#define RING_48K_SAMPLES        48000
#else
#define RING_8K_SAMPLES         (sizeof(ring8) / 2)
#define RING_16K_SAMPLES        (sizeof(ring16) / 2)
#define RING_48K_SAMPLES        (sizeof(ring48) / 2)
#endif

#if (IIS_MCLK == 12288000)
#define DEFAULT_RATE            IIS_SRATE_8K
#define DEFAULT_RING            RING_8K
#define DEFAULT_RING_SAMPLES    RING_8K_SAMPLES
#elif (IIS_MCLK == 24576000)
#define DEFAULT_RATE            IIS_SRATE_16K
#define DEFAULT_RING            RING_16K
#define DEFAULT_RING_SAMPLES    RING_16K_SAMPLES
#else
#error "Don't support this MCLK!"
#endif

#define DEFAULT_BUFF            g_receive_buffer
#define DEFAULT_BUFF_SAMPLES    (RECEIVE_BUFF_LEN / 2 - 50)

#define TEST_FUNC(x)    \
    aic_printf("----------%s start!----------\n", __func__, test_ret);    \
    test_ret = x;    \
    if (test_ret)    \
        aic_printf("!!!!!%s, ret=%d, test FAILED!!!!!\n", __func__, test_ret); \
    else    \
        aic_printf("#####%s, test SUCCESS#####\n", __func__);

#define TEST_DECL   \
    if (test_ret)    \
        aic_printf("!!!!!%s, ret=%d, test FAILED!!!!!\n", __func__, test_ret); \
    else    \
        aic_printf("#####%s, test SUCCESS#####\n", __func__);

#define TEST_RETURN \
    return test_ret;

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



union iis_control_fdx {
    u32 value;
    struct {
        u32 full_duplex : 1;
        u32 i2s_ftx_en  : 1;
        u32 i2s_frx_en  : 1;
        u32 reserve1    : 1;
        u32 fifo_rst    : 1;
        u32 reserve2    : 21;

        u32 ri2s_mask   : 1;
        u32 rfifo_empty_mask    : 1;
        u32 rfifo_aempty_mask   : 1;
        u32 rfifo_full_mask     : 1;
        u32 rfifo_afull_mask    : 1;
    } control;
};


const __ALIGNED(2) char ring8[] = {
    #include "1k_8000.h"
};

const __ALIGNED(2) char ring16[] = {
    #include "1k_16000.h"
};

const __ALIGNED(2) char ring48[] = {
    #include "1k_48000.h"
};

const union iis_control iis_master_transmit_control = {
    .control = {
        .dir_cfg = 1,    .ms_cfg = 1,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 1,  .ws_polar = 0,        .data_ws_del = 1,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control iis_slave_transmit_control = {
    .control = {
        .dir_cfg = 1,    .ms_cfg = 0,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 1,  .ws_polar = 0,        .data_ws_del = 1,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control iis_master_receive_control = {
    .control = {
        .dir_cfg = 0,    .ms_cfg = 1,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 1,  .ws_polar = 0,        .data_ws_del = 1,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control iis_slave_receive_control = {
    .control = {
        .dir_cfg = 0,    .ms_cfg = 0,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 1,  .ws_polar = 0,        .data_ws_del = 1,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control iis_master_duplex_control = {
    .control = {
        .dir_cfg = 1,    .ms_cfg = 1,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 0,  .ws_polar = 0,        .data_ws_del = 0,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control iis_slave_duplex_control = {
    .control = {
        .dir_cfg = 1,    .ms_cfg = 0,     .sfr_rst = 1,
        .fifo_rst = 1,   .chn_width = 2,  .ws_mode = 1,         .mono_mode = 0,
        .audio_mode = 0, .sck_polar = 0,  .ws_polar = 0,        .data_ws_del = 0,
        .data_align = 0, .data_order = 0, .host_data_align = 0, .i2s_stb = 0,
    },
};

const union iis_control_fdx iis_full_duplex_tx_control = {
    .control = {
        .full_duplex = 1, .i2s_ftx_en = 1, .i2s_frx_en = 0, .fifo_rst = 1,
    },
};

const union iis_control_fdx iis_full_duplex_rx_control = {
    .control = {
        .full_duplex = 1, .i2s_ftx_en = 0, .i2s_frx_en = 1, .fifo_rst = 1,
    },
};

const union iis_control_fdx iis_full_duplex_control = {
    .control = {
        .full_duplex = 1, .i2s_ftx_en = 1, .i2s_frx_en = 1, .fifo_rst = 1,
    },
};

static u32 test_ret;
static short g_receive_buffer[RECEIVE_BUFF_LEN];
extern void es8374_open(void);
extern void es8374_set_control(u32 val);
extern void es8374_set_mclk(u32 mclk);
extern void es8374_set_srate(u32 srate);
extern void es8374_close();

static int iis_transmit(u32 control, u32 fdx_control, u32 wid, u32 srate,
                        short *buff, u32 samples)
{
    u32 sent;

    if (!buff || !samples) {
        aic_printf("iis_transmit: error param!");
        return AIC_EIO;
    }

    iis_reset();
    iis_reset_fdx();

    if (fdx_control)
        iis_set_control_fdx(fdx_control);
    iis_set_control(control);
    iis_set_bitwidth(wid);
    iis_set_srate(srate);

    es8374_set_control(control);
    es8374_set_srate(srate);
    es8374_open();

    iis_enalbe(TRUE);
    sent = iis_send(buff, samples);
    iis_enalbe(FALSE);
    es8374_close();

    if (sent != samples) {
        aic_printf("iis_transmit: control=%d, fdx_control=%d, wid=%d, srate=%d\n",
                   control, fdx_control, wid, srate);
        aic_printf("iis_transmit: total=%d, sent=%d\n",
                   samples, sent);
    }

    return !(sent == samples);
}

int iis_transmit_int(u32 control, u32 fdx_control, u32 wid, u32 srate,
                     u32 wmk, short *buff, u32 samples)
{
    u32 sent;

    if (!buff || !samples) {
        aic_printf("iis_transmit: error param!");
        return AIC_EIO;
    }

    iis_reset();
    iis_reset_fdx();

    if (!fdx_control)
        iis_set_control_fdx(fdx_control);
    iis_set_control(control);

    es8374_set_control(control);
    es8374_set_srate(srate);
    //es8374_open();
    iis_set_bitwidth(wid);
    iis_set_srate(srate);
    iis_fill_zero();
    iis_set_aempty(wmk);
    iis_set_aempty_fdr(wmk);

    iis_clr_state();

    //iis_enalbe(TRUE);

    sent = iis_send_int(buff, samples);

    //iis_enalbe(FALSE);
    //es8374_close();

    if (sent != samples) {
        aic_printf("iis_transmit: control=%d, fdx_control=%d, wid=%d, srate=%d\n",
                   control, fdx_control, wid, srate);
        aic_printf("iis_transmit: total=%d, sent=%d\n",
                   samples, sent);
    }

    return !(sent == samples);
}

static int iis_receive(u32 control, u32 fdx_control, u32 wid, u32 srate,
                       short *buff, u32 samples)
{
    u32 received;

    if (!buff || !samples) {
        aic_printf("iis_receive: error param!");
        return AIC_EIO;
    }

    iis_reset();
    iis_reset_fdx();

    if (fdx_control)
        iis_set_control_fdx(fdx_control);
    iis_set_control(control);
    iis_set_bitwidth(wid);
    iis_set_srate(srate);

    es8374_set_control(control);
    es8374_set_srate(srate);
    es8374_open();

    iis_enalbe(TRUE);
    received = iis_recv(buff, samples);
    iis_enalbe(FALSE);
    es8374_close();

    if (received != samples) {
        aic_printf("iis_receive: control=%d, fdx_control=%d, wid=%d, srate=%d\n",
                   control, fdx_control, wid, srate);
        aic_printf("iis_receive: want=%d, received=%d\n",
                   samples, received);
    }

    return !(received == samples);
}

int iis_receive_int(u32 control, u32 fdx_control, u32 wid, u32 srate,
                    u32 wmk, short *buff, u32 samples)
{
   u32 received;

   if (!buff || !samples) {
       aic_printf("iis_receive: error param!");
       return AIC_EIO;
   }

   iis_reset();
   iis_reset_fdx();

   if (fdx_control)
       iis_set_control_fdx(fdx_control);
   iis_set_control(control);

   iis_set_bitwidth(wid);
   iis_set_srate(srate);
   iis_set_aempty(wmk);
   iis_set_aempty_fdr(wmk);

   es8374_set_control(control);
   es8374_set_srate(srate);
   es8374_open();

   iis_enalbe(TRUE);

   received = iis_recv_int(buff, samples);

   iis_enalbe(FALSE);
   es8374_close();

   if (received != samples) {
       aic_printf("iis_receive: control=%d, fdx_control=%d, wid=%d, srate=%d\n",
                  control, fdx_control, wid, srate);
       aic_printf("iis_receive: want=%d, received=%d\n",
                  samples, received);
   }

   return !(received == samples);
}

static int iis_full_duplex(u32 control, u32 fdx_control, u32 wid, u32 srate,
                           u32 samples)
{
   u32 samples_ret;

   iis_reset();
   iis_reset_fdx();
   iis_set_control_fdx(fdx_control);
   iis_set_control(control);
   iis_set_bitwidth(wid);
   iis_set_srate(srate);

   es8374_set_control(control);
   es8374_set_srate(srate);
   es8374_open();

   iis_enalbe(TRUE);
   samples_ret = iis_recv_send(samples);
   iis_enalbe(FALSE);
   es8374_close();

   if (samples_ret != samples) {
       aic_printf("full_duplex: control=%d, fdx_control=%d, wid=%d, srate=%d\n",
                  control, fdx_control, wid, srate);
       aic_printf("iis_receive: want=%d, real=%d\n",
                  samples, samples_ret);
   }

   return !(samples_ret == samples);
}

static int iis_master_transmit(void)
{
    union iis_control ctl = iis_master_transmit_control;

    IIS_CTL_STANDARD(ctl)
    TEST_FUNC(iis_transmit_int(ctl.value, 0, DEFAULT_WID,
                               DEFAULT_RATE, IIS_INT_WMK, DEFAULT_RING,
                               DEFAULT_RING_SAMPLES));
    IIS_CTL_L_JUSTIFY(ctl)
    TEST_FUNC(iis_transmit(ctl.value, 0, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));

    IIS_CTL_DSP_MODE(ctl)
    TEST_FUNC(iis_transmit(ctl.value, 0, DEFAULT_WID,
                           IIS_SRATE_16K, RING_16K, RING_16K_SAMPLES));


    IIS_CTL_STANDARD(ctl)
    TEST_FUNC(iis_transmit(ctl.value, 0, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));



    IIS_CTL_R_JUSTIFY(ctl)
    TEST_FUNC(iis_transmit(ctl.value, 0, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));

    TEST_RETURN
}

static int iis_slave_transmit(void)
{
    TEST_FUNC(iis_transmit(iis_slave_transmit_control.value, 0, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));

    iis_transmit_int(iis_slave_transmit_control.value, 0, DEFAULT_WID,
                     DEFAULT_RATE, IIS_INT_WMK, DEFAULT_RING,
                     DEFAULT_RING_SAMPLES);

    TEST_RETURN
}

static int iis_master_receive(void)
{
    TEST_FUNC(iis_receive(iis_master_receive_control.value, 0, DEFAULT_WID,
                          DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES));

    iis_transmit(iis_master_transmit_control.value,
                 iis_full_duplex_tx_control.value, DEFAULT_WID,
                 DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES);

    TEST_RETURN
}

static int iis_slave_receive(void)
{
    TEST_FUNC(iis_receive(iis_slave_receive_control.value, 0, DEFAULT_WID,
                          DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES));
    iis_transmit(iis_master_transmit_control.value,
                 iis_full_duplex_tx_control.value, DEFAULT_WID,
                 DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES);

    TEST_RETURN
}

static int iis_master_full_duplex_tx(void)
{
    TEST_FUNC(iis_transmit(iis_master_transmit_control.value,
                           iis_full_duplex_tx_control.value, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));
    TEST_RETURN
}

static int iis_slave_full_duplex_tx(void)
{
    TEST_FUNC(iis_transmit(iis_slave_transmit_control.value,
                           iis_full_duplex_tx_control.value, DEFAULT_WID,
                           DEFAULT_RATE, DEFAULT_RING, DEFAULT_RING_SAMPLES));
    TEST_RETURN
}

static int iis_master_full_duplex_rx(void)
{
    TEST_FUNC(iis_receive(iis_master_receive_control.value,
                          iis_full_duplex_rx_control.value, DEFAULT_WID,
                          DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES));

    iis_transmit(iis_master_transmit_control.value,
                 iis_full_duplex_tx_control.value, DEFAULT_WID,
                 DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES);

    TEST_RETURN
}

static int iis_slave_full_duplex_rx(void)
{
    TEST_FUNC(iis_receive(iis_slave_receive_control.value,
                          iis_full_duplex_rx_control.value, DEFAULT_WID,
                          DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES));
    iis_transmit(iis_master_transmit_control.value,
                 iis_full_duplex_tx_control.value, DEFAULT_WID,
                 DEFAULT_RATE, DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES);

    TEST_RETURN
}

static int iis_master_full_duplex(void)
{
    TEST_FUNC(iis_full_duplex(iis_master_transmit_control.value,
                              iis_full_duplex_control.value, DEFAULT_WID,
                              DEFAULT_RATE, DUPLEX_COUNT));
    TEST_RETURN
}

static int iis_slave_full_duplex(void)
{
    TEST_FUNC(iis_full_duplex(iis_slave_transmit_control.value,
                              iis_full_duplex_control.value, DEFAULT_WID,
                              DEFAULT_RATE, DUPLEX_COUNT));
    TEST_RETURN
}

static int iis_sidetone(u32 control, u32 fdx_control)
{
    iis_reset();
    iis_reset_fdx();

    if (fdx_control)
        iis_set_control_fdx(fdx_control);
    iis_set_control(control);
    iis_set_bitwidth(DEFAULT_WID);
    iis_set_srate(DEFAULT_RATE);

    es8374_set_control(control);
    es8374_set_srate(DEFAULT_RATE);
    es8374_open();

    iis_enable_side_hpf(FALSE);
    iis_enable_sidetone(TRUE);

    aic_printf("side tone start, default config!\n");
    iis_enalbe(TRUE);
    aic_mdelay(3000);

    aic_printf("%s start, change dvc!\n");
    iis_set_side_dvc(0x24, 0x24);
    aic_mdelay(3000);

    aic_printf("%s start, enable hpf!\n");
    iis_enable_side_hpf(TRUE);
    aic_mdelay(3000);

    aic_printf("%s start, change hpf!\n");
    iis_set_side_hpf(10, 10);
    aic_mdelay(3000);

    iis_enalbe(FALSE);
    es8374_close();

    return 0;
}

static int iis_master_sidetone(void)
{
    union iis_control control = iis_master_transmit_control;

    TEST_FUNC(iis_sidetone(control.value, 0));

    TEST_RETURN
}

static int iis_slave_sidetone(void)
{
    union iis_control control = iis_slave_transmit_control;

    TEST_FUNC(iis_sidetone(control.value, 0));

    TEST_RETURN
}

static int iis_loopback(u32 control, u32 fdx_control)
{
    iis_reset();
    iis_reset_fdx();

    if (fdx_control)
        iis_set_control_fdx(fdx_control);
    iis_set_control(control);
    iis_set_bitwidth(DEFAULT_WID);
    iis_set_srate(DEFAULT_RATE);
    iis_enable_sdo_mplxer(TRUE, 1, 1);
    iis_enable_loop(TRUE);

    iis_send_recv(DEFAULT_RING, DEFAULT_RING_SAMPLES,
                     DEFAULT_BUFF, DEFAULT_BUFF_SAMPLES);

    return memcmp(DEFAULT_RING, DEFAULT_BUFF,
                  MIN(DEFAULT_RING_SAMPLES, DEFAULT_BUFF_SAMPLES));
}

static int iis_master_loop(void)
{
    TEST_FUNC(iis_loopback(iis_master_transmit_control.value,
                           iis_full_duplex_control.value));

    TEST_RETURN
}

aic_case_register(iis_case);

