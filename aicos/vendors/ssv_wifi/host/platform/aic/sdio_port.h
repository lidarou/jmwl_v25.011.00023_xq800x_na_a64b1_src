/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


#ifndef _SDIO_PORT_H_
#define _SDIO_PORT_H_

#define SDIO_MANUFACTURER_ID_SSV 0x5653
#define SDIO_PRODUCT_ID_SSV_6158M 0x2060

//output timing
// 0: cmd  [0]:positive [1]:negative
// 1: data [0]:positive [1]:negative
#define SDIO_DEF_OUTPUT_TIMING 		0 // 3
#define SDIO_DEFAULT_BLOCK_SIZE  64 //256
extern ssv_type_bool sdio_readb_cmd52(ssv_type_u32 addr, ssv_type_u8 *data);
extern ssv_type_bool sdio_writeb_cmd52(ssv_type_u32 addr, ssv_type_u8 data);
extern ssv_type_bool sdio_write_cmd53(ssv_type_u32 dataPort,ssv_type_u8 *dat, size_t size);
extern ssv_type_bool sdio_read_cmd53(ssv_type_u32 dataPort,ssv_type_u8 *dat, size_t size);
extern ssv_type_bool _sdio_read_reg(ssv_type_u32 addr, ssv_type_u32 *data);
extern ssv_type_bool ssv_sdio_set_block_size(unsigned int blksize);
extern ssv_type_u32 sdio_get_block_size(void);
extern ssv_type_bool sdio_host_init(void (*sdio_isr)(void));
extern ssv_type_bool sdio_host_enable_isr(ssv_type_bool enable);
extern ssv_type_bool is_truly_isr(void);
void* sdio_allocate_dma_buffer(ssv_type_u32 buf_size);
void sdio_free_dma_buffer(void* buf_ptr);
ssv_type_bool sdio_host_detect_card(void);
ssv_type_bool sdio_set_bwidth(ssv_type_u8 bwidth);
ssv_type_bool sdio_set_clk(ssv_type_u32 clk);
#endif /* _SDIO_H_ */

