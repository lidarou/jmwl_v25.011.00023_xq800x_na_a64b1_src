/*****************************************************************************
 * Copyright (C) 2022 THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the MIT License (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://opensource.org/licenses/MIT
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef __TC_IOT_HAL_H__
#define __TC_IOT_HAL_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "tc_iot_libs_inc.h"

/**********************************************************************
 * Tencent Cloud IoT SDK HAL(Hardware Abstraction Layer)
 * Platform/OS/network/SSL dependant functions
 * Check OS folder for reference implementations
 * Require porting when adapt SDK to new platform/OS
 *********************************************************************/

// opaque thread/task handle
typedef unsigned long ThreadHandle_t;

// uniform thread runner function
typedef void (*ThreadRunFunc)(void *arg);

/**
 * @brief ThreadParams structure 线程创建数据结构
 * @param thread_name       线程名，必要参数，用户需要设置
 * @param thread_id         线程句柄，必要参数
 * @param thread_func       线程运行函数，必要参数，用户需要设置
 * @param user_arg          用户自定义参数，可选参数
 * @param stack_size        线程栈大小，单位字节。RTOS上为必要参数，用户需要设置
 * @param stack_ptr         线程栈入口指针，某些平台如threadX需要该参数
 * @param slice_tick        线程时间片大小，某些平台如RT-Thread需要指定该参数
 * @param priority          线程优先级，建议用ThreadPriorityLevel来描述级别
 *                          再由HAL_ThreadCreate实现里根据具体平台设置具体优先级
 */
typedef struct ThreadParams {
    char *thread_name;
    ThreadHandle_t thread_id;
    ThreadRunFunc thread_func;
    void *user_arg;
    uint32_t stack_size;
    void*    stack_ptr;
    uint32_t slice_tick;
    uint16_t priority;
} ThreadParams;


// the specific thread priority is platform dependent
// and it will be decided inside HAL_ThreadCreate
typedef enum { 
    THREAD_PRIORITY_LOW = -1,
    THREAD_PRIORITY_NORMAL = 10,
    THREAD_PRIORITY_HIGH   = 20,
    THREAD_PRIORITY_HIGHER = 30,
    THREAD_PRIORITY_HIGHEST = 40
} ThreadPriorityLevel;

/**
 * @brief Create a thread/task
 *
 * @param params  ThreadParams 线程创建的参数
 *                注意：该变量需要保证生命周期在线程启动后仍然有效，建议用static变量
 * 
 * @return 0 when success, or error code otherwise
 */
int HAL_ThreadCreate(ThreadParams *params);

/**
 * @brief Destroy a thread/task
 * 
 * @param thread_t thread handle
 *
 * @return QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_ThreadDestroy(ThreadHandle_t *thread_t);

/**
 * @brief Get current thread handle
 *
 * @return a thread handle
 */
unsigned long HAL_GetCurrentThreadHandle(void);

/**
 * @brief Create mutex
 *
 * @return a valid mutex handle when success, or NULL otherwise
 */
void *HAL_MutexCreate(void);

/**
 * @brief Destroy mutex
 *
 * @param mutex     mutex handle
 */
void HAL_MutexDestroy(void *mutex);

/**
 * @brief Lock a mutex in blocking way
 *
 * @param mutex     mutex handle
 */
int HAL_MutexLock(void *mutex);

/**
 * @brief Lock a mutex in non-blocking way
 *
 * @param mutex     mutex handle
 * @return 0 for success, or err code for failure
 */
int HAL_MutexTryLock(void *mutex);

/**
 * @brief Unlock/release mutex
 *
 * @param mutex     mutex handle
 */
int HAL_MutexUnlock(void *mutex);

/**
 * @brief Create a recursive mutex
 *
 * @return a valid recursive mutex handle when success, or NULL otherwise
 */
void *HAL_RecursiveMutexCreate(void);

/**
 * @brief Destroy a recursive mutex
 *
 * @param mutex     mutex handle
 */
