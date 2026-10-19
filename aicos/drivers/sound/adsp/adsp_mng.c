/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    ADSP manager, including power on/off control, log and dump etc.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1a2cd4
#define LOG_MODULE    AMOD_SOUND

#include <stdio.h>
#include <stdlib.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_sys.h>
#include <subsys_api.h>
#include <adsp.h>
#include <aic_io.h>
#include <aic_size.h>
#include <aic_init.h>
#include <aic_cmd_api.h>
#include <alog_api.h>
#include <reg_base.h>
#include <top_glb.h>
#include <aic_auto_reg.h>
#include <aic_clk_map.h>
#include <aic_iot_clk.h>

#include "snd_compat.h"
#include "adsp_mng.h"
#include "timer_inc.h"

#if 0
#define CONFIG_ADSP_LOG
#define CONFIG_ADSP_DUMP
#endif

#ifdef CONFIG_ADSP_LOG
#define ADSP_LOG_TH_NAME    "adsp log"
#define ADSP_LOG_TH_STACK   1024
#define ADSP_LOG_TH_Q_NUM   5
#define ADSP_LOG_TH_Q_NAME  "adsp log q"
#define ADSP_LOG_TH_PRI     30

#define ADSP_LOG_CHN        SND_IPC_CH_LOG

#define ADSP_LOG_PERIOD_SIZE    512
#define ADSP_LOG_PERIOD_CNT     2

struct adsp_log_msg {
    u32 addr;
    u32 size;
};

#endif

#ifdef CONFIG_ADSP_DUMP
#define ADSP_DUMP_TH_NAME   "adsp dump"
#define ADSP_DUMP_TH_STACK  1024
#define ADSP_DUMP_TH_Q_NUM  5
#define ADSP_DUMP_TH_Q_NAME "adsp dump q"
#define ADSP_DUMP_TH_PRI    30

#define ADSP_DUMP_CHN       SND_IPC_CH_DUMP
#define ADSP_DUMP_PERIOD_SIZE   650
#define ADSP_DUMP_PERIOD_CNT    3
#endif

#define ADSP_LOCK           mutex_lock(g_adsp_ctx.lock)
#define ADSP_UNLOCK         mutex_unlock(g_adsp_ctx.lock)

#define ADSP_POWER_REF_CNT  (g_adsp_ctx.power_ref_cnt)
#define ADSP_POWER_REF_INC  (g_adsp_ctx.power_ref_cnt++)
#define ADSP_POWER_REF_DEC  (g_adsp_ctx.power_ref_cnt--)

struct adsp_dump_msg {
    u32 addr;
    u32 size;
};

struct adsp_ctx {
    aos_thread_id log_th_id;
    aos_thread_id dump_th_id;

    u32 power_ref_cnt;
    u32 dump_pos;
    u32 master_vol;

    u32 session_id;
    u8 is_log_on;

    MUTEX_PTR lock;
};

static struct adsp_ctx g_adsp_ctx;

#ifdef CONFIG_ADSP_DUMP
static void adsp_dump_init(void);
#endif
#ifdef CONFIG_ADSP_LOG
static void adsp_log_init(void);
#endif
static void adsp_init_master_vol(void);

static int adsp_notify_power_off(void)
{
    struct adsp_cmd_common *pcmd =
        (struct adsp_cmd_common *)snd_ipc_get_cmd_buf();
    int ret = 0;

    pcmd->cmd = ADSP_CMD_POWER_OFF;

    ret = snd_ipc_send_cmd(pcmd);
    if (ret)
        SND_ERROR("adsp dump set failed!\n");

    return ret;
}

//#ifndef CONFIG_SUBSYS_CTRL_SUPPORT
#if 1
#define WAIT_CNT_MAX        100
#define ADSP_SHUTDOWN_COMP  0x1f

