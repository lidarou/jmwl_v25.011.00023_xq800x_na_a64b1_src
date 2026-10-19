/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the AIC OSA remote message function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08df92
#define LOG_MODULE    AMOD_AOS

#include <aic_osa.h>

#ifdef CONFIG_OSA_RMTE_MSG
#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aipc_api.h>
#include <aipc_cfg.h>
#include <aos_config.h>
#include <aos_thread.h>
#include <stdio.h>

#define RMSG_SEND_NAME_FMT  "rmsg_s%d"
#define RMSG_RECV_NAME_FMT  "rmsg_r%d"
#define RMSG_MAX_NAME_LEN   8

 /* the queue cnt must be the power of 2 */
#define RMSG_QUEUE_CNT              32

#define RMSG_SEND_THR_PRI           5
#define RMSG_RECV_THR_PRI           5

#define RMSG_SEND_THR_STACK_SIZE    512
#define RMSG_RECV_THR_STACK_SIZE    768

#define RMSG_B_FREE_BIT             BIT_0
#define RMSG_B_FSM_BIT              BIT_1
#define RMSG_B_MSG2FSM_BIT          BIT_2

enum {
    RTM_MSG = 0,
    RTM_FSM,
    RTM_MSG2FSM
};

/**
 * rmsg_header struct define.
 * This struct is used to communicate with different cores,
 * Must has the same struct in all cores.
 */
#ifdef WIN32
#pragma pack(push, 1)
typedef struct rmsg_header
{
    u8  dest_id;
    u8  msg_tp;
    u16 pad;
} rmsg_header_t;

typedef struct msg2fsm_header
{
    u8  src_thread_id;
    u8  src_process_id;
    u8  dest_process_id;
    u8  pad;
    u32 prim_id;
} msg2fsm_header_t;
#pragma pack(pop)
#else
typedef struct __attribute__((packed)) rmsg_header
{
    u8  dest_id;
    u8  msg_tp;
    u16 pad;
} rmsg_header_t;

typedef struct __attribute__((packed)) msg2fsm_header
{
    u8  src_thread_id;
    u8  src_process_id;
    u8  dest_process_id;
    u8  pad;
    u32 prim_id;
} msg2fsm_header_t;
#endif /* WIN32 */

/**
 * rmsg_item struct define.
 * This struct is used to save the msg to send queue,
 */
typedef struct rmsg_item
{
    u8   flag;
    u8   dest_id;
    u16  size;
    void *msg;
    void *msg2fsm;
} rmsg_item_t;

/**
 * rmsg_inst struct define.
 * This struct is used to discribe the rmsg instance.
 */
typedef struct rmsg_inst
{
    u8                  sys;
    u8                  brr;
    u8                  link;
    u8                  rsv;

    volatile u8         q_rd;
    volatile u8         q_wt;
    aos_thread_id       send_thr;
    aos_thread_id       recv_thr;

    aos_semaphore_ptr   sema;
    ABRR_HANDLE         brr_hd;
    char                send_name[RMSG_MAX_NAME_LEN];
    char                recv_name[RMSG_MAX_NAME_LEN];
    rmsg_item_t         send_que[RMSG_QUEUE_CNT];
} rmsg_inst_t;

/**
 * rmsg_mgr struct define.
 * This struct is used to manage all the rmsg instances.
 */
typedef struct rmsg_mgr {
    u8 inst_cnt;
    rmsg_inst_t *inst_arr;
} rmsg_mgr_t;

rmsg_mgr_t s_rmsg_mgr;

/****************************************************************************
 * Static function define.
 ****************************************************************************/

static rmsg_inst_t *aos_remote_msg_get_inst(u8 sys)
{
    rmsg_inst_t *inst;
    u8 i, inst_cnt;

    inst_cnt = s_rmsg_mgr.inst_cnt;
    inst = s_rmsg_mgr.inst_arr;

    /* A sys only has a instance, get the instance of specified sys. */
    for (i = 0; i < inst_cnt; i++, inst++) {
        if (inst->sys == sys)
            return inst;
    }

    return NULL;
}

