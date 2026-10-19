
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0f5833
#define LOG_MODULE    AMOD_DRV
#include <aic_io.h>
#include <cache.h>

#include "ppp_codec_reg.h"
#include "aic_ppp_codec_core.h"

static struct aic_ppp_fifo_manage ppp_encode_fifo = {0};
static struct aic_ppp_fifo_manage ppp_decode_fifo = {0};

static int aic_ppp_encode_init(UISR_FUNC isr_f, void *param,
                               u32 in_fifo_addr, u32 out_fifo_addr,
                               u32 in_fifo_depth, u32 out_fifo_depth)
{
    int ret;
    u32 val;

    memset((void *)in_fifo_addr, 0,
           sizeof(struct aic_hw_ppp_des) * (1 << in_fifo_depth));
    memset((void *)out_fifo_addr, 0,
           sizeof(struct aic_hw_ppp_des) * (1 << out_fifo_depth));

    memset(&ppp_encode_fifo, 0, sizeof(ppp_encode_fifo));
    ppp_encode_fifo.in_fifo_addr = (void *)in_fifo_addr;
    ppp_encode_fifo.out_fifo_addr = (void *)out_fifo_addr;

    AIC_WRITE32(PPP_ENCODE_ENC_IN_FIFO_ADDR, in_fifo_addr);
    AIC_WRITE32(PPP_ENCODE_ENC_OUT_FIFO_ADDR, out_fifo_addr);

    AIC_WRITE32(PPP_ENCODE_ENC_CR0, 0x0BC40008);

    AIC_WRITE32(PPP_ENCODE_ENC_CR1, 0x00002800);
    //AIC_WRITE32(PPP_ENCODE_ENC_SHW, 0x1);

    AIC_WRITE32(PPP_ENCODE_GAXIM, 0x80000000);

    val = AIC_READ32(PPP_ENCODE_ENC_IN_FIFO1);
    val &= (~PPP_ENCODE_ENC_IN_FIFO1_TOTAL_DEPTH_MASK);
    val |= (in_fifo_depth << 16);
    AIC_WRITE32(PPP_ENCODE_ENC_IN_FIFO1, val);

    val = AIC_READ32(PPP_ENCODE_ENC_OUT_FIFO1);
    val &= (~PPP_ENCODE_ENC_OUT_FIFO1_TOTAL_DEPTH_MASK);
    val |= (out_fifo_depth << 16);
    AIC_WRITE32(PPP_ENCODE_ENC_OUT_FIFO1, val);

    ret = register_irq(PPP_ENCODE_IRQ_NUM, isr_f, param, NULL, NULL);
    if (ret) {
        aicx_error(0x199aa0, "x", "request encode irq fail 0x%x\n", ret);
        return ret;
    }

    return AIC_SUCCESS;
}

static void aic_ppp_encode_soft_reset()
{
    AIC_WRITE32(PPP_ENCODE_ENC_SW_RST, PPP_ENCODE_ENC_SW_RST_MASK);
}

static u32 aic_ppp_encode_phy_get_in_fifo_filled_depth()
{
    return (AIC_READ32(PPP_ENCODE_ENC_STATUS0) &
        PPP_ENCODE_FILLED_DEPTH_IN_MASK);
}

static u32 aic_ppp_encode_phy_get_out_fifo_filled_depth()
{
    return ((AIC_READ32(PPP_ENCODE_ENC_STATUS0) &
        PPP_ENCODE_FILLED_DEPTH_OUT_MASK) >> 16);
}

static int aic_ppp_encode_get_in_fifo_curr_des(struct aic_hw_ppp_des **des,
                                               u16 num)
{
    u16 index;

    if (num > PPP_ENCODE_FIFO_TOTAL_DEPTH) {
        aicx_error(0x0ba061, "x", "get encode in fifo des fail num 0x%x\n", num);
        return AIC_EINVAL;
    }

    index = (num + ppp_encode_fifo.in_fifo_wptr) &
        (PPP_ENCODE_FIFO_TOTAL_DEPTH - 1);

    *des = &ppp_encode_fifo.in_fifo_addr[index];

    dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(*des),
                        PPP_CODEC_CACHE_LINE_SIZE);

    return AIC_SUCCESS;
}

