/* umdevxs_chrdev.c
 *
 * Character Device interface for the Linux UMDevXS driver.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : DDK-130_bsd                                              */
/*   Version       : 3.1                                                      */
/*   Configuration : DDK_EIP130_BSD                                           */
/*                                                                            */
/*   Date          : 2022-Feb-14                                              */
/*                                                                            */
/* Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright          */
/* notice, this list of conditions and the following disclaimer.              */
/*                                                                            */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/* notice, this list of conditions and the following disclaimer in the        */
/* documentation and/or other materials provided with the distribution.       */
/*                                                                            */
/* 3. Neither the name of the copyright holder nor the names of its           */
/* contributors may be used to endorse or promote products derived from       */
/* this software without specific prior written permission.                   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR      */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT           */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,      */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.       */
/* -------------------------------------------------------------------------- */

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// UMDevXS Character Device API
#include "umdevxs_chrdev.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_umdevxs.h"              // config options

#include "basic_defs.h"             // IDENTIFIER_NOT_USED
#include "umdevxs_internal.h"
#include "umdevxs_cmd.h"            // UMDevXS_CmdRsp_t

#ifndef UMDEVXS_REMOVE_DEVICE
#include "umdevxs_device.h"
#endif

#include <linux/fs.h>
#include <linux/mm.h>               // remap_pfn_range, PAGE_SHIFT
#include <linux/module.h>           // THIS_MODULE
#include <linux/version.h>          // LINUX_VERSION_CODE, KERNEL_VERSION
#include <linux/uaccess.h>          // copy_to/from_user, access_ok
#include <linux/errno.h>            // EIO


/*----------------------------------------------------------------------------
 * Definitions and macros
 */
#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif /* KERNEL_VERSION */



/*----------------------------------------------------------------------------
 * Local variables
 */

// character device related
static const char UMDevXS_ChrDev_module_name[] = UMDEVXS_MODULENAME"_c";
static int UMDevXS_ChrDev_major_nr = 0;

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
#include "shdevxs_kernel.h" // UMDevXS_KernelSupport_HandleCmd
#endif

static UMDevXS_HandleCmdFunction_t HandleCmdFunc;


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_fop_open
 */
static int
UMDevXS_ChrDev_fop_open(
        struct inode * inode,
        struct file * file_p)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: file_p=%p\n", __func__, file_p);

    IDENTIFIER_NOT_USED(inode);
    IDENTIFIER_NOT_USED(file_p);

    return 0;       // 0 = success
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_fop_release
 */
static int
UMDevXS_ChrDev_fop_release(
        struct inode * inode,
        struct file * file_p)
{
    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: file_p=%p\n", __func__, file_p);

#ifdef UMDEVXS_ENABLE_DEVICE_LOCK
    UMDevXS_Device_Unlock(file_p);
#endif

#ifndef UMDEVXS_REMOVE_SMBUF
    UMDevXS_SMBuf_CleanUp(file_p);
#endif

#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
    SHDevXS_Cleanup(file_p);
#endif

    IDENTIFIER_NOT_USED(inode);
    IDENTIFIER_NOT_USED(file_p);

    return 0;   // 0 = success
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_fop_mmap
 */
static int
UMDevXS_ChrDev_fop_mmap(
        struct file * file_p,
        struct vm_area_struct * vma_p)
{
    int Handle = vma_p->vm_pgoff;
    int Class = UMDevXS_Handle_GetClass(Handle);
#if !defined(UMDEVXS_REMOVE_DEVICE) || !defined(UMDEVXS_REMOVE_SMBUF)
    int Index = UMDevXS_Handle_GetIndex(Handle);
    unsigned int Length = vma_p->vm_end - vma_p->vm_start;

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: file_p=%p, Handle 0x%X [Class=%d, Index=0x%X] Length=0x%X\n",
             __func__, file_p, Handle, Class, Index, Length);
#endif

