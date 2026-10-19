/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the cp diag input source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1b112e
#define LOG_MODULE    AMOD_DIAG

#include <diag_cmd_inner.h>

#define DIAG_INPUT_NAME         "dgin"
#define DIAG_INPUT_STACK_SIZE   SIZE_1K
#define DIAG_INPUT_THREAD_PRI   10
#define DIAG_ONECE_READ_SIZE    SIZE_1K

typedef struct diag_input {
    aos_thread_id   thread;
    ABRR_HANDLE     brr_handle;
    diag_proc_t     *proc;
    diag_rsp_t      *rsp;
} diag_input_t;

static diag_input_t *s_diag_input;

void diag_cmd_input_thread(void *p)
{
    diag_input_t *input = p;
    diag_recv_cmd_t *diag_cmd;

    while (1) {
        /* recv cmd and add to diag proc queue. */
        diag_cmd = diag_cmd_recv_cmd_by_brr(input->brr_handle, ADIAG_LINK);
        diag_iddbg(0x18246b, "xxxd", "cp recv cmd sn=0x%x, cmd=(0x%x, 0x%x), data len =%d\n",
                  diag_cmd->sn, diag_cmd->cmd, diag_cmd->subcmd, diag_cmd->data_len);

        diag_proc_add_cmd(input->proc, diag_cmd);
    }
}

void diag_cmd_response(diag_rsp_cmd_t *rsp_cmd)
{
    diag_rsp_response(s_diag_input->rsp, rsp_cmd);
}

int diag_cmd_input_init(void)
{
    diag_input_t *input = aos_malloc(sizeof(*input));

    AIC_DUMMY_ASSERT(input);
    s_diag_input = input;

    diag_iddbg(0x014317, "", "diag input init\n");

    /* request the diag brr. */
    input->brr_handle = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DIAG);
    AIC_DUMMY_ASSERT(input->brr_handle);

    /* create diag responser. */
    input->rsp = diag_cmd_rsp_create(input->brr_handle);

    /* create diag processor */
    input->proc = diag_cmd_proc_create();

    /* create recv brr cmd thread. */
    input->thread =
        aos_create_thread_simply(DIAG_INPUT_NAME,
                                 diag_cmd_input_thread, input,
                                 DIAG_INPUT_STACK_SIZE, DIAG_INPUT_THREAD_PRI);
    AIC_DUMMY_ASSERT(AOS_INVALID_THREAD_ID != input->thread );

    return AOS_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(diag_cmd_input_init);

