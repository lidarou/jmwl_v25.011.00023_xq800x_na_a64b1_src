/******************************************************************************/
/*                                                                            */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines sdmmc common Interface.
 */

#ifndef __SDMMC_COMMON_H__
#define __SDMMC_COMMON_H__

#include "sdhost_global.h"

#define BUFFER_SIZE              SD_HOST_BLOCK_SIZE*8
#define DESC_SIZE                1024
// Pattern type
#define RND_TYPE_CONST           0 /* constant value */
#define RND_TYPE_INC_B           1 /* byte increment */
#define RND_TYPE_INC_W           2 /* word increment */
#define RND_TYPE_RANDOM          3 /* random value   */

#define sd_printf                alog_info
#define sd_delay                 aic_udelay

/* DEV_TYPE */
typedef enum {
    DEV_SD = 0,
    DEV_SDIO,
    DEV_EMMC
} DEV_TYPE;

/* BUS_WIDTH */
typedef enum {
    BUS_WIDTH_1 = 1,
    BUS_WIDTH_4 = 4,
    BUS_WIDTH_8 = 8
} BUS_WIDTH;

/* SDCARD_SPEED */
typedef enum {
    SDCARD_SPEED_DS = 0,
    SDCARD_SPEED_HS,
    SDCARD_SPEED_UHS_I
    /* SDCARD_SPEED_UHS_II not support */
} SDCARD_SPEED;

/* EMMC_SPEED */
typedef enum {
    EMMC_SPEED_SDR_BC = 0,
    EMMC_SPEED_SDR,
    EMMC_SPEED_DDR,
    EMMC_SPEED_HS200,
    EMMC_SPEED_HS400,
    EMMC_SPEED_HS400ES
} EMMC_SPEED;

/* UHS-I MODE */
typedef enum {
    UHSI_SDR12 = 0,
    UHSI_SDR25,
    UHSI_SDR50,
    UHSI_SDR104,
    UHSI_DDR50,
} UHSI_MODE;

/* DMA_MODE */
typedef enum {
    DMA_MODE_NONE = 0,
    DMA_MODE_SDMA,
    DMA_MODE_ADMA,  /* ADMA is obsolete, NOT SUPPORT */
    DMA_MODE_ADMA2,
    DMA_MODE_ADMA3  /* ADMA3 not support */
} DMA_MODE;

/* SDMMC_VOLTAGE */
typedef enum {
    VOLTAGE_1_8 = 0,
    VOLTAGE_3_3
} SDMMC_VOLTAGE;

extern volatile int sd_isr_counter;
extern volatile int emmc_isr_counter;


/* common test buffer for SD/MMC */
extern __attribute__((aligned(32))) uint8_t read_buffer[BUFFER_SIZE];
extern __attribute__((aligned(32))) uint8_t write_buffer[BUFFER_SIZE];
extern __attribute__((aligned(32))) uint8_t desc_buffer[DESC_SIZE];

unsigned char get_next_random_value(unsigned char type);
void fill_buffer(unsigned char* buff, unsigned long cnt,
                 unsigned char type, unsigned char seed);
void show_buffer(unsigned char* buff, unsigned long cnt);
unsigned long compare_buffers(unsigned char* buff_a, unsigned char* buff_b,
                              unsigned long cnt);
unsigned long check_buffers(unsigned char* buff_a, unsigned char* buff_b,
                            unsigned long cnt);
int sdmmc_test_init();

#endif // __SDMMC_COMMON_H__
