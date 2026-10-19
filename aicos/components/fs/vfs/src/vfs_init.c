/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 * DESCRIPTION
 *
 * This file defines the file system initialization interface.
 */

/**---------------------------------------------------------------------------*
 ** Included Files
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x16bdcb
#define LOG_MODULE    AMOD_FS

#include <aic_type.h>
#ifndef WIN32
#include <sysoff.h>
#endif
#include "vfs.h"
#include "fs_test.h"
#include "fs_api.h"

/**---------------------------------------------------------------------------*
 * Pre-processor Definitions
 **---------------------------------------------------------------------------*/
#define VFS_TASK_STACK_SIZE    2048
#define VFS_TASK_PRIORITY      30
#define VFS_QUEUE_NUM          10

#define VFS_NAME              "vfs"

/**---------------------------------------------------------------------------*
 ** Private Data
 **---------------------------------------------------------------------------*/
static aos_thread_id        vfs_thread_id;
static aos_queue_ptr        vfs_queue;
static aos_event_ptr        vfs_event;

/**---------------------------------------------------------------------------*
 ** Private Functions
 **---------------------------------------------------------------------------*/
static void vfs_entry(void *param)
{
    struct vfs_queue_msg fs_msg;
    aos_status status;

    vfs_config();

    while(1) {
        status = aos_recv_from_queue(vfs_queue, &fs_msg, AOS_WAIT_FOREVER);
        AIC_ASSERT(AOS_SUCCESS == status);

        switch (fs_msg.msg) {
            case VFS_MSG_UNINIT:
                vfs_unconfig();
                aos_set_event(vfs_event, VFS_MSG_UNINIT, AOS_OR);
                break;
            default:
                break;
        }
    }
}

/**---------------------------------------------------------------------------*
 ** Public Functions
 **---------------------------------------------------------------------------*/
/**
 * Name: fs_sync_reboot_handler
 *
 * Description:
 *   This is called from the reboot handle to sync the file system.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 */
static void vfs_sync_reboot_handler(void *param)
{
    struct vfs_queue_msg fs_msg;
    aos_status status;
    u32 actual_flags;

    fs_msg.msg = VFS_MSG_UNINIT;
    status = aos_send_to_queue(vfs_queue, &fs_msg, AOS_WAIT_FOREVER);
    AIC_ASSERT(AOS_SUCCESS == status);

    status = aos_get_event(vfs_event, VFS_MSG_UNINIT,
                            AOS_OR_CLEAR, &actual_flags,
                            AOS_WAIT_FOREVER);
    AIC_ASSERT(AOS_SUCCESS == status);
}

/**
 * Name: fs_init
 *
 * Description:
 *   This is called from the OS initialization logic to configure the file
 *   system.
 *
 * Input Parameters:
 *
 * Returned Value:
 *
 */
void vfs_init(void)
{
    vfs_fdtable_init();

    vfs_inode_init();

#ifndef WIN32
    sysoff_register_hook(vfs_sync_reboot_handler, AIC_NULL);
#endif

    vfs_thread_id = aos_create_thread(VFS_NAME, vfs_entry,
                                      AIC_NULL, VFS_TASK_STACK_SIZE,
                                      0, AIC_NULL, VFS_TASK_PRIORITY,
                                      AOS_AUTO_START);
    AOS_ASSERT(AOS_INVALID_THREAD_ID != vfs_thread_id);

    vfs_queue = aos_create_queue(VFS_NAME, sizeof(struct vfs_queue_msg),
                                 VFS_QUEUE_NUM);
    AOS_ASSERT(vfs_queue);

    vfs_event = aos_create_event(VFS_NAME);
    AOS_ASSERT(vfs_event);

#ifdef WIN32
    fs_dev_ram_init();
    fs_dev_win32_init();
    fxffs_init();
    romfs_init();
    fs_dev_file_init();
#ifdef CONFIG_FS_TEST
    fs_test_init();
#elif defined(CONFIG_FS_LIBC_TEST)
    fs_libc_test_init();
#endif
#endif
}

