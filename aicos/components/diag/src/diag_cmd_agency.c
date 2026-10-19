/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the diag cmd agency source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x03ebec
#define LOG_MODULE    AMOD_DIAG

#include <adbg_cfg_api.h>
#include <diag_cmd_inner.h>
#include <connmgr_api.h>

#define DIAG_AGENCY_CMD_NAME        "agcmd"
#define DIAG_AGENCY_RSP_NAME        "agrsp"

#define DIAG_AGENCY_CMD_STACK_SIZE  1024
#define DIAG_AGENCY_RSP_STACK_SIZE  512

#define DIAG_AGENCY_THREAD_PRI      5
#define DIAG_ONECE_SIZE             SIZE_4K

typedef struct diag_agency {
    u8              iodev;
    volatile u8     b_stop;
    aos_thread_id   cmd_thread;
    aos_thread_id   rsp_thread;
    ABRR_HANDLE     brr_handle;
    void            *diag_codec;
    diag_proc_t     *proc;
    diag_rsp_t      *rsp;
    u8              *buf;
} diag_agency_t;

static diag_agency_t *s_diag_agency;

#ifdef CONFIG_CONNECT_MGR
void diag_conn_write(void *buf, size_t size)
{
    size_t write, left = size;

    do {
        /* old dev can't used, discard it. */
        if (!s_diag_agency || s_diag_agency->b_stop)
            break;

        write = conn_data_send(CONN_DATA_DIAG, buf ,left, AOS_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(write <= left);
        if (write > left)
            break;

        left -= write;
    } while (left);
}
#endif

void diag_cmd_recv_thread(void *p)
{
    diag_agency_t *agency = p;
    u8 *buf = agency->buf;
    diag_recv_cmd_t *recv_cmd;
    size_t read, decode = 0;
    diag_rsp_cmd_t rsp;

    diag_iddbg(0x0d22e1, "", "diag cmd recv thread\n");
    agency->b_stop = 0;

    while (!agency->b_stop) {
        /* read the cmd from diag connect. */
        read = conn_data_recv(CONN_DATA_DIAG, agency->buf, DIAG_ONECE_SIZE, AOS_WAIT_FOREVER);
        if (agency->b_stop) {
            diag_iddbg(0x03d0eb, "", "exit diag_cmd_recv_thread!\n");
            return;
        }

        diag_iddbg(0x0d7f01, "d", "recv pc read =%d\n", read);
#ifdef WIN32
        if (read == 0)
            break;
#endif
        buf = agency->buf;

        while (read) {
            /* decode a cmd. */
            recv_cmd = diag_decode_cmd(agency->diag_codec, buf, read, &decode);
            if (recv_cmd) {
                /* may left some data after finish the cmd decode. */
                AIC_ASSERT(decode <= read);
                read -= decode;
                buf += decode;

                diag_iddbg(0x05bc37, "xxxxd", "recv pc sn=0x%x, sys=0x%x, cmd=(0x%x, 0x%x), data len =%d\n",
                           recv_cmd->sn, recv_cmd->sys, recv_cmd->cmd,
                           recv_cmd->subcmd, recv_cmd->data_len);

                /* check tail fail, resposeit and continue. */
                if (recv_cmd->err_no == DIAG_RSP_TAIL_CHECK_FAIL) {
                    aicx_error(0x1a5959, "", "Tail check failed\n");
                    diag_set_rsp(&rsp, recv_cmd->sn, DIAG_RSP_TAIL_CHECK_FAIL,
                                 recv_cmd->m2, recv_cmd->sys,  recv_cmd->cmd,
                                 recv_cmd->subcmd, NULL, 0);
                    diag_cmd_response(&rsp);
                    diag_recv_cmd_free(recv_cmd);
                    continue;
                }

               /* auto sys, will find in ap, if not found give it to cp. */
               if (recv_cmd->sys == AIC_SYS_AUTO) {
                    if (diag_proc_find_handler(recv_cmd->cmd))
                        recv_cmd->sys = AIC_SYS_AP;
                    else
                        recv_cmd->sys = AIC_SYS_CP;
                }

                if (recv_cmd->sys == AIC_SYS_AP) {
                    /* ap diag, add to diag proc queue */
                    diag_proc_add_cmd(agency->proc, recv_cmd);
                } else if (recv_cmd->sys == AIC_SYS_CP){
                    /* pass it to cp */
                    diag_cmd_send_cmd_by_brr(agency->brr_handle, ADIAG_LINK, recv_cmd);
                    diag_recv_cmd_free(recv_cmd);
                } else {
                    /* no found sys */
                    aicx_info(0x1d7bbb, "d", "no foud sys=%d.\n", recv_cmd->sys);
                    diag_set_rsp(&rsp, recv_cmd->sn, DIAG_RSP_NO_FOUND_SYS, recv_cmd->m2,
                                 recv_cmd->sys, recv_cmd->cmd, recv_cmd->subcmd,
                                 NULL, 0);
                    diag_cmd_response(&rsp);
                    diag_recv_cmd_free(recv_cmd);
                }
            } else {
                /* all the data in the buf be decoded. */
                read = 0;
            }
        }
    }
}

void diag_cmd_rsp_thread(void *p)
{
    diag_agency_t *agency = p;
    DIAG_CMD_HEAD_T *diag_cmd;

    diag_iddbg(0x05c4d6, "", "diag cmd rsp thread\n");

    while (1) {
        diag_cmd = diag_cmd_recv_rsp_by_brr(agency->brr_handle, ADIAG_LINK);
        if (diag_cmd) {
            /* recv a cmd send to AP */
            diag_recv_cmd_t *recv_cmd;
            u8 *diag_data;

            aicx_info(0x03b88d, "xxxd", "recv from brr cmd(0x%x, 0x%x), sn=0x%x, data len =%d\n",
                      diag_cmd->cmd, diag_cmd->subcmd,
                      diag_cmd->sn, diag_cmd->data_len);

            /* malloc the recv cmd */
            recv_cmd = aos_malloc(sizeof(*recv_cmd));
            AIC_DUMMY_ASSERT(recv_cmd);

            /* malloc the data */
            if (diag_cmd->data_len) {
                diag_data = aos_malloc(diag_cmd->data_len);
                AIC_DUMMY_ASSERT(diag_data);
                memcpy(diag_data, diag_cmd + 1, diag_cmd->data_len);
            } else {
                diag_data = NULL;
            }
            diag_set_recv(recv_cmd, diag_cmd->sn,
                          diag_cmd->err_no, 1, diag_cmd->sys,
                          diag_cmd->cmd, diag_cmd->subcmd,
                          diag_data, diag_cmd->data_len);

            diag_proc_add_cmd(agency->proc, recv_cmd);
            aos_free(diag_cmd);
        }
    }
}

void ap_send_diag_cmd_to_cp(u8 cmd, u16 subcmd, void *data, size_t size)
{
    diag_send_cmd_t diag_cmd;

    AIC_ASSERT(s_diag_agency && s_diag_agency->brr_handle);

    diag_set_send(&diag_cmd, 0, DIAG_RSP_SUCC, 1, AIC_SYS_CP,
                  cmd, subcmd, data, size);

    diag_cmd_send_cmd_by_brr(s_diag_agency->brr_handle, ADIAG_LINK, &diag_cmd);
}

void diag_cmd_response(diag_rsp_cmd_t *rsp_cmd)
{
    if (rsp_cmd->m2) {
        diag_iddbg(0x1260de, "", "m2 diag cmd can't sent to pc tools.\n");
        return;
    }

    diag_rsp_response(s_diag_agency->rsp, rsp_cmd);
}

int diag_cmd_agency_init(void)
{
    diag_agency_t *agency = aos_malloc(sizeof(*agency));
    void *buf = aos_dma_malloc(DIAG_ONECE_SIZE);

    AIC_ASSERT(agency);
    AIC_ASSERT(buf);

    s_diag_agency = agency;

    diag_iddbg(0x09564e, "", "diag agency init\n");
    agency->buf = buf;

    /* create diag codec. */
    agency->diag_codec = diag_codec_create();

    /* create diag responser. */
    agency->rsp = diag_cmd_rsp_create(agency->diag_codec);

    /* create diag processor */
    agency->proc = diag_cmd_proc_create();

    /* get the diag device. */
    agency->iodev = adbg_get_diag_dev();

#ifndef WIN32
    if (agency->iodev != AIC_IODEV_USB &&
        agency->iodev != AIC_IODEV_UART) {
        diag_iddbg(0x0ed83f, "d", "iodev error=%d. set do usb.\n", agency->iodev);
        adbg_set_diag_dev(AIC_IODEV_USB);
        agency->iodev = AIC_IODEV_USB;
    }
#endif

    /* request the diag brr. */
    agency->brr_handle = ablk_request_brr(AIC_SYS_CP, AIPC_BRR_DIAG);
    AIC_ASSERT(agency->brr_handle);

    /* set connect device. */
    if (conn_data_request_iodev(CONN_DATA_DIAG, agency->iodev) == AIC_SUCCESS) {
        /* create recv pc cmd thread. */
        agency->cmd_thread =
            aos_create_thread_simply(DIAG_AGENCY_CMD_NAME,
                                     diag_cmd_recv_thread, agency,
                                     DIAG_AGENCY_CMD_STACK_SIZE, DIAG_AGENCY_THREAD_PRI);
        AIC_DUMMY_ASSERT(AOS_INVALID_THREAD_ID != agency->cmd_thread);
    } else {
        agency->cmd_thread = AOS_INVALID_THREAD_ID;
    }

        /* create recv brr cmd thread. */
    agency->rsp_thread =
        aos_create_thread_simply(DIAG_AGENCY_RSP_NAME,
                                 diag_cmd_rsp_thread, agency,
                                 DIAG_AGENCY_RSP_STACK_SIZE, DIAG_AGENCY_THREAD_PRI);
    AIC_DUMMY_ASSERT(AOS_INVALID_THREAD_ID != agency->rsp_thread);

    return AOS_SUCCESS;
}
AIC_COMP_P2_INIT_EXPORT(diag_cmd_agency_init);

void diag_cmd_agency_reset_iodev(void)
{
    diag_agency_t *agency = s_diag_agency;
    u8 new_iodev = adbg_get_diag_dev();

    if (!agency || new_iodev == agency->iodev)
        return;

    agency->iodev = new_iodev;
    agency->b_stop = 1;

    if (agency->iodev != AIC_IODEV_NONE)
        conn_data_release_iodev(CONN_DATA_DIAG);

    /* wait for diag_cmd_recv_thread return from iodev recv. */
    aos_sleep(1000);

    /* delete the old cmd thread. */
    if (agency->cmd_thread != AOS_INVALID_THREAD_ID) {
        aos_delete_thread(agency->cmd_thread);
        agency->cmd_thread = AOS_INVALID_THREAD_ID;
    }

    if (agency->iodev == AIC_IODEV_NONE)
        return;

    /* set the connect device. */
    if (AIC_SUCCESS == conn_data_request_iodev(CONN_DATA_DIAG, agency->iodev)) {
        /* create new cmd thread. */
        agency->cmd_thread =
            aos_create_thread_simply(DIAG_AGENCY_CMD_NAME,
                                     diag_cmd_recv_thread, agency,
                                     DIAG_AGENCY_CMD_STACK_SIZE, DIAG_AGENCY_THREAD_PRI);
        AIC_DUMMY_ASSERT(AOS_INVALID_THREAD_ID != agency->cmd_thread);
    }
}