static int aic_ppp_encode_in_fifo_wptr_update(u16 num)
{
    u32 val;

    ppp_encode_fifo.in_fifo_wptr = (ppp_encode_fifo.in_fifo_wptr + num) &
        PPP_FIFO_TOTAL_DEPTH_MASK;

    val = AIC_READ32(PPP_ENCODE_ENC_IN_FIFO1);
    val &= (~PPP_ENCODE_ENC_IN_FIFO1_WPTR_MASK);
    val |= ppp_encode_fifo.in_fifo_wptr;
    AIC_WRITE32(PPP_ENCODE_ENC_IN_FIFO1, val);

    return AIC_SUCCESS;
}

static int aic_ppp_encode_get_out_fifo_curr_des(struct aic_hw_ppp_des **des,
                                                u16 num)
{
    u16 index;

    if (num > PPP_ENCODE_FIFO_TOTAL_DEPTH) {
        aicx_error(0x03e559, "x", "get encode out fifo des fail num 0x%x\n", num);
        return AIC_EINVAL;
    }

    index = (num + ppp_encode_fifo.out_fifo_rptr) &
        (PPP_ENCODE_FIFO_TOTAL_DEPTH - 1);

    *des = &ppp_encode_fifo.out_fifo_addr[index];
    dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(*des),
                        PPP_CODEC_CACHE_LINE_SIZE);

    return AIC_SUCCESS;
}

static int aic_ppp_encode_out_fifo_rptr_update(u16 num)
{
    u32 val;

    ppp_encode_fifo.out_fifo_rptr = (ppp_encode_fifo.out_fifo_rptr + num) &
        PPP_FIFO_TOTAL_DEPTH_MASK;

    val = AIC_READ32(PPP_ENCODE_ENC_OUT_FIFO1);
    val &= (~PPP_ENCODE_ENC_OUT_FIFO1_RPTR_MASK);
    val |= ppp_encode_fifo.out_fifo_rptr;
    AIC_WRITE32(PPP_ENCODE_ENC_OUT_FIFO1, val);

    return AIC_SUCCESS;
}

static int aic_ppp_encode_interrupt_config(u32 intr, u32 enable)
{
    u32 val;

    val = AIC_READ32(PPP_ENCODE_ENC_INT_ENABLE);

    if (enable)
        val |= intr;
    else
        val &= (~intr);

    AIC_WRITE32(PPP_ENCODE_ENC_INT_ENABLE, val);

    val = AIC_READ32(PPP_ENCODE_ENC_INT_ENABLE);

    if (enable) {
        if (!(val & intr)) {
            aicx_info(0x1ad6c7, "xd", "[PPP] encode intr config fail intr 0x%x, enable %d\n",
                       intr, enable);
            return AIC_EIO;
        }
    } else {
        if (val & intr) {
            aicx_info(0x1ad6c7, "xd", "[PPP] encode intr config fail intr 0x%x, enable %d\n",
                       intr, enable);
            return AIC_EIO;
        }
    }

    return AIC_SUCCESS;
}

static void aic_ppp_encode_clear_intr(u32 irq)
{
    AIC_WRITE32(PPP_ENCODE_ENC_INT_CLEAR, irq);
}

static int aic_ppp_encode_threshold_interrupt_config(u32 enable, u16 threshold)
{
    u32 val;

    aic_ppp_encode_clear_intr(AIC_PPP_THRESHOLD_INTR);

    val = AIC_READ32(PPP_ENCODE_ENC_CR0);
    val &= (~PPP_ENCODE_ENC_CR0_FIFO_THIRD_OUT_MASK);
    val |= threshold;
    AIC_WRITE32(PPP_ENCODE_ENC_CR0, val);

    return aic_ppp_encode_interrupt_config(AIC_PPP_THRESHOLD_INTR, enable);
}

