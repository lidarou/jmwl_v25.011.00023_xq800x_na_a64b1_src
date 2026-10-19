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
#define AIC_THIS_FILE    0x186ce5
#define LOG_MODULE    AMOD_ALOG

#include <aic_assert.h>
#include <aic_cmd_api.h>
#include <adbg_cfg_api.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <alog_api.h>
#include <alog_inner.h>
#include <alog_trace_filter.h>
#include <alog_outputer.h>
#include <stdlib.h>

/* After the base module(like nv) is initialized. */

int alog_trace_output(int argc, char **argv)
{
    u8 dev, com;
    int ret;

    /* trace, output help info.*/
    if (argc == 0) {
        CONS_OUT("------------------------------------\n");
        CONS_OUT("trace [get] \n"
                 "trace [set] [item] [value]:\n"
                 "dev:\n" "0--none\n" "1--uart\n" "2--usb\n" "3--file\n"
                 "com:\n" "0--com0\n" "x--comx\n");
        CONS_OUT("------------------------------------\n");
        return AIC_SUCCESS;
    }

    /* trace get process. */
    if (argc == 1) {
        if (0 == strcmp(argv[0], "get")) {
            dev = adbg_get_log_dev();
            com = adbg_get_log_dev_com();
            CONS_OUT("------------------------------------\n");
            CONS_OUT("trace output dev: %d\n", dev);
            CONS_OUT("trace output com: %d\n", com);
            CONS_OUT("------------------------------------\n");
            return AIC_SUCCESS;
        }
    }

    /* trace set process. */
    if (argc == 3) {
        if (0 == strcmp(argv[0], "set")) {
            if (0 == strcmp(argv[1], "dev")) {
                ret = atoi(argv[2]);
                if (ret < 0 || ret >= AIC_IODEV_CNT)
                    return -AIC_EINVAL;
                adbg_set_log_dev((u8)ret);
                CONS_OUT("set dev=%d.\n", ret);
                return AIC_SUCCESS;
            }

            if (0 == strcmp(argv[1], "com")) {
                ret = atoi(argv[2]);
                if (ret < 0 || ret >= AIC_COM_CNT)
                    return -AIC_EINVAL;
                adbg_set_log_dev((u8)ret);
                CONS_OUT("set dev=%d.\n", ret);
                return AIC_SUCCESS;
            }
        }
    }

    return -AIC_EINVAL;
}
#ifdef CONFIG_PLAT_CONSOLE
CONSOLE_EXPORT_CMD(trace, alog_trace_output, trace cmd);
#endif