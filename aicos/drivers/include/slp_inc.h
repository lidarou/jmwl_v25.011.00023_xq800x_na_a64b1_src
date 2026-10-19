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

#ifndef __SLP_INC_H__
#define __SLP_INC_H__

#include <aic_type.h>
#include <aic_list.h>
#include <aic_bitmap.h>
#ifdef CONFIG_RTOS_THREADX
#include <tx_api.h>
#endif

#define SLP_PRIORITY        255

#ifdef CONFIG_SUBSYS_AP
#ifdef CONFIG_SLEEP_R1P0
#define SLP_STACK_SIZE      400
#else
#define SLP_STACK_SIZE      240
#endif
#else
#define SLP_STACK_SIZE      160
#endif

#define WAKE_LOCK_INIT      BIT(0)          // wake lock init
#define WAKE_LOCK_ACTIVE    BIT(1)          // wake lock active

#define STEP_SIZE           10
#define WK_SIZE             4
#define CLR_SIZE            60
#define BOOT_SIZE           3

#define REG_APLL_VCO    0x411A0824
#define REG_UPLL_VCO    0x411A8824
#define REG_GPLL_VCO    0x411A4824
#define FREERUN_CNT_L   (REG_BASE_AON_SYSTMR + 0x0)
#define SYSTMR_CNT_L    (REG_BASE_AON_SYSTMR + 0x8)

#define SLP_FREQ_ARRAY_SIZE  5

#if 1//def CONFIG_SLEEP_R1P0
#define ONLY_RESUME_TOP
#endif
//#define DEBUG_PMIC
//#define DEBUG_WAKELOCK
//#define DEBUG_TOP
//#define LPC_BYPASS
//#define DEBUG_CP_STOP
#define RET_FLASH
//#define DEBUG_MEM_PWO
//#define DEBUG_TOPDEEP
#if 1//def CONFIG_SLEEP_R1P0
#define DEBUG_BOOT_TIMING_AP
#else
#define DEBUG_BOOT_TIMING_AP
#endif
#define DEBUG_BOOT_TIMING_CP
//#define AP_NV_CONTROL_DEEP
//#define DEBUG_CLR_INT
#define DEBUG_PLL_LOCK
//#define DEBUG_SUS_RES
//#define DEBUG_WAKWLOCK_RUNEB
#define DEBUG_JUMP_ZERO
//#define DEBUG_IRQ
//#define DEBUG_VOLT
#define DEBUG_DPLL

extern uint32_t s_slp_is_printf;

/* switch deep define. */
#define AIC_AP_DEEP     BIT(0)
#define AIC_CP_DEEP     BIT(1)
#define AIC_DEEP_ALL    (AIC_AP_DEEP|AIC_CP_DEEP)

#define PLL_LOCK_DONE   BIT(31)
#define AIC_CP_CAN_DEEP BIT(30)
#define AIC_CP_CAN_IDLE BIT(29)

#define APLL_RESET_LOCK SLOCK_2
#define APLL_RETRY_CNT  6

typedef enum _trace_step_e {
    CHECK_WAKELOCK     = 0x0,
    CHECK_EB           = 0x1,
    CHECK_CP           = 0x2,
    ENTER_SUSPENDING   = 0x3,
    ENTER_DEEP         = 0x4,
    EXIT_EARLY_RESUME  = 0x5,
    ENTER_RESUMEING    = 0x6,
    ENABLE_INT         = 0x7,
    MCU_DEEP_FAIL      = 0x8,
    MCU_DEEP_COUNT     = 0x9,
    CR5_DEEP_COUNT     = 0xa,
    DVFS_VAL           = 0xb,
    MCU_DEEP_FAST      = 0xc,
    CHECK_CP_STAT      = 0xd,
    CHECK_AON_TIMER    = 0xe,
    EXIT_RESUME        = 0xf,
    ENTER_LPM          = 0x10,
    TOP_PWRON          = 0x11,
} TRACE_STEP_E;

