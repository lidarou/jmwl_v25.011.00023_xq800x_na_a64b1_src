
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x07fb81
#define LOG_MODULE    AMOD_DRV
#include <aic_cmd.h>
#include <aic_init.h>
#include <aic_io.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <aic_packet_management.h>
#include <aos_config.h>
#include <connmgr_api.h>
#include <cache.h>
#include <slp_inc.h>

#include "aic_ppp_codec_core.h"

#define PPP_ENCODE 1
#define PPP_DECODE 2

static struct aic_ppp_codec_driver aic_ppp_codec_drv_cfg;

static int aic_ppp_decode_buf_is_empty(void)
{
    int empty = 0;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->ppp_decode_buf_wptr == cfg->ppp_decode_buf_rptr)
        empty = TRUE;

    return empty;
}

static int aic_ppp_decode_insert_buf(char *buf, size_t size)
{
    int notify = 0;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->ppp_decode_buf_wptr == cfg->ppp_decode_buf_rptr ||
        (((cfg->ppp_decode_buf_wptr - cfg->ppp_decode_buf_rptr) &
          AIC_PPP_DECODE_MASK) == (AIC_PPP_DECODE_MAX_BUF >> 3)) ||
        (((cfg->ppp_decode_buf_wptr - cfg->ppp_decode_buf_rptr) &
          AIC_PPP_DECODE_MASK) == (AIC_PPP_DECODE_MAX_BUF >> 2)) ||
        (((cfg->ppp_decode_buf_wptr - cfg->ppp_decode_buf_rptr) &
          AIC_PPP_DECODE_MASK) == (AIC_PPP_DECODE_MAX_BUF >> 1)))
        notify = TRUE;

    if (((cfg->ppp_decode_buf_wptr - cfg->ppp_decode_buf_rptr) &
         AIC_PPP_DECODE_MASK) == AIC_PPP_DECODE_MAX_BUF) {
        return AIC_ENOMEM;
    }

    cfg->ppp_decode_buf[cfg->ppp_decode_buf_wptr & (AIC_PPP_DECODE_MAX_BUF - 1)].buff = buf;
    cfg->ppp_decode_buf[cfg->ppp_decode_buf_wptr & (AIC_PPP_DECODE_MAX_BUF - 1)].size = size;
    cfg->ppp_decode_buf_wptr = (cfg->ppp_decode_buf_wptr + 1) & AIC_PPP_DECODE_MASK;

    if (notify)
        return 1;

    return AIC_SUCCESS;
}

static char *aic_ppp_decode_get_buf(u32 *size)
{
    char *buf = NULL;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->ppp_decode_buf_wptr == cfg->ppp_decode_buf_rptr) {
        return NULL;
    }

    buf = cfg->ppp_decode_buf[cfg->ppp_decode_buf_rptr & (AIC_PPP_DECODE_MAX_BUF - 1)].buff;
    if (size)
        *size = cfg->ppp_decode_buf[cfg->ppp_decode_buf_rptr & (AIC_PPP_DECODE_MAX_BUF - 1)].size;
    cfg->ppp_decode_buf_rptr = (cfg->ppp_decode_buf_rptr + 1) & AIC_PPP_DECODE_MASK;

    return buf;
}

static char *aic_ppp_decode_get_last_buf(u32 *size)
{
    char *buf = NULL;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->ppp_decode_buf_wptr == cfg->ppp_decode_buf_rptr) {
        return NULL;
    }

    buf = cfg->ppp_decode_buf[(cfg->ppp_decode_buf_wptr - 1) & (AIC_PPP_DECODE_MAX_BUF - 1)].buff;
    if (size)
        *size = cfg->ppp_decode_buf[(cfg->ppp_decode_buf_wptr - 1) & (AIC_PPP_DECODE_MAX_BUF - 1)].size;

    return buf;
}

static void aic_ppp_decode_update_last_buf_size(u32 size)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->ppp_decode_buf_wptr == cfg->ppp_decode_buf_rptr) {
        return;
    }

    cfg->ppp_decode_buf[(cfg->ppp_decode_buf_wptr - 1) & (AIC_PPP_DECODE_MAX_BUF - 1)].size = size;
}

static ISR_RET_E aic_ppp_decoder_isr_func(void *param)
{
    u32 val = 0, clear = 0;
    struct aic_ppp_codec_driver *cfg = param;

    val = cfg->ppp_decode_phy.get_irq_status();
    if (val & AIC_PPP_THRESHOLD_INTR)
        clear |= AIC_PPP_THRESHOLD_INTR;

    if (val & AIC_PPP_TIMEOUT_INTR)
        clear |= AIC_PPP_TIMEOUT_INTR;

    if (val & AIC_PPP_ERRCODE_INTR)
        clear |= AIC_PPP_ERRCODE_INTR;

    if (val & AIC_PPP_FLOW_CTRL_INTR) {
        cfg->decoder_flow_ctrl_cnt++;
        clear |= AIC_PPP_FLOW_CTRL_INTR;
    }

    if (val & AIC_PPP_CRC_ERROR_INTR)
        clear |= AIC_PPP_CRC_ERROR_INTR;

    cfg->ppp_decode_phy.clear_intr(clear);

    tx_event_flags_set(&cfg->nx_ppp->nx_ppp_event,
                       NX_PPP_EVENT_HW_OFFLOAD_RECEIVE,
                       TX_OR);

    return NONE_WORKER;
}

static ISR_RET_E aic_ppp_encoder_isr_func(void *param)
{
    u32 val = 0, clear = 0;
    struct aic_ppp_codec_driver *cfg = param;

    val = cfg->ppp_encode_phy.get_irq_status();
    if (val & AIC_PPP_THRESHOLD_INTR)
        clear |= AIC_PPP_THRESHOLD_INTR;

    if (val & AIC_PPP_TIMEOUT_INTR)
        clear |= AIC_PPP_TIMEOUT_INTR;

    if (val & AIC_PPP_ERRCODE_INTR)
        clear |= AIC_PPP_ERRCODE_INTR;

    if (val & AIC_PPP_FLOW_CTRL_INTR) {
        clear |= AIC_PPP_FLOW_CTRL_INTR;
        cfg->encoder_flow_ctrl_cnt++;
    }

    if (val & AIC_PPP_CRC_ERROR_INTR)
        clear |= AIC_PPP_CRC_ERROR_INTR;

    cfg->ppp_encode_phy.clear_intr(clear);

    aos_set_event(cfg->encode_event, PPP_ENCODE_SEND_EVENT, AOS_OR);

    return NONE_WORKER;
}

static void aic_ppp_add_used_cnt(u8 codec)
{
    AOS_INTERRUPT_DECLARE
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    AOS_INTERRUPT_DISABLE
    if (codec == PPP_ENCODE)
        cfg->encode_used_cnt++;
    else if (codec == PPP_DECODE)
        cfg->decode_used_cnt++;
    AOS_INTERRUPT_RESTORE
}

static void aic_ppp_dec_used_cnt(u8 codec)
{
    AOS_INTERRUPT_DECLARE
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    AOS_INTERRUPT_DISABLE
    if (codec == PPP_ENCODE)
        cfg->encode_used_cnt--;
    else if (codec == PPP_DECODE)
        cfg->decode_used_cnt--;

    if (cfg->encode_used_cnt < 0 || cfg->decode_used_cnt < 0)
        AIC_ASSERT(FALSE);
    AOS_INTERRUPT_RESTORE
}

