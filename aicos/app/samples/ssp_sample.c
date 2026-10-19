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

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x181739

#include <aic_convert.h>
#include <aic_errno.h>
#include <aic_gpio_pin.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_type.h>
#include <aic_at_resp_handle.h>
#include <cache.h>
#include <gpio_inc.h>
#include <pinlist.h>
#include <ssp.h>
#include <timer_inc.h>

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

/**
 * at_ssp_rw_cmd - response at^ssprw=?.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_ssp_rw_at_cmd(AT_REQ_RESP *at_req)
{
    ulong rsp[2];

    rsp[0] = (ulong)"^ssprw: dev, rw(0:read; 1:write; 2:read/write), size";
    rsp[1] = (ulong)AT_RESULT_OK;

    return aic_fill_transmit_cmd_resp(at_req, rsp, 2);
}

/**
 * at_ssp_rw_set_cmd - process at^ssprw=rw.
 *
 * Parameter:
 *  @at_req: at request.
 *
 * Return:
 */
u32 aic_ssp_rw_at_set_cmd(AT_REQ_RESP *at_req)
{
    u8 result_line[AT_MAX_AP_RESP_LENGTH] = {'\0'};
    ulong line_pointers[AT_REPLY_LINE_COUNT_ONCE];
    ulong *paras_arr;
    u16 count;
    int error_code = AIC_SUCCESS;
    u8 rw;
    u32 test_size;
    int ret, i, dev;

    do {
        paras_arr = aic_split_paras_to_str_array(at_req->at_req, "AT^SSPRW=",
                                                 &count);
        if (!paras_arr || count != 3) {
            error_code = AIC_EINVAL;
            break;
        }

        dev = (u8)(atoi((u8 *)paras_arr[0]));
        if (dev > 2) {
            error_code = AIC_EINVAL;
            break;
        }

        rw = (u8)(atoi((u8 *)paras_arr[1]));
        if (rw > 2) {
            error_code = AIC_E2BIG;
            break;
        }

        test_size = (atoi((u8 *)paras_arr[2]));

        if (s_ssp_dev_handle[dev] == 0) {
            error_code = AIC_E2BIG;
            break;
        }

        memset(s_ssp_tx_buf, 0x0, test_size);
        memset(s_ssp_rx_buf, 0x0, test_size);

        generate_test_pattern(s_ssp_tx_buf, test_size);
        memset(s_ssp_rx_buf, 0x0, test_size);

        dcache_invcc_by_range((u32)s_ssp_tx_buf, test_size);
        dcache_invcc_by_range((u32)s_ssp_rx_buf, test_size);

        if (rw == 0) {
            /* read */
            ret = ssp_read(s_ssp_dev_handle[dev], s_ssp_rx_buf, test_size);
            AIC_ASSERT_INFO(ret != 0,
                                "ssp%d read failed(%d)!\n", dev, ret);

            for (i = 0; i < test_size; i++)
                AIC_ASSERT_INFO(s_ssp_tx_buf[i] == s_ssp_rx_buf[i],
                                "read check failed at:i=%d\n", i);
        } else if (rw == 1) {
            /* write */
            ret = ssp_write(s_ssp_dev_handle[dev], s_ssp_tx_buf, test_size);
            AIC_ASSERT_INFO(ret == AIC_SUCCESS,
                                "ssp%d write failed(%d)!\n", dev, ret);
        } else if (rw == 2) {
            /* write read */
            ret = ssp_write_read(s_ssp_dev_handle[dev], s_ssp_tx_buf, s_ssp_rx_buf,
                                    test_size);
            AIC_ASSERT_INFO(ret == AIC_SUCCESS,
                                "ssp%d write read failed(%d)!\n", dev, ret);

            for (i = 0; i < test_size; i++)
                AIC_ASSERT_INFO(s_ssp_tx_buf[i] == s_ssp_rx_buf[i],
                                "write read check failed at:i=%d\n", i);
        }
    } while(0);

    if (error_code != AIC_SUCCESS) {
        snprintf(result_line, AT_MAX_AP_RESP_LENGTH,
                 "+CME ERROR: %d", error_code);
        line_pointers[0] = (ulong)result_line;
    } else {
        line_pointers[0] = (ulong)AT_RESULT_OK;
    }

    aic_free_paras_mem(paras_arr, count);

    return aic_fill_transmit_cmd_resp(at_req, line_pointers, 1);
}

/*
 *  Add ssp AT handle here.
 */
const static AT_CMD_HANDLE aic_ssp_at_handles[] =
{
    {
        "AT^SSPRW",
        aic_ssp_rw_at_cmd,
        NULL,
        aic_ssp_rw_at_set_cmd,
        NULL
    },
    {AIC_NULL},
};

#define SSP_MASTER
//#define SSP_SLAVE
/**
 * aic_ssp_cmd_init - ssp at cmd init.
 *
 * Parameter:
 *
 * Return:
 */
int aic_ssp_cmd_init(void)
{
    struct ssp_cfg cfg;
    int ret;

    aic_add_at_cmd_handle((AT_CMD_HANDLE *)aic_ssp_at_handles);

#ifdef SSP_MASTER
    /* register device 0 to ssp bus */
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    cfg.cs = 0;
    cfg.work_clk = 25000000;

    ret = ssp_device_register(&s_ssp_dev_handle[0], s_ssp_dev_name[0],
                                s_ssp_bus_name[1], &cfg);
    AIC_ASSERT(ret == AIC_SUCCESS);

    /* register device 1 to ssp bus */
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    cfg.cs = 1;
    cfg.work_clk = 1000000;
    ret = ssp_device_register(&s_ssp_dev_handle[1], s_ssp_dev_name[1],
                                s_ssp_bus_name[1], &cfg);
    AIC_ASSERT(ret == AIC_SUCCESS);
#elif defined(SSP_SLAVE)
    memset(&cfg, 0x0, sizeof(cfg));
    cfg.spi_mode = SPI_MODE_0;
    ret = ssp_device_register(&s_ssp_dev_handle[0], s_ssp_dev_name[0],
                                s_ssp_bus_name[1], &cfg);
    AIC_ASSERT(ret == AIC_SUCCESS);
#endif

    return AIC_SUCCESS;
}
AIC_NORMAL_COMP_P2_INIT_EXPORT(aic_ssp_cmd_init);

