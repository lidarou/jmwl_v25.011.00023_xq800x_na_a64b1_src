
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the connect main source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f7af8
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_boot_mode.h>
#include <adbg_cfg_api.h>
#include <connmgr_api.h>
#include <connmgr_inner.h>
#include <connmgr_at_bypass.h>
#include <connmgr_cfg.h>
#include <stdio.h>

#ifndef WIN32
#include <adbg_cfg_api.h>
#include <serial_inc.h>
#endif

/* cmd listen thread define. */
#define CONN_CMD_STACK_SIZE     1024
#define CONN_CMD_THREAD_PRI     10
/* BUF size must 512 alignment */
#define CONN_ONCE_READ_SIZE     SIZE_4K

/* save the instance of connet manager. */
static conn_cmd_mgr_t *s_cmd_conn_mgr[CONN_CMD_CNT];
static conn_data_mgr_t *s_data_conn_mgr[CONN_DATA_CNT];

/* save the ppp callback function and data. */
static conn_recv_cb s_conn_ppp_cb;
static void *s_conn_ppp_cb_data;

static void connect_cmd_recv_thread(void *p)
{
    conn_cmd_dev_t *dev = (conn_cmd_dev_t *)p;
    conn_cmd_mgr_t *mgr = s_cmd_conn_mgr[dev->biz];
    conn_recv_cb cmd_cb = mgr->cmd_cb;
    AIO_DEV_T *io_dev = dev->io_dev;
    void *data = mgr->data;
    size_t size;
    char *buf;
    u8 cnt = 0;

    while (TRUE) {
        /* read data from dev. */
        buf = CONNMGR_MALLOC(CONN_ONCE_READ_SIZE);
        if (!buf) {
            if (cnt++ % 10 == 0) {
                aicx_info(0x018dd7, "", "Malloc buf failed, try later!\n");
            }

            aos_sleep(100);
            continue;
        }

        do {
            size = aio_read(io_dev, buf, CONN_ONCE_READ_SIZE, AOS_WAIT_FOREVER);
        } while(!size);

        if (size < CONN_ONCE_READ_SIZE)
            buf[size] = 0;

#ifdef CONFIG_AT_BYPASS_TO_CP
        /* Once only read one char from at connect in at mode, bypass it to cp. */
        if (size == 1 && dev->biz == CONN_CMD_AT && dev->mode == CONN_AT_MODE) {
            conn_at_by_pass_cp(dev->index, buf[0]);
            CONNMGR_FREE(buf);
            continue;
        }
#endif

        /* ppp mode, callback ppp callback. */
        if (dev->mode == CONN_PPP_MODE) {
            conn_recv_cb ppp_cb = s_conn_ppp_cb;
            void *ppp_data = s_conn_ppp_cb_data;

            if (ppp_cb)
                ppp_cb(dev->index, buf, size, ppp_data);
            else
                CONNMGR_FREE(buf);
            continue;
        }

        /* callback cmd callback. */
        if (cmd_cb)
            cmd_cb(dev->index, buf, size, data);
        else
            CONNMGR_FREE(buf);
    }
}

static conn_cmd_dev_t *connect_cmd_init_dev(u8 biz, u8 cnt, const conn_dev_t *dev_tbl)
{
    conn_cmd_dev_t *dev_info = aos_zalloc(sizeof(*dev_info) *cnt);
    conn_cmd_dev_t *dev = dev_info;
    u8 i;

    aicx_debug(0x0d6d6d, "dd", "init cmd dev, biz=%d, cnt=%d\n", biz, cnt);

    AIC_DUMMY_ASSERT(dev);

    for (i = 0; i < cnt; i++, dev_tbl++, dev++) {
        /* basic mode can't use usb. */
        if (aic_boot_mode_is_basic() && dev_tbl->dev_type == AIC_IODEV_USB)
            continue;

        /* record the dev index in the dev_tbl */
        dev->index = i;
        dev->biz = biz;
        dev->io_dev = aio_request_dev(dev_tbl->dev_type, dev_tbl->dev_com);
        AIC_DUMMY_ASSERT(dev->io_dev);
        snprintf(dev->name, sizeof(dev->name),
                 CONN_NAME_FMT, dev_tbl->dev_type, dev_tbl->dev_com);
#ifndef WIN32
        dev->thread =
            aos_create_thread_simply(dev->name, connect_cmd_recv_thread, dev,
                                     CONN_CMD_STACK_SIZE, CONN_CMD_THREAD_PRI);
        AIC_DUMMY_ASSERT(AOS_INVALID_THREAD_ID != dev->thread);

        /* load the at uart com bandrate from flash. */
        if (CONN_CMD_AT == biz && dev_tbl->dev_type == AIC_IODEV_UART) {
            int rate;
            void *handle = aio_get_dev_uart_handle(dev->io_dev);

            AIC_DUMMY_ASSERT(handle);

            rate = (int)adbg_get_at_com_bandrate();
            if (rate > 100 && rate % 10 == 0)
                uart_set_baud_rate(handle, rate);
        }
#endif
    }

    return dev_info;
}

