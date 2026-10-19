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

#ifndef __MPU_CMSTAR_H__
#define __MPU_CMSTAR_H__

/* Reg definitions */
#define MPU_TYPE        0xE000ED90
#define MPU_CTRL        0xE000ED94
#define MPU_RNR         0xE000ED98
#define MPU_RBAR_A0     0xE000ED9C
#define MPU_RLAR_A0     0xE000EDA0
#define MPU_RBAR_A1     0xE000EDA4
#define MPU_RLAR_A1     0xE000EDA8
#define MPU_RBAR_A2     0xE000EDAC
#define MPU_RLAR_A2     0xE000EDB0
#define MPU_RBAR_A3     0xE000EDB4
#define MPU_RLAR_A3     0xE000EDB8
#define MPU_MAIR0       0xE000EDC0
#define MPU_MAIR1       0xE000EDC4

#define MPU_TYPE_NS     0xE002ED90
#define MPU_CTRL_NS     0xE002ED94
#define MPU_RNR_NS      0xE002ED98
#define MPU_RBAR_A0_NS  0xE002ED9C
#define MPU_RLAR_A0_NS  0xE002EDA0
#define MPU_RBAR_A1_NS  0xE002EDA4
#define MPU_RLAR_A1_NS  0xE002EDA8
#define MPU_RBAR_A2_NS  0xE002EDAC
#define MPU_RLAR_A2_NS  0xE002EDB0
#define MPU_RBAR_A3_NS  0xE002EDB4
#define MPU_RLAR_A3_NS  0xE002EDB8
#define MPU_MAIR0_NS    0xE002EDC0
#define MPU_MAIR1_NS    0xE002EDC4

/* Bit definitions */
#define MPU_EN            0x1
#define MPU_RG_EN         0x1
#define MPU_HFNMIENA_EN   0x1
#define MPU_PRIVDEFENA_EN 0x1
/* Note:set same ap for PL0 and PL1 */
#define MPU_AP_RDONLY     0x3
#define MPU_AP_RDWR       0x1
#define MPU_NO_EXEC       0x1
#define MPU_PNO_EXEC      0x1
/* Note:disable shareability */
#define MPU_ATTR0_NGNRNE  0x0
#define MPU_ATTR1_WTNWA   0xAA
#define MPU_ATTR2_WBNWA   0xEE
#define MPU_ATTR3_WBWA    0xFF
#define MPU_ATTR4_NC      0x44
#define MPU_DEV_NGNRNE    0x0
#define MPU_MEM_WTNWA     0x1
#define MPU_MEM_WBNWA     0x2
#define MPU_MEM_WBWA      0x3
#define MPU_MEM_NC        0x4

/* Structure definitions */
typedef union
{
    struct
    {
        u32 separate:1;   /* bit0 separate */
        u32 reserved1:7;  /* bit1~7 reserved */
        u32 dregion:8;    /* bit8~15 number of rg supported */
        u32 reserved2:16; /* bit16~31 reserved */
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
        u32 xn:1;       /* bit0 execute never */
        u32 ap:2;       /* bit1~2 access permissions */
        u32 sh:2;       /* bit3~4 shareability */
        u32 base:27;    /* bit5~31 base address of region*/
    } b;
    u32 w;
} MPU_RBAR_T;

typedef union
{
    struct
    {
        u32 en:1;         /* bit0 region enable */
        u32 attridx:3;    /* bit1~2 attributes index */
        u32 pxn:1;        /* bit3~4 privileged execute never */
        u32 limit:27;     /* bit5~31 end address of region*/
    } b;
    u32 w;
} MPU_RLAR_T;

typedef union
{
    struct
    {
        u32 attr0:8;
        u32 attr1:8;
        u32 attr2:8;
        u32 attr3:8;
    } b;
    u32 w;
} MPU_MAIR0_T;

typedef union
{
    struct
    {
        u32 attr4:8;
        u32 attr5:8;
        u32 attr6:8;
        u32 attr7:8;
    } b;
    u32 w;
} MPU_MAIR1_T;

#endif /* __MPU_CMSTAR_H__ */