typedef struct _task_wake_t {
    struct list_head    link;               // list link
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD           *thread_ptr;        // thread ptr
#else
    u32                 thread_id;
#endif
} TASK_WAKE_T;

typedef void (*SLP_HOOK_FUNC)(void *ptr);
typedef void (*SLP_HOOK_FUNC_EX)(void *ptr,int sleep);

typedef struct _slp_hook_t {
    struct list_head    link;               // list link
    SLP_HOOK_FUNC       suspend_func;       // suspend callback
    void*               suspend_param_ptr;  // suspend param
    SLP_HOOK_FUNC       resume_func;        // resume callback
    void*               resume_param_ptr;   // resume param
} SLP_HOOK_T;

typedef struct _slp_hook_ex_t {
    struct list_head    link;               // list link
    SLP_HOOK_FUNC_EX    suspend_func_ex;    // suspend callback
    void*               suspend_param_ptr;  // suspend param
    SLP_HOOK_FUNC_EX    resume_func_ex;     // resume callback
    void*               resume_param_ptr;   // resume param
} SLP_HOOK_EX_T;
typedef struct _slp_thread_t {
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD           thread;
#else
    u32                 los_task;
#endif
}SLP_THREAD_T;

typedef struct _slp_data_t {
    struct list_head    hook_info;
    u32                 iram_stack[SLP_STACK_SIZE];
    u8                  pll_lock_fail;
    u8                  pll_lock_success;
    u8                  pll_lock_count;
    u8                  can_deep;
    u32                 pll_lock;
    u32                 int_disable;
    u32                 sys_state;
    u32                 slp_cutdown;
    u32                 slp_offset;
    bool                has_enter_deep;
    bool                is_suspended;
    u8                  sleepind;
    u32                 delaysleep;
    u32                 serial_port_id;
    u32                 rst_apll_start;
    u32                 rst_apll_end;
    u32                 rst_get_spinlock;
    u32                 rst_put_spinlock;
    u32                 rst_step_start;
    u32                 rst_step_pwr_off;
    u32                 rst_step_pwr_on;
    u32                 rst_step_cnt;
    u32                 vco_val;
    u32                 top_pwr_on;
} SLP_DATA_T;

typedef enum _wk_step_e {
    WK_NULL,
    WK_GET,
    WK_PUT,
    WK_MISS_PUT,
} WK_STEP_E;

typedef struct _save_wakelock_t {
    WK_STEP_E   wk_step;
#ifdef CONFIG_RTOS_THREADX
    TX_THREAD   *thread_ptr;
#endif
    uint32_t    count;
    uint32_t    lrReg;
    uint64_t    tick;
} SAVE_WAKELOCK_T;

typedef struct _wk_info_t {
    SAVE_WAKELOCK_T wk_buf[WK_SIZE];
    uint32_t save_idx;
} WK_INFO_T;

typedef struct _save_clr_t {
    uint32_t  int_num;
    uint64_t  tick;
} SAVE_CLR_T;

typedef struct _clr_int_t {
    SAVE_CLR_T int_buf[CLR_SIZE];
    SAVE_CLR_T clr_buf[CLR_SIZE];
    uint32_t save_idx;
} CLR_INT_T;

typedef struct _reg_info_t {
    uint32_t addr;
    bool     is_closed;
} REG_INFO_T;

typedef struct _slp_time_info_t {
    uint32_t deep_time;
    uint32_t gate_time;
    uint32_t save_time;
    uint32_t cur_time;
} SLP_TIME_INFO_T;

typedef struct _slp_cp_boot_info_t {
    uint32_t cp_boot0;
    uint32_t cp_wait_ap_deep;
    uint32_t cp_get_ap_wake;
    uint32_t cp_wait_psram;
    uint32_t cp_get_psram;
    uint32_t cp_sim;
    uint32_t cp_latch;
    uint32_t cp_get_flash;
    uint32_t cp_late_suspend;
    uint32_t cp_wfi;
} SLP_CP_BOOT_INFO_T;

