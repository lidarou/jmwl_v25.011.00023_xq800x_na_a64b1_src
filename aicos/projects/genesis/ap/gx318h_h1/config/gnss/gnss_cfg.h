/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __GNSS_CFG_H__
#define __GNSS_CFG_H__

#include <pinlist.h>
#include <pmic_io.h>
#include <pmic_reg_base.h>
#include <pd_pad_ctrl.h>
#include <rtc_pad_ctrl.h>

#define GPS_RX_THREAD_STACK_SIZE    0x400
#define GPS_RX_THREAD_PRIORITY      4
#define GPS_RX_BUF_NUM          10

enum {
    GPS_UART_CLOSE,
    GPS_UART_OPEN,
};

typedef struct
{
    int com;
    int baud;
    void *handle;
    aos_thread_id rx_thread;
    int status;
    volatile int rx_status;
    struct pmu_user *gnss_vcore;
    struct pmu_user *gnss_vbak;
    char * vcore_name;
    char * vbak_name;
    GPIO_PIN_ID rst;
    GPIO_PIN_ID ana_en;
    struct pmu_user *gnss_ana;
    char * vana_name;
    int fw_enable;
    int enable_32k_less;
} GPS_DEVICE_T;

typedef struct
{
    u32 map;
    u32 block_num;
    u32 block_size;
    u8 *buf_ptr;
} GPS_MEM_POOL;

__attribute__((aligned(32))) u8 gps_nmea_buf[GPS_RX_BUF_NUM][UART_RX_BUF_SIZE];
GPS_MEM_POOL gps_mem_pool = {
    .map = 0,
    .block_num = GPS_RX_BUF_NUM,
    .block_size = UART_RX_BUF_SIZE,
    .buf_ptr = (u8 *)gps_nmea_buf,
};
GPS_DEVICE_T gps_dev = {
    .com = 2,
    .baud = 115200,
    .rst = GPIO_AON_8,
    .vcore_name = "VDDBK3V3",
    .fw_enable = 1,
};

void gps_pin_switch_enable(void)
{
}

void gps_pin_switch_disable(void)
{
}

extern HANDLE gpsTaskq;

#endif /* __GNSS_CFG_H__ */
