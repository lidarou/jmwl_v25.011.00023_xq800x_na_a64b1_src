/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 * DESCRIPTION
 *
 *   This file is the software driver of can module.
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0fd0bd
#define LOG_MODULE    AMOD_CAN

/* aic header */
#include <reg_base.h>
#include <cru_mcu.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <irq.h>
#include <aic_reg_base.h>
#include <ckm_en_glb_mcu.h>
#include <mcu_glb.h>
#include <aic_assert.h>
#include <errno.h>
#include <timer_inc.h>
#include <pmu_user.h>
#include <aic_io.h>
#include <gpio_inc.h>
#include "can_drv.h"

can_baud g_baud = CAN_BAUD_1M_1;
can_dev g_can_dev;

#define USE_INT 1

/* debug information */
#define DEV_PRINT         alog_info
#define DEV_PRINX         aicx_info

#define CAN_DEBUG
#ifdef CAN_DEBUG
#define PDEBUG            alog_info
#define PDEBUX            aicx_info
#else
#define PDEBUG(...)
#define PDEBUX(...)
#endif

#define ioread32(ADDR)              AIC_READ32(ADDR)
#define ioread8(ADDR)               AIC_READ8(ADDR)
#define iowrite32(VAL, ADDR)        AIC_WRITE32(ADDR, VAL)
#define iowrite8(VAL, ADDR)         AIC_WRITE8(ADDR, VAL)

#define CANFD_MASK                  0x01
#define CAN_MASK                    0x00
#define TX_MAX                      1

/* CAN registers set */
enum can_device_reg {
    CAN_RBUF_OFFSET           =   0x00,/*Receive Buffer Registers 0x00-0x4f(not finished)*/
    CAN_RBUF_ID_OFFSET        =   0x00,
    CAN_RBUF_CTL_OFFSET       =   0x04,
    CAN_RBUF_DATA_OFFSET      =   0x08,
    CAN_RBUF_RTS_OFFSET       =   0x10,

    CAN_TBUF_OFFSET           =   0x50,    /*Transmit Buffer Registers 0x50-0x97(not finished)*/
    CAN_TBUF_ID_OFFSET        =   0x50,
    CAN_TBUF_CTL_OFFSET       =   0x54,
    CAN_TBUF_DATA_OFFSET      =   0x58,

    CAN_TBUF_TTS_OFFSET       =   0x98,    /*Transmission Time Stamp 0x98-0x9f(not finished)*/

    CAN_CFG_STAT_OFFSET       =   0xa0,
    CAN_TCMD_OFFSET           =   0xa1,
    CAN_TCTRL_OFFSET          =   0xa2,
    CAN_RCTRL_OFFSET          =   0xa3,
    CAN_RTIE_OFFSET           =   0xa4,
    CAN_RTIF_OFFSET           =   0xa5,
    CAN_ERRINT_OFFSET         =   0xa6,
    CAN_LIMIT_OFFSET          =   0xa7,
    CAN_S_SEG_1_OFFSET        =   0xa8,
    CAN_S_SEG_2_OFFSET        =   0xa9,
    CAN_S_SJW_OFFSET          =   0xaa,
    CAN_S_PRESC_OFFSET        =   0xab,
    CAN_F_SEG_1_OFFSET        =   0xac,
    CAN_F_SEG_2_OFFSET        =   0xad,
    CAN_F_SJW_OFFSET          =   0xae,
    CAN_F_PRESC_OFFSET        =   0xaf,

    CAN_EALCAP_OFFSET         =   0xb0,
    CAN_RECNT_OFFSET          =   0xb2,
    CAN_TECNT_OFFSET          =   0xb3,
    CAN_ACFCTRL_OFFSET        =   0xb4,
    CAN_TIMECFG_OFFSET        =   0xb5,
    CAN_ACF_EN_0_OFFSET       =   0xb6,
    CAN_ACF_EN_1_OFFSET       =   0xb7,
    CAN_ACF_0_OFFSET          =   0xb8,
    CAN_ACF_1_OFFSET          =   0xb9,
    CAN_ACF_2_OFFSET          =   0xba,
    CAN_ACF_3_OFFSET          =   0xbb,
    CAN_VER_0_OFFSET          =   0xbc,
    CAN_VER_1_OFFSET          =   0xbd,
};

enum can_reg_bitchange{
    CAN_SET_RST_MASK         =   0x80,      /*Set Reset Bit*/
    CAN_OFF_RST_MASK         =   0x7f,    /*Reset Off Bit*/

    CAN_SET_FULLCAN_MASK     =   0x10,    /*set TTTBM as 1->full TTCAN mode*/
    CAN_OFF_FULLCAN_MASK     =   0xef,    /*set TTTBM as 0->separate PTB and STB mode*/

    CAN_SET_FIFO_MASK        =   0x20,    /*set TSMODE as 1->FIFO mode*/
    CAN_OFF_FIFO_MASK        =   0xdf,    /*set TSMODE as 0->Priority mode*/

    CAN_SET_TSONE_MASK       =   0x04,
    CAN_OFF_TSONE_MASK       =   0xfb,

    CAN_SET_TSALL_MASK       =   0x02,
    CAN_OFF_TSALL_MASK       =   0xfd,

    CAN_LBMEMOD_MASK         =   0x40,    /*set loop back mode, external*/
    CAN_LBMIMOD_MASK         =   0x20,

    CAN_SET_BUSOFF_MASK      =   0x01,
    CAN_OFF_BUSOFF_MASK      =   0xfe,

