/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implement the sdmmc common function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x08cc98
#define LOG_MODULE    AMOD_SDMMC

#include <stdio.h>
#include <stdint.h>
#include "sdmmc_common.h"
#include <mcu_glb.h>
#include <aic_errno.h>

/**---------------------------------------------------------------------------*/
/**                            macros define                                  */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*/
/**                           global variables                                */
/**---------------------------------------------------------------------------*/
#if 0
__attribute__((aligned(32))) uint8_t read_buffer[BUFFER_SIZE];
__attribute__((aligned(32))) uint8_t write_buffer[BUFFER_SIZE];
__attribute__((aligned(32))) uint8_t desc_buffer[DESC_SIZE];
#endif

/**---------------------------------------------------------------------------*/
/**                               functions                                   */
/**---------------------------------------------------------------------------*/
// static char  rnd_last_val;
static uint32_t  rnd_last_val = 0;
static uint8_t   rnd_offset   = 0; /* used in INC_W mode only*/

unsigned char get_next_random_value(unsigned char type)
{
    uint8_t _ret;

    switch (type) {
    case RND_TYPE_CONST:
        _ret = rnd_last_val & 0xff;
        break;
    case RND_TYPE_INC_B: /* Increment in range of one byte */
        _ret = rnd_last_val & 0xff;
        rnd_last_val = (rnd_last_val + 1) & 0xff;
        break;
    case RND_TYPE_INC_W: /* Increment in range of one word */
        _ret = ((rnd_last_val >> (8*rnd_offset)) & 0xff);
        if (rnd_offset == 3) rnd_last_val += 1;
        rnd_offset = (rnd_offset+1) & 0x03;
        break;
    default:    /* Generate random value */
        rnd_last_val = (rnd_last_val * 156795);
        _ret = rnd_last_val & 0xff;
        break;
    }
    return _ret;
}

void fill_buffer(unsigned char* buff, unsigned long cnt,
                 unsigned char type, unsigned char seed)
{
    unsigned long i;
    rnd_last_val = seed;
    for (i=0;i<cnt;i++)
        buff[i] = get_next_random_value(type);
}

void show_buffer(unsigned char* buff, unsigned long cnt)
{
    int i, j;

    for (i=0; i<cnt/8; i++) {
        sd_printf("Buff [%4d]: ", i*8);

    for (j=0; j<8; j++)
        sd_printf("0x%02x ", buff[i*8+j]);
        sd_printf("\r\n");
    }

    sd_printf("\r\n");
}

unsigned long compare_buffers(unsigned char* buff_wr,
                              unsigned char* buff_rd,
                              unsigned long cnt)
{
    unsigned long _err_cnt =0;
    unsigned int i;

    for (i=0;i<cnt;i++) {
    if (buff_wr[i] != buff_rd[i])
        if (_err_cnt++ < 20) {
            sd_printf("ERROR at byte 0x%x, buff_wr = 0x%x @0x%x, buff_rd = 0x%x @0x%x\r\n",i, buff_wr[i], &buff_wr[i], buff_rd[i], &buff_rd[i]);
        }
    }
    return _err_cnt;
}


unsigned long check_buffers(unsigned char* buff_a,
                            unsigned char* buff_b,
                            unsigned long cnt)
{
    unsigned long i;

    for (i=0;i<cnt;i++) {
        buff_a[i] = (i&0xff);
        buff_b[i] = (i&0xff);
    }

    return compare_buffers(buff_a, buff_b, cnt);
}

/**---------------------------------------------------------------------------*/
/**                        sdemmc common test cases                           */
/**---------------------------------------------------------------------------*/
#if 0
int sdmmc_test_init()
{
    static boolean init_flag = FALSE;

    if (init_flag)
        return AIC_SUCCESS;

    sd_printf("Initialization process\n");

    /* enable sdhc clk */
    AIC_WRITE32(REG_MCU_GLB_SDIOH_EB, BIT_MCU_GLB_SDIOH_EB);

    init_flag = TRUE;

    return AIC_SUCCESS;
}
#endif

