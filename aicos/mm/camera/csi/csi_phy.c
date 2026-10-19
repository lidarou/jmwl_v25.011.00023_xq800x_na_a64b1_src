/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0ad1e3
#define LOG_MODULE    AMOD_CAMERA

#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_size.h>
#include <mcu_glb.h>
#include <aic_auto_reg.h>
#include <aic_osa.h>

#include "csi.h"

#define AIC_CSI_DEBUG
#ifdef AIC_CSI_DEBUG
#include <timer_inc.h>
u32 csi_stop_time = 0;
#endif

void csi_enable(void)
{
    AIC_WRITE32(REG_MCU_GLB_CSI_EB_SET, BIT_MCU_GLB_CSI_EB);
    AIC_WRITE32(REG_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI_SET, BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI);
#ifdef REG_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI_CLR
    AIC_WRITE32(REG_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI_CLR, BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI);
#else
    AIC_WRITE32(REG_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI_SET, ~BIT_MCU_GLB_SW_RST_GLB_CLK_MCU_CSI);
#endif
}

void csi_disable(void)
{
#ifdef REG_MCU_GLB_CSI_EB_CLR
    AIC_WRITE32(REG_MCU_GLB_CSI_EB_CLR, BIT_MCU_GLB_CSI_EB);
#else
    AIC_WRITE32(REG_MCU_GLB_CSI_EB_SET, ~BIT_MCU_GLB_CSI_EB);
#endif
}

int csi_start(void)
{
    AIC_WRITE32(REG_CSI_CSI_START, BIT_CSI_CSI_ENABLE);

    return AIC_SUCCESS;
}

void csi_stop(void)
{
    int val;
#ifdef AIC_CSI_DEBUG
    u32 ts1, ts2;

    csi_stop_time = 0;
    ts1 = aic_get_timestamp();
#endif

    val = AIC_READ32(REG_CSI_CSI_START);
    val &= ~BIT_CSI_CSI_ENABLE;
    AIC_WRITE32(REG_CSI_CSI_START, val);

    if (aos_is_user_thread_context()) {
        /* wait until csi idle */
        while(1) {
            if (!(AIC_READ32(REG_CSI_CSI_STATUS0) & BIT(26))) {
#ifdef AIC_CSI_DEBUG
                csi_stop_time = 0;
#endif
                break;
            }

            aos_sleep(1);
#ifdef AIC_CSI_DEBUG
            ts2 = aic_get_timestamp();
            if (ts2 > ts1)
                csi_stop_time = ts2-ts1;
            else
                csi_stop_time = 0xffffffff - ts1 + ts2;
#endif
        }
    }
}

int is_csi_enabled(void)
{
    return AIC_READ32(REG_CSI_CSI_START) & BIT_CSI_CSI_ENABLE;
}

/* a debug mode, csi will count the width of image line to decide EOL */
int csi_cnt_mode_enable(boolean enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (enable)
        val |= BIT_CSI_WIDTH_EN;
    else
        val &= ~BIT_CSI_WIDTH_EN;

    AIC_WRITE32(REG_CSI_CSI_CR0, BIT_CSI_WIDTH_EN);

    return AIC_SUCCESS;
}

/* debug mode should config image width and pixel size */
int csi_cnt_mode_config(u32 image_width, u32 pixel_size)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    val &= ~BIT_CSI_IMG_WIDTH(0xfff);
    val |= BIT_CSI_IMG_WIDTH(image_width);

    if (pixel_size)
        val |= BIT_CSI_PXL_BYTE;
    else
        val &= ~BIT_CSI_PXL_BYTE;

    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