static void connect_create_cmd_conn_mgr(void)
{
    conn_cmd_mgr_t *mgr;
    const conn_dev_cfg_t *cfg = s_cmd_conn_tbl;
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_cmd_conn_tbl); i++, cfg++) {
        if (cfg->dev_cnt == 0)
            continue;
        mgr = (conn_cmd_mgr_t *)aos_zalloc(sizeof(*mgr));
        AIC_DUMMY_ASSERT(mgr);
        s_cmd_conn_mgr[i] = mgr;
        mgr->biz = cfg->biz;
        mgr->dev_cnt = cfg->dev_cnt;
        mgr->dev_info = connect_cmd_init_dev(mgr->biz, mgr->dev_cnt, cfg->dev_tbl);
    }
}

static void connect_create_data_conn_mgr(void)
{
    conn_data_mgr_t *mgr;
    const conn_dev_cfg_t *cfg = s_data_conn_tbl;
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_data_conn_tbl); i++, cfg++) {
        mgr = (conn_data_mgr_t *)aos_zalloc(sizeof(*mgr));
        AIC_ASSERT(mgr);
        mgr->biz = cfg->biz;
        mgr->dev_cnt = cfg->dev_cnt;
        mgr->dev_tbl = cfg->dev_tbl;

        /* default use the invalid iodev */
        mgr->dev_type = AIC_INVALID_IODEV;
        mgr->dev_com = AIC_INVALID_COM;
        s_data_conn_mgr[i] = mgr;
    }
}

static u8 conn_data_get_dev_com(u8 dev_type, u8 dev_cnt, const conn_dev_t *dev_tbl)
{
    u8 i;

    for (i = 0; i < dev_cnt; i++, dev_tbl++) {
        if (dev_tbl->dev_type == dev_type)
            return dev_tbl->dev_com;
    }

    return AIC_INVALID_COM;
}

static conn_cmd_dev_t *conn_cmd_get_connid_dev(u8 biz, u32 conn_id)
{
    AIC_ASSERT(biz < CONN_CMD_CNT && s_cmd_conn_mgr[biz] &&
               conn_id < s_cmd_conn_mgr[biz]->dev_cnt);

    return (s_cmd_conn_mgr[biz]->dev_info + conn_id);
}

static void *conn_cmd_get_dev_handle(u8 biz, u32 conn_id)
{
    conn_cmd_dev_t *dev = conn_cmd_get_connid_dev(biz, conn_id);

    return aio_get_dev_uart_handle(dev->io_dev);
}

static AIO_DEV_T *conn_data_get_iodev(u8 biz)
{

    AIC_ASSERT(biz < CONN_DATA_CNT && s_data_conn_mgr[biz]);
    return (s_data_conn_mgr[biz]->io_dev);
}

static void *conn_data_get_dev_handle(u8 biz)
{
    AIO_DEV_T *io_dev = conn_data_get_iodev(biz);

    return aio_get_dev_uart_handle(io_dev);
}

int connect_init(void)
{
    aicx_debug(0x05e78e, "", "connect init\n");

    connect_create_cmd_conn_mgr();
    connect_create_data_conn_mgr();

    return AIC_SUCCESS;
}
AIC_DRV_P2_INIT_EXPORT(connect_init);

void conn_cmd_register_recv_cb(u8 biz, conn_recv_cb cb, void *p)
{
    conn_cmd_mgr_t *mgr;

    if (biz >= CONN_CMD_CNT)
        return;

    mgr = s_cmd_conn_mgr[biz];
    if (mgr) {
        mgr->cmd_cb = cb;
        mgr->data = p;
    }
}

void conn_ppp_register_recv_cb(conn_recv_cb cb, void *p)
{
    s_conn_ppp_cb = cb;
    s_conn_ppp_cb_data = p;
}

