/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This aic int file, realize the system init function.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x13e119
#define LOG_MODULE    AMOD_INIT

#include <aic_auto_init_private.h>
#include <aic_cmdline.h>
#include <adbg_cfg_api.h>
#include <aipc_init.h>
#include <alog_init.h>
#include <ambox_init.h>
#include <cache.h>
#include <dma.h>
#include <ftlblock.h>
#include <irq.h>
#include <mpu.h>
#include <timer_inc.h>
#include <slp_inc.h>
#include <spinlock.h>
#include <aic_boot_mode.h>
#include <aic_version.h>
#include <serial_inc.h>
#include <aic_at_process_common.h>

#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#endif

#if defined (CONFIG_RTOS_THREADX)
#include <tx_api.h>
#elif defined(CONFIG_RTOS_LITEOS_M)
#include "los_task.h"
#endif

#ifdef CONFIG_AIC_OSA
#include <aic_osa.h>
#include <aos_config.h>
#endif

#ifdef CONFIG_MCX_SUPPORT
#include <aic_mcx.h>
#endif

#ifdef CONFIG_NVRAM_SUPPORT
#ifdef CONFIG_SUBSYS_AP
#include <aic_nvram_ap.h>
#else
#include <aic_nvram_api.h>
#endif
#endif

#ifdef CONFIG_DRIVER_UART_AIC
#include <serial.h>
#endif

#ifdef CONFIG_MISC_SUPPORT
#include "aic_md_api.h"
#endif

#ifdef CONFIG_FS
#include "vfs_init.h"
#endif

#if defined ( __CC_ARM ) || (__ARMCC_VERSION)
int aic_rt_no_semihost_init(void);
int aic_rt_no_heap_init(void);
#endif

#if defined(CONFIG_SLP_AT)&&defined(CONFIG_SUBSYS_AP)
extern int slp_at_cmd_init(void);
AIC_COMP_L2_INIT_EXPORT(slp_at_cmd_init);
#endif

#if defined(CONFIG_CMUX)
extern int cmux_conn_init(void);
extern int aic_cmux_init(void);
AIC_COMP_P1_INIT_EXPORT(aic_cmux_init);
AIC_COMP_P2_INIT_EXPORT(cmux_conn_init);
#endif

#if defined(CONFIG_WIFI_SCAN)

extern s32 aic_wifi_scan_cp_at_handles_init(void);
AIC_NORMAL_COMP_L2_INIT_EXPORT(aic_wifi_scan_cp_at_handles_init);

#if defined(CONFIG_CUSTOMER_CM_AT) || defined (CONFIG_GX_AT_V2)
#ifdef CONFIG_SDK_ENABLE
/* The supplied SDK Wi-Fi archive exports the legacy AT initializer only. */
extern int aic_wifi_scan_at_init(void);
AIC_NORMAL_COMP_L2_INIT_EXPORT(aic_wifi_scan_at_init);
#else
extern int wifi_scan_at_cm_init(void);
AIC_NORMAL_COMP_L2_INIT_EXPORT(wifi_scan_at_cm_init);
#endif
#elif defined(CONFIG_GX_AT_V1)
extern int aic_wifi_scan_at_init(void);
AIC_NORMAL_COMP_L2_INIT_EXPORT(aic_wifi_scan_at_init);
#endif

#endif /* CONFIG_WIFI_SCAN */

#if defined(CONFIG_D2D_L3_DUMMY)
extern s32 aic_fd2d_l3_init();
AIC_APP_P2_INIT_EXPORT(aic_fd2d_l3_init);
#endif

#ifdef CONFIG_PLAT_AUTO_INIT
AIC_ADD_INIT_LEVEL_END(0);
AIC_ADD_INIT_LEVEL_END(1);
AIC_ADD_INIT_LEVEL_END(2);
AIC_ADD_INIT_LEVEL_END(3);
AIC_ADD_INIT_LEVEL_END(4);
AIC_ADD_INIT_LEVEL_END(5);
AIC_ADD_INIT_LEVEL_END(6);
AIC_ADD_INIT_LEVEL_END(7);
AIC_ADD_INIT_LEVEL_END(8);
AIC_ADD_INIT_LEVEL_END(9);
AIC_ADD_INIT_LEVEL_END(a);
AIC_ADD_INIT_LEVEL_END(b);
AIC_ADD_INIT_LEVEL_END(c);
AIC_ADD_INIT_LEVEL_END(d);
AIC_ADD_INIT_LEVEL_END(e);

enum {
    state_init = 0,
    state_created,
    state_running,
    state_completed,
    state_deleted
};

/*
 * The init task priority must low than the apcp task priority(1).
 * Otherwise, if some init function read flash may wait the md mutex
 * And enhance the owner of md mutex task's priority to 1 and polling
 * the CP MCX ack event, than will cause the apcp task can't be excututed
 * to set the CP MCX ack. Finally,cause the MCX timeout assert.
 */
