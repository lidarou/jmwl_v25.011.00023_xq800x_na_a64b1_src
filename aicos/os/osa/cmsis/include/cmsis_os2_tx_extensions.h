/*
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef CMSIS_OS2_TX_EXTENSIONS_H_
#define CMSIS_OS2_TX_EXTENSIONS_H_

#include "cmsis_os2.h"

#define TX_PRIO_INVALID_FOR_CMSIS 0xFFFFFFFFU

/* Map priority in range [1, 56] to ThreadX priority in range [55, 0]
 * for simplicity we pretend there is a single range even though there is
 * a gap between osPriorityIdle (= 1) which we map to ThreadX priority 55
 * and the next priority, osPriorityLow (= 8), which we map onto ThreadX
 * priority 48.
 * For more detail, see the definition of osPriority_t in cmsis_os2.h
 * and the CMSIS documentation for this enum here:
 * https://www.keil.com/pack/doc/cmsis/RTOS2/html/group__CMSIS__RTOS__ThreadMgmt.html#gad4e3e0971b41f2d17584a8c6837342ec
 */
#define MAP_CMSIS_PRIORITY_TO_THREADX(priority)                                                   \
    ((((priority < osPriorityLow) || (priority > osPriorityISR)) && (priority != osPriorityIdle)) \
         ? TX_PRIO_INVALID_FOR_CMSIS                                                              \
         : osPriorityISR - (unsigned int)priority)

/* Map ThreadX priority in range [0, 55] to osPriority_t in range [56, 1]
 * for simplicity we pretend there is a single range even though there is
 * a gap that means ThreadX priorities from 49 to 54 don't correspond to
 * valid CMSIS-RTOSv2 priorities. */
#define MAP_THREADX_PRIORITY_TO_CMSIS(priority) ((priority > 48) && priority != 55) ? osPriorityError : (56 - priority)

#endif // CMSIS_OS2_TX_EXTENSIONS_H_
