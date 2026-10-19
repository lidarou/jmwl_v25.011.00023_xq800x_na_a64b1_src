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

#ifndef __MPU_CM7_H__
#define __MPU_CM7_H__

/* Reg definitions */
#define MPU_TYPE          0xE000ED90
#define MPU_CTRL          0xE000ED94
#define MPU_RNR           0xE000ED98
#define MPU_RBAR          0xE000ED9C
#define MPU_RASR          0xE000EDA0
#define MPU_RBAR_A1       0xE000EDA4
#define MPU_RASR_A1       0xE000EDA8
#define MPU_RBAR_A2       0xE000EDAC
#define MPU_RASR_A2       0xE000EDB0
#define MPU_RBAR_A3       0xE000EDB4
#define MPU_RASR_A3       0xE000EDB8

/* Bit definitions */
#define MPU_EN            0x1
#define MPU_RG_EN         0x1
#define MPU_HFNMIENA_EN   0x1
#define MPU_PRIVDEFENA_EN 0x1
/* Note:set same ap for PL0 and PL1 */
#define MPU_AP_NACCESS    0x0
#define MPU_AP_RDONLY     0x6
#define MPU_AP_RDWR       0x3
#define MPU_NO_EXEC       0x1
/* Note:disable shareability */
#define MPU_DEV_NGNRNE    0x0
#define MPU_MEM_WTNWA     0x2
#define MPU_MEM_WBNWA     0x3
#define MPU_MEM_WBWA      0xB
#define MPU_MEM_NC        0x8
/* Region size */
#define RG_32B            0x4
#define RG_64B            0x5
#define RG_128B           0x6
#define RG_256B           0x7
#define RG_512B           0x8
#define RG_1KB            0x9
#define RG_2KB            0xA
#define RG_4KB            0xB
#define RG_8KB            0xC
#define RG_16KB           0xD
#define RG_32KB           0xE
#define RG_64KB           0xF
#define RG_128KB          0x10
#define RG_256KB          0x11
#define RG_512KB          0x12
#define RG_1MB            0x13
#define RG_2MB            0x14
#define RG_4MB            0x15
#define RG_8MB            0x16
#define RG_16MB           0x17
#define RG_32MB           0x18
#define RG_64MB           0x19
#define RG_128MB          0x1A
#define RG_256MB          0x1B
#define RG_512MB          0x1C
#define RG_1GB            0x1D
#define RG_2GB            0x1E
#define RG_4GB            0x1F

/* Structure definitions */
typedef union
{
    struct
    {
        u32 separate:1;   /* bit0 separate */
        u32 reserved1:7;  /* bit1~7 reserved */
        u32 dregion:8;    /* bit8~15 number of rg supported */
        u32 iregion:8;    /* bit16~23 reserved */
        u32 reserved2:8;  /* bit24~31 reserved */
    } b;
    u32 w;
} MPU_TYPE_T;

typedef union
{
    struct
    {
        u32 enable:1;       /* bit0 mpu enable */
        u32 hfnmiena:1;     /* bit1 HFNMIENA */
        u32 privdefena:1;   /* bit2 PRIVDEFENA */
        u32 reserved:29;    /* bit3~31 reserved */
    } b;
    u32 w;
} MPU_CTRL_T;

typedef union
{
    struct
    {
        u32 region:8;       /* bit0~7 region */
        u32 reserved:24;    /* bit8~31 reserved */
    } b;
    u32 w;
} MPU_RNR_T;

typedef union
{
    struct
    {
        u32 region:4;       /* bit0~3 region number */
        u32 valid:1;        /* bit4 use rbar region number */
        u32 base:27;        /* bit5~31 base address of region*/
    } b;
    u32 w;
} MPU_RBAR_T;

typedef union
{
    struct
    {
        u32 en:1;         /* bit0 region enable */
        u32 size:5;       /* bit1~5 region size */
        u32 reserved1:2;  /* bit6~7 reserved */
        u32 srd:8;        /* bit8~15 sub region */
        u32 attrs:6;      /* bit16~21 attributes */
        u32 reserved2:2;  /* bit22~23 reserved */
        u32 ap:3;         /* bit24~26 ap */
        u32 reserved3:1;  /* bit27 reserved */
        u32 xn:1;         /* bit28 xn */
        u32 reserved4:3;  /* bit29~31 reserved */
    } b;
    u32 w;
} MPU_RASR_T;

struct rg_size {
    u32 idx;
    u64 size;
};

#endif /* __MPU_CM7_H__ */
