/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


//#include <config.h>

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05d16b
#define LOG_MODULE    AMOD_VENDOR
#include <stdio.h>
//#include <errno.h>
#include <string.h>
#include "porting.h"
#include "log.h"

#include <sdio_def.h>
#include <sdio_port.h>
#include <sdmmc_card.h>
#include <sdio.h>
#include <irq_chip.h>
#include <aic_gpio_pin.h>
#include <core_star.h>
//#include "gpio_drvapi.h"
//#include "gpio_prod_api.h"
//#include "sfs.h"
//#include "sdio_io.h"

#define SDIOM_MAX_FUNCS 1

int32_t sdio_wlan_probe(sdmmc_card *card);
int32_t sdio_wlan_remove(sdmmc_card *card);

typedef  sdmmc_card  card_handle;
typedef struct sdio_func{
	card_handle *card;
	uint8    num;
	sdio_function  *sdio_function;
}sdio_card;
sdio_card g_wlan_card;




static sdio_device_id   g_wlan_device={

    .func_code=SDIO_FUNC_CODE_WLAN,
    .manufacturer=SDIO_MANUFACTURER_ID_SSV,
    .product=	SDIO_PRODUCT_ID_SSV_6158M ,
};
_sdio_driver g_wlan_driver={
	.name = "6158M",
	.probe = sdio_wlan_probe,
	.remove = sdio_wlan_remove,
	.id = &g_wlan_device,
	

};


#define IRQF_TRIGGER_HIGH  1
#define IRQF_TRIGGER_LOW  2
#define FUNC_0  0
#define FUNC_1  1
#define SDIO_BLK_SIZE	512 //512
//#define SDIO_PORT_DEBUG
#define SDIO_DT_MODE_ADDR	0x0f
#define SDIO_FBR_SYSADDR_0	0x15c
//cache
#if 0
#define SYS_CACHE_LINE_SIZE  64U //64U
#define WCN_CACHE_ALIGNED(addr) (((ssv_type_u32)(addr) + (SYS_CACHE_LINE_SIZE - 1)) & (~(SYS_CACHE_LINE_SIZE - 1)))
#define WCN_CACHE_ALIGN_VARIABLE    __align(SYS_CACHE_LINE_SIZE)
#define IS_WCN_CACHE_ALIGNED(addr) !((ssv_type_u32)(addr) & (SYS_CACHE_LINE_SIZE - 1))
WCN_CACHE_ALIGN_VARIABLE static ssv_type_u32 sdiom_cache_align_addr[SYS_CACHE_LINE_SIZE/sizeof(ssv_type_u32)];
WCN_CACHE_ALIGN_VARIABLE static ssv_type_u32 sdiom_cache_align_data[SYS_CACHE_LINE_SIZE/sizeof(ssv_type_u32)];
#endif 

static unsigned int sdio_gpio_num = GPIO_AON_8;//pin 8 D1  
static ssv_type_u32 sdio_block_size;
typedef void (*SDIO_ISR_FUNC)(void);

SDIO_ISR_FUNC g_sdio_isr_func = NULL;
static void* port_dma_buf=NULL;

ssv_type_bool sdio_readb_cmd52(ssv_type_u32 addr, ssv_type_u8 *data)
{
      int err;
	ssv_type_u8  val;

	//LOG_PRINTF("sdio_nov:sdio_nov: sdio_readb_cmd52, addr: 0x%x", addr);

	val = sdio_io_readb(g_wlan_card.card->sdio_function[FUNC_1],addr, &err);
	if(err){
		LOG_PRINTF("sdio_nov:sdio_readb_cmd52 fail0!\n");
		
		return FALSE;
	}

	
    *data = val;
	
    	return TRUE;
}

ssv_type_bool sdio_writeb_cmd52(ssv_type_u32 addr, ssv_type_u8 data)
{
	int err;
#if 0	
	if (!g_wlan_card.sdio_function[FUNC_0]){

		LOG_PRINTF("sdio_nov: sdio card null fail0!\n");
		return FALSE;
	}
#endif 		
		
	err=sdio_io_writeb(g_wlan_card.card->sdio_function[FUNC_1], addr, data);
	if(err ){
		LOG_PRINTF("sdio_nov:sdio_writeb_cmd52 fail1!\n");
		
		return FALSE;
	}
	

    //LOG_PRINTF("sdio_nov:sdio_nov: sdio_writeb_cmd52, addr 0x%x, data=0x%x", addr, data);
    
	return TRUE;
}