static int aic_ppp_encode_timeout_interrupt_config(u32 enable, u16 threshold)
{
    u32 val;

    aic_ppp_encode_clear_intr(AIC_PPP_TIMEOUT_INTR);

    val = AIC_READ32(PPP_ENCODE_ENC_CR1);
    val &= (~PPP_ENCODE_ENC_CR1_FIFO_TIMER_OUT_MASK);
    val |= threshold;
    AIC_WRITE32(PPP_ENCODE_ENC_CR1, val);

    return aic_ppp_encode_interrupt_config(AIC_PPP_TIMEOUT_INTR, enable);
}

static int aic_ppp_encode_errcode_interrupt_config(u32 enable)
{
    aic_ppp_encode_clear_intr(AIC_PPP_ERRCODE_INTR);

    return aic_ppp_encode_interrupt_config(AIC_PPP_ERRCODE_INTR, enable);
}

static int aic_ppp_encode_flow_ctrl_interrupt_config(u32 enable)
{
    aic_ppp_encode_clear_intr(AIC_PPP_FLOW_CTRL_INTR);

    return aic_ppp_encode_interrupt_config(AIC_PPP_FLOW_CTRL_INTR, enable);
}

static int aic_ppp_encode_crc_error_interrupt_config(u32 enable)
{
    aic_ppp_encode_clear_intr(AIC_PPP_CRC_ERROR_INTR);

    return aic_ppp_encode_interrupt_config(AIC_PPP_CRC_ERROR_INTR, enable);
}

static u32 aic_ppp_encode_get_irq_status(void)
{
    return AIC_READ32(PPP_ENCODE_ENC_INT_OUT);
}

static void aic_ppp_encode_get_fifo_ptr(u16 *in_fifo_rptr, u16 *out_fifo_wptr)
{
    if (in_fifo_rptr)
        *in_fifo_rptr = AIC_READ32(PPP_ENCODE_ENC_IN_FIFO2) &
            PPP_ENCODE_ENC_IN_FIFO2_RPTR_MASK;

    if (out_fifo_wptr)
        *out_fifo_wptr = AIC_READ32(PPP_ENCODE_ENC_OUT_FIFO2) &
            PPP_ENCODE_ENC_OUT_FIFO2_WPTR_MASK;
}

static void aic_ppp_encode_get_in_fifo_addr(u32 *fifo_addr)
{
    if (fifo_addr)
        *fifo_addr = AIC_READ32(PPP_ENCODE_ENC_IN_FIFO_ADDR);
}

static int aic_ppp_decode_init(UISR_FUNC isr_f, void *param,
                               u32 in_fifo_addr, u32 out_fifo_addr,
                               u32 in_fifo_depth, u32 out_fifo_depth)
{
    int ret;
    u32 val;

    memset((void *)in_fifo_addr, 0,
           sizeof(struct aic_hw_ppp_des) * (1 << in_fifo_depth));
    memset((void *)out_fifo_addr, 0,
           sizeof(struct aic_hw_ppp_des) * (1 << out_fifo_depth));

    memset(&ppp_decode_fifo, 0, sizeof(ppp_decode_fifo));
    ppp_decode_fifo.in_fifo_addr = (void *)in_fifo_addr;
    ppp_decode_fifo.out_fifo_addr = (void *)out_fifo_addr;

    AIC_WRITE32(PPP_DECODE_ENC_IN_FIFO_ADDR, in_fifo_addr);
    AIC_WRITE32(PPP_DECODE_ENC_OUT_FIFO_ADDR, out_fifo_addr);

    AIC_WRITE32(PPP_DECODE_ENC_CR0, 0x0BC40008);

    AIC_WRITE32(PPP_DECODE_ENC_CR1, 0x00002800);

    //AIC_WRITE32(PPP_DECODE_ENC_SHW, 0x1);

    AIC_WRITE32(PPP_DECODE_GAXIM, 0x80000000);

    val = AIC_READ32(PPP_DECODE_ENC_IN_FIFO1);
    val &= (~PPP_DECODE_ENC_IN_FIFO1_TOTAL_DEPTH_MASK);
    val |= (in_fifo_depth << 16);
    AIC_WRITE32(PPP_DECODE_ENC_IN_FIFO1, val);

    val = AIC_READ32(PPP_DECODE_ENC_OUT_FIFO1);
    val &= (~PPP_DECODE_ENC_OUT_FIFO1_TOTAL_DEPTH_MASK);
    val |= (out_fifo_depth << 16);
    AIC_WRITE32(PPP_DECODE_ENC_OUT_FIFO1, val);

    ret = register_irq(PPP_DECODE_IRQ_NUM, isr_f, param, NULL, NULL);
    if (ret) {
        aicx_error(0x0507dc, "x", "request decode irq fail 0x%x\n", ret);
        return ret;
    }

    return AIC_SUCCESS;
}

