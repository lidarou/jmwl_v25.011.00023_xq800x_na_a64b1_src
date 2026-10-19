
#ifndef __IIS_CFG_H__
#define __IIS_CFG_H__

#include <ckm_en_glb_top.h>

#define IIS_PORT_NUM        (sizeof(iis_cfgs) / sizeof(*iis_cfgs))
#define IIS_MCLK            12288000

union iis_control {
    u32 value;
    struct {
        u32 i2s_en          : 1;
        u32 dir_cfg         : 1;
        u32 ms_cfg          : 1;
        u32 sfr_rst         : 1;
        u32 fifo_rst        : 1;
        u32 chn_width       : 3;
        u32 ws_mode         : 4;
        u32 mono_mode       : 1;
        u32 audio_mode      : 1;
        u32 sck_polar       : 1;
        u32 ws_polar        : 1;
        u32 data_ws_del     : 5;
        u32 data_align      : 1;
        u32 data_order      : 1;
        u32 host_data_align : 1;
        u32 i2s_stb         : 1;
        u32 intreq_mask     : 1;
        u32 i2s_mask        : 1;
        u32 fifo_empty_mask : 1;
        u32 fifo_aempty_mask: 1;
        u32 fifo_full_mask  : 1;
        u32 fifo_afull_mask : 1;
        u32 lr_pack         : 1;
    } control;
};

struct iis_cfg {
    u32 id;
    u32 hw_id_tx;   /* DMA hw request id */
    u32 hw_id_rx;
    u32 reg_base;
    u32 reg_en;
    u32 bit_en;
    u32 tx_offset;
    u32 rx_offset;
    u32 mclk;
    union iis_control ctl_info;
};

#ifdef WIN32
struct iis_cfg iis_cfgs[] = {
    0,              /* u32 id; */
    0,              /* u32 hw_id_tx */
    0,              /* u32 hw_id_rx */
    0x80000000,     /* u32 reg_base; */
    0x80000000,     /* u32 reg_en; */
    0x0,            /* u32 bit_en; */
    0,              /* u32 tx_offset; */
    0,              /* u32 rx_offset; */
    24576000,       /* u32 mclk; */
    /* union iis_control ctl_info; */
    {
        1,
    },
};
#else
struct iis_cfg iis_cfgs[] = {
    {
        .id = 0,
        .hw_id_tx = DMA_REQ_TOP_IIS0_TX,
        .hw_id_rx = DMA_REQ_TOP_IIS0_RX,
        .reg_base = REG_BASE_TOP_IIS0,
        .reg_en   = REG_CKM_EN_GLB_TOP_TOP_IIS_EB,
        .bit_en   = BIT_CKM_EN_GLB_TOP_TOP_IIS_EB,
        .tx_offset = REG_TX_ADDR,
        .rx_offset = REG_RX_ADDR,
        .mclk = IIS_MCLK,
        .ctl_info = {
            .control = {
                .ms_cfg          = 1,
                .sfr_rst         = 1,
                .fifo_rst        = 1,
                .audio_mode      = 0,
                .chn_width       = 2,
                .sck_polar       = 1,
                .ws_polar        = 0,
                .data_ws_del     = 1,
                .data_align      = 0,
                .data_order      = 0,
                .host_data_align = 0,
                .i2s_stb         = 0,
                .lr_pack         = 0,
            },
        },
    }
};
#endif


#endif /* __IIS_CFG_H__ */