    CAN_SET_TTSEN_MASK       =   0x80,    /*set ttsen, tts update enable*/
    CAN_SET_BRS_MASK         =   0x10,    /*can fd Bit Rate Switch mask*/
    CAN_OFF_BRS_MASK         =   0xef,
    CAN_SET_EDL_MASK         =   0x20,    /*Extended Data Length*/
    CAN_OFF_EDL_MASK         =   0xdf,

    CAN_SET_DLC_MASK         =   0x0f,
    CAN_SET_TENEXT_MASK      =   0x40,
    CAN_SET_IDE_MASK         =   0x80,
    CAN_OFF_IDE_MASK         =   0x7f,
    CAN_SET_RTR_MASK         =   0x40,
    CAN_OFF_RTR_MASK         =   0xbf,

    CAN_INTR_ALL_MASK        =   0xff,     /*all interrupts enable mask*/
    CAN_SET_RIE_MASK         =   0x80,
    CAN_OFF_RIE_MASK         =   0x7f,
    CAN_SET_RFIE_MASK        =   0x20,
    CAN_OFF_RFIE_MASK        =   0xdf,
    CAN_SET_RAFIE_MASK       =   0x10,
    CAN_OFF_RAFIE_MASK       =   0xef,
    CAN_SET_EIE_MASK         =   0x02,
    CAN_OFF_EIE_MASK         =   0xfd,

    CAN_TASCTIVE_MASK        =   0x02,
    CAN_RASCTIVE_MASK        =   0x04,

    CAN_SET_TBSEL_MASK       =   0x80,    /*message writen in STB*/
    CAN_OFF_TBSEL_MASK       =   0x7f,    /*message writen in PTB*/

    CAN_SET_STBY_MASK        =   0x20,
    CAN_OFF_STBY_MASK        =   0xdf,

    CAN_SET_TPE_MASK         =   0x10,    /*Transmit primary enable*/

    CAN_SET_TPA_MASK         =   0x08,

    CAN_SET_SACK_MASK        =   0x80,
    CAN_SET_RREL_MASK        =   0x10,

    CAN_RSTAT_NOT_EMPTY_MASK =   0x03,

    CAN_SET_RIF_MASK         =   0x80,
    CAN_OFF_RIF_MASK         =   0x7f,
    CAN_SET_ROIF_MASK        =   0x40,
    CAN_SET_RFIF_MASK        =   0x20,
    CAN_SET_RAFIF_MASK       =   0x10,

    CAN_SET_TPIF_MASK        =   0x08,    /*Transmission Primary Interrupt Flag*/
    CAN_SET_TSIF_MASK        =   0x04,
    CAN_SET_EIF_MASK         =   0x02,
    CAN_SET_AIF_MASK         =   0x01,


    CAN_SET_EWARN_MASK       =   0x80,
    CAN_SET_EPASS_MASK       =   0x40,
    CAN_SET_EPIE_MASK        =   0x20,
    CAN_SET_EPIF_MASK        =   0x10,
    CAN_SET_ALIE_MASK        =   0x08,
    CAN_SET_ALIF_MASK        =   0x04,
    CAN_SET_BEIE_MASK        =   0x02,
    CAN_SET_BEIF_MASK        =   0x01,
    CAN_OFF_EPIE_MASK        =   0xdf,
    CAN_OFF_BEIE_MASK        =   0xfd,

    CAN_SET_AFWL_MASK        =   0x40,
    CAN_SET_EWL_MASK         =   0x0b,

    CAN_SET_KOER_MASK        =   0xe0,
    CAN_SET_BIT_ERROR_MASK   =   0x20,
    CAN_SET_FORM_ERROR_MASK  =   0x40,
    CAN_SET_STUFF_ERROR_MASK =   0x60,
    CAN_SET_ACK_ERROR_MASK   =   0x80,
    CAN_SET_CRC_ERROR_MASK   =   0xa0,
    CAN_SET_OTH_ERROR_MASK   =   0xc0,
};

#define BITS_OF_BITTIMING_REG       8        /*seg1,seg2,sjw,prescaler all have 8 bits*/
#define FBITS_IN_BITTIMING_STR      32        /*in can_bittiming strucure every field has 32 bits---->u32*/
#define SEG_1_SHIFT                 0
#define SEG_2_SHIFT                 8
#define SJW_SHIFT                   16
#define PRESC_SHIFT                 24

#define TTSEN_8_32_SHIFT            24        /*TTSEN bit used for 32 bit register read or write*/
#define RTR_32_8_SHIFT              24

/* special address description flags for the CAN_ID */
#define CAN_EFF_FLAG 0x80000000U /* EFF/SFF is set in the MSB */
#define CAN_RTR_FLAG 0x40000000U /* remote transmission request */
#define CAN_ERR_FLAG 0x20000000U /* error message frame */

/* valid bits in CAN ID for frame formats */
#define CAN_SFF_MASK 0x000007FFU /* standard frame format (SFF) */
#define CAN_EFF_MASK 0x1FFFFFFFU /* extended frame format (EFF) */
#define CAN_ERR_MASK 0x1FFFFFFFU /* omit EFF, RTR, ERR flags */

/* can event */
/* Abort Interrupt */
#define CAN_EVT_AIF       CAN_SET_AIF_MASK
/* Error Interrupt */
#define CAN_EVT_EIF       CAN_SET_EIF_MASK
/* Transmission Secondary Interrupt */
#define CAN_EVT_TSIF      CAN_SET_TSIF_MASK
/* Transmission Primary Interrupt */
#define CAN_EVT_TPIF      CAN_SET_TPIF_MASK
/* RB Almost Full Interrupt */
#define CAN_EVT_RAFIF     CAN_SET_RAFIF_MASK
/* RB Full Interrupt */
#define CAN_EVT_RFIF      CAN_SET_RFIF_MASK
/* RB Overrun Interrupt */
#define CAN_EVT_ROIF      CAN_SET_ROIF_MASK
/* Receive Interrupt */
#define CAN_EVT_RIF       CAN_SET_RIF_MASK


