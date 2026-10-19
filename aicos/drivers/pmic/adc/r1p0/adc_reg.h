
/*****************************************************************************/
/*                                                                           */
/*         Copyright 2022 by AIC Micro Corporation. All rights reserved.     */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 * This file defines the glob register header, generated at 2022-09-19 14:58:53
 */

#ifndef __ADCCTRL_H__
#define __ADCCTRL_H__

#define REG_ADC_CTRL_ADC_TEST_MODE                                              (PMIC_BASE_ADC_CTRL + 0x0)
#define REG_ADC_CTRL_ADC_RSTN                                                   (PMIC_BASE_ADC_CTRL + 0x4)
#define REG_ADC_CTRL_ADC_SOC                                                    (PMIC_BASE_ADC_CTRL + 0x8)
#define REG_ADC_CTRL_ADC_AIN                                                    (PMIC_BASE_ADC_CTRL + 0xc)
#define REG_ADC_CTRL_ADC_SDIF                                                   (PMIC_BASE_ADC_CTRL + 0x10)
#define REG_ADC_CTRL_ADC_DISH                                                   (PMIC_BASE_ADC_CTRL + 0x14)
#define REG_ADC_CTRL_ADC_OFFSET                                                 (PMIC_BASE_ADC_CTRL + 0x18)
#define REG_ADC_CTRL_GLB_CTRL_ADC                                               (PMIC_BASE_ADC_CTRL + 0x1c)
#define REG_ADC_CTRL_ADC_SAMP_CH                                                (PMIC_BASE_ADC_CTRL + 0x20)
#define REG_ADC_CTRL_ADC_CH_SDIF                                                (PMIC_BASE_ADC_CTRL + 0x24)
#define REG_ADC_CTRL_ADC_SAMP_EN                                                (PMIC_BASE_ADC_CTRL + 0x28)
#define REG_ADC_CTRL_ADC_TRIG                                                   (PMIC_BASE_ADC_CTRL + 0x2c)
#define REG_ADC_CTRL_TSC_DEBOUNCE                                               (PMIC_BASE_ADC_CTRL + 0x30)
#define REG_ADC_CTRL_TSC_PRESS                                                  (PMIC_BASE_ADC_CTRL + 0x34)
#define REG_ADC_CTRL_TSC_TRIG_TIME                                              (PMIC_BASE_ADC_CTRL + 0x38)
#define REG_ADC_CTRL_TSC_CFG_TIME                                               (PMIC_BASE_ADC_CTRL + 0x3c)
#define REG_ADC_CTRL_TSC_SAMP_EN                                                (PMIC_BASE_ADC_CTRL + 0x40)
#define REG_ADC_CTRL_EOC_SAMP_POL                                               (PMIC_BASE_ADC_CTRL + 0x44)
#define REG_ADC_CTRL_TSC_GPIO8_CTRL                                             (PMIC_BASE_ADC_CTRL + 0x48)
#define REG_ADC_CTRL_TSC_GPIO9_CTRL                                             (PMIC_BASE_ADC_CTRL + 0x4c)
#define REG_ADC_CTRL_TSC_GPIO10_CTRL                                            (PMIC_BASE_ADC_CTRL + 0x50)
#define REG_ADC_CTRL_TSC_GPIO11_CTRL                                            (PMIC_BASE_ADC_CTRL + 0x54)
#define REG_ADC_CTRL_INT_EN                                                     (PMIC_BASE_ADC_CTRL + 0x58)
#define REG_ADC_CTRL_INT_CLR                                                    (PMIC_BASE_ADC_CTRL + 0x5c)
#define REG_ADC_CTRL_ADC_INT_STS                                                (PMIC_BASE_ADC_CTRL + 0x60)
#define REG_ADC_CTRL_ADC_INT_STS_RAW                                            (PMIC_BASE_ADC_CTRL + 0x64)
#define REG_ADC_CTRL_ADC_DATA                                                   (PMIC_BASE_ADC_CTRL + 0x68)
#define REG_ADC_CTRL_TSC_DATA                                                   (PMIC_BASE_ADC_CTRL + 0x6c)
#define REG_ADC_CTRL_ADC_AIP_DBG                                                (PMIC_BASE_ADC_CTRL + 0x70)
#define REG_ADC_CTRL_ADC_DBG                                                    (PMIC_BASE_ADC_CTRL + 0x78)

