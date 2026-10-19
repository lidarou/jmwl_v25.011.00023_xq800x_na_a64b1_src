/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *   This file is the head software driver of sleep module.
 *   It includes all the eipu common function api.
 *
 */

#ifndef __SYSOFF_INC_H__
#define __SYSOFF_INC_H__

#include <aic_type.h>
#include <aic_osa.h>
#include <aic_list.h>
#include <aic_bitmap.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

typedef enum {
    SYSOFF_MODE_SHUTDOWN    = BIT_0,
    SYSOFF_MODE_WARMRESET   = BIT_1,
    SYSOFF_MODE_COLDRESET   = BIT_2,
    SYSOFF_MODE_BASIC       = BIT_3,
    SYSOFF_MODE_APWDG       = BIT_4,
    SYSOFF_MODE_CPWDG       = BIT_5,
    SYSOFF_MODE_ADSPWDG     = BIT_6,
    SYSOFF_MODE_APCRASH     = BIT_7,
    SYSOFF_MODE_CPCRASH     = BIT_8,
    SYSOFF_MODE_ADSPCRASH   = BIT_9,
    SYSOFF_MODE_AUTODL      = BIT_10,
    SYSOFF_MODE_CHG_ALARM   = BIT_11,
    SYSOFF_MODE_GXEM        = BIT_12,
    SYSOFF_MODE_DPSHUTDW    = BIT_13,
    SYSOFF_MODE_MAX         =0xFFFFFFFF
} SYSOFF_MODE_MSG;

typedef void (*SYSOFF_HOOK_FUNC)(void *ptr);
typedef struct _sysoff_hook_t {
    struct list_head    link;               // list link
    SYSOFF_HOOK_FUNC       sysoff_func;       // sysoff callback
    void*               sysoff_param_ptr;  // sysoff param
} SYSOFF_HOOK_T;


#ifdef CONFIG_AIC_SYSOFF
void sysoff_reset_basic(void);
void sysoff_stop_cp(void);
void sysoff_shutdown();
void sysoff_emergency_shutdown();
void sysoff_reset();
void sysoff_sysdump();
int sysoff_set_dumpmode_flag(boolean is_sysdump);
int sysoff_set_dumpmode(SYSOFF_MODE_MSG sysoff_mode);
void sysoff_autodl();
void sysoff_emergency_reset();
void sysoff_chgalrm_reset(void);
int sysoff_unregister_hook(u32 handle);
u32 sysoff_register_hook(SYSOFF_HOOK_FUNC   sysoff_func,
                            void*           sysoff_param_ptr);
void sysoff_reset_basic(void);
void sysoff_stop_cp(void);
void sysoff_gxem_reset(void);
void sysoff_deepshutdw();
#else
inline void sysoff_basic(){}
inline void sysoff_shutdown(){}
inline void sysoff_emergency_shutdown(){}
inline void sysoff_reset(){}
inline void sysoff_sysdump(){}
inline int sysoff_set_dumpmode(SYSOFF_MODE_MSG sysoff_mode){return AOS_FALSE;}
inline void sysoff_autodl(){}
inline void sysoff_emergency_reset(){}
inline void sysoff_chgalrm_reset(void){}
inline int sysoff_unregister_hook(u32 handle){return AOS_FALSE;}
inline u32 sysoff_register_hook(SYSOFF_HOOK_FUNC   sysoff_func,
                            void*           sysoff_param_ptr){return 0;}
inline void sysoff_reset_basic(void){}
inline void sysoff_stop_cp(void){}
inline void sysoff_gxem_reset(void){}
inline void sysoff_deepshutdw(){}

#endif

#endif /* __SYSOFF_INC_H__ */