ssv_type_bool sdio_read_cmd53(ssv_type_u32 dataPort, ssv_type_u8 *dat, size_t size)
{
	int ret = 0;
	ssv_type_u32 rx_blocks = 0, blksize = 0;
	//char* temp_addr, *temp_addr2;
    //static int init_malloc = 0;
	//sdiom_cache_align_addr[0] = dataPort;
    ssv_type_u32 gSize;
    if (size > sdio_block_size) 
    {
        rx_blocks = (size + sdio_block_size - 1)/sdio_block_size;//sdio_block_size;
        blksize = sdio_block_size;
    } 
    else 
    {
        blksize = size;
        rx_blocks = 1;
    }
    gSize   =rx_blocks*blksize;

  
	ret= sdio_io_read_multi_fifo_b(g_wlan_card.card->sdio_function[FUNC_1], //FUNC_1
                                     dataPort,
                                     dat,
                                    gSize );
	if(ret){
		LOG_PRINTF("sdio_read_cmd53 size = %d, fail!\n", size);
		return FALSE;
	}
	//LOG_PRINTF("sdio_nov:sdio_read_cmd53 12, dat0=0x%x, dat2=0x%x, dat3=0x%x, dat4=0x%x", dat[0], dat[1], dat[2], dat[3]);
	return TRUE;
}

ssv_type_bool sdio_write_cmd53(ssv_type_u32 dataPort, ssv_type_u8 *dat, size_t size)
{
    int ret = 0;
	//char* temp_addr, *temp_addr2;
    ssv_type_u32 tx_blocks = 0, blksize = 0;
    ssv_type_s32 gSize;

	//sdiom_cache_align_addr[0] = dataPort;
  
    if (size > sdio_block_size) 
    {
        tx_blocks = (size + sdio_block_size - 1) / sdio_block_size;
        blksize = sdio_block_size;
    }
    else 
    {
        blksize = size;
        tx_blocks = 1;
    }

 //   size = blksize * tx_blocks;
    gSize =blksize * tx_blocks;
  
	ret= sdio_io_write_multi_fifo_b(g_wlan_card.card->sdio_function[FUNC_1],//FUNC_1
                                     dataPort, //sdiom_cache_align_addr[0],  //
                                     dat,
                                      gSize);
    

	if(ret){
		LOG_PRINTF("sdio_write_cmd53 size = %d, fail!!\n", size);
		return FALSE;
	}

	return TRUE;
}
#define DRV_Reg32(addr)               (*(volatile INT32U *)(addr))

static __inline int gpio_request_irq(ssv_type_u32 gpio_id, ssv_type_u32 trig_type,void (isr)(struct sdio_function_st * st) )
    //int (* isr)(ssv_type_u32, ssv_type_u32))
{

    ssv_type_s32 ret ;

	//aos_request_irq(IRQ_SDIOH,isr,NULL); // IRQ_AON_GPIO  IRQ_SDIOH
 
   ret= sdio_attach_irq(  g_wlan_card.card->sdio_function[FUNC_1],
                          isr);
     if(ret){
        LOG_PRINTF("sdio open-sido host init- sido attach irq fail \n");
        return FALSE;

     }                     
    return TRUE;
}
#if 0
 static AOS_INTERRUPT_DECLARE
static __inline void gpio_enable_irq(ssv_type_u32 gpio_id)
{
   // GPIO_EnableIntCtl(gpio_id);NVIC_DisableIRQ
//	NVIC_EnableIRQ(IRQ_SDIOH);
//    AOS_INTERRUPT_DISABLE
    AOS_INTERRUPT_RESTORE
}

