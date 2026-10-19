/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the common APIs for fs operation.
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "app_fs.h"
#include <aic_errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifndef WIN32
#include <aic_osa_ex.h>
#endif

/*******************************************
 *              DEFINES
 ******************************************/
#define APP_FS_COPY_BUF_SIZE 1024

/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * struct app_fs_sd_mount_cb - sd mount callback listener
 *@listener: cb function
 *@user_data: user data
 */
struct app_fs_sd_mount_cb {
    void (*listener)(bool mount, void *user_data);
    void *user_data;
};

/**
 * struct app_fs_dev_status - device status
 * @dev_id: device id
 * @volume: volume path, like "S:"
 * @is_mounted: volume is mounted or not
 */
struct app_fs_dev_status {
    char volume[APP_FS_VOLUME_LEN];
    bool is_mounted;
};

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
static lv_ll_t g_app_fs_sd_mount_cb = {0};
#ifndef WIN32
static aos_mutex_ptr g_app_fs_mutex = NULL;
static struct app_fs_dev_status g_app_fs_dev_status[APP_FS_DEV_ID_CNT] = {0};
#endif

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
#ifndef WIN32
/**
 * app_fs_lock - create a mutex to lock
 *
 * Return: NA
 */
static void app_fs_lock(void)
{
    if (g_app_fs_mutex && aos_is_user_thread_context()) {
        aos_get_mutex(g_app_fs_mutex, AOS_WAIT_FOREVER);
    }
}

/**
 * app_fs_unlock - put a mutex to lock
 *
 * Return: NA
 */
static void app_fs_unlock(void)
{
    if (g_app_fs_mutex && aos_is_user_thread_context()) {
        aos_put_mutex(g_app_fs_mutex);
    }
}

/**
 * notify_app_task - notify app thread to handle
 *
 * Return: NA
 */
static void notify_app_task(void)
{
    uint32_t app_msg = APP_FS_NOTIFY_SD_MOUNT_MSG;
    aos_status ret;

    extern aos_queue_ptr app_queue;
    ret = aos_send_to_queue(app_queue, &app_msg, AOS_NO_WAIT);
    aic_assert(ret == AOS_SUCCESS);
}
/**
 * fs_mount_hooker - hooker of fs mount
 * @volume_info: [IN] pointer to volume info
 * @param: user data
 *
 * Return: NA
 */
static void fs_mount_hooker(FS_VOLUME_INFO *volume_info, void *param)
{
    app_fs_lock();
    if (FS_DEV_SD == volume_info->devtype) {
        g_app_fs_dev_status[APP_FS_DEV_ID_SD].is_mounted =
            (bool)volume_info->mount;
        strcpy(g_app_fs_dev_status[APP_FS_DEV_ID_SD].volume,
               volume_info->volume_name);
        /* notify app task to handle */
        notify_app_task();
    } else if (FS_DEV_FLASH == volume_info->devtype) {
        g_app_fs_dev_status[APP_FS_DEV_ID_LOCAL].is_mounted =
            (bool)volume_info->mount;
        strcpy(g_app_fs_dev_status[APP_FS_DEV_ID_LOCAL].volume,
               volume_info->volume_name);
    } else {
        app_logi("fs_mount_hooker: receive a non-supported device type. ");
    }
    app_fs_unlock();
}
#endif
/**
 * compare_list_item - add an item to the tail of the scan list
 * @item_l: [IN] item left
 * @item_r: [IN] item right
 *
 * Return: 0 for equal, a negative value means that the left side is less than
 * the right side, and a positive value is the opposite
 */
static int compare_list_item(struct app_fs_scan_list_item *item_l,
                             struct app_fs_scan_list_item *item_r)
{
    int result = 0;

    if (item_l->type == item_r->type) {
        result = strcasecmp(item_l->name, item_r->name);
    } else {
        /* type of folder is smaller than that of file */
        result = (int)item_l->type - (int)item_r->type;
    }

    return result;
}