typedef struct _slp_psram_acc_info_t {
    uint32_t mcu_boot0;
    uint32_t psram_acc;
} SLP_PSRAM_ACC_INFO_T;

typedef struct _slp_ap_boot_info_t {
    SLP_PSRAM_ACC_INFO_T mcu_boot0[BOOT_SIZE];
    uint32_t mcu_pll_lock;
    uint32_t mcu_update_clk;
    uint32_t mcu_cache_enable;
    uint32_t mcu_psram_start;
    uint32_t mcu_psram_end;
    uint32_t mcu_flash;
    uint32_t mcu_enter_deep;
    uint32_t mcu_boot_idx;
} SLP_AP_BOOT_INFO_T;

#define PWR_SIZE 4

typedef enum _top_pwr_num_e
{
    SET_BIT0,
    EXIT_PD_CLR_BIT0,
    CLR_BIT0,
    START_POLL_PSRAM_LP,
    END_POLL_PSRAM_LP,
}TOP_PWR_NUM_E;

typedef struct _save_top_t {
    TOP_PWR_NUM_E  step;
    uint32_t  tick;
} SAVE_TOP_T;

typedef struct _top_pwr_info_t {
    SAVE_TOP_T pwr_buf[PWR_SIZE];
    uint32_t save_idx;
} TOP_PWR_INFO_T;

typedef struct _cp_slp_t {
#ifdef DEBUG_BOOT_TIMING_CP
    SLP_CP_BOOT_INFO_T cp_boot;
#endif
    u64  tick;
#ifdef DEBUG_PLL_LOCK
    u32  apll_val;
    u32  apll_tick;
#endif
    bool is_perf;
} CP_SLP_T;

typedef enum _level_e {
    LV_NULL,
    LV_APP,
    LV_LOW,
    LV_MID,
    LV_HIGH,
} LEVEL_E;

#ifdef CONFIG_DEEP_SLEEP
void slp_create_idle_thread(void);
int slp_unregister_sleep_hook(u32 handle);
int slp_register_sleep_hook(SLP_HOOK_FUNC   suspend_func,
                            void*           suspend_param_ptr,
                            SLP_HOOK_FUNC   resume_func,
                            void*           resume_param_ptr);
int slp_register_sleep_hookex(SLP_HOOK_FUNC_EX   suspend_func,
                            void*           suspend_param_ptr,
                            SLP_HOOK_FUNC_EX   resume_func,
                            void*           resume_param_ptr);
int slp_register_sleep_hook_ex(SLP_HOOK_FUNC    suspend_func,
                            void*           suspend_param_ptr,
                            SLP_HOOK_FUNC   resume_func,
                            void*           resume_param_ptr,
                            LEVEL_E         level);
int aic_get_wakelock(void);
int aic_put_wakelock(void);
bool aic_slp_is_suspended(void);
extern BOOL slp_has_wake_lock(void);
bool aic_check_deepsleep(void);
u64 slp_get_deepcnt(void);
#else
static inline void slp_create_idle_thread(void){}
static inline int slp_unregister_sleep_hook(u32 handle){return 0;}
static inline int slp_register_sleep_hook(SLP_HOOK_FUNC   suspend_func,
                            void*           suspend_param_ptr,
                            SLP_HOOK_FUNC   resume_func,
                            void*           resume_param_ptr){return 0;}
static inline int slp_register_sleep_hook_ex(SLP_HOOK_FUNC   suspend_func,
                            void*           suspend_param_ptr,
                            SLP_HOOK_FUNC   resume_func,
                            void*           resume_param_ptr,
                            LEVEL_E         level){return 0;}
static inline int aic_get_wakelock(void){return 0;}
static inline int aic_put_wakelock(void){return 0;}
static inline bool aic_slp_is_suspended(void){return FALSE;}
static inline u64 slp_get_deepcnt(void){return 0;}

/*static BOOL slp_has_wake_lock(void) {return TRUE;}
static bool aic_check_deepsleep(void) {return FALSE;}*/
#endif


#endif /* __SLP_INC_H__ */

