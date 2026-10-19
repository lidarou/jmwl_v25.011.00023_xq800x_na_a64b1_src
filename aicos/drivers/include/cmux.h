/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CMUX_H__
#define __CMUX_H__

#include <aic_type.h>
#include <aic_size.h>
#include <aic_osa.h>

#define CMUX_PORT_NUMBER            16              //according to customer defined, here update.
#define CMUX_DFLTFRM_LIST_LEN        15

#ifdef CONFIG_CMUX_PRIORITY
typedef enum {
    CMUX_BULKLOG_TYPE,
    CMUX_BULK_TYPE,
    CMUX_ISO_TYPE,
    CMUX_INT_TYPE,
    CMUX_CTRL_TYPE
} cmux_send_type_t;
#endif

/**
 * the cmux device attr.
 */
typedef struct cmux_dev_attr {
    u8                      com;
    u8                      recv_mode;
    u8                      frame_number;
#ifdef CONFIG_CMUX_PRIORITY
    cmux_send_type_t        send_type;
#endif
} cmux_dev_attr_t;

/* io device com define. */
enum {
    AIC_CMUX_VCOM0 = 0,
    AIC_CMUX_VCOM1,
    AIC_CMUX_VCOM2,
    AIC_CMUX_VCOM3,
    AIC_CMUX_VCOM4,
    AIC_CMUX_VCOM5,
    AIC_CMUX_VCOM6,
    AIC_CMUX_VCOM7,
    AIC_CMUX_VCOM8,
    AIC_CMUX_VCOM9,
    AIC_CMUX_VCOM10,
    AIC_CMUX_VCOM11,
    AIC_CMUX_VCOM12,
    AIC_CMUX_VCOM13,
    AIC_CMUX_VCOM14,
    AIC_CMUX_VCOM15,
    AIC_CMUX_VCOM_CNT,
};

enum {
    CMUX_MODE_SYNC = 0,
    CMUX_MODE_ASYNC,
};


/* io device type define. */
enum {
    AIC_CMUX_NONE = 0,
    AIC_CMUX_UART,
    AIC_CMUX_SPI,
    AIC_CMUX_CNT
};

/**
 * The cmd cmux business define:
 * @CMUX_CONN_AT: This cmux used with AT business.
 * @CMUX_CONN_DATA: This cmux used with data business.
 */
enum {
    CMUX_CONN_AT = 0x10,
    CMUX_CONN_DATA,
    CMUX_CONN_CTRL,
    CMUX_CONN_CNT
} ;

/**
 * the connect port define.
 * @vdev_type: The device type. (usb, uart, ...)
 * @vdev_com: The device com. (com0, com1, ...)
 */
typedef struct cmux_vdev {
    u8   vdev_type;
    u8   vdev_com;
} cmux_vdev_t;

/**
 * The cmux config data.
 * One cmd biz may usd multiple io dev at the same time,
 * But one data biz only usd one io dev at the same time.
 * @biz: the business.
 * @dev_cnt: the count of the device tabl,
 * @dev_tbl: the device tabl,
 */
typedef struct cmux_conn_cfg {
    u8          conn_type;
    u8          conn_cnt;
    const cmux_vdev_t   *dev_tbl;
    BOOL        is_callback;
} cmux_conn_cfg_t;

/**
 * the connect device number.
 */
typedef struct cmux_dev_num {
    u16  dev_sum_num;
    u16  dev_index[CMUX_PORT_NUMBER];
}cmux_dev_num_t;


#ifdef CONFIG_CMUX
size_t aic_cmux_write(void *handle, const void *buffer, size_t size, int timeout);
size_t aic_cmux_read(void *handle, void *buffer, size_t size, int timeout);
void *aic_cmux_open(int com);
void aic_cmux_close(void *handle);
u8 get_cmux_linkupstatus();
BOOL aic_cmux_channel_status(u8 channel_id);
void aic_cmux_set_realdev_baudrate(int rate);
int aic_cmux_get_realdev_baudrate();
#else
inline size_t aic_cmux_write(void *handle, const void *buffer, size_t size, int timeout){return 0;}
inline size_t aic_cmux_read(void *handle, void *buffer, size_t size, int timeout){return 0;}
inline void *aic_cmux_open(int com){return AIC_NULL;}
inline void aic_cmux_close(void *handle){return ;}
inline u8 get_cmux_linkupstatus(){return 0;}
inline BOOL aic_cmux_channel_status(u8 channel_id){return 0;}
inline void aic_cmux_set_realdev_baudrate(int rate){return;}
inline int aic_cmux_get_realdev_baudrate(void){return 0;}
#endif

typedef void (*cmux_recv_cb)(u32 cmux_vcom_id, u8 *buffer, size_t size, void *p);
size_t cmux_cmd_send(u8 biz, u32 conn_id, u8 *buf, size_t size, int timeout);

void cmux_conn_register_recv_cb(u8 biz, cmux_recv_cb cb, void *p);
#define cmux_at_register_recv_cb(cb, p) cmux_conn_register_recv_cb(CMUX_CONN_AT, (cb), (p))
#define cmux_at_send(id, buf, size)  cmux_cmd_send(CMUX_CONN_AT, (id), (buf), (size), AOS_WAIT_FOREVER)
cmux_dev_num_t cmux_get_at_sum_conid();
#ifdef CONFIG_CMUX
aos_queue_ptr cmux_conn_get_queue(u8 conn_type);
void *cmux_conn_port_id_handle(u8 port_id);
#else
inline aos_queue_ptr cmux_conn_get_queue(u8 conn_type){return (aos_queue_ptr)0xffffffff;}
inline void *cmux_conn_port_id_handle(u8 port_id){return AIC_NULL;}
#endif


#endif /* __CMUX_H__ */
