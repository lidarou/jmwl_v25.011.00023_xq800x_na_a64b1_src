
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines some mem apis that aic reimplemented apis.
 */

#define AIC_THIS_FILE    0x03197f
#define LOG_MODULE    AMOD_AOS

#ifndef CONFIG_PLAT_RAC_LIBC_MEM
#include <aic_type.h>
#else
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned int        size_t;

/*
 * CONFIG_HIGH_PERF
 * Open it will get higer performance and need more code size.
 */
#define CONFIG_HIGH_PERF    1

#if CONFIG_HIGH_PERF
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

void *memcpy(void *dst, const void *src, size_t size);
void *memmove(void *dst, const void *src, size_t size);
void *memset(void *dst, int v, size_t size);
#if 1
void *memset(void *dst, int v, size_t size)
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

void *memcpy(void *dst, const void *src, size_t size)
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

void *memcpy_reverse(void *dst, const void *src, size_t size)
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

void *memmove(void *dst, const void *src, size_t size)
{
    /* dst does not overlay the src. */
    if (src >= dst || (u8 *)dst >= (u8 *)src + size)
        return memcpy(dst, src, size);
    else
        return memcpy_reverse(dst, src, size);
}
#endif
#else
void *memset(void *dst, int v, size_t size)
{
    u8 *d = (u8 *)dst;
    size_t i;

    for (i = 0; i < size; i++)
        *d++ = (u8)v;

    return dst;
}

void *memcpy(void *dst, const void *src, size_t size)
{
    u8 *d = (u8 *)dst;
    u8 *s = (u8 *)src;
    size_t i;

    for (i = 0; i < size; i++)
        *d++ = *s++;

    return dst;
}

void *memmove(void *dst, const void *src, size_t size)
{
    size_t i;

    /* dst does not overlay the src. */
    if (src >= dst || (u8 *)dst >= (u8 *)src + size) {
        u8 *d = (u8 *)dst;
        u8 *s = (u8 *)src;

        for (i = 0; i < size; i++)
            *d++ = *s++;
    } else {
        u8 *d = (u8 *)dst + size - 1;
        u8 *s = (u8 *)src + size - 1;

        for (i = 0; i < size; i++)
            *d-- = *s--;
    }

    return dst;
}
#endif /* CONFIG_HIGH_PERF */
#endif /* CONFIG_PLAT_RAC_LIBC_MEM */

#ifndef CONFIG_RTOS_LITEOS_M
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

int memset_s(void *buf, size_t bufsz, int ch, size_t count)
{
    return (memset(buf, ch, MIN(bufsz, count)) ? 0 : -1);
}

int memmove_s(void *dst, size_t dstsz, void *src, size_t srcsz)
{
    return (memmove(dst, src, MIN(dstsz, srcsz)) ? 0 : -1);
}
#endif