#ifndef PPPD_DISABLE_DATA_SWAP
static int aic_ppp_swap_data(struct aic_ppp_codec_driver *cfg,
                             u8 *buf, size_t size)
{
    int i = 0;
    int cache_buf_size = PPP_DECODER_SWAP_THRESHOLD;

    if (*(buf + size - 1) == 0x7E)
        return AIC_SUCCESS;

    if (size == 1 && *buf == 0x7D) {
        i = 1;
        goto SWAP_OUT;
    }

    for (i = 0; i < PPP_DECODER_SWAP_THRESHOLD; i++) {
        if (*(buf + size - 1 - i) == 0x7E)
            break;
    }

    if (i >= PPP_DECODER_SWAP_THRESHOLD) {
        if ((*(buf + size - 1) == 0x5E) && (*(buf + size - 2) == 0x7d)) {
            aicx_info(0x035d03, "xdxx", "buf: 0x%x size: %d The last two bytes is 0x%x 0x%x\r\n",
                      buf, size, *(buf + size - 2), *(buf + size - 1));
            for (i = 0; i < size; i++) {
                if (*(buf + size - 1 - i) == 0x7E)
                    break;
            }

            cache_buf_size = PPP_MAX_RAW_DATA_LEN;
       } else {
            return AIC_SUCCESS;
       }
    } else {
        cache_buf_size = PPP_DECODER_SWAP_THRESHOLD;
    }

SWAP_OUT:
    if (cfg->cache_buf)
        AIC_ASSERT(FALSE);

    cfg->cache_buf = aos_dma_malloc(cache_buf_size);
    if (!cfg->cache_buf) {
        aicx_error(0x141e8f, "", "[PPP_DECODE] alloc cache buf fail\r\n");
        return AIC_ENOMEM;
    }

    memcpy(cfg->cache_buf, buf + size - i, i);

    cfg->need_cache = TRUE;
    cfg->swap_size = cache_buf_size - i;
    cfg->cache_size = i;

    return i;
}
#endif

static void aic_ppp_recv_task(u32 conn_id, u8 *buf, size_t size, void *p)
{
    int ret = 0, early_free = 0;
    u8 *last_buf;
    size_t last_buf_size;
    AOS_INTERRUPT_DECLARE
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (!cfg->start_flag || !size) {
        aicx_warn(0x1255a0, "d", "[PPP] ppp codec is stop when recv decode buf %d\n", size);
        aos_dma_free(buf);
        return;
    }

    AOS_INTERRUPT_DISABLE
    if ((size < 64) &&
        !aic_ppp_decode_buf_is_empty()) {
        last_buf = aic_ppp_decode_get_last_buf(&last_buf_size);
        if (last_buf && ((last_buf_size + size) < 2048)) {
            memcpy(last_buf + last_buf_size, buf, size);
            last_buf_size += size;
            aic_ppp_decode_update_last_buf_size(last_buf_size);
            early_free = TRUE;
        } else {
            ret = aic_ppp_decode_insert_buf(buf, size);
        }
    } else {
        ret = aic_ppp_decode_insert_buf(buf, size);
    }
    AOS_INTERRUPT_RESTORE

    if (early_free) {
        aos_dma_free(buf);
        return;
    }

    if (ret == 1) {
        aos_set_event(cfg->encode_event, PPP_DECODE_RECV_EVENT, AOS_OR);
    } else if (ret == AIC_ENOMEM) {
        aicx_info(0x1e54c3, "", "[PPP] decode buff is full\n");
        aos_dma_free(buf);
    }
}

static void aic_ppp_decode_recv(u8 *buf, size_t size)
{
    int ret;
    u16 offset = 0;
    size_t payload = 0;
#ifndef PPPD_DISABLE_DATA_SWAP
    u32 swap_size = 0, end_flag = 0;
#endif
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    aic_ppp_add_used_cnt(PPP_DECODE);

    if (cfg->start_flag) {
#ifndef PPPD_DISABLE_DATA_SWAP
        if (cfg->need_cache) {
            if (cfg->swap_size >= size) {
                swap_size = size;

                for (int i = 0; i < swap_size; i++) {
                    if (*(buf + i) == 0x7E) {
                        swap_size = i + 1;
                        end_flag = TRUE;
                        break;
                    }
                }

                if (end_flag) {
                    cfg->swap_size = 0;
                } else {
                    cfg->swap_size -= swap_size;
                }

                memcpy(cfg->cache_buf + cfg->cache_size, buf, swap_size);

                cfg->cache_size += swap_size;
                if (size > swap_size) {
                    size -= swap_size;
                    offset = swap_size;
                } else {
                    size = 0;
                }
            } else {
                swap_size = cfg->swap_size;

                for (int i = 0; i < swap_size; i++) {
                    if (*(buf + i) == 0x7E) {
                        swap_size = i + 1;
                        end_flag = TRUE;
                        break;
                    }
                }

                memcpy(cfg->cache_buf + cfg->cache_size, buf, swap_size);
                cfg->cache_size += swap_size;
                size -= swap_size;
                offset = swap_size;
                cfg->swap_size = 0;
            }

            if (!cfg->swap_size) {
                ret = aic_ppp_decode_transmit_packet(cfg->cache_buf, 0,
                                                     cfg->cache_size);
                if (ret)
                    aicx_warn(0x05cb76, "x", "ppp decode transmit packet fail 0x%x\r\n", ret);

                cfg->swap_size = 0;
                cfg->cache_size = 0;
                cfg->cache_buf = NULL;
                cfg->need_cache = 0;
            }
        }

        if ((size > PPP_DECODER_SWAP_THRESHOLD) ||
            (size == 1 && (*(buf + offset) == 0x7D))) {
            if (cfg->cache_buf)
                AIC_ASSERT(0);

            ret = aic_ppp_swap_data(cfg, buf + offset, size);
            if (ret > 0)
                size -= ret;
        }

        if (!size) {
            aos_dma_free(buf);
            aic_ppp_dec_used_cnt(PPP_DECODE);
            return;
        }
#endif
        do {
            if (size > 2048) {
                size -= 2048;
                payload = 2048;
            } else {
                payload = size;
                size = 0;
            }

            ret = aic_ppp_decode_transmit_packet(buf, offset, payload);
            if (ret)
                aicx_warn(0x05cb76, "x", "ppp decode transmit packet fail 0x%x\r\n", ret);
            offset += payload;

        } while (size);
    } else {
        aicx_info(0x0a05b7, "x", "[RECV] aic ppp is stop status 0x%x\r\n", cfg->start_flag);
        aos_dma_free(buf);
    }

    aic_ppp_dec_used_cnt(PPP_DECODE);
}

static void aic_ppp_decode_recv_task(void *p)
{
    u8 *addr;
    AOS_INTERRUPT_DECLARE
    u32 actual_flags_ptr, recv_size;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    while (1) {
        if (aic_ppp_decode_buf_is_empty())
            aos_get_event(cfg->encode_event, PPP_DECODE_RECV_EVENT, AOS_OR_CLEAR,
                          &actual_flags_ptr, AOS_WAIT_FOREVER);

        AOS_INTERRUPT_DISABLE
        addr = aic_ppp_decode_get_buf(&recv_size);
        AOS_INTERRUPT_RESTORE
        if (addr && recv_size) {
            aic_ppp_decode_recv(addr, recv_size);
            recv_size = 0;
        }
    }
}

static int aic_ppp_encode_send_condition(void)
{
    AOS_INTERRUPT_DECLARE
    u32 filled_depth = 0;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    AOS_INTERRUPT_DISABLE
    if (cfg->power_on) {
        filled_depth = aic_ppp_encode_get_out_fifo_filled_depth();
    }
    AOS_INTERRUPT_RESTORE

    return filled_depth;
}

