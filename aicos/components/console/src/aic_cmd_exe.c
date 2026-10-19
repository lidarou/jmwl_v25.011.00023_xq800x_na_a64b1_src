/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is used to execution console cmd.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x043ecb
#define LOG_MODULE    AMOD_CONS

#ifdef CONFIG_PLAT_CONSOLE

#include <aic_cmd.h>
#include <aic_console.h>
#include <aic_errno.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

/*
 * aic_cmd_split -Split the whole cmdline and save all the paramters.
 *
 * @argv: will save all the parameters start addres.
 *
 * @return: return parameters counts or err.
 */
static int aic_cmd_split(char *cmd, size_t len, char *argv[CONSOLE_MAX_ARGS])
{
    char *cur = cmd;
    size_t pos = 0, argc = 0;

    while (pos < len) {
        /* All param must start with bank and tab. */
        if ((*cur != ' ' && *cur != '\t'))
            return argc;

        /* skip white space */
        while ((*cur == ' ' || *cur == '\t') && pos < len) {
            /* Ends the previous parameter or cmd. */
            *cur = 0;
            cur++;
            pos++;
        }

        if(argc == CONSOLE_MAX_ARGS || pos == len)
            return argc;

        /* " xxx  xxx"  is one param */
        if (*cur == '"') {
            cur++;
            pos++;
            argv[argc] = cur; argc++;

            /* Find the next '"'. */
            while (*cur != '"' && pos < len) {
                /* skip \" */
                if (*cur == '\\' && *(cur + 1) == '"') {
                    cur++;
                    pos++;
                }

                cur++;
                pos++;
            }

            /* Can't find the next '"'. */
            if (pos >= len)
                return AIC_EINVAL;

            /*repalce " with 0. */
            *cur = '\0';
            cur++;
            pos++;
            continue;
        }

        argv[argc] = cur;
        argc++;
        while ((*cur != ' ' && *cur != '\t') && pos < len) {
            cur++;
            pos++;
        }
    }

    return argc;
}

static int aic_cmd_exec_cmd(char *cmd, size_t len)
{
    int argc = 0;
    size_t cmd_len = 0;
    scall_func call_func;
    char *argv[CONSOLE_MAX_ARGS];

    /* Find the command. */
    while (cmd_len < len && (cmd[cmd_len] != ' ' && cmd[cmd_len] != '\t'))
        cmd_len++;

    if (cmd_len == 0)
        return AIC_ENOEXEC;

    call_func = aic_cons_get_func(cmd, cmd_len);
    if (call_func == NULL)
        return AIC_ENOEXEC;

    /* Parse arguments. */
    memset(argv, 0x00, sizeof(argv));
    argc = aic_cmd_split(cmd + cmd_len, len - cmd_len, argv);
    if (argc < 0)
        return argc;

    /* Exec this command. */
    return call_func(argc, argv);
}

int aic_cmd_proc(char *cmd, size_t len)
{
    int err = 0;

    /* Skip the beginning white space of command. */
    while (*cmd  == ' ' || *cmd == '\t') {
        cmd++;
        len--;
    }

    if (len == 0)
        return 0;

    err = aic_cmd_exec_cmd(cmd, len);
    if (err)
        CONS_OUT("command error: err is %d\n", err);

    return err;
}

#endif /* CONFIG_PLAT_CONSOLE */
