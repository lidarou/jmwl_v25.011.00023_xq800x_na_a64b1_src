
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1bb289
#define LOG_MODULE    AMOD_AUDIO

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <alog_api.h>

#include "sincrsm_adpter.h"

#ifdef AICOS_LOGENABLE

extern void alog_output(uint8_t module, uint8_t level, const char *fmt, ...);
static char log_buf[256];
void resampler_log(uint8_t level, const char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vsnprintf(log_buf, 256, fmt, args);
    alog_output(LOG_MODULE, level, "%s", log_buf);
    va_end(args);
}

#endif

