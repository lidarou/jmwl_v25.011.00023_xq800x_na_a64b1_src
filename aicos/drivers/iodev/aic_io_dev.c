/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the AIC input/output device data and api.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x15aa8a
#define LOG_MODULE    AMOD_DRV

#include <aic_boot_mode.h>
#include <aic_bitmap.h>
#include <aic_io_dev.h>
#include <aic_osa.h>
#include <drv_file.h>
#include <stdio.h>

#ifdef CONFIG_CMUX
#include <cmux.h>
#include <cmux_cfg.h>
#endif

#ifdef WIN32
#include "drv_usart.h"
#else
#include <serial_inc.h>
#endif

#define AIC_IO_DEBUG    0

#if AIC_IO_DEBUG
#undef alog_debug
#undef aidx_debug

#define alog_debug  alog_info
#define aidx_debug  aidx_info
#endif

#ifdef CONFIG_CASE_NUMBER_FILE_NAME
extern u32 g_lte_case_number;
extern u32 g_lte_case_sub_number;
#endif

typedef struct _AIO_DEV _AIO_DEV_T;

/**
 * input output com device type
 */
typedef struct AIO_COM_DEV {
    u8          com;
    u8          ref_cnt;
    void        *com_handle;
    _AIO_DEV_T  *io_dev;
} AIO_COM_DEV_T;

/**
 * input output device type
 */
struct _AIO_DEV {
    u8     ref_cnt;
    u8     iodev;
    AIO_COM_DEV_T   *com_dev[AIC_COM_CNT];
    void*   (*open)(int com);
    void    (*close)(void *handle);
    size_t  (*read)(void *handle, void *buffer, size_t size, int timeout);
    size_t  (*write)(void *handle,  const void *buffer, size_t size, int timeout);
};

static _AIO_DEV_T *s_aio_dev[AIC_IODEV_CNT];

#ifdef WIN32
#include <io.h>
#define LOG_DIR_1TH     "../../../../out/win32/Debug/"
#define LOG_DIR_2TH     "log/"
#define LOG_FILE        "aic_log"
#else
#define LOG_DIR_1TH     "/aic_log/"
#define LOG_DIR_2TH     "/log/"
#define LOG_FILE        "aic_log"
#endif

#ifdef CONFIG_CDC_ACM_INSTANCE
#include <cdc_acm.h>

enum {
    CDC_ACM_VCOM_DEACTIVIED = 0,
    CDC_ACM_VCOM_ACTIVIED
};

#define CDC_ACM_NAME_FMT    "usb-%d"
#define CDC_ACM_NAME_LEN    6

typedef struct cdc_acm_vcom {
    u8                  com;
    u8                  vcom_state;
    char                name[CDC_ACM_NAME_LEN];
    aos_event_ptr       event_ptr;
    void                *cdc_handle;
} cdc_acm_vcom_t;

static int aic_io_cdc_acm_cb(uint32_t com, enum vcom_event ev)
{
    _AIO_DEV_T *_dev = s_aio_dev[AIC_IODEV_USB];
    AIO_COM_DEV_T *com_dev = _dev->com_dev[com];
    cdc_acm_vcom_t *acm_vcom;
    u32 actual_flags;

    AIC_DUMMY_ASSERT(com_dev);
    acm_vcom = com_dev->com_handle;

    if (ev == VCOM_ACTIVE) {
        aicx_debug(0x0cdff2, "d", "USB VOM(%d) active!\n", com);
        acm_vcom->vcom_state = CDC_ACM_VCOM_ACTIVIED;
        aos_set_event(acm_vcom->event_ptr, BIT(CDC_ACM_VCOM_ACTIVIED), AOS_OR);
    } else {
        aicx_debug(0x0f86e2, "d", "USB VOM(%d) deactive!\n", com);
        acm_vcom->vcom_state = CDC_ACM_VCOM_DEACTIVIED;
        /* clear the event flag. */
        aos_get_event(acm_vcom->event_ptr, BIT(CDC_ACM_VCOM_ACTIVIED),
                            AOS_OR_CLEAR, &actual_flags, AOS_NO_WAIT);
    }

    return AOS_SUCCESS;
}

