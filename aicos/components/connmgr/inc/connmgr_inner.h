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
#ifndef __CONNMGR_INNER_H__
#define __CONNMGR_INNER_H__

#include <aic_type.h>
#include <aic_io_dev.h>
#include <aic_osa.h>

#define CONN_NAME_FMT "cmd-%d-%d"
#define CONN_NAME_LEN 8

typedef enum {
    CON_CMD_TYPE = 0,
    CON_CMD_DATA,
} conn_type_t;

/**
 * the connect device define.
 * @dev_type: The device type. (usb, uart, ...)
 * @dev_com: The device com. (com0, com1, ...)
 */
typedef struct conn_dev {
    u8   dev_type;
    u8   dev_com;
} conn_dev_t;

/**
 * The connect config data.
 * One cmd biz may usd multiple io dev at the same time,
 * But one data biz only usd one io dev at the same time.
 * @biz: the business.
 * @dev_cnt: the count of the device tabl,
 * @dev_tbl: the device tabl,
 */
typedef struct conn_dev_cfg {
    u8          biz;
    u8          dev_cnt;
    const conn_dev_t   *dev_tbl;
} conn_dev_cfg_t;

/**
 * the connect device info.
 */
typedef struct conn_dev_info {
    u8              biz;
    u8              index;
    u8              mode;
    aos_thread_id   thread;
    char            name[CONN_NAME_LEN];
    AIO_DEV_T       *io_dev;
} conn_cmd_dev_t;

/**
 * the cmd connect mgr.
 */
typedef struct conn_cmd_mgr {
    u8              biz;
    u8              dev_cnt;
    conn_recv_cb    cmd_cb;
    void            *data;
    conn_cmd_dev_t  *dev_info;
} conn_cmd_mgr_t;

/**
 * the data connect mgr.
 */
typedef struct conn_data_mgr {
    u8              biz;
    u8              dev_type;
    u8              dev_com;
    u8              dev_cnt;
    AIO_DEV_T       *io_dev;
    const conn_dev_t *dev_tbl;
} conn_data_mgr_t;

#endif /* __CONNMGR_INNER_H__ */