/**
 * add_to_scan_list - add an item to the right position as per value
 * @list: [IN] scan list
 * @volume_name: [IN] the item to add
 *
 * Return: true for success, false for failure.
 */
static bool add_to_scan_list(struct app_fs_scan_list_entry *list,
                             struct app_fs_scan_list_item *item)
{
    bool result = true;

    if (NULL == list || NULL == item) {
        return false;
    }
    if (NULL == list->first) {
        list->first = item;
        list->last = item;
    } else {
        if (compare_list_item(item, list->first) < 0) {
            item->next = list->first;
            list->first = item;
        } else {
            struct app_fs_scan_list_item *p = list->first;
            bool inserted = false;
            while (p != NULL && p->next != NULL) {
                if (compare_list_item(item, p->next) < 0) {
                    item->next = p->next;
                    p->next = item;
                    inserted = true;
                    break;
                }
                p = p->next;
            }
            if (!inserted) {
                list->last->next = item;
                list->last = item;
            }
        }
    }
    list->total++;

    return result;
}

/**
 * get_real_path - get the real full path name
 * @file_name: [IN] file name
 *
 * Return: the full path name of the file. and the return memory
 * should be freed manually later.
 *
 * Note: internal use only. The implicit condition is
 * that the file is in the current working dir
 */
static char *get_real_path(const char *file_name)
{
    char *result = NULL;

    if (file_name != NULL) {
        char *dir = NULL;
        result = (char *)app_fs_malloc(APP_FS_MAX_PATH);
        if (result != NULL) {
            dir = getcwd(result, APP_FS_MAX_PATH);
        }

        if (dir != NULL) {
            int len = 0;

            len = strlen(dir) + strlen(file_name) + 1;
            if (len <= APP_FS_MAX_PATH - 1) {
                strcat(dir, APP_FS_FILE_SEPERATOR);
                strcat(dir, file_name);
            } else {
                if (result) {
                    app_fs_free(result);
                    result = NULL;
                }
            }
        }
    }

    return result;
}

/**
 * set_file_name - compose the file name as per is_recur
 * @list_item: [OUT] list item
 * @is_recur: [IN] the condition to compose
 * @src_name: [IN] the name of source file
 *
 * Return: NA
 *
 * Note: internal use only.
 * 1. If is_recur is true, compose the full path name
 * otherwise, just use the src_name only.
 * 2. The list item might allocate a new memory be accommodate the text length
 */
static void set_file_name(struct app_fs_scan_list_item **list_item,
                          bool is_recur, const char *src_name)
{
    if (is_recur) {
        char *real_path = get_real_path(src_name);
        if (real_path != NULL) {
            int len = strlen(real_path);
            struct app_fs_scan_list_item *old = *list_item;

            if (len > APP_FS_SHORT_PATH - 1) {
                /* reallocate a new memory */
                *list_item = (struct app_fs_scan_list_item *)app_fs_malloc(
                    sizeof(struct app_fs_scan_list_item) + len -
                    APP_FS_SHORT_PATH + 1);
                memcpy(*list_item, old, sizeof(struct app_fs_scan_list_item));
                app_fs_free(old);
            }
            strcpy((*list_item)->name, real_path);
            app_fs_free(real_path);
        } else {
            /* ignore this file*/
            (*list_item)->name[0] = 0;
        }
    } else {
        int len = strlen(src_name);
        struct app_fs_scan_list_item *old = *list_item;

        if (len > APP_FS_SHORT_PATH - 1) {
            /* reallocate a new memory */
            *list_item = (struct app_fs_scan_list_item *)app_fs_malloc(
                sizeof(struct app_fs_scan_list_item) + len - APP_FS_SHORT_PATH +
                1);
            memcpy(*list_item, old, sizeof(struct app_fs_scan_list_item));
            app_fs_free(old);
        }
        strcpy((*list_item)->name, src_name);
    }
}

/**
 * copy_folder - copy a folder
 * @src: [IN] full path of src folder
 * @dest: [IN] full path of dest folder
 * @is_move: [IN] true to delete the source folder
 *
 * Return: true for success, false for failure
 */