can_bittiming can_bittiming_cfg[8] = {
    /* 1M, [0x80] = [0]0x0008080F or [1]0x01040406 */
    {
        .s_seg_1 = 0xF,
        .s_seg_2 = 0x8,
        .s_sjw = 0x8,
        .s_presc = 0x0,
        .baud = 1000000,
    },
    {
        .s_seg_1 = 0x6,
        .s_seg_2 = 0x4,
        .s_sjw = 0x4,
        .s_presc = 0x1,
        .baud = 1000000,
    },
    /* 500K, [0x80] = [2]0x0108080F or [3]0x03030307 */
    {
        .s_seg_1 = 0xF,
        .s_seg_2 = 0x8,
        .s_sjw = 0x8,
        .s_presc = 0x1,
        .baud = 500000,
    },
    {
        .s_seg_1 = 0x7,
        .s_seg_2 = 0x3,
        .s_sjw = 0x3,
        .s_presc = 0x3,
        .baud = 500000,
    },
    /* 250K, [0x80] = [4]0x0308080F or [5]0x07030307 */
    {
        .s_seg_1 = 0xF,
        .s_seg_2 = 0x8,
        .s_sjw = 0x8,
        .s_presc = 0x3,
        .baud = 250000,
    },
    {
        .s_seg_1 = 0x7,
        .s_seg_2 = 0x3,
        .s_sjw = 0x3,
        .s_presc = 0x7,
        .baud = 250000,
    },
    /* 125K, [0x80] = [6]0x0708080F or [7]0x0F030307 */
    {
        .s_seg_1 = 0xF,
        .s_seg_2 = 0x8,
        .s_sjw = 0x8,
        .s_presc = 0x7,
        .baud = 125000,
    },
    {
        .s_seg_1 = 0x7,
        .s_seg_2 = 0x3,
        .s_sjw = 0x3,
        .s_presc = 0xF,
        .baud = 125000,
    }
};

#define CAL_BITTIMING_VALUE(x)    ( can_bittiming_cfg[x].s_seg_1 \
                                  | can_bittiming_cfg[x].s_seg_2 << 8 \
                                  | can_bittiming_cfg[x].s_sjw << 16 \
                                  | can_bittiming_cfg[x].s_presc << 24 )

#if 0
/* CAN Bittiming constants */
/*Bittiming setting from IPMS DS. */
/*CANFD*/
static struct can_bittiming_const_st can_bittiming_const = {
    .name = DRIVER_NAME,
    .tseg1_min = 2,
    .tseg1_max = 17,
    .tseg2_min = 1,
    .tseg2_max = 8,
    .sjw_max = 8,
    .brp_min = 1,
    .brp_max = 512,
    .brp_inc = 1,
};

/* CAN Data Bittiming constants */
/*CAN2.0*/
//TODO: t_seg2 for CAN FD nominal is differently defined
static struct can_bittiming_const_st can_data_bittiming_const = {
    .name = DRIVER_NAME,
    .tseg1_min = 2,
    .tseg1_max = 65,
    .tseg2_min = 1,
    .tseg2_max = 8,
    .sjw_max = 16,
    .brp_min = 1,
    .brp_max = 512,
    .brp_inc = 1,
};
#endif

static void can_write_reg(const struct can_dev_st *pdev, enum can_device_reg reg, u32 val)
{
    iowrite32(val, pdev->reg_base + reg);
}

static u32 can_read_reg(const struct can_dev_st *pdev, enum can_device_reg reg)
{
    return ioread32(pdev->reg_base + reg);
}

static void can_reg_set_bit(const struct can_dev_st *pdev, enum can_device_reg reg, u32 set_mask)
{
    u32 temp=0;
    temp = ioread32(pdev->reg_base + reg);
    iowrite32(temp|set_mask, pdev->reg_base + reg);
}

#if 0
static void can_reg_off_bit(const struct can_dev_st *pdev, enum can_device_reg reg, u32 set_mask)
{
    u32 temp=0;
    temp = ioread32(pdev->reg_base + reg);
    iowrite32(temp&set_mask, pdev->reg_base + reg);
}
#endif

static void can_register_set_bit(const struct can_dev_st *pdev, enum can_device_reg reg, enum can_reg_bitchange set_mask)
{
    u8 temp=0;
    temp = ioread8(pdev->reg_base + reg);
    iowrite8(temp|set_mask, pdev->reg_base + reg);
}

static void can_register_off_bit(const struct can_dev_st *pdev, enum can_device_reg reg, enum can_reg_bitchange set_mask)
{
    u8 temp=0;
    temp = ioread8(pdev->reg_base + reg);
    iowrite8(temp&set_mask, pdev->reg_base + reg);
}

static int set_reset_mode(struct can_dev_st *pdev, bool b_reset)
{
    u8 ret;
    ret = ioread32(pdev->reg_base + CAN_CFG_STAT_OFFSET);

    if (b_reset)
        ret |= (CAN_SET_RST_MASK);
    else
        ret &= ~CAN_SET_RST_MASK;

    iowrite32(ret, pdev->reg_base + CAN_CFG_STAT_OFFSET);

    return 0;
}

/*
 * can_cc_dlc2len(value) - convert a given data length code (dlc) of a
 * Classical CAN frame into a valid data length of max. 8 bytes.
 *
 * To be used in the CAN netdriver receive path to ensure conformance with
 * ISO 11898-1 Chapter 8.4.2.3 (DLC field)
 */