static void *aic_io_cdc_acm_open(int com)
{
    cdc_acm_vcom_t *acm_vcom = aos_zalloc(sizeof(*acm_vcom));

    AIC_DUMMY_ASSERT(acm_vcom);

    aicx_debug(0x1c8ddf, "d", "USB VOM(%d) open!\n", com);

    /* basic mode can't use usb. */
    if (aic_boot_mode_is_basic())
        return NULL;

    acm_vcom->com = com;
    snprintf(acm_vcom->name, sizeof(acm_vcom->name), CDC_ACM_NAME_FMT, com);
    acm_vcom->event_ptr = aos_create_event(acm_vcom->name);
    AIC_DUMMY_ASSERT(acm_vcom->event_ptr);

    acm_vcom->cdc_handle = usb_cdc_acm_open(com, aic_io_cdc_acm_cb);
    if (acm_vcom->cdc_handle) {
        enum vcom_event state = usb_cdc_acm_get_state(acm_vcom->cdc_handle);
        acm_vcom->vcom_state =(state == VCOM_ACTIVE ?
           CDC_ACM_VCOM_ACTIVIED : CDC_ACM_VCOM_DEACTIVIED);
    }

    return acm_vcom;
}

static void aic_io_cdc_acm_close(void *handle)
{
    cdc_acm_vcom_t *acm_vcom = handle;

    aicx_debug(0x167453, "d", "USB VOM(%d) close!\n", acm_vcom->com);

    aos_delete_event(acm_vcom->event_ptr);
    usb_cdc_acm_close(acm_vcom->cdc_handle);
    aos_free(acm_vcom);
}

static size_t aic_io_cdc_acm_read(void *handle, void *buffer, size_t size, int timeout)
{
    cdc_acm_vcom_t *acm_vcom = handle;
    u32 actual_flags;
    aos_status ret;
    size_t read;

    if (acm_vcom->vcom_state == CDC_ACM_VCOM_DEACTIVIED) {
        aicx_debug(0x1ced39, "d", "USB VOM(%d) read wait activie!\n", acm_vcom->com);
        ret = aos_get_event(acm_vcom->event_ptr, BIT(CDC_ACM_VCOM_ACTIVIED),
                            AOS_OR, &actual_flags, timeout);
        if (ret != AOS_SUCCESS)
            return 0;
    }

    read =  usb_cdc_acm_read(acm_vcom->cdc_handle, buffer, size);
    aicx_debug(0x04e2ed, "dd", "USB VOM(%d) read size=%d!\n", acm_vcom->com, read);

    return read;
}

static size_t aic_io_cdc_acm_write(void *handle, const void *buffer, size_t size, int timeout)
{
    cdc_acm_vcom_t *acm_vcom = handle;
    u32 actual_flags;
    size_t write;

    aicx_debug(0x01706a, "dd", "USB VOM(%d) write size=%d!\n", acm_vcom->com, size);

    if (acm_vcom->vcom_state == CDC_ACM_VCOM_DEACTIVIED) {
        aicx_debug(0x00d031, "d", "USB VOM(%d) write wait activie!\n", acm_vcom->com);
        aos_get_event(acm_vcom->event_ptr, BIT(CDC_ACM_VCOM_ACTIVIED),
                            AOS_OR, &actual_flags, timeout);
    }

    write = usb_cdc_acm_write(acm_vcom->cdc_handle, buffer, size);
    aicx_debug(0x1d7b9d, "ddd", "USB VOM(%d) write =%d, size=%d!\n", acm_vcom->com, write, size);

    return write;
}
#endif /* CONFIG_CDC_ACM_INSTANCE */

#ifdef WIN32
static void *aic_io_file_open(int com)
{
    int index, ret = 0;
    const  char *dir = LOG_DIR_1TH;
    char filename[100];

    /* if dir not exsist, create it */
    if (file_access(dir, NORMAL_MODE))
        ret = file_mkdir(dir);

    if (ret) {
        /* if dir not exsist, create it */
        dir = LOG_DIR_2TH;
        ret = 0;
        if (file_access(dir, NORMAL_MODE))
            ret = file_mkdir(dir);
    }

    AIC_ASSERT(0 == ret);

    /* skip exist log file */
    for (index = 0; 1 ;index++) {
#ifdef CONFIG_CASE_NUMBER_FILE_NAME
        if(0xFFFF == g_lte_case_sub_number)
            snprintf(filename, sizeof(filename), "%s%s_%s_%d_%d.log",
                     dir, LOG_FILE, (com == 0) ? "trace" : "msg",g_lte_case_number, index);
        else
            snprintf(filename, sizeof(filename), "%s%s_%s_%d_%d_%d.log",
                     dir, LOG_FILE, (com == 0) ? "trace" : "msg", g_lte_case_number,
                     g_lte_case_sub_number, index);
#else
        snprintf(filename, sizeof(filename), "%s%s_%s_%d.log",
                 dir, LOG_FILE, (com == 0) ? "trace" : "msg", index);
#endif
        /* not foud */
        if (file_access(filename, NORMAL_MODE))
            break;
    }

    return file_open(filename, "wb");
}
static size_t aic_io_file_read(void *handle, void *buffer, size_t size, int timeout)
{
    return file_read(handle, buffer, size);
}