static bool copy_folder(const char *src, const char *dest, bool is_move)
{
    DIR *src_dp;
    struct dirent *entry;
    struct stat src_st;
    bool result = true;

    app_logi("copy_folder: src - %s, dst - %s", src, dest);
    src_dp = opendir(src);
    if (!src_dp) {
        return false;
    }
    if (!app_fs_is_entry_exist(dest)) {
        /* create a dir */
#ifndef WIN32
        result = !mkdir(dest, 0755);
#else
        result = !mkdir(dest);
#endif
        app_logi("copy_folder: make dir %s - result %d", dest, result);
    }
    while (result && (entry = readdir(src_dp)) != NULL) {
        /* Found a directory, but ignore . and ..*/
        if (0 == strcmp(".", entry->d_name) ||
            0 == strcmp("..", entry->d_name)) {
            continue;
        }
        char *src_path = app_fs_malloc(strlen(src) + strlen(entry->d_name) + 2);
        if (src_path) {
            strcpy(src_path, src);
            strcat(src_path, APP_FS_FILE_SEPERATOR);
            strcat(src_path, entry->d_name);
        } else {
            result = false;
            break;
        }
        if (-1 == stat(src_path, &src_st)) {
            result = false;
            app_fs_free(src_path);
            break;
        }
        /* compose the path */
        char *dst_path =
            (char *)app_fs_malloc(strlen(dest) + strlen(entry->d_name) + 2);
        if (dst_path) {
            strcpy(dst_path, dest);
            strcat(dst_path, APP_FS_FILE_SEPERATOR);
            strcat(dst_path, entry->d_name);
        } else {
            result = false;
        }
        if (S_ISDIR(src_st.st_mode)) {
            result = app_fs_copy_folder(src_path, dst_path);
        } else if (S_ISREG(src_st.st_mode)) {
            result = app_fs_copy_file(src_path, dst_path);
            if (result && is_move) {
                app_fs_remove_file(src_path);
            }
        }
        if (dst_path) {
            app_fs_free(dst_path);
        }
        app_fs_free(src_path);
        if (!result) {
            break;
        }
    }
    if (result && is_move) {
        app_fs_remove_folder(src);
    }
    closedir(src_dp);

    app_logi("copy_folder: result - %d", result);
    return result;
}
/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * app_fs_scan_dir_ex - Scan a folder to list the files and subfolders with
 * filter
 * @src: [IN] path of the folder
 * @is_recur: [IN] true to scan subfolders recursively, false for current folder
 * only
 * @list: [OUT] the scan result of list, including files and subfolders
 * @file_filter: [IN] callback to filter the files.
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 * Details of file_filter:
 *   input parameter is type and name of a file/folder
 *   return true to filter in.
 */
