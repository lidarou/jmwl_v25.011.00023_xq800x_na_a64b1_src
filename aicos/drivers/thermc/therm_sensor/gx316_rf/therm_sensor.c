/*****************************************************************************/
/*                                                                           */
/*         Copyright 2023 by AIC Corporation. All rights reserved.           */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the gx316 rf thermal sensor ops interface
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x139dbc
#define LOG_MODULE    AMOD_DRV

#include <aic_osa.h>
#include <aic_io.h>
#include <aic_at_process_common.h>
#include <aic_cmdline.h>
#include <aic_init.h>
#include <aic_reg_base.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_at_resp_handle.h>
#include <irq_chip.h>
#include "../../therm_internal.h"
#include <slp_inc.h>
#include <efuse.h>
#include <timer_inc.h>
#define REG_BASE_TOP_RF_SDM         (REG_BASE_TOP_RF_TOP + 0x4000)
#define REG_BASE_APLL_TOP           (REG_BASE_TOP_PLL_RF + 0x0800)
#define REG_BASE_APLL_SDM           REG_BASE_TOP_PLL_RF
#define REG_BASE_UPLL_TOP           (REG_BASE_TOP_PLL_RF + 0x8800)
#define REG_BASE_UPLL_SDM           (REG_BASE_TOP_PLL_RF + 0x8000)
#define REG_BASE_GPLL_TOP           (REG_BASE_TOP_PLL_RF + 0x4800)
#define REG_BASE_GPLL_SDM           (REG_BASE_TOP_PLL_RF + 0x4000)

static void reg32_update(u32 pmu_reg, u32 pmu_msk, u32 pmu_val)
{
    u32 val;

    val = AIC_READ32(pmu_reg);
    val &= ~pmu_msk;

    pmu_val <<= aic_find_first_bit(pmu_msk) - 1;

    val |= pmu_val;

    AIC_WRITE32(pmu_reg, val);
}

#ifdef RF_INIT_BY_AP
void bbpll_init(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x0300, 4);//reg_rf_top_145  xdrv_pu_src_sel
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 14);//reg_bb_xdrv_sw_cfg  bb_xdrv_pu_cfg_wen
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 6);//reg_bb_xdrv_sw_cfg  bb_xdrv_pu_cfg
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 11);//reg_bb_xdrv_sw_cfg  bb_xdrv_cken4_cfg_wen
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x007C, 6);//xdrv_bit1   XDRV_BUFFER CLK_TEST_Enable
    /*
    bbpll_bit2  "[4] REF208M_MODE=0
    [3] REF208M_EN=0
    [2] REF26M_X2_EN=0  0: 26MHz Mode 1: 52MHz Mode
    [1] REF26M_EN=1"
    */
    reg32_update(REG_BASE_TOP_RF_TOP + 0x006C, GENMASK(1,4), 1);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x00B4, 5);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x0068, GENMASK(2,4), 2);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0078, 11);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x0078, GENMASK(5,7), 0);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0308, 1);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0308, 9);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x01B4, 15);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x01C8, GENMASK(4,10), 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x0234, 9);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x0080, GENMASK(0, 6), 0x60);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0308, 0);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0308, 8);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0300, 0);
    aic_mdelay(10);
    reg32_update(REG_BASE_TOP_RF_SDM + 0x18, GENMASK(0, 15), 0x38B6);
    reg32_update(REG_BASE_TOP_RF_SDM + 0x14, GENMASK(0, 15), 0xBE9F);
    reg32_update(REG_BASE_TOP_RF_SDM + 0x30, GENMASK(0, 2), 0x3);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_SDM + 0x08, 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_SDM + 0x08, 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_SDM, 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_SDM, 0);
    AIC_WRITE32(0XF001000C, 0X38010ab);
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
}