void HAL_RecursiveMutexDestroy(void *mutex);

/**
 * @brief Lock a recursive mutex
 *
 * @param mutex     mutex handle
 * @param try_flag  whether to use in non-block way
 */
int HAL_RecursiveMutexLock(void *mutex, int try_flag);

/**
 * @brief Unlock/release a recursive mutex
 *
 * @param mutex     mutex handle
 */
int HAL_RecursiveMutexUnLock(void *mutex);

/**
 * @brief Create condition variable
 *
 * @return a valid condition variable when success, or NULL otherwise
 */
void *HAL_CondCreate(void);

/**
 * @brief Destroy condition variable
 *
 * @param cond     condition variable handle
 */
void HAL_CondFree(void *cond_);

/**
 * @brief Signal/trigger condition variable
 *
 * @param cond     condition variable handle
 */
int HAL_CondSignal(void *cond_, int broadcast);

/**
 * @brief Wait for condition variable trigger
 *
 * @param cond     condition variable handle
 * @param lock     lock variable handle
 * @param timeout_ms timeout value in milliseconds (0 means forever)
 */
int HAL_CondWait(void *cond, void *lock, unsigned long timeout_ms);

/**
 * @brief Malloc memory
 *
 * @param size   Expected memory size (unit: byte)
 * @return       pointer to the memory
 */
void *HAL_Malloc(uint32_t size);

/**
 * @brief Free memory
 *
 * @param ptr   pointer to the pre-malloc memory
 */
void HAL_Free(void *ptr);

/**
 * @brief Print data to console in format
 *
 * @param fmt   print format
 * @param ...   variable number of arguments
 */
void HAL_Printf(const char *fmt, ...);

/**
 * @brief Print data to string in format
 *
 * @param str   destination string
 * @param len   Max size of the output
 * @param fmt   print format
 * @param ...   variable number of arguments
 * @return      number of bytes that print successfull
 */
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

/**
 Print data to string in format
  *
  * @param str   destination string
  * @param len   Max size of the output
  * @param fmt   print format
  * @param ap    arguments list
  * @return      number of bytes that print successfull

 */
int HAL_Vsnprintf(char *str, const int len, const char *fmt, va_list ap);

/**
 * @brief get errno
 *
 * @return laset errno
 */
int HAL_Geterrno();

/**
 * @brief set errno to newval
 *
 * @param new_val: set to errno value
 */
void HAL_Seterrno(int new_val);


/**
 * @brief Sleep for a while
 *
 * @param ms sleep interval in millisecond
 */
void HAL_SleepMs(uint32_t ms);
////////////////////////////////////////////////////////////////////////// 
// utc/local timestamp and timer functions
////////////////////////////////////////////////////////////////////////// 
#define TIME_FORMAT_STR_LEN (24)

/**
 * @brief Get local time in format: %y-%m-%d %H:%M:%S.%ms (2023-01-11 18:01:12.973)
 *
 * @param time_str buffer to store string which size should be >= TIME_FORMAT_STR_LEN
 * @return string of formatted time
 */
char *HAL_GetLocalTime(char *time_str, size_t time_str_len);

/**
 * @brief Get timestamp in second
 *
 * @return   timestamp in second
 */
long HAL_GetTimeSecond(void);

/**
 * @brief Get timestamp in millisecond
 *
 * @return   timestamp in millisecond
 */
uint32_t HAL_GetTimeMs(void);

/**
 * @brief Set system timestamp in millisecond
 *
 * @param timestamp_ms timestamp in millisecond
 * @return   0 for success
 */
int HAL_SetTimeMs(size_t timestamp_ms);

/**
 * @brief Set system timestamp in millisecond
 *
 * @param timestamp_sec timestamp in second
 * @return   0 for success
 */
int HAL_SetTimeSecond(size_t timestamp_sec);

/**
 * Define timer structure, platform dependant
 */