int app_fs_scan_dir_ex(const char *src, bool is_recur,
                       struct app_fs_scan_list_entry *list,
                       bool (*file_filter)(enum app_fs_file_type type,
                                           const char *name))
{
    int result = AIC_SUCCESS;
    DIR *dp = NULL;
    struct dirent *entry = NULL;
    struct stat statbuf;

    if (src != NULL) {
        dp = opendir(src);
    } else {
        result = AIC_EINVAL;
    }

    if (dp != NULL) {
        struct app_fs_scan_list_item *new_item = NULL;
        chdir(src);
        while ((entry = readdir(dp)) != NULL) {
            if (-1 == stat(entry->d_name, &statbuf)) {
                result = AIC_ENOENT;
                break;
            }
            if (S_ISDIR(statbuf.st_mode)) { /* a folder */
                /* Found a directory, but ignore . and ..*/
                if (0 == strcmp(".", entry->d_name) ||
                    0 == strcmp("..", entry->d_name)) {
                    continue;
                }
                bool filter_in = true;
                if (file_filter) {
                    filter_in =
                        file_filter(APP_FS_FILE_TYPE_FOLDER, entry->d_name);
                }
                if (!filter_in) {
                    continue;
                }
                new_item = (struct app_fs_scan_list_item *)app_fs_malloc(
                    sizeof(struct app_fs_scan_list_item));
                if (new_item != NULL) {
                    new_item->type = APP_FS_FILE_TYPE_FOLDER;
                    new_item->m_time = statbuf.st_mtime;
                    set_file_name(&new_item, is_recur, entry->d_name);
                    new_item->next = NULL;
                    add_to_scan_list(list, new_item);
                } else {
                    result = AIC_ENOMEM;
                    break;
                }
                if (is_recur) {
                    app_fs_scan_dir_ex(entry->d_name, is_recur, list,
                                       file_filter);
                }
            } else { /* a file */
                bool filter_in = true;
                if (file_filter) {
                    filter_in =
                        file_filter(APP_FS_FILE_TYPE_FILE, entry->d_name);
                }
                if (!filter_in) {
                    continue;
                }
                new_item = (struct app_fs_scan_list_item *)app_fs_malloc(
                    sizeof(struct app_fs_scan_list_item));
                if (new_item != NULL) {
                    new_item->type = APP_FS_FILE_TYPE_FILE;
                    new_item->m_time = statbuf.st_mtime;
                    set_file_name(&new_item, is_recur, entry->d_name);
                    new_item->next = NULL;
                    add_to_scan_list(list, new_item);
                } else {
                    result = AIC_ENOMEM;
                    break;
                }
            }
        }
#ifdef WIN32 /* SDK does not support change dir to parent yet */
        chdir("..");
#endif
        closedir(dp);
    } else {
        result = AIC_ENOTDIR;
    }

    return result;
}

/**
 * app_fs_free_scan_list - free scan list
 * @list: [IN] the scan list to free
 *
 * Return: NA
 *
 * Note: this function will free the child nodes only, list itself will
 * not be freed
 */
void app_fs_free_scan_list(struct app_fs_scan_list_entry *list)
{
    struct app_fs_scan_list_item *p = list->first;
    if (p != NULL) {
        while (p->next != NULL) {
            struct app_fs_scan_list_item *tmp = p;
            p = p->next;
            app_fs_free(tmp);
        }

        app_fs_free(p);
    }
    memset(list, 0, sizeof(struct app_fs_scan_list_entry));
}

/**
 * app_fs_is_folder - check if the given path is folder or not
 * @path: [IN] file/folder path
 *
 * Return: NA
 */
bool app_fs_is_folder(const char *path)
{
    bool result = false;

    if (path != NULL) {
        struct stat statbuf;
        if (-1 != stat(path, &statbuf)) {
            if (S_ISDIR(statbuf.st_mode)) {
                result = true;
            }
        }
    }
    return result;
}

/**
 * app_fs_remove_folder - remove a folder
 * @path: [IN] folder full path
 *
 * Return: true for success, false for failure
 */
bool app_fs_remove_folder(const char *path)
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_entry;
    struct dirent *entry;
    bool result = true;

    if (!app_fs_is_folder(path)) {
        app_loge("app_fs_remove_folder: attempt to remove a non-folder\n");
        return false;
    }
    /* if not possible to read the directory for this user */
    if ((dir = opendir(path)) == NULL) {
        app_loge("app_fs_remove_folder: unable to opendir %s\n", path);
        return false;
    }
    /* the length of the path */
    path_len = strlen(path);
    /* iteration through entries in the directory */
    while ((entry = readdir(dir)) != NULL) {
        /* skip entries "." and ".." */
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        /* determinate a full path of an entry */
        full_path = app_fs_malloc(path_len + strlen(entry->d_name) + 2);
        if (full_path) {
            strcpy(full_path, path);
            strcat(full_path, APP_FS_FILE_SEPERATOR);
            strcat(full_path, entry->d_name);
            /* stat for the entry */
            if (-1 != stat(full_path, &stat_entry)) {
                /* recursively remove a nested directory */
                if (S_ISDIR(stat_entry.st_mode) != 0) {
                    result = app_fs_remove_folder(full_path);
                } else {
                    /* remove a file object */
                    if (unlink(full_path) == 0) {
                        app_logi("app_fs_remove_folder: Removed a file: %s\n",
                                 full_path);
                    } else {
                        result = false;
                        app_loge(
                            "app_fs_remove_folder: Can`t remove a file: %s\n",
                            full_path);
                    }
                }
            }
            app_fs_free(full_path);
            if (!result) {
                break;
            }
        } else {
            result = false;
            app_loge("app_fs_remove_folder: no memory\n");
            break;
        }
    }
    closedir(dir);
    if (result) {
        /* remove the devastated directory and close the object of it */
        if (rmdir(path) == 0)
            app_logi("app_fs_remove_folder: Removed a directory: %s\n", path);
        else
            app_loge("app_fs_remove_folder: Can't remove a directory: %s\n",
                     path);
    }
    return result;
}

