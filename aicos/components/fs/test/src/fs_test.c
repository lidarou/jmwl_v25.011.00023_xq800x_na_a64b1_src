/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the basic fs test operations.
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 ** Dependencies
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x17872d
#define LOG_MODULE    AMOD_FS

#include <aic_errno.h>
#include <aic_type.h>
#include <stdio.h>
#include <errno.h>
#include <aic_init.h>
//#include "fs_cfg.h"
#include "fs_api.h"
//#include "fs_test.h"
#include "vfs_init.h"

/**---------------------------------------------------------------------------*
 ** Macro Definitions
 **---------------------------------------------------------------------------*/
#define FS_TEST_TASK_STACK_SIZE    4096
#define FS_TEST_TASK_PRIORITY      32
#define FS_TEST_QUEUE_NUM          10

#define FS_RAM_TEST_DIR_NAME       "RAM:/dir1/dir2"

#define FS_FLASH_TEST_DIR_NAME     "/dir1/dir2"

#define FS_SD_TEST_DIR_NAME        "D:/dir1/dir2"

#define FS_WIN32_TEST_DIR_NAME_C   "C:/dir1/dir2"
#define FS_WIN32_TEST_DIR_NAME_D   "D:/dir1/dir2"

#define FS_TEST_BUF_LEN             100
#define FS_TEST_BUF                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"

#define FS_TEST_NUM                 10

/**---------------------------------------------------------------------------*
 ** Static Global Variables
 **---------------------------------------------------------------------------*/
static aos_thread_id fs_test_thread_id;

/**---------------------------------------------------------------------------*
 ** Functions
 **---------------------------------------------------------------------------*/

void fs_dir_print(const char *dir_name, struct list_head *list)
{
    struct list_head *pos, *next;
    struct fs_dir_info *dir;

    aicx_info(0x07c7bb, "s", "dir[%s]:\n", dir_name);
    aicx_info(0x0acb0d, "", "name | type | size.\n");

    list_for_each_safe(pos, next, list) {
        dir = list_entry(pos, struct fs_dir_info, node);

        aicx_info(0x01d3ae, "ssd", "%s %s %d \n", dir->dent.d_name,
                  dir->dent.d_type == DT_DIR ? "dir" : "file",
                  dir->dent.d_reclen);

        list_del(&dir->node);
        aos_free(dir);
    }
}

static int loopfind(struct fs_loopfind_info *findinfo)
{
    struct dirent *dent = findinfo->dent;

    if (dent->d_type & DT_DIR) {
        aicx_info(0x14e351, "s", "%s:\n", findinfo->fullpath);
    } else {
        aicx_info(0x090928, "s", "%s\n", findinfo->fullpath);
    }

    return AIC_SUCCESS;
}

/**
 * fs_test_entry - fs test task entry.
 *
 * Parameter:
 * @param: task input param.
 *
 * Return:
 */
