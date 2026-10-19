/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system errno interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14143f

#ifndef WIN32
#include <errno.h>
#endif
#include "vfs.h"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
#ifdef WIN32
int errno;
#endif

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
void vfs_seterrno(int errcode)
{
    errno = errcode < 0 ? -errcode : errcode;
}

int vfs_geterrno(void)
{
    return errno;
}