/**
 * app_fs_remove_file - remove a file
 * @path: [IN] file full path
 *
 * Return: true for success, false for failure
 */
bool app_fs_remove_file(const char *path)
{
    bool result = false;

    if (unlink(path) == 0) {
        result = true;
    }
    app_logi("app_fs_remove_file: %s, %d", path, result);
    return result;
}

/**
 * app_fs_is_entry_exist - check if the entry already exists or not
 * @path: [IN] full path of the entry
 *
 * Return: true for yes, false for no
 */
bool app_fs_is_entry_exist(const char *path)
{
    bool result = false;
    struct stat st;
    if (stat(path, &st) != -1) {
        result = true;
    }
    return result;
}

/**
 * app_fs_copy_file - copy a file
 * @src: [IN] full path of src file
 * @dest: [IN] full path of dest file
 *
 * Return: true for success, false for failure
 */
bool app_fs_copy_file(const char *src, const char *dest)
{
    bool result = true;
    if (!src || !dest) {
        return false;
    }
    app_logi("app_fs_copy_file: src - %s, dest - %s", src, dest);
    int src_fd, dest_fd;
    src_fd = open(src, O_RDONLY);
    if (-1 == src_fd) {
        app_logi("app_fs_copy_file: failed to open src fd");
        return false;
    }
    dest_fd = open(dest, O_WRONLY | O_CREAT);
    if (-1 == dest_fd) {
        app_logi("app_fs_copy_file: failed to dest src fd");
        close(src_fd);
        return false;
    }
    /* malloc a temp buffer to transfer data */
    uint8_t *buffer = (uint8_t *)app_fs_malloc(APP_FS_COPY_BUF_SIZE);
    if (buffer) {
        int read_cnt = 0;
        while ((read_cnt = read(src_fd, buffer, APP_FS_COPY_BUF_SIZE)) > 0) {
            app_logi("app_fs_copy_file: read %d bytes", read_cnt);
            write(dest_fd, buffer, read_cnt);
        }
        if (-1 == read_cnt) {
            app_loge("app_fs_copy_file: failed to read file");
            result = false;
        }
        app_fs_free(buffer);
    } else {
        app_loge("app_fs_copy_file: failed to malloc buffer");
        result = false;
    }

    close(src_fd);
    close(dest_fd);

    app_logi("app_fs_copy_file: result - %d", result);
    return result;
}

/**
 * app_fs_move_file - move a file
 * @src: [IN] full path of src file
 * @dest: [IN] full path of dest file
 *
 * Return: true for success, false for failure
 */
bool app_fs_move_file(const char *src, const char *dest)
{
    bool result = false;

    result = app_fs_copy_file(src, dest);
    if (result) {
        app_fs_remove_file(src);
    }

    return result;
}

/**
 * app_fs_copy_folder - copy a folder
 * @src: [IN] full path of src folder
 * @dest: [IN] full path of dest folder
 *
 * Return: true for success, false for failure
 */
bool app_fs_copy_folder(const char *src, const char *dest)
{
    return copy_folder(src, dest, false);
}

