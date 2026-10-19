/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    The log trace test file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x142bec
#define LOG_MODULE    AMOD_ALOG

#include <aic_init.h>
#include <aic_osa.h>
#include <alog_trace_test.h>
#include <sprintf_nano.h>
#include <stdio.h>

int alog_test_int(void)
{
    int i = 0;

    aicx_info(0x09a0b8, "", "alog_test_main.\n");

    aicx_bitstr(0x1bc6db, "dmmmS", "%d. test0 str: [bits(0x%m)], [bits(0x%m)], [bits(0x%m)], bitsstr(0x%S)\n",
                i++, 0x00000011, 0x00111100, 0x11000000, LOG_BITSTR_TEST0);

    aicx_bitstr(0x0cb795, "dmmmS", "%d. test1 str: [bits(0x%m)], [bits(0x%m)], [bits(0x%m)], bitsstr(0x%S)\n",
                i++, 0x00000011, 0x00111100, 0x11000000, LOG_BITSTR_TEST1);

    aicx_bitstr(0x007558, "dmmmS", "%d. test2 str: [bits(0x%m)], [bits(0x%m)], [bits(0x%m)], bitsstr(0x%S)\n",
                i++, 0x00000011, 0x00111100, 0x11000000, LOG_BITSTR_TEST2);

    aicx_bitstr(0x04c56f, "dMMMSSS", "%d. test3 str0: [BITS(0x%M)], str1: [BITS(0x%M)], str2: [BITS(0x%M)], "\
                "bitsstr(0x%S), bitsstr(0x%S), bitsstr(0x%S)\n",
                i++, 0x00000011, 0x00111100, 0x11000000,
                LOG_BITSTR_TEST0, LOG_BITSTR_TEST1, LOG_BITSTR_TEST2);

    return 0;
}
//AIC_COMP_P2_INIT_EXPORT(alog_test_int);

int snprintf_test_int(void)
{
    int i;
    char buffer[50];

    aicx_info(0x007946, "", "snprintf_libc test start.\n");

    for (i = 0; i < 10000; i++)
        snprintf(buffer, sizeof(buffer), " %s test i=%d.\n", "snprintf_nano", i);

    aicx_info(0x18d0e9, "", "snprintf_libc test end.\n");

    aicx_info(0x08c567, "", "snprintf_nano test start.\n");

    for (i = 0; i < 10000; i++)
        snprintf_nano(buffer, sizeof(buffer), " %s test i=%d.\n", "snprintf_nano", i);

    aicx_info(0x1bd2fb, "", "snprintf_nano test end.\n");
    return 0;
}
//AIC_COMP_P2_INIT_EXPORT(snprintf_test_int);

#define MEMSET_32BYTE(dst, v32, size) do { \
        *((u32*)(dst + 0x00)) = v32; \
        *((u32*)(dst + 0x04)) = v32; \
        *((u32*)(dst + 0x08)) = v32; \
        *((u32*)(dst + 0x0c)) = v32; \
        *((u32*)(dst + 0x10)) = v32; \
        *((u32*)(dst + 0x14)) = v32; \
        *((u32*)(dst + 0x18)) = v32; \
        *((u32*)(dst + 0x1c)) = v32; \
        dst += 0x20; \
        size -= 0x20; \
    } while (0)

#define MEMSET_16BYTE(dst, v32, size) do { \
        *((u32*)(dst + 0x00)) = v32; \
        *((u32*)(dst + 0x04)) = v32; \
        *((u32*)(dst + 0x08)) = v32; \
        *((u32*)(dst + 0x0c)) = v32; \
        dst += 0x10; \
        size -= 0x10; \
    } while (0)

#define MEMSET_8BYTE(dst, v32, size) do { \
        *((u32*)(dst + 0x00)) = v32; \
        *((u32*)(dst + 0x04)) = v32; \
        dst += 0x08; \
        size -= 0x08; \
    } while (0)