#define can_cc_dlc2len(dlc)    (MIN(dlc, CAN_MAX_DLEN))
#define can_cc_len2dlc(len)    ((len < CAN_MAX_DLEN) ? len : CAN_MAX_DLEN)
#if 0
/* helper to get the data length code (DLC) for Classical CAN raw DLC access */
static inline u8 can_get_cc_dlc(const struct can_frame *cf, const u32 ctrlmode)
{
    /* return len8_dlc as dlc value only if all conditions apply */
    if ((ctrlmode & CAN_CTRLMODE_CC_LEN8_DLC) &&
        (cf->len == CAN_MAX_DLEN) &&
        (cf->len8_dlc > CAN_MAX_DLEN && cf->len8_dlc <= CAN_MAX_RAW_DLC))
        return cf->len8_dlc;

    /* return the payload length as dlc value */
    return cf->len;
}
#endif

/* CAN DLC to real data length conversion helpers */
static const u8 dlc2len[] = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 12, 16, 20, 24, 32, 48, 64
};

/* get data length from raw data length code (DLC) */
u8 can_fd_dlc2len(u8 dlc)
{
    return dlc2len[dlc & 0x0F];
}

static const u8 len2dlc[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,      /* 0  - 8  */
    9, 9, 9, 9,                     /* 9  - 12 */
    10, 10, 10, 10,                 /* 13 - 16 */
    11, 11, 11, 11,                 /* 17 - 20 */
    12, 12, 12, 12,                 /* 21 - 24 */
    13, 13, 13, 13, 13, 13, 13, 13, /* 25 - 32 */
    14, 14, 14, 14, 14, 14, 14, 14, /* 33 - 40 */
    14, 14, 14, 14, 14, 14, 14, 14, /* 41 - 48 */
    15, 15, 15, 15, 15, 15, 15, 15, /* 49 - 56 */
    15, 15, 15, 15, 15, 15, 15, 15  /* 57 - 64 */
};

/* map the sanitized data length to an appropriate data length code */
u8 can_fd_len2dlc(u8 len)
{
    if (len > CANFD_MAX_DLEN)
        return CANFD_MAX_DLC;

    return len2dlc[len];
}

#if 0
static int can_device_driver_bittime_configuration(struct can_dev_st *pdev)
{
#if 1
    struct can_bittiming *bt = &(pdev->bittiming);
    struct can_bittiming *dbt = &(pdev->data_bittiming);
    u32 reset_test, bittiming_temp, data_bittiming_temp;

    reset_test = ioread8(pdev->reg_base + CAN_CFG_STAT_OFFSET);
    if(!(reset_test & CAN_SET_RST_MASK)){
        DEV_PRINT("Not in reset mode, cannot set bit timing\n");
        return -EPERM;
    }

    //bittiming_temp=((bt->phase_seg1-2)<<SEG_1_SHIFT)|((bt->phase_seg2-1)<<SEG_2_SHIFT)|((bt->sjw-1)<<SJW_SHIFT)|((bt->brp-1)<<PRESC_SHIFT);
    bittiming_temp=((bt->phase_seg1+bt->prop_seg+1-2)<<SEG_1_SHIFT)|((bt->phase_seg2-1)<<SEG_2_SHIFT)|((bt->sjw-1)<<SJW_SHIFT)|((bt->brp-1)<<PRESC_SHIFT);
    /*The input bittime setting is incorrect, should be correct*/
    if( (((int)(bt->phase_seg1+bt->prop_seg+1)-2)<0) || (((int)(bt->phase_seg2)-1)<0) || (((int)(bt->sjw)-1)<0) || (((int)(bt->brp)-1)<0) ){
        PDEBUG("slow bittime configuration is incorrect\n");
        return -EINVAL;
    }

    PDEBUG( "bittiming_temp value is %08x\n",bittiming_temp);
    can_write_reg(pdev, CAN_S_SEG_1_OFFSET, bittiming_temp);

    if(pdev->can_or_canfd==CANFD_MASK) {
        //data_bittiming_temp=((dbt->phase_seg1-2)<<SEG_1_SHIFT)|((dbt->phase_seg2-1)<<SEG_2_SHIFT)|((dbt->sjw-1)<<SJW_SHIFT)|((dbt->brp-1)<<PRESC_SHIFT);
        data_bittiming_temp=((dbt->phase_seg1+dbt->prop_seg+1-2)<<SEG_1_SHIFT)|((dbt->phase_seg2-1)<<SEG_2_SHIFT)|((dbt->sjw-1)<<SJW_SHIFT)|((dbt->brp-1)<<PRESC_SHIFT);
        PDEBUG("test phase_seg1 is %d\n",dbt->phase_seg1);

        //if( ((dbt->phase_seg1-2)<0) || (dbt->phase_seg2-1<0) || (dbt->sjw-1<0) || (dbt->brp-1<0)){
        if( (((int)(dbt->phase_seg1+dbt->prop_seg+1)-2)<0) || (((int)(dbt->phase_seg2)-1)<0) || (((int)(dbt->sjw)-1)<0) || (((int)(dbt->brp)-1)<0) ){
            PDEBUG("fast bittime configuration is incorrect\n");
            return -EINVAL;
        }
        can_write_reg(pdev, CAN_F_SEG_1_OFFSET, data_bittiming_temp);
    }

    /*reset off*/
    can_register_off_bit(pdev, CAN_CFG_STAT_OFFSET,CAN_OFF_RST_MASK);
    /*print configured slow and fast bit rate*/
    PDEBUG( "Slow bit rate configuration: %08x\n", can_read_reg(pdev, CAN_S_SEG_1_OFFSET));
    PDEBUG( "Fast bit rate configuration: %08x\n", can_read_reg(pdev, CAN_F_SEG_1_OFFSET));
#endif
    return 0;
}
#endif