    // get the handle class
    switch(Class)
    {
#ifndef UMDEVXS_REMOVE_DEVICE
        case UMDEVXS_HANDLECLASS_DEVICE:
            {
                int res;

#ifdef UMDEVXS_ENABLE_DEVICE_LOCK
                res = UMDevXS_Device_LockRangeIndex(Index, Length, file_p);
                if (res != 0)
                {
                    LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: failed to lock device\n",
                             __func__);
                    return -EIO;
                }
#endif

                res = UMDevXS_Device_Map(Index, Length, vma_p);
                if (res == 0)
                {
                    // mapping successful
                    return 0;       // ## RETURN ##
                }

                LOG_INFO(UMDEVXS_LOG_PREFIX "%s: UMDevXS_Device_Map returned %d\n",
                         __func__, res);
            }
            break;
#endif

#ifndef UMDEVXS_REMOVE_SMBUF
        case UMDEVXS_HANDLECLASS_SMBUF:
            {
                int res;

#ifdef UMDEVXS_ENABLE_BUFFER_APPCHECK
                if (file_p !=  UMDevXS_SMBuf_GetAppID(Index))
                {
                    LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: Invalid  buffer\n", __func__);
                    return -EIO;
                }
#endif

                res = UMDevXS_SMBuf_Map(Index, Length, vma_p);
                if (res == 0)
                {
                    // mapping successful
                    return 0;       // ## RETURN ##
                }

                LOG_INFO(UMDEVXS_LOG_PREFIX "%s: UMDevXS_SMBuf_Map returned %d\n",
                         __func__, res);
            }
            break;
#endif

        //case UMDEVXS_HANDLECLASS_DMABUF:
        default:
            LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Invalid handle (0x%x, %d, %d)\n",
                     __func__, Handle, Class, UMDevXS_Handle_GetIndex(Handle));
            return -EIO;
    }

    IDENTIFIER_NOT_USED(file_p);

    return -EIO;
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_fop_read
 *
 * Read interface is used to wait for interrupts.
 */
static ssize_t
UMDevXS_ChrDev_fop_read(
        struct file * file_p,
        char __user * buf,
        size_t count,
        loff_t * ppos)
{
    int res;

    if (file_p == NULL || buf == NULL)
    {
        return -EIO;
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX "%s: file_p=%p, count=%lu, ppos=%p\n",
             __func__, file_p, (unsigned long)count, ppos);

#ifndef UMDEVXS_REMOVE_INTERRUPT
    res = UMDevXS_Interrupt_WaitWithTimeout(count & UMDEVXS_INTERRUPT_TIMEOUT_MASK,
                                            count >> UMDEVXS_INTERRUPT_INT_ID_OFFSET);
#else
    res = -1;
#endif
    if (res < 0)
    {
        return -EIO;
    }

    IDENTIFIER_NOT_USED(file_p);
    IDENTIFIER_NOT_USED(buf);
    IDENTIFIER_NOT_USED(count);
    IDENTIFIER_NOT_USED(ppos);

    return res;
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_fop_write
 *
 * Write interface is used for Cmd/Rsp passing.
 */
static ssize_t
UMDevXS_ChrDev_fop_write(
        struct file * file_p,
        const char __user * buf,
        size_t count,
        loff_t * ppos)
{
    if (file_p == NULL || buf == NULL)
    {
        return -EIO;
    }

    if (count != sizeof(UMDevXS_CmdRsp_t))
    {
        LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Expected count=%d\n",
                 __func__, (int)sizeof(UMDevXS_CmdRsp_t));
        return -EINVAL;     // ## RETURN ##
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
    if (!access_ok(VERIFY_WRITE, buf, sizeof(UMDevXS_CmdRsp_t)))
#else
    if (!access_ok(buf, sizeof(UMDevXS_CmdRsp_t)))
#endif
    {
        LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Failed on access_ok\n", __func__);
        return -EINVAL;     // ## RETURN ##
    }

    {
        UMDevXS_CmdRsp_t CmdRsp;
#ifndef LOG_INFO_ENABLED
#ifdef UMDEVXS_CHRDEV_LOG_ERRORS
        UMDevXS_CmdRsp_t CmdRsp_CopyForErrorReport;
#endif /* UMDEVXS_CHRDEV_LOG_ERRORS */
#endif /* !LOG_INFO */

        // copy the CmdRsp from application space into kernel space
        if (copy_from_user(&CmdRsp, buf, sizeof(UMDevXS_CmdRsp_t)) != 0)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Failed on copy_from_user\n",
                     __func__);
            return -EINVAL;     // ## RETURN ##
        }

        if (CmdRsp.Magic != UMDEVXS_CMDRSP_MAGIC)
        {
            LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Failed on Magic check\n",
                     __func__);
            return -EINVAL;     // ## RETURN ##
        }

