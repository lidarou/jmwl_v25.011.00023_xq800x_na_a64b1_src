#ifndef __SINCRSM_ADPTER_H__
#define __SINCRSM_ADPTER_H__

#include <stdint.h>
#include <sys/types.h>

#define AICOS_LOGENABLE


#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#define AIC_RESAMPLER_ENTER  9000
#define AIC_RESAMPLER_EXIT  9020


#ifdef __cplusplus
extern "C" {
#endif

extern void aic_sys_assert(char *fmt, ...);

#define RSM_AIC_ASSERT(n) do { \
    if(!(n)) \
       aic_sys_assert((char*)"AP ASSERT:", __FILE__, __LINE__); \
    } while(0)

#define RSM_USER_TRACE_EVENT_INSERT(id,a,b)  _tx_trace_user_event_insert(id,a,b,(unsigned long)0, (unsigned long)0)

extern unsigned int  _tx_trace_user_event_insert(unsigned long event_id, unsigned long info_field_1, unsigned long info_field_2, unsigned long info_field_3, unsigned long info_field_4);

void resampler_log(uint8_t level, const char* fmt, ...);

#ifdef __cplusplus
}
#endif




#endif
