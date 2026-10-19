/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the diag cmd processor source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ce3d5
#define LOG_MODULE    AMOD_DIAG

#include <diag_cmd_inner.h>

typedef struct diag_cb {
    DIAG_HANDLER_T handler;
    void           *private;
} diag_cb_t;

#ifdef CONFIG_SUBSYS_AP
#define DIAG_PROC_STACK_SIZE    SIZE_4K
#define DIAG_PROC_PRI           5
#else
#define DIAG_PROC_STACK_SIZE    SIZE_4K
#define DIAG_PROC_PRI           15
#endif
#define DIAG_PROC_QCNT          10
#define DIAG_PROC_NAME          "dgprc"

/* cmd handler table */
static diag_cb_t s_diag_cb[DIAG_CMD_CNT];

void diag_register_cmd_handler(u8 cmd, DIAG_HANDLER_T handler, void *pri)
{
    AIC_DUMMY_ASSERT(cmd < DIAG_CMD_CNT);
    if (cmd >= DIAG_CMD_CNT)
        return;

    diag_iddbg(0x055f02, "x", "register handler, cmd=0x%x\n", cmd);

    s_diag_cb[cmd].handler = handler;
    s_diag_cb[cmd].private = pri;
}

void *diag_proc_find_handler(u8 cmd)
{
    return s_diag_cb[cmd].handler;
}

static void diag_proc_task(void *p)
{
    diag_proc_t *proc = (diag_proc_t *)p;
    diag_recv_cmd_t *recv_cmd;
    DIAG_HANDLER_T handler;
    diag_rsp_cmd_t rsp;
    void *pri;
    u8 index;
    int ret;

    diag_iddbg(0x1d5728, "", "cmd proc task\n");

    while (TRUE) {
        ret = aos_recv_from_queue(proc->queue, (void*)&recv_cmd, AOS_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AOS_SUCCESS);
        index = recv_cmd->cmd;

        aicx_info(0x01e112, "xxxd", "proc sn=0x%x, cmd=(0x%x, 0x%x), data len =%d\n",
                  recv_cmd->sn, recv_cmd->cmd, recv_cmd->subcmd, recv_cmd->data_len);

        if (index >= DIAG_CMD_CNT) {
            aicx_error(0x1646e6, "x", "invalid cmd=0x%x\n", recv_cmd->cmd);
            diag_set_rsp(&rsp, recv_cmd->sn, DIAG_RSP_NO_FOUND_CMD,
                         recv_cmd->m2, recv_cmd->sys, recv_cmd->cmd,
                         recv_cmd->subcmd, NULL, 0);
            diag_cmd_response(&rsp);
            diag_recv_cmd_free(recv_cmd);
            continue;
        }

        /* call the cmd callback function. */
        handler = s_diag_cb[index].handler;
        pri = s_diag_cb[index].private;

        if (handler)
            handler(recv_cmd, pri);
        else {
            aicx_error(0x16abae, "x", "can't find diag cmd(0x%x) handle\n", recv_cmd->cmd);
            diag_set_rsp(&rsp, recv_cmd->sn, DIAG_RSP_NO_FOUND_CMD_HANDLER,
                         recv_cmd->m2, recv_cmd->sys, recv_cmd->cmd,
                         recv_cmd->subcmd, NULL, 0);
            diag_cmd_response(&rsp);
        }

        /* must free the cmd. */
        diag_recv_cmd_free(recv_cmd);
    }
}

void diag_proc_add_cmd(diag_proc_t *proc, diag_recv_cmd_t *recv_cmd)
{
    aos_send_to_queue(proc->queue, (void*)&recv_cmd, AOS_WAIT_FOREVER);
}

diag_proc_t *diag_cmd_proc_create(void)
{
    diag_proc_t *proc;

    proc = aos_malloc(sizeof(*proc));
    AIC_DUMMY_ASSERT(proc);

    diag_iddbg(0x14b694, "", "cmd proc init\n");

    /* create processor thread. */
    proc->thread =
        aos_create_thread_simply(DIAG_PROC_NAME, diag_proc_task, proc,
                                 DIAG_PROC_STACK_SIZE, DIAG_PROC_PRI);
    AIC_DUMMY_ASSERT(proc->thread != AOS_INVALID_THREAD_ID);

    /* create processor queue. */
    proc->queue = aos_create_queue(DIAG_PROC_NAME, sizeof(void*), DIAG_PROC_QCNT);
    AIC_DUMMY_ASSERT(proc->queue);

    return proc;
}

void diag_cmd_proc_destroy(diag_proc_t *proc)
{
    aos_delete_thread(proc->thread);
    aos_delete_queue(proc->queue);
    aos_free(proc);
}