#define MEMSET_4BYTE(dst, v32, size) do { \
        *((u32*)(dst)) = v32; \
        dst += 0x04; \
        size -= 0x04; \
    } while (0)

#define MEMSET_2BYTE(dst, v16, size) do { \
        *((u16*)(dst)) = v16; \
        dst += 0x02; \
        size -= 0x02; \
    } while (0)

#define MEMSET_1BYTE(dst, v, size) do { \
        *((u8*)(dst)) = v; \
        dst += 0x01; \
        size -= 0x01; \
    } while (0)

#define COPY_32BYTE(dst, src, size) do { \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        *((u32*)(dst + 0x08)) = *((u32*)(src + 0x08)); \
        *((u32*)(dst + 0x0c)) = *((u32*)(src + 0x0c)); \
        *((u32*)(dst + 0x10)) = *((u32*)(src + 0x10)); \
        *((u32*)(dst + 0x14)) = *((u32*)(src + 0x14)); \
        *((u32*)(dst + 0x18)) = *((u32*)(src + 0x18)); \
        *((u32*)(dst + 0x1c)) = *((u32*)(src + 0x1c)); \
        dst += 0x20; \
        src += 0x20; \
        size -= 0x20; \
    } while (0)

#define COPY_16BYTE(dst, src, size) do { \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        *((u32*)(dst + 0x08)) = *((u32*)(src + 0x08)); \
        *((u32*)(dst + 0x0c)) = *((u32*)(src + 0x0c)); \
        dst += 0x10; \
        src += 0x10; \
        size -= 0x10; \
    } while (0)

#define COPY_8BYTE(dst, src, size) do { \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        dst += 0x08; \
        src += 0x08; \
        size -= 0x08; \
    } while (0)

#define COPY_4BYTE(dst, src, size) do { \
        *((u32*)(dst)) = *((u32*)(src)); \
        dst += 0x04; \
        src += 0x04; \
        size -= 0x04; \
    } while (0)

#define COPY_2BYTE(dst, src, size) do { \
        *((u16*)(dst)) = *((u16*)(src)); \
        dst += 0x02; \
        src += 0x02; \
        size -= 0x02; \
    } while (0)

#define COPY_1BYTE(dst, src, size) do { \
        *((u8*)(dst)) = *((u8*)(src)); \
        dst += 0x01; \
        src += 0x01; \
        size -= 0x01; \
    } while (0)

#define COPY_32BYTE_REV(dst, src, size) do { \
        dst -= 0x20; \
        src -= 0x20; \
        size -= 0x20; \
        *((u32*)(dst + 0x1c)) = *((u32*)(src + 0x1c)); \
        *((u32*)(dst + 0x18)) = *((u32*)(src + 0x18)); \
        *((u32*)(dst + 0x14)) = *((u32*)(src + 0x14)); \
        *((u32*)(dst + 0x10)) = *((u32*)(src + 0x10)); \
        *((u32*)(dst + 0x0c)) = *((u32*)(src + 0x0c)); \
        *((u32*)(dst + 0x08)) = *((u32*)(src + 0x08)); \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
    } while (0)

#define COPY_16BYTE_REV(dst, src, size) do { \
        dst -= 0x10; \
        src -= 0x10; \
        size -= 0x10; \
        *((u32*)(dst + 0x0c)) = *((u32*)(src + 0x0c)); \
        *((u32*)(dst + 0x08)) = *((u32*)(src + 0x08)); \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
    } while (0)

#define COPY_8BYTE_REV(dst, src, size) do { \
        dst -= 0x08; \
        src -= 0x08; \
        size -= 0x08; \
        *((u32*)(dst + 0x04)) = *((u32*)(src + 0x04)); \
        *((u32*)(dst + 0x00)) = *((u32*)(src + 0x00)); \
    } while (0)

#define COPY_4BYTE_REV(dst, src, size) do { \
        dst -= 0x04; \
        src -= 0x04; \
        size -= 0x04; \
        *((u32*)(dst)) = *((u32*)(src)); \
    } while (0)