struct Timer {
#if defined(__linux__) && defined(__GLIBC__)
    struct timeval end_time;
#else
    uintptr_t end_time;
#endif
};

typedef struct Timer Timer;

/**
 * @brief Check if timer expires or not
 *
 * @param timer     reference to timer
 * @return          true = expired, false = not expired yet
 */
bool HAL_Timer_expired(Timer *timer);

/**
 * @brief Set the countdown/expired value for the timer
 *
 * @param timer         reference to timer
 * @param timeout_ms    countdown/expired value (unit: millisecond)
 */
void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms);

/**
 * @brief Set the countdown/expired value for the timer
 *
 * @param timer         reference to timer
 * @param timeout       countdown/expired value (unit: second)
 */
void HAL_Timer_countdown(Timer *timer, unsigned int timeout);

/**
 * @brief Check the remain time of the timer
 *
 * @param timer     reference to timer
 * @return          0 if expired, or the left time in millisecond
 */
int HAL_Timer_remain(Timer *timer);

/**
 * @brief Init the timer
 *
 * @param timer reference to timer
 */
void HAL_Timer_init(Timer *timer);

////////////////////////////////////////////////////////////////////////// 
// TLS and TCP network structure and operations
////////////////////////////////////////////////////////////////////////// 

#ifndef MAX_SIZE_OF_CLIENT_ID
#define MAX_SIZE_OF_CLIENT_ID (80)
#endif
/**
 * @brief Define structure for TLS connection parameters
 *
 */
typedef struct {
    const char *ca_crt;
    uint16_t ca_crt_len;

    /**
     * only support device with PSK
     */
    const char *psk;                         // PSK string
    char psk_id[MAX_SIZE_OF_CLIENT_ID + 1];  // PSK ID
    size_t psk_length;  // PSK length
    unsigned int timeout_ms;  // SSL handshake timeout in millisecond

} SSLConnectParams;

typedef SSLConnectParams TLSConnectParams;

/**
 * @brief Setup TLS connection with server
 *
 * @param   pConnectParams reference to TLS connection parameters
 * @host    server address
 * @port    server port
 * @return  TLS connect handle when success, or 0 otherwise
 */
uintptr_t HAL_TLS_Connect(TLSConnectParams *pConnectParams, const char *host, int port);

/**
 * @brief Disconnect with TLS server and release resources
 *
 * @param handle TLS connect handle
 */
void HAL_TLS_Disconnect(uintptr_t handle);

/**
 * @brief Write data via TLS connection
 *
 * @param handle        TLS connect handle
 * @param data          source data to write
 * @param totalLen      length of data
 * @param timeout_ms    timeout value in millisecond
 * @param written_len   length of data written successfully
 * @return              QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_TLS_Write(uintptr_t handle, unsigned char *data, size_t totalLen, uint32_t timeout_ms, size_t *written_len);

/**
 * @brief Read data via TLS connection
 *
 * @param handle        TLS connect handle
 * @param data          destination data buffer where to put data
 * @param totalLen      length of data
 * @param timeout_ms    timeout value in millisecond
 * @param read_len      length of data read successfully
 * @return              QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_TLS_Read(uintptr_t handle, unsigned char *data, size_t totalLen, uint32_t timeout_ms, size_t *read_len);

/**
 * @brief Setup TCP connection with server
 *
 * @host    server address
 * @port    server port
 * @return  TCP socket handle (value>0) when success, or 0 otherwise
 */
uintptr_t HAL_TCP_Connect(const char *host, uint16_t port);

/**
 * @brief Disconnect with server and release resource
 *
 * @param fd TCP Socket handle
 * @return  0 when success
 */
int HAL_TCP_Disconnect(uintptr_t fd);

