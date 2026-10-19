/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic data type.
 */

#ifndef __AIC_TYPE_H__
#define __AIC_TYPE_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long       ulong;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef short               s16;
typedef int                 s32;
typedef long                slong;
typedef long long           s64;

typedef float               f32;
typedef double              f64;

typedef _Bool               boolean;
typedef unsigned long       base_t;
typedef unsigned long       addr_t;
typedef unsigned int        size_t;

typedef unsigned char       __u8;
typedef unsigned short      __u16;
typedef unsigned int        __u32;
typedef unsigned long long  __u64;

typedef signed char         __s8;
typedef short               __s16;
typedef int                 __s32;
typedef long long           __s64;

#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif

typedef __u16 __bitwise__ __le16;
typedef __u16 __bitwise__ __be16;
typedef __u32 __bitwise__ __le32;
typedef __u32 __bitwise__ __be32;

#define __cpu_to_le32(x) ((__force __le32)(__u32)(x))
#define __le32_to_cpu(x) ((__force __u32)(__le32)(x))
#define __cpu_to_le16(x) ((__force __le16)(__u16)(x))
#define __le16_to_cpu(x) ((__force __u16)(__le16)(x))

#ifndef __IO
#define __IO volatile
#endif

#ifndef __iomem
#define __iomem
#endif

#ifndef __force
#define __force
#endif

#ifndef WIN32
#ifndef BOOL
#define BOOL   _Bool
#define DEFINED_BOOL
#endif
#endif
#ifndef inline
#define inline __inline
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL    (void*)0
#endif

#ifndef __volatile__
#define __volatile__    volatile
#endif

#ifndef __uintptr
#define __uintptr   u32
#endif

#define AIC_TRUE    TRUE
#define AIC_FALSE   FALSE
#define AIC_NULL    NULL

#define EOK         (OK)
#define SIGALRM     (14)
#define SIG_ERR     (ERROR)

#define UNUSED_PARAM(p)
#ifdef WIN32
#define AIC_SECTION(sec)
#else
#define AIC_SECTION(sec)    __attribute__((used, section(sec)))
#endif

/**
 * offsetof - the offset of a member relative to the struct.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 * @member_type: the type of the member
 *
 */
#ifndef container_of
#if defined ( __GNUC__ )
#define container_of(ptr, type, member) ({            \
    const typeof( ((type *)0)->member ) *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) );})
#else
#define container_of(ptr, type, member) \
    (type *)( (char *)ptr - offsetof(type,member) )
#endif

#endif  // end of container_of

/* ring buffer move to next index. */
#define RING_NEXT_INDEX(index, cnt) ((index) + 1 == (cnt) ? 0 : (index) + 1)

/*
 * The macros of ring buf define here.
 * Action: the ring buffer size be the power of 2.
 */
#define GET_RING_INDEX(cnt, ptr)        (((cnt) - 1) & (ptr))
#define IS_RING_EMPTY(rd, wt)           ((rd) == (wt))
#define IS_RING_NOT_EMPTY(rd, wt)       ((rd) != (wt))
#define IS_RING_FULL(cnt, rd, wt)       ((rd) + (cnt) == (wt))
#define IS_RING_NOT_FULL(cnt, rd, wt)   ((rd) + (cnt) != (wt))

/**
 * the macor conf must be defined, when conf > 0, return true,
 * otherwise, return false.
 */
#define IS_CONF_ENABLED(conf) (conf)

/**
 * the macor conf must be defined, when conf == 0, return true,
 * otherwise, return false.
 */
#define IS_CONF_DISABLED(conf) (!conf)

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
        }
#endif

#endif /* __AIC_TYPE_H__ */