#ifndef LOG_INFO_ENABLED
#ifdef UMDEVXS_CHRDEV_LOG_ERRORS
        memcpy(&CmdRsp_CopyForErrorReport, &CmdRsp, sizeof(UMDevXS_CmdRsp_t));
#endif /* UMDEVXS_CHRDEV_LOG_ERRORS */
#endif /* !LOG_INFO */

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "Cmd: Opcode=%d, Handle=0x%x, uint1/2/3=%u/%u/%u, ptr1=%p\n",
                 CmdRsp.Opcode, CmdRsp.Handle,
                 CmdRsp.uint1, CmdRsp.uint2, CmdRsp.uint3, CmdRsp.ptr1);

        // handle the request
        switch(CmdRsp.Opcode)
        {
#ifndef UMDEVXS_REMOVE_SMBUF
            case UMDEVXS_OPCODE_SMBUF_ALLOC:
            case UMDEVXS_OPCODE_SMBUF_REGISTER:
            case UMDEVXS_OPCODE_SMBUF_FREE:
            case UMDEVXS_OPCODE_SMBUF_ATTACH:
            case UMDEVXS_OPCODE_SMBUF_DETACH:
            case UMDEVXS_OPCODE_SMBUF_GETBUFINFO:
            case UMDEVXS_OPCODE_SMBUF_SETBUFINFO:
            case UMDEVXS_OPCODE_SMBUF_COMMIT:
            case UMDEVXS_OPCODE_SMBUF_REFRESH:
                UMDevXS_SMBuf_HandleCmd(file_p, &CmdRsp);
                break;
#endif
#ifndef UMDEVXS_REMOVE_DEVICE
            case UMDEVXS_OPCODE_DEVICE_FIND:
                UMDevXS_Device_HandleCmd_Find(&CmdRsp);
                break;

            case UMDEVXS_OPCODE_DEVICE_FINDRANGE:
                UMDevXS_Device_HandleCmd_FindRange(&CmdRsp);
                break;

            case UMDEVXS_OPCODE_DEVICE_SETPRIV:
                UMDevXS_Device_HandleCmd_SetPrivileged(file_p, &CmdRsp);
                break;

            case UMDEVXS_OPCODE_DEVICE_ENUM:
                UMDevXS_Device_HandleCmd_Enum(&CmdRsp);
                break;

#ifndef UMDEVXS_REMOVE_DEVICE_PCICFG
            case UMDEVXS_OPCODE_DEVICE_PCICFG_READ32:
                UMDevXS_PCIDev_HandleCmd_Read32(&CmdRsp);
                break;

            case UMDEVXS_OPCODE_DEVICE_PCICFG_WRITE32:
                UMDevXS_PCIDev_HandleCmd_Write32(&CmdRsp);
                break;
#endif // UMDEVXS_REMOVE_DEVICE_PCICFG
#endif
            default:
                if (HandleCmdFunc)
                {
                    int rc = HandleCmdFunc(file_p, &CmdRsp);
                    if (rc < 0)
                    {
                        return rc;
                    }
                }
#ifdef UMDEVXS_ENABLE_KERNEL_SUPPORT
                SHDevXS_HandleCmd(file_p, &CmdRsp);
#else
                else
                {
                    LOG_CRIT(UMDEVXS_LOG_PREFIX "Cmd (%d) not supported\n",
                             CmdRsp.Opcode);
                    CmdRsp.Error = 1;
                }
#endif
                break;
        }

        // copy the result back to the application space
        {
            // get rid of const attr
            char __user * writebuf_p = (char __user *)buf;

            if (copy_to_user(writebuf_p, &CmdRsp, sizeof(UMDevXS_CmdRsp_t)) != 0)
            {
                LOG_INFO(UMDEVXS_LOG_PREFIX "%s: Failed on copy_to_user\n",
                         __func__);
                return -EINVAL;     // ## RETURN ##
            }
        }

        LOG_INFO(UMDEVXS_LOG_PREFIX
                 "Rsp: Error=%d, Handle=0x%x, uint1/2/3=%u/%u/%u, ptr1=%p\n",
                 CmdRsp.Error, CmdRsp.Handle,
                 CmdRsp.uint1, CmdRsp.uint2, CmdRsp.uint3, CmdRsp.ptr1);