/**
 * @brief Write data via TCP connection
 *
 * @param fd            TCP socket handle
 * @param data          source data to write
 * @param len           length of data
 * @param timeout_ms    timeout value in millisecond
 * @param written_len   length of data written successfully
 * @return              QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_TCP_Write(uintptr_t fd, const unsigned char *data, uint32_t len, uint32_t timeout_ms, size_t *written_len);

/**
 * @brief Read data via TCP connection
 *
 * @param fd            TCP socket handle
 * @param data          destination data buffer where to put data
 * @param len           length of data
 * @param timeout_ms    timeout value in millisecond
 * @param read_len      length of data read successfully
 * @return              QCLOUD_RET_SUCCESS for success, or err code for failure
 */
int HAL_TCP_Read(uintptr_t fd, unsigned char *data, uint32_t len, uint32_t timeout_ms, size_t *read_len);


////////////////////////////////////////////////////////////////////////// 
// File operations
////////////////////////////////////////////////////////////////////////// 
/**
 * @brief Opens the filename pointed to by filename using the given mode.
 * 
 * @param filename: the name length, and whether relative path or absolute path,
 *                  is platform dependant
 * @param mode: mode is platform dependant, belowing info is from Linux manual
 *            ┌─────────────┬───────────────────────────────┐
              │fopen() mode │ open() flags                  │
              ├─────────────┼───────────────────────────────┤
              │     r       │ O_RDONLY                      │
              ├─────────────┼───────────────────────────────┤
              │     w       │ O_WRONLY | O_CREAT | O_TRUNC  │
              ├─────────────┼───────────────────────────────┤
              │     a       │ O_WRONLY | O_CREAT | O_APPEND │
              ├─────────────┼───────────────────────────────┤
              │     r+      │ O_RDWR                        │
              ├─────────────┼───────────────────────────────┤
              │     w+      │ O_RDWR | O_CREAT | O_TRUNC    │
              ├─────────────┼───────────────────────────────┤
              │     a+      │ O_RDWR | O_CREAT | O_APPEND   │
              └─────────────┴───────────────────────────────┘ 
 * @return  a valid handle(FILE *) or NULL when open failed
 */
void *HAL_FileOpen(const char *filename, const char *mode);

/**
 * @brief Reads data from the given stream into the array pointed to by ptr.
 */
size_t HAL_FileRead(void *ptr, size_t size, size_t nmemb, void *fp);

/**
 * @brief Writes data from the array pointed to by ptr to the given stream.
 */
size_t HAL_FileWrite(const void *ptr, size_t size, size_t nmemb, void *fp);

#define HAL_SEEK_SET 0
#define HAL_SEEK_CUR 1
#define HAL_SEEK_END 2
/**
 * @brief Sets the file position of the stream to the given offset. The argument
 * offset signifies the number of bytes to seek from the given whence position.
 */
int HAL_FileSeek(void *fp, long int offset, int whence);

/**
 * @brief Closes the stream. All buffers are flushed.
 */
int HAL_FileClose(void *fp);

/**
 * @brief Deletes the given filename so that it is no longer accessible.
 */
int HAL_FileRemove(const char *filename);

/**
 * @brief Sets the file position to the beginning of the file of the given stream.
 */
int HAL_FileRewind(void *fp);

/**
 * @brief Causes the filename referred to, by old_filename to be changed to new_filename
 */
int HAL_FileRename(const char *old_filename, const char *new_filename);

/**
 * @brief Tests the end-of-file indicator for the given stream.
 */
int HAL_FileEof(void *fp);

/**
 * @brief Tests the error indicator for the given stream.
 */
int HAL_FileError(void *fp);

/**
 * @brief Get the current pose of stream.
 */
long HAL_FileTell(void *fp);

/**
 * @brief Get the stream size.
 */
long HAL_FileSize(void *fp);

/**
 * @brief Get one line from the stream.
 */
char *HAL_FileGets(char *str, int n, void *fp);

/**
 * @brief Flushes the output buffer of a stream.
 */
int HAL_FileFlush(void *fp);


#if defined(__cplusplus)
}
#endif

#endif /* __TC_IOT_HAL_H__ */
