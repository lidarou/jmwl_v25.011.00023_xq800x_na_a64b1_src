/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This aic console source file, realize the aic console function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08cb86
#define LOG_MODULE    AMOD_CONS

#ifdef CONFIG_PLAT_CONSOLE
#include <aic_assert.h>
#include <aic_cmd.h>
#include <aic_console.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_io_dev.h>
#include <stdio.h>
#include <stdarg.h>
#include <diag_cmd_api.h>

/* use this feature to to control diag console. */
#define CONFIG_DIAG_CONSOLE     1

#ifdef CONFIG_CONNECT_MGR
#include <connmgr_api.h>
#else
#define AIC_CONSOLE_COM     AIC_COM0
#define AIC_CONSOLE_DEV     AIC_IODEV_UART
#endif

#define CONSOLE_BUF_SIZE    128

static const struct aic_cons_scall s_scall_begin CMD_START_SEC;
static const struct aic_cons_scall s_scall_end CMD_END_SEC;
static struct aic_console_cmd s_console;

#define CMDMOV(pre, cur, len) do { \
    uint16_t size; \
    char *to, *from; \
 \
    cur = s_console.cmd_cur; \
    len = s_console.cmd_len; \
 \
    if (pre) { \
        to = &s_console.cmd_buf[cur]; \
        from = to + 1; \
    } else { \
        from = &s_console.cmd_buf[cur]; \
        to = from +1; \
    }\
 \
    size = s_console.cmd_len - cur; \
    memmove(to, from, size); \
} while(0)
#define CMDMOV_PRE(cur, len)    CMDMOV(TRUE, cur, len)
#define CMDMOV_NEXT(cur, len)   CMDMOV(FALSE, cur, len)

void aic_cons_list_related_cmds(char *cmd, boolean info)
{
    int minimum_len = 0xffff;
    const char *shortest_name = NULL;
    const char *cur_name = NULL;
    const struct aic_cons_scall *cur;

    CONS_OUT("\n");

    for (cur = (&s_scall_begin + 1); cur < &s_scall_end; cur++) {
        if (strncmp(cur->name, CONS_CMD_HEAD, CMD_HEAD_LEN) != 0)
            continue;

        cur_name = (const char *) &cur->name[CMD_HEAD_LEN];
        if (*cmd == 0 || strncmp(cmd, cur_name, strlen(cmd)) == 0) {
            if (*cmd != 0 && minimum_len > strlen(cur_name)) {
                shortest_name = cur_name;
                minimum_len = strlen(cur_name);
            }

#ifdef CONFIG_CONSOLE_CMD_HAS_INFO
            if (info)
                CONS_OUT("%s\n", cur_name, cur->info);
            else
#endif
                CONS_OUT("%s\n", cur_name);
        }
    }

    /* use shortest cmd as whole cmd. */
    if (shortest_name != NULL)
        strncpy(cmd, shortest_name, minimum_len);

    /*  show the shortest cmd. */
    if (*cmd)
        CONS_OUT("%s%s", CONS_PROMPT, cmd);
}

#ifdef CONFIG_CONSOLE_CMD_FULL_FUNC
#ifdef CONFIG_PLAT_CMD_HISTORY
static void aic_cons_handle_history(struct aic_console_cmd *cons)
{
    /* Clear the old line. */
    CONS_OUT("\r\033[2K\r");
    /* out the new line. */
    CONS_OUT("\r%s%s", CONS_PROMPT, cons->cmd_buf);
}

static void aic_cons_add_history(struct aic_console_cmd *cons)
{
    uint16_t pre, cur;

    if (cons->cmd_len == 0)
        return;

    pre = CONS_PRE_HISTORY(cons->history_next, CONS_HISTORY_LINES);

    /* if current cmd is same as last cmd, don't push */
    if (memcmp(cons->history_cmd[pre], cons->cmd_buf, cons->cmd_len) == 0)
        return;

    /* save cmd in history. */
    cur = cons->history_next;
    memset(&cons->history_cmd[cur][0], 0, CONS_CMD_SIZE);
    memcpy(&cons->history_cmd[cur][0], cons->cmd_buf, cons->cmd_len);
    cons->history_cur = cur;
    if (cons->history_cnt < CONS_HISTORY_LINES)
        cons->history_cnt++;

    /* loop cover the history */
    cons->history_next =
        CONS_NEXT_HISTORY(cons->history_next, CONS_HISTORY_LINES);
}

