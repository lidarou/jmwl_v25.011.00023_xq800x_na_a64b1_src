/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is enter of photo directory manager
 */

/*********************
 *      INCLUDES
 *********************/
#include "photo_dir_mgr.h"
#include "photo_object.h"
#include "aic_osa_ex.h"
#include <dirent.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**********************
 *   PUBLIC FUNCTIONS
 **********************/
/**
 * photo_dir_init() - initialize the photo directory according to the
 *                    specified path.
 * @path: path the full path of photo file stored
 * @cb: callback for photo files scanning result notification
 * Return: 0 means success, -1 means failure.
 */
int photo_dir_init(char *path, photo_nofity_callback cb)
{
    DIR *dir = NULL;
    struct dirent *dir_ent = NULL;
    struct file_attr attr;
    struct stat st;
    char path_name[PHOTO_PATH_NAME_LEN];
    int path_len = strlen(path);

    if (!path || !path_len || path_len >= PHOTO_DIR_LEN) {
        app_loge("%s: invalid param\n", __func__);
        return -1;
    }

    strcpy(path_name, path);
    path_name[path_len - 1] = '\0'; /* for gx fs */
    if ((dir = opendir(path_name)) == 0) {
        app_loge("%s: open %s failed\n", __func__, path_name);
        return -1;
    }

    memset(attr.name, '\0', sizeof(attr.name));
    memset(path_name, '\0', sizeof(path_name));
    strcpy(path_name, path);

    do {
        if ((dir_ent = readdir(dir)) == 0)
            break;

        if (dir_ent->d_type == DT_REG) {
            /* regular file */
            int name_len = strlen(dir_ent->d_name);

            if (name_len + path_len >= PHOTO_PATH_NAME_LEN) {
                app_loge("%s: name %s too long\n", __func__, dir_ent->d_name);
                continue;
            }

            strcpy(path_name + path_len, dir_ent->d_name);
            strcpy(attr.name, dir_ent->d_name);

            if (-1 != stat(path_name, &st)) {
                attr.atime = st.st_mtime;
                attr.size = st.st_size;
            } else {
                app_logi("%s: name:%s\n", __func__, path_name);
                continue;
            }

            app_logi("%s: name:%s\n", __func__, path_name);
            if (cb)
                cb(&attr, (void *)path_name);
        } else {
            /* donothing */;
        }
    } while (1);

    closedir(dir);

    return 0;
}

/**
 * photo_dir_deinit() - release the resource in memory for managing
 *                      current directory.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void photo_dir_deinit(void) { return; }

/**
 * photo_delete() - delete a specified photo from the directory.
 * @name: name of file to delete from the directory.
 * @arg2: none.
 * Return:  0 means success, -1 means failure.
 */
int photo_delete(const char *name)
{
    if (!name) {
        app_loge("%s:invalid param\n", __func__);
        return -1;
    }

    if (unlink(name)) {
        app_loge("%s: %s remove failed\n", __func__, name);
        return -1;
    }

    return 0;
}

/**
 * photo_rename() - rename a specified photo file.
 * @name: the old name of photo file to rename.
 * @new_name: the new name of photo file to rename.
 * Return:  0 means success, -1 means failure.
 */
int photo_rename(const char *name, const char *new_name)
{
    if (!name || !new_name) {
        app_loge("%s:invalid param\n", __func__);
        return -1;
    }

    if (rename(name, new_name)) {
        app_loge("%s: file rename failed\n", __func__);
        return -1;
    }

    return 0;
}

/**
 * photo_write() - write a new photo to the directory.
 * @name: name of file to write to the directory.
 * @data: image data.
 * @len: length of data
 * Return:  0 means success, -1 means failure.
 */
int photo_write(const char *name, const u8 *data, u32 len)
{
    FILE *fp = NULL;
    u32 w_cnt = 0;

    if (!name || !data || !len) {
        app_loge("%s:invalid param\n", __func__);
        return -1;
    }

    fp = fopen(name, "wb");
    if (!fp) {
        app_loge("%s: file open failed\n", __func__);
        return -1;
    }

    w_cnt = fwrite(data, 1, len, fp);
    if (w_cnt != len) {
        app_loge("%s: data write failed,%d\n", __func__, w_cnt);
        return -1;
    }

    fclose(fp);

    return 0;
}