void gx316_default_init(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);

    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x007C, 0x0800);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0084, 0x0058);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x00A0, 0x123F);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x00D0, 0xCB0C);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x00D4, 0x5318);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x00F8, 0x3EF0);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0100, 0x4424);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0108, 0x6570);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0110, 0x18C5);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0114, 0xD0D4);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0118, 0x0055);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x012C, 0x7FA3);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0140, 0x0A77);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0178, 0x0006);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0184, 0x7FA3);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0198, 0x0A77);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x01C8, 0x4008);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0240, 0x2494);
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x01FC, 0x001F);
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
}

void txdac_310_init(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x28, 0x8808 );
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x50, 0x1    );
    AIC_WRITE32(REG_BASE_APLL_SDM       , 0x1    );
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x14, 0x1    );
    AIC_WRITE32(REG_BASE_APLL_SDM       , 0x0    );
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x10, 0x1    );
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x0 , 0x12001);
    AIC_WRITE32(REG_BASE_APLL_TOP + 0x4 , 0x11901);
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x28, 0x8808 );
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x50, 0x1    );
    AIC_WRITE32(REG_BASE_UPLL_SDM       , 0x1    );
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x14, 0x1    );
    AIC_WRITE32(REG_BASE_UPLL_SDM       , 0x0    );
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x10, 0x1    );
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x8 , 0x12101);
    AIC_WRITE32(REG_BASE_UPLL_TOP + 0x24, 0x3    );
    AIC_WRITE32(REG_BASE_GPLL_TOP + 0x28, 0x8808 );
    AIC_WRITE32(REG_BASE_GPLL_TOP + 0x50, 0x1    );
    AIC_WRITE32(REG_BASE_GPLL_SDM       , 0x1    );
    AIC_WRITE32(REG_BASE_GPLL_TOP + 0x10, 0x1    );
    AIC_WRITE32(REG_BASE_GPLL_TOP + 0x24, 0x1    );
    aic_udelay(1000);
    AIC_WRITE32(0x42050008, 0x0  );
    AIC_WRITE32(0x400c0220, 0x0  );
    AIC_WRITE32(0x400c0230, 0x0  );
    AIC_WRITE32(0x400c0290, 0x0  );
    AIC_WRITE32(0x32350000, 0x0  );
    AIC_WRITE32(0x30030034, 0x1  );
    AIC_WRITE32(0x30030014, 0x1  );
    AIC_WRITE32(0x30030018, 0x1  );
    AIC_WRITE32(0x322c0000, 0x1  );
    AIC_WRITE32(0x322c0004, 0x1  );
    AIC_WRITE32(0x322c0008, 0x1  );
    AIC_WRITE32(0x322c000c, 0x1  );
    AIC_WRITE32(0x322c0010, 0x1  );
    AIC_WRITE32(0x322c0014, 0x1  );
    AIC_WRITE32(0x322c0018, 0x1  );
    AIC_WRITE32(0x322c001c, 0x1  );
    AIC_WRITE32(0x322c0020, 0x1  );
    AIC_WRITE32(0x322c0024, 0x1  );
    AIC_WRITE32(0x322c0028, 0x1  );
    AIC_WRITE32(0x322c002c, 0x1  );
    AIC_WRITE32(0x322c0030, 0x1  );
    AIC_WRITE32(0x322c0034, 0x1  );
    AIC_WRITE32(0x322c0038, 0x1  );
    AIC_WRITE32(0xb0000404, 0x3ff);
    AIC_WRITE32(0x322c0188, 0x1  );
    AIC_WRITE32(0x322c018c, 0x1  );
    AIC_WRITE32(0x32350000, 0x0  );
    AIC_WRITE32(0x32350004, 0x0  );
    AIC_WRITE32(0x42050008, 0x0  );
    AIC_WRITE32(0x411e0048, 0x0  );
    aic_mdelay(100);
    reg32_update(0x42010000, GENMASK(0,1),0);
    reg32_update(0x42010004, GENMASK(0,1),0);
    AIC_REG_CLRBIT(0x42010034, 0);
    reg32_update(0x411e0000, GENMASK(0,1),0);
    reg32_update(0x411e0030, GENMASK(0,1),1);
    reg32_update(0x411e0020, GENMASK(0,1),0);
    reg32_update(0x411e001c, GENMASK(0,1),0);
    AIC_WRITE32(0x322c0004, 0x1       );
    AIC_WRITE32(0x322c0028, 0x1       );
    AIC_WRITE32(0x32340000, 0xffffffff);
    AIC_WRITE32(0x411e001c, 0x0       );
    AIC_WRITE32(0x411e0048, 0x0       );
    AIC_WRITE32(0x400c0220, 0x0000    );
    AIC_WRITE32(0x400c0230, 0x0000    );
    AIC_WRITE32(0x400c0290, 0x0000    );
}
#endif