/* set can transceiver to shutdown mode, depending on HW design */
void can_set_shdn(boolean b_shdn)
{
    if (b_shdn)
        aic_gpio_output(16, GPIO_OUTPUT_LVL_HIGH);
    else
        aic_gpio_output(16, GPIO_OUTPUT_LVL_LOW);
}

/* set can transceiver power, depending on HW design */
int can_set_power(boolean b_on)
{
    void *power_can = (void *)pmu_get("pmu_can", "DVDD3V");

    if (b_on) {
        /* turn on can power */
        pmu_voltage_set(power_can, 3000000, 10000);
        pmu_enable(power_can);
    } else {
        pmu_disable(power_can);
    }
    return 0;
}

/* set can transceiver to stby mode */
int set_can_stby_mode(struct can_dev_st *pdev, boolean b_on)
{
    if (b_on) {
        can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_STBY_MASK);
        //PDEBUG( "enter STBY mode\n");
    } else {
        can_register_off_bit(pdev, CAN_TCMD_OFFSET, CAN_OFF_STBY_MASK);
        //PDEBUG( "exit STBY mode\n");
    }

    return 0;
}

/* set can timing */
void can_set_bit_timing(can_dev *pdev, u32 baudrate)
{
    /*config bittiming*/
    set_reset_mode(pdev, AIC_TRUE);
    iowrite32(CAL_BITTIMING_VALUE(baudrate), pdev->reg_base + CAN_S_SEG_1_OFFSET);
    set_reset_mode(pdev, AIC_FALSE);
}

#if USE_INT
static void can_tx_interrupt(struct can_dev_st *pdev, u8 isr)
{
    /*wait till transmission of the PTB or STB finished*/
    while(isr & (CAN_SET_TPIF_MASK|CAN_SET_TSIF_MASK)) {
        if(isr & CAN_SET_TPIF_MASK) {
            can_register_set_bit(pdev,CAN_RTIF_OFFSET,CAN_SET_TPIF_MASK);
            aos_set_event(pdev->can_event, CAN_EVT_TPIF, AOS_OR);
        }
        if(isr & CAN_SET_TSIF_MASK) {
            can_register_set_bit(pdev,CAN_RTIF_OFFSET,CAN_SET_TSIF_MASK);
            aos_set_event(pdev->can_event, CAN_EVT_TSIF, AOS_OR);
        }
        isr = ioread8(pdev->reg_base+CAN_RTIF_OFFSET);
    }
    //netif_wake_queue(ndev);
}

static void canfd_rxfull_interrupt(struct can_dev_st *pdev, u8 isr)
{
    if(isr & CAN_SET_RAFIF_MASK) {
        can_register_set_bit(pdev, CAN_RTIF_OFFSET,CAN_SET_RAFIF_MASK);
        aos_set_event(pdev->can_event, CAN_EVT_RAFIF, AOS_OR);
    }
}

static ISR_RET_E can_interrupt(void *arg)
{
    struct can_dev_st *pdev = (struct can_dev_st *)arg;;
    u8 isr,eir;
    u8 isr_handled, eir_handled;

    /*read the value of interrupt status register and error interrupt register*/
    isr = ioread8(pdev->reg_base + CAN_RTIF_OFFSET);
    eir = ioread8(pdev->reg_base + CAN_ERRINT_OFFSET);

    isr_handled = 0;
    eir_handled = 0;

    /* Check for Tx interrupt and Processing it */
    if (isr & (CAN_SET_TPIF_MASK|CAN_SET_TSIF_MASK)){
        can_tx_interrupt(pdev, isr);
        isr_handled |= (CAN_SET_TPIF_MASK|CAN_SET_TSIF_MASK);
    }

    if (isr & (CAN_SET_RAFIF_MASK|CAN_SET_RFIF_MASK) ) {
        canfd_rxfull_interrupt(pdev,isr);
        isr_handled |= (CAN_SET_RAFIF_MASK|CAN_SET_RFIF_MASK);
    }

    /*    Check Rx interrupt and Processing the receive interrupt routine*/
    if (isr & CAN_SET_RIF_MASK){
        can_register_off_bit(pdev,CAN_RTIE_OFFSET, CAN_OFF_RIE_MASK);
        can_register_set_bit(pdev,CAN_RTIF_OFFSET, CAN_SET_RIF_MASK);
        //napi_schedule(&pdev->napi);
        isr_handled |= CAN_SET_RIF_MASK;
    }

    if ( (isr & CAN_SET_EIF_MASK) | (eir & (CAN_SET_EPIF_MASK | CAN_SET_BEIF_MASK))){
    //if (isr & CAN_SET_EIF_MASK){
        PDEBUG("Error Error Error\n");
        //can_register_off_bit(priv,CAN_ERRINT_OFFSET,CAN_OFF_EPIE_MASK&CAN_OFF_BEIE_MASK);
        //can_register_off_bit(priv,CAN_RTIE_OFFSET,CAN_OFF_EIE_MASK);

        /*reset EPIF and BEIF. Reset EIF. What if during error interrupt, new interrupt appears?*/
        can_register_set_bit(pdev,CAN_ERRINT_OFFSET,eir & (CAN_SET_EPIF_MASK|CAN_SET_BEIF_MASK));
        can_register_set_bit(pdev,CAN_RTIF_OFFSET,isr & CAN_SET_EIF_MASK);

#if 0
        canfd_error_interrupt(pdev,isr,eir);
        isr_handled |= CAN_SET_EIF_MASK;
        eir_handled |=(CAN_SET_EPIF_MASK | CAN_SET_BEIF_MASK);
#endif
    }
    if ((isr_handled == 0) && (eir_handled == 0)) {
        DEV_PRINT("unhandled interrupt!");
        DEV_PRINT("ISR/EIR: %02X/%02X\n", isr, eir);
        return NONE_WORKER;
    }

    //DEV_PRINT("ISR/ISR_HANDLED: %02X/%02X\n", isr, isr_handled);
    //DEV_PRINT("EIR/EIR_HANDLED: %02X/%02X\n", eir, eir_handled);

    //iowrite8(CAN_INTR_ALL_MASK, pdev->reg_base + CAN_RTIE_OFFSET);
    can_reg_set_bit(pdev, CAN_RTIE_OFFSET, CAN_INTR_ALL_MASK);
    return NONE_WORKER;
}
#endif