static size_t aic_io_file_write(void *handle, const void *buffer, size_t size, int timeout)
{
    return file_write(handle, buffer, size);
}
#endif

static size_t aic_io_uart_read(void *handle, void *buffer, size_t size, int timeout)
{
    return uart_read(handle, buffer, size);
}

static size_t aic_io_uart_write(void *handle, const void *buffer, size_t size, int timeout)
{
    if (aos_is_user_thread_context())
        return uart_write(handle, buffer, size);
    else
        return uart_write_dir(handle, buffer, size);
}

#ifdef CONFIG_CMUX
#define AIO_CMUX_DELAY          10
#define CONFIG_CMUX_DEV_DBG     0

#if CONFIG_CMUX_DEV_DBG
typedef struct cmux_vcom {
    u32                 read_cnt;
    u32                 write_cnt;
    u64                 total_read;
    u64                 total_write;
    void                *cmux_handle;
} cmux_vcom_t;

static void *aic_io_cmux_open(int com)
{
    cmux_vcom_t *cmux_vcom = aos_zalloc(sizeof(*cmux_vcom));

    AIC_DUMMY_ASSERT(cmux_vcom);

    cmux_vcom->cmux_handle = aic_cmux_open(com);
    AIC_ASSERT(cmux_vcom->cmux_handle);

    return cmux_vcom;
}

static void aic_io_cmux_close(void *handle)
{
    cmux_vcom_t *cmux_vcom = handle;

    aic_cmux_close(cmux_vcom->cmux_handle);
    aos_free(cmux_vcom);
}

static size_t aic_io_cmux_read(void *handle, void *buffer, size_t size, int timeout)
{
    cmux_vcom_t *cmux_vcom = handle;
    size_t read;

    read =  usb_cdc_acm_read(cmux_vcom->cmux_handle, buffer, size);

    cmux_vcom->read_cnt++;
    cmux_vcom->total_read += read;

    return read;
}

static size_t aic_io_cmux_write(void *handle, const void *buffer, size_t size, int timeout)
{
    cmux_vcom_t *cmux_vcom = handle;

    size_t left = size, write = 0, once;
    const u8 *pos = buffer;

    while (left) {
        once = MIN(left, CMUX_BUFFER_PERFRAME_MAX);
        write = aic_cmux_write(cmux_vcom->cmux_handle, pos, once, timeout);
        AIC_ASSERT (write <= once);
        if (write == 0) {
            if (!aos_is_user_thread_context())
                break;

            if (timeout != AOS_WAIT_FOREVER) {

                if (timeout < AIO_CMUX_DELAY)
                    break;

                timeout -= AIO_CMUX_DELAY;
            }

            aos_sleep(AIO_CMUX_DELAY);
            continue;
        }

#if 0
        /* cmux may use uart, send too quckly may lost data, msleep 10 */
        if (!aos_is_user_thread_context())
            aic_mdelay(AIO_CMUX_DELAY);
        else
            aos_sleep(AIO_CMUX_DELAY);
#endif

        cmux_vcom->write_cnt++;
        cmux_vcom->total_write += write;

        left -= write;
        pos += write;
    }

    return (size - left);
}
#else
#define aic_io_cmux_open    aic_cmux_open;
#define aic_io_cmux_close   aic_cmux_close;
#define aic_io_cmux_read    aic_cmux_read;

static size_t aic_io_cmux_write(void *handle, const void *buffer, size_t size, int timeout)
{
    size_t left = size, write = 0, once;
    const u8 *pos = buffer;

    while (left) {
        once = MIN(left, CMUX_BUFFER_PERFRAME_MAX);
        write = aic_cmux_write(handle, pos, once, timeout);
        AIC_ASSERT (write <= once);
        if (write == 0) {
            if (!aos_is_user_thread_context())
                break;

            if (timeout != AOS_WAIT_FOREVER) {

                if (timeout < AIO_CMUX_DELAY)
                    break;

                timeout -= AIO_CMUX_DELAY;
            }

            aos_sleep(AIO_CMUX_DELAY);
            continue;
        }

#if 0
        /* cmux may use uart, send too quckly may lost data, msleep 10 */
        if (!aos_is_user_thread_context())
            aic_mdelay(AIO_CMUX_DELAY);
        else
            aos_sleep(AIO_CMUX_DELAY);
#endif

        left -= write;
        pos += write;
    }

    return (size - left);
}
#endif /* CONFIG_CMUX_DEV_DBG */
#endif

/*******************************************************************************
 *         aio dev api define
 ******************************************************************************/
