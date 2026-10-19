/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the aic cmd Interface.
 */

#ifndef __AIC_CMD_API_H__
#define __AIC_CMD_API_H__

#include <aic_type.h>
#include <stdarg.h>

#define CONS_CMD_HEAD  "cmd_"
#define CMD_HEAD_LEN   4

#define CMD_START_SEC  __attribute__((section(".aiccmd0")))
#define CMD_SYS_SEC    __attribute__((section(".aiccmd1")))
#define CMD_USER_SEC   __attribute__((section(".aiccmd2")))
#define CMD_END_SEC    __attribute__((section(".aiccmd10")))

typedef int (*scall_func)(int argc, char **argv);

#ifdef CONFIG_CONSOLE_CMD_HAS_INFO
struct aic_cons_scall
{
    const char    *name;
    scall_func    func;
    const char    *info;
};

#define CONSOLE_EXPORT(cmd, func, info) \
    const char cons_##cmd##_name[] = #cmd; \
    const char cons_##cmd##_info[] = #info; \
    const struct aic_cons_scall cons_##cmd CMD_SYS_SEC = \
    { \
        cons_##cmd##_name, \
        (scall_func)&func, \
        cons_##cmd##_info \
    }

/**
 * CONSOLE_EXPORT_CMD - This macro exports a cmd for console.
 * all the cmd function well store in ConsSec section.
 *
 * @func:  the process func of the cmd.
 * @cmd:   the console cmd.
 */
#define CONSOLE_EXPORT_CMD(cmd, func, info) \
    CONSOLE_EXPORT(cmd_##cmd, func, info)
#else
struct aic_cons_scall
{
    const char    *name;
    scall_func    func;
};

#define CONSOLE_EXPORT(cmd, func) \
    const char cons_##cmd##_name[] = #cmd; \
    const struct aic_cons_scall cons_##cmd CMD_SYS_SEC = \
    { \
        cons_##cmd##_name, \
        (scall_func)&func, \
    }

/**
 * CONSOLE_EXPORT_CMD - This macro exports a cmd for console.
 * all the cmd function well store in ConsSec section.
 *
 * @func:  the process func of the cmd.
 * @cmd:   the console cmd.
 */
#define CONSOLE_EXPORT_CMD(cmd, func, info) \
    CONSOLE_EXPORT(cmd_##cmd, func)
#endif

/******************************************************************
 * cmd format:
 * 1. max support 5 args.
 * 2. max cmd len 20
 * 3. The parameters are separated by white spaces(' ' or '\t').
 * 4. If spaces are required in on parameter, enclose them wihh ".
 * likes below:
 * cmd
 * cmd p1 p2 p3
 * cmd p1 p2 p3
 * cmd "p1" p2 "p3"
 * cmd " \"test\" "cmd\" "
*******************************************************************/
#define CONSOLE_MAX_ARGS    30
#define CONS_CMD_SIZE       128

/**
 * CONS_OUT - This macro exports to printf a format string to consone.
 */
void aic_cons_printf(const char *fmt, ...);

#ifdef CONFIG_PLAT_CONSOLE
#define CONS_OUT aic_cons_printf
#else
#define CONS_OUT(fmt, ...)
#endif

#endif /* __AIC_CMD_API_H__ */

