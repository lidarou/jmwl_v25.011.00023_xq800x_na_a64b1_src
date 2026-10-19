/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the diag cmd responser source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x11a0c3
#define LOG_MODULE    AMOD_DIAG

#include <diag_cmd_inner.h>

#define DIAG_RSP_STACK_SIZE     512
#define DIAG_RSP_PRI            5
#define DIAG_RSP_QCNT           10
#define DIAG_RSP_NAME          "dgrsp"

static void diag_rsp_task(void *p)
{
    diag_rsp_t *rsp = (diag_rsp_t *)p;
    DIAG_CMD_HEAD_T *diag_cmd;
    int ret;

    diag_iddbg(0x1b2a6d, "", "cmd rsp task\n");

    while (TRUE) {
        ret = aos_recv_from_queue(rsp->queue, (void*)&diag_cmd, AOS_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AOS_SUCCESS);

        diag_iddbg(0x04c12c, "xxxx", "diag rsp sn=0x%x, recv cmd=(0x%x, 0x%x), data_len=0x%x\n",
                   diag_cmd->sn, diag_cmd->cmd, diag_cmd->subcmd, diag_cmd->data_len);

#ifdef CONFIG_CONNECT_MGR
        /* send to pc. */
        diag_conn_write(diag_cmd, DIAG_CMD_PACKET_LEN(diag_cmd));
#else
        /* send to remote sys. */
        diag_cmd_send_rsp_by_brr(rsp->handle, ADIAG_LINK, diag_cmd);
#endif

        /* must free the data. */
        aos_free(diag_cmd);
    }
}

void diag_rsp_response(diag_rsp_t *rsp, diag_rsp_cmd_t *rsp_cmd)
{
    DIAG_CMD_HEAD_T *diag_cmd;
    int ret;

    diag_cmd = diag_encode_cmd(rsp_cmd);
    diag_iddbg(0x0feb12, "xxxxd", "rsp cmd, sn=0x%x, sys=0x%x, cmd=(0x%x, 0x%x), data len =%d\n",
               diag_cmd->sn, diag_cmd->sys, diag_cmd->cmd,
               diag_cmd->subcmd, diag_cmd->data_len);

    ret = aos_send_to_queue(rsp->queue, (void*)&diag_cmd, AOS_WAIT_FOREVER);
    AOS_ASSERT(ret == AOS_SUCCESS);
}

diag_rsp_t *diag_cmd_rsp_create(void *handle)
{
    diag_rsp_t *rsp;

    rsp = aos_malloc(sizeof(*rsp));
    AIC_DUMMY_ASSERT(rsp);

    diag_iddbg(0x0a5d15, "", "cmd resp init\n");

    /* save the handle. */
    rsp->handle = handle;

    /* create responser thread. */
    rsp->thread =
        aos_create_thread_simply(DIAG_RSP_NAME, diag_rsp_task, rsp,
                                 DIAG_RSP_STACK_SIZE, DIAG_RSP_PRI);
    AIC_DUMMY_ASSERT(rsp->thread != AOS_INVALID_THREAD_ID);

    /* create responser queue. */
    rsp->queue = aos_create_queue(DIAG_RSP_NAME, sizeof(void*), DIAG_RSP_QCNT);
    AIC_DUMMY_ASSERT(rsp->queue);

    return rsp;
}

void diag_cmd_rsp_destroy(diag_rsp_t *rsp)
{
    aos_delete_thread(rsp->thread);
    aos_delete_queue(rsp->queue);
    aos_free(rsp);
}

