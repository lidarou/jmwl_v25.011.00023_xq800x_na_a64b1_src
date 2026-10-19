/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The alog outputer source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x109771
#define LOG_MODULE    AMOD_ALOG

#include <alog_inner.h>
#include <alog_frame.h>
#include <alog_outputer.h>
#include <alog_store_api.h>
#include <adbg_cfg_api.h>
#include <serial_inc.h>
#include <stdio.h>

/* if not open CONFIG_ALOG_FRAME_SUPPORT, msg must use a independent com. */
#ifndef CONFIG_ALOG_FRAME_SUPPORT
#ifdef CONFIG_ALOG_MSG_SUPPORT
#define ALOG_MSG_USE_A_COM
#endif
#endif

#define ALOP_OUTPUT_NAME "lgop"
#define ALOP_INPUTER_MARK \
    (BIT(ALTRACE_LINK) | BIT(ALMSG_LINK) | BIT(ALDATA_LINK))

#define ALOG_INPUTER_TASK_PRI    2

#ifdef WIN32
#define ALOG_MSG_USE_A_COM
#define ALOG_OUPUT_IN_WIN32(b, l) do { \
        if (link_id == ALTRACE_LINK) { \
            size_t s; \
            AOS_INTERRUPT_DECLARE \
     \
            AOS_INTERRUPT_DISABLE \
     \
            for (s = 0; s < (l); s++) \
                putchar(*((b) + s)); \
     \
            AOS_INTERRUPT_RESTORE \
        } \
    } while (0)
#else
#define ALOG_OUPUT_IN_WIN32(b, l) do {} while (0)
#endif

static void alop_output_block(ALOP_T *alop, u8 brr, u8 link_id, ABLK_T *blk);

#ifdef CONFIG_CONNECT_MGR
typedef struct blk_list_item {
    u8      brr;
    u8      link_id;
    ABLK_T  blk;
    struct list_head    next;
} blk_list_item_t;

#define LOG_READY   "log ready"
bool volatile s_log_ready;

/* read data from usb, size must align 512. */
#define USB_LOG_BUF_SIZE    512
#define ALOP_WAIT_PC_STACK  1024

#ifdef CONFIG_ALOG_BUF_BLOCK
#define MAX_BUFFER_CNT      20

