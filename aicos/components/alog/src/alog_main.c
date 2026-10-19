/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log module main source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02cace
#define LOG_MODULE    AMOD_ALOG

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <adbg_cfg_api.h>
#include <alog_api.h>
#include <alog_init.h>
#include <alog_inner.h>
#include <alog_trace_filter.h>
#include <alog_data.h>
#include <alog_trace.h>
#include <alog_outputer.h>
#include <alog_store_api.h>

#ifdef CONFIG_ALOG_MSG_SUPPORT
#include <alog_message.h>
#include <alog_msg_filter.h>
#endif

typedef struct ALOG {
    const ALOG_CFG_T   *cfg;
    ABRR_HANDLE         brr;
    ALOG_TRACE_T        *alog_trace;
#ifdef CONFIG_ALOG_MSG_SUPPORT
    ALOG_MSG_T          *alog_msg;
#endif
    ALOG_DATA_T         *alog_data;
    ALOP_T              *log_outputer;
} ALOG_T;

ALOG_T s_alog;

static void alog_creaete_alop(u8 new_dev)
{
    u8 com = adbg_get_log_dev_com();
    const ALBRR_CFG_T *brr_cfg;
    u8 cnt;

    brr_cfg = alog_get_inputter_brr_cfg(&cnt);
    s_alog.log_outputer = alop_create(new_dev, com);
    if (!s_alog.log_outputer)
        return;

    alop_create_inputer(s_alog.log_outputer, cnt, brr_cfg);
}

void alog_early_init(void)
{
    /* trace and msg filter init */
    alog_trace_filter_init();

#ifdef CONFIG_ALOG_MSG_SUPPORT
    alog_msg_filter_init();
#endif
}

int alog_init(void)
{
    /* get cfg .*/
    s_alog.cfg = alog_get_cfg();

    /* request debug ablk brr. */
    s_alog.brr = ablk_request_brr(s_alog.cfg->sys, s_alog.cfg->brr);
    AIC_ASSERT(s_alog.brr);

    aicx_debug(0x1c192e, "dd", "init, sys=%d, brr=%d\n", s_alog.cfg->sys, s_alog.cfg->brr);

    /* create alog_trace. */
    s_alog.alog_trace = alog_trace_create(s_alog.brr);

#ifdef CONFIG_ALOG_MSG_SUPPORT
    /* create alog_trace. */
    s_alog.alog_msg = alog_msg_create(s_alog.brr);
#endif

    /* create alog_data. */
    s_alog.alog_data = alog_data_create(s_alog.cfg->sys);

#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    alog_uart_log_handle_init();
#endif

    return AIC_SUCCESS;
}

/* After the base module(like nv) is initialized. */
int alog_later_init(void)
{
    u8 dev_type, com;

#ifdef CONFIG_SUBSYS_CP
    alog_trace_filter_init_fast();
#endif

    /* create log outpter. */
    dev_type = adbg_get_log_dev();
    com = adbg_get_log_dev_com();

#ifdef CONFIG_ALOG_STORE_FS
    if (aic_boot_mode_is_normal())
        alog_store_pre_init();
#endif

    aicx_debug(0x05e3db, "dd", "init, outputdev type=%d, com=%d\n", dev_type, com);
    if (dev_type != AIC_IODEV_NONE) {
#if IS_CONF_ENABLED(CONFIG_ABLK_DYNAMIC_SET_LOOPBACKL)
        /* log brr must set to loopback for local system. */
        ablk_set_loopback(s_alog.brr, TRUE);
#ifdef CONFIG_ALOG_MSG_SUPPORT
        ablk_set_loopback(s_alog.alog_msg->brr, TRUE);
#endif
#endif

#ifdef CONFIG_ALOG_STORE_FS
        /* if dev_type is file, init alog store. */
        if (dev_type == AIC_IODEV_FILE &&
            aic_boot_mode_is_normal()) {
            alog_store_init();
        }
#endif
        alog_creaete_alop(dev_type);
    }

    return AIC_SUCCESS;
}
AIC_COMP_P1_INIT_EXPORT(alog_later_init);

int alog_deinit(void)
{
    /* destroy log outpter. */
    if (s_alog.log_outputer) {
        alop_destroy(s_alog.log_outputer);
        s_alog.log_outputer = NULL;
    }

    /* destroy alog_trace, alog_msg, alog_data. */
    alog_trace_destroy(s_alog.alog_trace);
#ifdef CONFIG_ALOG_MSG_SUPPORT
    alog_msg_destroy(s_alog.alog_msg);
#endif
    alog_data_destroy(s_alog.alog_data);

    /* relase brr. */
    ablk_release_brr(s_alog.brr);

    return AIC_SUCCESS;
}

static void alog_destory_alop(void)
{
    ALOP_T *alop = s_alog.log_outputer;

    s_alog.log_outputer = NULL;
    alop_destroy(alop);
}

void alog_set_output_dev(u8 new_dev)
{
    u8 old_dev;

    aicx_debug(0x148f79, "d", "set out dev=%d\n", new_dev);

    if (!s_alog.log_outputer)
        old_dev = AIC_IODEV_NONE;
    else
        old_dev = s_alog.log_outputer->dev_type;

    /* no change. */
    if (old_dev == new_dev)
        return;

    /* firstly destroy.old alop. */
    if (s_alog.log_outputer)
        alog_destory_alop();

#if IS_CONF_ENABLED(CONFIG_ABLK_DYNAMIC_SET_LOOPBACKL)
#ifdef CONFIG_SUBSYS_CP
    /* cp config the log brr loopback. */
    ablk_set_loopback(s_alog.brr, (new_dev != AIC_IODEV_NONE));
#ifdef CONFIG_ALOG_MSG_SUPPORT
    ablk_set_loopback(s_alog.alog_msg->brr, (new_dev != AIC_IODEV_NONE));
#endif
#endif
#endif

    /* than create the alop.. */
    if (new_dev != AIC_IODEV_NONE)
        alog_creaete_alop(new_dev);
}

void alog_flush_all_log_in_assert(void)
{
#ifdef CONFIG_ALOG_MSG_SUPPORT
    if (s_alog.alog_msg && s_alog.alog_msg->buf_mgr) {
        /* flush msg. */
        almsg_flush_msg_queue_in_assert();
        albf_mgr_set_output(s_alog.alog_msg->buf_mgr, s_alog.log_outputer);
        albf_mgr_flush_all_buf_in_assert(s_alog.alog_msg->buf_mgr);
    }
#endif

    /* flush trace. */
    if (s_alog.alog_trace && s_alog.alog_trace->buf_mgr) {
        albf_mgr_set_output(s_alog.alog_trace->buf_mgr, s_alog.log_outputer);
        albf_mgr_flush_all_buf_in_assert(s_alog.alog_trace->buf_mgr);
    } else {
#if IS_CONF_ENABLED(ALOG_EARLY_BOOY_LOG)
        alog_trace_flush_boot_log();
#endif
    }
}

void alog_output_data_in_assert(void *addr, size_t size)
{
    if (s_alog.alog_trace && s_alog.alog_trace->buf_mgr) {
        ABLK_T blk;

        blk.addr = addr;
        blk.size = size;
        albf_mgr_output_block_in_assert(s_alog.alog_trace->buf_mgr, ALDATA_LINK, &blk);
    }
}