AIO_DEV_T *aio_request_dev(u8 iodev, u8 com)
{
    _AIO_DEV_T *_dev;
    AIO_COM_DEV_T *com_dev;
    AOS_INTERRUPT_DECLARE

    AIC_ASSERT(iodev < AIC_IODEV_CNT && com < AIC_COM_CNT);
    AOS_INTERRUPT_DISABLE
    _dev = s_aio_dev[iodev];

    if (!_dev) {
        /* malloc and init it. */
        //AOS_INTERRUPT_RESTORE
        _dev = aos_zalloc(sizeof(*_dev));
        AIC_DUMMY_ASSERT(_dev);
        //AOS_INTERRUPT_DISABLE

        switch(iodev) {
        case AIC_IODEV_UART:
            _dev->open = uart_open;
            _dev->close = uart_close;
            _dev->read = aic_io_uart_read;
            _dev->write = aic_io_uart_write;
            break;

#ifdef CONFIG_CDC_ACM_INSTANCE
        case AIC_IODEV_USB:
            _dev->open = aic_io_cdc_acm_open;
            _dev->close = aic_io_cdc_acm_close;
            _dev->read = aic_io_cdc_acm_read;
            _dev->write = aic_io_cdc_acm_write;
            break;
#endif

#ifdef CONFIG_CMUX
        case AIC_IODEV_CMUX:
            _dev->open = aic_io_cmux_open;
            _dev->close = aic_io_cmux_close;
            _dev->read = aic_io_cmux_read;
            _dev->write = aic_io_cmux_write;
            break;
#endif

#ifdef WIN32
        case AIC_IODEV_FILE:
            _dev->open = aic_io_file_open;
            _dev->close = file_close;
            _dev->read = aic_io_file_read;
            _dev->write = aic_io_file_write;
            break;
#endif

        default:
            AIC_ASSERT(0);
            break;
        }

        /* add ref cnt and save in s_aio_dev[iodev]. */
        _dev->ref_cnt = 0;
        _dev->iodev = iodev;
        s_aio_dev[iodev] = _dev;
    }

    /* get com dev */
    com_dev = _dev->com_dev[com];

    /* com_dev is null, open and init it. */
    if (!com_dev) {
        //AOS_INTERRUPT_RESTORE
        com_dev = aos_zalloc(sizeof(*com_dev));
        AIC_DUMMY_ASSERT(com_dev);
        //AOS_INTERRUPT_DISABLE

        com_dev->com_handle = _dev->open(com);
        AIC_ASSERT(com_dev->com_handle);
        com_dev->com = com;

        /* refrence the io_dev */
        com_dev->io_dev = _dev;
        _dev->ref_cnt++;
        _dev->com_dev[com] = com_dev;
    }

    com_dev->ref_cnt++;
    AOS_INTERRUPT_RESTORE

    return (AIO_DEV_T *)com_dev;
}

void aio_release_dev(AIO_DEV_T *dev)
{
    AIO_COM_DEV_T *com_dev = (AIO_COM_DEV_T *)(dev);
    _AIO_DEV_T *_dev = com_dev->io_dev;
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    /* decrease the ref_cnt */
    com_dev->ref_cnt--;

    /* if com_dev ref_cnt is 0 close com and free it */
    if (!com_dev->ref_cnt) {
        _dev->close(com_dev->com_handle);
        _dev->com_dev[com_dev->com] = NULL;
        _dev->ref_cnt--;
        //AOS_INTERRUPT_RESTORE
        aos_free(com_dev);
        //AOS_INTERRUPT_DISABLE
    }

    /* _dev ref_cnt > 0 don't rease _dev */
    if (_dev->ref_cnt == 0) {
        /* free _dev. */
        s_aio_dev[_dev->iodev] = NULL;
        aos_free((void*)_dev);
    }

    AOS_INTERRUPT_RESTORE
}

size_t aio_read(AIO_DEV_T *dev, void *buffer, size_t size, int timeout)
{
    AIO_COM_DEV_T *com_dev = (AIO_COM_DEV_T *)(dev);
    _AIO_DEV_T *_dev = com_dev->io_dev;

    return _dev->read(com_dev->com_handle, buffer, size, timeout);
}

size_t aio_write(AIO_DEV_T *dev, const void *buffer, size_t size, int timeout)
{
    AIO_COM_DEV_T *com_dev = (AIO_COM_DEV_T *)(dev);
    _AIO_DEV_T *_dev = com_dev->io_dev;

    return _dev->write(com_dev->com_handle, buffer, size, timeout);
}

void *aio_get_dev_uart_handle(AIO_DEV_T *dev)
{
    AIO_COM_DEV_T *com_dev = (AIO_COM_DEV_T *)(dev);
    _AIO_DEV_T *_dev = com_dev->io_dev;

    if (_dev->iodev == AIC_IODEV_UART)
        return com_dev->com_handle;

    return NULL;
}