int adsp_wait_power_off(void)
{
    u32 val, i = 0;

    do {
        aos_sleep(2);
        val = readl((void *)REG_ADSP_PD_STATE);
    } while ((val != ADSP_SHUTDOWN_COMP) && i++ < WAIT_CNT_MAX);

    aicx_info(0x033f9e, "sd", "ADSP Power Off %s, i=%d\n",
              (val == ADSP_SHUTDOWN_COMP) ? "OK" : "FAILED!!!", i);

    if (val == ADSP_SHUTDOWN_COMP)
        return 0;

    i = 0;
    writel((void *)REG_ADSP_POWER_CTRL, BIT_ADSP_FRC_SHUTDOWN);
    do {
        aos_sleep(2);
        val = readl((void *)REG_ADSP_PD_STATE);
    } while (val != ADSP_SHUTDOWN_COMP && i++ < WAIT_CNT_MAX);
    aicx_info(0x06e9da, "sd", "ADSP Power Off force %s, i=%d\n",
              (val == ADSP_SHUTDOWN_COMP) ? "OK" : "FAILED!!!", i);

    return 0;
}
#endif

int adsp_power_on(BOOL is_high)
{
    ADSP_LOCK;

    if (ADSP_POWER_REF_CNT == 0) {
        clk_set_parent_comp(CLK_BLK_DSP_CORE, 0);
        clk_set_rate_comp(CLK_BLK_DSP_CORE, 153600000 * 2);
        clk_enable(CLK_BLK_DSP_CORE_EN);
#ifdef CONFIG_SUPPORT_ADSP
        subsys_power_on(AIC_SYS_ADSP);
#else
        AICX_ASSERT_IFX(0, 0x0bb682, "", "adsp not boot!\n");
#endif
        /* TODO: tmp value */
        aos_sleep(20);

#ifdef CONFIG_ADSP_LOG
        adsp_log_init();
#endif

#ifdef CONFIG_ADSP_DUMP
        adsp_dump_init();
#endif
        adsp_init_master_vol();
    }
    ADSP_POWER_REF_INC;

    ADSP_UNLOCK;

    return 0;
}

int adsp_power_off(void)
{
    ADSP_LOCK;

    if (ADSP_POWER_REF_CNT > 0) {
        ADSP_POWER_REF_DEC;
        if (!ADSP_POWER_REF_CNT) {
            adsp_notify_power_off();
            adsp_wait_power_off();
            clk_disable(CLK_BLK_DSP_CORE_EN);
        }
    }

    ADSP_UNLOCK;

    return 0;
}

int adsp_boot_test(void)
{
    int i;

    writel((void *)0x41090088, 0);
    for (i = 0; i < 100; i++) {
        adsp_power_on(0);
        aos_sleep(200);
        adsp_power_off();
        aicx_error(0x148dc6, "dd", "loop count=%d, adsp boot count = %d\n",
                    i, readl((void *)0x41090088));
    }

    return 0;
}

#ifdef CONFIG_ADSP_LOG

/* TODO: should measure if need send msg to task,
         or send to log system directly */
static int adsp_log_callback(void *data, u8 *addr, u32 size)
{
    alog_dsp_trace(addr, size);
    snd_ipc_prepare_recv_buff(ADSP_LOG_CHN, 0);

    return 0;
}

#if 0
static void adsp_log_thread_entry(void *data)
{
    struct adsp_ctx *ctx = (struct adsp_ctx *)data;
    struct adsp_log_msg *msg;

    snd_ipc_chn_open(ADSP_LOG_CHN, ADSP_LOG_PERIOD_SIZE,
                     ADSP_LOG_PERIOD_CNT,
                     adsp_log_callback, NULL);
    while(1) {
        msg = aos_recv_msg(ctx->log_th_id, AOS_WAIT_FOREVER);
        if (!msg)
            continue;

        /* TODO: send log to log system now */

        snd_free(msg);

        /* TODO: check if need prepare */
        snd_ipc_prepare_recv_buff(ADSP_LOG_CHN, 0);
    }
}

static int adsp_log_init(void)
{
    g_adsp_ctx.log_th_id =  aos_create_thread(ADSP_LOG_TH_NAME,
                                              adsp_log_thread_entry,
                                              &g_adsp_ctx,
                                              ADSP_LOG_TH_STACK,
                                              ADSP_LOG_TH_Q_NUM,
                                              ADSP_LOG_TH_Q_NAME,
                                              ADSP_LOG_TH_PRI, TRUE);
    return 0;
}
#else
void adsp_log_start(void *data)
{
    int i;

    snd_ipc_chn_open(ADSP_LOG_CHN, ADSP_LOG_PERIOD_SIZE,
                     ADSP_LOG_PERIOD_CNT,
                     adsp_log_callback, NULL);
    /* prepair log buffer */
    for (i = 0; i < ADSP_LOG_PERIOD_CNT; i++)
        snd_ipc_prepare_recv_buff(ADSP_LOG_CHN, 0);
}