void conn_at_set_work_mode(u32 conn_id, conn_mode_t mode)
{
    conn_cmd_dev_t *dev = conn_cmd_get_connid_dev(CONN_CMD_AT, conn_id);

    aicx_debug(0x166e02, "dd", "comid=%d, set mode=%d\n", conn_id, mode);

    dev->mode = mode;
}

size_t conn_cmd_send(u8 biz, u32 conn_id, u8 *buf, size_t size, int timeout)
{
    conn_cmd_dev_t *dev = conn_cmd_get_connid_dev(biz, conn_id);

    aicx_debug(0x0eab22, "dd", "conn cmd send, biz=%d, comid=%d\n", biz, conn_id);

    if (!dev->io_dev)
        return 0;

    return aio_write(dev->io_dev, buf, size, timeout);
}

int conn_cmd_set_baud_rate(u8 biz, u32 conn_id, int rate, boolean is_need_to_save)
{
#ifndef WIN32
    void *handle;

    /* only support get at uart bandrate now. */
    if (biz != CONN_CMD_AT)
        return AIC_ENOTSUP;

    handle = conn_cmd_get_dev_handle(biz, conn_id);
    if (!handle)
        return AIC_ENODEV;

    aicx_debug(0x12cd1e, "ddd", "conn cmd set biz=%d,conn_id=%d rate=%d\n", biz, conn_id, rate);

    uart_set_baud_rate(handle, rate);

    /* save it to flash. */
    adbg_set_at_com_bandrate(rate);
    if (is_need_to_save)
        adbg_save_debug_nv_cfg(AIC_TRUE);
#endif

    return AIC_SUCCESS;
}

int conn_cmd_get_baud_rate(u8 biz, u32 conn_id)
{
    u32 val = 115200;
#ifndef WIN32
    void *handle;

    /* only support get at uart bandrate now. */
    conn_id = conn_get_uart_at_conid;
    if (AIC_ENODEV == conn_id)
        return AIC_ENODEV;

    handle = conn_cmd_get_dev_handle(biz, conn_id);
    if (!handle)
        return AIC_ENODEV;

    uart_get_baud_rate(handle, &val);

    aicx_debug(0x0a37d7, "ddd", "conn cmd get biz=%d,conn_id=%d rate=%d\n", biz, conn_id, val);

#if 0
    /* Load it from flash and check it. */
    if (biz == CONN_CMD_AT) {
        u32 rate = adbg_get_at_com_bandrate();
        aicx_info(0x0d3de0, "d", "conn cmd get at com rate=%d.\n", rate);
        if (rate > 0)
            AIC_ASSERT(val == (int)rate);
    }
#endif
#endif /* WIN32 */

    return (int)val;
}

int conn_cmd_fctrl_set(u8 biz, u32 conn_id, boolean enable)
{
#ifndef WIN32
    void *handle = conn_cmd_get_dev_handle(biz, conn_id);;

    if (!handle)
        return AIC_ENODEV;

    uart_set_fctrl(handle, enable ? 1 : 0);
#endif

    return AIC_SUCCESS;
}

int conn_cmd_fctrl_get(u8 biz, u32 conn_id, u32 *val)
{
#ifndef WIN32
    void *handle = conn_cmd_get_dev_handle(biz, conn_id);;

    if (!handle)
        return AIC_ENODEV;

    uart_get_fctrl(handle, val);
#endif

    return AIC_SUCCESS;
}

size_t conn_data_recv(u8 biz, u8 *buf, size_t size, int timeout)
{
    AIO_DEV_T *io_dev = conn_data_get_iodev(biz);

    aicx_debug(0x1b0188, "d", "conn data recv, biz=%d\n", biz);

    if (!io_dev)
        return 0;

    return aio_read(io_dev, buf, size, timeout);
}

size_t conn_data_send(u8 biz, u8 *buf, size_t size, int timeout)
{
    AIO_DEV_T *io_dev = conn_data_get_iodev(biz);

    aicx_debug(0x039901, "d", "conn data send, biz=%d\n", biz);

    if (!io_dev)
        return 0;

    return aio_write(io_dev, buf, size, timeout);
}