/*REG_ADC_CTRL_ADC_TEST_MODE*/
#define BIT_ADC_CTRL_ADC1_SW_TEST_MODE                                          BIT(0)

/*REG_ADC_CTRL_ADC_RSTN*/
#define BIT_ADC_CTRL_RSTN_ADC1_18_REG_EB                                        BIT(1)
#define BIT_ADC_CTRL_RSTN_ADC1_18_REG                                           BIT(0)

/*REG_ADC_CTRL_ADC_SOC*/
#define BIT_ADC_CTRL_SOC_ADC1_18_REG_EB                                         BIT(1)
#define BIT_ADC_CTRL_SOC_ADC1_18_REG                                            BIT(0)

/*REG_ADC_CTRL_ADC_AIN*/
#define BIT_ADC_CTRL_SAINADC1_18_REG_EB                                         BIT(4)
#define BIT_ADC_CTRL_SAINADC1_18_REG(x)                                         (((x) & 0xf))

/*REG_ADC_CTRL_ADC_SDIF*/
#define BIT_ADC_CTRL_SDIFADC1_18_REG_EB                                         BIT(1)
#define BIT_ADC_CTRL_SDIFADC1_18_REG                                            BIT(0)

/*REG_ADC_CTRL_ADC_DISH*/
#define BIT_ADC_CTRL_REG_DISHADC1_18                                            BIT(0)

/*REG_ADC_CTRL_ADC_OFFSET*/
#define BIT_ADC_CTRL_REG_OFFSET_ADC1_18(x)                                      (((x) & 0x3f))

/*REG_ADC_CTRL_GLB_CTRL_ADC*/
#define BIT_ADC_CTRL_SOC_HIGN_ADC_WAIT_NUM(x)                                   (((x) & 0xf) << 8)
#define BIT_ADC_CTRL_RSTN_HIGN_ADC_WAIT_NUM(x)                                  (((x) & 0xf) << 4)
#define BIT_ADC_CTRL_EN_HIGN_ADC_WAIT_NUM(x)                                    (((x) & 0xf))

/*REG_ADC_CTRL_ADC_SAMP_CH*/
#define BIT_ADC_CTRL_ADC1_AIN_CH_SEL(x)                                         (((x) & 0xf))

/*REG_ADC_CTRL_ADC_CH_SDIF*/
#define BIT_ADC_CTRL_ADC1_CH_SDIF_EN                                            BIT(0)

/*REG_ADC_CTRL_ADC_SAMP_EN*/
#define BIT_ADC_CTRL_ADC1_AIN_SAMP_EN                                           BIT(0)

/*REG_ADC_CTRL_ADC_TRIG*/
#define BIT_ADC_CTRL_ADC1_MANUAL_TRIG                                           BIT(0)

/*REG_ADC_CTRL_TSC_DEBOUNCE*/
#define BIT_ADC_CTRL_TSC_DEBOUNCE_CHK_NUM(x)                                    (((x) & 0x1ff))

/*REG_ADC_CTRL_TSC_PRESS*/
#define BIT_ADC_CTRL_TSC_PRESS_CHK_NUM(x)                                       (((x) & 0x1ff))

/*REG_ADC_CTRL_TSC_TRIG_TIME*/
#define BIT_ADC_CTRL_TSC_NEXT_TRIG_NUM(x)                                       (((x) & 0x3f))

/*REG_ADC_CTRL_TSC_CFG_TIME*/
#define BIT_ADC_CTRL_TSC_PRE_CHARGE_NUM(x)                                      (((x) & 0x1ff))

/*REG_ADC_CTRL_TSC_SAMP_EN*/
#define BIT_ADC_CTRL_TSC_SAMP_EN                                                BIT(0)

/*REG_ADC_CTRL_EOC_SAMP_POL*/
#define BIT_ADC_CTRL_ADC1_EOC_SAMP_POL                                          BIT(0)

