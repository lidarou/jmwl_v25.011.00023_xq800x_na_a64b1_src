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

#ifndef __TC_IOT_PLATFORM_INC_H__
#define __TC_IOT_PLATFORM_INC_H__


// OS dependant
// enable the specific macro in different platform

#if defined(__linux__)
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif


#ifdef PLATFORM_FREERTOS
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/task.h"
#endif


#ifdef JIELI_RTOS
#include "os_api.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/inet.h"
#endif


#ifdef PLATFORM_RTTHREAD
#include <rtthread.h>
#include <rtconfig.h>
#include <dfs_posix.h>
#endif

#ifdef PLAT_USE_THREADX
// HAL for ASR3603 platform which is based on ThreadX
#include "osa.h"
#include "osa_mem.h"
#include "tx_api.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/netif.h"

typedef int ssize_t;
#endif

#ifdef PLATFORM_HAS_CMSIS
#include "cmsis_os.h"
#endif


#ifdef OS_ANDROID
#include <android/log.h>
#endif


#ifdef WIN32
#include <Windows.h>
#include <direct.h>
#include <limits.h>
#define getcwd(buffer, len) _getcwd(buffer, len)
typedef unsigned long ssize_t;
#endif

#ifdef PLATFORM_AICOS
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "aic_osa.h"
#include "aos_mem.h"
#include "aic_net_api.h"

//In GX316 sdk,if use signal(), halu port interface must user posix function,
//that will run slowly, so we use aic_thread_xxx(), and wo can't use signal
//signal() belong posix, mixed use posix and aic_thread_xxx(), task_struct is diff
//which will cause program dump
#ifndef signal
#define signal(x, y)  0    
#endif


#endif

#endif /* __TC_IOT_PLATFORM_INC_H__ */
