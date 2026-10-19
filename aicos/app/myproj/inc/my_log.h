#ifndef __MY_LOG_H__
#define __MY_LOG_H__

#define MY_LOG_ENABLE           (1)
#define MY_LOG_MAX_LEN          256

#if MY_LOG_ENABLE
#define my_log_printf my_log_printf_internal
#else
#define my_log_printf(level, fmt, args...) { }
#endif

void my_log_init(void);
void my_log_printf_internal(int log_level, char *fmt, ...);

#endif