/*REG_ADC_CTRL_TSC_GPIO8_CTRL*/
#define BIT_ADC_CTRL_TSC_GPIO8_DRV                                              BIT(1)
#define BIT_ADC_CTRL_TSC_GPIO8_IS                                               BIT(0)

/*REG_ADC_CTRL_TSC_GPIO9_CTRL*/
#define BIT_ADC_CTRL_TSC_GPIO9_DRV                                              BIT(1)
#define BIT_ADC_CTRL_TSC_GPIO9_IS                                               BIT(0)

/*REG_ADC_CTRL_TSC_GPIO10_CTRL*/
#define BIT_ADC_CTRL_TSC_GPIO10_DRV                                             BIT(1)
#define BIT_ADC_CTRL_TSC_GPIO10_IS                                              BIT(0)

/*REG_ADC_CTRL_TSC_GPIO11_CTRL*/
#define BIT_ADC_CTRL_TSC_GPIO11_DRV                                             BIT(1)
#define BIT_ADC_CTRL_TSC_GPIO11_IS                                              BIT(0)

/*REG_ADC_CTRL_INT_EN*/
#define BIT_ADC_CTRL_TSC_INTR_EN                                                BIT(1)
#define BIT_ADC_CTRL_ADC_INTR_EN                                                BIT(0)

/*REG_ADC_CTRL_INT_CLR*/
#define BIT_ADC_CTRL_TSC_INTR_CLR                                               BIT(1)
#define BIT_ADC_CTRL_ADC_INTR_CLR                                               BIT(0)

/*REG_ADC_CTRL_ADC_INT_STS*/
#define BIT_ADC_CTRL_TSC_INTR_STS                                               BIT(1)
#define BIT_ADC_CTRL_ADC_INTR_STS                                               BIT(0)

/*REG_ADC_CTRL_ADC_INT_STS_RAW*/
#define BIT_ADC_CTRL_TSC_INTR_STS_RAW                                           BIT(1)
#define BIT_ADC_CTRL_ADC_INTR_STS_RAW                                           BIT(0)

/*REG_ADC_CTRL_ADC_DATA*/
#define BIT_ADC_CTRL_ADC1_DATA(x)                                               (((x) & 0xfff))

/*REG_ADC_CTRL_TSC_DATA*/
#define BIT_ADC_CTRL_TSC_SAMPLING                                               BIT(17)
#define BIT_ADC_CTRL_TSC_PRESS_FLAG                                             BIT(16)
#define BIT_ADC_CTRL_TSC_Y_DATA(x)                                              (((x) & 0xff) << 8)
#define BIT_ADC_CTRL_TSC_X_DATA(x)                                              (((x) & 0xff))

/*REG_ADC_CTRL_ADC_AIP_DBG*/
#define BIT_ADC_CTRL_EOC_ADC1_18                                                BIT(8)
#define BIT_ADC_CTRL_REG_SAINADC1_18(x)                                         (((x) & 0xf) << 4)
#define BIT_ADC_CTRL_SOC_ADC1_18                                                BIT(3)
#define BIT_ADC_CTRL_REG_SDIFADC1_18                                            BIT(2)
#define BIT_ADC_CTRL_RSTN_ADC1_18                                               BIT(1)
#define BIT_ADC_CTRL_EN_ADC1_18                                                 BIT(0)

/*REG_ADC_CTRL_ADC_DBG*/
#define BIT_ADC_CTRL_SOC_CNT_REPEAT(x)                                          (((x) & 0xf) << 20)
#define BIT_ADC_CTRL_SOC_CNT(x)                                                 (((x) & 0xfff) << 8)
#define BIT_ADC_CTRL_ADC1_FSM_BUSY                                              BIT(5)
#define BIT_ADC_CTRL_ADC1_FSM_CS(x)                                             (((x) & 0x1f))


#define GP202_CKM_EN_GLB_CLK_PD_ADC                                               (0x8018)
#define BIT_CKM_EN_GLB_CLK_PD_ADC                                                 BIT(0)

