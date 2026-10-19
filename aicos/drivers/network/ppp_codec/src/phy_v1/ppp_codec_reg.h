#ifndef __PPP_CODEC_REG_H__
#define __PPP_CODEC_REG_H__

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_bitmap.h>
#include <irq.h>
#include <reg_base.h>
#include <mcu_glb.h>
#include <ppp_cfg.h>

struct aic_hw_ppp_des {
    u64 address : 36;
    u16 in_offset : 12;
    u16 in_length : 12;
    u32 out_offset : 24;
    u16 out_length : 12;
    u8 seq;
    u8 errcode : 3;
    u8 check : 1;
    u32 reserved : 20;
} __attribute__((packed));

struct aic_ppp_fifo_manage {
    u16 in_fifo_rptr;
    u16 in_fifo_wptr;
    u16 out_fifo_rptr;
    u16 out_fifo_wptr;

    struct aic_hw_ppp_des *in_fifo_addr;
    struct aic_hw_ppp_des *out_fifo_addr;
};

#define PPP_ENCODE_FIFO_TOTAL_DEPTH               128
#define PPP_ENCODE_FIFO_TOTAL_POWER_DEPTH         7

#define PPP_DECODE_FIFO_TOTAL_DEPTH               128
#define PPP_DECODE_FIFO_TOTAL_POWER_DEPTH         7

#define PPP_ENCODE_ENC_START_MASK                 BIT(0)
#define PPP_ENCODE_ENC_START                      (PPP_ENCODE_REG_START + 0x00)

#define PPP_ENCODE_ENC_IN_FIFO_ADDR               (PPP_ENCODE_REG_START + 0x04)

#define PPP_ENCODE_ENC_IN_FIFO1_TOTAL_DEPTH_MASK  GENMASK(16, 19)
#define PPP_ENCODE_ENC_IN_FIFO1_WPTR_MASK         GENMASK(0, 15)
#define PPP_ENCODE_ENC_IN_FIFO1                   (PPP_ENCODE_REG_START + 0x08)

#define PPP_ENCODE_ENC_IN_FIFO2_RPTR_MASK         GENMASK(0, 15)
#define PPP_ENCODE_ENC_IN_FIFO2                   (PPP_ENCODE_REG_START + 0x0C)

#define PPP_ENCODE_ENC_OUT_FIFO_ADDR              (PPP_ENCODE_REG_START + 0x10)

#define PPP_ENCODE_ENC_OUT_FIFO1_TOTAL_DEPTH_MASK GENMASK(16, 19)
#define PPP_ENCODE_ENC_OUT_FIFO1_RPTR_MASK        GENMASK(0, 15)
#define PPP_ENCODE_ENC_OUT_FIFO1                  (PPP_ENCODE_REG_START + 0x14)

#define PPP_ENCODE_ENC_OUT_FIFO2_WPTR_MASK        GENMASK(0, 15)
#define PPP_ENCODE_ENC_OUT_FIFO2                  (PPP_ENCODE_REG_START + 0x18)

#define PPP_ENCODE_ENC_CR0_MAP_BYPASS_MASK        BIT(31)
#define PPP_ENCODE_ENC_CR0_MAX_SIZE_MASK          GENMASK(16, 27)
#define PPP_ENCODE_ENC_CR0_FIFO_THIRD_OUT_MASK    GENMASK(0, 7)
#define PPP_ENCODE_ENC_CR0                        (PPP_ENCODE_REG_START + 0x1C)

#define PPP_ENCODE_ENC_CR1_FIFO_TIMER_OUT_MASK    GENMASK(0, 15)
#define PPP_ENCODE_ENC_CR1                        (PPP_ENCODE_REG_START + 0x20)

#define PPP_ENCODE_ENC_FORCE_SHW_MASK             BIT(0)
#define PPP_ENCODE_ENC_SHW                        (PPP_ENCODE_REG_START + 0x24)

#define PPP_ENCODE_ENC_INT_OUT                    (PPP_ENCODE_REG_START + 0x28)

#define PPP_ENCODE_ENC_INT_ENABLE                 (PPP_ENCODE_REG_START + 0x2C)

#define PPP_ENCODE_ENC_INT_CLEAR                  (PPP_ENCODE_REG_START + 0x30)

#define PPP_ENCODE_ENC_INT_RAW_MASK               GENMASK(0, 15)
#define PPP_ENCODE_ENC_INT_RAW                    (PPP_ENCODE_REG_START + 0x34)

