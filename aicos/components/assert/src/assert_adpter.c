/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert Interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x146a73
#define LOG_MODULE    AMOD_SYS

#include <aic_errno.h>
#include <aic_cmd.h>
#include <aic_mcx.h>
#include <aic_osa.h>
#include <ambox_api.h>
#include <wdt.h>
#include <timer_inc.h>
#include <aipc_api.h>
#include <assert_adpter.h>
#include <diag_cmd_inner.h>
#include <stdio.h>

#define MAX_COMM_BLK_SIZE           160
#define MAX_WAIT_ASSERT_ACK_TIME    1000

#ifdef CONFIG_SUBSYS_AP
int ap_notify_other_sys_assert(u8 sys)
{
    char *str;
    ABLK_T blk;
    AMSG_T msg;
    ABRR_HANDLE brr;
    int left_time = MAX_WAIT_ASSERT_ACK_TIME;
    int ret = -1;

    amsg_fill_msg(&msg, AMOD_SYS, AIPC_MSG_ASSERT, 0);
    (void)amsg_raw_send_msg(sys, &msg);

    brr = ablk_request_brr(AIC_SYS_CP, AIPC_BRR_COMM);
    if (brr) {
        /* wait assert ack, until timeout. */
        while (left_time > 0) {
            aic_mdelay(10);
            left_time -= 10;

            if (AIC_SUCCESS != ablk_recv_block_any(brr, STATUS_LINK, &blk, FALSE))
                continue;

            str = (char *)blk.addr;
            *(str + blk.size) = 0;

            /* wait ack, break. */
            if (strstr(str, CP_ASSERT_ACK)) {
                left_time = 0;
                ret = 0;
            }

            ablk_release_block_any(brr, &blk, FALSE);
        }
    } else {
        /* If get brr fail, still return 0. */
        ret = 0;
    }

    return ret;
}
#endif /* CONFIG_SUBSYS_AP */

#ifdef CONFIG_SUBSYS_CP
#define ASSERT_GET_BLOCK(brr, size, blk, ret) do { \
        ret = ablk_get_block_any(brr, size, &blk, FALSE); \
        ambox_empty_recvbox_inassert(); \
    } while (ret != AIC_SUCCESS)

#define ASSERT_SEND_BLOCK(brr, link, blk, ret) do { \
        ret = ablk_send_block_any(brr, link, &blk, FALSE); \
        ambox_empty_recvbox_inassert(); \
    } while(ret != AIC_SUCCESS)

#define ASSERT_RECV_BLOCK(brr, link, blk, ret) do { \
        ret = ablk_recv_block_any(brr, link, &blk, FALSE); \
        ambox_empty_recvbox_inassert(); \
    } while(ret != AIC_SUCCESS)

diag_recv_cmd_t s_diag_cmd;

static void assert_diag_encode_cmd(diag_rsp_cmd_t *rsp_cmd,
                                   DIAG_CMD_HEAD_T *diag_cmd)
{
    diag_cmd->magic = DIAG_MAGIC;
    diag_cmd->sn = rsp_cmd->sn;
    diag_cmd->cmd = rsp_cmd->cmd;
    diag_cmd->subcmd = rsp_cmd->subcmd;
    diag_cmd->data_len = rsp_cmd->data_len;
    diag_cmd->err_no = rsp_cmd->err_no;
    diag_cmd->sys = AIC_THIS_SYS;
}

static void assert_diag_send_cmd(ABRR_HANDLE brr, u8 link,
                                 DIAG_CMD_HEAD_T *diag_cmd, void *data)
{
    size_t once, cpy, left = DIAG_CMD_PACKET_LEN(diag_cmd);
    u8 *dst, *src = (u8 *)data;
    bool first = TRUE, last;
    ABLK_T blk;
    int ret;

    while (left) {
        once = MIN(SIZE_2K, left);
        last = (left == once);

        /* poll get blk. */
        ASSERT_GET_BLOCK(brr, once, blk, ret);
        dst = blk.addr;

        if (first) {
            first = 0;

           /* copy the head */
            memcpy(dst, diag_cmd, sizeof(*diag_cmd));
            left -= sizeof(*diag_cmd);
            once -= sizeof(*diag_cmd);
            dst += sizeof(*diag_cmd);
        }

        /* copy data, src can be NULL, becuase the cp boot code addres is 0. */
        cpy = last ? (once - 1) : once;
        memcpy(dst, src, cpy);
        left -= cpy;
        dst += cpy;

        if (last) {
            /* add the tail. */
            *dst = DIAG_TAIL;
            left -= 1;

            /* left must be 0 here. */
            while (left);
        }

        /* poll send blk. */
        ASSERT_SEND_BLOCK(brr, ADIAG_LINK, blk, ret);
        src += once;
    }
}

void cp_assert_notify_ap(const char *info_string)
{
    ABRR_HANDLE brr;
    ABLK_T blk;
    size_t size;
    int ret;

    /* use AIPC_BRR_COMM, STATUS_LINK to notify. */
    brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_COMM);
    if (brr) {
        size = strlen(info_string) + 1;
        if (size > MAX_COMM_BLK_SIZE)
            size = MAX_COMM_BLK_SIZE;

        ASSERT_GET_BLOCK(brr, size, blk, ret);

        memcpy(blk.addr, info_string, size);
        blk.size = size;
        if (size == MAX_COMM_BLK_SIZE)
            *((u8*)blk.addr + size - 1) = 0;

        ASSERT_SEND_BLOCK(brr, STATUS_LINK, blk, ret);

        ablk_release_brr(brr);
    }
}

bool cp_assert_is_brr_blocked(u8 brr)
{
    ABRR_HANDLE handle;
    handle = ablk_request_brr(AIC_SYS_AP, brr);
    if (handle) {
        return ablk_no_free_blk(handle);
    }

    return false;
}

diag_recv_cmd_t *assert_diag_cmd_recv(void)
{
    ABRR_HANDLE brr;
    ABLK_T blk;
    int ret;

    brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DIAG);
    while (!brr)
        ;

    /* poll diag link. */
    ASSERT_RECV_BLOCK(brr, ADIAG_LINK, blk, ret);

    /* assert, only need diag_head. */
    memcpy(&s_diag_cmd, blk.addr, sizeof(s_diag_cmd));

    ablk_release_block_any(brr, &blk, FALSE);

    return &s_diag_cmd;
}

void assert_diag_cmd_response(diag_rsp_cmd_t *rsp_cmd)
{
    ABRR_HANDLE brr;
    DIAG_CMD_HEAD_T diag_cmd;

    brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DIAG);
    while (!brr)
        ;

    assert_diag_encode_cmd(rsp_cmd, &diag_cmd);
    assert_diag_send_cmd(brr, ADIAG_LINK, &diag_cmd, rsp_cmd->data);
}
#endif /* CONFIG_SUBSYS_CP */

void sys_notify_ap_assert(void)
{
#ifdef CONFIG_SUBSYS_AP
    AIC_ASSERT(0);
#else
    AOS_INTERRUPT_DECLARE
    AMSG_T msg;

    AOS_INTERRUPT_DISABLE

    amsg_fill_msg(&msg, AMOD_SYS, AIPC_MSG_ASSERT, 0);
    (void)amsg_raw_send_msg(AIC_SYS_AP, &msg);

    AIC_ASSERT(0);
    AOS_INTERRUPT_RESTORE
#endif
}

