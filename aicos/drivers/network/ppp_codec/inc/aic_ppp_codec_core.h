#ifndef __AIC_PPP_CODEC_CORE_H__
#define __AIC_PPP_CODEC_CORE_H__

#include <aic_type.h>
#include <aic_io_dev.h>

#include "nx_ppp.h"
#include "ppp_codec_reg.h"

#define PPP_DECODER_SWAP_THRESHOLD      4

#define PPP_CODEC_PACKET_OFFSET         64

#define PPP_CODEC_CACHE_LINE_SIZE       32

#define CACHE_LINE_OFFSET(addr)         (((u32)(addr) & 0x1F) ? \
                                         (0x20 - (((u32)(addr)) & 0x1F)) : 0)

#define CACHE_LINE_LEFT_OFFSET(addr)    (((u32)(addr)) - (((u32)(addr)) & (~0x1F)))

#define PPP_GET_CACHE_LINE_ADDR(addr)   (((u32)(addr)) & (~0x1F))

#define PPP_CALC_CACHE_LINE_SIZE(addr, len) DCLINE_GRANULARITY(addr, len)

#define PPP_ENCODE_FIFO_THRESHOLD       8
#define PPP_ENCODE_FIFO_TIMEOUT         1000

#define PPP_DECODE_FIFO_THRESHOLD       8
#define PPP_DECODE_FIFO_TIMEOUT         1000

#define PPP_MAX_DATA_LEN                1508
#define PPP_MAX_RAW_DATA_LEN            3012

#define PPP_ENCODE_SEND_EVENT           0x01
#define PPP_DECODE_RECV_EVENT           0x02
#define PPP_ENCODE_POWERON_EVENT        0x04

#define PPP_HEAP_LOW_WATER              25000

#define AIC_PPP_DECODE_MASK             0xFFFF
#define AIC_PPP_DECODE_MAX_BUF          256

struct aic_ppp_decode_buf {
    char *buff;
    u32 size;
};

struct aic_ppp_codec_driver {
    NX_PPP *nx_ppp;

    u8 is_compress;
    u8 start_flag;
    u8 power_on;
    int encode_used_cnt;
    int decode_used_cnt;

    aos_thread_id send_thread_id;
    aos_thread_id recv_thread_id;

    struct aic_ppp_codec_phy_ops ppp_encode_phy;
    struct aic_ppp_codec_phy_ops ppp_decode_phy;

    void *ppp_encoder_in_fifo_addr;
    void *ppp_encoder_out_fifo_addr;
    void *ppp_decoder_in_fifo_addr;
    void *ppp_decoder_out_fifo_addr;

    u8 need_cache;
    u32 swap_size;
    u8 *cache_buf;
    u32 cache_size;

    void *last_addr;
    u32 comm_handle;

    aos_event_ptr encode_event;
    NX_PACKET *encode_packet_head;
    NX_PACKET *encode_packet_last;

    NX_PACKET *decode_receive_partial_packet;

    u32 iodev_type;
    u16 ppp_decode_buf_wptr;
    u16 ppp_decode_buf_rptr;
    struct aic_ppp_decode_buf ppp_decode_buf[AIC_PPP_DECODE_MAX_BUF];

    u32 encoder_flow_ctrl_cnt;
    u32 decoder_flow_ctrl_cnt;

    u32 encoder_err_cnt;
};

int aic_ppp_codec_register(NX_PPP *ppp_ptr);

int aic_ppp_codec_unregister(void);

u16 aic_ppp_encode_get_out_fifo_filled_depth(void);

int aic_ppp_encode_get_packet(u16 index, u8 **addr,
                              u16 *out_len, u32 *offset);

int aic_ppp_encode_update_out_fifo_rptr(u16 num);

int aic_ppp_encode_transmit_packet(NX_PACKET *packet_ptr, u16 index);

int aic_ppp_encode_update_in_fifo_wptr(u16 num);

u16 aic_ppp_encode_get_in_fifo_filled_depth(void);

u16 aic_ppp_decode_get_out_fifo_filled_depth(void);

int aic_ppp_decode_get_packet(void);

int aic_ppp_decode_update_out_fifo_rptr(u16 num);

int aic_ppp_decode_transmit_packet(void *packet_ptr, u16 offset, u16 len);

int aic_ppp_decode_update_in_fifo_wptr(u16 num);

u16 aic_ppp_decode_get_in_fifo_filled_depth(void);

int aic_ppp_codec_stop(void);
void aic_ppp_codec_set_compress(u8 compress);

void aic_ppp_codec_start_process(u32 com);
void aic_ppp_codec_stop_process(void);

#endif