static void aos_remote_msg_send_do(rmsg_inst_t *rmsg, rmsg_item_t *item)
{
    rmsg_header_t head;
    size_t total;
    ABLK_T blk;
    u8 *buf;
    int ret;

    total = item->size + sizeof(rmsg_header_t);

    /* fill the header */
    head.dest_id = item->dest_id;
    if (item->flag & RMSG_B_MSG2FSM_BIT) {
        head.msg_tp = RTM_MSG2FSM;
        total += sizeof(msg2fsm_header_t);
    } else if (item->flag & RMSG_B_FSM_BIT) {
        head.msg_tp = RTM_FSM;
    } else {
        head.msg_tp = RTM_MSG;
    }

    /* get a blk to save msg, the whole msg must save in one blk. */
    ret = ablk_get_block(rmsg->brr_hd, total, &blk, AIPC_WAIT_FOREVER);
    AIC_ASSERT(ret == AIC_SUCCESS);
    blk.size = total;
    buf = (u8 *)blk.addr;

    /* copy rmsg header. */
    memcpy(buf, &head, sizeof(head));
    buf += sizeof(head);

    /* if has msg2fsm, copy the header. */
    if (item->msg2fsm) {
        msg2fsm_header_t *msg2fsm = item->msg2fsm;
        memcpy(buf, msg2fsm, sizeof(*msg2fsm));
        buf += sizeof(*msg2fsm);
        aos_free(msg2fsm);
    }

    /* copy rmsg body. */
    if (head.msg_tp == RTM_FSM) {
#ifdef CONFIG_FSMMSG_FEATURE
        aos_fsm_msg *fsm;

        /* fsmmsg */
        fsm = (aos_fsm_msg *)item->msg;
        memcpy(buf, fsm, sizeof(*fsm));
        buf += sizeof(*fsm);

        /* fsmmsg parm */
        if (FSM_PARAM_SIZE(fsm)) {
            memcpy(buf, FSM_PARAM_PTR(fsm), FSM_PARAM_SIZE(fsm));
            buf += FSM_PARAM_SIZE(fsm);
        }

        /* fsmmsg peer */
        if (FSM_PEER_BUF_SIZE(fsm)) {
            memcpy(buf, FSM_PEER_BUF_PTR(fsm), FSM_PEER_BUF_SIZE(fsm));
            buf += FSM_PEER_BUF_SIZE(fsm);
        }

        if (item->flag & RMSG_B_FREE_BIT)
            aos_free_fsm_msg(fsm);
#else
        AIC_ASSERT(0);
#endif
    } else {
        memcpy(buf, item->msg, item->size);
        /* if need free, free it. */
        if (item->flag & RMSG_B_FREE_BIT)
            aos_free(item->msg);
    }

    /* send the blk to remote. */
    ret = ablk_send_block(rmsg->brr_hd, rmsg->link, &blk, AIPC_WAIT_FOREVER);
    AIC_ASSERT(ret == AIC_SUCCESS);
}