#define PPP_ENCODE_ENC_SW_RST_MASK                BIT(0)
#define PPP_ENCODE_ENC_SW_RST                     (PPP_ENCODE_REG_START + 0x38)

#define PPP_ENCODE_AXIM_DBG_EN_MASK               BIT(31)
#define PPP_ENCODE_AXIM_AMXLEN_8_MASK             BIT(30)
#define PPP_ENCODE_AXIM_TRANS_STOP_MASK           BIT(29)
#define PPP_ENCODE_AXIM_W_OUTSTANDING_MASK        GENMASK(16, 23)
#define PPP_ENCODE_AXIM_R_OUTSTANDING_MASK        GENMASK(8, 15)
#define PPP_ENCODE_AXIM_INTERVAL_CYCLE_MASK       GENMASK(0, 7)
#define PPP_ENCODE_GAXIM                          (PPP_ENCODE_REG_START + 0x3C)

#define PPP_ENCODE_AXIM_STATUS0                   (PPP_ENCODE_REG_START + 0x40)

#define PPP_ENCODE_AXIM_STATUS1                   (PPP_ENCODE_REG_START + 0x44)

#define PPP_ENCODE_AXIM_STATUS2                   (PPP_ENCODE_REG_START + 0x48)

#define PPP_ENCODE_AXIM_STATUS3                   (PPP_ENCODE_REG_START + 0x4C)

#define PPP_ENCODE_FILLED_DEPTH_OUT_MASK          GENMASK(16, 31)
#define PPP_ENCODE_FILLED_DEPTH_IN_MASK           GENMASK(0, 15)
#define PPP_ENCODE_ENC_STATUS0                    (PPP_ENCODE_REG_START + 0x50)

#define PPP_ENCODE_CRC_WRONG_WPTR                 GENMASK(16, 31)
#define PPP_ENCODE_CRC_WRONG_VALUE                GENMASK(0, 15)
#define PPP_ENCODE_ENC_STATUS1                    (PPP_ENCODE_REG_START + 0x54)

#define PPP_DECODE_ENC_START_MASK                 BIT(0)
#define PPP_DECODE_ENC_START                      (PPP_DECODE_REG_START + 0x00)

#define PPP_DECODE_ENC_IN_FIFO_ADDR               (PPP_DECODE_REG_START + 0x04)

#define PPP_DECODE_ENC_IN_FIFO1_TOTAL_DEPTH_MASK  GENMASK(16, 19)
#define PPP_DECODE_ENC_IN_FIFO1_WPTR_MASK         GENMASK(0, 15)
#define PPP_DECODE_ENC_IN_FIFO1                   (PPP_DECODE_REG_START + 0x08)

#define PPP_DECODE_ENC_IN_FIFO2_RPTR_MASK         GENMASK(0, 15)
#define PPP_DECODE_ENC_IN_FIFO2                   (PPP_DECODE_REG_START + 0x0C)

#define PPP_DECODE_ENC_OUT_FIFO_ADDR              (PPP_DECODE_REG_START + 0x10)

#define PPP_DECODE_ENC_OUT_FIFO1_TOTAL_DEPTH_MASK GENMASK(16, 19)
#define PPP_DECODE_ENC_OUT_FIFO1_RPTR_MASK        GENMASK(0, 15)
#define PPP_DECODE_ENC_OUT_FIFO1                  (PPP_DECODE_REG_START + 0x14)

#define PPP_DECODE_ENC_OUT_FIFO2_WPTR_MASK        GENMASK(0, 15)
#define PPP_DECODE_ENC_OUT_FIFO2                  (PPP_DECODE_REG_START + 0x18)

#define PPP_DECODE_ENC_CR0_MAP_BYPASS_MASK        BIT(31)
#define PPP_DECODE_ENC_CR0_MAX_SIZE_MASK          GENMASK(16, 27)
#define PPP_DECODE_ENC_CR0_FIFO_THIRD_OUT_MASK    GENMASK(0, 7)
#define PPP_DECODE_ENC_CR0                        (PPP_DECODE_REG_START + 0x1C)

#define PPP_DECODE_ENC_CR1_FIFO_TIMER_OUT_MASK    GENMASK(0, 15)
#define PPP_DECODE_ENC_CR1                        (PPP_DECODE_REG_START + 0x20)

#define PPP_DECODE_ENC_FORCE_SHW_MASK             BIT(0)
#define PPP_DECODE_ENC_SHW                        (PPP_DECODE_REG_START + 0x24)

#define PPP_DECODE_ENC_INT_OUT                    (PPP_DECODE_REG_START + 0x28)