static void aic_ppp_decode_soft_reset()
{
    AIC_WRITE32(PPP_DECODE_ENC_SW_RST, PPP_DECODE_ENC_SW_RST_MASK);
}

static u32 aic_ppp_decode_phy_get_in_fifo_filled_depth()
{
    return (AIC_READ32(PPP_DECODE_ENC_STATUS0) &
        PPP_DECODE_FILLED_DEPTH_IN_MASK);
}

static u32 aic_ppp_decode_phy_get_out_fifo_filled_depth()
{
    return ((AIC_READ32(PPP_DECODE_ENC_STATUS0) &
        PPP_DECODE_FILLED_DEPTH_OUT_MASK) >> 16);
}

static int aic_ppp_decode_get_in_fifo_curr_des(struct aic_hw_ppp_des **des,
                                               u16 num)
{
    u16 index;

    if (num > PPP_DECODE_FIFO_TOTAL_DEPTH) {
        aicx_error(0x1958fc, "x", "get decode in fifo des fail num 0x%x\n", num);
        return AIC_EINVAL;
    }

    index = (num + ppp_decode_fifo.in_fifo_wptr) &
        (PPP_DECODE_FIFO_TOTAL_DEPTH - 1);

    *des = &ppp_decode_fifo.in_fifo_addr[index];
    dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(*des),
                        PPP_CODEC_CACHE_LINE_SIZE);

    return AIC_SUCCESS;
}

static int aic_ppp_decode_in_fifo_wptr_update(u16 num)
{
    u32 val;

    ppp_decode_fifo.in_fifo_wptr = (ppp_decode_fifo.in_fifo_wptr + num) &
        PPP_FIFO_TOTAL_DEPTH_MASK;

    val = AIC_READ32(PPP_DECODE_ENC_IN_FIFO1);
    val &= (~PPP_DECODE_ENC_IN_FIFO1_WPTR_MASK);
    val |= ppp_decode_fifo.in_fifo_wptr;
    AIC_WRITE32(PPP_DECODE_ENC_IN_FIFO1, val);

    return AIC_SUCCESS;
}

static int aic_ppp_decode_get_out_fifo_curr_des(struct aic_hw_ppp_des **des,
                                                u16 num)
{
    u16 index;

    if (num > PPP_DECODE_FIFO_TOTAL_DEPTH) {
        aicx_error(0x10f92a, "x", "get decode out fifo des fail num 0x%x\n", num);
        return AIC_EINVAL;
    }

    index = (num + ppp_decode_fifo.out_fifo_rptr) &
        (PPP_DECODE_FIFO_TOTAL_DEPTH - 1);

    *des = &ppp_decode_fifo.out_fifo_addr[index];
    dcache_inv_by_range(PPP_GET_CACHE_LINE_ADDR(*des),
                        PPP_CODEC_CACHE_LINE_SIZE);

    return AIC_SUCCESS;
}

static int aic_ppp_decode_out_fifo_rptr_update(u16 num)
{
    u32 val;

    ppp_decode_fifo.out_fifo_rptr = (ppp_decode_fifo.out_fifo_rptr + num) &
        PPP_FIFO_TOTAL_DEPTH_MASK;

    val = AIC_READ32(PPP_DECODE_ENC_OUT_FIFO1);
    val &= (~PPP_DECODE_ENC_OUT_FIFO1_RPTR_MASK);
    val |= ppp_decode_fifo.out_fifo_rptr;
    AIC_WRITE32(PPP_DECODE_ENC_OUT_FIFO1, val);

    return AIC_SUCCESS;
}

