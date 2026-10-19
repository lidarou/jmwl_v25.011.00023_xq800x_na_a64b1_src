#ifndef __SPI_OP_H__
#define __SPI_OP_H__

#include <stdint.h>

#define pr_debug printf
#define pr_err printf

#define SPI_DEBUG 0

#define DEFAULT_DEV_NAME	"/dev/spidev1.0"
#define DEFAULT_DEV_SPEED	23000000

int spi_open(char *dev_name, int speed_setting);
int spi_transfer(const uint8_t *tx_buf, uint8_t *rx_buf, int len);
int spi_transfer_n(uint8_t *tx_buf[], uint8_t *rx_buf[], int len[], int count);
int spi_read(uint8_t *rx_buf, int len);
int spi_write(uint8_t *tx_buf, int len);
int spi_close(void);

#endif
