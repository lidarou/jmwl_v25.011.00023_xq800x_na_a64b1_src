#define		CJC8910_IIC_ADDRESS                         0x18   //CE state low	0x30 ;	CE state high 0X32

#define     CJC8910_R0_LEFT_INPUT_VOLUME                0x01   // Audio input left channel volume; R/W
#define     CJC8910_R2_LOUT1_VOLUME                     0x05   // Audio output letf channel1 volume; R/W
#define     CJC8910_R5_ADC_DAC_CONTROL                  0x0A   // ADC and DAC CONTROL; R/W
#define     CJC8910_R7_AUDIO_INTERFACE                  0x0E   // Digital Audio interface format; R/W
#define     CJC8910_R8_SAMPLE_RATE                      0x10   // Clock and Sample rate control; R/W

#define     CJC8910_R10_LEFT_DAC_VOLUME                 0x15   // Left channel DAC digital volume; R/W

#define     CJC8910_R12_BASS_CONTROL                    0x18   // BASS control; R/W
#define     CJC8910_R13_TREBLE_CONTROL                  0x1A   // Treble control; R/W
#define     CJC8910_R15_RESET                           0x1E   // RESET  ; R/W

#define     CJC8910_R17_ALC1_CONTROL                   	0x22   // ALC1; R/W

#define     CJC8910_R18_ALC2_CONTROL                    0x24   // ALC2; R/W

#define     CJC8910_R19_ALC3_CONTROL                    0x26   // ALC3; R/W
#define     CJC8910_R20_NOISE_GATE_CONTROL              0x28  // NOISE_GATE_CONTROL; R/W
#define     CJC8910_R21_LEFT_ADC_VOLUME                 0x2B   // Left ADC digital volume; R/W


#define     CJC8910_R23_ADDITIONAL_CONTROL1             0x2E   // Additional control; R/W
#define     CJC8910_R24_ADDITIONAL_CONTROL2             0x30   // Additional control; R/W
#define     CJC8910_R27_ADDITIONAL_CONTROL3             0x36   // Additional control; R/W


#define     CJC8910_R32_ADCL_SIGNAL_PATH                0x41   // ADC signal path; R/W
#define     CJC8910_R33_MIC                             0x42   // MIC

#define     CJC8910_R34_AUX                             0x44   //AUX; R/W

#define     CJC8910_R35_LEFT_OUT_MIX2_L                 0x46   // Left out Mix (2); R/W
#define     CJC8910_R35_LEFT_OUT_MIX2_H                 0X47   //

#define     CJC8910_R37_ADC_PDN                         0x4A   // Adc_pdn se; R/W

#define     CJC8910_R67_LOW_POWER_PLAYBACK              0x86   // Low power playback; R/W

#define     CJC8910_R25_PWR_MGMT1_L                     0x32   // Power management1 and VMIDSEL; R/W
#define     CJC8910_R25_PWR_MGMT1_H                     0x33   // Power management1 and VMIDSEL; R/W
#define     CJC8910_R26_PWR_MGMT2_L                     0x34   // Power management2 and DAC left Power down; R/W
#define     CJC8910_R26_PWR_MGMT2_H                     0x35   // Power management2 and DAC right Power up; R/W
