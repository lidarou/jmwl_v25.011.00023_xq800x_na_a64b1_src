/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file define arch related mpu informations
 *
 */

#ifndef __MPU_CR5_H__
#define __MPU_CR5_H__

#include <aic_bitmap.h>
#include <aic_type.h>

/* Bit definitions */
#define MPU_CTRL_BIT   BIT_0
#define MPU_RG_EN      0x1
/* Note:set same ap for PL0 and PL1 */
#define MPU_AP_NACCESS 0x0
#define MPU_AP_RDONLY  0x6
#define MPU_AP_RDWR    0x3
#define MPU_NO_EXEC    0x1
/* Note:disable shareability */
#define MPU_DEV_NGNRNE 0x0
#define MPU_DEV_SH     0x1
#define MPU_DEV_NSH    0x10
#define MPU_MEM_WTNWA  0x2
#define MPU_MEM_WBNWA  0x3
#define MPU_MEM_WBWA   0xB
#define MPU_MEM_NC     0x8

/* Minimum and max region size */
#define RG_MIN_SIZE 0x4
#define RG_MAX_SIZE 0x20

#if defined(__CC_ARM) /* Arm compiler 5 */

__STATIC_INLINE u32 get_mpuir(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c0, c0, 4");
    return val;
}

__STATIC_INLINE void set_mpuir(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c0, c0, 4");
}

__STATIC_INLINE u32 get_rbar(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c6, c1, 0");
    return val;
}

__STATIC_INLINE void set_rbar(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c6, c1, 0");
}

__STATIC_INLINE u32 get_drsr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c6, c1, 2");
    return val;
}

__STATIC_INLINE void set_drsr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c6, c1, 2");
}

__STATIC_INLINE u32 get_racr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c6, c1, 4");
    return val;
}

__STATIC_INLINE void set_racr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c6, c1, 4");
}

__STATIC_INLINE u32 get_rgnr(void)
{
    u32 val;

    __ASM __volatile__("mrc p15, 0, val, c6, c2, 0");
    return val;
}

__STATIC_INLINE void set_rgnr(u32 val)
{
    __ASM __volatile__("mcr p15, 0, val, c6, c2, 0");
}

#elif defined(__GNUC__) /* Normal GCC and AC6 */

__STATIC_INLINE u32 get_mpuir(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c0, c0, 4"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_mpuir(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c0, c0, 4"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_rbar(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c6, c1, 0"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_rbar(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c6, c1, 0"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_drsr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c6, c1, 2"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_drsr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c6, c1, 2"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_racr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c6, c1, 4"\
    :"=r"(val)\
    );

    return val;
}

__STATIC_INLINE void set_racr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c6, c1, 4"\
    ::"r"(val)\
    );
}

__STATIC_INLINE u32 get_rgnr(void)
{
    u32 val;

    __ASM __volatile__(\
    "mrc p15, 0, %0, c6, c2, 0"\
    :"=r"(val)\
    );
    return val;
}

__STATIC_INLINE void set_rgnr(u32 val)
{
    __ASM __volatile__(\
    "mcr p15, 0, %0, c6, c2, 0"\
    ::"r"(val)\
    );
}

#else
 #error Unknown compiler
#endif

/* Structure definitions */
typedef union
{
    struct
    {
        u32 separate:1;    /* bit0 separate */
        u32 reserved1:7;   /* bit1~7 reserved */
        u32 dregion:8;     /* bit8~15 number of rg supported */
        u32 reserved2:16;  /* bit16~31 reserved */
    } b;
    u32 w;
} MPU_IR_T;

typedef union
{
    struct
    {
        u32 region:4;       /* bit0~3 region */
        u32 reserved:28;    /* bit4~31 reserved */
    } b;
    u32 w;
} MPU_RGNR_T;

typedef union
{
    struct
    {
        u32 reserved:2;     /* bit0~1 region number */
        u32 base:30;        /* bit2~31 base address of region*/
    } b;
    u32 w;
} MPU_DRBAR_T;

typedef union
{
    struct
    {
        u32 en:1;           /* bit0 region enable */
        u32 size:5;         /* bit1~5 region size */
        u32 reserved1:2;    /* bit6~7 reserved */
        u32 srd:8;          /* bit8~15 sub region */
        u32 reserved2:16;   /* bit16~31 attributes */
    } b;
    u32 w;
} MPU_DRSR_T;

typedef union
{
    struct
    {
        u32 attrs:6;       /* bit0~5 attributes */
        u32 reserved1:2;   /* bit6~7 reserved */
        u32 ap:3;          /* bit8~10 attributes */
        u32 reserved2:1;   /* bit11 reserved */
        u32 xn:1;          /* bit12 reserved */
        u32 reserved:19;   /* bit13~31 reserved */
    } b;
    u32 w;
} MPU_RACR_T;

struct rg_size {
    u32 idx;
    u32 size;
};

#endif /* __MPU_CR5_H__ */
