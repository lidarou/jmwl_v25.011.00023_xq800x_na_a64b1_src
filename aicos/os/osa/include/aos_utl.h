/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines AOS UTL Interface.
 */

#ifndef __AOS_UTL_H__
#define __AOS_UTL_H__

/* Ensure that standard C is used to process the API information. */
#ifdef __cplusplus
extern "C" {
#endif

#include <aic_type.h>

/**---------------------------------------------------------------------------*/
/**                          constant variables                               */
/**---------------------------------------------------------------------------*/
#define aos_htons       aos_ntohs
#define aos_htonl       aos_ntohl
#define aos_pow         aos_ipow


/**---------------------------------------------------------------------------*/
/**                          type define                                      */
/**---------------------------------------------------------------------------*/
/* section platform_code_core will place to cr5 TCM */
#ifdef CONFIG_CPU_CORTEX_R5
#define PLAT_TCM_CODE __attribute__((section(".platform_code_core")))
#else
#define PLAT_TCM_CODE
#endif

/**---------------------------------------------------------------------------*/
/**                          function prototype                               */
/**---------------------------------------------------------------------------*/
extern f64 pow(f64 x, f64 y);

PLAT_TCM_CODE void aos_memcpy(void *dest_ptr, const void *src_ptr, u32 copy_count);
PLAT_TCM_CODE void aos_memset(void *dest_ptr, u8 value, u32 count);
void aos_memmove(void *dest_ptr, const void *src_ptr, u32 copy_count);

/* random APIs are added temporarily for compile and test.
 * It will be replaced by security module API later */
void aos_init_random_seed(void);
void aos_set_random_seed(u32 seed_temp);
s32 aos_random_value(s32 simin, s32 simax);
s32 aos_random_value_ex(u32 seed, s32 simin, s32 simax);

void aos_enable_tracex(void);
void aos_disable_tracex(void);
void aos_insert_isr_enter(u32 isr_id);
void aos_insert_isr_exit(u32 isr_id);
void aos_dump_trace_buffer(void);
void aos_memory_dump(void);
void aos_enable_stackcheck(void);
/* Ensure that standard C is used to process the API information. */

static inline u32 aos_btol_u32(u32 value)
{
    u32 tmp;
    tmp = (value << 24) & 0xff000000;
    tmp |= (value << 8) & 0x00ff0000;
    tmp |= (value >> 8) & 0x0000ff00;
    tmp |= (value >> 24) & 0x000000ff;
    return tmp;
}

static inline u16 aos_btol_u16(u16 value)
{
    u16 tmp;
    tmp = value & 0x00ff;
    tmp <<= 8;
    tmp = tmp | ((value>>8) & 0x00ff);
    return tmp;
}

static inline u8 * aos_itoa(u32 value, u8 *name, u32 radix)
{
    u16 i = 0;
    u16 j;
    u32 temp_div = value;
    u32 res;

    if(!radix)
        return NULL;

    do {
        for (j = i ; j > 0 ; j--)
            name[j] = name[j-1];

        res = temp_div % radix;
        temp_div /= radix;

        if(res < 10)
            name[0] = (u8)(res + 0x30);
        else
            name[0] =(u8)(res + 0x60 - 9);
        i++;
    } while(temp_div);

    name[i] = '\0';
    return &name[0];
}

static inline u32 aos_ntohl(u32 value)
{
#ifdef BIGENDIAN_SWITCH
    return value;
#else
    u32 tmp;
    tmp = (value << 24) & 0xff000000;
    tmp |= (value << 8) & 0x00ff0000;
    tmp |= (value >> 8) & 0x0000ff00;
    tmp |= (value >> 24) & 0x000000ff;
    return tmp;
#endif
}

static inline u16 aos_ntohs(u16 value)
{
#ifdef BIGENDIAN_SWITCH
    return value;
#else
    u16 tmp;
    tmp = value & 0x00ff;
    tmp <<= 8;
    tmp = tmp | ((value>>8) & 0x00ff);
    return tmp;
#endif
}

static inline u16 aos_get_u16(u8 *ptr)
{
    u16  temp;
    temp = (u16)(*ptr);
    temp = ((temp<<8) & 0xFF00) | ((u16)(*(ptr+1)));
    return temp;
}

static inline void aos_set_u16(u8 *ptr, u16 value)
{
    *(ptr) = (u8)((value>>8) & 0xFF);
    *(ptr + 1) = (u8)(value & 0xFF);
}

static inline u32 aos_get_u32(u8 *ptr)
{
    u32  temp;
    temp = (((u32)*ptr) << 24) & 0xFF000000;
    temp |= (((u32)*(ptr+1)) << 16) & 0x00FF0000;
    temp |= (((u32)*(ptr+2)) << 8) & 0x0000FF00;
    temp |= ((u32)*(ptr+3)) & 0x000000FF;

    return temp;
}

static inline void aos_set_u32(u8 *ptr, u32 value)
{
    *(ptr) = (u8)((value>>24) & 0xFF);
    *(ptr + 1) = (u8)((value>>16) & 0xFF);
    *(ptr + 2) = (u8)((value>>8) & 0xFF);
    *(ptr + 3) = (u8)((value) & 0xFF);
}

static inline u16 aos_get_host_u16(u8 *ptr)
{
#ifdef BIGENDIAN_SWITCH
    u16  temp;
    temp = (u16)(*ptr);
    temp = ((temp<<8) & 0xFF00) | ((u16)(*(ptr+1)));
    return temp;
#else
    u16  temp;
    temp = (u16)(*(ptr+1));
    temp = ((temp<<8) & 0xFF00) | ((u16)(*ptr));
    return temp;
#endif
}

static inline void aos_set_host_u16(u8 *ptr, u16 value)
{
#ifdef BIGENDIAN_SWITCH
    *(ptr) = (u8)((value>>8) & 0xFF);
    *(ptr + 1) = (u8)(value & 0xFF);
#else
    *(ptr) = (u8)(value & 0xFF);
    *(ptr + 1) = (u8)((value>>8) & 0xFF);
#endif

}

static inline u32 aos_get_host_u32(u8 *ptr)
{
#ifdef BIGENDIAN_SWITCH
    u32  temp;
    temp = (((u32)*ptr) << 24) & 0xFF000000;
    temp |= (((u32)*(ptr+1)) << 16) & 0x00FF0000;
    temp |= (((u32)*(ptr+2)) << 8) & 0x0000FF00;
    temp |= ((u32)*(ptr+3)) & 0x000000FF;

    return temp;
#else
    u32  temp;
    temp = ((u32)*ptr) & 0x000000FF;
    temp |= (((u32)*(ptr+1)) << 8) & 0x0000FF00;
    temp |= (((u32)*(ptr+2)) << 16) & 0x00FF0000;
    temp |= (((u32)*(ptr+3)) << 24) & 0xFF000000;

    return temp;
#endif
}

static inline void aos_set_host_u32(u8 *ptr, u32 value)
{
#ifdef BIGENDIAN_SWITCH
    *(ptr) = (u8)((value>>24) & 0xFF);
    *(ptr + 1) = (u8)((value>>16) & 0xFF);
    *(ptr + 2) = (u8)((value>>8) & 0xFF);
    *(ptr + 3) = (u8)((value) & 0xFF);
#else
    *(ptr) = (u8)((value) & 0xFF);
    *(ptr + 1) = (u8)((value>>8) & 0xFF);
    *(ptr + 2) = (u8)((value>>16) & 0xFF);
    *(ptr + 3) = (u8)((value>>24) & 0xFF);
#endif
}

static inline u32 aos_ipow(u32 op, u32 exp)
{
    u32 result = op;

    if(exp == 0)
        return 1;

    for(; exp > 1; exp--)
        result = result * op;

    return result;
}

static inline f64 aos_fpow(f64 op, f64 exp)
{
    return pow(op, exp);
}

#ifdef __cplusplus
        }
#endif

#endif/* __AOS_UTL_H__ */