static void aic_ppp_send_task(void *p)
{
    u8 *addr = NULL;
    u16 out_len = 0;
    u32 offset = 0, actual_flags_ptr, send_size;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    while (1) {
        if (!cfg->start_flag || !aic_ppp_encode_send_condition())
            aos_get_event(cfg->encode_event,
                          PPP_ENCODE_SEND_EVENT | PPP_ENCODE_POWERON_EVENT,
                          AOS_OR_CLEAR, &actual_flags_ptr, AOS_WAIT_FOREVER);

        if (!cfg->start_flag) {
            aicx_info(0x116b66, "dd", "[PPPE] PPP don't start %d depth %d\n",
                      cfg->start_flag, aic_ppp_encode_send_condition());
            continue;
        }

        aic_ppp_add_used_cnt(PPP_ENCODE);

ENCODE_SEND_AGAIN:
        if (aic_ppp_encode_get_packet(0, &addr, &out_len, &offset) ==
            AIC_SUCCESS) {
            if (!addr || !out_len)
                continue;

            if (cfg->start_flag) {
                send_size = conn_ppp_send_ex(cfg->comm_handle, addr + offset,
                                             out_len, 0);
                if (send_size != out_len)
                    aicx_warn(0x105515, "dd", "[PPP_DRIVER] send size %d out len %d, have data loss\r\n",
                              send_size, out_len);
                if (cfg->nx_ppp)
                    cfg->nx_ppp->nx_ppp_bytes_sent += out_len;
            } else {
                aicx_info(0x0e3171, "", "[PPP_DRIVER] PPP has stop when send\r\n");
            }

            aos_dma_free(addr);
            addr = NULL;
            out_len = 0;
            offset = 0;

            if (cfg->start_flag)
                goto ENCODE_SEND_AGAIN;
        } else {
            if (!cfg->start_flag || !cfg->power_on)
                aicx_info(0x100c14, "dd", "ppp codec encode now status %d %d\r\n", cfg->start_flag,
                          cfg->power_on);
        }

        aic_ppp_dec_used_cnt(PPP_ENCODE);
    }
}

static void aic_ppp_decode_buf_reset(void)
{
    u8 *addr;
    AOS_INTERRUPT_DECLARE
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    while (!aic_ppp_decode_buf_is_empty()) {
        AOS_INTERRUPT_DISABLE
        addr = aic_ppp_decode_get_buf(NULL);
        AOS_INTERRUPT_RESTORE
        aos_dma_free(addr);
    }

    //cfg->iodev_type = AIC_INVALID_IODEV;
    cfg->ppp_decode_buf_rptr = 0;
    cfg->ppp_decode_buf_wptr = 0;
    memset(cfg->ppp_decode_buf, 0,
           sizeof(struct aic_ppp_decode_buf) * AIC_PPP_DECODE_MAX_BUF);
}

static int aic_ppp_encoder_power_off(void)
{
    int i;
    u8 *addr;
    u16 out_len;
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    u32 offset, fill_depth, retry_cnt = 5;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    while (cfg->encode_used_cnt && retry_cnt) {
        aicx_warn(0x066a5c, "d", "ppp encode is running used_cnt %d\n", cfg->encode_used_cnt);
        retry_cnt--;
        aos_sleep(50);
    }

ENCODE_CHECK_AGAIN:
    fill_depth = aic_ppp_encode_get_out_fifo_filled_depth();
    for (i = 0; i < fill_depth; i++) {
        aic_ppp_encode_get_packet(0, &addr, &out_len, &offset);
        aicx_debug(0x07cacc, "dxdd", "encode index %d addr 0x%x out_len %d offset %d\r\n",
                   i, addr, out_len, offset);
        if (addr) {
            aos_dma_free(addr);
        } else {
            aicx_warn(0x0330a3, "", "get addr fail when encode power off\n");
        }
    }

    if (aic_ppp_encode_get_in_fifo_filled_depth() ||
        aic_ppp_encode_get_out_fifo_filled_depth()) {
        aicx_info(0x126104, "xx", "fifo is not empty when encoder power off in 0x%x out 0x%x\r\n",
                  aic_ppp_encode_get_in_fifo_filled_depth(),
                  aic_ppp_encode_get_out_fifo_filled_depth());
        goto ENCODE_CHECK_AGAIN;
    }

    AOS_INTERRUPT_DISABLE
    while (cfg->encode_packet_head) {
        packet_ptr = cfg->encode_packet_head;
        cfg->encode_packet_head = cfg->encode_packet_head->nx_packet_next;
        if (!cfg->encode_packet_head)
            cfg->encode_packet_last = NULL;
        packet_ptr->nx_packet_next = NULL;
        AOS_INTERRUPT_RESTORE
        nx_packet_transmit_release(packet_ptr);
        AOS_INTERRUPT_DISABLE
    }
    AOS_INTERRUPT_RESTORE

    aicx_info(0x0d33a9, "x", "ppp encode power off success 0x%x\r\n", cfg->encode_packet_head);

    return AIC_SUCCESS;
}

static int aic_ppp_decoder_power_off(void)
{
    int ret, i;
    struct aic_hw_ppp_des *des;
    u32 fill_depth, retry_cnt = 5;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    while (cfg->decode_used_cnt && retry_cnt) {
        aicx_warn(0x11b6e1, "d", "ppp decode is running used_cnt %d\n", cfg->decode_used_cnt);
        retry_cnt--;
        aos_sleep(10);
    }

    if (cfg->decode_used_cnt && !retry_cnt) {
        aicx_warn(0x1b5fc9, "x", "ppp decode power off fail 0x%x\n", cfg->decode_used_cnt);
        return AIC_EBUSY;
    }

    aic_ppp_codec_stop();

DECODE_CHECK_AGAIN:
    fill_depth = aic_ppp_decode_get_out_fifo_filled_depth();
    for (i = 0; i < fill_depth; i++) {
        ret = cfg->ppp_decode_phy.get_out_fifo_curr_des(&des, i);
        if (ret) {
            aicx_error(0x0ed9ca, "xd", "ppp decode power off out fifo get des fail 0x%x i %d\r\n",
                       ret, i);
            i++;
            break;
        }

        if (((u32)des->address != (u32)cfg->last_addr) &&
            des->address && cfg->last_addr) {
            aos_dma_free(cfg->last_addr);
            cfg->last_addr = NULL;
        }

        cfg->last_addr = (void *)(u32)des->address;
    }

    aic_ppp_decode_update_out_fifo_rptr(i);
    if (i != fill_depth) {
        aicx_info(0x0a3b31, "xx", "ppp decode power off out fifo fail 0x%x 0x%x\r\n",
                  i, fill_depth);
        return AIC_EBUSY;
    }

    if (aic_ppp_decode_get_in_fifo_filled_depth() ||
        aic_ppp_decode_get_out_fifo_filled_depth()) {
        aicx_info(0x111c94, "xx", "fifo is not empty when decoder power off in 0x%x out 0x%x\r\n",
                  aic_ppp_decode_get_in_fifo_filled_depth(),
                  aic_ppp_decode_get_out_fifo_filled_depth());
        goto DECODE_CHECK_AGAIN;
    }

    aic_ppp_decode_buf_reset();
    aicx_info(0x1a9712, "", "ppp decode power off success\r\n");

    return AIC_SUCCESS;
}

static int aic_ppp_decoder_power_on(void)
{
    int ret;
    u32 in_fifo_addr = 0;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    aic_ppp_decode_buf_reset();

    cfg->ppp_decode_phy.get_in_fifo_addr(&in_fifo_addr);
    if (in_fifo_addr != 0) {
        aicx_info(0x0a8bde, "", "ppp decoder ip don't power off\r\n");
        return AIC_SUCCESS;
    }

    ret = cfg->ppp_decode_phy.init(aic_ppp_decoder_isr_func,
                                   &aic_ppp_codec_drv_cfg,
                                   (u32)cfg->ppp_decoder_in_fifo_addr,
                                   (u32)cfg->ppp_decoder_out_fifo_addr,
                                   PPP_DECODE_FIFO_TOTAL_POWER_DEPTH,
                                   PPP_DECODE_FIFO_TOTAL_POWER_DEPTH);
    if (ret)
        return ret;

    ret = cfg->ppp_decode_phy.threshold_irq_config(true,
                                                   PPP_DECODE_FIFO_THRESHOLD);
    if (ret)
        return ret;

    ret = cfg->ppp_decode_phy.timeout_irq_config(true,
                                                 PPP_DECODE_FIFO_TIMEOUT);
    if (ret)
        return ret;

    ret = cfg->ppp_decode_phy.flow_ctrl_irq_config(true);
    if (ret)
        return ret;

    return AIC_SUCCESS;
}