#ifdef CONFIG_DRIVER_CAN_TEST
void can_test_entry(void *input)
{
    can_dev * pdev= &g_can_dev;
    can_frame cf;
//    u32 t1=0, t2=0, i=0, j=0, rate=0;

    can_set_filter(pdev, 0);

//    t1 = aos_get_tickcount();

    while (AIC_TRUE) {
        /* standard id */
        cf.can_id = 0x13;
        cf.len = 8;
        cf.data[0] = 0x1D1C1B1A;
        cf.data[1] = 0x1B1A1F1E;
        can_send_high_priority(pdev, &cf);

        /* extend id */
        cf.can_id = 0x81FFFFFF;
        cf.len = 8;
        cf.data[0] = 0xEDECEBEA;
        cf.data[1] = 0xEBEAEFEE;
        can_send(pdev, &cf);

        can_recv(pdev, &cf);

#if 0
        i++;
        if (i%100000 == 0) {
            j++;
            t2 = aos_get_tickcount();
            rate = 800000*1000/(t2-t1);
            DEV_PRINT( "can rate: %u bytes/s, %u times, %u ms\n", rate, j, t2-t1);
            t1 = t2;
        }
#endif
        aos_sleep(500);
    }

}
#endif

int can_init(void)
{
    can_dev *pdev = &g_can_dev;

    int ret = AIC_SUCCESS;

    pdev->reg_base = REG_BASE_BLK_MCU_CAN;
    pdev->can_or_canfd = CAN_MASK;

    can_set_power(AIC_TRUE);

#if 1
    /*config pinmap to CAN_TX, CAN_RX, CAN_STBY*/
    iowrite32(0x0, 0x411600B4);
    iowrite32(0x0, 0x411600B8);
    iowrite32(0x0, 0x411600BC);
    /* config pinmap to GPIO16 to control CAN Transceiver */
    iowrite32(0x3, 0x41160060);
#endif

    /* enable can controller */
    iowrite32(BIT_CKM_EN_GLB_MCU_MCU_CAN_EB, REG_CKM_EN_GLB_MCU_MCU_CAN_EB);

    /*enable can timer*/
    //iowrite32(0xC<<1, REG_CRU_MCU_CLK_MCU_CAN_TMR_CFG);    /*[4:1]:divider, [0]:sel_clk,0:26M, 1:2M*/
    iowrite32(0x1, REG_CRU_MCU_CLK_MCU_CAN_TMR_CFG);    /*[4:1]:divider, [0]:sel_clk,0:26M, 1:2M*/
    iowrite32(0x1, REG_CKM_EN_GLB_MCU_MCU_CAN_TMR_EB);

    /* set 1M baudrate */
    can_set_bit_timing(pdev, (u32)g_baud);

    /*interrupts enable*/
    //iowrite8(CAN_INTR_ALL_MASK, pdev->reg_base + CAN_RTIE_OFFSET);
    can_reg_set_bit(pdev, CAN_RTIE_OFFSET, CAN_INTR_ALL_MASK);

    /* can cia603 time-stamping enable */
    can_reg_set_bit(pdev, CAN_ACFCTRL_OFFSET, 1<<8);

#if USE_INT
    /* register irq handler */
    ret = register_irq(IRQ_MCU_CAN, can_interrupt, (void *)pdev, NULL, NULL);
    if (ret != AIC_SUCCESS) {
        DEV_PRINT("irq allocation for CAN failed\n");
        return ret;
    }
    //PDEBUG( "register_irq finished\n");
#endif

    /* set can tranceiver to idle mode */
    aic_gpio_request(16);    /* GPIO16 t0 control can stby mode, high active */
    can_set_shdn(AIC_FALSE);

    pdev->can_event = aos_create_event("can");
    AIC_ASSERT(pdev->can_event);

#ifdef CONFIG_DRIVER_CAN_TEST
    aos_create_thread_simply("can_test", can_test_entry, AOS_NULL, 2048, 128);
#endif

    PDEBUG( "can_init success\n");
    return ret;
}

