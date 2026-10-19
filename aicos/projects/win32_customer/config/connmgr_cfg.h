/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines connect manager configurations.
 */

#ifndef __CONNMGR_CFG_H__
#define __CONNMGR_CFG_H__

/* at cmd dev config tbl. */
const static conn_dev_t s_at_dev_tbl[] = {
    {AIC_IODEV_UART,    AIC_COM1},
#ifdef CONFIG_CDC_ACM_INSTANCE
    {AIC_IODEV_USB,     AIC_COM2}, /* use the usb com2. */
#endif
};

/* console only support one com. */
const static conn_dev_t s_cons_dev_tbl[1] = {
    {AIC_IODEV_UART,    AIC_COM0},
};

/* log dev config tbl. */
const static conn_dev_t s_log_dev_tbl[] = {
    {AIC_IODEV_UART,    AIC_COM0}, /* use the mcu uart */
#ifdef CONFIG_CDC_ACM_INSTANCE
    {AIC_IODEV_USB,     AIC_COM0}, /* use the usb com0. */
#endif
};

/* diag dev config tbl. */
const static conn_dev_t s_diag_dev_tbl[] = {
    {AIC_IODEV_UART,    AIC_COM2},
#ifdef CONFIG_CDC_ACM_INSTANCE
    {AIC_IODEV_USB,     AIC_COM1},/* use the usb com1. */
#endif
};

/* cmd connect config tbl. */
const static conn_dev_cfg_t s_cmd_conn_tbl[] = {
    {CONN_CMD_AT,   ARRAY_SIZE(s_at_dev_tbl),   s_at_dev_tbl},
    {CONN_CMD_CONS, ARRAY_SIZE(s_cons_dev_tbl), s_cons_dev_tbl}
};

/* data connect config tbl. */
const static conn_dev_cfg_t s_data_conn_tbl[] = {
    {CONN_DATA_LOG,  ARRAY_SIZE(s_log_dev_tbl),  s_log_dev_tbl},
    {CONN_DATA_DIAG, ARRAY_SIZE(s_diag_dev_tbl), s_diag_dev_tbl}
};

#endif /* __CONNMGR_CFG_H__ */

