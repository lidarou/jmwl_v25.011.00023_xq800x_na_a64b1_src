/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This is the diag cmd brr source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x19edf7
#define LOG_MODULE    AMOD_DIAG

#include <diag_cmd_inner.h>
#include <connmgr_api.h>
#include <alog_store_api.h>
#include <adbg_cfg_api.h>
#ifdef CONFIG_ALOG_STORE_FS
#ifdef WIN32
#define ASSERT_ASSERT_SUBCMD    1
#define ASSERT_ALLINFO_SUBCMD   (u16)'a'
#define ASSERT_RESET_SUBCMD     (u16)'r'
#define ASSERT_DUMP_SUBCMD      (u16)'d'
#define ASSERT_DUMP_FINISH      (u16)'f'
#else
#include <assert_adpter.h>
#endif
#endif

#define DIAG_RECV_HEAD_SIZE         (sizeof(diag_recv_cmd_t) - sizeof(void *))
#define DIAG_RECV_PACKET_LEN(cmd)   (DIAG_RECV_HEAD_SIZE + (cmd)->data_len)

#ifdef CONFIG_SUBSYS_CP
diag_recv_cmd_t *diag_cmd_recv_cmd_by_brr(ABRR_HANDLE brr, u8 link)
{
    diag_recv_cmd_t *recv_cmd;
    size_t left, cpy;
    ABLK_T blk;
    u8 *dst, *src;
    int ret;

    /* check head. */
    do {
        ret = ablk_recv_block(brr, link, &blk, AIPC_WAIT_FOREVER);
    } while (AIC_EAGAIN == ret);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

    /* make sure blk.size >= DIAG_RECV_HEAD_SIZE */
    AIC_ASSERT(blk.size >= DIAG_RECV_HEAD_SIZE);
    src = (u8 *)blk.addr;

    /* recv head */
    recv_cmd = aos_malloc(sizeof(*recv_cmd));
    AIC_DUMMY_ASSERT(recv_cmd);
    memcpy(recv_cmd, src, DIAG_RECV_HEAD_SIZE);

    diag_iddbg(0x006b9a, "xxxdd", "recv cmd(0x%x, 0x%x) from brr, sn=0x%x, size=%d, total_len=%d\n",
               recv_cmd->cmd, recv_cmd->subcmd,  recv_cmd->sn, blk.size, recv_cmd->data_len);

    if (recv_cmd->data_len == 0) {
        recv_cmd->data = NULL;
        ablk_release_block(brr, &blk);
        return recv_cmd;
    }

    /* recv data */
    recv_cmd->data = aos_malloc(recv_cmd->data_len);
    AIC_DUMMY_ASSERT(recv_cmd->data);

    src += DIAG_RECV_HEAD_SIZE;
    left = recv_cmd->data_len;

    /* copy the left data in first blk. */
    cpy = (blk.size - DIAG_RECV_HEAD_SIZE);
    dst = (u8 *)recv_cmd->data;
    memcpy(dst, src, cpy);
    left -= cpy;
    dst += cpy;
    ablk_release_block(brr, &blk);

    /* recv left data */
    while (left) {
        ret = ablk_recv_block(brr, link, &blk, AIPC_WAIT_FOREVER);

        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
        AIC_DUMMY_ASSERT(blk.size <= left);
        memcpy(dst, (u8 *)blk.addr, blk.size);

        left -= blk.size;
        dst += blk.size;

        diag_iddbg(0x1cdec9, "xxdd", "recv cmd(0x%x, 0x%x) from brr, size =%d, left=%d\n",
                  recv_cmd->cmd, recv_cmd->subcmd, blk.size, left);
        ablk_release_block(brr, &blk);
    }

    aicx_info(0x12893d, "xxd", "recv brr cmd=(0x%x, 0x%x) done, data len =%d\n",
              recv_cmd->cmd, recv_cmd->subcmd, recv_cmd->data_len);

    return recv_cmd;
}