int conn_data_request_iodev(u8 biz, u8 iodev)
{
    conn_data_mgr_t *mgr;

    aicx_debug(0x0e7ebf, "dd", "conn data request dev, biz=%d, iodev=%d\n", biz, iodev);

    if (biz >= CONN_DATA_CNT || !s_data_conn_mgr[biz])
        return AIC_ENODEV;

    mgr = s_data_conn_mgr[biz];
    if (mgr->dev_type == iodev || AIC_IODEV_NONE == iodev)
        return AIC_SUCCESS;

    /* must none dev can requst new dev. */
    AIC_ASSERT(!mgr->io_dev);

    /* open new dev. */
    mgr->dev_type = iodev;
    mgr->dev_com = conn_data_get_dev_com(iodev, mgr->dev_cnt, mgr->dev_tbl);
    if (mgr->dev_com == AIC_INVALID_COM)
        return AIC_ENODEV;

    mgr->io_dev = aio_request_dev(mgr->dev_type, mgr->dev_com);
    AIC_DUMMY_ASSERT(mgr->io_dev);

    if (CONN_DATA_LOG == biz)
        adbg_set_log_dev_com(mgr->dev_com);
    else /* if (CONN_DATA_DIAG == biz) */
        adbg_set_diag_dev_com(mgr->dev_com);

    /* init the uart band rate. */
    if (iodev == AIC_IODEV_UART) {
        u32 rate;
        void *handle = aio_get_dev_uart_handle(mgr->io_dev);

        if (biz == CONN_DATA_LOG)
            rate = adbg_get_log_baudrate();
        else /* if (biz == CONN_DATA_DIAG) */
            rate = adbg_get_diag_baudrate();

        if (handle)
            uart_set_baud_rate(handle, (int)rate);
    }

    return AIC_SUCCESS;
}

void conn_data_release_iodev(u8 biz)
{
    aicx_debug(0x0a529e, "d", "conn data relase dev, biz=%d\n", biz);

    if (biz < CONN_DATA_CNT && s_data_conn_mgr[biz]) {
        conn_data_mgr_t *mgr = s_data_conn_mgr[biz];
        if (mgr->io_dev) {
            aio_release_dev(mgr->io_dev);
            mgr->io_dev = NULL;
            mgr->dev_type = AIC_IODEV_NONE;
        }
    }
}

int conn_data_get_dev_com_by_type(u8 biz, u8 type)
{
    const conn_dev_cfg_t *cfg = s_data_conn_tbl;
    const conn_dev_t   *dev_tbl;
    u32 i;

    for (i = 0; i < ARRAY_SIZE(s_data_conn_tbl); i++, cfg++) {
        if (cfg->biz != biz)
            continue;

        dev_tbl = cfg->dev_tbl;
        for (i = 0; i < cfg->dev_cnt; i++, dev_tbl++) {
            if (dev_tbl->dev_type == type)
                return (int)dev_tbl->dev_com;
        }
    }

    return AIC_ENODEV;
}

int conn_data_set_baud_rate(u8 biz, int rate)
{
    void *handle = conn_data_get_dev_handle(biz);

    aicx_debug(0x0511fe, "dd", "conn data set biz=%d, rate=%d\n", biz, rate);

    if (!handle)
        return AIC_ENODEV;

    uart_set_baud_rate(handle, rate);

    return AIC_SUCCESS;
}

u32 conn_get_at_conid(u8 type)
{
    int i;

    /* basic mode can't use usb. */
    if (aic_boot_mode_is_basic() && type == AIC_IODEV_USB)
        return AIC_ENODEV;

    for (i= 0; i < ARRAY_SIZE(s_at_dev_tbl); i++) {
        if (s_at_dev_tbl[i].dev_type == type)
            return i;
    }

    return AIC_ENODEV;
}

/**
 * conn_get_at_dev_type - get the at dev type by specified conid.
 *
 * @conn_id: The specified conid.
 *
 */
u8 conn_get_at_dev_type(u32 conn_id)
{
    if (conn_id < ARRAY_SIZE(s_at_dev_tbl))
        return s_at_dev_tbl[conn_id].dev_type;

    return AIC_INVALID_IODEV;
}

/**
 * conn_get_at_com_number - get the at dev com number by specified conn_id.
 *
 * @conn_id: The specified conid.
 *
 */
u8 conn_get_at_com_number(u32 conn_id)
{
    if (conn_id < ARRAY_SIZE(s_at_dev_tbl))
        return s_at_dev_tbl[conn_id].dev_com;

    return AIC_INVALID_COM;
}

