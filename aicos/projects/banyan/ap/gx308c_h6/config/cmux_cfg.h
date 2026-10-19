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

#ifndef __CMUX_CFG_H__
#define __CMUX_CFG_H__

#define AIC_CMUX_COM     AIC_COM3 //real uart port, diag port
#define AIC_CMUX_DEV     AIC_IODEV_UART

/* CMUX using long frame mode by default */
#define CMUX_BUFFER_PERFRAME_MAX                1504
#define CMUX_BUFFER_SEND_MAX                    4096

#define AIC_CMUX_MDAT_COM                       AIC_CMUX_VCOM5
#define AIC_CMUX_DATA_COM                       AIC_CMUX_VCOM6
#define AIC_CMUX_VOICE_COM                      AIC_CMUX_VCOM7
#define AIC_CMUX_MCTRL_COM                      AIC_CMUX_VCOM10
#define AIC_CMUX_LOG_COM                        AIC_CMUX_VCOM11
#define AIC_CMUX_DIAG_COM                       AIC_CMUX_VCOM12

const static cmux_dev_attr_t s_cmux_dev_info[] = {
#ifdef CONFIG_CMUX_PRIORITY
    {AIC_CMUX_VOICE_COM, CMUX_MODE_SYNC,  0, CMUX_ISO_TYPE},
    {AIC_CMUX_DATA_COM,  CMUX_MODE_ASYNC, 32, CMUX_BULK_TYPE},
    {AIC_CMUX_VCOM4,     CMUX_MODE_ASYNC, 32, CMUX_BULK_TYPE},
    {AIC_CMUX_LOG_COM,   CMUX_MODE_ASYNC, 0, CMUX_BULKLOG_TYPE},
#else
    {AIC_CMUX_VOICE_COM, CMUX_MODE_SYNC,  0},
    {AIC_CMUX_DATA_COM,  CMUX_MODE_ASYNC, 32},
    {AIC_CMUX_VCOM4,     CMUX_MODE_ASYNC, 32}
#endif
};

/* cmux dev config tbl. */
const static cmux_vdev_t s_cmux_at_vdevtbl[] = {
    {AIC_CMUX_UART,    AIC_CMUX_VCOM3},/* use the top uart0 */
    {AIC_CMUX_UART,    AIC_CMUX_VCOM2},
    {AIC_CMUX_UART,    AIC_CMUX_VCOM4},
    {AIC_CMUX_UART,    AIC_CMUX_VCOM13}
};
/* cmux dev config tbl. */
const static cmux_vdev_t s_cmux_data_vdevtbl[] = {
};


/* =========================================================================*/
/* cmux connect config tbl. */
const static cmux_conn_cfg_t s_cmux_conn_tbl[] = {
    {CMUX_CONN_AT, ARRAY_SIZE(s_cmux_at_vdevtbl), s_cmux_at_vdevtbl, AIC_TRUE},
};

#endif /* __CMUX_CFG_H__ */