static __inline void gpio_disable_irq(ssv_type_u32 gpio_id)
{
	//NVIC_DisableIRQ(IRQ_SDIOH);
    AOS_INTERRUPT_DISABLE
   // GPIO_DisableIntCtl(gpio_id);NVIC_EnableIRQ
}
#endif
ssv_type_bool sdio_host_enable_isr(ssv_type_bool enable)
{    
 #if 1
    if (enable)
      aos_sdio_irq_enable(g_wlan_card.card->sdio_function[FUNC_1],1);
    else 
      aos_sdio_irq_enable(g_wlan_card.card->sdio_function[FUNC_1],0);
 #endif
    return TRUE;
}

ssv_type_bool ssv_sdio_set_block_size(unsigned int blksize)
{
    unsigned char blk[2];
    //ssv_type_u8 in, out;
    int err_ret = 0;
    int err_ret1 = 0, err_ret2 = 0;
    
    if ((blksize == 0) || (blksize > 512))
    {
        blksize = SDIO_DEFAULT_BLOCK_SIZE;
    }

    blk[0] = blksize & 0x0ff;
    blk[1] = (blksize >> 8)&0x0ff;

   // sdio_claim_host(sdio_function[FUNC_0]);
   err_ret=sdio_io_writeb(g_wlan_card.card->sdio_function[FUNC_1], 0x110, blk[0]);
//	sdio_writeb(g_wlan_card.card->sdio_function[FUNC_0], blk[0], 0x110, &err_ret);
	if(err_ret ){
		LOG_PRINTF("sdio_nov:ssv_sdio_set_block_size fail0!\n");
		
		return FALSE;
	}

   err_ret= sdio_io_writeb(g_wlan_card.card->sdio_function[FUNC_1],0x111, blk[1]);
	if(err_ret ){
		LOG_PRINTF("sdio_nov:ssv_sdio_set_block_size fail0!\n");
	//	sdio_release_host(sdio_function[FUNC_0]);
		return FALSE;
	}

    blk[0] = 0;
    blk[1] = 0;
	err_ret1 = sdio_io_readb(g_wlan_card.card->sdio_function[FUNC_1], 0x110, &err_ret);
	if(err_ret ){
		LOG_PRINTF("sdio_nov:sdio_readb 0x110 fail0!\n");
	
		return FALSE;
	}

    LOG_PRINTF("sdio_nov:sdio_readb 0x110 = %x\n", err_ret1);

	err_ret2 = sdio_io_readb(g_wlan_card.card->sdio_function[FUNC_1], 0x111, &err_ret);
	if(err_ret ){
		LOG_PRINTF("sdio_nov:sdio_readb 0x111 fail0!\n");

		return FALSE;
	}
    
    LOG_PRINTF("sdio_nov:sdio_readb 0x111 = %x\n", err_ret2);
//	sdio_release_host(sdio_function[FUNC_0]);

    LOG_PRINTF("ssv_sdio_set_block_size blksize = %d\n", blksize);
    
    sdio_block_size = blksize;
    return TRUE;
    
}

ssv_type_u32 sdio_get_block_size(void)
{
    return sdio_block_size;
}

ssv_type_bool sdio_set_clk(ssv_type_u32 clk)
{
    return TRUE;
}

ssv_type_bool sdio_set_bwidth(ssv_type_u8 bwidth)
{
    //if(SDIO_BUS_1_BIT == bwidth)
    //    return sdio_set_bus_width(USE_ONE_BUS);

    //if(SDIO_BUS_4_BIT == bwidth)
    //    return sdio_set_bus_width(USE_FOUR_BUS);
    return TRUE;
}
extern ssv_type_u32 sdio_host_isr_count;
#if 0
void sdio_host_isr (ssv_type_u32 gpio_id, ssv_type_u32 gpio_state)
{
    //if (SDHCI_INTR_STS_CARD_INTR & arg)
	sdio_host_isr_count++;
    {
        sdio_host_enable_isr(FALSE);
        if (g_sdio_isr_func)
        {
            g_sdio_isr_func();
        }
    }
}
#endif
 void sdio_host_isr(struct sdio_function_st * st)
{

    sdio_host_isr_count++;
    {
        sdio_host_enable_isr(FALSE); //sentton
        if (g_sdio_isr_func)
        {
            g_sdio_isr_func();
        }
    }
 }
ssv_type_bool is_truly_isr(void)
{
    return TRUE;
}