static boolean aic_cons_proc_arrow_key(char c)
{
    /* RESET -->SPEC1 -->SPEC2 -->RESET */
    if (c == AIC_KEY_SPEC1) {
        s_console.state = CONS_SPEC1_STATE;
        return TRUE;
    }

    if (c == AIC_KEY_SPEC2) {
        if (s_console.state == CONS_SPEC1_STATE ||
            s_console.state == CONS_SPEC2_STATE)
            s_console.state = CONS_SPEC2_STATE;
        else
           s_console.state = CONS_RESET_STATE;

        return TRUE;
    }

    if (s_console.state != CONS_SPEC2_STATE
        /* the SPEC2 0x5b may be intercepted by simulator tool, pass SPEC2. */
        && s_console.state != CONS_SPEC1_STATE)
        return FALSE;

    switch (c) {
    case AIC_KEY_UP:
    case AIC_KEY_DOWN:
        if (c == AIC_KEY_UP)
            s_console.history_cur =
                CONS_PRE_HISTORY(s_console.history_cur, s_console.history_cnt);
        else
            s_console.history_cur =
                CONS_NEXT_HISTORY(s_console.history_cur, s_console.history_cnt);

       memcpy(s_console.cmd_buf,
                   &s_console.history_cmd[s_console.history_cur][0],
                   CONS_CMD_SIZE);
       s_console.cmd_cur = s_console.cmd_len = strlen(s_console.cmd_buf);
       aic_cons_handle_history(&s_console);
       break;

    case AIC_KEY_LEFT:
        /* cusor back */
        if (s_console.cmd_cur) {
            CONS_OUT("\b");
            s_console.cmd_cur--;
        }
        break;

    case AIC_KEY_RIGHT:
        /* cusor forward */
        if (s_console.cmd_cur < s_console.cmd_len) {
            CONS_OUT("%c", s_console.cmd_buf[s_console.cmd_cur]);
            s_console.cmd_cur++;
        }
        break;

    default:
      break;
    }

    s_console.state = CONS_RESET_STATE;
    return TRUE;
}
#endif

static void aic_proc_backspace(void)
{
    uint16_t i, cur, len;

    if (s_console.cmd_cur == 0)
        return;

    s_console.cmd_len--;
    s_console.cmd_cur--;

    /* The cursor is in the middle of the cmd. */
    if (s_console.cmd_len > s_console.cmd_cur) {
        CMDMOV_PRE(cur, len);
        s_console.cmd_buf[len] = 0;

        CONS_OUT("\b%s  \b", &s_console.cmd_buf[cur]);

        /* move the cursor to the origin position */
        for (i = cur; i <= len; i++)
            CONS_OUT("\b");
    } else {
        CONS_OUT("\b \b");
        s_console.cmd_buf[s_console.cmd_len] = 0;
    }
}
#endif

static void aic_proc_normal_key(char c)
{
    uint16_t i, cur, len;

    /* Process normal key, arrived the max cmd len, skip it. */
    if (s_console.cmd_len +1 == CONS_CMD_SIZE)
        return;

    /* The cursor is in the middle of the cmd. */
    if (s_console.cmd_cur < s_console.cmd_len) {
        CMDMOV_NEXT(cur, len);
        s_console.cmd_buf[cur] = c;
        CONS_OUT("%s", &s_console.cmd_buf[cur]);

        /* move the cursor to new position */
        for (i = cur; i < len; i++)
            CONS_OUT("\b");
    } else {
        s_console.cmd_buf[s_console.cmd_len] = c;
        CONS_OUT("%c", c);
    }

    s_console.cmd_len++;
    s_console.cmd_cur++;
}