int can_set_filter(can_dev *pdev, can_filter_cfg *config)
{
    set_reset_mode(pdev, AIC_TRUE);
#if 0
    /*enable AE_0*/
    iowrite8(0x1, pdev->reg_base + CAN_ACF_EN_0_OFFSET);        /*0xb6*/
    /*select ACODE_0*/
    iowrite8(0x0, pdev->reg_base + CAN_ACFCTRL_OFFSET);         /*0xb4*/
    /*config ACODE_0 to 0x3*/
    iowrite32(0x3, pdev->reg_base + CAN_ACF_0_OFFSET);           /*0xb8*/
    /*select AMASK_0*/
    iowrite8(0x20, pdev->reg_base + CAN_ACFCTRL_OFFSET);        /*0xb4*/
    /*config AMASK_0 to 0x1FFFFFFC*/
    iowrite32(0x1FFFFFFC, pdev->reg_base + CAN_ACF_0_OFFSET);   /*0xb8*/
#endif
    /*enable AE_0(0xb6:bit0), and select ACODE_0(0xb4:bit5&bit[3:0])*/
    can_reg_set_bit(pdev, CAN_ACFCTRL_OFFSET, 1<<16);
    /*config ACODE_0 to 0x3*/
    can_write_reg(pdev, CAN_ACF_0_OFFSET, 0x3);
    /*enable AE_0(0xb6:bit0), and select AMASK_0(0xb4:bit5&bit[3:0])*/
    can_reg_set_bit(pdev, CAN_ACFCTRL_OFFSET, 1<<16 | 0x20);
    /*config AMASK_0 to 0x1FFFFFFC*/
    can_write_reg(pdev, CAN_ACF_0_OFFSET, 0x1FFFFFFC);
    set_reset_mode(pdev, AIC_FALSE);
    return 0;
}

/*
 * Return: 1 on success and 0 on failure.
 */
int can_recv(can_dev *pdev, can_frame *cf)
{
    u32 can_id;
    u8 dlc, control;
    u32 actual_flags, ret;

    PDEBUG("can_rx_polling...\n");

    ret = aos_get_event(pdev->can_event, CAN_EVT_RAFIF, AOS_OR_CLEAR, &actual_flags, AOS_WAIT_FOREVER);
    if (ret) {
        DEV_PRINT("can_recv get event error! ret: 0x%X", ret);
        return AOS_ERROR;
    }

    //PDEBUG("receive frame!!!\n");

    control = ioread8(pdev->reg_base + CAN_RBUF_CTL_OFFSET);
    can_id = can_read_reg(pdev, CAN_RBUF_ID_OFFSET);
    dlc = ioread8(pdev->reg_base + CAN_RBUF_CTL_OFFSET) & CAN_SET_DLC_MASK;

    /*read  CAN data length*/
    cf->len = can_cc_dlc2len(dlc);
    //PDEBUG("DLC in can_rx is %02x",dlc);
    /*change the CANFD id into socketcan id format*/
    if(control & CAN_SET_IDE_MASK){
        /*extended format*/
        cf->can_id = can_id;
        cf->can_id |=  CAN_EFF_FLAG ;
        PDEBUG("recv ext frame!");
    /*bit 29,error message not defined here*/
    //TODO:error message
    }
    else{
        cf->can_id = can_id;
        cf->can_id &= (~CAN_EFF_FLAG) ;
        PDEBUG("recv std frame!");
    }

    if(control & CAN_SET_RTR_MASK)
        cf->can_id |= CAN_RTR_FLAG ;

    /* Data*/
    if(!(control & CAN_SET_RTR_MASK)){
        cf->data[0] = can_read_reg(pdev, CAN_RBUF_DATA_OFFSET);
        cf->data[1] = can_read_reg(pdev, CAN_RBUF_DATA_OFFSET+4);
    }

    PDEBUG("Receive buffer:id 0x%08x\n", ioread32(pdev->reg_base + CAN_RBUF_ID_OFFSET));
    PDEBUG("Receive buffer:Control 0x%08x\n", ioread32(pdev->reg_base + CAN_RBUF_CTL_OFFSET));
    PDEBUG("Receive buffer:data0 0x%08x\n", ioread32(pdev->reg_base + CAN_RBUF_DATA_OFFSET));
    PDEBUG("Receive buffer:data1 0x%08x\n", ioread32(pdev->reg_base + CAN_RBUF_DATA_OFFSET + 4));
    PDEBUG("RTS: 0x%08x\n", ioread32(pdev->reg_base + CAN_RBUF_RTS_OFFSET));

    /*point to next packet and will save to buffer*/
    /*reset RREL to release RB slot, next RBUF will be updated. Then reset receive interrupt flag(we can not reset interrupt flag here)*/
    can_register_set_bit(pdev, CAN_RCTRL_OFFSET, CAN_SET_RREL_MASK);

    return 0;
}