#define COPY_2BYTE_REV(dst, src, size) do { \
        dst -= 0x02; \
        src -= 0x02; \
        size -= 0x02; \
        *((u16*)(dst)) = *((u16*)(src)); \
    } while (0)

#define COPY_1BYTE_REV(dst, src, size) do { \
        dst -= 0x01; \
        src -= 0x01; \
        size -= 0x01; \
        *((u8*)(dst)) = *((u8*)(src)); \
    } while (0)

void *memset_test(void *dst, int v, size_t size)
{
    u8 *d = (u8*)dst;
    u16 v16 = ((v << 8) | v);
    u32 v32 = ((v << 24) | (v << 16) | v16);

    while (size) {
        /* dst and src are all aligined with sizeof(u32) */
        if (0 == ((u32)d & (sizeof(u32) - 1))) {
            if (size >= 0x20) {
                MEMSET_32BYTE(d, v32 , size);
            } else if (size >= 0x10) {
                MEMSET_16BYTE(d, v32, size);
            } else if (size >= 0x08) {
                MEMSET_8BYTE(d, v32, size);
            } else if (size >= 0x04) {
                MEMSET_4BYTE(d, v32, size);
            }  else if (size >= 0x02) {
                MEMSET_2BYTE(d, v16, size);
            } else {
                MEMSET_1BYTE(d, v, size);
            }
            continue;
        } 

        /* dst and src are all aligined with sizeof(u16) */
        if (0 == ((u32)d & (sizeof(u16) - 1))) {
            if (size >= 0x02) {
                MEMSET_2BYTE(d, v16, size);
            } else {
                MEMSET_1BYTE(d, v, size);
            }
            continue;
        }

        /* memset 1 byte once. */
        MEMSET_1BYTE(d, v, size);
    }

    return dst;
}

void *memcpy_test(void *dst, const void *src, size_t size)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    /* dst and src are all aligined with sizeof(u32) */
    if (0 == (((u32)dst - (u32)src) & (sizeof(u32) - 1))) {
        /* proc the data not aligined with sizeof(u32) */
        while (size && (u32)d & (sizeof(u32) - 1)) {
            COPY_1BYTE(d, s, size);
        }

        while (size) {
            if (size >= 0x20) {
                COPY_32BYTE(d, s, size);
            } else if (size >= 0x10) {
                COPY_16BYTE(d, s, size);
            } else if (size >= 0x08) {
                COPY_8BYTE(d, s, size);
            } else if (size >= 0x04) {
                COPY_4BYTE(d, s, size);
            }  else if (size >= 0x02) {
                COPY_2BYTE(d, s, size);
            } else {
                COPY_1BYTE(d, s, size);
            }
        }
        return dst;
    } 

    /* dst and src are all aligined with sizeof(u16) */
    if (0 == (((u32)dst - (u32)src) & (sizeof(u16) - 1))) {
        /* proc the data not aligined with sizeof(u16) */
        if (size && (u32)d & (sizeof(u16) - 1)) {
            COPY_1BYTE(d, s, size);
        }

        while (size) {
            if(size >= 0x02) {
                COPY_2BYTE(d, s, size);
            } else {
                COPY_1BYTE(d, s, size);
            }
        }
        return dst;
    }

    /* copy 1 byte once. */
    while (size) {
        COPY_1BYTE(d, s, size);
    }

    return dst;
}

