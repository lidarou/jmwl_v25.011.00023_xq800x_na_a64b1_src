/************************************************************************ 
* Copyright © 2022, ICOE (Shanghai) Technologies Co.,Ltd. All rights reserved.              *
* Information in this document is provided in connection with ICOE’s  products.             *
* No license, express or implied, by estoppels or otherwise, to any intellectual property   *
* rights is granted by this document. Except as  provided in ICOE’s Terms and               *
* Conditions of Sale for such products, ICOE assumes no liability whatsoever, and           *
* ICOE disclaims any express or implied warranty, relating to sale and/or use of            *
* ICOE products including liability or warranties relating to fitness for a particular      *
* purpose, merchantability, or infringement of any patent, copyright or other               *
* intellectual property right. ICOE products are not intended for use in    medical, life   *
* saving, or life sustaining applications.                                                  * 
* ICOE (Shanghai) Technologies Co.,Ltd may make changes to specifications and               * 
* product descriptions at any time, without notice.                                         * 
* You may obtain copies of documents which have a document number and are                   *
* referenced in this document, or other ICOE literature may be obtained by                  * 
* mailing to xiaopingjiang@icoe-tech.com.                                                                                                       *
************************************************************************/ 

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13be7a
#define LOG_MODULE    AMOD_GNSS

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <serial_inc.h>
#include <gpio_inc.h>
#include "icoe_gps_os.h"
#include "icoe_gps_serial.h"
#include "icoe_gps_os.h"
#include "icoe_gps_hal.h"
#include "icoe_gps_nmea.h"
#include <pmu_user.h>
#include <gnss/gnss_cfg.h>

void gps_reset_gpio_init(void)
{
    aic_gpio_request(gps_dev.rst);
    if (gps_dev.vana_name)
        aic_gpio_request(gps_dev.ana_en);
}

void gps_reset_gpio(void)
{
    aic_gpio_output(gps_dev.rst, GPIO_OUTPUT_LVL_LOW);
    aos_sleep(20);
    aic_gpio_output(gps_dev.rst, GPIO_OUTPUT_LVL_HIGH);
}

u32 gps_uart_read(uint8_t *data, uint32_t size)
{
    return uart_read(gps_dev.handle, data, size);
}

u32 gps_uart_read_dir(uint8_t *data, uint32_t size)
{
    return uart_read_dir(gps_dev.handle, data, size);
}

void gps_get_packet_data(char* buf, int size)
{
    u32 actual;

    while(size) {
        actual = gps_uart_read(buf, size);
        size = size - actual;
        buf = buf + actual;
    }
}

int gps_get_alloc_buffer(void)
{
    int i = gps_mem_pool.block_num;

    while (i == gps_mem_pool.block_num) {
        for (i = 0; i < gps_mem_pool.block_num; i++)
             if (!((gps_mem_pool.map >> i) & 0x1))
                break;
       if (i == gps_mem_pool.block_num)
            aos_sleep(10);
    }
    gps_mem_pool.map = gps_mem_pool.map | (0x1 << i);
    return i;
}

void gps_free_buffer(int index)
{
    if (!((gps_mem_pool.map >> index) & 0x1))
        return;

    gps_mem_pool.map = gps_mem_pool.map & (~(0x1 << index));
}

void gps_read_data(void *data)
{
    struct GPS_MSG msg;
    int length, index;
    u8 *bufptr;

    while (1)  {
        if (!gps_dev.status) {
            gps_dev.rx_status = 0;
            aos_suspend_thread(gps_dev.rx_thread);
        }
        gps_dev.rx_status = 1;
        index = gps_get_alloc_buffer();
        bufptr = gps_mem_pool.buf_ptr + index * gps_mem_pool.block_size;
        memset(bufptr, 0, gps_mem_pool.block_size);
        length = gps_uart_read(bufptr,  gps_mem_pool.block_size);
        //gps_get_packet_data(bufptr, gps_mem_pool.block_size);

        msg.ev = UM_GPS_RECV_IND;
        msg.param0 = (uint32_t)length;
        msg.param1 = bufptr;
        msg.param2 = index;

        gps_send_qmsg(gpsTaskq, &msg, AOS_WAIT_FOREVER);
    }
}

void gps_uart_start_receive(void)
{
    aos_resume_thread(gps_dev.rx_thread);
}

int gps_uart_open(void)
{
    int ret = -1;
    gps_log(LOG_INFO, "enter %s\n", __FUNCTION__);
    gps_dev.handle = uart_open(gps_dev.com);
    gps_log(LOG_INFO, "leave %s\n", __FUNCTION__);
    if (!gps_dev.handle)
        return ret;

    gps_set_baudrate(gps_dev.baud);
    gps_dev.status = GPS_UART_OPEN;

    return 0;
}

u32 gps_uart_write(const uint8_t *data, uint32_t size)
{
    return uart_write(gps_dev.handle, data, size);
}