static int aic_ppp_encoder_power_on(void)
{
    int ret;
    u32 in_fifo_addr = 0;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    cfg->ppp_encode_phy.get_in_fifo_addr(&in_fifo_addr);
    if (in_fifo_addr != 0) {
        aicx_info(0x161a58, "", "ppp encoder ip don't power off\r\n");
        if (aic_ppp_encode_get_out_fifo_filled_depth())
            aos_set_event(cfg->encode_event, PPP_ENCODE_SEND_EVENT, AOS_OR);
        return AIC_SUCCESS;
    }

    ret = cfg->ppp_encode_phy.init(aic_ppp_encoder_isr_func,
                                   &aic_ppp_codec_drv_cfg,
                                   (u32)cfg->ppp_encoder_in_fifo_addr,
                                   (u32)cfg->ppp_encoder_out_fifo_addr,
                                   PPP_ENCODE_FIFO_TOTAL_POWER_DEPTH,
                                   PPP_ENCODE_FIFO_TOTAL_POWER_DEPTH);
    ret = cfg->ppp_encode_phy.threshold_irq_config(true,
                                                   PPP_ENCODE_FIFO_THRESHOLD);
    if (ret)
        return ret;

    ret = cfg->ppp_encode_phy.timeout_irq_config(true,
                                                 PPP_ENCODE_FIFO_TIMEOUT);
    if (ret)
        return ret;

    ret = cfg->ppp_encode_phy.flow_ctrl_irq_config(true);
    if (ret)
        return ret;

    return AIC_SUCCESS;
}

static int aic_ppp_codec_power_on(void)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->start_flag)
        return AIC_SUCCESS;

    AIC_WRITE32(PPP_DECODE_CORE_CLK, PPP_DECODE_DEFAULT_CLK);
    AIC_WRITE32(PPP_ENCODE_CORE_CLK, PPP_ENCODE_DEFAULT_CLK);

#ifdef PPP_CLK_FORCE_ON
    /* Force coreclk/axiclk/apbclk on */
    //AIC_WRITE32(0x420100f4, 2);
    AIC_WRITE32(0x42010084, 2);
    AIC_WRITE32(0x420100C0, 2);

    //AIC_WRITE32(0x420100f0, 2);
    AIC_WRITE32(0x42010088, 2);
    AIC_WRITE32(0x420100C4, 2);
#endif
    AIC_WRITE32(PPP_ENCODE_IP_POWER, 0x1);
    AIC_WRITE32(PPP_DECODE_IP_POWER, 0x1);
#if 0
    AIC_WRITE32(PPP_DECODE_ENC_SHW, 0x1);
    AIC_WRITE32(PPP_ENCODE_ENC_SHW, 0x1);
#endif
    cfg->power_on = TRUE;

    ret = aic_ppp_decoder_power_on();
    if (ret) {
        aicx_error(0x1addab, "x", "ppp decoder power on fail 0x%x\n", ret);
        return ret;
    }

    ret = aic_ppp_encoder_power_on();
    if (ret) {
        aicx_error(0x18a37a, "x", "ppp encoder power on fail 0x%x\n", ret);
        return ret;
    }

#ifdef PPP_CLK_FORCE_ON
    AIC_WRITE32(PPP_ENCODE_REG_START + 0x64, 0x00A300A5);
    AIC_WRITE32(PPP_DECODE_REG_START + 0x64, 0x00A300A5);
#endif
    cfg->start_flag = TRUE;

    return ret;
}

static int aic_ppp_codec_power_off(void)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (!cfg->start_flag)
        return AIC_SUCCESS;

    cfg->start_flag = FALSE;

    ret = aic_ppp_decoder_power_off();
    if (ret) {
        aicx_error(0x10fef3, "x", "ppp decoder power off fail 0x%x\n", ret);
        return ret;
    }

    ret = aic_ppp_encoder_power_off();
    if (ret) {
        aicx_error(0x1a0230, "x", "ppp encoder power off fail 0x%x\n", ret);
        return ret;
    }

#if 0
    AIC_WRITE32(PPP_DECODE_ENC_SHW, 0x0);
    AIC_WRITE32(PPP_ENCODE_ENC_SHW, 0x0);
#endif
    cfg->power_on = FALSE;

    AIC_WRITE32(PPP_ENCODE_IP_POWER, 0x0);
    AIC_WRITE32(PPP_DECODE_IP_POWER, 0x0);

#ifdef PPP_CLK_FORCE_ON
    //AIC_WRITE32(0x420100f4, 0);
    AIC_WRITE32(0x42010084, 0);
    AIC_WRITE32(0x420100C0, 0);

    //AIC_WRITE32(0x420100f0, 0);
    AIC_WRITE32(0x42010088, 0);
    AIC_WRITE32(0x420100C4, 0);
#endif

    return ret;
}

int aic_ppp_codec_stop(void)
{
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    AOS_INTERRUPT_DISABLE
    packet_ptr = cfg->decode_receive_partial_packet;
    cfg->decode_receive_partial_packet = NULL;
    AOS_INTERRUPT_RESTORE

    if (packet_ptr)
        nx_packet_release(packet_ptr);

    return AIC_SUCCESS;
}

u16 aic_ppp_encode_get_out_fifo_filled_depth(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    return cfg->ppp_encode_phy.get_out_fifo_filled_depth();
}

int aic_ppp_encode_get_packet(u16 index, u8 **addr,
                              u16 *out_len, u32 *offset)
{
    int ret;
    void *dst_addr;
    NX_PACKET *packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_hw_ppp_des *des;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    AOS_INTERRUPT_DISABLE
    packet_ptr = cfg->encode_packet_head;

    if (!cfg->power_on) {
        AOS_INTERRUPT_RESTORE
        aicx_info(0x08271f, "", "[PPP] ppp encode is power off when get packet\n");
        return AIC_ENODEV;
    }

    if (!aic_ppp_encode_get_out_fifo_filled_depth()) {
        AOS_INTERRUPT_RESTORE
        //alog_info("[PPP] ppp encode out fifo is empty when get packet\n");
        return AIC_ENODATA;
    }

    if (packet_ptr) {
        cfg->encode_packet_head = cfg->encode_packet_head->nx_packet_next;
        if (!cfg->encode_packet_head)
            cfg->encode_packet_last = NULL;
        packet_ptr->nx_packet_next = NULL;
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        AOS_INTERRUPT_RESTORE
        nx_packet_transmit_release(packet_ptr);
        AOS_INTERRUPT_DISABLE
    } else {
        aicx_error(0x1a66af, "", "ppp encoder nx packet list wrong.\n");
        AOS_INTERRUPT_RESTORE
        AIC_ASSERT(FALSE);
        return AIC_ENOENT;
    }

    ret = cfg->ppp_encode_phy.get_out_fifo_curr_des(&des, index);
    if (ret) {
        aicx_error(0x11cf88, "x", "[PPP_ENCODE] out fifo get curr des fail 0x%x\r\n", ret);
        AOS_INTERRUPT_RESTORE
        return AIC_ENOENT;
    }

    AOS_INTERRUPT_RESTORE

    if (!des->address) {
        cfg->encoder_err_cnt++;
        aicx_info(0x08c8fc, "xxxxx", "[PPP_ENCODE] des 0x%x PPP encoder addr 0x%x out_length 0x%x out_offset 0x%x seq 0x%x\r\n",
                  des, des->address, des->out_length, des->out_offset, des->seq);
        *addr = NULL;
        *offset = 0;
        *out_len = 0;
        aic_ppp_encode_update_out_fifo_rptr(1);
        return AIC_SUCCESS;
    }

    if (des->out_offset & BIT(23))
        dst_addr = (void *)(u32)(des->address - (des->out_offset & ~(BIT(23))));
    else
        dst_addr = (void *)(u32)(des->address + des->out_offset);

    *out_len = des->out_length;

    *addr = (void *)((u32)dst_addr - des->reserved);
    *offset = des->reserved;

    aicx_debug(0x01577d, "xxxxx", "get des 0x%x PPP encoder addr 0x%x out_length 0x%x out_offset 0x%x addr 0x%x\n",
               des, des->address, des->out_length, des->out_offset, *addr);

    dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(*addr + *offset),
                        PPP_CALC_CACHE_LINE_SIZE(*addr + *offset, *out_len));
    aic_ppp_encode_update_out_fifo_rptr(1);

    return AIC_SUCCESS;
}

