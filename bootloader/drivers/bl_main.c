/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#include <cache.h>
#include "console/console.h"
#include "initcall.h"
#include <mpu.h>
#include <tzc.h>
#include <clk/clk.h>
#include "platform/platform.h"

int call_initcalls(void)
{
    const struct initcall_t *call;
    int ret = 0;

    for (call = initcall_begin; call < initcall_end; call++) {
        ret = call->func();
        if (ret) {
            log_print("%s() init fail.\n", call->name);
            return ret;
        }
        log_print("%s() init ok!\n", call->name);
    }

    return ret;
}

void cpu_core_init(void)
{
    mpu_config();
    platform_init();
}

int main(void)
{
    int ret;
    enum CLK_32K_SRC_SEL clk_32k_src_sel;

    clk_init();
    tzc_init();
    cpu_core_init();
    console_init();
    ret = call_initcalls();
    if (ret)
        return ret;

#if defined(CLK_TCXO_EXTERNAL_32K)
    clk_32k_src_sel = TCXO_EXTERNAL_32K;
#elif defined(CLK_TCXO_32K)
    clk_32k_src_sel = TCXO_32K;
#elif defined(CLK_DCXO_EXTERNAL_32K)
    clk_32k_src_sel = DCXO_EXTERNAL_32K;
#elif defined(CLK_DCXO_32K)
    clk_32k_src_sel = DCXO_32K;
#elif defined(CLK_DCXO_32KLESS)
    clk_32k_src_sel = DCXO_32KLESS;
#elif defined(CLK_TCXO_RC32K)
    clk_32k_src_sel = TCXO_RC32K;
#else
    clk_32k_src_sel = DCXO_RC32K;
#endif

    pmic_clk_init(clk_32k_src_sel);

    log_print("aic bootloader boot enter (%s)...\n", platform.name);
    ret = platform_do_boot();
    log_print("%s init fail... (ret = %d) enter while(1).\n", platform.name, ret);
    while(1);

    return 0;
}