#define PMIC_BASE_ADC                                                             (0x00009400)
#define GP202_ADC_CTRL_GP202_ADC_RG_IBIAS_SEL                                     (PMIC_BASE_ADC + 0x0)
#define GP202_ADC_CTRL_GP202_ADC_LDO_CFG                                          (PMIC_BASE_ADC + 0x4)
#define GP202_ADC_CTRL_GP202_ADC_RG_FLT_CFG                                       (PMIC_BASE_ADC + 0x8)
#define GP202_ADC_CTRL_GP202_ADC_CAL_CFG                                          (PMIC_BASE_ADC + 0xc)
#define GP202_ADC_CTRL_GP202_ADC_OFFSET                                           (PMIC_BASE_ADC + 0x10)
#define GP202_ADC_CTRL_GP202_ADC_AIN_SEL                                          (PMIC_BASE_ADC + 0x14)
#define GP202_ADC_CTRL_GP202_ADC_INT_EN                                           (PMIC_BASE_ADC + 0x18)
#define GP202_ADC_CTRL_GP202_ADC_INT_MASK                                         (PMIC_BASE_ADC + 0x1c)
#define GP202_ADC_CTRL_GP202_ADC_INT_CLR                                          (PMIC_BASE_ADC + 0x20)
#define GP202_ADC_CTRL_REG_SAMP_WAIT_NUM                                          (PMIC_BASE_ADC + 0x24)
#define GP202_ADC_CTRL_GP202_ADC_TRIG                                             (PMIC_BASE_ADC + 0x28)
#define GP202_ADC_CTRL_GP202_ADC_CLK_EDGE_SEL                                     (PMIC_BASE_ADC + 0x2c)
#define GP202_ADC_CTRL_ADC_EN_FORCE                                               (PMIC_BASE_ADC + 0x30)
#define GP202_ADC_CTRL_ADC_LDO_EN_FORCE                                           (PMIC_BASE_ADC + 0x34)
#define GP202_ADC_CTRL_ADC_RSTN_FORCE                                             (PMIC_BASE_ADC + 0x38)
#define GP202_ADC_CTRL_ANA_FLAG_OK_INV                                            (PMIC_BASE_ADC + 0x3c)
#define GP202_ADC_CTRL_ADC_INT_STS                                                (PMIC_BASE_ADC + 0x50)
#define GP202_ADC_CTRL_ADC_INT_STS_RAW                                            (PMIC_BASE_ADC + 0x54)
#define GP202_ADC_CTRL_ADC_DATA_RPT                                               (PMIC_BASE_ADC + 0x58)
#define GP202_ADC_CTRL_ADC_FSM_CS_RPT                                             (PMIC_BASE_ADC + 0x5c)
#define GP202_ADC_CTRL_DBG_ANA_INTERFACE                                          (PMIC_BASE_ADC + 0x60)
/*GP202_ADC_CTRL_GP202_ADC_RG_IBIAS_SEL*/
#define BIT_ADC_CTRL_GP202_ADC_RG_IBIAS_SEL(x)                                    (((x) & 0xf))

/*GP202_ADC_CTRL_GP202_ADC_LDO_CFG*/
#define BIT_ADC_CTRL_GP202_ADC_LDO_TRIM(x)                                        (((x) & 0x7) << 4)
#define BIT_ADC_CTRL_GP202_ADC_LDO_CTUNE(x)                                       (((x) & 0x7))

/*GP202_ADC_CTRL_GP202_ADC_RG_FLT_CFG*/
#define BIT_ADC_CTRL_GP202_ADC_RG_FLTRES(x)                                       (((x) & 0xf) << 4)
#define BIT_ADC_CTRL_GP202_ADC_RG_FLTCAP(x)                                       (((x) & 0xf))

/*GP202_ADC_CTRL_GP202_ADC_CAL_CFG*/
#define BIT_ADC_CTRL_GP202_ADC_CALIBRA_EN                                         BIT(4)
#define BIT_ADC_CTRL_GP202_ADC_CAL(x)                                             (((x) & 0xf))

