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

/**
 * platform related setting
 */

#ifndef _GPS_SERIAL_H_
#define _GPS_SERIAL_H_

#include <aic_type.h>

#define XCPU_CACHE_LINE_SIZE (16)
#define XCPU_CACHE_LINE_MASK (XCPU_CACHE_LINE_SIZE - 1)

#define UART_RX_DMA_SIZE (256)
#define UART_RX_BUF_SIZE (256)
#define UART_TX_BUF_SIZE (4 * 1024)
#define UART_RX_POLL_PERIOD (100) // ms

#define UART_WAIT_TX_DONE_TIMEOUT (500)

#define GPS_UART_POOL_SIZE (UART_RX_DMA_SIZE + UART_RX_BUF_SIZE + UART_TX_BUF_SIZE)

int gps_uart_init(void);
int gps_uart_open(void);
int gps_uart_close(void);
u32 gps_uart_read(uint8_t *data, uint32_t size);
u32 gps_uart_read_dir(uint8_t *data, uint32_t size);
u32 gps_uart_write(const uint8_t *data, uint32_t size);
s32 gps_tty_write(char* data, unsigned int len);
int gps_set_baudrate(u32 baudrate);
s32 gps_wakeup(void);
s32 gps_sleep(void);
void gps_enable_power(void);
void gps_disable_power(void);
void gps_uart_start_receive(void);
void gps_reset_gpio_init(void);
void gps_reset_gpio(void);
void gps_get_packet_data(char* buf, int size);
void gps_free_buffer(int index);
void gps_pin_switch_enable(void);
void gps_pin_switch_disable(void);
int gps_firmware_download_enable(void);
s32 gps_aidtime(void);
#endif /* _GPS_SERIAL_H_ */