void diag_cmd_send_rsp_by_brr(ABRR_HANDLE brr, u8 link, DIAG_CMD_HEAD_T *diag_cmd)
{
    size_t once, left = DIAG_CMD_PACKET_LEN(diag_cmd);
    u8 *dst, *src = (u8 *)diag_cmd;
    ABLK_T blk;
    int ret;

    diag_iddbg(0x03138b, "xxd", "send cmd(0x%x, 0x%x) to brr, total len=%d\n",
              diag_cmd->cmd, diag_cmd->subcmd,  left);

    while (left) {
        once = MIN(SIZE_2K, left);
        ret = ablk_get_block(brr, once, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

        left -= once;
        dst = blk.addr;

        /* copy cmd */
        memcpy(dst, src, once);
        ret = ablk_send_block(brr, link, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

        diag_iddbg(0x00f1ea, "xxdd", "send cmd(0x%x, 0x%x) to brr, size =%d, left=%d\n",
                  diag_cmd->cmd, diag_cmd->subcmd, blk.size, left);

        src += once;
    }

    aicx_info(0x063cb8, "xxd", "send cmd(0x%x, 0x%x) to brr done, data len =%d\n",
              diag_cmd->cmd, diag_cmd->subcmd, diag_cmd->data_len);
}
#endif

#if defined (CONFIG_SUBSYS_AP) || defined (WIN32)
void diag_cmd_send_cmd_by_brr(ABRR_HANDLE brr, u8 link, diag_recv_cmd_t *recv_cmd)
{
    size_t once, left, total = DIAG_RECV_PACKET_LEN(recv_cmd);
    u8 *dst, *src = recv_cmd->data;
    ABLK_T blk;
    int ret;

    left = total;

    diag_iddbg(0x1cddb3, "xxxd", "send cmd(0x%x, 0x%x) to brr, sn=0x%x, total len=%d\n",
              recv_cmd->cmd, recv_cmd->subcmd,  recv_cmd->sn, recv_cmd->data_len);

    while (left) {
        once = MIN(SIZE_2K, left);
        ret = ablk_get_block(brr, once, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
        dst = (u8 *)blk.addr;

        if (left == total) {
            /* first copy head. */
            memcpy(dst, (u8 *)recv_cmd, DIAG_RECV_HEAD_SIZE);
            once -= DIAG_RECV_HEAD_SIZE;
            left -= DIAG_RECV_HEAD_SIZE;
            dst += DIAG_RECV_HEAD_SIZE;
        }

        /* copy data */
        memcpy(dst, src, once);
        ret = ablk_send_block(brr, link, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
        left -= once;
        src += once;

        diag_iddbg(0x00f1ea, "xxdd", "send cmd(0x%x, 0x%x) to brr, size =%d, left=%d\n",
                  recv_cmd->cmd, recv_cmd->subcmd, blk.size, left);
    }

    aicx_info(0x143761, "xxxd", "send sn=0x%x, cmd=(0x%x, 0x%x) to brr done, data len =%d\n",
              recv_cmd->sn, recv_cmd->cmd,
              recv_cmd->subcmd, recv_cmd->data_len);
}

static void diag_cmd_brr_write(u8 cmd, u16 subcmd,void *addr, size_t size,
                               bool head, bool tail)
{
#ifdef CONFIG_ALOG_STORE_FS
    /* remove the head and the tail. */
    u8 *data = (u8 *)addr;
    size_t save = size;

    if (head) {
        data += sizeof(DIAG_CMD_HEAD_T);
        save -= sizeof(DIAG_CMD_HEAD_T);
    }

    if (tail)
        save -= 1;

    /* log save to file. */
    if (cmd == DIAG_CMD_ASSERT &&
        AIC_IODEV_FILE == adbg_get_log_dev() &&
        alog_store_is_ready()) {

        switch (subcmd) {
        case ASSERT_DUMP_SUBCMD:
            alog_store_save_dump(data, save, FALSE);
            return;

        case ASSERT_DUMP_FINISH:
            alog_store_save_dump(data, save, TRUE);
            return;

        case ASSERT_ALLINFO_SUBCMD:
            alog_store_save_assert_info(data, save, FALSE);
            return;

        default:
            break;
        }
    }
#endif

    diag_conn_write(addr,size);
}

DIAG_CMD_HEAD_T *diag_cmd_recv_rsp_by_brr(ABRR_HANDLE brr, u8 link)
{
    DIAG_CMD_HEAD_T *diag_cmd = NULL;
    DIAG_CMD_HEAD_T *diag_head;
    u8 *dst = NULL;
    size_t left;
    ABLK_T blk;
    u16 subcmd = 0;
    u8 cmd = 0;
    int ret;

    /* check head. */
    do {
        ret = ablk_recv_block(brr, link, &blk, AIPC_WAIT_FOREVER);
    } while (AIC_EAGAIN == ret);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

    diag_head = (DIAG_CMD_HEAD_T *)blk.addr;
    aicx_info(0x1bc2f1, "xxxd", "recv rsp sn=0x%x, cmd=(0x%x, 0x%x) from brr, data len =%d\n",
              diag_head->sn, diag_head->cmd, diag_head->subcmd, diag_head->data_len);

    AIC_DUMMY_ASSERT(diag_head->magic == DIAG_MAGIC ||
               diag_head->magic == DIAG_MAGIC_2);
    left = DIAG_CMD_PACKET_LEN(diag_head);
    AIC_ASSERT(blk.size <= left);

    if (diag_head->magic == DIAG_MAGIC_2) {
        /* magic2, not to pc, alloc cmd. */
        diag_cmd = aos_malloc(left);
        AIC_DUMMY_ASSERT(diag_cmd);
    }

    /* first blk */
    left -= blk.size;

    if (diag_cmd) {
        dst = (u8 *)diag_cmd;
        memcpy(dst, (u8 *)blk.addr, blk.size);
        dst += blk.size;
    } else {
        cmd = diag_head->cmd;
        subcmd = diag_head->subcmd;
        diag_cmd_brr_write(cmd, subcmd, blk.addr, blk.size, TRUE, (left == 0));
    }

    ablk_release_block(brr, &blk);

    /* left blk */
    while (left) {
        ret = ablk_recv_block(brr, link, &blk, AIPC_WAIT_FOREVER);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);
        AIC_ASSERT(blk.size <= left);

        left -= blk.size;

        if (diag_cmd) {
            memcpy(dst, (u8 *)blk.addr, blk.size);
            dst += blk.size;
        } else {
            diag_cmd_brr_write(cmd, subcmd, blk.addr, blk.size, FALSE, (left == 0));
        }

        ablk_release_block(brr, &blk);
    }

    return diag_cmd;
}
#endif