/*Mode could be chosen, by reading corresponding registers.*/
/*This part could also be removed, when only one mode is used*/
static int set_can_xmit_mode(can_dev *pdev)
{
    switch(pdev->tx_mode) {
        case XMIT_FULL:
            can_register_set_bit(pdev, CAN_TCTRL_OFFSET, CAN_SET_FULLCAN_MASK);
            //PDEBUG("Full can mode\n");
            break;

        case XMIT_SEP_FIFO:
            can_register_off_bit(pdev, CAN_TCTRL_OFFSET, CAN_OFF_FULLCAN_MASK);
            can_register_set_bit(pdev, CAN_TCTRL_OFFSET, CAN_SET_FIFO_MASK);
            can_register_off_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TBSEL_MASK);
            //PDEBUG( "FIFO mode\n");
            break;

        case XMIT_SEP_PRIO:
            can_register_off_bit(pdev, CAN_TCTRL_OFFSET, CAN_OFF_FULLCAN_MASK);
            can_register_off_bit(pdev, CAN_TCTRL_OFFSET, CAN_OFF_FIFO_MASK);
            can_register_off_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TBSEL_MASK);
            //PDEBUG( "Priority mode\n");
            break;

        case XMIT_PTB_MODE:
            //iowrite8(0x0, CAN_TCMD_OFFSET);
            can_write_reg(pdev, CAN_CFG_STAT_OFFSET, 0x00900000);
            //PDEBUG("PTB Mode\n");
            break;

        case XMIT_STB_TSONE_MODE:
            //can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TBSEL_MASK);
            can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, CAN_SET_TBSEL_MASK<<8);
            //PDEBUG("STB TSONE mode\n");
            break;

        case XMIT_STB_TSALL_MODE:
            //can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TBSEL_MASK);
            can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, CAN_SET_TBSEL_MASK<<8);
            //PDEBUG("STB TSALL mode\n");
            break;

        default:
            break;
    }

    return 0;
}
#if 0
static int set_can_tsnext(can_dev *pdev)
{
    can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, CAN_SET_TBSEL_MASK<<8 | CAN_SET_TENEXT_MASK<<16);
    return 0;
}
#endif
static int start_can_xmit(struct can_dev_st *pdev)
{
    switch(pdev->tx_mode) {
        case XMIT_PTB_MODE:
            //can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TPE_MASK);
            can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, CAN_SET_TPE_MASK<<8);
            //PDEBUG("Start PTB Mode Transmit!\n");
            break;

        case XMIT_STB_TSONE_MODE:
            //can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TSONE_MASK);
            can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, (CAN_SET_TSONE_MASK|CAN_SET_TBSEL_MASK)<<8 | CAN_SET_TENEXT_MASK<<16);
            //PDEBUG("Start STB TSONE mode Transmit!\n");
            break;

        case XMIT_STB_TSALL_MODE:
            //can_register_set_bit(pdev, CAN_TCMD_OFFSET, CAN_SET_TSALL_MASK);
            can_reg_set_bit(pdev, CAN_CFG_STAT_OFFSET, (CAN_SET_TSALL_MASK|CAN_SET_TBSEL_MASK)<<8 | CAN_SET_TENEXT_MASK<<16);
            //PDEBUG("Start STB TSALL mode Transmit!\n");
            break;

        default:
            PDEBUG("Error! Invalid xmit mode!\n");
            break;
    }

    return 0;
}

static int can_transmit(can_dev *pdev, can_frame *cf, u32 tx_mode)
{
    u32 ttsen, id, ctl, ret, actual_flags;

    pdev->tx_mode = tx_mode;

    set_can_xmit_mode(pdev);

    ttsen = CAN_SET_TTSEN_MASK << TTSEN_8_32_SHIFT;
    ctl = can_cc_len2dlc(cf->len);

    if (cf->can_id & CAN_EFF_FLAG) {
        /*ext id*/
        id = (cf->can_id & CAN_EFF_MASK);
        ctl |= CAN_SET_IDE_MASK;
        DEV_PRINT("send ext frame!");
    } else {
        /*standard id*/
        id = (cf->can_id & CAN_SFF_MASK);
        DEV_PRINT("send std frame!");
    }

    /* enable TTS */
    id |= ttsen;

    if(cf->can_id & CAN_RTR_FLAG) {
        ctl |= CAN_SET_RTR_MASK;
        can_write_reg(pdev, CAN_TBUF_ID_OFFSET, id);
        can_write_reg(pdev, CAN_TBUF_CTL_OFFSET, ctl);
    } else {
        ctl &= CAN_OFF_RTR_MASK;
        can_write_reg(pdev, CAN_TBUF_ID_OFFSET, id);
        can_write_reg(pdev, CAN_TBUF_CTL_OFFSET,ctl);
        can_write_reg(pdev, CAN_TBUF_DATA_OFFSET, cf->data[0]);
        can_write_reg(pdev, CAN_TBUF_DATA_OFFSET+4, cf->data[1]);
        PDEBUG("CONFIG_STAT Reg: 0x%08x\n", ioread32(pdev->reg_base + CAN_CFG_STAT_OFFSET));
        PDEBUG("Send buffer:id 0x%08x\n", ioread32(pdev->reg_base + CAN_TBUF_ID_OFFSET));
        PDEBUG("Send buffer:Control 0x%08x\n", ioread32(pdev->reg_base + CAN_TBUF_CTL_OFFSET));
        PDEBUG("Send buffer:data: 0x%08x, 0x%08x\n", ioread32(pdev->reg_base + CAN_TBUF_DATA_OFFSET),
                                                     ioread32(pdev->reg_base + CAN_TBUF_DATA_OFFSET + 4));
    }

    start_can_xmit(pdev);

#if USE_INT
    ret = aos_get_event(pdev->can_event, CAN_EVT_TPIF | CAN_EVT_TSIF, AOS_OR_CLEAR, &actual_flags, 1000);
    if (ret) {
        DEV_PRINT("can_transmit get event error! ret: 0x%X", ret);
        return AOS_ERROR;
    }
#else
    do {
        actual_flags = ioread8(pdev->reg_base + CAN_RTIF_OFFSET);
     }while((actual_flags & (CAN_SET_TPIF_MASK | CAN_SET_TSIF_MASK | CAN_SET_EIF_MASK)) == 0);
    PDEBUG("CAN_RTIF: 0x%08x\n", actual_flags);
    can_register_set_bit(pdev,CAN_RTIF_OFFSET,(CAN_SET_TPIF_MASK | CAN_SET_TSIF_MASK | CAN_SET_EIF_MASK));
    can_reg_set_bit(pdev, CAN_RTIE_OFFSET, CAN_INTR_ALL_MASK);
#endif
    PDEBUG("TTS: 0x%08x\n", ioread32(pdev->reg_base + CAN_TBUF_TTS_OFFSET));

    return 0;
}

int can_send(can_dev *pdev, can_frame *cf)
{
    return can_transmit(pdev, cf, XMIT_STB_TSONE_MODE);
}

int can_send_high_priority(can_dev *pdev, can_frame *cf)
{
    return can_transmit(pdev, cf, XMIT_PTB_MODE);
}

AIC_NORMAL_DRV_L1_INIT_EXPORT(can_init);

