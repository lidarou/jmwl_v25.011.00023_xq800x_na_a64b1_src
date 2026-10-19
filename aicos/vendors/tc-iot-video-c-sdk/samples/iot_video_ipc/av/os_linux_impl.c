/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1da58c
#define LOG_MODULE    AMOD_VENDOR

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include "fs_api.h"

#include "wmpf/wxvoip_os_impl.h"
#include "tc_iot_log.h"
#include "tc_iot_hal.h"

// #define VOIP_MEM_STRACCE

#ifdef VOIP_MEM_STRACCE
typedef struct {
    void *p;
    size_t size;
} memory_item_t;

static memory_item_t memory_item[1024] = { 0 };
static size_t malloc_size = 0;
#endif

static void *os_linux_malloc(size_t size)
{
    void *p = malloc(size);

    if (p) {
        memset(p, 0, size);

        #ifdef VOIP_MEM_STRACCE

        for (int i=0; i < 1024; i++) {
            if (memory_item[i].p == 0) {
                memory_item[i].p = p;
                memory_item[i].size = size;

                printf("malloc %p\n", p);
                break;
            }
        }

        malloc_size += size;
        printf("->  %zu\n", malloc_size);
        #endif
    }

    return p; 
}

static void os_linux_free(void *ptr)
{
    if (ptr) {
        #ifdef VOIP_MEM_STRACCE

        for (int i=0; i < 1024; i++) {
            if (memory_item[i].p == ptr) {
                
                malloc_size -= memory_item[i].size;
                printf("free %p %zu\n", ptr, memory_item[i].size);

                memory_item[i].p = 0;
                memory_item[i].size = 0;

                printf("->  %zu\n", malloc_size);
                break;
            }
        }
        #endif

        free(ptr);
    }
}


void MallocList()
{   
    #ifdef VOIP_MEM_STRACCE

    printf("mem report:\n");

    for (int i=0; i < 1024; i++) {
        if (memory_item[i].p != 0) {
            printf("\tptr %p %zu\n", memory_item[i].p, memory_item[i].size);
        }
    }

    #endif
}

static void os_linux_print(const char *fmt, va_list ap)
{
	//临时实现，客户自己修改，避免异常
    char buf[512] = {0};
    HAL_Vsnprintf(buf, 512, fmt, ap);

    Log_d("rocky wx %s", buf);
    // vprintf(fmt, ap);
    // fflush(stdout);
}

static uint32_t os_linux_gettime_ms(void)
{
    struct timeval time_val = {0};
    uint32_t       time_ms;

    gettimeofday(&time_val, NULL);
    time_ms = time_val.tv_sec * 1000 + time_val.tv_usec / 1000;

    return time_ms;
}

static uint32_t os_linux_gettime_second(void)
{
    return (uint32_t)time(NULL);
}

static void os_linux_msleep(uint32_t ms)
{
    aos_sleep(ms);
}

static void os_linux_getmac(uint8_t *mac, size_t mac_len)
{
    memset(mac, 0x12, mac_len);   

    // struct ifaddrs *ifaddr = NULL;
    // struct ifaddrs *ifa = NULL;
    // size_t i = 0;

    // if (getifaddrs(&ifaddr) == -1) {
    //     return;
    // }

    // for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    //     if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) ) {
    //         struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;

    //         /* skip loopback device */
    //         if (strcasecmp(ifa->ifa_name, "lo") == 0) {
    //             continue;
    //         }

    //         for (i = 0; i < s->sll_halen && i < mac_len; i++) {
    //             mac[i] = s->sll_addr[i];
    //         }
    //         break;
    //     }
    // }

    // freeifaddrs(ifaddr);
}

static size_t os_linux_get_disksize(void)
{
    int disk_size = 64; //64KB
    return disk_size;

}

static size_t os_linux_get_memsize(void)
{
    int memsize = 4 * 1024;
    return memsize;
}

static const char * os_linux_get_platform(void)
{
    return "GX316";
}

static int os_linux_create_file(const char *path)
{
	//原本是RDWR | CREATE,不清楚归芯SDK底层实现，暂时只做读操作，客户实现RDWR | CREATE
    return fs_open(path, FS_O_RDONLY);
}

static size_t os_linux_read_file(int fd, void *buf, size_t count)
{
    return fs_read(fd, buf, count);
}

static size_t os_linux_write_file(int fd, const void *buf, size_t count)
{
    return fs_write(fd, buf, count);
}

static int os_linux_close_file(int fd)
{
    return fs_close(fd);
}

static long int os_linux_random(void)
{
	//客户调用他的random函数
    return HAL_GetTimeMs();
}

wxvoip_os_impl_t os_impl = {
    .os_malloc = os_linux_malloc,
    .os_free = os_linux_free,
    .os_printf = os_linux_print,
    .os_gettime_ms = os_linux_gettime_ms,
    .os_gettime_second = os_linux_gettime_second,
    .os_msleep = os_linux_msleep,
    .os_getmac = os_linux_getmac,
    .os_get_disksize = os_linux_get_disksize,
    .os_get_memsize = os_linux_get_memsize,
    .os_get_platform = os_linux_get_platform,
    .os_create_file = os_linux_create_file,
    .os_read_file = os_linux_read_file,
    .os_write_file = os_linux_write_file,
    .os_close_file = os_linux_close_file,
    .os_random = os_linux_random
};