void *memcpy_reverse_test(void *dst, const void *src, size_t size)
{
    u8 *d = (u8 *)dst + size;
    const u8 *s = (const u8 *)src + size;

    /* dst and src are all aligined with sizeof(u32) */
    if (0 == (((u32)d - (u32)s) & (sizeof(u32) - 1))) {
        /* proc the data not aligined with sizeof(u32) */
        while (size && (u32)d & (sizeof(u32) - 1)) {
            COPY_1BYTE_REV(d, s, size);
        }

        while (size) {
            if (size >= 0x20) {
                COPY_32BYTE_REV(d, s, size);
            } else if (size >= 0x10) {
                COPY_16BYTE_REV(d, s, size);
            } else if (size >= 0x08) {
                COPY_8BYTE_REV(d, s, size);
            } else if (size >= 0x04) {
                COPY_4BYTE_REV(d, s, size);
            }  else if (size >= 0x02) {
                COPY_2BYTE_REV(d, s, size);
            } else {
                COPY_1BYTE_REV(d, s, size);
            }
        }
        return dst;
    } 

    /* dst and src are all aligined with sizeof(u16) */
    if (0 == (((u32)dst - (u32)src) & (sizeof(u16) - 1))) {
        /* proc the data not aligined with sizeof(u16) */
        if (size && (u32)d & (sizeof(u16) - 1)) {
            COPY_1BYTE_REV(d, s, size);
        }

        while (size) {
            if(size >= 0x02) {
                COPY_2BYTE_REV(d, s, size);
            } else {
                COPY_1BYTE_REV(d, s, size);
            }
        }
        return dst;
    }

    /* copy 1 byte once. */
    while (size) {
        COPY_1BYTE_REV(d, s, size);
    }

    return dst;
}

void *memmove_test(void *dst, const void *src, size_t size)
{
    /* dst does not overlay the src. */
    if (src >= dst || (u8 *)dst >= (u8 *)src + size)
        return memcpy_test(dst, src, size);
    else
        return memcpy_reverse_test(dst, src, size);
}

int memcmp_test(u8 *dst, u8 *src, size_t size)
{
    size_t i, j;

    for (i = 0; i < size; i++) {
        while (*dst != *src)
            ;
        dst++;
        src++;
    }

    return 0;
}