#define INIT_TASK_PRI   4

static aos_thread_id    s_init_thread;
static u8               s_init_thread_state;
init_func               g_init_func;

#ifndef CONFIG_SYS_HIGH_PERF
void aic_levle_init(char *lv, const AUTO_INIT_T *lv_init_end)
{
    const AUTO_INIT_T *init; \

    alog_debug("###LV %s init.\n", lv);

    for (init = lv_init_end + 1; init->func; init++) \
    {
        if (init->boot_dis &  BIT(aic_boot_mode))
            continue;

        if ((aic_boot_mode == BOOT_MODE_BASIC) &&
            sys_is_belong_extend_image(init->func)) {
            aicx_debug(0x0ad3d2, "x", "init err: func 0x%x is in extend img.\n", init->func); 
\
            continue;
        }

        g_init_func = init->func;
        (init->func)();
    } 
}
#endif

static void init_task_func(void *p)
{
    s_init_thread_state = state_running;

    aic_get_wakelock();

    /* driver p1 init. */
    AIC_DRV_L1_INIT();

    /* driver p2 init. */
    AIC_DRV_L2_INIT();

    /* componets p1 init. */
    AIC_COMP_L1_INIT();

    /* componets p2 init. */
    AIC_COMP_L2_INIT();

    /* App p1 init*/
    AIC_APP_L1_INIT();

    /* App p2 init. */
    AIC_APP_L2_INIT();

    aic_put_wakelock();

    s_init_thread_state = state_completed;
}

static void aic_start_init_task(void)
{
    char version[AIC_VERSION_LEN + 1];

    aicx_debug(0x0f9972, "", "aic_start_init_task\n");

    /* print the softwate version info. */
    aic_get_version(version, sizeof(version));
    alog_output(LOG_MODULE, ALOG_LV_INFO, "%s", version);

#ifdef CONFIG_AIC_CTRL_SUPPORT
    aic_ctrl_str_notify(version);
#endif

    s_init_thread = aos_create_thread_simply("Init_task", init_task_func,
                                             NULL, 2048, INIT_TASK_PRI);

    if (s_init_thread != AOS_INVALID_THREAD_ID)
        s_init_thread_state = state_created;
}

void aic_delete_init_task(void)
{
    if (s_init_thread_state == state_completed) {
        aos_delete_thread(s_init_thread);
        s_init_thread_state =state_deleted;
    }
}
#endif /* CONFIG_PLAT_AUTO_INIT */

/**
 * aic_prepare_init() - do some things befor __main,
 * such as cpu clock, flash and pll.
*/
void aic_prepare_init(void)
{

}

/**
 * aic_chip_early_init() - the most base chip init, such as timer, uart.
 * Keep the most base chip initialization entry before all
 * modules are initialized.
*/
static void aic_chip_early_init(void)
{
#ifdef CONFIG_DRIVER_TIMER
    aic_tmr_ealy_init();
#endif

#ifdef CONFIG_DRIVER_SPINLOCK
    spinlock_init();
#endif

    /* parse cmdline. */
    aic_cmdline_parse();

#ifdef CONFIG_MHA
    extern void mha_init(void);
    mha_init();
#endif

#ifdef CONFIG_SUBSYS_AP
    extern void pin_init(void);
    pin_init();
#endif
}

/**
 * aic_chip_init() - the base chip init.
 * such as flash driver. Keep a base chip initialization entry before all
 * driver modules are initialized.
*/
static void aic_chip_init(void)
{

}

/**
 * aic_rtos_early_init() - init the most basic os componets.
 * such as system heap init, .
 */
static void aic_rtos_early_init(void)
{
#ifdef CONFIG_AIC_OSA
    /*
     * init aos heap. block pool and byte pool can be configured
     * in aos_config.c/aos_config.h
     */
    aos_init_pools();

    /* init aos thread description table. */
    aos_init_thread_desc_tbl();
#endif

    aic_boot_mode_init();

    aic_sys_assert_init();

    /*
     * only used for arm compiler, open __no_semihosting
     * and open __no_heap.
     */
#if defined ( __CC_ARM ) || (__ARMCC_VERSION)
    aic_rt_no_semihost_init();
    aic_rt_no_heap_init();
#endif
}

/**
 * aic_driver_early_init() - the driver early init.
 * such as irq,ssp, flash driver. Keep a base chip initialization entry before
 * all driver modules are initialized.
*/
static void aic_driver_early_init(void)
{
#ifdef CONFIG_DRIVER_UART_AIC
    aic_uart_init();
#endif

    irq_init();

    aic_dma_init();

#ifdef CONFIG_DRIVER_GPIO
    extern int aic_gpio_init(void);
    aic_gpio_init();
#endif

#ifdef CONFIG_SSP_DRIVER
    extern void ssp_init(void);
    ssp_init();
#endif

#ifdef CONFIG_FMD_SNOR
    extern void snor_init(void);
    snor_init();
#endif

#ifdef CONFIG_DRIVER_AMBOX
    /* init ambox */
    ambox_init();
#endif
}

