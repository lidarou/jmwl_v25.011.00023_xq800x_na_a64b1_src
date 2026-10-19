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
#ifndef __AIC_IO_DEV_H__
#define __AIC_IO_DEV_H__
#include <aic_type.h>
#include <alog_api.h>

/* io device type define. */
enum {
    AIC_IODEV_NONE = 0,
    AIC_IODEV_UART,
    AIC_IODEV_USB,
    AIC_IODEV_FILE,
#ifdef CONFIG_CMUX
    AIC_IODEV_CMUX,
#endif
    AIC_IODEV_CNT
};

/* io device com define. */
enum {
    AIC_COM0 = 0,
    AIC_COM1,
    AIC_COM2,
    AIC_COM3,
    AIC_COM4,
    AIC_COM5,
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
    AIC_COM6,
    AIC_COM7,
    AIC_COM8,
    AIC_COM9,
    AIC_COM10,
    AIC_COM11,
    AIC_COM12,
    AIC_COM13,
    AIC_COM14,
    AIC_COM15,
#endif
    AIC_COM_CNT,
};

#define AIC_INVALID_IODEV   0xFF
#define AIC_INVALID_COM     0xFF

/**
 * AIO_DEV_T define
 */
typedef void AIO_DEV_T;

/*
 * AIO_DEV interface define
 */

/**
 * aio_request_dev - request a AIO_DEV_T handle.
 *
 * @iodev: device type: AIC_IODEV_UART or AIC_IODEV_USB.
 * @com: which com port will request.
 *
 * @return: AIO_DEV_T pointer, NULL - failed.
 */
AIO_DEV_T *aio_request_dev(u8 iodev, u8 com);
void aio_release_dev(AIO_DEV_T *dev);
void *aio_get_dev_uart_handle(AIO_DEV_T *dev);

size_t aio_read(AIO_DEV_T *dev, void *buffer, size_t size, int timeout);
size_t aio_write(AIO_DEV_T *dev, const void *buffer, size_t size, int timeout);

#endif /* __AIC_IO_DEV_H__ */

