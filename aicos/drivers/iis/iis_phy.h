/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the IIS registers and each bit of them.
 */

#ifndef __IIS_H__
#define __IIS_H__

#include <aic_bitmap.h>
#include <reg_base.h>

#define IIS_HANDLE              unsigned int

#define CHECK_IIS_REG(r)    \
    if (r > REG_CTL_MAX) {  \
        alog_error("wrong iis reg = 0x%x\n", r);    \
        return 0;           \
    }

#define REG_IIS0_DMA_SEL         REG_TOP_GLB_DMA_SEL_IIS0
#define REG_IIS1_DMA_SEL         REG_TOP_GLB_DMA_SEL_IIS1

#define IIS_FIFO_DEPTH          32
#define IIS_AFULL_LEVEL         20
#define IIS_AEMPTY_LEVEL        12

#define IIS_BIT_WID_16          16
#define IIS_BIT_WID_32          32
#define IIS_DEFAULT_WID         IIS_BIT_WID_16
#define IIS_DEFAULT_SRATE       8000

#define REG_I2S_CTRL            0x00
#define REG_I2S_CTRL_FDX        0x04
#define REG_I2S_SRES            0x08
#define REG_I2S_SRES_FDR        0x0C
#define REG_I2S_SRATE           0x10
#define REG_I2S_STAT            0x14
#define REG_FIFO_LEVEL          0x18
#define REG_FIFO_AEMPTY         0x1C
#define REG_FIFO_AFULL          0x20
#define REG_FIFO_LEVEL_FDR      0x24
#define REG_FIFO_AEMPTY_FDR     0x28
#define REG_FIFO_AFULL_FDR      0x2C
#define REG_TDM_CTRL            0x30
#define REG_TDM_FD_DIR          0x34
#define REG_SIDE_CFG            0x38

#define REG_CTL_MAX             0x38

#define REG_TX_ADDR             0x40
#define REG_RX_ADDR             0x40

/* REG_I2S_CTRL: 0x00 */
#define I2S_EN              BIT(0)
#define DIR_CFG             BIT(1)
#define SFR_RST             BIT(3)
#define MS_CFG              BIT(2)
#define FIFO_RST            BIT(4)
#define CHN_WIDTH           BIT(5)
#define WS_MODE             BIT(8)
#define MONO_MODE           BIT(12)
#define AUDIO_MODE          BIT(13)
#define SCK_POLAR           BIT(14)
#define WS_POLAR            BIT(15)
#define DATA_WS_DEL         BIT(16)
#define DATA_ALIGN          BIT(21)
#define DATA_ORDER          BIT(22)
#define HOST_DATA_ALIGN     BIT(23)
#define I2S_STB             BIT(24)
#define INTREQ_MASK         BIT(25)
#define I2S_MASK            BIT(26)
#define FIFO_EMPTY_MASK     BIT(27)
#define FIFO_AEMPTY_MASK    BIT(28)
#define FIFO_FULL_MASK      BIT(29)
#define FIFO_AFULL_MASK     BIT(30)
#define LR_PACK             BIT(31)

/* REG_I2S_CTRL_FDX: 0x04 */
#define FULL_DUPLEX         BIT(0)
#define I2S_FTX_EN          BIT(1)
#define I2S_FRX_EN          BIT(2)
#define FIFO_RST            BIT(4)
#define RI2S_MASK           BIT(26)
#define RFIFO_EMPTY_MASK    BIT(27)
#define RFIFO_AEMPTY_MASK   BIT(28)
#define RFIFO_FULL_MASK     BIT(29)
#define RFIFO_AFULL_MASK    BIT(30)

/* REG_I2S_SRES: 0x08 */
#define RESOLUTION          BIT(0)

/* REG_I2S_SRES_FDR: 0x0C */
#define RRESOLUTION         BIT(0)
#define SDO_L_PHASE         BIT(21) /* TBD */
#define SDO_L_MULTIPLEXER   BIT(22) /* 22 - 24 */
#define SDO_R_PHASE         BIT(25) /* TBD */
#define SDO_R_MULTIPLEXER   BIT(26)
#define LOOP_R_SWITCH       BIT(29)
#define LOOP_L_SWITCH       BIT(30)
#define SDO_MULTIPLEXER_SWTITCH     BIT(31)

#define SDO_L_PHASE_S               21
#define SDO_L_MULTIPLEXER_S         22
#define SDO_R_PHASE_S               25
#define SDO_R_MULTIPLEXER_S         26
#define LOOP_L_SWITCH_S             29
#define LOOP_R_SWITCH_S             30
#define SDO_MULTIPLEXER_SWTITCH_S   31

/* REG_I2S_SRATE: 0x10 */
#define SAMPLE_RATE         BIT(0)

/* REG_I2S_STAT: 0x14 */
#define TDATA_UNDERR        BIT(0)
#define RDATA_OVRERR        BIT(1)
#define FIFO_EMPTY          BIT(2)
#define FIFO_AEMPTY         BIT(3)
#define FIFO_FULL           BIT(4)
#define FIFO_AFULL          BIT(5)
#define RFIFO_EMPTY         BIT(16)
#define RFIFO_AEMPTY        BIT(17)
#define RFIFO_FULL          BIT(18)
#define RFIFO_AFULL         BIT(19)

/* FIFO_LEVEL: 0x18 */
#define FIFO_LEVEL          BIT(0)

