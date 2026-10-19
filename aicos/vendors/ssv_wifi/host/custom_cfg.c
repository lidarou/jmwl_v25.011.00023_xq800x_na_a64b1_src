/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/


/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x098a2c
#define LOG_MODULE    AMOD_VENDOR

#include <porting.h>
#include <ssv_common.h>
#include <host_config.h>
#include <log.h>
#include <ssv_hal.h>
#include <ssv_devinfo.h>


#define MAX_OFDM_RATE_GAIN_INDEX                 (9)
#define OFDM_RATE_GAIN_64QAM_OFFSET              (0)
#define OFDM_RATE_GAIN_16QAM_OFFSET              (2)
#define OFDM_RATE_GAIN_QPSK_OFFSET               (4)
#define OFDM_RATE_GAIN_BPSK_OFFSET               (6)

#define EFUSE_RATE_GAIN_MASK_CCK                 (0x01)
#define EFUSE_RATE_GAIN_MASK_LEGACY              (0x02)
#define EFUSE_RATE_GAIN_MASK_HT20                (0x04)
#define EFUSE_RATE_GAIN_MASK_HT40                (0x08)

#define EFUSE_RATE_TBL_MASK_B_RATE               (0xf0)
#define EFUSE_RATE_TBL_SFT_B_RATE                (4)
#define EFUSE_RATE_TBL_MASK_LEGACY_RATE          (0xf0)
#define EFUSE_RATE_TBL_SFT_LEGACY_RATE           (4)
#define EFUSE_RATE_TBL_MASK_HT20_RATE            (0x0f)
#define EFUSE_RATE_TBL_SFT_HT20_RATE             (0)
#define EFUSE_RATE_TBL_MASK_HT40_RATE            (0x0f)
#define EFUSE_RATE_TBL_SFT_HT40_RATE             (0)


