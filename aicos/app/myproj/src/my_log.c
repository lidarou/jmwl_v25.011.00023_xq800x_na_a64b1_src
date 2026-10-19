#include "my_common.h"


void my_log_init(void)
{
    /* connmgr_cfg.h routes SDK logs to USB AIC_COM0. */
    alog_set_log_level_no_save(AMOD_APP, ALOG_LV_INFO);
    my_log_printf(1, "[MYPROJ] log output: USB LOG, module=APP (use LT)");
}


/*********************************************************************
**函数名称:  my_log_printf
**入口参数:  MOD_TYPE  --  模块的ID编号
**             **        --  可变参数列表
**出口参数:
**函数功能:  打印模块的相关trace信息
*********************************************************************/
void my_log_printf_internal(int log_level, char *fmt, ...)
{
    char log_buffer[MY_LOG_MAX_LEN];
    va_list ap;

    /* Existing callers use project-specific levels; retain all as APP INFO. */
    (void)log_level;
    va_start(ap, fmt);
    vsnprintf(log_buffer, sizeof(log_buffer), fmt, ap);
    va_end(ap);

    /* alog copies the text into its managed buffers for USB transmission. */
    alog_output(AMOD_APP, ALOG_LV_INFO, "%s", log_buffer);
}

