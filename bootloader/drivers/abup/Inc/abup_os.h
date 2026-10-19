/*****************************************************************************
* Copyright (c) 2018 ABUP.Co.Ltd. All rights reserved.
* File name: abup_os.h
* Description:
* Author: Ray Shen
* Version: v1.0
* Date: 20181101
*****************************************************************************/
#ifndef _ABUP_OS_H_
#define _ABUP_OS_H_

#if defined(ABUP_RTTHREAD)
#include "rtconfig.h"
#endif
#define ABUP_BOOTLOADER
#if defined(ABUP_TLV_SERVER)||defined(ABUP_OPENFOTA)
#define ABUP_TIMERS
#define ABUP_QUEUE_MSG
#endif
//1 debug, otherwise user
#define ABUP_BOOTLOADER_DEBUG      0
#define ABUP_BOOTLOADER_VERSION    "1.0"
//#define ABUP_BOOTLOADER_DEBUG_ALL   0
#endif

