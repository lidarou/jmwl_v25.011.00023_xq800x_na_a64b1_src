/**************************************************************************
**   FileName: CJC8911_Reg.h                                              *
**     Author: tf                                                         *
**  Copyright: Wuhan GuangHuaXin Technology (C)2012. All rights reserved  *
**   HomePage: www.csc-ic.com                                             *
**       Date: 2018-03-15 13:58:09                                        *
**Description: The register address seek list of CJC8911                  *
***************************************************************************/
#ifndef CJC8911Reg_H_
#define CJC8911Reg_H_


#ifdef      CJC8911_CSB_Pull_up
#define     CJC8911_Write_Address                      0x36
#define     CJC8911_Read_Address                       0x37
#endif

/* register addr */

#define     CJC8911_REG_NUM                             0x100   // Total number of registers

#define     CJC8911_R0_LEFT_INPUT_VOLUME                0x01   // Audio input left channel volume; R/W

#define     CJC8911_R2_LOUT1_VOLUME                     0x05   // Audio output letf channel1 volume; R/W


#define     CJC8911_R5_ADC_DAC_CONTROL                  0x0A   // ADC and DAC CONTROL; R/W
#define     CJC8911_R7_AUDIO_INTERFACE                  0x0E   // Digital Audio interface format; R/W
#define     CJC8911_R8_SAMPLE_RATE                      0x10   // Clock and Sample rate control; R/W

#define     CJC8911_R10_LEFT_DAC_VOLUME                 0x15   // Left channel DAC digital volume; R/W
#define     CJC8911_R11_RIGHT_DAC_VOLUME                0x17   // Right channel DAC digital volume; R/W

#define     CJC8911_R12_BASS_CONTROL                    0x18   // BASS control; R/W
#define     CJC8911_R13_TREBLE_CONTROL                  0x1A   // Treble control; R/W
#define     CJC8911_R15_RESET                           0x1E   // RESET  ; R/W
#define     CJC8911_R16_3D_CONTROL                      0x20   // 3D control; R/W
#define     CJC8911_R17_ALC1_CONTROL                   0x22   // ALC1; R/W

#define     CJC8911_R18_ALC2_CONTROL                     0x24   // ALC2; R/W

#define     CJC8911_R19_ALC3_CONTROL                    0x26   // ALC3; R/W
#define     CJC8911_R20_NOISE_GATE_CONTROL              0x28  // NOISE_GATE_CONTROL; R/W
#define     CJC8911_R21_LEFT_ADC_VOLUME                 0x2B   // Left ADC digital volume; R/W
#define     CJC8911_R22_RIGHT_ADC_VOLUME                 0x2D   // Right ADC digital volume; R/W

#define     CJC8911_R23_ADDITIONAL_CONTROL1             0x2E   // Additional control; R/W
#define     CJC8911_R24_ADDITIONAL_CONTROL2             0x30   // Additional control; R/W
#define     CJC8911_R27_ADDITIONAL_CONTROL3             0x36   // Additional control; R/W

#define     CJC8911_R31_ADC_INPUT_MODE                  0x3E   // ADC input mode; R/W
#define     CJC8911_R32_ADCL_SIGNAL_PATH                0x41   // ADC signal path; R/W
#define     CJC8911_R33_MIC                           0x42   // MIC

#define     CJC8911_R34_AUX                           0x44   //AUX; R/W

#define     CJC8911_R35_LEFT_OUT_MIX2_L                 0x46   // Left out Mix (2); R/W
#define     CJC8911_R35_LEFT_OUT_MIX2_H                 0X47   //
#define     CJC8911_R36_RIGHT_OUT_MIX1_L                0x48   // Right out Mix (1; R/W
#define     CJC8911_R36_RIGHT_OUT_MIX1_H                0x49   // Left DAC to Right mixer enable; R/W
#define     CJC8911_R37_ADC_PDN                         0x4A   // Adc_pdn se; R/W

#define     CJC8911_R43_LOW_POWER_PLAYBACK              0x86   // Low power playback; R/W

#define     CJC8911_R25_PWR_MGMT1_L                     0x32   // Power management1 and VMIDSEL; R/W
#define     CJC8911_R25_PWR_MGMT1_H                     0x33   // Power management1 and VMIDSEL; R/W
#define     CJC8911_R26_PWR_MGMT2_L                     0x34   // Power management2 and DAC left Power down; R/W
#define     CJC8911_R26_PWR_MGMT2_H                     0x35   // Power management2 and DAC right Power up; R/W

#endif