/**
 * app_fs_move_folder - move a folder
 * @src: [IN] full path of src folder
 * @dest: [IN] full path of dest folder
 *
 * Return: true for success, false for failure
 */
bool app_fs_move_folder(const char *src, const char *dest)
{
    return copy_folder(src, dest, true);
}

/**
 * app_fs_get_file_name_ext - get the extenion name of a file
 * @file_name: [IN] name of a file
 *
 * Return: extension name
 *
 */
const char *app_fs_get_file_name_ext(const char *file_name)
{
    const char *dot = strrchr(file_name, '.');
    if (!dot || dot == file_name)
        return "";
    return dot + 1;
}

/**
 * app_fs_sd_mount_listener_register - add listener to sd mount event
 * @listener: callback of listener
 * @user_data: user data
 *
 * Return: NA
 */
void app_fs_sd_mount_listener_register(void (*listener)(bool mount,
                                                        void *user_data),
                                       void *user_data)
{
    void *iter = NULL;
    bool is_added = false;
    /* check if the same listener has been added before or not*/
    _LV_LL_READ(&g_app_fs_sd_mount_cb, iter)
    {
        struct app_fs_sd_mount_cb *p = (struct app_fs_sd_mount_cb *)iter;
        if (p->listener == listener) {
            p->user_data = user_data;
            is_added = true;
            break;
        }
    }
    if (!is_added) {
        struct app_fs_sd_mount_cb *new_node =
            _lv_ll_ins_tail(&g_app_fs_sd_mount_cb);
        if (new_node) {
            new_node->listener = listener;
            new_node->user_data = user_data;
        } else {
            app_loge("app_fs_sd_mount_listener_register: failed");
        }
    }
}

/**
 * app_fs_sd_mount_listener_deregister - remove listener to sd mount event
 * @listener: callback of listener
 *
 * Return: NA
 */
void app_fs_sd_mount_listener_deregister(void (*listener)(bool mount,
                                                          void *user_data))
{
    void *iter = NULL;
    bool is_found = false;
    /* check if the same listener has been added before or not*/
    _LV_LL_READ(&g_app_fs_sd_mount_cb, iter)
    {
        struct app_fs_sd_mount_cb *p = (struct app_fs_sd_mount_cb *)iter;
        if (p->listener == listener) {
            is_found = true;
            break;
        }
    }
    if (is_found && iter) {
        _lv_ll_remove(&g_app_fs_sd_mount_cb, iter);
        lv_mem_free(iter);
    }
}

/**
 * app_fs_register_mount_hook - register fs mount hook into OS
 *
 * Return: NA
 */
void app_fs_register_mount_hook(void)
{
    _lv_ll_init(&g_app_fs_sd_mount_cb, sizeof(struct app_fs_sd_mount_cb));

#ifdef WIN32
    /* do nothing here in simulator*/
#else
    memset(g_app_fs_dev_status, 0, sizeof(g_app_fs_dev_status));
    /* create mutex */
    if (NULL == g_app_fs_mutex) {
        g_app_fs_mutex = aos_create_mutex("app_fs_dev_status_mutex");
        if (NULL == g_app_fs_mutex) {
            app_loge("app fs dev status create mutex failed.\n");
        }
    }
    fs_register_mount_hook(fs_mount_hooker, NULL);
#endif
}

/**
 * app_fs_deregister_mount_hook - deregister fs mount hook into OS
 *
 * Return: NA
 */
void app_fs_deregister_mount_hook(void)
{
#ifdef WIN32
    /* do nothing here in simulator*/
#else
    /* clear sd mount cb listeners */
    void *iter = NULL;
    _LV_LL_READ(&g_app_fs_sd_mount_cb, iter)
    {
        struct app_fs_sd_mount_cb *p = (struct app_fs_sd_mount_cb *)iter;
        app_fs_free(p);
    }
    _lv_ll_clear(&g_app_fs_sd_mount_cb);

    fs_unregister_mount_hook(fs_mount_hooker);
    app_fs_lock();
    memset(g_app_fs_dev_status, 0, sizeof(g_app_fs_dev_status));
    app_fs_unlock();
    if (g_app_fs_mutex) {
        aos_delete_mutex(g_app_fs_mutex);
        g_app_fs_mutex = NULL;
    }
#endif
}