static int sdio_interrupt_init(void)
{
	int ret;

	LOG_PRINTF("sdio_host_init:sdiom_gpio_num: %u\n", sdio_gpio_num);

	
	ret = gpio_request_irq(sdio_gpio_num, IRQF_TRIGGER_HIGH, sdio_host_isr);
	if (ret != TRUE) {
		LOG_PRINTF("sdio_host_init:request irq err!!!gpio is %d!!!", sdio_gpio_num);
		return -1;
	}

	return 0;
}
int32_t sdio_wlan_probe(sdmmc_card *card)
{
	 sdmmc_host * sdio_host;
	 if (card==NULL)
	 	return -1;
	sdio_host = card->host;
	LOG_PRINTF("sdio wlan probe card_type %d  \n",
				card->card_type);
				 
	g_wlan_card.card=card;
	g_wlan_card.num = 1; //only one sdio card
	g_wlan_card.sdio_function =(sdio_function *)card->sdio_function;
	//card->sdio_function_num;
	sdio_enable_func(card->sdio_function[FUNC_1]);

 //sdio_disable_func(card->sdio_function[g_wlan_card.num-1]);

     
	  ssv_sdio_set_block_size(SDIO_BLK_SIZE);//init wlan0

      sdio_set_block_size(card->sdio_function[FUNC_1],SDIO_BLK_SIZE);  

	   LOG_PRINTF("sdio_host_init: blk size=%d\r\n", sdio_block_size);

	  sdio_interrupt_init();
	  sdio_host_enable_isr(FALSE);

	  if (sdio_writeb_cmd52(REG_OUTPUT_TIMING_REG, SDIO_DEF_OUTPUT_TIMING))
    {
        //   SDIO_FAIL_RET(0, "sdio_write_byte(0x55, %d)\n", SDIO_DEF_OUTPUT_TIMING);
        LOG_PRINTF("sdio_host_init:output timing to %d blk size=%d\r\n", SDIO_DEF_OUTPUT_TIMING, sdio_block_size);
    }

	  return 0;
	  

}
int32_t sdio_wlan_remove(sdmmc_card *card)
{
	return sdio_disable_func(card->sdio_function[FUNC_1]);
}
ssv_type_bool sdio_host_init(void (*sdio_isr)(void))
{
    //ssv_type_u32 ret = 0;
    //ssv_type_u8 in;
    int err_ret = 0;
    //int i =0;

	//sdio_open();
	//init_sdio
//	ret=ssv_sdio_init();
	//g_wlan_driver
	g_sdio_isr_func = sdio_isr;
	
	err_ret =  sdio_register_driver(&g_wlan_driver);  //register driver
	if (err_ret  ){

		LOG_PRINTF("==sentton wlan sdio prob fail\n");
		return FALSE;
	}

   

    LOG_PRINTF("sdio_host_init:%-20s : %s\n", "host_int", "ok");
    
    return TRUE;
}

ssv_type_bool sdio_host_deinit(void)
{
    //int ret = 0;
    
    LOG_PRINTF("sdio_host_init: %s() => :%d\r\n",__FUNCTION__,__LINE__);

    sdio_host_enable_isr(0);

    return TRUE;
}


ssv_type_bool sdio_host_detect_card(void)
{
   return TRUE;
}

void* sdio_allocate_dma_buffer(ssv_type_u32 buf_size)
{
    if(!port_dma_buf)
    {
        port_dma_buf = malloc(buf_size);
        LOG_PRINTF("sdio_host_init:sdio dma buf=0x%x,size=%d\r\n",port_dma_buf,buf_size);
        if((unsigned int)port_dma_buf & 3)
        {
            LOG_PRINTF("sdio_host_init:port_dma_buf is not align 4 byte 0x%x\r\n",port_dma_buf);
        }

    }
    return port_dma_buf;
}

void sdio_free_dma_buffer(void* buf_ptr)
{
    if(!buf_ptr)
    {
        if(port_dma_buf!=buf_ptr)
        {
            LOG_PRINTF("sdio_host_init:not dma buf(0x%x,0x%x)\r\n",port_dma_buf,buf_ptr);
            return;
        }
        free(port_dma_buf);
        port_dma_buf = NULL;
        LOG_PRINTF("sdio_host_init:sdio_free_dma_buffer\r\n");
    }
}

