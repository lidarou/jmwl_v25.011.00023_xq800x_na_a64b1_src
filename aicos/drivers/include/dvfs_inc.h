/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file is the head software driver of dvfs module.
 *   It includes all the dvfs common function api.
 *
 */

#ifndef __DVFS_INC_H__
#define __DVFS_INC_H__

#include <aic_type.h>
#include <aic_bitmap.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

typedef enum _dvfs_type_e {
    RISE                    = BIT(0),
    FALL                    = BIT(1),

    AP_APP_BOOST            = BIT(6),
    AP_CMSTAR_BOOT          = BIT(7),
    CP_CR5_BOOT             = BIT(8),
    CP_IRAM                 = BIT(9),
    CP_NORFLASH             = BIT(10),
    CP_PSRAM                = BIT(11),
    AP_IRAM                 = BIT(12),
    AP_NORFLASH             = BIT(13),
    AP_PSRAM                = BIT(14),
    AUDIO_DMA               = BIT(15),
    AUDIO_CP_AHB_S1         = BIT(16),
    AUDIO_TL420             = BIT(17),
    CP_DMA_CIPHER           = BIT(18),
    CP_AHB_S1               = BIT(19),
    CP_MATRIX               = BIT(20),
    CP_CR5                  = BIT(21),
    SYSCPU_LZMA             = BIT(22),
    AP_PPP                  = BIT(23),
    AP_CSI                  = BIT(24),
    AP_LCDC                 = BIT(25),
    AP_SDIO                 = BIT(26),
    AP_USB                  = BIT(27),
    AP_EIP130               = BIT(28),
    AP_DMAC                 = BIT(29),
    AP_MCU_AHB              = BIT(30),
    AP_CMSTAR               = BIT(31),
} DVFS_TYPE_E;

typedef enum _dvfs_testpint_e {
    TP_SWITCH               = BIT(2),
    TP_SWITCH_FREQ          = BIT(3),
    TP_SWITCH_FREQ_HIGH     = BIT(4),
    TP_SWITCH_FREQ_LOW      = BIT(5),
    TP_SWITCH_VOL           = BIT(6),
    TP_SWITCH_VOL_HIGH      = BIT(7),
    TP_SWITCH_VOL_LOW       = BIT(8),
} DVFS_TP_E;

#ifdef CONFIG_SLEEP_R1P0
#define AP_CMSTAR_VOTER (AP_CMSTAR_BOOT \
                       | AP_APP_BOOST   \
                       | AP_CMSTAR      \
                       | AUDIO_DMA      \
                       | AP_DMAC)
#else
#define AP_CMSTAR_VOTER (AP_CMSTAR_BOOT \
                       | AP_APP_BOOST   \
                       | AP_CMSTAR      \
                       | SYSCPU_LZMA    \
                       | AUDIO_DMA      \
                       | AP_DMAC)
#endif

#define CP_CR5_VOTER (CP_CR5       \
                    | CP_CR5_BOOT)

#define AP_MODULE (AP_CMSTAR_BOOT \
                 | AP_PPP       \
                 | AP_CSI       \
                 | AP_LCDC      \
                 | AP_SDIO      \
                 | AP_USB       \
                 | AP_EIP130    \
                 | AP_DMAC      \
                 | AP_MCU_AHB   \
                 | AP_CMSTAR    \
                 | AUDIO_TL420  \
                 | AP_APP_BOOST \
                 | SYSCPU_LZMA  \
                 | AUDIO_DMA)

#define CP_MODULE CP_CR5_VOTER

#define TOP_MODULE (CP_IRAM     \
                 | CP_NORFLASH  \
                 | CP_PSRAM     \
                 | AP_IRAM      \
                 | AP_NORFLASH  \
                 | AP_PSRAM)

#ifdef CONFIG_DVFS
void dvfs_init(void);

/* Note: when call dvfs switch:
 * - example:
 *      aic_dvfs_switch(RISE | AP_USB);
 * - param:
 *      RISE:this means rising freq(!!!***param must take RISE or FALL***!!!).
 *      AP_USB:this means which module needs dfs.
 * - note:
 *      (1)psram/iram/flash will rise freq after rise voltage to 0.9v,and fall freq before fall voltage to 0.8v.
 *      (2)lcdc/csi/usb will just vote,but will not switch freq.
 *      (3)mstar/cr5 can vote and switch freq.
 *      (4)this func can only be called in aos_is_user_thread_context();
 */
#ifdef CONFIG_SLEEP_R1P0
void aic_dvfs_switch(DVFS_TYPE_E event);

/* Note: when call dvfs force:
 * - example:
 *      aic_dvfs_force(RISE | AP_MSTAR);
 * - param:
 *      RISE:this means rising freq(!!!***param must take RISE or FALL***!!!).
 *      AP_MSTAR:this means which module needs dfs.
 * - note:
 *      now,only sleep/resume need this func.

 */
 #endif
void aic_dvfs_force(DVFS_TYPE_E event);

int aic_dvfs_get_vol(u32 *voltage_uV);

int aic_dvfs_get_freq(u32 *array_ptr);
#ifdef CONFIG_SUBSYS_AP
void aic_dvfs_debug(void);
void aic_dvfs_boost(bool value);
#else
void aic_dvfs_cr5_switch(u32 value);
#endif
#else
void inline dvfs_init(void) {
    return;
}
void inline aic_dvfs_switch(DVFS_TYPE_E event) {
    return;
}
void inline aic_dvfs_force(DVFS_TYPE_E event) {
    return;
}
int inline aic_dvfs_get_vol(u32 *voltage_uV) {
    return AIC_EPERM;
}
int inline aic_dvfs_get_freq(u32 *array_ptr) {
    return AIC_EPERM;
}
inline void aic_dvfs_debug(void) {
    return;
}
inline void aic_dvfs_boost(bool value) {
    return;
}

inline void aic_dvfs_cr5_switch(u32 value) {
    return;
}

#endif

#endif /* __DVFS_INC_H__ */