#ifndef LOG_INFO_ENABLED
// (no duplicate logging when INFO logging is enabled)
#ifdef UMDEVXS_CHRDEV_LOG_ERRORS
        if (CmdRsp.Error)
        {
            LOG_WARN(UMDEVXS_LOG_PREFIX
                     "Cmd: Opcode=%d, Handle=0x%x, uint1/2/3=%u/%u/%u, ptr1=%p\n",
                     CmdRsp_CopyForErrorReport.Opcode,
                     CmdRsp_CopyForErrorReport.Handle,
                     CmdRsp_CopyForErrorReport.uint1,
                     CmdRsp_CopyForErrorReport.uint2,
                     CmdRsp_CopyForErrorReport.uint3,
                     CmdRsp_CopyForErrorReport.ptr1);
            LOG_WARN(UMDEVXS_LOG_PREFIX
                     "Rsp: Error=%d, Handle=0x%x, uint1/2/3=%u/%u/%u, ptr1=%p\n",
                     CmdRsp.Error,
                     CmdRsp.Handle,
                     CmdRsp.uint1,
                     CmdRsp.uint2,
                     CmdRsp.uint3,
                     CmdRsp.ptr1);
        }
#endif /* UMDEVXS_CHRDEV_LOG_ERRORS */
#endif /* !LOG_INFO */
    }

    IDENTIFIER_NOT_USED(ppos);

    // extremely important to return 'count' upon success
    // otherwise we can get called forever
    return count;
}


/*----------------------------------------------------------------------------
 * File Operations structure
 *
 * Used by the character device.
 * Contains pointers to the handler functions.
 */
static struct file_operations UMDevXS_ChrDev_fops =
{
#ifdef _MSC_VER
    // microsoft compiler does not support partial initializers
    // NOTE: struct must have fields in this order
    UMDevXS_ChrDev_fop_read,
    UMDevXS_ChrDev_fop_write,
    UMDevXS_ChrDev_fop_mmap,
    UMDevXS_ChrDev_fop_open,
    UMDevXS_ChrDev_fop_release
#else
    .owner = THIS_MODULE,
    .open = UMDevXS_ChrDev_fop_open,
    .release = UMDevXS_ChrDev_fop_release,

    // mmap support is used to map a buffer into user space
    .mmap = UMDevXS_ChrDev_fop_mmap,

    // read is used in a blocking fashion to wait for interrupts
    .read = UMDevXS_ChrDev_fop_read,

    // write is used for cmd/rsp passing
    .write = UMDevXS_ChrDev_fop_write
#endif
};


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_HandleCmdFunc_Set
 */
void
UMDevXS_ChrDev_HandleCmdFunc_Set(UMDevXS_HandleCmdFunction_t Func)
{
    HandleCmdFunc = Func;
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_Init
 */
int
UMDevXS_ChrDev_Init(void)
{
    int Status;

    // register the character device side
    Status = register_chrdev(UMDevXS_ChrDev_major_nr,
                             UMDevXS_ChrDev_module_name,
                             &UMDevXS_ChrDev_fops);
    if (Status < 0)
    {
        LOG_CRIT(UMDEVXS_LOG_PREFIX "%s: Failed to register the chrdev (%d)\n",
                 __func__, Status);
        return Status;
    }

    if (UMDevXS_ChrDev_major_nr == 0)
    {
        UMDevXS_ChrDev_major_nr = Status;
    }

    LOG_INFO(UMDEVXS_LOG_PREFIX "created device '%s', major=%d\n",
             UMDevXS_ChrDev_module_name, UMDevXS_ChrDev_major_nr);

#ifdef UMDEVXS_ENABLE_DEVICE_LOCK
    UMDevXS_DeviceLock_Init();
#endif

    return 0;   // 0 = success
}


/*----------------------------------------------------------------------------
 * UMDevXS_ChrDev_UnInit
 */
void
UMDevXS_ChrDev_UnInit(void)
{
    if (UMDevXS_ChrDev_major_nr)
    {
        unregister_chrdev(UMDevXS_ChrDev_major_nr,
                          UMDevXS_ChrDev_module_name);

        LOG_INFO(UMDEVXS_LOG_PREFIX "destroyed device '%s', major=%d\n",
                 UMDevXS_ChrDev_module_name, UMDevXS_ChrDev_major_nr);

        UMDevXS_ChrDev_major_nr = 0;
    }
}


/* end of file umdevxs_chrdev.c */
