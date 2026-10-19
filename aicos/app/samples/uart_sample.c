/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file defines the uart driver sample code.
 */
/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
#include <aic_type.h>
#include <alog_api.h>
#include <aic_osa_ex.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <serial_inc.h>

#ifdef CONFIG_SUBSYS_AP
#define UART_SAMPLE_COM                 5   /* top uart1 */
#else
#define UART_SAMPLE_COM                 1   /* top uart1 */
#endif

/* open the uart software loopback test. */
#define UART_LOOP_BACK_SW       0

#if IS_CONF_ENABLED(UART_LOOP_BACK_SW)
#include <aic_io_dev.h>
#include <aic_size.h>

#define LOOPBACK_COM        UART_SAMPLE_COM
#define LOOPBACK_BANDRATE   115200
#define ONCE_READ_SIZE      128

#define READ_THREAD_PRI      20
#define WRITE_THREAD_PRI     21
#define LOOP_QUEUE_NUM       128

aos_thread_id uart_recv_thread_id;
aos_thread_id uart_send_thread_id;
aos_queue_ptr uart_queue_id;

u32 *serial_handle;

typedef struct uart_data {
    u8 *addr;
    size_t size;
} uart_data_t;

static void uart_recv_entry(void *param)
{
    size_t read;
    char *buf;
    uart_data_t *data;

    /**
     * 1.Com number is aic_uart array index, aic_uart at aicos\projects\xxx\config\serial\serial_config.h;
     * 2.use aic_uart[index] parameter to set serial.
     */
    serial_handle = uart_open(LOOPBACK_COM);
    aic_assert(serial_handle);

    /* change baudrate */
    uart_set_baud_rate(serial_handle, LOOPBACK_BANDRATE);

    while(1) {
        /* read data. */
        buf = aic_dma_malloc(ONCE_READ_SIZE);

        read = uart_read(serial_handle, buf, ONCE_READ_SIZE);
        if (read == 0 || read > ONCE_READ_SIZE) {
            drv_logw("uart_recv: err size = (%d)!\n", read);
            aic_dma_free(buf);
            continue;
        }

        //drv_logi("uart_recv: read size = (%d)!\n", read);

        data = aic_malloc(sizeof(*data));
        data->addr = buf;
        data->size = read;
        aos_send_to_queue(uart_queue_id, (void*)&data, AOS_WAIT_FOREVER);
    }

    /* close uart. */
    uart_close(serial_handle);
}

static void uart_send_entry(void *param)
{
    size_t write, left, once;
    char *buf;
    uart_data_t *data;
    aos_status ret;

    while(1) {
        ret = aos_recv_from_queue(uart_queue_id, (void *)&data, AOS_WAIT_FOREVER);
        if (ret != AOS_SUCCESS)
            continue;

        left = data->size;
        buf = data->addr;
        write = 0;

        while (left) {
            once = uart_write(serial_handle, buf + write, left);
            if (once > left) {
                drv_loge("uart_send: write err left=%d, once=%d!\n", left, once);
                break;
            }
            left -= once;
            write += once;
        }

        //drv_logi("uart_send: write size = (%d)!\n", write);
        aic_dma_free(buf);
        aic_free(data);
    }
}
#else /* UART_LOOP_BACK_TEST */
/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define UART_SAMPLE_TASK_STACK_SIZE     2048
#define UART_SAMPLE_TASK_PRIORITY       32
#define UART_SAMPLE_BUF_SIZE            2048
#define UART_SAMPLE_BANDRATE            115200

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
aos_thread_id uart_sample_thread_id;
char __attribute__ ((aligned(32))) uart_sample_buf[UART_SAMPLE_BUF_SIZE];

#define UART_LOOP_BACK_HARD     0

/**---------------------------------------------------------------------------*
 ** function
 **---------------------------------------------------------------------------*/

#if IS_CONF_ENABLED(UART_LOOP_BACK_HARD)
/**
* uart_hard_loopback_test - hard loop back
*
* Parameter:
*
* Return:
*
*/
void uart_hard_loopback_test(void)
{
    u32 *serial_handle;

    serial_handle = uart_open(UART_SAMPLE_COM);
    uart_set_baud_rate(serial_handle, 115200);
    uart_mode_set(serial_handle, UART_REMOTE_LOOPBACK_MODE);
}
#endif

/**
* uart_sample_entry - uart sample task.
*
* Parameter:
*   @param: task param.
*
* Return:
*
*/
static void uart_sample_entry(void *param)
{
    u32 *serial_handle;
    size_t rsize;
    u8 *ptr;

#if IS_CONF_ENABLED(UART_LOOP_BACK_HARD)
    uart_hard_loopback_test();
    return;
#endif

    ptr = uart_sample_buf;
    aos_memset(uart_sample_buf, 0, UART_SAMPLE_BUF_SIZE);

    /**
     * 1.Com number is aic_uart array index, aic_uart at aicos\projects\xxx\config\serial\serial_config.h;
     * 2.use aic_uart[index] parameter to set serial.
     */
    serial_handle = uart_open(UART_SAMPLE_COM);

    /* change baudrate */
    uart_set_baud_rate(serial_handle, UART_SAMPLE_BANDRATE);
    drv_logi("uart_sample: change baudrate(%d)!\n", UART_SAMPLE_BANDRATE);

    while(1) {
        /* read data. */
        rsize = uart_read(serial_handle, ptr, UART_SAMPLE_BUF_SIZE);
        drv_logi("uart_sample: read size = (%d)!\n", rsize);

        /* write data. */
        rsize = uart_write(serial_handle, ptr, rsize);
        drv_logi("uart_sample: write size = (%d)!\n", rsize);
    }

    /* close uart. */
    uart_close(serial_handle);
}
#endif /* UART_LOOP_BACK_SW */

/**
* uart_sample_init - uart sample init.
*
* Return:
*   AIC_SUCCESS is pass, other fail.
*/
int uart_sample_init(void)
{
#if IS_CONF_ENABLED(UART_LOOP_BACK_SW)
    uart_queue_id = aos_create_queue("uart_loop_queue",
                                     sizeof(uart_data_t *), LOOP_QUEUE_NUM);
    aic_assert(uart_queue_id);

    uart_recv_thread_id = aos_create_thread("uart_recv_task",
                                              uart_recv_entry, NULL,
                                              SIZE_2K,
                                              0, AIC_NULL,
                                              READ_THREAD_PRI,
                                              AOS_AUTO_START);
    aic_assert(AOS_INVALID_THREAD_ID != uart_recv_thread_id);

    uart_send_thread_id = aos_create_thread("uart_send_task",
                                              uart_send_entry, NULL,
                                              SIZE_2K,
                                              0, AIC_NULL,
                                              WRITE_THREAD_PRI,
                                              AOS_AUTO_START);
    aic_assert(AOS_INVALID_THREAD_ID != uart_send_thread_id);
#else
    uart_sample_thread_id = aos_create_thread("uart_sample_task",
                                              uart_sample_entry, NULL,
                                              UART_SAMPLE_TASK_STACK_SIZE,
                                              0, AIC_NULL,
                                              UART_SAMPLE_TASK_PRIORITY,
                                              AOS_AUTO_START);
    aic_assert(AOS_INVALID_THREAD_ID != uart_sample_thread_id);
#endif /* UART_LOOP_BACK_TEST */

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P1_INIT_EXPORT(uart_sample_init);

