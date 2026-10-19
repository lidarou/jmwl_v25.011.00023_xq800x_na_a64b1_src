/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the  api of aic console.
 */

#ifndef __AIC_CONSOLE_H__
#define __AIC_CONSOLE_H__

#ifdef CONFIG_PLAT_CONSOLE
#include <aic_cmd_api.h>
#include <aic_osa.h>

#define CONS_PRIORITY        31
#define CONS_STACK_SIZE      2048
#define CONS_HISTORY_LINES   8
#define CONS_PROMPT          "$"
#define CONS_NAME            "console"

#ifdef CONFIG_PLAT_CMD_HISTORY
#define CONS_PRE_HISTORY(index, cnt) \
    ((index) > 0) ? ((index) - 1) : cnt - 1

#define CONS_NEXT_HISTORY(index, cnt) \
    ((index) == cnt - 1) ? 0 : ((index) + 1)
#endif

/*
 * Arrow keys keyboard scan code:
 * up key:    0x1b, 0x5b, 0x41
 * down key:  0x1b, 0x5b, 0x42
 * right key: 0x1b, 0x5b, 0x43
 * left key:  0x1b, 0x5b, 0x44
 */
#define AIC_KEY_UP      0x41
#define AIC_KEY_DOWN    0x42
#define AIC_KEY_RIGHT   0x43
#define AIC_KEY_LEFT    0x44
#define AIC_KEY_SPEC1   0x1b
#define AIC_KEY_SPEC2   0x5b

#define IS_ARROW_KEY(c) ((0x40 < c) && (c < 0x45))
/* RESET -->SPEC1 -->SPEC2 -->RESET */
enum aic_console_stat
{
    CONS_RESET_STATE = 0,
    CONS_SPEC1_STATE,
    CONS_SPEC2_STATE
};

struct aic_console_cmd
{
    enum aic_console_stat  state;

#ifdef CONFIG_PLAT_CMD_HISTORY
    uint16_t    history_cnt;
    uint16_t    history_cur; /* display cur history cmd index. */
    uint16_t    history_next;/* save the next history cmd index. */
    char        history_cmd[CONS_HISTORY_LINES][CONS_CMD_SIZE];
#endif

    char        cmd_buf[CONS_CMD_SIZE];
    uint8_t     cmd_len;/* cmd lenth */
    uint8_t     cmd_cur;/* cmd cursor */
    void *      cons_dev;/* console device, maybe uart. */
};

#endif /* CONFIG_PLAT_CONSOLE */

#endif /* __AIC_CONSOLE_H__ */