static int aic_ppp_decode_interrupt_config(u32 intr, u32 enable)
{
    u32 val;

    val = AIC_READ32(PPP_DECODE_ENC_INT_ENABLE);

    if (enable)
        val |= intr;
    else
        val &= (~intr);

    AIC_WRITE32(PPP_DECODE_ENC_INT_ENABLE, val);

    val = AIC_READ32(PPP_DECODE_ENC_INT_ENABLE);

    if (enable) {
        if (!(val & intr)) {
            aicx_info(0x0b1c19, "xd", "[PPP] decode intr config fail intr 0x%x, enable %d\n",
                       intr, enable);
            return AIC_EIO;
        }
    } else {
        if (val & intr) {
            aicx_info(0x0b1c19, "xd", "[PPP] decode intr config fail intr 0x%x, enable %d\n",
                       intr, enable);
            return AIC_EIO;
        }
    }

    return AIC_SUCCESS;
}

static void aic_ppp_decode_clear_intr(u32 irq)
{
    AIC_WRITE32(PPP_DECODE_ENC_INT_CLEAR, irq);
}

static int aic_ppp_decode_threshold_interrupt_config(u32 enable, u16 threshold)
{
    u32 val;

    aic_ppp_decode_clear_intr(AIC_PPP_THRESHOLD_INTR);

    val = AIC_READ32(PPP_DECODE_ENC_CR0);
    val &= ~PPP_DECODE_ENC_CR0_FIFO_THIRD_OUT_MASK;
    val |= threshold;
    AIC_WRITE32(PPP_DECODE_ENC_CR0, val);

    return aic_ppp_decode_interrupt_config(AIC_PPP_THRESHOLD_INTR, enable);
}

static int aic_ppp_decode_timeout_interrupt_config(u32 enable, u16 threshold)
{
    u32 val;

    aic_ppp_decode_clear_intr(AIC_PPP_TIMEOUT_INTR);

    val = AIC_READ32(PPP_DECODE_ENC_CR1);
    val &= ~PPP_DECODE_ENC_CR1_FIFO_TIMER_OUT_MASK;
    val |= threshold;
    AIC_WRITE32(PPP_DECODE_ENC_CR1, val);

    return aic_ppp_decode_interrupt_config(AIC_PPP_TIMEOUT_INTR, enable);
}

static int aic_ppp_decode_errcode_interrupt_config(u32 enable)
{
    aic_ppp_decode_clear_intr(AIC_PPP_ERRCODE_INTR);

    return aic_ppp_decode_interrupt_config(AIC_PPP_ERRCODE_INTR, enable);
}

static int aic_ppp_decode_flow_ctrl_interrupt_config(u32 enable)
{
    aic_ppp_decode_clear_intr(AIC_PPP_FLOW_CTRL_INTR);

    return aic_ppp_decode_interrupt_config(AIC_PPP_FLOW_CTRL_INTR, enable);
}

static int aic_ppp_decode_crc_error_interrupt_config(u32 enable)
{
    aic_ppp_decode_clear_intr(AIC_PPP_CRC_ERROR_INTR);

    return aic_ppp_decode_interrupt_config(AIC_PPP_CRC_ERROR_INTR, enable);
}

static u32 aic_ppp_decode_get_irq_status(void)
{
    return AIC_READ32(PPP_DECODE_ENC_INT_OUT);
}

static void aic_ppp_decode_get_fifo_ptr(u16 *in_fifo_rptr, u16 *out_fifo_wptr)
{
    if (in_fifo_rptr)
        *in_fifo_rptr = AIC_READ32(PPP_DECODE_ENC_IN_FIFO2) &
            PPP_DECODE_ENC_IN_FIFO2_RPTR_MASK;

    if (out_fifo_wptr)
        *out_fifo_wptr = AIC_READ32(PPP_DECODE_ENC_OUT_FIFO2) &
            PPP_DECODE_ENC_OUT_FIFO2_WPTR_MASK;
}

static void aic_ppp_decode_get_in_fifo_addr(u32 *fifo_addr)
{
    if (fifo_addr)
        *fifo_addr = AIC_READ32(PPP_DECODE_ENC_IN_FIFO_ADDR);
}