/* config data endian */
int csi_endian_mode_config(u32 endian)
{
    int val;

    if (endian > 3)
        return AIC_EINVAL;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    val &= ~BIT_CSI_ENDIAN(0x3);
    val |= BIT_CSI_ENDIAN(endian);

    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_lsb_mode(u32 mode)
{
     int val;

     val = AIC_READ32(REG_CSI_CSI_CR0);
     /* LSB first */
     if (mode)
         val |= BIT_CSI_LSB_FIRST;
     else
         val &= ~BIT_CSI_LSB_FIRST;
     AIC_WRITE32(REG_CSI_CSI_CR0, val);

     return AIC_SUCCESS;
}

/* used to switch line output */
int csi_wire_swap(u32 swap)
{
     int val;

     val = AIC_READ32(REG_CSI_CSI_CR0);
     if (swap)
         val |= BIT_CSI_WIRE_SWAP;
     else
         val &= ~BIT_CSI_WIRE_SWAP;
     AIC_WRITE32(REG_CSI_CSI_CR0, val);

     return AIC_SUCCESS;
}

int csi_wire_num_config(u32 wire_num)
{
    int val;

    if ((wire_num > 4) || (wire_num == 3))
        return AIC_EINVAL;
    wire_num = wire_num - 1;
    val = AIC_READ32(REG_CSI_CSI_CR0);
    val &= ~BIT_CSI_WIRE_TYPE(0x3);
    val |= BIT_CSI_WIRE_TYPE(wire_num);
    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_crc_enable(int enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (enable)
        val |= BIT_CSI_CRC_EN;
    else
        val &= ~BIT_CSI_CRC_EN;
    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_mformat_enable(int enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (enable)
        val |= BIT_CSI_MFORMAT_EN;
    else
        val &= ~BIT_CSI_MFORMAT_EN;
    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

/* 1 use csi internal frame valid signal, 0 use csi EOF sync code */
int csi_eof_int_enable(u32 enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (enable)
        val |= BIT_CSI_INT_PULSE_SEL;
    else
        val &= ~BIT_CSI_INT_PULSE_SEL;
    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_debug_enable(int enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_SYNC);
    if (enable)
        val |= BIT_CSI_DEBUG_EN;
    else
        val &= ~BIT_CSI_DEBUG_EN;
    AIC_WRITE32(REG_CSI_CSI_SYNC, val);

    return AIC_SUCCESS;

}

int csi_cap_mode_enable(int enable)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_SYNC);
    if (enable)
        val |= BIT_CSI_CAP_MODE;
    else
        val &= ~BIT_CSI_CAP_MODE;
    AIC_WRITE32(REG_CSI_CSI_SYNC, val);

    return AIC_SUCCESS;
}

/* only useful for YUV, Y output as one plane, UV output as another plane */
int csi_output_plane_config(u32 plane_mode)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (plane_mode)
        val |= BIT_CSI_PLANE2_FLAG;
    else
        val &= ~BIT_CSI_PLANE2_FLAG;

    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_ddr_mode_config(u32 ddr_mode)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_CR0);
    if (ddr_mode)
        val |= BIT_CSI_DDR_MODE;
    else
        val &= ~BIT_CSI_DDR_MODE;

    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

/* unit is byte */
int csi_rxfifo_thrd_config(int size)
{
    int val;

    if (size > 0x1f)
        return AIC_EINVAL;

    val = AIC_READ32(REG_CSI_CSI_CR0);

    val &= ~BIT_CSI_RXFIFO_THRD(0x1f);
    val |= BIT_CSI_RXFIFO_THRD(size);
    AIC_WRITE32(REG_CSI_CSI_CR0, val);

    return AIC_SUCCESS;
}

int csi_set_sync_word_head(u32 sync_word)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_SYNC);
    val &= ~BIT_CSI_SYNC_WORD(0xffffff);
    val |= BIT_CSI_SYNC_WORD(sync_word);
    AIC_WRITE32(REG_CSI_CSI_SYNC, val);

    return AIC_SUCCESS;
}

int csi_set_sync_word_id(u32 sof, u32 sol, u32 eol, u32 eof)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_PACKET);
    val &= ~(BIT_CSI_SOF_WORD(0xff) | BIT_CSI_SOL_WORD(0xff) |
             BIT_CSI_EOL_WORD(0xff) | BIT_CSI_EOF_WORD(0xff));
    val |= BIT_CSI_SOF_WORD(sof) | BIT_CSI_SOL_WORD(sol) |
           BIT_CSI_EOL_WORD(eol) | BIT_CSI_EOF_WORD(eof);
    AIC_WRITE32(REG_CSI_CSI_PACKET, val);

    return AIC_SUCCESS;
}

int csi_frame_addr_config(u32 addr)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_FRAME_ADDR);
    addr &= ~(BIT(0) | BIT(1));
    val = addr;
    AIC_WRITE32(REG_CSI_CSI_FRAME_ADDR, val);

    return AIC_SUCCESS;
}

u32 csi_get_frame_addr_config(void)
{
    return AIC_READ32(REG_CSI_CSI_FRAME_ADDR);
}

int csi_frame_addr2_config(u32 addr2)
{
    int val;

    val = AIC_READ32(REG_CSI_CSI_FRAME_ADDR_2P);
    addr2 &= ~(BIT(0) | BIT(1));
    val = addr2;
    AIC_WRITE32(REG_CSI_CSI_FRAME_ADDR_2P, val);

    return AIC_SUCCESS;
}

/* csi cannot write data to memory more than the size */
int csi_set_addr_limit(u32 image_size)
{
    int val, sz_4k;

    sz_4k = image_size / SIZE_4K;
    if (image_size % SIZE_4K)
        sz_4k++;

    val = AIC_READ32(REG_CSI_CSI_ADDR_LIMIT);
    val &= ~BIT_CSI_MEM_MAX_SIZE(0xff);
    val |= sz_4k;
    AIC_WRITE32(REG_CSI_CSI_ADDR_LIMIT, val);

    return AIC_SUCCESS;
}

void csi_int_unmask_all(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_MASK, ~(BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5));
}

void csi_int_mask_all(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_MASK, BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5);
}

void csi_int_clear_all(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5);
}

void csi_int_mask_sof(void)
{
    u32 val;

    val = AIC_READ32(REG_CSI_CSI_INT_MASK);
    AIC_WRITE32(REG_CSI_CSI_INT_MASK, val | BIT_CSI_START_OF_FRAME);
}

void csi_int_mask_eol(void)
{
    u32 val;

    val = AIC_READ32(REG_CSI_CSI_INT_MASK);
    AIC_WRITE32(REG_CSI_CSI_INT_MASK, val | BIT_CSI_EOL);
}

void csi_int_mask_sol(void)
{
    u32 val;

    val = AIC_READ32(REG_CSI_CSI_INT_MASK);
    AIC_WRITE32(REG_CSI_CSI_INT_MASK, val | BIT_CSI_SOL);
}

void csi_int_clear_eof(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_END_OF_FRAME);
}

void csi_int_clear_sof(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_START_OF_FRAME);
}

void csi_int_clear_rxfifo_full(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_START_RX_IS_FULL);
}

void csi_int_clear_timeout(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_TIMEOUT);
}

void csi_int_clear_eol(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_EOL);
}

void csi_int_clear_sol(void)
{
    AIC_WRITE32(REG_CSI_CSI_INT_CLEAR, BIT_CSI_SOL);
}