/*GP202_ADC_CTRL_GP202_ADC_OFFSET*/
#define BIT_ADC_CTRL_GP202_ADC_OFFSET_MODE                                        BIT(8)
#define BIT_ADC_CTRL_GP202_ADC_OFFSET_VAL(x)                                      (((x) & 0x3f))

/*GP202_ADC_CTRL_GP202_ADC_AIN_SEL*/
#define BIT_ADC_CTRL_GP202_ADC_AIN_SEL(x)                                         (((x) & 0xf))

/*GP202_ADC_CTRL_GP202_ADC_INT_EN*/
#define BIT_ADC_CTRL_GP202_ADC_INT_EN                                             BIT(0)

/*GP202_ADC_CTRL_GP202_ADC_INT_MASK*/
#define BIT_ADC_CTRL_GP202_ADC_INT_MASK                                           BIT(0)

/*GP202_ADC_CTRL_GP202_ADC_INT_CLR*/
#define BIT_ADC_CTRL_GP202_ADC_INT_CLR                                            BIT(0)

/*GP202_ADC_CTRL_REG_SAMP_WAIT_NUM*/
#define BIT_ADC_CTRL_GP202_ADC_EN_WAIT_NUM(x)                                     (((x) & 0x3f) << 4)
#define BIT_ADC_CTRL_GP202_ADC_RSTN_WAIT_NUM(x)                                   (((x) & 0xf))

/*GP202_ADC_CTRL_GP202_ADC_TRIG*/
#define BIT_ADC_CTRL_GP202_ADC_TRIG                                               BIT(0)

/*GP202_ADC_CTRL_GP202_ADC_CLK_EDGE_SEL*/
#define BIT_ADC_CTRL_GP202_ADC_CLK_EDGE_SEL                                       BIT(0)

/*GP202_ADC_CTRL_ADC_EN_FORCE*/
#define BIT_ADC_CTRL_ADC_EN_FORCE_VAL                                             BIT(1)
#define BIT_ADC_CTRL_ADC_EN_FORCE_EB                                              BIT(0)

/*GP202_ADC_CTRL_ADC_LDO_EN_FORCE*/
#define BIT_ADC_CTRL_ADC_LDO_EN_FORCE_VAL                                         BIT(1)
#define BIT_ADC_CTRL_ADC_LDO_EN_FORCE_EB                                          BIT(0)

/*GP202_ADC_CTRL_ADC_RSTN_FORCE*/
#define BIT_ADC_CTRL_ADC_RSTN_FORCE_VAL                                           BIT(1)
#define BIT_ADC_CTRL_ADC_RSTN_FORCE_EB                                            BIT(0)

/*GP202_ADC_CTRL_ANA_FLAG_OK_INV*/
#define BIT_ADC_CTRL_REG_FLAG_OK_INV                                              BIT(0)

/*GP202_ADC_CTRL_ADC_INT_STS*/
#define BIT_ADC_CTRL_ADC_INT_STS                                                  BIT(0)

/*GP202_ADC_CTRL_ADC_INT_STS_RAW*/
#define BIT_ADC_CTRL_ADC_INT_STS_RAW                                              BIT(0)

/*GP202_ADC_CTRL_ADC_DATA_RPT*/
#define BIT_ADC_CTRL_DBG_ADC_DATA_ERR                                             BIT(12)
#define BIT_ADC_CTRL_ADC_DATA_RPT(x)                                              (((x) & 0xfff))

/*GP202_ADC_CTRL_ADC_FSM_CS_RPT*/
#define BIT_ADC_CTRL_ADC_FSM_CS_RPT(x)                                            (((x) & 0xf))

/*GP202_ADC_CTRL_DBG_ANA_INTERFACE*/
#define BIT_ADC_CTRL_ADC_AINSEL_18(x)                                             (((x) & 0x3ff) << 4)
#define BIT_ADC_CTRL_ADC_RSTN_18                                                  BIT(2)
#define BIT_ADC_CTRL_ADC_LDO_EN_18                                                BIT(1)
#define BIT_ADC_CTRL_ADC_EN_18                                                    BIT(0)
#endif /*__ADCCTRL_H__*/