int aic_ppp_encode_update_out_fifo_rptr(u16 num)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    ret = cfg->ppp_encode_phy.out_fifo_rptr_update(num);
    if (ret) {
        aicx_error(0x02f87c, "x", "PPP_ENCODER update out fifo rptr fail 0x%x\r\n", ret);
        return ret;
    }

    return AIC_SUCCESS;
}

int aic_ppp_encode_transmit_packet(NX_PACKET *packet_ptr,
                                   u16 index)
{
    int ret;
    void *dst_addr;
    NX_PACKET *new_packet_ptr;
    AOS_INTERRUPT_DECLARE
    struct aic_hw_ppp_des *des;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;
    u32 total_heap_size = 0, used_heap_size = 0, free_heap_size = 0;

    ret = aos_get_heap_usage(&total_heap_size, &used_heap_size);
    if (ret) {
        total_heap_size = 0;
        used_heap_size = 0;
        free_heap_size = 0;
    } else {
        free_heap_size = total_heap_size - used_heap_size;
    }

    if (free_heap_size && (free_heap_size < PPP_HEAP_LOW_WATER) &&
        (packet_ptr->nx_packet_length >= 40)) {
        aicx_warn(0x13f69d, "dxx", "[PPP] free heap is low water %d 0x%x 0x%x\n",
                  free_heap_size, *packet_ptr->nx_packet_prepend_ptr,
                  *(packet_ptr->nx_packet_prepend_ptr + 1));
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        nx_packet_transmit_release(packet_ptr);
        return AIC_ENOMEM;
    }

    aic_ppp_add_used_cnt(PPP_ENCODE);
    if (!cfg->start_flag) {
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        nx_packet_transmit_release(packet_ptr);
        aicx_warn(0x02be99, "", "ppp codec has stop\r\n");
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return AIC_EBUSY;
    }

    if ((cfg->iodev_type == AIC_IODEV_UART &&
         packet_ptr->nx_packet_reserved & AIC_PACKET_SHARE_MEM) ||
        packet_ptr->nx_packet_next) {
        ret = _nx_packet_allocate(cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool,
                                  &new_packet_ptr, NX_PHYSICAL_HEADER, 0);
        if (ret != NX_SUCCESS) {
            aicx_error(0x02dfa5, "x", "[PPP] alloc fail 0x%x when send\n", ret);
            packet_ptr->nx_packet_length -= 2;
            packet_ptr->nx_packet_prepend_ptr += 2;
            _nx_packet_transmit_release(packet_ptr);
            aic_ppp_dec_used_cnt(PPP_ENCODE);
            return AIC_ENOMEM;
        }

        ret = _nx_packet_data_append(new_packet_ptr,
                                     packet_ptr->nx_packet_prepend_ptr,
                                     packet_ptr->nx_packet_length,
                                     cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool, 0);
        if (ret != NX_SUCCESS) {
            aicx_error(0x0c21d4, "x", "[PPP] append fail 0x%x when send\n", ret);

            packet_ptr->nx_packet_length -= 2;
            packet_ptr->nx_packet_prepend_ptr += 2;
            _nx_packet_transmit_release(packet_ptr);
            _nx_packet_transmit_release(new_packet_ptr);
            aic_ppp_dec_used_cnt(PPP_ENCODE);
            return AIC_ENOMEM;
        }

        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        _nx_packet_transmit_release(packet_ptr);
        packet_ptr = new_packet_ptr;
    }

    if (packet_ptr->nx_packet_next)
        aicx_info(0x0a4d6a, "", "[PPP] occur scatter packet\n");

    if ((PPP_ENCODE_FIFO_TOTAL_DEPTH -
        cfg->ppp_encode_phy.get_in_fifo_filled_depth()) < (index + 1)) {
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        nx_packet_transmit_release(packet_ptr);
        aicx_error(0x1ae3c8, "x", "ppp encoder in fifo is full 0x%x\r\n",
                   cfg->ppp_encode_phy.get_in_fifo_filled_depth());
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return AIC_ENOMEM;
    }

    dst_addr = aos_dma_malloc(packet_ptr->nx_packet_length * 2 + 16);
    if (!dst_addr) {
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        nx_packet_transmit_release(packet_ptr);
        aicx_error(0x0d929f, "", "PPP_ENCODER transmit alloc fail\r\n");
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return AIC_ENOMEM;
    }

    dcache_clean_by_range(PPP_GET_CACHE_LINE_ADDR(dst_addr),
                          PPP_CALC_CACHE_LINE_SIZE(dst_addr, packet_ptr->nx_packet_length * 2 + 16));
    dcache_clean_by_range(PPP_GET_CACHE_LINE_ADDR(packet_ptr->nx_packet_prepend_ptr),
                          PPP_CALC_CACHE_LINE_SIZE(packet_ptr->nx_packet_prepend_ptr,
                                                   packet_ptr->nx_packet_length));

    ret = cfg->ppp_encode_phy.get_in_fifo_curr_des(&des, index);
    if (ret) {
        aicx_error(0x080267, "x", "PPP_ENCODER get curr des fail 0x%x\r\n", ret);
        aos_dma_free(dst_addr);
        packet_ptr->nx_packet_length -= 2;
        packet_ptr->nx_packet_prepend_ptr += 2;
        nx_packet_transmit_release(packet_ptr);
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return ret;
    }

    memset(des, 0, sizeof(struct aic_hw_ppp_des));
    des->address = (u32)packet_ptr->nx_packet_prepend_ptr;
    des->in_length = packet_ptr->nx_packet_length;
    des->reserved = CACHE_LINE_OFFSET(dst_addr);

    dst_addr = (void *)((u32)dst_addr + CACHE_LINE_OFFSET(dst_addr));
    if ((u32)dst_addr > (u32)packet_ptr->nx_packet_prepend_ptr)
        des->out_offset = (u32)dst_addr -
            (u32)packet_ptr->nx_packet_prepend_ptr;
    else
        des->out_offset = ((u32)packet_ptr->nx_packet_prepend_ptr -
                           (u32)dst_addr) | BIT(23);

    aicx_debug(0x1782cb, "xxxx", "[PPPE] send des 0x%x addr 0x%x dst addr 0x%x offset 0x%x\n",
               des, (u32)packet_ptr->nx_packet_prepend_ptr, dst_addr,
               des->out_offset);

    AOS_INTERRUPT_DISABLE
    if (cfg->encode_packet_head) {
        cfg->encode_packet_last->nx_packet_next = packet_ptr;
        cfg->encode_packet_last = packet_ptr;
    } else {
        cfg->encode_packet_head = packet_ptr;
        cfg->encode_packet_last = packet_ptr;
    }
    AOS_INTERRUPT_RESTORE

    dcache_clean_by_range(PPP_GET_CACHE_LINE_ADDR(des),
                          PPP_CODEC_CACHE_LINE_SIZE);
    aic_ppp_dec_used_cnt(PPP_ENCODE);

    return AIC_SUCCESS;
}

int aic_ppp_encode_update_in_fifo_wptr(u16 num)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    aic_ppp_add_used_cnt(PPP_ENCODE);
    if (!cfg->start_flag) {
        aicx_warn(0x1d9bb6, "", "ppp codec has stop\n");
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return AIC_EBUSY;
    }

    ret = cfg->ppp_encode_phy.in_fifo_wptr_update(num);
    if (ret) {
        aicx_error(0x17b2ea, "x", "PPP_ENCODER update in fifo wptr fail 0x%x\n", ret);
        aic_ppp_dec_used_cnt(PPP_ENCODE);
        return ret;
    }

    aic_ppp_dec_used_cnt(PPP_ENCODE);

    return AIC_SUCCESS;
}

u16 aic_ppp_encode_get_in_fifo_filled_depth(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    return cfg->ppp_encode_phy.get_in_fifo_filled_depth();
}