static void fs_test_entry(void *param)
{
    char *dst_buf, *src_buf, *file_name, *dev_file_name[FS_TEST_NUM] = {0};
    char tmp_name[NAME_MAX], tmp_name1[NAME_MAX], default_dir[NAME_MAX];
    char *volume_name[FS_TEST_NUM] = {0}, *ptr;
    struct fs_loopfind_info *findinfo;
    u32 i ,len, fs_num = 0, count;
    u64 free_bytes, total_bytes;
    struct list_head list;
    FS_FILE_INFO fileinfo;
    struct dirent *dent;
    struct stat st;
    ssize_t rsize;
    long roffset;
    int fd, ret;
    DIR *dir;

#ifdef CONFIG_FS_DEV_RAM
    volume_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(volume_name[fs_num], "RAM:", strlen("RAM:"));
    dev_file_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(dev_file_name[fs_num], FS_RAM_TEST_DIR_NAME,
               strlen(FS_RAM_TEST_DIR_NAME));
    ++fs_num;
#endif

#ifdef CONFIG_FS_DEV_FLASH
    volume_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(volume_name[fs_num], "C:", strlen("C:"));
    dev_file_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(dev_file_name[fs_num], FS_FLASH_TEST_DIR_NAME,
               strlen(FS_FLASH_TEST_DIR_NAME));
    ++fs_num;
#endif

#ifdef CONFIG_DRIVER_SDMMC
    volume_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(volume_name[fs_num], "D:", strlen("D:"));
    dev_file_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(dev_file_name[fs_num], FS_SD_TEST_DIR_NAME,
               strlen(FS_SD_TEST_DIR_NAME));
    ++fs_num;
#endif

#ifdef CONFIG_FS_DEV_WIN32
    volume_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(volume_name[fs_num], "C:", strlen("C:"));
    dev_file_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(dev_file_name[fs_num], FS_WIN32_TEST_DIR_NAME_C,
               strlen(FS_WIN32_TEST_DIR_NAME_C));
    ++fs_num;

    volume_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(volume_name[fs_num], "D:", strlen("D:"));
    dev_file_name[fs_num] = aos_zalloc(NAME_MAX);
    aos_memcpy(dev_file_name[fs_num], FS_WIN32_TEST_DIR_NAME_D,
               strlen(FS_WIN32_TEST_DIR_NAME_D));
    ++fs_num;
#endif

    dst_buf = aos_zalloc(FS_TEST_BUF_LEN);
    src_buf = aos_zalloc(FS_TEST_BUF_LEN);
    file_name = aos_zalloc(NAME_MAX);
    //volume_info = aos_zalloc(sizeof(FS_VOLUME_INFO));
    aos_memcpy(src_buf, FS_TEST_BUF, strlen(FS_TEST_BUF));

    list_head_init(&list);
    findinfo = aos_zalloc(sizeof(struct fs_loopfind_info));
    errno = 0;

    fd = open("/O/apps/t3_producer", O_RDONLY);
    /* Check the file open status.  */
    AOS_ASSERT(fd >= 0);
    AOS_ASSERT(errno == AIC_SUCCESS);

    rsize = read(fd, dst_buf, FS_TEST_BUF_LEN);
    //AOS_ASSERT(rsize == strlen(dst_buf));
    //AOS_ASSERT(errno == AIC_SUCCESS);

    ret = close(fd);
    AOS_ASSERT(ret == AIC_SUCCESS);
    AOS_ASSERT(errno == AIC_SUCCESS);

    while(1) {
        for (count = 0; count < fs_num; count++) {
            ret = fs_getdiskfreespace(volume_name[count], &free_bytes,
                                      &total_bytes);
            AOS_ASSERT(ret == AIC_SUCCESS);

            if (dev_file_name[count]) {
                len = strlen(dev_file_name[count]);
                ret = mkdir(dev_file_name[count], O_CREAT);
                AOS_ASSERT(ret == 0);
            } else {
                continue;
            }

            i = 0;
            while(i < 10) {
                aos_memset(file_name, 0, NAME_MAX);
                snprintf(file_name, NAME_MAX,
                         "%s/", dev_file_name[count]);
                aos_memcpy(file_name + strlen(file_name), volume_name[count],
                           strlen(volume_name[count]) - 1);
                snprintf(file_name + strlen(file_name),
                         NAME_MAX - strlen(file_name),
                         "_test%d.txt", i);
                /* Open the test file.  */
                fd = open(file_name, O_RDWR | O_CREAT);
                /* Check the file open status.  */
                AOS_ASSERT(fd >= 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                /* Seek to the beginning of the test file.  */
                roffset = lseek(fd, 0, FS_SEEK_BEGIN);
                AOS_ASSERT(roffset == 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                rsize = write(fd, src_buf, strlen(src_buf));
                AOS_ASSERT(rsize == strlen(src_buf));

                /* Seek to the end of the test file.  */
                roffset = lseek(fd, 0, FS_SEEK_BEGIN);
                AOS_ASSERT(roffset == 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                /* Seek to the end of the test file.  */
                roffset = lseek(fd, 0, FS_SEEK_END);
                AOS_ASSERT(roffset == strlen(src_buf));
                AOS_ASSERT(errno == AIC_SUCCESS);

                aos_memset(dst_buf, 0x0, FS_TEST_BUF_LEN);
                rsize = read(fd, dst_buf, FS_TEST_BUF_LEN);
                ret = errno;

                aos_memset(dst_buf, 0x0, FS_TEST_BUF_LEN);
                roffset = lseek(fd, 0, FS_SEEK_BEGIN);
                AOS_ASSERT(roffset == 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                rsize = read(fd, dst_buf, FS_TEST_BUF_LEN);
                AOS_ASSERT(rsize == strlen(dst_buf));
                AOS_ASSERT(errno == AIC_SUCCESS);

                AOS_ASSERT(!memcmp(src_buf, dst_buf, strlen(dst_buf)));

                //ret = fflush(fd);
                //AOS_ASSERT(ret == AIC_SUCCESS);

                /* Close the test file.  */
                ret = close(fd);
                AOS_ASSERT(ret == AIC_SUCCESS);
                fd = -1;
                AOS_ASSERT(errno == AIC_SUCCESS);

                fd = open(file_name, O_RDWR | O_CREAT);
                /* Check the file open status.  */
                AOS_ASSERT(fd >= 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                aos_memset(dst_buf, 0x0, FS_TEST_BUF_LEN);
                roffset = lseek(fd, 0, FS_SEEK_BEGIN);
                AOS_ASSERT(roffset >= 0);
                AOS_ASSERT(errno == AIC_SUCCESS);

                rsize = read(fd, dst_buf, FS_TEST_BUF_LEN);
                AOS_ASSERT(rsize == strlen(dst_buf));
                AOS_ASSERT(errno == AIC_SUCCESS);

                AOS_ASSERT(!memcmp(src_buf, dst_buf, strlen(dst_buf)));

                /* Close the test file.  */
                ret = close(fd);
                AOS_ASSERT(ret == AIC_SUCCESS);
                fd = -1;
                AOS_ASSERT(errno == AIC_SUCCESS);

                //ret = unlink(file_name);
                //AOS_ASSERT(ret == AIC_SUCCESS);
                i++;
            }
            file_name[len] = '\0';
            errno = 0;

#if 0
            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->dir = "";
            findinfo->callback = (u32)loopfind;
            ret = fs_loopfind("", findinfo);
            AIC_ASSERT(ret == AIC_SUCCESS);

            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_ALL;
            findinfo->dir = "*";
            ret = fs_loopfind("*", findinfo);
            AIC_ASSERT(ret == AIC_SUCCESS);

            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_ALL;
            findinfo->dir = "**";
            ret = fs_loopfind("**", findinfo);
            AIC_ASSERT(ret == AIC_SUCCESS);
#endif
            ret = fs_getinfo(volume_name[count], &fileinfo);
            AIC_ASSERT(ret == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s",
                     volume_name[count]);
            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->callback = (u32)loopfind;
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->param = (void *)tmp_name;
            ret = fs_loopfind(tmp_name, findinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);

            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            ret = stat(tmp_name, &st);
            AOS_ASSERT(ret == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir4");
            ret = mkdir(tmp_name, O_CREAT);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s",
                     volume_name[count]);
            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->callback = (u32)loopfind;
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->param = (void *)tmp_name;
            ret = fs_loopfind(tmp_name, findinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);

            ret = fs_lsdir(volume_name[count], &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            fs_dir_print(volume_name[count], &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/");
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            ret = fs_getinfo(volume_name[count], &fileinfo);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1");
            ret = fs_getinfo(tmp_name, &fileinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1/dir2");
            ret = fs_getinfo(tmp_name, &fileinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s",
                     volume_name[count]);
            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->callback = (u32)loopfind;
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->param = (void *)tmp_name;
            ret = fs_loopfind(tmp_name, findinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1/dir2/");
            aos_memcpy(tmp_name + strlen(tmp_name), volume_name[count],
                       strlen(volume_name[count]) - 1);
            snprintf(tmp_name + strlen(tmp_name),
                     NAME_MAX - strlen(tmp_name), "%s",
                     "_test0.txt");
            ret = fs_getinfo(tmp_name, &fileinfo);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = fs_getdiskfreespace(volume_name[count], &free_bytes,
                                      &total_bytes);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = fs_lsdir(volume_name[count], &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(volume_name[count], &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1");
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/dir1/dir2/",
                     volume_name[count]);
            aos_memcpy(tmp_name + strlen(tmp_name), volume_name[count],
                       strlen(volume_name[count]) - 1);
            snprintf(tmp_name + strlen(tmp_name),
                     NAME_MAX - strlen(tmp_name), "%s",
                     "_test0.txt");

            aos_memset(tmp_name1, 0x0, NAME_MAX);
            snprintf(tmp_name1, NAME_MAX, "%s/dir1/dir2/ftest456.txt", volume_name[count]);
            ret = fs_rename(tmp_name, tmp_name1);
            AOS_ASSERT(ret == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1/dir2");
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/%s", volume_name[count],
                     "fs1.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/");
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1");
            ret = chdir(tmp_name);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s", "fs1.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1");
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1/dir2");
            dir = opendir(tmp_name);
            AOS_ASSERT(dir != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            roffset = telldir(dir);
            AOS_ASSERT(errno == AIC_SUCCESS);

            seekdir(dir, 5);
            AOS_ASSERT(errno == AIC_SUCCESS);

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            roffset = telldir(dir);
            AOS_ASSERT(errno == AIC_SUCCESS);

            rewinddir(dir);
            AOS_ASSERT(errno == AIC_SUCCESS);

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            roffset = telldir(dir);
            AOS_ASSERT(errno == AIC_SUCCESS);

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/%s", volume_name[count],
                     "fs1.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir1/dir2/fs2.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            dent = readdir(dir);
            AOS_ASSERT(dent != NULL);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = closedir(dir);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/fs1.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/fs2.txt");
            fd = open(tmp_name, FS_O_RDWR | FS_O_CREATE);
            /* Check the file open status.  */
            AOS_ASSERT(fd >= 0);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = close(fd);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fd = -1;

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/dir4",
                     volume_name[count]);
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(tmp_name1, 0x0, NAME_MAX);
            snprintf(tmp_name1, NAME_MAX, "%s/dir1",
                     volume_name[count]);
            ret = fs_move(tmp_name1, tmp_name);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/dir4/dir2",
                     volume_name[count]);
            ret = fs_lsdir(tmp_name, &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            fs_dir_print(tmp_name, &list);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s/**",
                     volume_name[count]);
            aos_memset(findinfo, 0x0, sizeof(struct fs_loopfind_info));
            findinfo->findtype = FS_FIND_ALLDIR;
            findinfo->deltype = FS_DEL_FILE;
            findinfo->dir = tmp_name;
            findinfo->callback = (u32)loopfind;
            ret = fs_loopfind(tmp_name, findinfo);
            AIC_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir4/dir2");
            ret = unlink(tmp_name);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(tmp_name, 0x0, NAME_MAX);
            snprintf(tmp_name, NAME_MAX, "%s%s", volume_name[count],
                     "/dir4");
            ret = unlink(tmp_name);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = fs_lsdir(volume_name[count], &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(volume_name[count], &list);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = mkfs("fxffs", volume_name[count]);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);

            ret = fs_lsdir(volume_name[count], &list);
            AOS_ASSERT(ret == AIC_SUCCESS);
            AOS_ASSERT(errno == AIC_SUCCESS);
            fs_dir_print(volume_name[count], &list);

            aos_memset(default_dir, 0x0, NAME_MAX);
            ptr = getcwd(default_dir, NAME_MAX);
            AOS_ASSERT(ptr);
            AOS_ASSERT(errno == AIC_SUCCESS);
        }
        aos_sleep(500);
    }
}

void fs_mount_cb(struct fs_mount_info *mntinfo, void *param)
{
    fs_unregister_mount_hook(fs_mount_cb);
}

/**
 * fs_test_init - fs test init.
 *
 * Parameter:
 *
 * Return:
 */
int fs_test_init(void)
{
    fs_register_mount_hook(fs_mount_cb, NULL);
    fs_test_thread_id = aos_create_thread("fs_test_task", fs_test_entry,
                                          AIC_NULL, FS_TEST_TASK_STACK_SIZE,
                                          0, AIC_NULL, FS_TEST_TASK_PRIORITY,
                                          AOS_AUTO_START);
    AOS_ASSERT(AOS_INVALID_THREAD_ID != fs_test_thread_id);

    return AIC_SUCCESS;
}

AIC_NORMAL_COMP_P1_INIT_EXPORT(fs_test_init);

#if defined(CONFIG_FX_UTF8_SUPPORT) && defined (CONFIG_VFS_UTF8_DEBUG)
#include <aic_console.h>
int vfs_test(int argc, char **argv)
{
    UNUSED_PARAM(argc);
    u32 id = strtoul(argv[0], NULL, 10);
    CONS_OUT("vfs_test %d\n", id);

    char *path;
    int fd;

    switch (id) {
    case 0:
        path = "/D/新建文件夹/ultralonglonglonglonglonglongname/b.txt";
        fd = open((const char *)path, O_RDONLY);
        CONS_OUT("open <%s> : %d\n", path, fd);
        break;

    case 1:
        path = "/D/新建文件夹/short/e.txt";
        fd = open((const char *)path, O_RDONLY);
        CONS_OUT("open <%s> : %d\n", path, fd);
        break;

    case 2:
        path = "/D/新建文件夹/ultralonglonglonglonglonglongname/哈哈哈哈哈哈哈哈.txt";
        fd = open((const char *)path, O_RDONLY| O_CREAT);
        CONS_OUT("open&create <%s> : %d\n", path, fd);
        if (fd > 0) {
            close(fd);
        }
        break;

    case 3:
        path = "/D/新建文件夹/中文.txt";
        fd = open((const char *)path, O_RDONLY | O_CREAT);
        CONS_OUT("open&create simple <%s> : %d\n", path, fd);
        break;

    case 4:
        path = "/D/新建文件夹/一个文件夹/文本文件.txt";
        fd = open(path, O_RDWR | O_CREAT);
        CONS_OUT("open&create eng <%s> : %d\n", path, fd);
        if (fd > 0) {
            close(fd);
        }
        break;

    case 10:
        path = "/D/hhh";
        fd = mkdir((const char *)path, O_RDWR | O_CREAT);
        CONS_OUT("mkdir <%s> : %d\n", path, fd);
        break;

    case 11:
        path = "/D/hhh/一个文件夹";
        fd = mkdir((const char *)path, O_RDWR | O_CREAT);
        CONS_OUT("mkdir <%s> : %d\n", path, fd);
        break;

    case 12:
        path = "/D/hhh/一个文件夹/二个文件夹";
        fd = mkdir((const char *)path, O_RDWR | O_CREAT);
        CONS_OUT("mkdir <%s> : %d\n", path, fd);
        break;

    case 13:
        path = "/D/新建文件夹/一个文件夹";
        fd = mkdir((const char *)path, O_RDWR | O_CREAT);
        CONS_OUT("mkdir <%s> : %d\n", path, fd);
        break;

    case 20:
        path = "/D/新建文件夹/中文again.txt";
        fd = open((const char *)path, O_RDONLY | O_CREAT);
        CONS_OUT("open&create again <%s> : %d\n", path, fd);
        if (fd > 0) {
            close(fd);
        }
        break;

    case 21:
        path = "/D/新建文件夹/中文again.txt";
        char *new_path = "/D/新建文件夹/again中文.txt";
        fd = rename(path, new_path);
        CONS_OUT("rename <%s> to <%s>: %d\n", path, new_path, fd);
        break;

    case 22:
        path = "/D/新建文件夹/一个文件夹/文本文件.txt";
        char *new_path_2 = "/D/ultralonglonglonglonglonglongname/renamed.txt";
        fd = rename(path, new_path_2);
        CONS_OUT("rename <%s> to <%s>: %d\n", path, new_path_2, fd);
        break;

    case 23:
        path = "/D/hhh/一个文件夹/二个文件夹";
        char *new_path_3 = "/D/ultralonglonglonglonglonglongname/文件夹重命名";
        fd = rename(path, new_path_3);
        CONS_OUT("rename <%s> to <%s>: %d\n", path, new_path_3, fd);
        break;

    case 30:
        path = "/D/新建文件夹/一个文件夹";
        fd = rmdir(path);
        CONS_OUT("rmdir <%s>: %d\n", path, fd);
        break;

    case 31:
        path = "/D/新建文件夹/一个文件夹/文本文件.txt";
        fd = unlink(path);
        CONS_OUT("unlink <%s>: %d\n", path, fd);
        break;

    case 40:
        path = "/D/新建文件夹";
        DIR *dir = opendir(path);
        CONS_OUT("opendir <%s>: 0x%x\n", path, (uintptr_t)dir);
        if (dir) {
            struct dirent *dirent = readdir(dir);
            CONS_OUT("readdir: %s\n", dirent->d_name);
            closedir(dir);
        }
        break;

    default:
        CONS_OUT("Invalid id: %d!!!!", id);
        break;
    }

    return 0;
}
CONSOLE_EXPORT_CMD(vfs, vfs_test, vfs <id>);
#endif