int gps_uart_close(void )
{
    gps_dev.status = GPS_UART_CLOSE;
    while (gps_dev.rx_status);
    uart_close(gps_dev.handle);
    return 0;
}

int gps_set_baudrate(u32 baudrate)
{
    gps_dev.baud = baudrate;

    AICX_ASSERT_IFX(gps_dev.handle, 0x1d82ed, "", "serial open fail handler == NULL");
    uart_set_baud_rate(gps_dev.handle, baudrate);

    return 0;
}

void gps_uart_destroy()
{
    gps_uart_close();
}

s32 gps_tty_write(char* data, unsigned int len)
{
    u32 length, actual_len = 0;

    while(len)
    {
        length = gps_uart_write(data + actual_len, len);
        actual_len += length;
        len -= length;
    }

    return 0;
}

s32 gps_wakeup(void)
{
    s32 ret = 0;
    s8 cmd[] = "$CFGGLP,0\r\n";

    //gps_mutex_lock(&tty_if_lock);
    if(gps_tty_write(cmd, sizeof(cmd)) < 0) {
        gps_log(LOG_ERROR, "wakeup failed\n");
        ret = -1;
    }
    //gps_mutex_unlock(&tty_if_lock);

    return ret;
}

s32 gps_sleep(void)
{
    s32 ret = 0;
    s8 cmd[] = "$CFGGLP,1\r\n";

    //gps_mutex_lock(&tty_if_lock);
    if(gps_tty_write(cmd, strlen(cmd)) < 0) {
        gps_log(LOG_ERROR, "go to sleep failed\n");
        ret = -1;
    }
    //gps_mutex_unlock(&tty_if_lock);

    return ret;
}

s32 gps_aidtime(void)
{
    s8 cmd[] = "$AIDTIME,2018,12,31,23,59,59,999\r\n";
    aos_systime time;
    s32 ret = 0;

    if (!gps_dev.enable_32k_less)
        return ret;

    aos_get_systime(&time);
    snprintf(cmd, sizeof(cmd), "$AIDTIME,%d,%d,%d,%d,%d,%d,%d\r\n",
        time.year, time.month, time.day, time.hour,
        time.minute, time.second, time.msecond);
    if(gps_tty_write(cmd, strlen(cmd)) < 0) {
        gps_log(LOG_ERROR, "send aidtime fail\n");
        ret = -1;
    }

    return ret;
}

void gps_enable_vbk_power(void)
{
    pmu_enable(gps_dev.gnss_vbak);
}

void gps_disable_vbk_power(void)
{
    pmu_disable(gps_dev.gnss_vbak);
}

void gps_enable_power(void)
{
    pmu_enable(gps_dev.gnss_vcore);
    if (gps_dev.vana_name)
        pmu_enable(gps_dev.gnss_ana);
}

void gps_disable_power(void)
{
    pmu_disable(gps_dev.gnss_vcore);
    if (gps_dev.vana_name)
        pmu_disable(gps_dev.gnss_ana);
}

int gps_firmware_download_enable(void)
{
    return gps_dev.fw_enable;
}

int gps_uart_init(void)
{
    gps_reset_gpio_init();

    if (gps_dev.vcore_name) {
        gps_dev.gnss_vcore = pmu_get("gnss_vcore", gps_dev.vcore_name);
        if (!gps_dev.gnss_vcore) {
            gps_log(LOG_ERROR,"get %s failed!\n", gps_dev.vcore_name);
            return AIC_ENXIO;
        }
        pmu_voltage_set(gps_dev.gnss_vcore, 1900000, 30000);
    }

    if (gps_dev.vbak_name) {
        gps_dev.gnss_vbak = pmu_get("gnss_vbak", gps_dev.vbak_name);
        if (!gps_dev.gnss_vbak) {
            gps_log(LOG_ERROR, "get %s failed!\n", gps_dev.vbak_name);
            goto out_vcore;
        }
        pmu_voltage_set(gps_dev.gnss_vbak, 1800000, 10000);
    }

    if (gps_dev.vana_name) {
        gps_dev.gnss_ana = pmu_get("gnss_vana", gps_dev.vana_name);
        if (!gps_dev.gnss_ana) {
            gps_log(LOG_ERROR, "get %s failed!\n", gps_dev.vana_name);
            goto out_vbak;
        }
        pmu_voltage_set(gps_dev.gnss_ana, 1800000, 10000);
    }
    gps_enable_vbk_power();

    gps_dev.rx_thread = aos_create_thread("GPS read", gps_read_data, NULL,
                GPS_RX_THREAD_STACK_SIZE, 0, NULL, GPS_RX_THREAD_PRIORITY,
                AOS_DONT_START);
    return AIC_SUCCESS;

out_vbak:
    if (gps_dev.vbak_name)
        pmu_put(gps_dev.gnss_vbak);
out_vcore:
    if (gps_dev.vcore_name)
        pmu_put(gps_dev.gnss_vcore);

    return AIC_ENXIO;
}

AIC_NORMAL_APP_L1_INIT_EXPORT(gps_uart_init);