u16 aic_ppp_decode_get_out_fifo_filled_depth(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    return cfg->ppp_decode_phy.get_out_fifo_filled_depth();
}

int aic_ppp_decode_get_packet(void)
{
    u8 mem_type, ppp_header_len;
    int ret, i = 0;
    u32 buf_size = 0;
    u16 filled_depth;
    u8 *dst_addr, *buf;
    static u8 in_progress = 0;
    struct aic_hw_ppp_des *des;
    NX_PACKET *err_packet_ptr = NULL;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;
    NX_PACKET *head_packet_ptr = NULL, *last_packet_ptr = NULL;

    if (cfg->nx_ppp->nx_ppp_head_packet)
        return AIC_EAGAIN;

    aic_ppp_add_used_cnt(PPP_DECODE);
    if (!cfg->start_flag) {
        aicx_warn(0x1d9bb6, "", "ppp codec has stop\n");
        aic_ppp_dec_used_cnt(PPP_DECODE);
        return AIC_EBUSY;
    }

    filled_depth = cfg->ppp_decode_phy.get_out_fifo_filled_depth();
    if (filled_depth) {
        head_packet_ptr = cfg->decode_receive_partial_packet;
    } else {
        aic_ppp_dec_used_cnt(PPP_DECODE);
        return AIC_ENODATA;
    }

    for (i = 0; i < filled_depth; i++) {
        ret = cfg->ppp_decode_phy.get_out_fifo_curr_des(&des, i);
        if (ret) {
            aicx_error(0x113749, "xd", "PPP_DECODER out fifo get curr des fail 0x%x i %d\r\n",
                       ret, i);
            break;
        }

        if (((u32)des->address != (u32)cfg->last_addr) &&
            des->address && cfg->last_addr) {
            aos_dma_free(cfg->last_addr);
            cfg->last_addr = NULL;
        }

        if (des->out_offset & BIT(23))
            dst_addr = (u8 *)(u32)(des->address -
                                   (des->out_offset & ~(BIT(23))));
        else
            dst_addr = (u8 *)(u32)(des->address + des->out_offset);

        dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(dst_addr),
                            PPP_CALC_CACHE_LINE_SIZE(dst_addr,
                            des->out_length));

        if (des->errcode || (!des->check && !des->seq) ||
            (!des->check && (des->seq & BIT(7)))) {
#if 0
            for (u8 j = 0; j < des->out_length; j++)
                 aicx_info(0x001b67, "dx", "j %d 0x%x\r\n", j, *(dst_addr + j));
#endif

            if ((des->out_length == 1) && (*dst_addr == 0x7E)) {
                in_progress = 0;
                cfg->last_addr = (u8 *)(u32)des->address;
                if (head_packet_ptr) {
                    nx_packet_release(head_packet_ptr);
                    head_packet_ptr = NULL;
                }
                continue;
            }

            aicx_error(0x01cb62, "dxxxxddx", "[PPP_DRIVER] index %d decoder get des 0x%x errcode 0x%x ck 0x%x seq 0x%x in_len %d out_len %d head_packet_ptr 0x%x\r\n",
                       i, des, des->errcode, des->check, des->seq, des->in_length,
                       des->out_length, head_packet_ptr);

            if (des->out_length > PPP_MAX_DATA_LEN) {
                aicx_error(0x0e05db, "d", "[PPP] decode err des len %d err\n", des->out_length);
                in_progress = 0;
                cfg->last_addr = (u8 *)(u32)des->address;
                if (head_packet_ptr) {
                    nx_packet_release(head_packet_ptr);
                    head_packet_ptr = NULL;
                }
                continue;
            }

            if (head_packet_ptr) {
                ret = nx_packet_copy(head_packet_ptr, &err_packet_ptr,
                                     cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool,
                                     NX_NO_WAIT);
                nx_packet_release(head_packet_ptr);
                head_packet_ptr = NULL;
            } else {
                ret = nx_packet_allocate(cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool,
                                         &err_packet_ptr, 24, NX_NO_WAIT);
            }

            if (ret == NX_SUCCESS) {
                nx_packet_data_append(err_packet_ptr, dst_addr, des->out_length,
                                      cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool,
                                      NX_NO_WAIT);
                cfg->nx_ppp->nx_ppp_non_ppp_packet_handler(err_packet_ptr);
            } else {
                aicx_error(0x07a557, "", "[PPP_DRIVER] alloc packet fail when non ppp occur\r\n");
            }

            in_progress = 0;
            cfg->last_addr = (u8 *)(u32)des->address;
            continue;
        }

        if (!head_packet_ptr && (des->seq > 1)) {
            aicx_warn(0x1a1cfa, "xx", "The header packet is null 0x%x 0x%x\r\n",
                      in_progress, des->seq);
            in_progress = 0;
            cfg->last_addr = (u8 *)(u32)des->address;
            continue;
        }

        if ((des->out_length == 1) && (*dst_addr == 0x7E) && !des->seq) {
            if (head_packet_ptr) {
                nx_packet_release(head_packet_ptr);
                head_packet_ptr = NULL;
                aicx_warn(0x1897b4, "", "incomplete frame before an empty frame\r\n");
            }

            aicx_warn(0x0896c8, "x", "ppp decoder get empty ppp 0x%x\r\n", in_progress);
            in_progress = 0;
            cfg->last_addr = (u8 *)(u32)des->address;
            continue;
        }

        if (des->seq && !(des->seq & BIT(7)))
            in_progress = 1;

        if (!des->seq)
            buf_size = des->out_length;
        else
            buf_size = PPP_MAX_DATA_LEN;

        if (!des->seq || des->seq == 1) {
            if (head_packet_ptr) {
                nx_packet_release(head_packet_ptr);
                head_packet_ptr = NULL;
            }
            buf_size = buf_size + PPP_CODEC_PACKET_OFFSET + 4;
            buf = aic_ppp_alloc_mem(buf_size, &mem_type);
            if (!buf) {
                aicx_error(0x10da58, "", "aic ppp alloc mem fail\r\n");
                cfg->last_addr = (u8 *)(u32)des->address;
                continue;
            }

            aicx_debug(0x0dd9c2, "xx", "[PPP_DRIVER] mem type 0x%x addr 0x%x\r\n", mem_type, buf);
            head_packet_ptr =
                aic_packet_build(buf, buf_size,
                                 PPP_CODEC_PACKET_OFFSET,
                                 cfg->nx_ppp->nx_ppp_ip_ptr->nx_ip_default_packet_pool,
                                 mem_type);
            if (!head_packet_ptr) {
                aicx_error(0x01b881, "", "PPP_DECODER alloc nx packet fail\r\n");
                cfg->last_addr = (u8 *)(u32)des->address;
                aic_ppp_release_mem(buf, mem_type, buf_size);
                continue;
            }

            if ((*dst_addr == 0x7E) && (*(dst_addr + 1) == 0xFF) &&
                (*(dst_addr + 2) == 0x03)) {
                ppp_header_len = 5;
            } else if ((*dst_addr == 0xFF) && (*(dst_addr + 1) == 0x03)) {
                ppp_header_len = 4;
            } else if (cfg->is_compress && (*dst_addr == 0x7E)) {
                ppp_header_len = 3;
            } else if (cfg->is_compress) {
                ppp_header_len = 2;
            } else {
                ppp_header_len = 0;
                aicx_error(0x1525d1, "xxxx", "calc ppp header fail 0x%x 0x%x 0x%x 0x%x\r\n",
                           cfg->is_compress, *dst_addr, *(dst_addr + 1),
                           *(dst_addr + 2));
            }

            aicx_debug(0x024f67, "xxx", "[PPP_DRIVER] packet 0x%x buf 0x%x header_len 0x%x\r\n",
                       head_packet_ptr, buf, ppp_header_len);
            head_packet_ptr->nx_packet_prepend_ptr += (0x4 -
                ((u32)(head_packet_ptr->nx_packet_prepend_ptr + ppp_header_len) &
                 0x3));
        }

        aicx_debug(0x139bf3, "xxxxx", "dst 0x%x in_len 0x%x in_off 0x%x out len 0x%x out off 0x%x\r\n",
                   dst_addr, des->in_length, des->in_offset,
                   des->out_length, des->out_offset);

        if ((des->out_length + head_packet_ptr->nx_packet_length) <= PPP_MAX_DATA_LEN) {
            memcpy(head_packet_ptr->nx_packet_prepend_ptr +
                   head_packet_ptr->nx_packet_length, dst_addr,
                   des->out_length);
            head_packet_ptr->nx_packet_length += des->out_length;
        } else {
            aicx_error(0x1055c5, "dd", "PPP DECODER err packet detected %d %d\n",
                       des->out_length, head_packet_ptr->nx_packet_length);
            nx_packet_release(head_packet_ptr);
            head_packet_ptr = NULL;
            cfg->last_addr = (u8 *)(u32)des->address;
            continue;
        }

        if (!des->seq || (des->seq & BIT(7))) {
            u8 tmp = 0;
            in_progress = 0;

            head_packet_ptr->nx_packet_append_ptr -= 3;
            head_packet_ptr->nx_packet_length -= 3;

            if (*head_packet_ptr->nx_packet_prepend_ptr == 0x7E &&
                (*(u16 *)(head_packet_ptr->nx_packet_prepend_ptr + 1) ==
                0x03FF)) {
                tmp = 3;
            } else if ((*(u16 *)head_packet_ptr->nx_packet_prepend_ptr ==
                       0x03FF)) {
                tmp = 2;
            } else if (*head_packet_ptr->nx_packet_prepend_ptr == 0x7E &&
                       cfg->is_compress) {
                tmp = 1;
            } else if (*head_packet_ptr->nx_packet_prepend_ptr != 0x7E &&
                       cfg->is_compress) {
                tmp = 0;
            } else {
                aicx_error(0x083a93, "xx", "PPP DECODER non ppp detected again 0x%x 0x%x\n",
                           des->errcode, des->seq);
                nx_packet_release(head_packet_ptr);
                head_packet_ptr = NULL;
                cfg->last_addr = (u8 *)(u32)des->address;
                continue;
            }

            aicx_debug(0x021a48, "xxx", "head packet 0x%x prepend 0x%x tmp 0x%x\n",
                       head_packet_ptr, head_packet_ptr->nx_packet_prepend_ptr,
                       tmp);

            head_packet_ptr->nx_packet_prepend_ptr += tmp;
            head_packet_ptr->nx_packet_length -= tmp;
            head_packet_ptr->nx_packet_append_ptr =
                head_packet_ptr->nx_packet_prepend_ptr +
                head_packet_ptr->nx_packet_length;

            if (!cfg->nx_ppp->nx_ppp_head_packet)
                cfg->nx_ppp->nx_ppp_head_packet = head_packet_ptr;
            else
                last_packet_ptr->nx_packet_queue_next = head_packet_ptr;
            last_packet_ptr = head_packet_ptr;

            head_packet_ptr = NULL;
        }

        cfg->last_addr = (u8 *)(u32)des->address;
    }

    if (des->seq && !(des->seq & BIT(7))) {
        cfg->decode_receive_partial_packet = head_packet_ptr;
    } else {
        cfg->decode_receive_partial_packet = NULL;
    }

    cfg->ppp_decode_phy.out_fifo_rptr_update(i);
    aic_ppp_dec_used_cnt(PPP_DECODE);

    return AIC_SUCCESS;
}

