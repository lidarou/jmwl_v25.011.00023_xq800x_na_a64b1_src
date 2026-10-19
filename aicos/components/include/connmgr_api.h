/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the connect-mgr module-related data and api.
 */
#ifndef __CONNMGR_API_H__
#define __CONNMGR_API_H__

#include <aic_type.h>
#include <aic_io_dev.h>

/**
 * The work mode of the at connect:
 * @CONN_AT_MODE: AT mode
 * @CONN_PPP_MODE: PPP mode.
 */
typedef enum {
    CONN_AT_MODE = 0,
    CONN_PPP_MODE
} conn_mode_t;

/**
 * The data connect business define:
 * @CONN_DATA_LOG: This connect used with log business.
 * @CONN_DATA_DIAG: This connect used with diagnosis business.
 */
typedef enum {
    CONN_DATA_LOG = 0,
    CONN_DATA_DIAG,
    CONN_DATA_CNT
} conn_data_biz_t;

/**
 * The cmd connect business define:
 * @CONNECT_AT: This connect used with AT business.
 * @CONNECT_CONS: This connect used with console business.
 */
typedef enum {
    CONN_CMD_AT = 0,
    CONN_CMD_CONS,
    CONN_CMD_CNT
} conn_cmd_biz_t;

/* The ppp data use the same biz as the at cmd. */
#define CONN_PPP_BIZ    CONN_CMD_AT

/**
 * conn_recv_cb - It is the receive ppp data callback function.
 * @conn_id: The connect id of current connect that received data.
 * @buf: The pointer of ppp data, must free it sfter use.
 * @size: the size of the ppp data
 * @p: the private data pointer of the caller.
 */
typedef void (*conn_recv_cb)(u32 conn_id, u8 *buf, size_t size, void *p);

/* conn_cmd_register_recv_cb - register a callback function to receive data.
 *
 * @biz: The specified business.
 * @cb: the callback function for receive data from the specified business
 *      connect, can be null. if it is null call, receive data by the api
        connmgr_recv.
 * @p the private pointer of the caller.
 */
void conn_cmd_register_recv_cb(u8 biz, conn_recv_cb cb, void *p);

/* conn_at_register_recv_cb - register a callback function to receive data.
 *
 * @cb: the callback function for receive data from the specified business
 *      connect, can be null. if it is null call, receive data by the api
        connmgr_recv.
 * @p the private pointer of the caller.
 */
#define conn_at_register_recv_cb(cb, p) conn_cmd_register_recv_cb(CONN_CMD_AT, (cb), (p))

/* conn_ppp_register_recv_cb - register a callback function to receive ppp data.
 *
 * @cb: the callback function for receive ppp data
 * @p the private pointer of the caller.
 */
void conn_ppp_register_recv_cb(conn_recv_cb cb, void *p);

/**
 * conn_at_send - Send some at data from specified connect id
                  of the specified connect func.
 *
 * @conn_id: The connect id of the specified business.
 * @buf: The pointer of the data.
 * @size: The size of the data.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: actual write size.
 */
#define conn_at_send(id, buf, size)  conn_cmd_send(CONN_CMD_AT, (id), (buf), (size), AOS_WAIT_FOREVER)

/**
 * conn_ppp_send - Send some at data from specified connect id
                  of the specified connect func.
 *
 * @conn_id: The connect id of the specified business.
 * @buf: The pointer of the data.
 * @size: The size of the data.
 *
 * @return: actual write size.
 */
#define conn_ppp_send(id, buf, size) conn_cmd_send(CONN_PPP_BIZ, (id), (buf), (size), AOS_WAIT_FOREVER)
#define conn_ppp_send_ex(id, buf, size, timout) conn_cmd_send(CONN_PPP_BIZ, (id), (buf), (size), timout)

/**
 * conn_at_set_work_mode - Set work mode of at connect.
 * @conn_id: The connect id of the specified business.
 * @mode: CONN_AT_MODE or CONN_PPP_MODE.
 */
void conn_at_set_work_mode(u32 conn_id, conn_mode_t mode);

/**
 * conn_cmd_send - Send some data from specified connect id
                  of the specified connect func.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @buf: The pointer of the data.
 * @size: The size of the data.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 * @return: actual write size.
 */
