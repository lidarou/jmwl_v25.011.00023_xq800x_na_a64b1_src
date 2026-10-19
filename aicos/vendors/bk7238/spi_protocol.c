#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cache.h>
#include <aic_osa.h>
#include <aic_osa_ex.h>
#include "spi.h"
#include "spi_protocol.h"
#include <timer_cfg.h>
#include <gpio_inc.h>

static aos_mutex_ptr g_mutex;
#define SPI_LOCK()   aos_get_mutex(g_mutex, AOS_WAIT_FOREVER);
#define SPI_UNLOCK() aos_put_mutex(g_mutex);
uint8_t spi_txbuf[SPI_TX_SIZE_MAX] = {0};

static uint8_t CACHE_LINE_ALIGN spi_dummy_buf[MAX_NET_PKT_LEN] = {0};

extern int ssp_write(u32 handle, u8 *buf, u32 size);
extern int ssp_read(u32 handle, u8 *buf, u32 size);
extern int ssp_write_read(u32 handle, u8 *wbuf, u8 *rbuf, u32 size);

//phase1 : type + length
//phase2 : type + length + data
static void set_short(uint8_t *out, uint16_t in)
{
    out[0] = (in >> 8) & 0xff;
    out[1] = in  & 0xff;
}

static uint16_t get_short(uint8_t *buffer)
{
    uint16_t value;

    value = (buffer[0] << 8) | buffer[1];
    return value;
}

static int send_spi(uint8_t *buffer, uint32_t len)
{
    return spi_write(buffer, len);
}

static int receive_spi(uint8_t *buffer, uint32_t len)
{
    return spi_read(buffer, len);
}

static void wait_spi_slave(uint32_t u_secs)
{
    aic_ndelay(u_secs * 1000);
}

#define DEFAULT_SPI_WAIT_US     1500

#define WAIT_SPI() wait_spi_slave(DEFAULT_SPI_WAIT_US)

#define STUFF_BYTE 0x55

static uint8_t *g_tx_buffer = NULL;
//static uint8_t *g_rx_buffer = NULL;

int init_spi_buffers(void)
{
    g_mutex = aos_create_mutex("bk7238 spi mutex");

    return 0;
}

void deinit_spi_buffers(void)
{
    ;
}

void set_wifi(uint32_t handle, uint8_t *ssid, uint8_t *pwd)
{
    int ret;
    uint8_t *ptr = spi_txbuf;
    g_tx_buffer = malloc( BK_HEAD_LEN + MAX_NET_PKT_LEN);
    uint8_t *p = g_tx_buffer;
    // uint8_t buffer[120];
    int len;

    len = strlen(ssid) + 1 + strlen(pwd) + 1;
    if (BK_HEAD_LEN + len > 120) {
        if (g_tx_buffer)
            free(g_tx_buffer);
        return;
    }

    SPI_LOCK();
    set_short(ptr, BK_CMD_SET_WIFI);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle,spi_txbuf, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(p, BK_DATA_VALID_OUT);
    set_short(p + 2, len);
    sprintf(p + 4, "%s%c%s", ssid, 0, pwd);
    ret = ssp_write(handle,g_tx_buffer, BK_HEAD_LEN + len);
    SPI_UNLOCK();
    free(g_tx_buffer);
}

int set_ap(uint32_t handle, uint8_t *ssid, uint8_t *pwd)
{
    int ret;
    uint8_t *ptr = spi_txbuf;
    g_tx_buffer = malloc( BK_HEAD_LEN + MAX_NET_PKT_LEN);
    uint8_t *p = g_tx_buffer;
    // uint8_t buffer[120];
    int len;

    len = strlen(ssid) + 1 + strlen(pwd) + 1;
    if (BK_HEAD_LEN + len > 120) {
        if (g_tx_buffer)
            free(g_tx_buffer);
        return -1;
    }

    SPI_LOCK();
    set_short(ptr, BK_CMD_SET_WIFI_AP);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle,spi_txbuf, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(p, BK_DATA_VALID_OUT);
    set_short(p + 2, len);
    sprintf(p + 4, "%s%c%s", ssid, 0, pwd);
    ret = ssp_write(handle,g_tx_buffer, BK_HEAD_LEN + len);
    SPI_UNLOCK();
    free(g_tx_buffer);

    return ret;
}

