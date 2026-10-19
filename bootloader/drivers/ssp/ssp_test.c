/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the ssp driver test case.
 */

#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_size.h>
#include <aic_type.h>
#include <cache.h>
#include <initcall.h>
#include <ssp.h>
#include <timer.h>

#define SSP_TEST_STACK_SIZE (SIZE_4K)
#define SSP_TEST_SIZE   (50)

#define SSP_TEST_BUFFER_SIZE    (2048)
static char s_ssp_bus_name[2][8] = {"ssp0", "ssp1"};
static u32 s_ssp_dev_handle[2] = {0};
static char s_ssp_dev_name[2][16] = {"ssp_dev0", "ssp_dev1"};
static u8 CACHE_LINE_ALIGN s_ssp_tx_buf[SSP_TEST_BUFFER_SIZE];
static u8 CACHE_LINE_ALIGN s_ssp_rx_buf[SSP_TEST_BUFFER_SIZE];

static void generate_test_pattern(u8 *buffer, u32 length)
{
    u32 i;

    for (i = 0; i < length; i++)
        buffer[i] = (i + 1) & 0xFF;
}

void ssp_test_entry(void)
{
    u32 test_size;
    int ret, i, dev;

    test_size = SSP_TEST_SIZE;

    memset(s_ssp_tx_buf, 0x0, test_size);
    memset(s_ssp_rx_buf, 0x0, test_size);

    generate_test_pattern(s_ssp_tx_buf, test_size);
    memset(s_ssp_rx_buf, 0x0, test_size);

    dcache_invcc_by_range((u32)s_ssp_tx_buf, test_size);
    dcache_invcc_by_range((u32)s_ssp_rx_buf, test_size);

    dev = 0;

    /* read */
    ret = ssp_read(s_ssp_dev_handle[dev], s_ssp_rx_buf, test_size);
    ASSERT_INFO(ret != 0, "ssp%d read failed(%d)!\n", dev, ret);

    for (i = 0; i < test_size; i++)
        ASSERT_INFO(s_ssp_tx_buf[i] == s_ssp_rx_buf[i],
                    "ssp test read check failed at:i=%d\n", i);
    log_print("ssp test read pass!\n");

    /* write */
    ret = ssp_write(s_ssp_dev_handle[dev], s_ssp_tx_buf, test_size);
    ASSERT_INFO(ret == AIC_SUCCESS,
                        "ssp%d write failed(%d)!\n", dev, ret);
    log_print("ssp test write end!\n");

    /* write read */
    ret = ssp_write_read(s_ssp_dev_handle[dev], s_ssp_tx_buf, s_ssp_rx_buf,
                            test_size);
    ASSERT_INFO(ret == AIC_SUCCESS,
                        "ssp%d write read failed(%d)!\n", dev, ret);

    for (i = 0; i < test_size; i++)
        ASSERT_INFO(s_ssp_tx_buf[i] == s_ssp_rx_buf[i],
                        "ssp write read check failed at:i=%d\n", i);
    log_print("ssp test write read pass!\n");
}

#define SSP_MASTER
//#define SSP_SLAVE
int ssp_test_init(void)
{
    struct ssp_cfg cfg;
    int ret;

#ifdef SSP_MASTER
    /* register device 0 to ssp bus */
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    cfg.cs = 0;
    cfg.work_clk = 25000000;

    ret = ssp_device_register(&s_ssp_dev_handle[0], s_ssp_dev_name[0],
                                s_ssp_bus_name[1], &cfg);
    ASSERT(ret == AIC_SUCCESS);

    /* register device 1 to ssp bus */
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    cfg.cs = 1;
    cfg.work_clk = 1000000;
    ret = ssp_device_register(&s_ssp_dev_handle[1], s_ssp_dev_name[1],
                                s_ssp_bus_name[1], &cfg);
    ASSERT(ret == AIC_SUCCESS);
#elif defined(SSP_SLAVE)
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    ret = ssp_device_register(&s_ssp_dev_handle[0], s_ssp_dev_name[0],
                                s_ssp_bus_name[1], &cfg);
    ASSERT(ret == AIC_SUCCESS);
#endif

    ssp_test_entry();

    return AIC_SUCCESS;
}
driver_init(ssp_test_init);
