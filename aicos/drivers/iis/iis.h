
#include <aic_type.h>

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

