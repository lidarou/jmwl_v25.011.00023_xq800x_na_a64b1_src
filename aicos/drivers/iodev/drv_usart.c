/******************************************************************************/
/*                                                                             */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                             */
 /******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    temp use drv_usart.c befor uart driver ready.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x052eef
#define LOG_MODULE    AMOD_DRV

#include <aic_assert.h>
#include <aic_errno.h>
#include "drv_usart.h"

#ifdef CONFIG_AIC_CHIP_FORLINX
extern int DbgConsole_SendDataReliable(uint8_t *ch, size_t size);
extern int DbgConsole_Getchar(void);
#endif

static u32 s_uart_dummy;
void *uart_open(int com)
{
    return &s_uart_dummy;
}

void uart_close(void *handle)
{

}

size_t uart_read(void *handle, void *buffer, size_t size)
{
#ifdef CONFIG_AIC_CHIP_FORLINX
    int ch;
    size_t read;
    char *p = buffer;

    for (read = 0; read < size; read++) {
        ch = DbgConsole_Getchar();
        if (ch < 0)
            break;
        p[read] = ch;
    }

    return read;
#endif
    return 0;
}

size_t uart_write(void *handle, const void *buffer, size_t size)
{
#ifdef CONFIG_AIC_CHIP_FORLINX
    return (size_t)DbgConsole_SendDataReliable((uint8_t *)buffer, size);
#endif
    return size;
}

void uart_set_baud_rate(void *handle, int rate)
{

}

size_t uart_write_dir(void *handle, const void *buffer, size_t size)
{
    return size;
}

