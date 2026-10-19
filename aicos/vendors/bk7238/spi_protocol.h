#ifndef __SPI_PROTOCOL_H__
#define __SPI_PROTOCOL_H__

#include <stdint.h>

#define BK_SPI_DIR_IN   0x1100
#define BK_SPI_DIR_OUT  0x2200

#define BK_CMD_SET_WIFI     (BK_SPI_DIR_OUT | 0x01)
#define BK_CMD_SET_SLEEP    (BK_SPI_DIR_OUT | 0x02)
#define BK_CMD_CLEAR_EVENT  (BK_SPI_DIR_OUT | 0x03)
#define BK_CMD_BLE_CONFIG   (BK_SPI_DIR_OUT | 0x04)
#define BK_CMD_SET_WIFI_AP  (BK_SPI_DIR_OUT | 0x05)
//...
#define BK_CMD_WRITE_PKT    (BK_SPI_DIR_OUT | 0x50)
#define BK_DATA_VALID_OUT   (BK_SPI_DIR_OUT | 0x5E)
#define BK_DATA_VALID_OUT2  (BK_SPI_DIR_OUT | 0x6E)

#define BK_CMD_GET_IP       (BK_SPI_DIR_IN | 0x01)
#define BK_CMD_GET_MAC      (BK_SPI_DIR_IN | 0x02)
//...
#define BK_CMD_PEEK_PKT_LEN (BK_SPI_DIR_IN | 0x52)
#define BK_CMD_READ_PKT     (BK_SPI_DIR_IN | 0x53)
#define BK_CMD_GPIO_DOWN    (BK_SPI_DIR_IN | 0x54)
#define BK_DATA_VALID_IN    (BK_SPI_DIR_IN | 0xE5)
#define BK_DATA_INVALID     (BK_SPI_DIR_IN | 0xEE)

#define BK_HEAD_LEN         4

#define MAX_NET_PKT_LEN     1600

#define SPI_TX_SIZE_MAX     (MAX_NET_PKT_LEN + BK_HEAD_LEN)


//event handling
#define EVT_PKT_TYPE_LEN    2
#define RX_FIX              2
#define NEXT_PKT_DATA_LEN   2

#define EVT_TYPE_GOT_PKT                0x10
#define EVT_TYPE_GOT_IP                 0x11
#define EVT_TYPE_DISCONNECTED           0x12
#define DISCONNECTED_REASON_WRONG_SSID  0x01
#define DISCONNECTED_REASON_WRONG_PWD   0x02
#define DISCONNECTED_REASON_AP_KICK     0x03
#define EVT_TYPE_WAKEUP                 0x13
#define WAKEUP_REASON_BOOT              0x01
#define WAKEUP_REASON_REMOTE            0x02

int init_spi_buffers(void);
void deinit_spi_buffers(void);

void set_wifi(uint32_t handle, uint8_t *ssid, uint8_t *pwd);
int set_ap(uint32_t handle, uint8_t *ssid, uint8_t *pwd);
int enter_deepsleep(void);
int spi_ble_config(void);
int clear_event(uint32_t handle,uint8_t event);

void get_ip(uint32_t handle, uint8_t *data, int data_len);
void get_mac(uint32_t handle, uint8_t *data, int data_len);

int receive_net_pkt(uint32_t handle,uint8_t *data, int data_len, u16 *next_pkt_len);
int receive_next_pkt(uint32_t handle, uint8_t *data, int pkt_len, u16 *next_pkt_len);
int send_net_pkt(uint32_t handle, uint8_t *data, int data_len);
//int receive_net_pkt(uint32_t handle,uint8_t *data, int data_len);

#endif