int aic_ppp_decode_update_out_fifo_rptr(u16 num)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    ret = cfg->ppp_decode_phy.out_fifo_rptr_update(num);
    if (ret) {
        aicx_error(0x09b8ae, "x", "PPP_DECODER update out fifo rptr fail 0x%x\n", ret);
        return ret;
    }

    return AIC_SUCCESS;
}

int aic_ppp_decode_transmit_packet(void *packet_ptr, u16 offset, u16 len)
{
    int ret;
    struct aic_hw_ppp_des *des;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (offset > 0xFF4 || len > 0xFF4)
        AIC_ASSERT(0);

    if (cfg->ppp_decode_phy.get_in_fifo_filled_depth() >=
        PPP_DECODE_FIFO_TOTAL_DEPTH) {
        aicx_error(0x0038a0, "x", "PPP DECODER send data fail filled depth 0x%x\n",
                   cfg->ppp_decode_phy.get_in_fifo_filled_depth());
        aos_dma_free(packet_ptr);

        return AIC_ENOBUFS;
    }

    ret = cfg->ppp_decode_phy.get_in_fifo_curr_des(&des, 0);
    if (ret || !des) {
        aicx_error(0x06c9c8, "x", "PPP_DECODER get curr des fail 0x%x\n", ret);
        aos_dma_free(packet_ptr);
        return ret;
    }

    dcache_clean_by_range(PPP_GET_CACHE_LINE_ADDR((u32)packet_ptr + offset),
                          PPP_CALC_CACHE_LINE_SIZE((u32)packet_ptr + offset, len));

    memset(des, 0, sizeof(struct aic_hw_ppp_des));
    des->address = (u64)(u32)packet_ptr;
    des->in_offset = offset;
    des->out_offset = offset;
    des->in_length = len;

    dcache_clean_by_range(PPP_GET_CACHE_LINE_ADDR(des),
                          PPP_CODEC_CACHE_LINE_SIZE);
    cfg->ppp_decode_phy.in_fifo_wptr_update(1);

    return AIC_SUCCESS;
}

int aic_ppp_decode_update_in_fifo_wptr(u16 num)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    ret = cfg->ppp_decode_phy.in_fifo_wptr_update(num);
    if (ret) {
        aicx_error(0x0a7f4e, "x", "PPP_DECODER update in fifo wptr fail 0x%x\n", ret);
        return ret;
    }

    return AIC_SUCCESS;
}

u16 aic_ppp_decode_get_in_fifo_filled_depth(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    return cfg->ppp_decode_phy.get_in_fifo_filled_depth();
}

void aic_ppp_codec_set_compress(u8 compress)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    cfg->is_compress = compress;
}

void aic_ppp_codec_start_process(u32 com)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    aic_get_wakelock();
    cfg->comm_handle = com;
    cfg->iodev_type = conn_get_at_dev_type(com);

    aicx_info(0x0b1bd4, "dd", "[PPP] ppp codec start com %d iodev_tye %d\n",
              cfg->comm_handle, cfg->iodev_type);
    aic_ppp_codec_power_on();
}

void aic_ppp_codec_stop_process(void)
{
    int ret;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    ret = aic_ppp_codec_power_off();
    if (ret)
        aicx_info(0x1c3f01, "x", "ppp codec power off fail 0x%x\n", ret);

    cfg->is_compress = 0;
    cfg->iodev_type = AIC_INVALID_IODEV;

    cfg->comm_handle = 0xFF;
    if (cfg->cache_buf) {
        aos_dma_free(cfg->cache_buf);
        cfg->swap_size = 0;
        cfg->cache_size = 0;
        cfg->cache_buf = NULL;
        cfg->need_cache = 0;
    }

    if (cfg->last_addr) {
        aos_dma_free(cfg->last_addr);
        cfg->last_addr = NULL;
    }

    aicx_info(0x086423, "", "[PPP] ppp codec stop\n");

    aic_put_wakelock();
}