void adsp_log_stop(void *data)
{
    snd_ipc_chn_close(ADSP_LOG_CHN);
}

#endif

static void adsp_log_set(u8 is_on)
{
    struct adsp_cmd_log *pcmd;

    pcmd = snd_ipc_get_cmd_buf();
    pcmd->is_on = is_on;
    pcmd->ipc_chn = SND_IPC_CH_LOG;
    pcmd->cmd = ADSP_CMD_LOG;
    snd_ipc_send_cmd(pcmd);

    if (is_on)
        adsp_log_start(&g_adsp_ctx);
    else
        adsp_log_stop(&g_adsp_ctx);
}

static int adsp_log(int argc, char **argv)
{
    u32 is_on;

    if (argc > 0)
        is_on = atoi(argv[0]);
    else
        return 0;

    ADSP_LOCK;
    if (ADSP_POWER_REF_CNT > 0)
        adsp_log_set(is_on);
    ADSP_UNLOCK;

    g_adsp_ctx.is_log_on = is_on;

    return 0;
}
CONSOLE_EXPORT_CMD(adsp_log, adsp_log, adsp_log control);

static void adsp_log_init(void)
{
    adsp_log_set(g_adsp_ctx.is_log_on);
}

#endif

#ifdef CONFIG_ADSP_DUMP
#define ADSP_DUMP_POS_OFFSET    100

static int adsp_dump_callback(void *data, u8 *addr, u32 size)
{
    u16 pos = *(u16 *)addr;

    /* check if pos is on */
    if (!(g_adsp_ctx.dump_pos & (1 << pos))) {
        SND_ERROR("pos %d not set\n", pos);
        goto prepair_buff;
    }
    if (size) {
        SND_DBG("dump pos:%d, 0x%x, 0x%x, 0x%x, 0x%x\n",
                *(addr + 2), *(addr + 3), *(addr + 4), *(addr + 5));
        audio_data(addr + 2, size, pos + ADSP_DUMP_POS_OFFSET);
    }

prepair_buff:
    snd_ipc_prepare_recv_buff(ADSP_DUMP_CHN, 0);

    return 0;
}

static void adsp_dump_start(void)
{
    int i;

    snd_ipc_chn_open(ADSP_DUMP_CHN, ADSP_DUMP_PERIOD_SIZE,
                     ADSP_DUMP_PERIOD_CNT,
                     adsp_dump_callback, NULL);
    /* prepair log buffer */
    for (i = 0; i < ADSP_DUMP_PERIOD_CNT; i++)
        snd_ipc_prepare_recv_buff(ADSP_DUMP_CHN, 0);
}

static void adsp_dump_stop(void)
{
    snd_ipc_chn_close(ADSP_DUMP_CHN);
}

int adsp_dump_set(u32 en, u32 pos)
{
    struct adsp_cmd_dump *pcmd;
    int ret = 0;

    ADSP_LOCK;
    if (ADSP_POWER_REF_CNT > 0) {
        pcmd = (struct adsp_cmd_dump *)snd_ipc_get_cmd_buf();
        pcmd->cmd = ADSP_CMD_DUMP;
        pcmd->en = en;
        pcmd->pos = pos;

        ret = snd_ipc_send_cmd(pcmd);
        if (ret) {
            ADSP_UNLOCK;
            CONS_OUT("adsp dump set failed!\n");
            return 0;
        }
    }
    ADSP_UNLOCK;

    if (en) {
        /* check and only start once */
        if (!g_adsp_ctx.dump_pos && pos)
            adsp_dump_start();
        g_adsp_ctx.dump_pos |= pos;
    } else {
        g_adsp_ctx.dump_pos &= ~pos;
        if (!g_adsp_ctx.dump_pos)
            adsp_dump_stop();
    }
    if (g_adsp_ctx.dump_pos)
        CONS_OUT("adsp dump current pos: 0x%x\n", g_adsp_ctx.dump_pos);
    else
        CONS_OUT("adsp dump all points are closed!n");

    return ret;
}

