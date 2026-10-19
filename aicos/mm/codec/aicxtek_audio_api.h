#ifndef AICXTEK_AUDIO_API_H
#define AICXTEK_AUDIO_API_H

typedef void* (memory_malloc)(void *mem_state, unsigned int size8);
typedef void (memory_free)(void *mem_state, void *mem_ptr);

typedef void (aic_liblog)(int log_level, const char* fmt, ...);

enum log_level {
    AICXTEK_LOG_LEVEL_DEBUG,
    AICXTEK_LOG_LEVEL_INFO,
    AICXTEK_LOG_LEVEL_WARING,
    AICXTEK_LOG_LEVEL_ERROR,
};

typedef struct memory_api_s
{
    void          *heap_state;
    memory_malloc *pf_heap_malloc;
    memory_free   *pf_heap_free;
    void          *scratch_state;
    memory_malloc *pf_scratch_malloc;
    memory_free   *pf_scratch_free;
} memory_api_t;


#define CMD_SET(item)      ((item) & ~0x0100)
#define CMD_GET(item)      ((item) |  0x0100)
#define CMD_ITEM(cmd)      ((cmd)  &  0x00ff)
#define CMD_FOR_SET(item)  (((item)&0x0100) > 0 ? 0 : 1)

typedef void* (audio_create)(memory_api_t* p_memory, void* init_param, unsigned int param_size32);
typedef void   (audio_delete)(void* handle);
typedef int    (audio_process)(void* handle, char* in_buf, unsigned int in_size8, char* out_buf, unsigned int* io_out_size8);
typedef int    (audio_control)(void* handle, int param_code, int* p_param, unsigned int param_size32);

#endif