//default table
struct st_rf_table def_rf_table ={
                    /* rt_config */
                    { { 9,  9,  9,  9,  9,  9,  9} , 0x98, 0x98, 7, 9, 10, 7, 7, 7, 7, 7, 1},
                    /* ht_config */
                    { {11, 11, 11, 11, 11, 11, 11} , 0x98, 0x98, 7, 9, 10, 7, 7, 7, 7, 7, 1},
                    /* lt_config */
                    { { 8,  8,  8,  8,  8,  8,  8} , 0xa5, 0xa5, 7, 9, 10, 7, 7, 7, 7, 7, 1},
                    /* rf_gain */
                    4,
                    /* rate_gain_b */
                    4,
                    /* rate_config_g */
                    {15, 13, 11, 9},
                    /* rate_config_20n */
                    {15, 13, 11, 9},
                    /* rate_config_40n */
                    {15, 13, 11, 9},
                    /* low_boundarty */
                    10,
                    /* high_boundary */
                    75,
                    /* boot flag */
                    EN_FIRST_BOOT,
                    /* work mode */
                    EN_WORK_NOMAL,
                    /* rt_5g_config */
                    { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
                    /* ht_5g_config */
                    { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
                    /* lt_5g_config */
                    { 13, 10, 10, 9, 0x9264924a, 0x96dbb6cc },
                    /* band_f0_threshold */
                    0x141E,
                    /* band_f1_threshold */
                    0x157C,
                    /* band_f2_threshold */
                    0x1644,
                    /* Signature */
                    RF_API_SIGNATURE,
                    /* Structure version */
                    RF_API_TABLE_VERSION,
                    /* DCDC flag */
                    1,
                    /* external pa settings */
                    { 0, 0, 1, 0xd, 8, 8, 0xc, 0},
                    /* RTC 32K XTAL setting */
                    0,
                    /* Tx Lowpower setting */
                    1,
                    /* ble_rt_config */
                    {{8, 8}},
                    /* ble_ht_config */
                    {{8, 8}},
                    /* ble_lt_config */
                    {{8, 8}},
                    /*ble DTM mode*/
                     0,
                    /*external padpd flag*/
                     0,
                    /*external padpd setting*/
                    {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 
                     0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
                     0x200, 0x200, 0x200, 0x200, 0x200, 0x200,
                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                     0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 
                     0x0, 0x0},
                    /* ht2_config */
                    { {13, 13, 13, 13, 13, 13, 13} , 0xb4, 0xb4, 7, 9, 10, 7, 7, 7, 7, 7, 1},
                    /* lt2_config */
                    { { 7,  7,  7,  7,  7,  7,  7} , 0xa5, 0xa5, 7, 9, 10, 7, 7, 7, 7, 7, 1},
                    /*ble_ht2_config*/
                    {{8, 8}},
                    /*ble_lt2_config*/
                    {{8, 8}},
                    /* low2_boundarty */
                    -5,
                    /* high2_boundary */
                    90,
};

extern struct Host_cfg g_host_cfg;
#if 0
//------------mapping from value to index----------------------
static int _ssv_get_rf_table_rate_gain_idx_by_value(u32 value)
{
    u8 rate_gain_value[] = {0x4C, 0x51, 0x56, 0x5B, 0x60, 0x66, 0x6C, 0x72, 0x79, 0x80, 0x88, 0x90, 0x98, 0xA1, 0xAB, 0xB5};
    u8 rate_gain_num = (u8)(sizeof(rate_gain_value) / sizeof((rate_gain_value)[0]));
    u8 rate_gain_idx = 0;

    // rate_gain_index start from 1
    // rate gain_index 0 may be correct default value
    for (rate_gain_idx = 1; rate_gain_idx < rate_gain_num; rate_gain_idx++)
    {
        if (rate_gain_value[rate_gain_idx] == value)
            break;
    }

    if (rate_gain_idx == rate_gain_num) {
        LOG_ERROR("Cannot find rate gain[%u] index, reset to default value\n", value);
        rate_gain_idx = 0;
    }

    return rate_gain_idx;
}

static int _ssv_get_rf_table_bain_gain_idx_by_value(u32 value)
{
    u8 band_gain_value[] = {0x4C, 0x51, 0x56, 0x5B, 0x60, 0x66, 0x6C, 0x72, 0x79, 0x80, 0x88, 0x90, 0x98, 0xA1, 0xAB, 0xB5};
    u8 band_gain_num = (u8)(sizeof(band_gain_value)/sizeof((band_gain_value)[0]));
    u8 band_gain_idx = 0;

    // band_gain_index start from 1
    // band gain_index 0 may be correct default value
    for (band_gain_idx = 1; band_gain_idx < band_gain_num; band_gain_idx++)
    {
        if (band_gain_value[band_gain_idx] == value)
            break;
    }

    if (band_gain_idx == band_gain_num) {
        LOG_ERROR("Cannot find band gain[%u] index, reset to default value\n", value);
        band_gain_idx = 0;
    }

    return band_gain_idx;
}
#endif
#define MAX_2G_BAND_GAIN    7
#define MAX_GAIN_INDEX  16

int customer_update_rf_table(void)
{
    ssv_type_u8 value = 0x0;
    int i = 0;
    
    if (ssv_hal_efuse_read_xtal(&value) == 0)
    {
        LOG_PRINTF("[efuse] xtal 0x%x\r\n", value);
        def_rf_table.rt_config.freq_xi = value;
        def_rf_table.rt_config.freq_xo = value;
        def_rf_table.lt_config.freq_xi = value;
        def_rf_table.lt_config.freq_xo = value;
        def_rf_table.ht_config.freq_xi = value;
        def_rf_table.ht_config.freq_xo = value;
        def_rf_table.lt_2_config.freq_xi = value;
        def_rf_table.lt_2_config.freq_xo = value;
        def_rf_table.ht_2_config.freq_xi = value;
        def_rf_table.ht_2_config.freq_xo = value;
    }

    if (ssv_hal_efuse_read_tx_power1(&value) == 0)
    {
        ssv_type_u8 value_2g_band = 0x0;
        //ssv_type_u8 value_5g_band = 0x0;
        
        LOG_PRINTF("[efuse] tx_power1 0x%x\r\n", value);
        
        value_2g_band = ((value & 0x0f) >> 0);

        if(g_host_cfg.rf_band_gain_offset != 0)        
        {
            if (((value_2g_band + g_host_cfg.rf_band_gain_offset ) > 0) && ((value_2g_band+g_host_cfg.rf_band_gain_offset ) <= 15 ))
            {
                value_2g_band += g_host_cfg.rf_band_gain_offset;
                LOG_PRINTF("band gain offset %d, value %d\r\n", g_host_cfg.rf_band_gain_offset , value_2g_band);
            }
        }
        
        for (i = 0; i < MAX_2G_BAND_GAIN; i++) 
        {
            def_rf_table.rt_config.band_gain[i] = value_2g_band;
            def_rf_table.lt_config.band_gain[i] = value_2g_band;
            def_rf_table.ht_config.band_gain[i] = value_2g_band;
            def_rf_table.lt_2_config.band_gain[i] = value_2g_band;
            def_rf_table.ht_2_config.band_gain[i] = value_2g_band;
        }
    }
    
#if 0
    if (ssv_hal_efuse_read_tx_power2(&value) == 0)
    {
        ssv_type_u8 value_5g_band = 0x0;
        LOG_PRINTF("[efuse] tx_power2 0x%x\r\n", value);
        value_5g_band = ((value & 0x0f) >> 0);
        if (value_5g_band < MAX_GAIN_INDEX)
        {
            def_rf_table.rt_5g_config.bbscale_band2 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
            def_rf_table.lt_5g_config.bbscale_band2 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
            def_rf_table.ht_5g_config.bbscale_band2 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
        }
        else
        {
            LOG_PRINTF("[efuse] txpower2 0x%x (band2)! invalid r\n", value_5g_band);
        }

        value_5g_band = ((value & 0xf0) >> 4);
        if (value_5g_band < MAX_GAIN_INDEX)
        {
            def_rf_table.rt_5g_config.bbscale_band3 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
            def_rf_table.lt_5g_config.bbscale_band3 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
            def_rf_table.ht_5g_config.bbscale_band3 = _ssv_get_rf_table_bain_gain_idx_by_value(band_gain_tbl[value_5g_band]);
        }
        else
        {
            LOG_PRINTF("[efuse] txpower2 0x%x ! (band3) invalid\r\n", value_5g_band);
        }
    }
#endif

    // set b rate gain and ht40 rate gain
    if (ssv_hal_efuse_read_rate_gain_b_n40(&value) == 0)
    {
        ssv_type_u8 value_rate_gain_b = 0x0;
        ssv_type_u8 value_rate_config_40n = 0x0;   
        
        LOG_PRINTF("[efuse] rate_gain_b_n40 0x%x\r\n", value);


        if((value) && (g_host_cfg.efuse_rate_gain_mask & EFUSE_RATE_GAIN_MASK_CCK) )
        {
            value_rate_gain_b = ((value & EFUSE_RATE_TBL_MASK_B_RATE) >> EFUSE_RATE_TBL_SFT_B_RATE);
            
            if (g_host_cfg.rf_b_rate_offset != 0)
            {
                if (((value_rate_gain_b + g_host_cfg.rf_b_rate_offset) > 0) && ((value_rate_gain_b + g_host_cfg.rf_b_rate_offset) <= 15 ))
                {
                    value_rate_gain_b += g_host_cfg.rf_b_rate_offset;
                    LOG_PRINTF("rate gain b gain offset %d, value %d\r\n", g_host_cfg.rf_b_rate_offset, value_rate_gain_b);
                }
            }

            def_rf_table.rate_gain_b  = value_rate_gain_b;
            LOG_PRINTF("rate gain b use value %d\r\n", value_rate_gain_b);
        }else{
            if (g_host_cfg.rf_b_rate_offset != 0)
            {
                value_rate_gain_b = def_rf_table.rate_gain_b;
                if (((value_rate_gain_b + g_host_cfg.rf_b_rate_offset) > 0) && ((value_rate_gain_b + g_host_cfg.rf_b_rate_offset) <= 15 ))
                {
                    value_rate_gain_b += g_host_cfg.rf_b_rate_offset;
                    LOG_PRINTF("rate gain b gain offset %d, value %d\r\n", g_host_cfg.rf_b_rate_offset, value_rate_gain_b);
                    def_rf_table.rate_gain_b  = value_rate_gain_b;
                }
            }

            LOG_PRINTF("rate gain b use default value\r\n");
        }


        if((value) && (g_host_cfg.efuse_rate_gain_mask & EFUSE_RATE_GAIN_MASK_HT40))
        {
            value_rate_config_40n = ((value & EFUSE_RATE_TBL_MASK_HT40_RATE) >> EFUSE_RATE_TBL_SFT_HT40_RATE);
            
            if (g_host_cfg.rf_ht40_rate_offset != 0)
            {
                if (((value_rate_config_40n + g_host_cfg.rf_ht40_rate_offset) > 0) && ((value_rate_config_40n + g_host_cfg.rf_ht40_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_config_40n += g_host_cfg.rf_ht40_rate_offset;
                    LOG_PRINTF("rate gain ht40 gain offset %d, value %d\r\n", g_host_cfg.rf_ht40_rate_offset, value_rate_config_40n);
                }
            }

            if ((value_rate_config_40n > 0) && (value_rate_config_40n <= MAX_OFDM_RATE_GAIN_INDEX)) 
            {
                def_rf_table.rate_config_40n.rate1 = value_rate_config_40n + OFDM_RATE_GAIN_BPSK_OFFSET;
                def_rf_table.rate_config_40n.rate2 = value_rate_config_40n + OFDM_RATE_GAIN_QPSK_OFFSET;
                def_rf_table.rate_config_40n.rate3 = value_rate_config_40n + OFDM_RATE_GAIN_16QAM_OFFSET;
                def_rf_table.rate_config_40n.rate4 = value_rate_config_40n + OFDM_RATE_GAIN_64QAM_OFFSET;

                LOG_PRINTF("rate gain ht40 use value %d\r\n", value_rate_config_40n);
            } else
                LOG_PRINTF("incorrect rate gain ht40 index value %d\r\n", value_rate_config_40n);
        }
        else
        {
            if (g_host_cfg.rf_ht40_rate_offset != 0)
            {
                value_rate_config_40n = def_rf_table.rate_config_40n.rate4;
                if (((value_rate_config_40n + g_host_cfg.rf_ht40_rate_offset) > 0) && ((value_rate_config_40n + g_host_cfg.rf_ht40_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_config_40n += g_host_cfg.rf_ht40_rate_offset;
                    LOG_PRINTF("rate gain ht40 gain offset %d, value %d\r\n", g_host_cfg.rf_ht40_rate_offset, value_rate_config_40n);

                    def_rf_table.rate_config_40n.rate1 = value_rate_config_40n + OFDM_RATE_GAIN_BPSK_OFFSET;
                    def_rf_table.rate_config_40n.rate2 = value_rate_config_40n + OFDM_RATE_GAIN_QPSK_OFFSET;
                    def_rf_table.rate_config_40n.rate3 = value_rate_config_40n + OFDM_RATE_GAIN_16QAM_OFFSET;
                    def_rf_table.rate_config_40n.rate4 = value_rate_config_40n + OFDM_RATE_GAIN_64QAM_OFFSET;

                }
            }

            LOG_PRINTF("rate gain ht40 use default value\r\n");
        }

    }

    // set legacy rate gain
    if (ssv_hal_efuse_read_rate_gain_g_n20(&value) == 0)
    {
        ssv_type_u8 value_rate_gain_g = 0x0;
        ssv_type_u8 value_rate_config_20n = 0x0;
        
        LOG_PRINTF("[efuse] rate_gain_g_n20 0x%x\r\n", value);


        if((value) && (g_host_cfg.efuse_rate_gain_mask & EFUSE_RATE_GAIN_MASK_LEGACY))
        {
            value_rate_gain_g = ((value & EFUSE_RATE_TBL_MASK_LEGACY_RATE) >> EFUSE_RATE_TBL_SFT_LEGACY_RATE);
            
            if (g_host_cfg.rf_g_rate_offset != 0)
            {
                if (((value_rate_gain_g + g_host_cfg.rf_g_rate_offset) > 0) && ((value_rate_gain_g + g_host_cfg.rf_g_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_gain_g += g_host_cfg.rf_g_rate_offset;
                    LOG_PRINTF("rate gain g gain offset %d, value %d\r\n", g_host_cfg.rf_g_rate_offset, value_rate_gain_g);
                }
            }

            if ((value_rate_gain_g > 0)&&(value_rate_gain_g <= MAX_OFDM_RATE_GAIN_INDEX)) {
                def_rf_table.rate_config_g.rate1 = value_rate_gain_g + OFDM_RATE_GAIN_BPSK_OFFSET;
                def_rf_table.rate_config_g.rate2 = value_rate_gain_g + OFDM_RATE_GAIN_QPSK_OFFSET;
                def_rf_table.rate_config_g.rate3 = value_rate_gain_g + OFDM_RATE_GAIN_16QAM_OFFSET;
                def_rf_table.rate_config_g.rate4 = value_rate_gain_g + OFDM_RATE_GAIN_64QAM_OFFSET;
                LOG_PRINTF("rate gain legacy use value %d\r\n", value_rate_gain_g);
            } else
                LOG_PRINTF("incorrect rate gain legacy index value %d\r\n", value_rate_gain_g);

        }
        else
        {
            if (g_host_cfg.rf_g_rate_offset != 0)
            {
                value_rate_gain_g = def_rf_table.rate_config_g.rate4;
                if (((value_rate_gain_g + g_host_cfg.rf_g_rate_offset) > 0) && ((value_rate_gain_g + g_host_cfg.rf_g_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_gain_g += g_host_cfg.rf_g_rate_offset;
                    LOG_PRINTF("rate gain g gain offset %d, value %d\r\n", g_host_cfg.rf_g_rate_offset, value);
                    def_rf_table.rate_config_g.rate1 = value_rate_gain_g + OFDM_RATE_GAIN_BPSK_OFFSET;
                    def_rf_table.rate_config_g.rate2 = value_rate_gain_g + OFDM_RATE_GAIN_QPSK_OFFSET;
                    def_rf_table.rate_config_g.rate3 = value_rate_gain_g + OFDM_RATE_GAIN_16QAM_OFFSET;
                    def_rf_table.rate_config_g.rate4 = value_rate_gain_g + OFDM_RATE_GAIN_64QAM_OFFSET;
                }
            }

            LOG_PRINTF("rate gain legacyg use default value\r\n");
        }
        
        if((value) &&(g_host_cfg.efuse_rate_gain_mask & EFUSE_RATE_GAIN_MASK_HT20))
        {
            value_rate_config_20n = ((value & EFUSE_RATE_TBL_MASK_HT20_RATE) >> EFUSE_RATE_TBL_SFT_HT20_RATE);
            
            if (g_host_cfg.rf_ht20_rate_offset != 0)
            {
                if (((value_rate_config_20n + g_host_cfg.rf_ht20_rate_offset) > 0) && ((value_rate_config_20n + g_host_cfg.rf_ht20_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_config_20n += g_host_cfg.rf_ht20_rate_offset;
                    LOG_PRINTF("rate gain ht20 gain offset %d, value %d\r\n", g_host_cfg.rf_ht20_rate_offset, value_rate_config_20n);
                }
            }
            
            if ((value_rate_config_20n > 0)&&(value_rate_config_20n <= MAX_OFDM_RATE_GAIN_INDEX)) 
            {
                def_rf_table.rate_config_20n.rate1 = value_rate_config_20n + OFDM_RATE_GAIN_BPSK_OFFSET;
                def_rf_table.rate_config_20n.rate2 = value_rate_config_20n + OFDM_RATE_GAIN_QPSK_OFFSET;
                def_rf_table.rate_config_20n.rate3 = value_rate_config_20n + OFDM_RATE_GAIN_16QAM_OFFSET;
                def_rf_table.rate_config_20n.rate4 = value_rate_config_20n + OFDM_RATE_GAIN_64QAM_OFFSET;

                LOG_PRINTF("rate gain ht20 use value %d\r\n", value_rate_config_20n);
            } else
                LOG_PRINTF("incorrect rate gain ht20 index value %d\r\n", value_rate_config_20n);
        }
        else
        {
            if (g_host_cfg.rf_ht20_rate_offset != 0)
            {
                value_rate_config_20n = def_rf_table.rate_config_20n.rate4;
                if (((value_rate_config_20n + g_host_cfg.rf_ht20_rate_offset) > 0) && ((value_rate_config_20n + g_host_cfg.rf_ht20_rate_offset) <= MAX_OFDM_RATE_GAIN_INDEX ))
                {
                    value_rate_config_20n += g_host_cfg.rf_ht20_rate_offset;
                    LOG_PRINTF("rate gain ht20 gain offset %d, value %d\r\n", g_host_cfg.rf_ht20_rate_offset, value_rate_config_20n);

                    def_rf_table.rate_config_20n.rate1 = value_rate_config_20n + OFDM_RATE_GAIN_BPSK_OFFSET;
                    def_rf_table.rate_config_20n.rate2 = value_rate_config_20n + OFDM_RATE_GAIN_QPSK_OFFSET;
                    def_rf_table.rate_config_20n.rate3 = value_rate_config_20n + OFDM_RATE_GAIN_16QAM_OFFSET;
                    def_rf_table.rate_config_20n.rate4 = value_rate_config_20n + OFDM_RATE_GAIN_64QAM_OFFSET;

                }
            }

            LOG_PRINTF("rate gain ht20 use default value\r\n");
        }


    }
    //set ble gain
    if (ssv_hal_efuse_read_ble_gain(&value) == 0)
    {
        ssv_type_u8 i = 0, ble_gain = 0;
        ble_gain = ((value & 0x0f) >> 0);
        LOG_PRINTF("ble gain use %d\r\n", ble_gain);
        
        for (i = 0; i < BLE_CHANNEL_PART_COUNT; i++) {
            def_rf_table.ble_rt_config.band_gain[i] = ble_gain;
            def_rf_table.ble_lt_config.band_gain[i] = ble_gain;
            def_rf_table.ble_lt_2_config.band_gain[i] = ble_gain;
            def_rf_table.ble_ht_config.band_gain[i] = ble_gain;
            def_rf_table.ble_ht_2_config.band_gain[i] = ble_gain;
        }
    }

    //set volt regulator
    if(g_host_cfg.rf_volt_regulator == SSV6XXX_VOLT_LDO_CONVERT)
            def_rf_table.dcdc_flag = 0;
    else
            def_rf_table.dcdc_flag = 1;

    //set tw_lowpower
    def_rf_table.tx_lowpower_flag = g_host_cfg.rf_tx_lowpower;

    //set padpd
    if(0 != g_host_cfg.rf_padpd)
    {
        def_rf_table.lt_config.padpd_cali = 1;
        def_rf_table.lt_2_config.padpd_cali = 1;
        def_rf_table.rt_config.padpd_cali = 1;
        def_rf_table.ht_config.padpd_cali = 1;
        def_rf_table.ht_2_config.padpd_cali = 1;
    }
    else
    {
        def_rf_table.lt_config.padpd_cali = 0;
        def_rf_table.lt_2_config.padpd_cali = 0;
        def_rf_table.rt_config.padpd_cali = 0;
        def_rf_table.ht_config.padpd_cali = 0;
        def_rf_table.ht_2_config.padpd_cali = 0;
    }
    return 0;
}

struct st_rf_table *p_rf_table=(struct st_rf_table *)&def_rf_table;
int dump_rf_table(void)
{
    LOG_PRINTF("===================\r\n");
    LOG_PRINTF("RT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->rt_config.band_gain[0], p_rf_table->rt_config.band_gain[1], p_rf_table->rt_config.band_gain[2], p_rf_table->rt_config.band_gain[3], p_rf_table->rt_config.band_gain[4], p_rf_table->rt_config.band_gain[5], p_rf_table->rt_config.band_gain[6],
                p_rf_table->rt_config.freq_xi, p_rf_table->rt_config.freq_xo,
                p_rf_table->rt_config.ldo_rxafe, p_rf_table->rt_config.ldo_dcdcv, p_rf_table->rt_config.ldo_dldov,
                p_rf_table->rt_config.pa_vcas1, p_rf_table->rt_config.pa_vcas2, p_rf_table->rt_config.pa_vcas3, p_rf_table->rt_config.pa_bias, p_rf_table->rt_config.pa_cap,
                p_rf_table->rt_config.padpd_cali
                );
    LOG_PRINTF("HT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->ht_config.band_gain[0], p_rf_table->ht_config.band_gain[1], p_rf_table->ht_config.band_gain[2], p_rf_table->ht_config.band_gain[3], p_rf_table->ht_config.band_gain[4], p_rf_table->ht_config.band_gain[5], p_rf_table->ht_config.band_gain[6],
                p_rf_table->ht_config.freq_xi, p_rf_table->ht_config.freq_xo,
                p_rf_table->ht_config.ldo_rxafe, p_rf_table->ht_config.ldo_dcdcv, p_rf_table->ht_config.ldo_dldov,
                p_rf_table->ht_config.pa_vcas1, p_rf_table->ht_config.pa_vcas2, p_rf_table->ht_config.pa_vcas3, p_rf_table->ht_config.pa_bias, p_rf_table->ht_config.pa_cap,
                p_rf_table->ht_config.padpd_cali
                );
    LOG_PRINTF("HT2 Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->ht_2_config.band_gain[0], p_rf_table->ht_2_config.band_gain[1], p_rf_table->ht_2_config.band_gain[2], p_rf_table->ht_2_config.band_gain[3], p_rf_table->ht_2_config.band_gain[4], p_rf_table->ht_2_config.band_gain[5], p_rf_table->ht_2_config.band_gain[6],
                p_rf_table->ht_2_config.freq_xi, p_rf_table->ht_2_config.freq_xo,
                p_rf_table->ht_2_config.ldo_rxafe, p_rf_table->ht_2_config.ldo_dcdcv, p_rf_table->ht_2_config.ldo_dldov,
                p_rf_table->ht_2_config.pa_vcas1, p_rf_table->ht_2_config.pa_vcas2, p_rf_table->ht_2_config.pa_vcas3, p_rf_table->ht_2_config.pa_bias, p_rf_table->ht_2_config.pa_cap,
                p_rf_table->ht_2_config.padpd_cali
                );
    LOG_PRINTF("LT Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->lt_config.band_gain[0], p_rf_table->lt_config.band_gain[1], p_rf_table->lt_config.band_gain[2], p_rf_table->lt_config.band_gain[3], p_rf_table->lt_config.band_gain[4], p_rf_table->lt_config.band_gain[5], p_rf_table->lt_config.band_gain[6],
                p_rf_table->lt_config.freq_xi, p_rf_table->lt_config.freq_xo,
                p_rf_table->lt_config.ldo_rxafe, p_rf_table->lt_config.ldo_dcdcv, p_rf_table->lt_config.ldo_dldov,
                p_rf_table->lt_config.pa_vcas1, p_rf_table->lt_config.pa_vcas2, p_rf_table->lt_config.pa_vcas3, p_rf_table->lt_config.pa_bias, p_rf_table->lt_config.pa_cap,
                p_rf_table->lt_config.padpd_cali
                );
    LOG_PRINTF("LT2 Config:\t %2d %2d %2d %2d %2d %2d %2d %3d %3d %2d %2d %2d %2d %2d %2d %2d %2d %2d\r\n", 
                p_rf_table->lt_2_config.band_gain[0], p_rf_table->lt_2_config.band_gain[1], p_rf_table->lt_2_config.band_gain[2], p_rf_table->lt_2_config.band_gain[3], p_rf_table->lt_2_config.band_gain[4], p_rf_table->lt_2_config.band_gain[5], p_rf_table->lt_2_config.band_gain[6],
                p_rf_table->lt_2_config.freq_xi, p_rf_table->lt_2_config.freq_xo,
                p_rf_table->lt_2_config.ldo_rxafe, p_rf_table->lt_2_config.ldo_dcdcv, p_rf_table->lt_2_config.ldo_dldov,
                p_rf_table->lt_2_config.pa_vcas1, p_rf_table->lt_2_config.pa_vcas2, p_rf_table->lt_2_config.pa_vcas3, p_rf_table->lt_2_config.pa_bias, p_rf_table->lt_2_config.pa_cap,
                p_rf_table->lt_2_config.padpd_cali
                );    
    LOG_PRINTF("===================\r\n");
    LOG_PRINTF("RF Gain:\t %2d\r\n", p_rf_table->rf_gain);
    LOG_PRINTF("B Rate Gain:\t %2d\r\n", p_rf_table->rate_gain_b);
    LOG_PRINTF("G Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_g.rate1, p_rf_table->rate_config_g.rate2, p_rf_table->rate_config_g.rate3, p_rf_table->rate_config_g.rate4 );
    LOG_PRINTF("20N Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_20n.rate1, p_rf_table->rate_config_20n.rate2, p_rf_table->rate_config_20n.rate3, p_rf_table->rate_config_20n.rate4 );
    LOG_PRINTF("40N Rate Gain:\t %2d, %2d, %2d, %2d\r\n", p_rf_table->rate_config_40n.rate1, p_rf_table->rate_config_40n.rate2, p_rf_table->rate_config_40n.rate3, p_rf_table->rate_config_40n.rate4 );
    LOG_PRINTF("Temperature Boundary:\t %d, %d\r\n", p_rf_table->low_boundary, p_rf_table->high_boundary);
    LOG_PRINTF("===================\r\n");       
    LOG_PRINTF("5G RT Config:\t %2d %2d %2d %2d 0x%x 0x%x\r\n", 
                p_rf_table->rt_5g_config.bbscale_band0, p_rf_table->rt_5g_config.bbscale_band1, p_rf_table->rt_5g_config.bbscale_band2, p_rf_table->rt_5g_config.bbscale_band3,
                p_rf_table->rt_5g_config.bias1, p_rf_table->rt_5g_config.bias2
                );
    LOG_PRINTF("5G HT Config:\t %2d %2d %2d %2d 0x%x 0x%x\r\n", 
                p_rf_table->ht_5g_config.bbscale_band0, p_rf_table->ht_5g_config.bbscale_band1, p_rf_table->ht_5g_config.bbscale_band2, p_rf_table->ht_5g_config.bbscale_band3,
                p_rf_table->ht_5g_config.bias1, p_rf_table->ht_5g_config.bias2
                );
    LOG_PRINTF("5G LT Config:\t %2d %2d %2d %2d 0x%x 0x%x\r\n", 
                p_rf_table->lt_5g_config.bbscale_band0, 
                p_rf_table->lt_5g_config.bbscale_band1, p_rf_table->lt_5g_config.bbscale_band2, p_rf_table->lt_5g_config.bbscale_band3,
                p_rf_table->lt_5g_config.bias1, p_rf_table->lt_5g_config.bias2
                );    
    LOG_PRINTF("5G Band Threshold:\t %4d %4d %4d\r\n", p_rf_table->band_f0_threshold, p_rf_table->band_f1_threshold, p_rf_table->band_f2_threshold);    
    LOG_PRINTF("===================\r\n");       
    return 0;
}


void customer_setting_rftable(int is_5g)
{
    if (is_5g)
    {
        def_rf_table.rt_config.padpd_cali = 1;
        def_rf_table.ht_config.padpd_cali = 1;
        def_rf_table.lt_config.padpd_cali = 1;
        def_rf_table.ht_2_config.padpd_cali = 1;
        def_rf_table.lt_2_config.padpd_cali = 1;
    }
    else
    {
        def_rf_table.rt_config.padpd_cali = 0;
        def_rf_table.ht_config.padpd_cali = 0;
        def_rf_table.lt_config.padpd_cali = 0;
        def_rf_table.ht_2_config.padpd_cali = 0;
        def_rf_table.lt_2_config.padpd_cali = 0;
    }
}

ssv_type_u8 vendor_info[] = {
    0x00, 0x00, 0x02, 0x00, 0x21, 0x01, 0x0A, 0x43, 0x61, 0x72, 0x20, 0x43, 0x44, 0x2F, 0x44, 0x56, 
    0x44, 0x02, 0x07, 0x53, 0x75, 0x6E, 0x70, 0x6C, 0x75, 0x73, 0x03, 0x09, 0x41, 0x6E, 0x67, 0x65, 
    0x6C, 0x20, 0x31, 0x2E, 0x30, 0x04, 0x03, 0x00, 0xA0, 0x40, 0x06, 0x06, 0xFC, 0x4B, 0xBC, 0x87, 
    0xFF, 0xAE, 0x07, 0x06, 0xFC, 0x4B, 0xBC, 0x87, 0xFF, 0xAF, 0xDD, 0x04, 0x00, 0xA0, 0x40, 0x00};

ssv_type_s8 customer_get_vendor_ie_info(ssv_type_u8 *OUI, ssv_type_u8 **info, int *len)
{          
     return -1;
     if (!OUI || !info || !len)
     {
        return -1;
     }
     OUI[0] = 0x4C;
     OUI[1] = 0xA3;
     OUI[2] = 0xA7;

      *len = 64;
      *info = vendor_info;
      return 0;
}

ssv_type_u32 customer_ap_auto_select_channel(ssv_type_u16 channel_mask, enum channel_bandwidth              ch_bw)
{   
    int i = 0;
    ssv_type_u32 count = 0xFFFFFFFF, minCount = 0xFFFFFFFF, best_channel = 0xFFFFFFFF;
        
    for(i = 0; i < MAX_2G_CHANNEL_NUM; i++)
    {
        if(channel_mask&(0x01<<i))
        {
            //LOG_PRINTF("  %2d      %7d       %5d     %5d \r\n",i,gDeviceInfo->APInfo->channel_edcca_count[i],gDeviceInfo->APInfo->channel_packet_count[i],ssv6xxx_aplist_get_count_by_channel(i));
            count = ssv6xxx_aplist_get_count_by_channel(ssv6xxx_wifi_ch_bitmask_to_num(FALSE,i));
            LOG_PRINTF("Channel %d,  AP num %d\r\n", ssv6xxx_wifi_ch_bitmask_to_num(FALSE,i), count);
            if((0xFFFFFFFF == minCount) && (0xFFFFFFFF == best_channel)){
                minCount = count;
                best_channel = i;
            }

            if(count <= minCount){
                minCount = count;
                best_channel = i;
            }
        }
    }

    return best_channel;
}