size_t conn_cmd_send(u8 biz, u32 conn_id, u8 *buf, size_t size, int timeout);

/**
 * conn_cmd_fctrl_set -Set the connect dev fctrl.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @enable: enable or disable the fctrl.
 *
 * @return: succ or err number.
 */
int conn_cmd_fctrl_set(u8 biz, u32 conn_id, boolean enable);

/**
 * conn_cmd_fctrl_set -Set the connect dev fctrl.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @val: store the fctrl status. 0: diable fctrl, 1: enable fctrl.
 *
 * @return: succ or err number.
 */
int conn_cmd_fctrl_get(u8 biz, u32 conn_id, u32 *val);

/**
 * conn_cmd_set_baud_rate -Set the connect dev baud rate.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @enable: enable or disable the fctrl.
 *
 * @return: succ or err number.
 */
int conn_cmd_set_baud_rate(u8 biz, u32 conn_id, int rate, boolean is_need_to_save);

/**
 * conn_cmd_get_baud_rate -Get the connect dev baud rate.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @enable: enable or disable the fctrl.
 *
 * @return: err number or the baud rate.
 */
int conn_cmd_get_baud_rate(u8 biz, u32 conn_id);

/**
 * conn_data_recv -Read the data from specified connect func.
 *
 * @biz: The specified business.
 * @conn_id: The connect id of the specified business.
 * @timeout: the time of timeout, the unit is milliseconds.
 *
 */
size_t conn_data_recv(u8 biz, u8 *buf, size_t size, int timeout);

/**
 * conn_data_send -Read some data from specified bearer and link.
 *
 * @biz: The specified business.
 * @buf: The pointer of the data.
 * @size: The size of the data.
 *
 * @return: actual write size.
 */
size_t conn_data_send(u8 biz, u8 *buf, size_t size, int timeout);

/**
 * conn_data_request_iodev - Request a iodev for the specified biz..
 *
 * @biz: The specified business.
 * @iodev: The new iodev of the connect.
 *
 * @ 0: succ, other value, failed.
 */
int conn_data_request_iodev(u8 biz, u8 iodev);

/**
 * conn_data_release_iodev - Release a iodev for the specified biz.
 *
 * @biz: The specified business.
 *
 */
void conn_data_release_iodev(u8 biz);

/**
 * conn_data_get_dev_com_by_type -
 * get the device com by the specified biz, and device type.
 *
 * @biz: The specified business.
 * @type: The device type.
 *
 */
int conn_data_get_dev_com_by_type(u8 biz, u8 type);
#define conn_get_log_dev_com(type)  conn_data_get_dev_com_by_type(CONN_DATA_LOG, type)
#define conn_get_diag_dev_com(type) conn_data_get_dev_com_by_type(CONN_DATA_DIAG, type)

/**
 * conn_data_set_baud_rate - set the device band rate of the specified biz.
 *
 * @biz: The specified business.
 *
 */
int conn_data_set_baud_rate(u8 biz, int rate);

/**
 * conn_get_at_biz - get the specified at biz number.
 *
 * @biz: The specified business.
 *
 */
u32 conn_get_at_conid(u8 type);
#define conn_get_uart_at_conid   conn_get_at_conid(AIC_IODEV_UART)
#define conn_get_usb_at_conid   conn_get_at_conid(AIC_IODEV_USB)

/**
 * conn_get_at_dev_type - get the at dev type by specified conid.
 *
 * @conn_id: The specified conid.
 *
 */
u8 conn_get_at_dev_type(u32 conn_id);

/**
 * conn_get_at_com_number - get the at dev com number by specified conn_id.
 *
 * @conn_id: The specified conid.
 *
 */
u8 conn_get_at_com_number(u32 conn_id);

/*
 * CONNMGR_MALLOC, use it malloc ,must use CONNMGR_FREE to free.
 * the ppp data need cacheline align, so use aos_dma_malloc
 */
#define CONNMGR_MALLOC  aos_dma_malloc
#define CONNMGR_FREE    aos_dma_free

#endif /* __CONNMGR_API_H__ */