/**
 * app_fs_get_volume - Get the volume name based on the device id
 * @device_id: [IN] the id of the device, local disk or SD
 * @volume_name: [OUT] the text string of the volume name
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_fs_get_volume(enum app_fs_dev_id device_id, char *volume_name)
{
    int result = AIC_SUCCESS;
#ifdef WIN32
    switch (device_id) {
    case APP_FS_DEV_ID_LOCAL:
        strcpy(volume_name, APP_FS_LOCAL_ROOT);
        break;

    case APP_FS_DEV_ID_SD:
        strcpy(volume_name, APP_FS_SD_ROOT);
        break;

    default:
        result = AIC_EINVAL;
        break;
    }
#else
    LV_ASSERT(device_id < APP_FS_DEV_ID_CNT);
    FS_VOLUME_INFO volume_info = {0};
    u32 devtype = FS_DEV_MAX;

    switch (device_id) {
    case APP_FS_DEV_ID_LOCAL:
        devtype = FS_DEV_FLASH;
        break;

    case APP_FS_DEV_ID_SD:
        devtype = FS_DEV_SD;
        break;

    default:
        return AIC_EINVAL;
    }
    result = fs_getdevinfo(devtype, &volume_info);
    if (AIC_SUCCESS == result) {
        app_fs_lock();
        g_app_fs_dev_status[device_id].is_mounted = (bool)volume_info.mount;
        strcpy(g_app_fs_dev_status[device_id].volume, volume_info.volume_name);
        if (g_app_fs_dev_status[device_id].is_mounted) {
            strcpy(volume_name, g_app_fs_dev_status[device_id].volume);
            /* add file seperator */
            strcat(volume_name, APP_FS_FILE_SEPERATOR);
        } else {
            result = AIC_ENOENT;
        }
        app_fs_unlock();
    }

#endif
    return result;
}

/**
 * app_fs_is_sd_mounted - Check if sd is mounted or not
 *
 * Return: true for mounted, false for not mounted
 */
bool app_fs_is_sd_mounted(void)
{
    bool result = false;
#ifdef WIN32
    char volume[10] = {0};
    if (AIC_SUCCESS == app_fs_get_volume(APP_FS_DEV_ID_SD, volume)) {
        DIR *dp = NULL;

        dp = opendir(volume);
        if (dp != NULL) {
            closedir(dp);
            result = true;
        }
    }

    return result;
#else
    FS_VOLUME_INFO volume_info = {0};

    if (AIC_SUCCESS == fs_getdevinfo(FS_DEV_SD, &volume_info)) {
        app_fs_lock();
        g_app_fs_dev_status[APP_FS_DEV_ID_SD].is_mounted =
            (bool)volume_info.mount;
        strcpy(g_app_fs_dev_status[APP_FS_DEV_ID_SD].volume,
               volume_info.volume_name);
        result = g_app_fs_dev_status[APP_FS_DEV_ID_SD].is_mounted;
        app_fs_unlock();
    }

    return result;
#endif
}

/**
 * app_fs_handle_sd_mount_msg - handle sd mount msg
 *
 * Return: NA
 * This should be run in app task.
 */
void app_fs_handle_sd_mount_msg(void)
{
    void *iter = NULL;
    bool is_mounted = false;
#ifndef WIN32
    app_fs_lock();
    is_mounted = g_app_fs_dev_status[APP_FS_DEV_ID_SD].is_mounted;
    app_fs_unlock();
#endif
    _LV_LL_READ(&g_app_fs_sd_mount_cb, iter)
    {
        struct app_fs_sd_mount_cb *p = (struct app_fs_sd_mount_cb *)iter;
        if (p->listener) {
            p->listener(is_mounted, p->user_data);
        }
    }
}