#define PPP_DECODE_ENC_INT_ENABLE                 (PPP_DECODE_REG_START + 0x2C)

#define PPP_DECODE_ENC_INT_CLEAR                  (PPP_DECODE_REG_START + 0x30)

#define PPP_DECODE_ENC_INT_RAW_MASK               GENMASK(0, 15)
#define PPP_DECODE_ENC_INT_RAW                    (PPP_DECODE_REG_START + 0x34)

#define PPP_DECODE_ENC_SW_RST_MASK                BIT(0)
#define PPP_DECODE_ENC_SW_RST                     (PPP_DECODE_REG_START + 0x38)

#define PPP_DECODE_AXIM_DBG_EN_MASK               BIT(31)
#define PPP_DECODE_AXIM_AMXLEN_8_MASK             BIT(30)
#define PPP_DECODE_AXIM_TRANS_STOP_MASK           BIT(29)
#define PPP_DECODE_AXIM_W_OUTSTANDING_MASK        GENMASK(16, 23)
#define PPP_DECODE_AXIM_R_OUTSTANDING_MASK        GENMASK(8, 15)
#define PPP_DECODE_AXIM_INTERVAL_CYCLE_MASK       GENMASK(0, 7)
#define PPP_DECODE_GAXIM                          (PPP_DECODE_REG_START + 0x3C)

#define PPP_DECODE_AXIM_STATUS0                   (PPP_DECODE_REG_START + 0x40)

#define PPP_DECODE_AXIM_STATUS1                   (PPP_DECODE_REG_START + 0x44)

#define PPP_DECODE_AXIM_STATUS2                   (PPP_DECODE_REG_START + 0x48)

#define PPP_DECODE_AXIM_STATUS3                   (PPP_DECODE_REG_START + 0x4C)

#define PPP_DECODE_FILLED_DEPTH_OUT_MASK          GENMASK(16, 31)
#define PPP_DECODE_FILLED_DEPTH_IN_MASK           GENMASK(0, 15)
#define PPP_DECODE_ENC_STATUS0                    (PPP_DECODE_REG_START + 0x50)

#define PPP_DECODE_CRC_WRONG_WPTR                 GENMASK(16, 31)
#define PPP_DECODE_CRC_WRONG_VALUE                GENMASK(0, 15)
#define PPP_DECODE_ENC_STATUS1                    (PPP_DECODE_REG_START + 0x54)

#define PPP_FIFO_TOTAL_DEPTH_MASK                 0xFFFF

#define AIC_PPP_THRESHOLD_INTR                    BIT(0)
#define AIC_PPP_TIMEOUT_INTR                      BIT(1)
#define AIC_PPP_ERRCODE_INTR                      BIT(2)
#define AIC_PPP_FLOW_CTRL_INTR                    BIT(3)
#define AIC_PPP_CRC_ERROR_INTR                    BIT(4)

enum aic_ppp_errcode {
    AIC_NON_PPP = 1,
    AIC_LENGTH_ERR = 2,
};

struct aic_ppp_codec_phy_ops {
    int (*init)(UISR_FUNC isr_f, void *param,
                u32 in_fifo_addr, u32 out_fifo_addr,
                u32 in_fifo_depth, u32 out_fifo_depth);
    void (*soft_reset)(void);
    u32 (*get_in_fifo_filled_depth)(void);
    u32 (*get_out_fifo_filled_depth)(void);
    int (*in_fifo_wptr_update)(u16 num);
    int (*get_in_fifo_curr_des)(struct aic_hw_ppp_des **des, u16 num);
    int (*out_fifo_rptr_update)(u16 num);
    int (*get_out_fifo_curr_des)(struct aic_hw_ppp_des **des, u16 num);
    void (*clear_intr)(u32 irq);
    int (*interrupt_config)(u32 intr, u32 enable);
    int (*threshold_irq_config)(u32 enable, u16 threshold);
    int (*timeout_irq_config)(u32 enable, u16 threshold);
    int (*errcode_irq_config)(u32 enable);
    int (*flow_ctrl_irq_config)(u32 enable);
    int (*crc_error_irq_config)(u32 enable);
    u32 (*get_irq_status)(void);
    void (*get_fifo_ptr)(u16 *in_fifo_rptr, u16 *out_fifo_wptr);
    void (*get_in_fifo_addr)(u32 *fifo_addr);
};

void aic_ppp_encode_register(struct aic_ppp_codec_phy_ops *ppp_encode_ops);

void aic_ppp_decode_register(struct aic_ppp_codec_phy_ops *ppp_decode_ops);

#endif