void mem_test_init_size(size_t init_size)
{
    size_t i, j;
    void *p1, *p2;
    u8 *src = aos_malloc(init_size);
    u8 *dst = aos_malloc(init_size);
    u8 *src_test = aos_malloc(init_size);
    u8 *dst_test = aos_malloc(init_size);

    /* test memset */
    for (i = 0; i < init_size; i++) {
        *(src + i) = (u8)i;
        *(src_test + i) = (u8)(i + 1);
        *(dst + i) = (u8)(init_size - i);
        *(dst_test + i) = (u8)(init_size - i -1);
    }

    for (i = 0; i < init_size; i++) {
        p1 = memset(src + i, (u8)i, init_size - i);
        p2 = memset_test(src_test + i, (u8)i, init_size - i);
        while (((u8*)p1 - src) != ((u8*)p2 - src_test));
        while (0 != memcmp_test(src, src_test, init_size - i));
    }

    /* test memcpy */
    for (i = 0; i < init_size; i++) {
        *(src + i) = (u8)i;
        *(src_test + i) = (u8)i;
        *(dst + i) = (u8)(i + 1);
        *(dst_test + i) = (u8)(init_size - i -1);
    }

    for (i = 0; i < init_size; i++) {
        p1 = memcpy(dst + i, src + i, init_size - i);
        p2 = memcpy_test(dst_test + i, src_test + i, init_size - i);
        while (((u8*)p1 - dst) != ((u8*)p2 - dst_test));
        while (0 != memcmp_test(dst, dst_test, init_size - i));
    }

    /* test memmove */
    for (i = 0; i < init_size; i++) {
        *(src + i) = (u8)i;
        *(src_test + i) = (u8)i;
        *(dst + i) = (u8)(i + 1);
        *(dst_test + i) = (u8)(i + 1);
    }

    for (i = 0; i < init_size/2; i++) {
        p1 = memmove(dst, dst + i, i);
        p2 = memmove_test(dst_test, dst_test + i, i);
        while (((u8*)p1 - dst) != ((u8*)p2 - dst_test));
        while (0 != memcmp_test(dst, dst_test, i));
    }

    for (i = 0; i < init_size/4; i++) {
        p1 = memmove(dst + i, dst,  i*2 + 1);
        p2 = memmove_test(dst_test + i, dst_test, i*2 + 1);
        while (((u8*)p1 - dst) != ((u8*)p2 - dst_test));
        while (0 != memcmp_test(dst + i, dst_test + i, i*2 + 1));
    }

    /* memset test */
    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = 0; i <= init_size; i++) {
        memset(dst, 0x5a, i);
        memset_test(dst_test, 0x5a, i);
        while (0 != memcmp_test(dst, dst_test, init_size));
    }

    /* memcpy test */
    for (i = 0; i < init_size; i++) {
        *(src + i) = i;
        *(src_test + i) = i;
    }

    memset(dst, 0xff, init_size);
    memset(dst_test, 0xff, init_size);
    for (i = 0; i <= init_size; i++) {
        memcpy(dst, src, i);
        memcpy_test(dst_test, src_test, i);
        while (0 != memcmp_test(dst, dst_test, init_size));
    }

    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = i; i < init_size/2; i++) {
       for (j = 0; j < init_size/2; j++) {
         memmove(dst, dst + i, j);
         memmove_test(dst_test, dst_test + i, j);
        while (0 != memcmp_test(dst, dst_test, init_size));
       }
    }

    /* memmove test */
    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = 1; i <= init_size; i++) {
        memmove(dst, dst + init_size - i, i);
        memmove_test(dst_test, dst_test + init_size - i, i);
        while (0 != memcmp_test(dst, dst_test, init_size));
    }

    /* memmove test */
    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = 1; i <= init_size/2; i++) {
        memmove(dst + init_size/2 - i, dst, init_size/2 + i);
        memmove_test(dst_test + init_size/2 - i, dst_test, init_size/2 + i);
        while (0 != memcmp_test(dst, dst_test, init_size));
    }

    /* memmove test */
    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = i; i < init_size/2; i++) {
       for (j = 0; j < init_size/2; j++) {
         memmove(dst, dst + i, j);
         memmove_test(dst_test, dst_test + i, j);
        while (0 != memcmp_test(dst, dst_test, init_size));
       }
    }

    for (i = 0; i < init_size; i++) {
        *(dst + i) = i;
        *(dst_test + i) = i;
    }

    for (i = i; i < init_size/2; i++) {
       for (j = 0; j < init_size/2; j++) {
         memmove(dst + init_size/2 - i, dst, j);
         memmove_test(dst_test + init_size/2 - i, dst_test, j);
        while (0 != memcmp_test(dst, dst_test, init_size));
       }
    }

    aos_free(src);
    aos_free(src_test);
    aos_free(dst);
    aos_free(dst_test);
}

void mem_test_int()
{
    size_t size;

    for (size = 1; size < 0x400; size++)
        mem_test_init_size(size);
}

int memcpy_test_int(void)
{ 
    int i;
    u8 *src = aos_malloc(0x800);
    u8 *dst = aos_malloc(0x800);

    aicx_info(0x1c2afc, "", "memcpy start.\n");
    for (i = 0; i < 100; i++) {
        memset((void*)src, i, 0x800);
        memcpy((void*)dst, (void*)src, 0x800);
        memmove((void*)dst, dst + 1, 0x7ff);
    }
    aicx_info(0x05d105, "", "memcpy end.\n");

    aicx_info(0x0408ce, "", "memcpy test start.\n");
    for (i = 0; i < 100; i++) {
        memset_test((void*)src, i, 0x800);
        memcpy_test((void*)dst, (void*)src, 0x800);
        memmove_test((void*)dst, dst + 1, 0x7ff);
    }
    aicx_info(0x03ac25, "", "memcpy test end.\n");

    aos_free(src);
    aos_free(dst);

    mem_test_int();
    return 0;
}
//AIC_COMP_P2_INIT_EXPORT(memcpy_test_int);