int rf_tsensor_start(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00BC, 1);//PU_CM_BG    打开CM bandgap
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00C4, 1);//PU_RX_BG    打开RX bandgap
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00C8, 0);//PU_TX_BG    打开TX bandgap
//    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0300, 4);//xdrv_pu_src_sel
//    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 14);//bb_xdrv_pu_cfg_wen
//    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 6);//bb_xdrv_pu_cfg
    reg32_update(REG_BASE_TOP_RF_TOP + 0x007c, GENMASK(14,15), 0);//XDRV_BUFFER_4给THERMAL模块的时钟分频控制信号：00为13MHz，01为8.6MHz，10为6.5MHz，11为4.3MHz
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x007c, 2);//XDRV_BUFFER_4里给THERM的26M的使能
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0240, 1);//RG_TS_CHOP_EN   开启termal int chop使能
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00CC, 0);//PU_TS   开启tsense power switch
    aic_udelay(1000);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0240, 0);//RG_TS_RSTN_INT  释放int复位信号

    /*
    REG_TXTOP_TEST<6:0> "PAD_TEST_DC测试端口使能：
    [10]TXTOP_TEST<6>DAC_LDO_REF
    [9]TXTOP_TEST<5>AVDD09_LDO_FLT
    [8]TXTOP_TEST<4>VCO_REG_BUF_TX
    [7]TXTOP_TEST<3>AVDD_GRO1
    [6]TXTOP_TEST<2>DVDD_CLK_BUF
    [5]TXTOP_TEST<1>DVDD
    [4]TXTOP_TEST<0>VCO_LDO_VREF_0P75"
    */
    reg32_update(REG_BASE_TOP_RF_TOP + 0x01C8, GENMASK(4, 10), 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x0234, 9);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x0080, GENMASK(0, 6), 0);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x01B4, 15); //ts_sdm_out_en   ts_sdm_out_en->  1:en   0:disable

    return 0;
}

u32 rf_tsensor_calibrate(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);

    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00BC, 1);//PU_CM_BG    打开CM bandgap
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00C4, 1);//PU_RX_BG    打开RX bandgap
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00C8, 0);//PU_TX_BG    打开TX bandgap
//    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0300, 4);//xdrv_pu_src_sel
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 14);//bb_xdrv_pu_cfg_wen
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0304, 6);//bb_xdrv_pu_cfg
    reg32_update(REG_BASE_TOP_RF_TOP + 0x007c, GENMASK(14,15), 0);//XDRV_BUFFER_4给THERMAL模块的时钟分频控制信号：00为13MHz，01为8.6MHz，10为6.5MHz，11为4.3MHz
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x007c, 2);//XDRV_BUFFER_4里给THERM的26M的使能
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0240, 1);//RG_TS_CHOP_EN   开启termal int chop使能
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x00CC, 0);//PU_TS   开启tsense power switch
    aic_udelay(1000);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x0240, 0);//RG_TS_RSTN_INT  释放int复位信号

    /*
    REG_TXTOP_TEST<6:0> "PAD_TEST_DC测试端口使能：
    [10]TXTOP_TEST<6>DAC_LDO_REF
    [9]TXTOP_TEST<5>AVDD09_LDO_FLT
    [8]TXTOP_TEST<4>VCO_REG_BUF_TX
    [7]TXTOP_TEST<3>AVDD_GRO1
    [6]TXTOP_TEST<2>DVDD_CLK_BUF
    [5]TXTOP_TEST<1>DVDD
    [4]TXTOP_TEST<0>VCO_LDO_VREF_0P75"
    */
    reg32_update(REG_BASE_TOP_RF_TOP + 0x01C8, GENMASK(4, 10), 0);
    AIC_REG_CLRBIT(REG_BASE_TOP_RF_TOP + 0x0234, 9);
    reg32_update(REG_BASE_TOP_RF_TOP + 0x0080, GENMASK(0, 6), 0);
    AIC_REG_SETBIT(REG_BASE_TOP_RF_TOP + 0x01B4, 15); //ts_sdm_out_en   ts_sdm_out_en->  1:en   0:disable

    AIC_WRITE32(0X41090040, 0X1);
    AIC_WRITE32(0X41120018, 0XBB8);
    AIC_WRITE32(0X41120004, 0X02580000);
    aic_udelay(1000);
    AIC_WRITE32(0X41120000, 0X4);
    aic_mdelay(10);

    return AIC_READ32(0x41120008);
}