static void aic_cons_proc_char(char c)
{

    /* received null skip it. */
    if (c == '\0')
        return;

#ifdef CONFIG_CONSOLE_CMD_FULL_FUNC
#ifdef CONFIG_PLAT_CMD_HISTORY
    /* Process arrow key. */
    if (aic_cons_proc_arrow_key(c))
        return;
#endif

    /* Process tab key */
    if (c == '\t') {
        int i;

        /* move the cursor to the beginning of cmd_buf. */
        for (i = 0; i < s_console.cmd_cur; i++)
            CONS_OUT("\b");

        aic_cons_list_related_cmds(&s_console.cmd_buf[0], FALSE);
        s_console.cmd_cur = s_console.cmd_len = strlen(s_console.cmd_buf);

        return;
    }

    /* Process backspace  */
    if (c == 0x8 || c == 0x7f) {
        aic_proc_backspace();
        return;
    }
#endif

    /* Process end of cmd_buf */
    if (c == '\r' || c == '\n') {
#ifdef CONFIG_PLAT_CMD_HISTORY
        aic_cons_add_history(&s_console);
#endif
        CONS_OUT("\n");

        /* Process cmd. */
        aic_cmd_proc(s_console.cmd_buf, s_console.cmd_len);

        CONS_OUT(CONS_PROMPT);

        /* clear cmd buf. */
        memset(s_console.cmd_buf, 0, sizeof(s_console.cmd_buf));
        s_console.cmd_cur = s_console.cmd_len = 0;
        return;
    }

    aic_proc_normal_key(c);
}

#if IS_CONF_ENABLED(CONFIG_DIAG_CONSOLE)
#define DIAG_CONS_SUBCMD_00  0x00

static void diag_cons_send(void *data, size_t size)
{
    diag_rsp_cmd_t rsp;

    diag_set_rsp(&rsp, DIAG_SPEC_SN_CONSOLE, DIAG_RSP_SUCC, 0, AIC_THIS_SYS,
                 DIAG_CMD_CONSOLE, DIAG_CONS_SUBCMD_00, data, size);
    diag_cmd_response(&rsp);
}

static void aic_cons_diag_cb(diag_send_cmd_t *diag_cmd, void *pri)
{
    char *str, *buf;
    size_t len = diag_cmd->data_len;

    aicx_info(0x14cf31, "xd", "cons diag, recv subcmd=0x%x, size=%d\n", diag_cmd->subcmd, len);
    if (DIAG_CONS_SUBCMD_00 != diag_cmd->subcmd || len == 0) {
        diag_rsp_cmd_t rsp;

        aicx_error(0x0dede8, "x", "cons diag, recv  invalid subcmd=0x%x\n", diag_cmd->subcmd);
        diag_set_rsp(&rsp, DIAG_SPEC_SN_CONSOLE, DIAG_RSP_NO_FOUND_SUBCMD, 0,
                     diag_cmd->sys, DIAG_CMD_CONSOLE, DIAG_CONS_SUBCMD_00,
                     NULL, 0);
        diag_cmd_response(&rsp);
        return;
    }

    buf = aos_malloc(len + 1);
    memcpy(buf, diag_cmd->data, len);
    buf[len] = 0;
    str = buf;

    aicx_debug(0x0b4ac2, "s", "cons diag, recv str=%s\n", str);

    while (*str && len > 0) {
        aic_cons_proc_char(*str);
        str++;
        len--;
    }

    aos_free(buf);
}
#else
#ifdef CONFIG_CONNECT_MGR
static void aic_cons_recv_cb(u32 conn_id, u8 *buf, size_t size, void *p)
{
    char *str = (char *)buf;
    size_t len = size;

    while (*str && len > 0) {
        aic_cons_proc_char(*str);
        str++;
        len--;
    }

    /* must free the buf. */
    CONNMGR_FREE(buf);
}
#else
#ifdef CONFIG_SUBSYS_CP_ONLY
static char aic_cons_read_character(void)
{
    char character = 0;

    while (aio_read(s_console.cons_dev, &character, 1, AOS_WAIT_FOREVER) != 1)
        ;

    return character;
}