static void aos_remote_msg_recv_do(rmsg_inst_t *rmsg, ABLK_T *blk)
{
#ifdef CONFIG_FSMMSG_FEATURE
    aos_fsm_msg *fsm_src, *fsm_dst;
    u32 ret;
#endif
    u32 msg_size = blk->size;
    rmsg_header_t *head;
    u8 *buf = blk->addr;

    /* get the rmsg header */
    head = (rmsg_header_t *)buf;
    buf += sizeof(*head);
    msg_size -= sizeof(*head);

    if (head->msg_tp == RTM_MSG) {
        /* get the msg. */
        aos_msg_ptr msg = (aos_msg_ptr) aos_malloc(msg_size);
        AIC_DUMMY_ASSERT(msg);
        memcpy(msg, buf, msg_size);

        /* send fsm msg to thread. */
        aos_send_msg(head->dest_id, (aos_msg_ptr *)msg, AOS_WAIT_FOREVER);
        return;
    }

    /* if has msg2fsm, get the header. */
    if (head->msg_tp == RTM_MSG2FSM) {
        msg2fsm_header_t *msg2fsm_h = (msg2fsm_header_t *)buf;

#ifndef CONFIG_FSMMSG_FEATURE
        msg2fsm_t *msg2fsm;
        u32 total_size;

        AIC_ASSERT(msg_size >= sizeof(*msg2fsm_h));
        total_size = (msg_size - sizeof(*msg2fsm_h) + sizeof(*msg2fsm));

        /* alloc the msg. */
        msg2fsm = (msg2fsm_t *) aos_malloc(total_size);
        AIC_DUMMY_ASSERT(msg2fsm);

        /* copy the msg2fsm. */
        FSM_DEST_THREAD_ID(msg2fsm) = head->dest_id;
        FSM_SRC_THREAD_ID(msg2fsm) = msg2fsm_h->src_thread_id;
        FSM_SRC_PROCESS_ID(msg2fsm) = msg2fsm_h->src_process_id;
        FSM_DEST_PROCESS_ID(msg2fsm) = msg2fsm_h->dest_process_id;
        FSM_PRIMITIVE_ID(msg2fsm) = msg2fsm_h->prim_id;

        buf += sizeof(*msg2fsm_h);
        msg_size -= sizeof(*msg2fsm_h);

        /* copy msg */
        msg2fsm->msg = (aos_msg_ptr)(msg2fsm + 1);
        msg2fsm->msg_size = msg_size;

        memcpy(msg2fsm->msg, buf, msg_size);

        /* send fsm msg to thread. */
        aos_send_msg(head->dest_id, (aos_msg_ptr *)msg2fsm, AOS_WAIT_FOREVER);
        return;
#else
        buf += sizeof(*msg2fsm_h);
        msg_size -= sizeof(*msg2fsm_h);
        fsm_dst = aos_get_fsm_msg(msg_size, 0);
        AIC_DUMMY_ASSERT(fsm_dst);
        aos_init_fsm_msg_header(fsm_dst, 0);
        FSM_PARAM_SIZE(fsm_dst) = msg_size;
        FSM_DEST_THREAD_ID(fsm_dst) = head->dest_id;
        FSM_SRC_THREAD_ID(fsm_dst) = msg2fsm_h->src_thread_id;
        FSM_SRC_PROCESS_ID(fsm_dst) = msg2fsm_h->src_process_id;
        FSM_DEST_PROCESS_ID(fsm_dst) = msg2fsm_h->dest_process_id;
        FSM_PRIMITIVE_ID(fsm_dst) = msg2fsm_h->prim_id;

        if (FSM_PARAM_SIZE(fsm_dst)) {
            memcpy(FSM_PARAM_PTR(fsm_dst), buf, FSM_PARAM_SIZE(fsm_dst));
            buf += FSM_PARAM_SIZE(fsm_dst);
        }
#endif
    } else {
#ifdef CONFIG_FSMMSG_FEATURE
        /* head.msg_tp == RTM_FSM */
        fsm_src = (aos_fsm_msg *)buf;
        fsm_dst = aos_get_fsm_msg(FSM_PARAM_SIZE(fsm_src),
                                  FSM_PEER_BUF_SIZE(fsm_src));
        AIC_DUMMY_ASSERT(fsm_dst);

        /* copy fsmmsg */
        aos_copy_fsm_msg_header(fsm_dst, fsm_src);
        buf += sizeof(*fsm_dst);

        /* copy fsmmsg param */
        if (FSM_PARAM_SIZE(fsm_dst)) {
            memcpy(FSM_PARAM_PTR(fsm_dst), buf, FSM_PARAM_SIZE(fsm_dst));
            buf += FSM_PARAM_SIZE(fsm_dst);
        }

        /* copy fsmmsg peer */
        if (FSM_PEER_BUF_SIZE(fsm_dst)) {
            memcpy(FSM_PEER_BUF_PTR(fsm_dst), buf, FSM_PEER_BUF_SIZE(fsm_dst));
            buf += FSM_PEER_BUF_SIZE(fsm_dst);
        }
#endif
    }

#ifdef CONFIG_FSMMSG_FEATURE
    /* send fsm msg to thread, and no trace it. */
    ret = aos_send_fsm_msg_debug(head->dest_id, fsm_dst, AOS_WAIT_FOREVER, FALSE);
    AIC_ASSERT(ret == AIC_SUCCESS);
#endif
}