static void adsp_dump_init(void)
{
    struct adsp_cmd_dump *pcmd;

    if (!g_adsp_ctx.dump_pos)
        return;

    pcmd = (struct adsp_cmd_dump *)snd_ipc_get_cmd_buf();
    pcmd->cmd = ADSP_CMD_DUMP;
    pcmd->en = TRUE;
    pcmd->pos = g_adsp_ctx.dump_pos;

    if (snd_ipc_send_cmd(pcmd)) {
        SND_ERROR("adsp dump set failed!\n");
        return;
    }

    adsp_dump_start();

    return;
}

#endif

static int adsp_dbg(int argc, char **argv)
{
    u32 *p;
    u32 data;

    if (argc > 0)
        data = atoi(argv[0]);
    else
        return 0;

    p = snd_ipc_get_cmd_buf();
    *p = ADSP_CMD_DEBUG;
    *(p + 1) = data;
    snd_ipc_send_cmd((void *)p);

    return 0;
}
CONSOLE_EXPORT_CMD(adsp_dbg, adsp_dbg, adsp_dbg command);

static int adsp_urc(void *rsp_ptr, u32 data)
{
#if (CONFIG_ALOG_STATIC_LEVEL >= ALOG_LV_ERROR)
    u32 cmd = (u32)rsp_ptr;
#endif

    SND_ERROR("DSP urc, cmd=%d, type=%d, p2=%d\n",
              cmd & 0xffff, (cmd >> 16), cmd);

    subsys_dump(AIC_SYS_ADSP);

    return 0;
}

int adsp_set_master_vol(u32 vol)
{
    u32 *p;
    int ret;

    if (vol > ADSP_MVOL_LEVEL_MAX) {
        SND_ERROR("adsp vol out of range, vol=%d!\n", vol);
        return AIC_EPERM;
    }

    if (vol == g_adsp_ctx.master_vol)
        return 0;

    /* if not power on, only save it, do not send to dsp */
    if (ADSP_POWER_REF_CNT == 0) {
        g_adsp_ctx.master_vol = vol;
        return 0;
    }
    p = snd_ipc_get_cmd_buf();
    /* fill data same as struct adsp_cmd_master_vol */
    *p = ADSP_CMD_MASTER_VOL;
    *(p + 1) = vol;

    ret =snd_ipc_send_cmd((void *)p);
    if (!ret)
        g_adsp_ctx.master_vol = vol;

    return ret;
}

int adsp_set_mute_ul(u32 is_mute)
{
    u32 *p;

    /* if not power on, do not send to dsp */
    if (ADSP_POWER_REF_CNT == 0)
        return 0;

    p = snd_ipc_get_cmd_buf();
    /* fill data same as struct adsp_cmd_master_vol */
    *p = ADSP_CMD_MUTE_UL;
    *(p + 1) = is_mute;

    return snd_ipc_send_cmd((void *)p);
}

static void adsp_init_master_vol(void)
{
    u32 *p;

    p = snd_ipc_get_cmd_buf();
    /* fill data same as struct adsp_cmd_master_vol */
    *p = ADSP_CMD_MASTER_VOL;
    *(p + 1) = g_adsp_ctx.master_vol;

    snd_ipc_send_cmd((void *)p);
}

int adsp_send_command(void *cmd_ptr)
{
    if (ADSP_POWER_REF_CNT == 0) {
        snd_ipc_put_cmd_buf();
        SND_INFO("adsp cmd ingored, not boot!\n");
        return 0;
    }
    return snd_ipc_send_cmd(cmd_ptr);
}

static int adsp_init(void)
{
    mutex_init(g_adsp_ctx.lock);
    g_adsp_ctx.is_log_on = TRUE;

    /* this session id must be 0 */
    g_adsp_ctx.session_id = snd_ipc_async_reg(adsp_urc, 0);
    g_adsp_ctx.master_vol = 10; /* default */

    return 0;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(adsp_init);