int enter_deepsleep(void)
{
    int ret;
    uint8_t buffer[6];
    int len;

    len = 0;

    SPI_LOCK();
    set_short(buffer, BK_CMD_SET_SLEEP);
    set_short(buffer + 2, 0);
    ret = send_spi(buffer, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(buffer, BK_DATA_VALID_OUT);
    set_short(buffer + 2, len);
    ret = send_spi(buffer, BK_HEAD_LEN + len);
    SPI_UNLOCK();

    return ret > 0 ? 0 : -1;
}

int spi_ble_config(void)
{
    int ret;
    uint8_t buffer[6];
    int len;

    len = 0;

    SPI_LOCK();
    set_short(buffer, BK_CMD_BLE_CONFIG);
    set_short(buffer + 2, 0);
    ret = send_spi(buffer, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(buffer, BK_DATA_VALID_OUT);
    set_short(buffer + 2, len);
    ret = send_spi(buffer, BK_HEAD_LEN + len);
    SPI_UNLOCK();

    return ret > 0 ? 0 : -1;
}

int clear_event(uint32_t handle,uint8_t event)
{
    int ret;
    uint8_t buffer[6];
    int len;

    len = 2;

    SPI_LOCK();
    set_short(buffer, BK_CMD_CLEAR_EVENT);
    set_short(buffer + 2, 0);
    ret = ssp_write(handle,buffer, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(buffer, BK_DATA_VALID_OUT);
    set_short(buffer + 2, len);
    set_short(buffer + 4, event << 8 | 0x00);
    ret = ssp_write(handle,buffer, BK_HEAD_LEN + len);
    SPI_UNLOCK();

    return ret;
}

void send_net_pkt2(uint32_t handle, uint8_t *data, int data_len)
{
    int ret;
    //uint8_t *ptr = spi_txbuf;
    g_tx_buffer = malloc( BK_HEAD_LEN + MAX_NET_PKT_LEN);
    uint8_t *send_buffer = g_tx_buffer;

    if (!send_buffer || (data_len <= 0) || (data_len > MAX_NET_PKT_LEN)) {
        if (g_tx_buffer)
            free(g_tx_buffer);
        return;
    }

    SPI_LOCK();
    set_short(spi_txbuf, BK_DATA_VALID_OUT);
    set_short(spi_txbuf + 2, 0);
    ret = ssp_write(handle,spi_txbuf, BK_HEAD_LEN);

    WAIT_SPI();

    set_short(g_tx_buffer, BK_CMD_WRITE_PKT);
    set_short(g_tx_buffer + 2, data_len);
    memcpy(g_tx_buffer + 4, data, data_len);
    ret = ssp_write(handle,g_tx_buffer, BK_HEAD_LEN + data_len);
    SPI_UNLOCK();
    free(g_tx_buffer);
}

int send_net_pkt(uint32_t handle, uint8_t *data, int data_len)
{
    int ret;
    uint8_t *send_buffer = data;

    if (!send_buffer || (data_len <= 0) || (data_len > MAX_NET_PKT_LEN)) {
        return -1;
    }

    send_buffer -= BK_HEAD_LEN;

    SPI_LOCK();
    set_short(send_buffer, BK_DATA_VALID_OUT2);
    set_short(send_buffer + 2, data_len);
    memcpy(send_buffer + 4, data, data_len);
    ret = ssp_write(handle, send_buffer, BK_HEAD_LEN + data_len);
    if (ret) {
        alog_warn("[BK] send pkt fail %d %d\n", ret, BK_HEAD_LEN + data_len);
    }
    SPI_UNLOCK();

    return ret;
}

int get_ip_old(uint8_t *ip_out)
{
    int ret;
    uint8_t buffer[30];
    int len;
    uint16_t cmd_resp;

    len = 16;   //xxx.xxx.xxx.xxx

    SPI_LOCK();
    set_short(buffer, BK_CMD_GET_IP);
    set_short(buffer + 2, 0);
    ret = send_spi(buffer, BK_HEAD_LEN);

    WAIT_SPI();

    ret = receive_spi(buffer, BK_HEAD_LEN + RX_FIX + len);
    SPI_UNLOCK();
    cmd_resp = get_short(buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN)
        return -1;

    memcpy(ip_out, buffer + RX_FIX + 4, len);
    ip_out[len-1] = '\0';
    return 0;
}

void get_ip(uint32_t handle, uint8_t *data, int data_len)
{
    int ret;
    uint8_t *ptr = spi_txbuf;
    uint8_t buffer[8 + 16 * 4];
    // int len;
    uint16_t cmd_resp;

    // len = 16 * 4; //xxx.xxx.xxx.xxx

    SPI_LOCK();
    set_short(ptr, BK_CMD_GET_IP);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle,ptr, BK_HEAD_LEN);

    WAIT_SPI();

    ret = ssp_write_read(handle,spi_dummy_buf, buffer, BK_HEAD_LEN + RX_FIX + data_len);
    SPI_UNLOCK();
    cmd_resp = get_short(buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN)
        return;

    memcpy(data, buffer + RX_FIX + 4, data_len);
    data[data_len-1] = '\0';
    alog_info("ip:%s\n",data);
}

void get_mac(uint32_t handle, uint8_t *data, int data_len)
{
    int ret;
    uint8_t *ptr = spi_txbuf;

    // int len;
    uint16_t cmd_resp;
    uint8_t buffer[30];

    // len = 18; //xx:xx:xx:xx:xx:xx

    SPI_LOCK();
    set_short(ptr, BK_CMD_GET_MAC);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle, spi_txbuf, BK_HEAD_LEN);

    WAIT_SPI();
    ret = ssp_write_read(handle,spi_dummy_buf, buffer, BK_HEAD_LEN + RX_FIX + data_len);
    SPI_UNLOCK();
    cmd_resp = get_short(buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN) {
        alog_info("mac: cmd resp is error 0x%x, ret %d\n", cmd_resp, ret);
        return;
    }

    memcpy(data, buffer + RX_FIX + 4, data_len);
    data[data_len-1] = '\0';
    alog_info("mac:%s\n",data);
}

#if 0//
int receive_net_pkt(uint8_t *data, int data_len)
{
    int ret;
    uint8_t buffer[6];
    int len;
    uint16_t real_len;
    uint16_t cmd_resp;
    uint16_t real_len_2;
    uint8_t *recv_buffer = g_rx_buffer;
    int valid = 1;

    if (!recv_buffer || (data_len <= 0) || (data_len > MAX_NET_PKT_LEN))
        return -1;

    //first round
    len = 2;
    SPI_LOCK();
    set_short(buffer, BK_CMD_PEEK_PKT_LEN);
    set_short(buffer + 2, 0);
    ret = send_spi(buffer, BK_HEAD_LEN);

    WAIT_SPI();

    ret = receive_spi(buffer, BK_HEAD_LEN + RX_FIX + len);
    SPI_UNLOCK();
    cmd_resp = get_short(buffer + RX_FIX);
    if (cmd_resp == BK_DATA_INVALID) //no pkt yet
        return 0;

    if (cmd_resp != BK_DATA_VALID_IN)
        return -1;
    real_len = get_short(buffer + RX_FIX + 4);
    if (real_len > data_len) {
        valid = -2;
        real_len = 0; //let it go
    } else if (real_len == 0) { //no pkt yet
        return 0;
    }

    //second round
    WAIT_SPI();
    //wait_spi_slave(500);

    SPI_LOCK();
    set_short(buffer, BK_CMD_READ_PKT);
    set_short(buffer + 2, 0);
    ret = send_spi(buffer, BK_HEAD_LEN);

    WAIT_SPI();

    recv_buffer[0] = STUFF_BYTE;
    ret = receive_spi(recv_buffer, BK_HEAD_LEN + RX_FIX + real_len);
    SPI_UNLOCK();
    cmd_resp = get_short(recv_buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN)
        valid = -3;
    real_len_2 = get_short(recv_buffer + RX_FIX + 2);
    if (real_len_2 != real_len) {
        printf("first %d then %d\n", real_len, real_len_2);
        valid = -4;
    }

    if (valid > 0)
    memcpy(data, recv_buffer + RX_FIX + 4, real_len);

    return valid > 0 ? real_len : valid;
}
#else//big lock
#define PKT_DELAY_MS  7//7-95us
#if 1
int receive_net_pkt(uint32_t handle,uint8_t *data, int data_len, u16 *next_pkt_len)
{
    int ret;
    int len;
    uint16_t real_len;
    uint16_t cmd_resp;
    uint16_t real_len_2;
    uint16_t next_len;
    uint8_t buffer[16];
    uint8_t *ptr = spi_txbuf;
    uint8_t *recv_buffer = data;
    int valid = 1;
    if (!recv_buffer || (data_len <= 0) || (data_len > MAX_NET_PKT_LEN))
        return -1;

    if (next_pkt_len)
        *next_pkt_len = 0;
    //first round
    len = 2;
    SPI_LOCK();
    set_short(ptr, BK_CMD_PEEK_PKT_LEN);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle, spi_txbuf, BK_HEAD_LEN);

    wait_spi_slave(PKT_DELAY_MS);

    ret = ssp_write_read(handle, spi_dummy_buf, buffer, BK_HEAD_LEN + RX_FIX + len);

    cmd_resp = get_short(buffer + RX_FIX);
    if (cmd_resp == BK_DATA_INVALID) //no pkt yet
    {
        SPI_UNLOCK();
        alog_info("[BK7238] invalid cmd_resp: 0x%x\n", cmd_resp);
        return 0;
    }

    if (cmd_resp != BK_DATA_VALID_IN)
    {
        SPI_UNLOCK();
        alog_info("[BK7238] cmd_resp: 0x%x\n", cmd_resp);
        return -1;
    }
    real_len = get_short(buffer + RX_FIX + 4);
    if (real_len > data_len || real_len == 0) {
        SPI_UNLOCK();
        alog_info("[BK7238] real len %d is wrong\n", real_len);
        return 0;
    }

    //second round
    wait_spi_slave(1);//0-20us,1-30us

    //SPI_LOCK();
    set_short(buffer, BK_CMD_READ_PKT);
    set_short(buffer + 2, 0);
    ret = ssp_write(handle, buffer, BK_HEAD_LEN);

    wait_spi_slave(PKT_DELAY_MS);

    recv_buffer -= (RX_FIX + BK_HEAD_LEN + EVT_PKT_TYPE_LEN + NEXT_PKT_DATA_LEN);
    recv_buffer[0] = STUFF_BYTE;
    ret = ssp_write_read(handle,spi_dummy_buf, recv_buffer, BK_HEAD_LEN + RX_FIX + 4 + real_len);
    SPI_UNLOCK();
    cmd_resp = get_short(recv_buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN)
        valid = -3;
    real_len_2 = get_short(recv_buffer + RX_FIX + 2);
    next_len = get_short(recv_buffer + RX_FIX + 4 + 2);
    if (next_pkt_len)
        *next_pkt_len = next_len;
    if (real_len_2 != real_len) {
        printf("first %d then %d\n", real_len, real_len_2);
        valid = -4;
        return -5;
    }

    return real_len_2 - 4;
}
#endif
#endif