int aic_ppp_codec_register(NX_PPP *ppp_ptr)
{
    u32 fifo_size;
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    cfg->nx_ppp = ppp_ptr;

    fifo_size = sizeof(struct aic_hw_ppp_des) * PPP_DECODE_FIFO_TOTAL_DEPTH;

    if (!cfg->ppp_decoder_in_fifo_addr)
        cfg->ppp_decoder_in_fifo_addr = aos_dma_malloc(fifo_size);
    else
        aicx_info(0x09d6a7, "", "ppp_decoder_in_fifo_addr has val when register\n");

    if (!cfg->ppp_decoder_in_fifo_addr ||
        ((u32)cfg->ppp_decoder_in_fifo_addr & 0x3)) {
        aicx_error(0x156d92, "x", "[PPP_CODEC] alloc decoder in fifo fail 0x%x\n",
                   (u32)cfg->ppp_decoder_in_fifo_addr);
        return AIC_ENOMEM;
    }

    dcache_clean_by_range((u32)cfg->ppp_decoder_in_fifo_addr,
                          PPP_CALC_CACHE_LINE_SIZE((u32)cfg->ppp_decoder_in_fifo_addr,
                                                   fifo_size));

    if (!cfg->ppp_decoder_out_fifo_addr)
        cfg->ppp_decoder_out_fifo_addr = aos_dma_malloc(fifo_size);
    else
        aicx_info(0x1a5525, "", "ppp_decoder_out_fifo_addr has val when register\n");

    if (!cfg->ppp_decoder_out_fifo_addr ||
        ((u32)cfg->ppp_decoder_out_fifo_addr & 0x3)) {
        aicx_error(0x1cf125, "x", "[PPP_CODEC] alloc decoder out fifo fail 0x%x\n",
                   (u32)cfg->ppp_decoder_out_fifo_addr);
        return AIC_ENOMEM;
    }
    dcache_clean_by_range((u32)cfg->ppp_decoder_out_fifo_addr,
                          PPP_CALC_CACHE_LINE_SIZE((u32)cfg->ppp_decoder_out_fifo_addr,
                                                   fifo_size));

    fifo_size = sizeof(struct aic_hw_ppp_des) * PPP_ENCODE_FIFO_TOTAL_DEPTH;
    if (!cfg->ppp_encoder_in_fifo_addr)
        cfg->ppp_encoder_in_fifo_addr = aos_dma_malloc(fifo_size);
    else
        aicx_info(0x09caf6, "", "ppp_encoder_in_fifo_addr has val when register\n");

    if (!cfg->ppp_encoder_in_fifo_addr ||
        ((u32)cfg->ppp_encoder_in_fifo_addr & 0x3)) {
        aicx_error(0x0cc8b6, "x", "[PPP_CODEC] alloc encoder in fifo fail 0x%x\n",
                   (u32)cfg->ppp_encoder_in_fifo_addr);
        return AIC_ENOMEM;
    }

    dcache_clean_by_range((u32)cfg->ppp_encoder_in_fifo_addr, fifo_size);

    if (!cfg->ppp_encoder_out_fifo_addr)
        cfg->ppp_encoder_out_fifo_addr = aos_dma_malloc(fifo_size);
    else
        aicx_info(0x140fd1, "", "ppp_encoder_out_fifo_addr has val when register\n");

    if (!cfg->ppp_encoder_out_fifo_addr ||
        ((u32)cfg->ppp_encoder_out_fifo_addr & 0x3)) {
        aicx_error(0x03ec94, "x", "[PPP_CODEC] alloc encoder out fifo fail 0x%x\n",
                   (u32)cfg->ppp_encoder_out_fifo_addr);
        return AIC_ENOMEM;
    }

    dcache_clean_by_range((u32)cfg->ppp_encoder_out_fifo_addr, fifo_size);

    return AIC_SUCCESS;
}

int aic_ppp_codec_unregister(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    if (cfg->start_flag) {
        aicx_error(0x0cbdcc, "", "ppp codec is running when unregister\n");
        return AIC_EBUSY;
    }
#if 0
    if (cfg->ppp_encoder_out_fifo_addr) {
        aos_dma_free(cfg->ppp_encoder_out_fifo_addr);
        cfg->ppp_encoder_out_fifo_addr = NULL;
    }

    if (cfg->ppp_encoder_in_fifo_addr) {
        aos_dma_free(cfg->ppp_encoder_in_fifo_addr);
        cfg->ppp_encoder_in_fifo_addr = NULL;
    }

    if (cfg->ppp_decoder_out_fifo_addr) {
        aos_dma_free(cfg->ppp_decoder_out_fifo_addr);
        cfg->ppp_decoder_out_fifo_addr = NULL;
    }

    if (cfg->ppp_decoder_in_fifo_addr) {
        aos_dma_free(cfg->ppp_decoder_in_fifo_addr);
        cfg->ppp_decoder_in_fifo_addr = NULL;
    }
#endif
    cfg->nx_ppp = NULL;

    return AIC_SUCCESS;
}

#if 0
static int aic_ppp_cap_cmd(int argc, char **argv)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    //aic_start_ppp_pcap = 1;
    CONS_OUT("iodev_type %d\n", cfg->iodev_type);
    return 0;
}
CONSOLE_EXPORT_CMD(ppp_cap, aic_ppp_cap_cmd, aic ppp cap);
#endif

#if 0
static int aic_ppp_codec_power_off_cmd(int argc, char **argv)
{
    int ret;

    aic_ppp_codec_stop_process();
    ret = aic_ppp_codec_unregister();
    aicx_info(0x0f15e8, "x", "aic_ppp_codec_unregister 0x%x\n", ret);

    return AIC_SUCCESS;
}

CONSOLE_EXPORT_CMD(ppp_codec_power_off_test, aic_ppp_codec_power_off_cmd,
                   ppp codec power off);

static int aic_ppp_encode_test_cmd(int argc, char **argv)
{
    void *buf;
    NX_PACKET *packet_ptr;
    u8 encode_data[] = {
        0x80, 0x21, 0x01, 0x2B, 0x00, 0x16, 0x03, 0x06,
        0x01, 0x02, 0x03, 0x04, 0x81, 0x06, 0x00, 0x00,
        0x00, 0x00, 0x83, 0x06, 0x00, 0x00, 0x00, 0x00
    };

    buf = aos_zalloc(sizeof(encode_data));
    if (!buf) {
       aicx_info(0x15f99f, "", "[ppp] get mem fail\n");
       return AIC_ENOMEM;
    }

    packet_ptr = aic_packet_build(buf, sizeof(encode_data), 0,
                                  &ppp_pool_0,
                                  AIC_PACKET_DYNAMIC_MEM);

    memcpy(packet_ptr->nx_packet_prepend_ptr, encode_data, sizeof(encode_data));
    packet_ptr->nx_packet_ip_header = packet_ptr->nx_packet_prepend_ptr;
    packet_ptr->nx_packet_append_ptr = packet_ptr->nx_packet_prepend_ptr +
        sizeof(encode_data);
    packet_ptr->nx_packet_length = sizeof(encode_data);

    aic_ppp_encode_transmit_packet(packet_ptr, 0);
    aic_ppp_encode_update_in_fifo_wptr(1);

    return 0;
}
CONSOLE_EXPORT_CMD(ppp_encode_test, aic_ppp_encode_test_cmd, ppp encode test);
#endif
int aic_ppp_codec_init(void)
{
    struct aic_ppp_codec_driver *cfg = &aic_ppp_codec_drv_cfg;

    memset(cfg, 0 , sizeof(struct aic_ppp_codec_driver));

    aic_ppp_encode_register(&cfg->ppp_encode_phy);
    aic_ppp_decode_register(&cfg->ppp_decode_phy);

    cfg->send_thread_id = aos_create_thread("ppp codec send", aic_ppp_send_task,
                                            (void *)cfg, SIZE_1K, 0, NULL, 8,
                                            AOS_AUTO_START);
    if (cfg->send_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x10f094, "", "[PPP] create ppp send task fail\n");
        return AIC_ENOMEM;
    }

    cfg->recv_thread_id = aos_create_thread("ppp codec recv", aic_ppp_decode_recv_task,
                                            (void *)cfg, SIZE_1K, 0, NULL, 12,
                                            AOS_AUTO_START);
    if (cfg->recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x1abeaa, "", "[PPP] create ppp recv task fail\n");
        return AIC_ENOMEM;
    }

    cfg->encode_event = aos_create_event("ppp encode event");
    if (cfg->encode_event == AOS_NULL) {
        aicx_error(0x0ff8ba, "", "[PPP] create ppp encode event fail\n");
        return AIC_EPERM;
    }

    conn_ppp_register_recv_cb(aic_ppp_recv_task, cfg);

    return AIC_SUCCESS;
}