/*
 * send msg thread, procc the send queue.
 */
static void aos_remote_msg_send_thr(void *p)
{
    rmsg_inst_t *rmsg = p;
    AOS_INTERRUPT_DECLARE
    rmsg_item_t *item;
    u8 index;

    while (TRUE) {
        (void)aos_get_semaphore(rmsg->sema, AOS_WAIT_FOREVER);

        AOS_INTERRUPT_DISABLE

        /* check the send queue */
        while (IS_RING_NOT_EMPTY(rmsg->q_rd, rmsg->q_wt)) {
            /* get it */
            index = GET_RING_INDEX(RMSG_QUEUE_CNT, rmsg->q_rd);
            item = &rmsg->send_que[index];

            /* send it */
            AOS_INTERRUPT_RESTORE
            aos_remote_msg_send_do(rmsg, item);
            AOS_INTERRUPT_DISABLE

            /* update read ptr. */
            rmsg->q_rd++;
        }

        AOS_INTERRUPT_RESTORE
    }
}

/*
 * recv msg thread, recv it and send to loacal task.
 */
void aos_remote_msg_recv_thr(void *p)
{
    rmsg_inst_t *rmsg = p;
    ABLK_T blk;
    int ret;

    while (TRUE) {
        /* one blk has and only has a whole msg. */
        ret = ablk_recv_block(rmsg->brr_hd, rmsg->link, &blk, AIPC_WAIT_FOREVER);
        if (ret != AIC_SUCCESS)
            continue;

        /* send to local task and relase it. */
        aos_remote_msg_recv_do(rmsg, &blk);
        ablk_release_block(rmsg->brr_hd, &blk);
    }
}

/****************************************************************************
 * extern function define.
 ****************************************************************************/
int aos_send_msg_to_rmte_ex(u8 sys, u8 dest_id,void *msg, u16 msg_size,
                            msg2fsm_header_t *msg2fsm, bool b_fsm, bool b_free)
{
    AOS_INTERRUPT_DECLARE
    rmsg_item_t *item;
    rmsg_inst_t *rmsg;
    u8 index, flag = 0;

    /* Can't get the rmsg instance of specified sys, return no device error. */
    rmsg = aos_remote_msg_get_inst(sys);
    if (!rmsg)
        return AIC_ENODEV;

    AOS_INTERRUPT_DISABLE

    /* If send queue full, return busy error. */
    if (IS_RING_FULL(RMSG_QUEUE_CNT, rmsg->q_rd, rmsg->q_wt)) {
        aicx_warn(0x109df4, "ddd", "send msg to rmte, sys=%d, dest_id=%d, b_fsm=%d.\n",
                  sys, dest_id, b_fsm);
        AOS_INTERRUPT_RESTORE
        //AIPC_ASSERT(0);
        return AIC_EBUSY;
    }

    /* get a item to save the msg. */
    index = GET_RING_INDEX(RMSG_QUEUE_CNT, rmsg->q_wt);
    item = &rmsg->send_que[index];

    if (b_free)
        flag |= RMSG_B_FREE_BIT;

    if (b_fsm)
        flag |= RMSG_B_FSM_BIT;
    else if (msg2fsm)
        flag |= RMSG_B_MSG2FSM_BIT;

    item->flag = flag;
    item->dest_id = dest_id;
    item->msg = msg;
    item->msg2fsm = msg2fsm;
    item->size = msg_size;

    /* update write ptr */
    rmsg->q_wt++;

    AOS_INTERRUPT_RESTORE

    aos_put_semaphore(rmsg->sema);

    return AIC_SUCCESS;
}