#define NEXT_PKT_DELAY       10
#define GPIO_CHECK_RETRY_MAX 200
int receive_next_pkt(uint32_t handle, uint8_t *data, int pkt_len, u16 *next_pkt_len)
{
    int ret;
    //uint8_t buffer[6];
    uint16_t cmd_resp;
    uint16_t real_len_2, next_len;
    uint8_t *ptr = spi_txbuf;
    uint8_t *recv_buffer;
    int valid = 1;
    int gpio_check_count = 0;
    static u8 pull_up_timeout = 0;

    recv_buffer = data;

    if (!recv_buffer || (pkt_len <= 0) || (pkt_len > MAX_NET_PKT_LEN))
        return -1;
    if (next_pkt_len)
        *next_pkt_len = 0;

    if (!pull_up_timeout) {
        while (aic_gpio_level_get(14) && gpio_check_count < GPIO_CHECK_RETRY_MAX) {
            gpio_check_count++;
            wait_spi_slave(NEXT_PKT_DELAY);
        }

        if (gpio_check_count >= GPIO_CHECK_RETRY_MAX) {
            alog_warn("[BK] gpio pull down timeout\n");
            SPI_LOCK();
            set_short(ptr, BK_CMD_GPIO_DOWN);
            set_short(ptr + 2, 0);
            ret = ssp_write(handle, spi_txbuf, BK_HEAD_LEN);
            SPI_UNLOCK();
            return 0;
        }

        gpio_check_count = 0;
    }

    recv_buffer -= (RX_FIX + BK_HEAD_LEN + EVT_PKT_TYPE_LEN + NEXT_PKT_DATA_LEN);
    SPI_LOCK();
    set_short(ptr, BK_CMD_READ_PKT);
    set_short(ptr + 2, 0);
    ret = ssp_write(handle, spi_txbuf, BK_HEAD_LEN);

    //SPI_LOCK();
    do {
        gpio_check_count++;
        wait_spi_slave(NEXT_PKT_DELAY);
    } while (!aic_gpio_level_get(14) && gpio_check_count < GPIO_CHECK_RETRY_MAX);

    if (gpio_check_count >= GPIO_CHECK_RETRY_MAX) {
        alog_warn("[BK] gpio pull up timeout count %d\n", pull_up_timeout);
        pull_up_timeout++;

        if (pull_up_timeout >= 10) {
            set_short(ptr, BK_CMD_GPIO_DOWN);
            set_short(ptr + 2, 0);
            ret = ssp_write(handle, spi_txbuf, BK_HEAD_LEN);
            pull_up_timeout = 0;
        }

        SPI_UNLOCK();
        return 0;
    }

    pull_up_timeout = 0;

    wait_spi_slave(30);

    recv_buffer[0] = STUFF_BYTE;
    set_short(spi_dummy_buf, BK_CMD_GPIO_DOWN);
    set_short(spi_dummy_buf + 2, 0);
    ret = ssp_write_read(handle, spi_dummy_buf, recv_buffer,
                         BK_HEAD_LEN + RX_FIX + 4 + pkt_len); // 4 for event(2B) + next_len(2B)

    SPI_UNLOCK();

    cmd_resp = get_short(recv_buffer + RX_FIX);
    if (cmd_resp != BK_DATA_VALID_IN) {
        alog_warn("[BK] recv invalid packets cmd_resp 0x%x gpio_check_count %d\n",
                  cmd_resp, gpio_check_count);
        return -3;
    }

    real_len_2 = get_short(recv_buffer + RX_FIX + 2);
    next_len = get_short(recv_buffer + RX_FIX + 4 + 2);
    if (next_pkt_len)
        *next_pkt_len = next_len;
    /*
    if (real_len_2 != real_len) {
        printf("first %d then %d\n", real_len, real_len_2);
        valid = -4;
    }
    */
    if (real_len_2 > MAX_NET_PKT_LEN || real_len_2 == 0) {
        real_len_2 = 0;
        valid = -4;
    }

    alog_debug("[BK] gpio_check_count %d\n", gpio_check_count);
    alog_debug("[MAC] recv ret %d real_len_2 %d next_len %d valid: 0x%x\n", ret,
               real_len_2, next_len, valid);
    return real_len_2 - 4;
}