void aic_ppp_encode_register(struct aic_ppp_codec_phy_ops *ppp_encode_ops)
{
    ppp_encode_ops->init = aic_ppp_encode_init;
    ppp_encode_ops->soft_reset = aic_ppp_encode_soft_reset;
    ppp_encode_ops->get_in_fifo_filled_depth =
        aic_ppp_encode_phy_get_in_fifo_filled_depth;
    ppp_encode_ops->get_out_fifo_filled_depth =
        aic_ppp_encode_phy_get_out_fifo_filled_depth;
    ppp_encode_ops->in_fifo_wptr_update =
        aic_ppp_encode_in_fifo_wptr_update;
    ppp_encode_ops->out_fifo_rptr_update =
        aic_ppp_encode_out_fifo_rptr_update;
    ppp_encode_ops->clear_intr = aic_ppp_encode_clear_intr;
    ppp_encode_ops->interrupt_config = aic_ppp_encode_interrupt_config;
    ppp_encode_ops->threshold_irq_config =
        aic_ppp_encode_threshold_interrupt_config;
    ppp_encode_ops->timeout_irq_config =
        aic_ppp_encode_timeout_interrupt_config;
    ppp_encode_ops->errcode_irq_config =
        aic_ppp_encode_errcode_interrupt_config;
    ppp_encode_ops->flow_ctrl_irq_config =
        aic_ppp_encode_flow_ctrl_interrupt_config;
    ppp_encode_ops->crc_error_irq_config =
        aic_ppp_encode_crc_error_interrupt_config;
    ppp_encode_ops->get_irq_status = aic_ppp_encode_get_irq_status;
    ppp_encode_ops->get_in_fifo_curr_des = aic_ppp_encode_get_in_fifo_curr_des;
    ppp_encode_ops->get_out_fifo_curr_des =
        aic_ppp_encode_get_out_fifo_curr_des;
    ppp_encode_ops->get_fifo_ptr = aic_ppp_encode_get_fifo_ptr;
    ppp_encode_ops->get_in_fifo_addr = aic_ppp_encode_get_in_fifo_addr;
}

void aic_ppp_decode_register(struct aic_ppp_codec_phy_ops *ppp_decode_ops)
{
    ppp_decode_ops->init = aic_ppp_decode_init;
    ppp_decode_ops->soft_reset = aic_ppp_decode_soft_reset;
    ppp_decode_ops->get_in_fifo_filled_depth =
        aic_ppp_decode_phy_get_in_fifo_filled_depth;
    ppp_decode_ops->get_out_fifo_filled_depth =
        aic_ppp_decode_phy_get_out_fifo_filled_depth;
    ppp_decode_ops->in_fifo_wptr_update =
        aic_ppp_decode_in_fifo_wptr_update;
    ppp_decode_ops->out_fifo_rptr_update =
        aic_ppp_decode_out_fifo_rptr_update;
    ppp_decode_ops->clear_intr = aic_ppp_decode_clear_intr;
    ppp_decode_ops->interrupt_config = aic_ppp_decode_interrupt_config;
    ppp_decode_ops->threshold_irq_config =
        aic_ppp_decode_threshold_interrupt_config;
    ppp_decode_ops->timeout_irq_config =
        aic_ppp_decode_timeout_interrupt_config;
    ppp_decode_ops->errcode_irq_config =
        aic_ppp_decode_errcode_interrupt_config;
    ppp_decode_ops->flow_ctrl_irq_config =
        aic_ppp_decode_flow_ctrl_interrupt_config;
    ppp_decode_ops->crc_error_irq_config =
        aic_ppp_decode_crc_error_interrupt_config;
    ppp_decode_ops->get_irq_status = aic_ppp_decode_get_irq_status;
    ppp_decode_ops->get_in_fifo_curr_des = aic_ppp_decode_get_in_fifo_curr_des;
    ppp_decode_ops->get_out_fifo_curr_des =
        aic_ppp_decode_get_out_fifo_curr_des;
    ppp_decode_ops->get_fifo_ptr = aic_ppp_decode_get_fifo_ptr;
    ppp_decode_ops->get_in_fifo_addr = aic_ppp_decode_get_in_fifo_addr;
}

