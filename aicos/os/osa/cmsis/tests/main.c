/*
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x045088
#define LOG_MODULE    AMOD_AOS

#include "cmsis_rv2.h"

#include <assert.h>
#include <stdio.h>

#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa_ex.h>

static int serial_setup()
{
    return 0;
}

// Use by the CMSIS-RTOSv2 validation suite for printing
int stdout_putchar(int ch)
{
    return ch;
}

int cmsis_test_main(void)
{
    if (serial_setup() != 0) {
        printf("Failed to set up MCU Driver HAL serial\r\n");
        return -1;
    }

    // Run tests (this always returns 0)
    (void)cmsis_rv2();

    return 0;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(cmsis_test_main);