int aos_send_msg2fsm_to_rmte(u8 sys, msg2fsm_ptr_t msg, bool b_free)
{
    msg2fsm_header_t *msg2fsm = aos_malloc(sizeof(*msg2fsm));

    AIC_DUMMY_ASSERT(msg2fsm);

    msg2fsm->src_thread_id = msg->src_thread_id;
    msg2fsm->src_process_id = msg->src_process_id;
    msg2fsm->dest_process_id = msg->dest_process_id;
    msg2fsm->prim_id = msg->prim_id;

    return aos_send_msg_to_rmte_ex(sys, msg->dest_thread_id, msg->msg,
                                   msg->msg_size, msg2fsm, FALSE, b_free);
}

int aos_send_msg_to_remote(u8 sys, aos_thread_id dest_id,
                           aos_msg_ptr msg, u16 msg_size, bool b_free)
{
    return aos_send_msg_to_rmte_ex(sys, dest_id, msg, msg_size,
                                   NULL, FALSE, b_free);
}

#ifdef CONFIG_FSMMSG_FEATURE
int aos_send_fsm_msg_to_remote(u8 sys, aos_fsm_msg *msg, bool b_free)
{
    return aos_send_msg_to_rmte_ex(sys, msg->dest_thread_id, msg,
                                   aos_fsm_msg_total_size(msg),
                                   NULL, TRUE, b_free);
}
#endif

int aos_remote_msg_init(void)
{
    const RMSG_CFG_T *cfg;
    rmsg_inst_t *rmsg;
    u8 cnt, i;

    /* Get the rmsg cfg data. */
    cfg = aipc_get_rmsg_cfg(&cnt);
    s_rmsg_mgr.inst_cnt = cnt;

    rmsg = aos_zalloc(sizeof(*rmsg) * cnt);
    AIC_DUMMY_ASSERT(rmsg);
    s_rmsg_mgr.inst_arr = rmsg;

    /* Create all rmsg instance. */
    for (i = 0; i < cnt; i++, rmsg++, cfg++) {
        rmsg->brr_hd = ablk_request_brr(cfg->sys, cfg->brr_id);
        AIC_ASSERT(rmsg->brr_hd);

        rmsg->sys = cfg->sys;
        rmsg->brr = cfg->brr_id;
        rmsg->link = cfg->link_id;

        snprintf(rmsg->send_name, sizeof(rmsg->send_name), RMSG_SEND_NAME_FMT, cfg->sys);
        snprintf(rmsg->recv_name, sizeof(rmsg->recv_name), RMSG_RECV_NAME_FMT, cfg->sys);

        rmsg->sema = aos_create_semaphore(rmsg->send_name, 0);
        AIC_DUMMY_ASSERT(rmsg->sema);

        rmsg->send_thr =
            aos_create_thread_simply(rmsg->send_name, aos_remote_msg_send_thr,
                                     (void*)rmsg, RMSG_SEND_THR_STACK_SIZE,
                                     RMSG_SEND_THR_PRI);
        AIC_DUMMY_ASSERT(rmsg->send_thr);

        rmsg->recv_thr =
            aos_create_thread_simply(rmsg->recv_name, aos_remote_msg_recv_thr,
                                     (void*)rmsg, RMSG_RECV_THR_STACK_SIZE,
                                     RMSG_RECV_THR_PRI);

        AIC_DUMMY_ASSERT(rmsg->recv_thr);
    }

    return 0;
}
AIC_COMP_P2_INIT_EXPORT(aos_remote_msg_init);

void aos_remote_msg_deinit(void)
{
    rmsg_inst_t *rmsg;
    u8 cnt, i;

    cnt  = s_rmsg_mgr.inst_cnt;
    rmsg = s_rmsg_mgr.inst_arr;

    for (i = 0; i < cnt; i++, rmsg++) {
        aos_delete_thread(rmsg->send_thr);
        aos_delete_thread(rmsg->recv_thr);
        aos_delete_semaphore(rmsg->sema);
        ablk_release_brr(rmsg->brr_hd);
    }

    aos_free(s_rmsg_mgr.inst_arr);

    s_rmsg_mgr.inst_cnt = 0;
    s_rmsg_mgr.inst_arr = NULL;
}

#endif /* CONFIG_OSA_RMTE_MSG */