static void aic_cons_thread_entry(void *p)
{
    char c;

    /* show version and console prompt. */
    aic_software_version();
    CONS_OUT(CONS_PROMPT);

    while (1) {
        c = aic_cons_read_character();
        aic_cons_proc_char(c);
    }
}
#endif /* CONFIG_SUBSYS_CP_ONLY */
#endif /* CONFIG_CONNECT_MGR */
#endif /* CONFIG_DIAG_CONSOLE */

int aic_cons_init(void)
{
#if IS_CONF_ENABLED(CONFIG_DIAG_CONSOLE)
    diag_register_cmd_handler(DIAG_CMD_CONSOLE, aic_cons_diag_cb, NULL);
#else
#ifdef CONFIG_CONNECT_MGR
    conn_cmd_register_recv_cb(CONN_CMD_CONS, aic_cons_recv_cb, NULL);
#else
#ifdef CONFIG_SUBSYS_CP_ONLY
    /* request console io device. */
    s_console.cons_dev = aio_request_dev(AIC_CONSOLE_DEV, AIC_CONSOLE_COM);
    AIC_DUMMY_ASSERT(s_console.cons_dev);
    /* Create the main thread.  */
    (void)aos_create_thread_simply(CONS_NAME, aic_cons_thread_entry,
                                   NULL, CONS_STACK_SIZE, CONS_PRIORITY);
#endif /* CONFIG_SUBSYS_CP_ONLY */
#endif /* CONFIG_CONNECT_MGR */
#endif /* CONFIG_DIAG_CONSOLE */

    return 0;
}
AIC_EXTIMG_COMP_P2_INIT_EXPORT(aic_cons_init);

scall_func aic_cons_get_func(char *cmd, int size)
{
    const struct aic_cons_scall *cur;
    scall_func call_func = NULL;

    for (cur = (&s_scall_begin + 1); cur < &s_scall_end; cur++) {
        if (strncmp(cur->name, CONS_CMD_HEAD, CMD_HEAD_LEN) != 0)
            continue;

        if (strncmp(&cur->name[CMD_HEAD_LEN], cmd, size) == 0 &&
            cur->name[CMD_HEAD_LEN + size] == '\0') {
            call_func = (scall_func)cur->func;
            break;
        }
    }

    return call_func;
}

int aic_cons_help(int argc, char **argv)
{
    char cmd[1] = {0};

    UNUSED_PARAM(argv);

    if (argc > 0)
        return AIC_EINVAL;

    CONS_OUT("All cmd list:\n");
    aic_cons_list_related_cmds(cmd, FALSE);

    return 0;
}
CONSOLE_EXPORT_CMD(help, aic_cons_help, help);

void aic_cons_printf(const char *fmt, ...)
{
    va_list args;
    size_t len;
    char cons_buf[CONSOLE_BUF_SIZE];

    va_start(args, fmt);
    len = vsnprintf(cons_buf, sizeof(cons_buf) - 1, fmt, args);
    va_end(args);

    if (!len)
        return;

#if IS_CONF_ENABLED(CONFIG_DIAG_CONSOLE)
    diag_cons_send(cons_buf, len);
#else
#ifdef CONFIG_CONNECT_MGR
    conn_cmd_send(CONN_CMD_CONS, 0,  cons_buf, len, AOS_WAIT_FOREVER);
#else
    aio_write(s_console.cons_dev, cons_buf, len, AOS_WAIT_FOREVER);
#endif /* CONFIG_CONNECT_MGR */
#endif /* CONFIG_DIAG_CONSOLE */
}

#endif /* CONFIG_PLAT_CONSOLE */