static void alog_output_flush_buffer(ALOP_T *alop, bool output)
{
    struct list_head *head = &alop->blk_list;
    blk_list_item_t *pos, *temp;
    int ret;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    if (alop->list_cnt == 0 || alop->list_mutex == NULL)
    {
        AOS_INTERRUPT_RESTORE
        return;
    }
    alop->list_cnt = 0;
    AOS_INTERRUPT_RESTORE

    ret = aos_get_mutex(alop->list_mutex, AOS_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

    /* free all log and release list */
    list_for_each_entry_safe(pos, temp, head, next, blk_list_item_t) {
        list_del(&pos->next);

        if (output)
            alop_output_block(alop, pos->brr, pos->link_id, &pos->blk);

        aos_free(pos);
    }

    if (ret == AIC_SUCCESS)
        aos_put_mutex(alop->list_mutex);
}

void alop_output_buffer_blk(ALOP_T *alop, u8 brr, u8 link_id, ABLK_T *blk)
{
    blk_list_item_t *item;
    int ret;

    if (!alop->list_mutex)
        return;

    ret = aos_get_mutex(alop->list_mutex, AOS_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

    if (alop->list_cnt < MAX_BUFFER_CNT) {
        item = (blk_list_item_t *)aos_malloc_relaxed(sizeof(*item) + blk->size);
        if (item) {
            AOS_INTERRUPT_DECLARE
            item->brr = brr;
            item->link_id = link_id;
            item->blk.addr = (void *)(item + 1);
            memcpy(item->blk.addr, blk->addr, blk->size);
            item->blk.size = blk->size;

            list_add_tail(&item->next, &alop->blk_list);
            AOS_INTERRUPT_DISABLE
            alop->list_cnt++;
            AOS_INTERRUPT_RESTORE
        }
    }

    if (ret == AIC_SUCCESS)
        aos_put_mutex(alop->list_mutex);
}
#endif

static void alop_destroy_wait_res(ALOP_T *alop, bool thread)
{
    aos_thread_id thread_id = AOS_INVALID_THREAD_ID;
    aos_timer_ptr timer_ptr = NULL;
#ifdef CONFIG_ALOG_BUF_BLOCK
    aos_mutex_ptr mutex_ptr = NULL;
#endif
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    /* delete the wait pc ready thead. */
    if (thread && alop->thread != AOS_INVALID_THREAD_ID) {
        thread_id = alop->thread;
        alop->thread = AOS_INVALID_THREAD_ID;
    }

    if (alop->del_timer) {
        timer_ptr = alop->del_timer;
        alop->del_timer = NULL;
    }

#ifdef CONFIG_ALOG_BUF_BLOCK
    if (alop->list_mutex) {
        mutex_ptr = alop->list_mutex;
        alop->list_mutex = NULL;
    }
#endif

    AOS_INTERRUPT_RESTORE

    /* delete the wait pc ready thead. */
    if (thread_id != AOS_INVALID_THREAD_ID)
        aos_delete_thread(thread_id);

    if (timer_ptr)
        aos_delete_timer(timer_ptr);

#ifdef CONFIG_ALOG_BUF_BLOCK
    if (mutex_ptr)
        aos_delete_mutex(mutex_ptr);
#endif
}

static void alop_del_thread_timer(void *p)
{
    ALOP_T *alop = (ALOP_T *)p;
    aos_thread_id thread_id = AOS_INVALID_THREAD_ID;

    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE
    s_log_ready = TRUE;
    if (alop->thread != AOS_INVALID_THREAD_ID) {
        alop->thread = AOS_INVALID_THREAD_ID;
        thread_id = alop->thread;
    }
    AOS_INTERRUPT_RESTORE

#ifdef CONFIG_ALOG_BUF_BLOCK
    alog_output_flush_buffer(alop, FALSE);
#endif

    /* delete the wait pc ready thead. */
    if (thread_id != AOS_INVALID_THREAD_ID)
        aos_delete_thread(thread_id);
}

static void alop_wait_log_ready_cmd(void *p)
{
    char *buf;
    bool ready = FALSE;
    size_t size;
    u8 dev;
    u8 wait_cnt = 90;
    ALOP_T *alop = (ALOP_T *)p;
    AOS_INTERRUPT_DECLARE

    buf = CONNMGR_MALLOC(USB_LOG_BUF_SIZE);
    AIC_DUMMY_ASSERT (buf);

#ifdef CONFIG_ALOG_BUF_BLOCK
    alop->list_mutex = aos_create_mutex("lgbl");
    AIC_DUMMY_ASSERT(alop->list_mutex);
#endif

    alop->del_timer = aos_create_timer("lgbl", alop_del_thread_timer,
                                       alop, 10 * 1000,
                                       AOS_AUTO_ACTIVATE, AOS_NO_PERIOD);
    AIC_DUMMY_ASSERT(alop->del_timer);

    aicx_debug(0x00c9bc, "", "wait log ready from usb.\n");

    do {
        size = conn_data_recv(CONN_DATA_LOG, buf, USB_LOG_BUF_SIZE, 100);

        /* logdev may switch: usb ==> uart. */
        dev = adbg_get_log_dev();
        if (AIC_IODEV_USB != dev) {
            if (dev != AIC_IODEV_NONE)
                ready = TRUE;
            break;
        }

        wait_cnt--;
        if (size == 0) {
            aos_sleep(100);
            continue;
        }

        if (size >= USB_LOG_BUF_SIZE)
            size = USB_LOG_BUF_SIZE - 1;

        buf[size] = 0;
        if (strstr(buf, LOG_READY)) {
            ready = TRUE;
            break;
        }
    } while (wait_cnt && !alop->b_stop);

    CONNMGR_FREE(buf);

    AOS_INTERRUPT_DISABLE
#ifdef CONFIG_ALOG_BUF_BLOCK
    if (alop->list_cnt) {
        AOS_INTERRUPT_RESTORE
        alog_output_flush_buffer(alop, ready);
        AOS_INTERRUPT_DISABLE
    }
#endif

    s_log_ready = TRUE;
    AOS_INTERRUPT_RESTORE

    alop_destroy_wait_res(alop, FALSE);

    aicx_debug(0x0191f0, "d", "exit wait log ready=%d.\n", ready);
}
#endif /* CONFIG_CONNECT_MGR */

static void alop_output_block(ALOP_T *alop, u8 brr, u8 link_id, ABLK_T *blk)
{
    size_t left = blk->size, size, once;
#ifndef CONFIG_CONNECT_MGR
    AIO_DEV_T *iodev = alop->io_dev;
#endif
    u8 *buf = blk->addr;
    int ret;

    /* ALOG_TRACE_UART_DIRECT open, don't output trace again. */
#if IS_CONF_ENABLED(ALOG_TRACE_UART_DIRECT)
    if (link_id == ALTRACE_LINK)
        return;
#endif

#ifdef CONFIG_ALOG_FRAME_SUPPORT
    /* skip the frame header. */
    ALOG_OUPUT_IN_WIN32(buf + ALOG_FRAME_H_SIZE, left - ALOG_FRAME_H_SIZE);
#else
    ALOG_OUPUT_IN_WIN32(buf, left);
#endif

#ifdef ALOG_MSG_USE_A_COM
    /* ALMSG_LINK use msg_dev and msg_com. */
    if (link_id == ALMSG_LINK)
        iodev = alop->msg_dev;
#endif

    /*
     * get the mutex.
     * The latter block must wait for the previous block to complete output.
     */

    ret = aos_get_mutex(alop->mutex, AOS_WAIT_FOREVER);
    AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

#ifdef CONFIG_ALOG_STORE_FS
    /* WIN32, need AIC_IODEV_FILE to save log. */
    if (AIC_IODEV_FILE == alop->dev_type) {
        alog_store_save_log(buf, left);
        /* put the mutex. */
        if (ret == AIC_SUCCESS)
            aos_put_mutex(alop->mutex);
        return;
    }
#endif

    size = 0;
    while (!alop->b_stop && left) {
#ifdef CONFIG_CONNECT_MGR
        once = conn_data_send(CONN_DATA_LOG, buf + size, left, AOS_WAIT_FOREVER);
#else
        once = aio_write(iodev, buf + size, left, AOS_WAIT_FOREVER);
#endif
        AIC_DUMMY_ASSERT(once <= left);

        if (once == 0 || once > left) {
            aicx_debug(0x1964f4, "", "write failed, try later\n.");
            /* don't try again. */
            break;
        }

        size += once;
        left -= once;
    }

    /* put the mutex. */
    if (ret == AIC_SUCCESS)
        aos_put_mutex(alop->mutex);
}

void alop_output_block_in_assert(ALOP_T *alop, u8 link_id, ABLK_T *blk)
{
    size_t left = blk->size, size, once;
    u8 *buf = blk->addr;

#ifdef CONFIG_ALOG_FRAME_SUPPORT
    /* skip the frame header. */
    ALOG_OUPUT_IN_WIN32(buf + ALOG_FRAME_H_SIZE, left - ALOG_FRAME_H_SIZE);
#else
    ALOG_OUPUT_IN_WIN32(buf, left);

    /* if not enable alog frame,filter the message log. */
#ifdef ALOG_MSG_USE_A_COM
    if (link_id == ALMSG_LINK)
        return;
#endif

#endif

    size = 0;
    while (left) {
        /* only uart can output data in assert. */
        #ifdef WIN32
        if (alop->dev_type != AIC_IODEV_NONE)
        #else
        if (alop->dev_type == AIC_IODEV_UART
            #ifdef CONFIG_CMUX
            || alop->dev_type == AIC_IODEV_CMUX
            #endif
            )
        #endif
        {
#ifdef CONFIG_CONNECT_MGR
            once = conn_data_send(CONN_DATA_LOG, buf + size, left, AOS_NO_WAIT);
#else
            once = aio_write(alop->io_dev, buf + size, left, AOS_NO_WAIT);
#endif
        } else {
            once = left;
        }

        if  (!once)
            break;

        size += once;
        left -= once;
    }
}

static void alop_inputer_handler(u8 sys, u8 brr, u8 link_id, ABLK_T *blk, void *p)
{
    ALOP_T *alop = p;

    aicx_debug(0x12e018, "", "inouter handle!\n");

    /* none, drop the block that's still in transmission. */
    if (alop->b_stop || alop->dev_type == AIC_IODEV_NONE)
        return;

#ifdef CONFIG_CONNECT_MGR
    /* check pc tool is ready receive log to avoid lost ap log. */
    if (alop->thread != AOS_INVALID_THREAD_ID
        && sys == AIC_SYS_AP) {
#ifdef CONFIG_ALOG_BUF_BLOCK
        AOS_INTERRUPT_DECLARE

        AOS_INTERRUPT_DISABLE
        if (!s_log_ready) {
            AOS_INTERRUPT_RESTORE
            alop_output_buffer_blk(alop, brr, link_id, blk);
            return;
        }
        AOS_INTERRUPT_RESTORE
#else
        while (!s_log_ready)
            aos_sleep(100);
#endif
        alop_destroy_wait_res(alop, TRUE);
    }
#endif

    /* put the block, may be bocked in it. */
    alop_output_block(alop, brr, link_id, blk);
}

ALOP_T *alop_create(u8 dev_type, u8 com)
{
    ALOP_T *alop;

    /* alloc alop. */
    alop = (ALOP_T *)aos_zalloc(sizeof(*alop));
    AIC_DUMMY_ASSERT(alop);

    /* init outputer_dev. */
    alop->dev_type = dev_type;
    alop->dev_com = com;

    aicx_debug(0x1cb95d, "dd", "create alop type=%d, com=%d!\n", dev_type, alop->dev_com);

    if (dev_type != AIC_IODEV_NONE
#ifndef WIN32
        /* WIN32, need AIC_IODEV_FILE to save log. */
        && dev_type != AIC_IODEV_FILE
#endif
    ) {
#ifdef CONFIG_CONNECT_MGR
        if (AIC_SUCCESS != conn_data_request_iodev(CONN_DATA_LOG, dev_type)) {
            aos_free(alop);
            return NULL;
        }

        aicx_debug(0x0361ea, "d", "### if s_log_ready=%d.\n", s_log_ready);
        if (!s_log_ready &&
            AIC_IODEV_USB == adbg_get_log_dev()) {
#ifdef CONFIG_ALOG_BUF_BLOCK
            list_head_init(&alop->blk_list);
#endif
            alop->thread =
            aos_create_thread_simply("w-pc", alop_wait_log_ready_cmd, alop,
                                     ALOP_WAIT_PC_STACK, ALOG_INPUTER_TASK_PRI);
        } else {
            alop->thread = AOS_INVALID_THREAD_ID;
        }
#else
        alop->io_dev = aio_request_dev(alop->dev_type, alop->dev_com);
        AIC_DUMMY_ASSERT(alop->io_dev);
        if (alop->dev_type == AIC_IODEV_UART) {
            u32 rate = adbg_get_log_baudrate();
            void *handle = aio_get_dev_uart_handle(alop->io_dev);

            AIC_DUMMY_ASSERT(handle);
            uart_set_baud_rate(handle, (int)rate);
        }

#ifdef ALOG_MSG_USE_A_COM
        /* use trace com + 1 as message com. */
        alop->msg_dev = aio_request_dev(alop->dev_type, com + 1);
        AIC_DUMMY_ASSERT(alop->msg_dev);
#endif /* ALOG_MSG_USE_A_COM */
#endif /* CONFIG_CONNECT_MGR */
    }

    /* create mutex. */
    alop->mutex = aos_create_mutex(ALOP_OUTPUT_NAME);
    AIC_DUMMY_ASSERT(alop->mutex);

    return alop;
}

void alop_create_inputer(ALOP_T *alop, u8 cnt, const ALBRR_CFG_T *brr_cfg)
{
    int i;

    /* alloc inputer array, and create inputer. */
    alop->inputer_cnt = cnt;
    alop->inputer = aos_malloc(sizeof(*alop->inputer) * alop->inputer_cnt);
    for (i = 0; i < alop->inputer_cnt; i++, brr_cfg++) {
        alop->inputer[i] =
            abrr_inputer_create(brr_cfg->sys, brr_cfg->brr, ALOP_INPUTER_MARK,
                                0, 0,
                                ALOG_INPUTER_TASK_PRI, alop_inputer_handler,
                                alop);
        abrr_inputer_start(alop->inputer[i]);
    }
}

void alop_destroy(ALOP_T *alop)
{
    u32 i;

    alop->b_stop = 1;

    /* first, close iodev. */
#ifdef CONFIG_CONNECT_MGR
    conn_data_release_iodev(CONN_DATA_LOG);

#ifdef CONFIG_ALOG_BUF_BLOCK
    alog_output_flush_buffer(alop, FALSE);
#endif

    alop_destroy_wait_res(alop, TRUE);
#else
    /* release io_dev */
    if (alop->io_dev)
        aio_release_dev(alop->io_dev);

#ifdef ALOG_MSG_USE_A_COM
    /* release msg_dev */
    if (alop->msg_dev)
        aio_release_dev(alop->msg_dev);
#endif /* ALOG_MSG_USE_A_COM */
#endif /* CONFIG_CONNECT_MGR */

    /* wait for output threa return from iodev write. */
    aos_sleep(1000);

    /* Destroy inputer. */
    for (i = 0; i < alop->inputer_cnt; i++)
        abrr_inputer_destroy(alop->inputer[i]);

    /* destroy mutex. */
    aos_delete_mutex(alop->mutex);

    /* free other instance. */
    aos_free(alop->inputer);
    aos_free(alop);
}

void alop_output_flush_in_assert(ALOP_T *alop)
{
    ABRR_INPUTER_T *inputer;
    u8 link_id;
    ABLK_T blk;
    u32 links;
    int ret;
    u8 i, link_cnt;

    for (i = 0; i < alop->inputer_cnt; i++) {
        inputer = alop->inputer[i];
        if (!inputer || !inputer->brr)
            continue;

        /* listen all link_sets link */
        links = ablk_select(inputer->brr,
                            inputer->link_mark, AIPC_NO_WAIT);
        link_cnt = ablk_get_link_cnt(inputer->brr, TRUE);

        for (link_id = 0; link_id < link_cnt; link_id++) {
            /* no data, next link */
            if (!(links & BIT(link_id)))
                continue;

            do {
                ret = ablk_recv_block_any(inputer->brr, link_id, &blk, FALSE);
                if (ret != AIC_SUCCESS)
                    break;

                alop_output_block_in_assert(alop, link_id, &blk);
                ablk_release_block_any(inputer->brr, &blk, FALSE);
            } while (ret == AIC_SUCCESS);
        }
    }
}