/* REG_FIFO_AEMPTY: 0x1C */
#define AEMPTY_THRESHOLD    BIT(0)

/* REG_FIFO_AFULL: 0x20 */
#define AFULL_THRESHOLD     BIT(0)

/* REG_FIFO_LEVEL_FDR: 0x24 */
#define FDR_LEVEL           BIT(0)

/* REG_FIFO_AEMPTY_FDR: 0x28 */
#define AEMPTY_THRESHOLD    BIT(0)

/* REG_FIFO_AFULL_FDR: 0x2C */
#define AFULL_THRESHOLD     BIT(0)

/* REG_TDM_CTRL: 0x30 */
#define TDM_EN              BIT(0)
#define CHN_NO              BIT(1)
#define CH0_EN              BIT(16)
#define CH1_EN              BIT(17)
#define CH2_EN              BIT(18)
#define CH3_EN              BIT(19)
#define CH4_EN              BIT(20)
#define CH5_EN              BIT(21)
#define CH6_EN              BIT(22)
#define CH7_EN              BIT(23)
#define CH8_EN              BIT(24)
#define CH9_EN              BIT(25)
#define CH10_EN             BIT(26)
#define CH11_EN             BIT(27)
#define CH12_EN             BIT(28)
#define CH13_EN             BIT(29)
#define CH14_EN             BIT(30)
#define CH15_EN             BIT(31)

/* REG_TDM_FD_DIR: 0x34 */
#define CH0_TXEN            BIT(0)
#define CH1_TXEN            BIT(1)
#define CH2_TXEN            BIT(2)
#define CH3_TXEN            BIT(3)
#define CH4_TXEN            BIT(4)
#define CH5_TXEN            BIT(5)
#define CH6_TXEN            BIT(6)
#define CH7_TXEN            BIT(7)
#define CH8_TXEN            BIT(8)
#define CH9_TXEN            BIT(9)
#define CH10_TXEN           BIT(10)
#define CH11_TXEN           BIT(11)
#define CH12_TXEN           BIT(12)
#define CH13_TXEN           BIT(13)
#define CH14_TXEN           BIT(14)
#define CH15_TXEN           BIT(15)

#define CH0_TXEN            BIT(0)
#define CH1_RXEN            BIT(1)
#define CH2_RXEN            BIT(2)
#define CH3_RXEN            BIT(3)
#define CH4_RXEN            BIT(4)
#define CH5_RXEN            BIT(5)
#define CH6_RXEN            BIT(6)
#define CH7_RXEN            BIT(7)
#define CH8_RXEN            BIT(8)
#define CH9_RXEN            BIT(9)
#define CH10_RXEN           BIT(10)
#define CH11_RXEN           BIT(11)
#define CH12_RXEN           BIT(12)
#define CH13_RXEN           BIT(13)
#define CH14_RXEN           BIT(14)
#define CH15_RXEN           BIT(15)

/* REG_SIDE_CFG: 0x38 */
#define DVC_COFF_L          BIT(0)
#define HPF_COFF_L          BIT(9)
#define HPF_SWITCH_L        BIT(13)
#define SIDE_SWITCH_L       BIT(14)     /* b'11: enable */
#define DVC_COFF_R          BIT(16)     /* TBD */
#define HPF_COFF_R          BIT(25)     /* TBD */
#define HPF_SWITCH_R        BIT(29)
#define SIDE_SWITCH_R       BIT(30)     /* TBD */

#define DVC_COFF_L_S        0
#define HPF_COFF_L_S        9
#define HPF_SWITCH_L_S      13
#define SIDE_SWITCH_L_S     14
#define DVC_COFF_R_S        16
#define HPF_COFF_R_S        25
#define HPF_SWITCH_R_S      29
#define SIDE_SWITCH_R_S     30

#if 0
void iis_enalbe(BOOL en);
void iis_reset(void);
void iis_set_control(u32 control);
void iis_set_bitwidth(u32 wid);
void iis_set_bitwidth_fdr(u32 wid);
void iis_set_srate(u32 sample_rate);
void iis_reset_fdx(void);
void iis_set_control_fdx(u32 control);
u32 iis_read_state(void);
u32 iis_read_level(void);
void iis_set_aempty(u32 count);
void iis_set_afull(u32 count);
u32 iis_read_level_fdr(void);
void iis_set_aempty_fdr(u32 count);
void iis_set_afull_fdr(u32 count);

u32 iis_send(short *buff, u32 count);
u32 iis_recv(short *buff, u32 count);
u32 iis_recv_send(u32 count);
u32 iis_send_int(short *buff, u32 count);
u32 iis_recv_int(short *buff, u32 count);
int iis_recv_send_int(u32 count);

void iis_set_tx_int(BOOL en);
void iis_set_rx_int(BOOL en);
void iis_set_rx_int_fdx(BOOL en);
void iis_fill_zero(void);
void iis_clr_state(void);

void iis_enable_loop(BOOL en);
void iis_enable_sdo_mplxer(BOOL en, int val_l, int val_r);
void iis_enable_sdo_phase(BOOL en_l, BOOL en_r);
void iis_set_side_dvc(u32 dvc_l, u32 dvc_r);
void iis_set_side_hpf(u32 coff_l, u32 coff_r);
void iis_enable_side_hpf(BOOL en);
void iis_enable_sidetone(BOOL en);
void iis_send_recv(short *src_buff, u32 src_cnt, short *rec_buff, u32 rec_cnt);
#endif
#endif /* __IIS_H__ */
