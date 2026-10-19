/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** Copyright (c) 2018 Intel Corporation
 **
 ** SPDX-License-Identifier: Apache-2.0
 **
 ** This file defines the basic posix fcntl operation interface.
 **---------------------------------------------------------------------------*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x141f0c
#define LOG_MODULE    AMOD_AOS

#include <stdio.h>
#include <stdarg.h>

#include <sys/ioctl.h>
#ifdef CONFIG_NX_BSD
#include <nxd_bsd.h>
#endif
#ifdef CONFIG_USE_LWIP
#include <lwipopts.h>
#include <lwip/sockets.h>
#endif
#ifdef CONFIG_FS
#include <fs_api.h>
#endif

/**
 * ioctl:
 *  this function shall perform a variety of control functions on devices.
 *
 * Parameter:
 *  @fildes: input the file description.
 *  @cmd: input the specified command.
 *  @data: input represents the additional information that is needed by this
 *         specific device to perform the requested function.
 *
 * Return:
 *  0 on successful completion. Otherwise, negative integer shall be returned
 *  and errno set to indicate the error.
 */
int ioctl(int fildes, int cmd, ...)
{
    int ret = -1;
    va_list ap;
    void *arg;

    va_start(ap, cmd);
    arg = va_arg(ap, void *);
    va_end(ap);

#ifdef CONFIG_NX_BSD
    if (fildes >= NX_BSD_SOCKFD_START) {
        ret = anet_ioctl(fildes, cmd, (int *)arg);
    } else
#endif
#ifdef CONFIG_USE_LWIP
    if (fildes >= LWIP_SOCKET_OFFSET) {
        ret = lwip_ioctl(fildes, cmd, (int *)arg);
    } else
#endif
    {
#ifdef CONFIG_FS
        ret = fs_ioctl(fildes, cmd, arg);
#endif
    }

    return ret;
}