#if defined (CONFIG_DRIVER_EFUSE) && (CONFIG_SUBSYS_AP)
VOID rf_thermal_init(void)
{
    u32 val;
    int ret;

    val = AIC_READ32(REG_BASE_TOP_RF_TOP + 0x007C);
    val &= 0x3FFF | 0x4;
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x007C, val);

    val = AIC_READ32(REG_BASE_TOP_RF_TOP + 0x0240);
    val |= 0x2;
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0240, val);

    val = AIC_READ32(REG_BASE_TOP_RF_TOP + 0x00CC);
    val |= 0x1;
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x00CC, val);

    val = AIC_READ32(REG_BASE_TOP_RF_TOP + 0x0240);
    val |= 0x1;
    AIC_WRITE32(REG_BASE_TOP_RF_TOP + 0x0240, val);

//	AIC_WRITE32(0x4109001C, 0x1);//REG_TOP_GLB_EFUSE_EB
//	AIC_WRITE32(0x41090020, 0x1);//REG_TOP_GLB_DJTAG_EB
    AIC_WRITE32(0x41090040, 0x1);//REG_TOP_GLB_THMC_EB
    AIC_WRITE32(0x41120018, 0xBB8);//REG_BASE_TOP_THMC + 0x18
    val = 0;
    ret = aic_efuse_block_read(0x4, &val);
    if(!ret && val) {
        if((((val>>12)&0xFFF) > 1040) && (((val>>12)&0xFFF) < 1940)) {
            AIC_WRITE32(0x4112000C, ((val>>12)&0xFFF)<<16);
        } else {
            AIC_WRITE32(0x4112000C, 0x05d40000);
        }
    } else {
        AIC_WRITE32(0x4112000C, 0x05d40000);//REG_BASE_TOP_THMC + 0xC
    }
    AIC_WRITE32(0x41120000,0x0);
    aic_mdelay(30);
    AIC_WRITE32(0x41120000,0x2);
}
#endif

int rf_tsensor_init(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
#ifdef RF_INIT_BY_AP
    AIC_WRITE32(0X400C0430, 0x0);
    txdac_310_init();
    gx316_default_init();

    //sync with rf init
    rf_thermal_init();
#endif

    return 0;
}

int rf_tsensor_stop(void)
{
    aicx_warn(0x09a6cb, "sds", "%s %d %s \n", __FILE__, __LINE__, __func__);
    return 0;
}

struct thermal_sensor_ops gx316_rf_tsensor = {
    .init = rf_tsensor_init,
    .start = rf_tsensor_start,
    .stop = rf_tsensor_stop,
    .get_factor = rf_tsensor_calibrate,
};

int gx316_tsensor_init(void)
{
    aic_tsensor_register(&gx316_rf_tsensor);

    return AIC_SUCCESS;
}

AIC_EARLY_INIT_EXPORT(gx316_tsensor_init);