/**
 * aic_rtos_init() - init rtos.
 * contains some basic componets of os.
 * such as nv and aos.
*/
static void aic_rtos_init(void)
{
#ifdef CONFIG_FTL
    ftl_init();
#endif

#ifdef CONFIG_MISC_SUPPORT
    aic_md_init();
#endif

#ifdef CONFIG_NVRAM_SUPPORT
#ifdef CONFIG_SUBSYS_CP
    aic_nvram_initialize();
#endif
#endif

    /* load debug_nv cfg from nv. */
    adbg_load_debug_nv_cfg();
#ifdef CONFIG_SUBSYS_AP
    aic_load_net_nv_cfg();
#endif

#ifdef CONFIG_AIPC_SUPPORT
    /* init aipc */
    aipc_init();
#endif

#ifdef CONFIG_ALOG_SUPPORT
    /* init alog */
    alog_init();
#endif

#ifdef CONFIG_NVRAM_SUPPORT
#ifdef CONFIG_SUBSYS_AP
    aic_nvram_ap_initialize();
#endif
#endif

#ifdef CONFIG_MCX_SUPPORT
    mcx_init();
#endif

#ifdef CONFIG_FS
    if (!aic_boot_mode_is_basic() ||
         !sys_is_belong_extend_image((void*)vfs_init)) {
        vfs_init();
    }
#endif

#ifdef CONFIG_AIC_OSA
#ifdef AOS_CONFIG_TRACEX
    /* enable tracex */
    if (adbg_is_tracex_on())
        aos_enable_tracex();
    else
        aos_disable_tracex();
#endif

    /* enable stack checking */
#ifdef AOS_ENABLE_STACK_CHECKING
    aos_enable_stackcheck();
#endif

    /*
     * create aos standard threads, which are described
     * in g_thread_config_tbl in aos_config.c
     */
    aos_init_standard_thread();
#endif /* CONFIG_AIC_OSA */

    /* basic mode don't create sleep task. */
    if(!aic_boot_mode_is_basic())
        slp_create_idle_thread();
}

static void aic_init_all(void)
{
    /*init it early for system eraly log. */
    alog_early_init();

    /* the early chip init. */
    aic_chip_early_init();

    /* the early rtos init. */
    aic_rtos_early_init();

    /* the driver early init. */
    aic_driver_early_init();

#ifdef CONFIG_PLAT_AUTO_INIT
    /* Early init */
    AIC_EARLY_INIT();
#endif

    /* rtos init. */
    aic_rtos_init();

    /* chip init. */
    aic_chip_init();

#ifdef CONFIG_PLAT_AUTO_INIT
    /* driver p1 init. */
    AIC_DRV_P1_INIT();

    /* driver p2 init. */
    AIC_DRV_P2_INIT();

    /* componets p1 init. */
    AIC_COMP_P1_INIT();

    /* componets p2 init. */
    AIC_COMP_P2_INIT();

    /* App p1 init*/
    AIC_APP_P1_INIT();

    /* App p2 init. */
    AIC_APP_P2_INIT();

    /* modem init. */
    AIC_MODEM_INIT();

    /* start init task. */
    aic_start_init_task();
#endif
}

#ifdef CONFIG_RTOS_LITEOS_M
extern u32 aos_init_mem(void);

static void aic_init_los(void)
{
    aos_init_mem();

    if (LOS_KernelInit() == LOS_OK) {
        aic_init_all();
        LOS_Start();
        /* TODO: error handling. */
    } else {
        /* TODO: error handling. */
    }
}
#endif

/**
 * main() - The main entry point.
 */
void main(void)
{
    /* make sure vectors located at 0 seen correctly */
    dcache_clean_all();
    icache_inv_all();

    mpu_config_after_load_image();

#ifdef CONFIG_RTOS_THREADX
    /* Enter the ThreadX kernel. */
    tx_kernel_enter();
#elif defined(CONFIG_RTOS_LITEOS_M)
    aic_init_los();
#else
    #error Unknow RTOS kernel!
#endif
}

#ifdef CONFIG_RTOS_THREADX
/**
 * tx_application_define() -
 * the application initialization function, will be called in.
 * _tx_initialize_kernel_enter.
 */
void tx_application_define(void *first_unused_memory)
{
    /* memset it to 0, in assert function will used it to save aon iram. */
    *(u32 *)first_unused_memory = 0;

    aic_init_all();
}
#endif

void aic_init_charge_dis(void)
{
    AIC_EARLY_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_DRV_P1_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_DRV_P2_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_COMP_P1_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_COMP_P2_INIT_DIS(BOOT_MODE_CHARGE);

    AIC_DRV_L1_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_DRV_L2_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_COMP_L1_INIT_DIS(BOOT_MODE_CHARGE);
    AIC_COMP_L2_INIT_DIS(BOOT_MODE_CHARGE);
}
