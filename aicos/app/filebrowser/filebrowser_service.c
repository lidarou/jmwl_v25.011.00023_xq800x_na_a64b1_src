/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file implements the service functionalities of file
 *    browser(FB) app.
 */

/*******************************************
 *              INCLUDES
 ******************************************/
#include "filebrowser_service.h"
#include <aic_errno.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef WIN32
#include <fileapi.h>
#else
#include <aos_thread.h>
#include <sys/statfs.h>
#endif

/*******************************************
 *              DEFINES
 ******************************************/
#ifndef WIN32
#define FB_SRV_TASK_STACK_SIZE (2048u)
/* ATTENTION: MUST BE LOWER THAN PRIO OF APP TASK */
#define FB_SRV_TASK_COM_PRIO (36u)
#define FB_SRV_TASK_QUEUE_NUM (10)
#define FB_SRV_TASK_QUEUE_NAME "fb_srv_queue"
#define FB_SRV_TASK_QUEUE_TIMEOUT (5)
#endif

/*******************************************
 *              TYPEDEFS
 ******************************************/
#ifndef WIN32
/**
 * enum fb_srv_msg_id - message id used in file browser service task
 * @FB_SRV_MSG_ID_FORMAT: format device
 * @FB_SRV_MSG_ID_MOVE: move files/folders
 * @FB_SRV_MSG_ID_COPY: copy files/folders
 */
enum fb_srv_msg_id {
    FB_SRV_MSG_ID_FORMAT,
    FB_SRV_MSG_ID_MOVE,
    FB_SRV_MSG_ID_COPY,

    FB_SRV_MSG_ID_UNKNOWN = 0xFFFFFFFF
};

/**
 * struct fb_srv_ctx - context of file browser service task
 * @thread: thread id of the task
 * @queue: pointer to the queue of the task
 * @to_stop_task: flag to set when need to quit task
 * @task_stopped: flag to indicate if the task is stopped or not
 */
struct fb_srv_ctx {
    aos_thread_id thread;
    aos_queue_ptr queue;
    bool to_stop_task;
    bool task_stopped;
};

/**
 * struct fb_srv_msg - msg structure
 * @id: id of the message
 * @data: data of the message
 */
struct fb_srv_msg {
    enum fb_srv_msg_id id;
    void *data;
};

/**
 * struct fb_srv_msg_id_format - data structure of FB_SRV_MSG_ID_FORMAT
 * @volume: volume path of the device to format
 * @notify_cb: callback to notify the format result
 */
struct fb_srv_msg_id_format {
    char volume[FS_VOLUME_NAME_MAX_LEN];
    void (*notify_cb)(int result);
};
#endif

/*******************************************
 *              STATIC VARIABLES
 ******************************************/
#ifndef WIN32
struct fb_srv_ctx g_fb_srv_ctx = {.thread = AOS_INVALID_THREAD_ID,
                                  .queue = NULL,
                                  .to_stop_task = false,
                                  .task_stopped = false};
#endif

/*******************************************
 *              STATIC FUNCTIONS
 ******************************************/
#ifndef WIN32
/**
 * fb_srv_handle_format_msg - handle format in task
 * @msg_data: [IN] msg data
 *
 * Return: NA
 */
static void fb_srv_handle_format_msg(void *msg_data)
{
    struct fb_srv_msg_id_format *data = (struct fb_srv_msg_id_format *)msg_data;
    int result;

    result = fs_format(data->volume);
    if (data->notify_cb) {
        data->notify_cb(result);
    }
    fb_app_free(msg_data);
}

/**
 * fb_srv_task_handler - handler of the task
 * @thread_input: [IN] thread input
 *
 * Return: NA
 */
static void fb_srv_task_handler(void *thread_input)
{
    g_fb_srv_ctx.task_stopped = false;

    while (!g_fb_srv_ctx.to_stop_task) {
        struct fb_srv_msg *msg =
            (struct fb_srv_msg *)aos_recv_msg(g_fb_srv_ctx.thread, 5000);
        if (!msg)
            continue;
        switch (msg->id) {
        case FB_SRV_MSG_ID_FORMAT:
            fb_srv_handle_format_msg(msg->data);
            break;
        case FB_SRV_MSG_ID_MOVE:
            break;
        case FB_SRV_MSG_ID_COPY:
            break;
        default:
            break;
        }
        fb_app_free(msg);
    }

    g_fb_srv_ctx.task_stopped = true;
}

/**
 * fb_srv_encap_msg - encapsulate a message
 * @id: [IN] msg id
 * @data: [IN] msg data
 *
 * Return: pointer to the encapsulated msg
 */
static struct fb_srv_msg *fb_srv_encap_msg(enum fb_srv_msg_id id, void *data)
{
    struct fb_srv_msg *msg = NULL;
    msg = (struct fb_srv_msg *)fb_app_malloc(sizeof(struct fb_srv_msg));
    if (msg) {
        msg->id = id;
        msg->data = data;
    }
    return msg;
}

static int fb_srv_send_msg(aos_msg_ptr msg, u32 wait_option)
{
    int result = AIC_EFAULT;

    if (g_fb_srv_ctx.thread) {
        result = aos_send_msg(g_fb_srv_ctx.thread, msg, wait_option);
    }

    return result;
}
#endif

/**
 * fs_get_node_count - Scan a folder to get count of subfolders and files
 * @path: [IN] path of the folder
 * @detail: [OUT] the detail result
 *
 * Return: true for success, false for failure
 */
static bool fs_get_node_count(const char *path, struct fb_folder_detail *detail)
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_entry;
    struct dirent *entry;
    bool result = true;

    /* if not possible to read the directory for this user */
    if ((dir = opendir(path)) == NULL) {
        app_loge("fs_get_node_count: unable to opendir %s\n", path);
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
                    detail->subfolder_cnt++;
                    result = fs_get_node_count(full_path, detail);
                } else {
                    detail->file_cnt++;
                    detail->size += stat_entry.st_size;
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

    return result;
}

/**
 * extract_entry_short_name - extract the entry short name from full path
 * @full_path: [IN] the full path of the folder
 *
 * Return: pointer to short name. NULL for failure
 * Note: the buffer of the short name should be freed manually
 */
static char *extract_entry_short_name(const char *full_path)
{
    char *short_name = NULL;

    LV_ASSERT(full_path != NULL);
    int len = strlen(full_path);
    if (len) {
        const char *p = strrchr(full_path, FB_FILE_SEPERATOR_CHR);
        if (p) {
            short_name = (char *)fb_app_malloc(strlen(p));
            if (short_name) {
                strcpy(short_name, p + 1);
            }
        }
    }

    return short_name;
}
/*******************************************
 *              GLOBAL FUNCTIONS
 ******************************************/
/**
 * app_srv_fb_format - Format local disk or SD
 * @device_id: [IN] the id of the device, local disk or SD
 * @notify_cb: [IN] callback to receive the format result.
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 * This is an async function call, the final result can be notified by cb.
 * Final result options:
 *      AIC_SUCCESS for success
 *      otherwise to refer to aic_errno.h
 */
int app_srv_fb_format(enum app_fs_dev_id device_id,
                      void (*notify_cb)(int result))
{
    int result = AIC_EACCES;
    LV_ASSERT(device_id < APP_FS_DEV_ID_CNT);

#ifndef WIN32
    struct fb_srv_msg *msg = NULL;
    struct fb_srv_msg_id_format *msg_data =
        (struct fb_srv_msg_id_format *)fb_app_malloc(
            sizeof(struct fb_srv_msg_id_format));
    if (msg_data) {
        result = app_fs_get_volume(device_id, msg_data->volume);
        if (AIC_SUCCESS == result) {
            msg_data->notify_cb = notify_cb;
            msg = fb_srv_encap_msg(FB_SRV_MSG_ID_FORMAT, msg_data);
            if (msg) {
                result = fb_srv_send_msg(msg, 10);
            }
        }
        /* memory to free if not success */
        if (AIC_SUCCESS != result) {
            fb_app_free(msg_data);
            if (msg) {
                fb_app_free(msg);
            }
        }
    }
#endif

    return result;
}

/**
 * app_srv_fb_scandir_ex - Scan a folder to list the files and subfolders with
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
int app_srv_fb_scandir_ex(const char *src, bool is_recur,
                          struct app_fs_scan_list_entry *list,
                          bool (*file_filter)(enum app_fs_file_type type,
                                              const char *name))
{
    int result = app_fs_scan_dir_ex(src, is_recur, list, file_filter);

    return result;
}

/**
 * app_srv_fb_scandir - Scan a folder to list the files and subfolders
 * @src: [IN] path of the folder
 * @is_recur: [IN] true to scan subfolders recursively, false for current folder
 * only
 * @list: [OUT] the scan result of list, including files and subfolders
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_scandir(const char *src, bool is_resur,
                       struct app_fs_scan_list_entry *list)
{
    int result = app_fs_scan_dir_ex(src, is_resur, list, NULL);
    return result;
}

/**
 * app_srv_fb_free_scan_list - free scan list
 * @list: [IN] the scan list to free
 *
 * Return: NA
 *
 * Note: this function will free the child nodes only, list itself will
 * not be freed
 */
void app_srv_fb_free_scan_list(struct app_fs_scan_list_entry *list)
{
    app_fs_free_scan_list(list);
}

/**
 * app_srv_fb_create_path_node_list - create a node list which saves paths
 *
 * Return: NA
 * Note: each node saves a full path of a file/folder.
 */
lv_ll_t *app_srv_fb_create_path_node_list(void)
{
    lv_ll_t *result = NULL;
    result = fb_app_malloc(sizeof(lv_ll_t));
    if (result) {
        _lv_ll_init(result, sizeof(char *));
    }
    return result;
}

/**
 * app_srv_fb_path_node_list_add - add a path into path node list
 *
 * Return: NA
 */
bool app_srv_fb_path_node_list_add(lv_ll_t *list, const char *path)
{
    bool result = false;
    char *new_str = NULL;
    char **node = NULL;

    if (!path || !list) {
        return false;
    }
    node = (char **)_lv_ll_ins_tail(list);
    if (!node) {
        return false;
    }
    new_str = (char *)fb_app_malloc(strlen(path) + 1);
    if (new_str) {
        strcpy(new_str, path);
        *node = new_str;
        result = true;
    }

    return result;
}

/**
 * app_srv_fb_free_path_node_list - free the memory used by path node list
 *
 * Return: NA
 */
void app_srv_fb_free_path_node_list(lv_ll_t *list)
{
    void *iter = NULL;
    if (!list) {
        return;
    }
    _LV_LL_READ(list, iter)
    {
        char **str_p = (char **)iter;
        /* free node */
        app_fs_free(*str_p);
    }
    _lv_ll_clear(list);
    fb_app_free(list);
}

/**
 * app_srv_fb_copy - Copy a file or folder to a destination folder
 * @src: [IN] paths of the source file/folder
 * @dest: [IN] path of the destination folder to copy
 * @prog_cb: [IN, optional] the callback to notify the current copy progress
 * @user_data: [IN, optional] the user data to invoke prog_cb
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 *
 * prog_cb and user_data should be NULL if you don't need notification of the
 * progress.
 */
int app_srv_fb_copy(struct fb_entry_list *src, const char *dest,
                    void (*prog_cb)(void *user_data, enum fb_progress phase,
                                    int done, int total),
                    void *user_data)
{
    int result = AIC_SUCCESS;
    bool is_ok = true;
    LV_ASSERT(src != NULL);
    app_srv_fb_entry_list_add_ref(src);
    struct fb_entry_item *p = src->first;
    while (p != NULL && is_ok) {
        char *short_name = extract_entry_short_name(p->name);
        if (!short_name) {
            result = AIC_ENOMEM;
            break;
        }
        char *dst_path =
            (char *)app_fs_malloc(strlen(dest) + strlen(short_name) + 2 + 20);
        if (!dst_path) {
            fb_app_free(short_name);
            result = AIC_ENOMEM;
            break;
        } else {
            strcpy(dst_path, dest);
            strcat(dst_path, APP_FS_FILE_SEPERATOR);
            strcat(dst_path, short_name);
        }
        /* check if the src and dest are the same */
        if (0 == strcmp(p->name, dst_path)) {
            /* need to create a copy */
            int copy_no = 0;
            do {
                if (copy_no > 0) {
                    sprintf(dst_path, "%s%s%s_copy(%d)", dest,
                            APP_FS_FILE_SEPERATOR, short_name, copy_no);
                } else {
                    sprintf(dst_path, "%s%s%s_copy", dest,
                            APP_FS_FILE_SEPERATOR, short_name);
                }
                copy_no++;
            } while (app_fs_is_entry_exist(dst_path));
        }
        if (APP_FS_FILE_TYPE_FOLDER == p->type) {
            is_ok = app_fs_copy_folder(p->name, dst_path);
        } else if (APP_FS_FILE_TYPE_FILE == p->type) {
            is_ok = app_fs_copy_file(p->name, dst_path);
        }
        fb_app_free(dst_path);
        fb_app_free(short_name);
        p = p->next;
    }
    app_srv_fb_entry_list_release(src);

    return result;
}

/**
 * app_srv_fb_move - Move a file or folder to a destination folder
 * @src: [IN] paths of the source file/folder
 * @dest: [IN] path of the destination folder to copy
 * @prog_cb: [IN, optional] the callback to notify the current copy progress
 * @user_data: [IN, optional] the user data to invoke prog_cb
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 *
 * prog_cb and user_data should be NULL if you don't need notification of the
 * progress.
 */
int app_srv_fb_move(struct fb_entry_list *src, const char *dest,
                    void (*prog_cb)(void *user_data, enum fb_progress phase,
                                    int done, int total),
                    void *user_data)
{
    LV_ASSERT(src != NULL);
    int result = AIC_SUCCESS;
    bool is_ok = true;
    LV_ASSERT(src != NULL);
    app_srv_fb_entry_list_add_ref(src);
    struct fb_entry_item *p = src->first;
    while (p != NULL && is_ok) {
        char *short_name = extract_entry_short_name(p->name);
        if (!short_name) {
            result = AIC_ENOMEM;
            break;
        }
        char *dst_path =
            (char *)app_fs_malloc(strlen(dest) + strlen(short_name) + 2);
        if (!dst_path) {
            fb_app_free(short_name);
            result = AIC_ENOMEM;
            break;
        } else {
            strcpy(dst_path, dest);
            strcat(dst_path, APP_FS_FILE_SEPERATOR);
            strcat(dst_path, short_name);
        }
        /* check if the src and dest are the same */
        if (0 != strcmp(p->name, dst_path)) {
            if (APP_FS_FILE_TYPE_FOLDER == p->type) {
                is_ok = app_fs_move_folder(p->name, dst_path);
            } else if (APP_FS_FILE_TYPE_FILE == p->type) {
                is_ok = app_fs_move_file(p->name, dst_path);
            }
        }
        fb_app_free(dst_path);
        fb_app_free(short_name);
        p = p->next;
    }
    app_srv_fb_entry_list_release(src);

    return result;
}

/**
 * app_srv_fb_rename - Rename a file or folder
 * @old: [IN] the full path of the source file or folder
 * @new: [IN] new name of the file or folder
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_rename(const char *old, const char *new)
{
    int result = AIC_SUCCESS;

    // TBD

    return result;
}

/**
 * app_srv_fb_remove - Remove a file or folder
 * @path: [IN] the full path of the file or folder
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_remove(const char *path)
{
    bool result = true;

    if (app_fs_is_folder(path)) {
        result = app_fs_remove_folder(path);
    } else {
        result = app_fs_remove_file(path);
    }

    return result;
}

/**
 * app_srv_fb_is_valid_folder - Check if the folder is valid
 * @path: [IN] the full path of the folder
 *
 * Return: true for valid, false for not
 * Note: internal use only
 */
bool app_srv_fb_is_valid_folder(const char *path)
{
    return app_fs_is_folder(path);
}

/**
 * app_srv_fb_get_volume_detail - get detail of the volume
 * @device_id: [IN] id
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_get_volume_detail(enum app_fs_dev_id device_id,
                                 struct fb_volume_detail *detail)
{
    int result = AIC_SUCCESS;

    if (!detail) {
        return AIC_EINVAL;
    }
    memset(detail, 0, sizeof(struct fb_volume_detail));
    detail->device = device_id;
#ifdef WIN32
    ULARGE_INTEGER avail, total, total_free;
    const char *root_path_name = APP_FS_LOCAL_ROOT;
    WINBOOL ret;

    if (APP_FS_DEV_ID_SD == device_id) {
        root_path_name = APP_FS_SD_ROOT;
    }
    ret = GetDiskFreeSpaceEx(root_path_name, &avail, &total, &total_free);
    if (ret) {
        detail->capacity = total.QuadPart;
        detail->avail = avail.QuadPart;
    }
#else
    char volume[FS_VOLUME_NAME_MAX_LEN];

    result = app_fs_get_volume(device_id, volume);
    if (AIC_SUCCESS == result) {
        struct statfs volume_stat;

        result = statfs(volume, &volume_stat);
        if (AIC_SUCCESS == result) {
            detail->capacity =
                (uint64_t)volume_stat.f_blocks * (uint64_t)volume_stat.f_bsize;
            detail->avail =
                (uint64_t)volume_stat.f_bfree * (uint64_t)volume_stat.f_bsize;
        }
    }
#endif

    return result;
}

/**
 * app_srv_fb_get_folder_detail - get detail of a folder
 * @path: [IN] the full path of the folder
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_get_folder_detail(const char *path,
                                  struct fb_folder_detail *detail)
{
    bool result = false;
    if (!path || !detail) {
        return false;
    }
    /* check if it is a vaild folder */
    if (app_srv_fb_is_valid_folder(path)) {
        memset(detail, 0, sizeof(struct fb_folder_detail));
        /* get count of files and count of subfolders */
        result = fs_get_node_count(path, detail);
    }

    return result;
}

/**
 * app_srv_fb_get_file_size - get size of a file
 * @path: [IN] the full path of the file
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_get_file_detail(const char *path, struct fb_file_detail *detail)
{
    bool result = false;
    if (!path || !detail) {
        return false;
    }
    /* get file size */
    struct stat statbuf;
    if (-1 != stat(path, &statbuf)) {
        if (!S_ISDIR(statbuf.st_mode)) {
            detail->size = (uint32_t)statbuf.st_size;
            result = true;
        }
    }
    if (result) {
        lv_res_t res;
        lv_img_header_t header;
        res = lv_img_decoder_get_info(path, &header);
        if (LV_RES_OK == res) {
            detail->is_img = true;
            detail->img_h = header.h;
            detail->img_w = header.w;
        } else {
            detail->is_img = false;
            detail->img_h = 0;
            detail->img_w = 0;
        }
    }

    return result;
}

/**
 * app_srv_fb_create_entry_list - create an entry list
 *
 * Return: pointer to the created entry list, NULL for failure
 */
struct fb_entry_list *app_srv_fb_create_entry_list(void)
{
    struct fb_entry_list *result = NULL;

    result =
        (struct fb_entry_list *)fb_app_malloc(sizeof(struct fb_entry_list));
    if (result) {
        result->first = NULL;
        result->last = NULL;
        result->total = 0;
        result->ref = 0;
    }
    return result;
}

/**
 * app_srv_fb_create_entry_list - create an entry list
 *
 * Return: the new reference count, -1 for failure
 */
int app_srv_fb_entry_list_add_ref(struct fb_entry_list *list)
{
    LV_ASSERT(list != NULL);
    return ++list->ref;
}

/**
 * app_srv_fb_entry_list_release - release entry list
 * @list: [IN] the entry list to release
 *
 * Return: NA
 */
void app_srv_fb_entry_list_release(struct fb_entry_list *list)
{
    LV_ASSERT(list != NULL);
    list->ref--;
    if (list->ref > 0) {
        return;
    }
    struct fb_entry_item *p = list->first;
    if (p != NULL) {
        while (p->next != NULL) {
            struct fb_entry_item *tmp = p;
            p = p->next;
            fb_app_free(tmp->name);
            fb_app_free(tmp);
        }
        fb_app_free(p);
    }
    fb_app_free(list);
}

/**
 * app_srv_fb_add_to_entry_list - add an entry into list
 * @list: [IN] entry list
 * @type: [IN] type of the entry
 * @path: [IN] full path of the entry
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_add_to_entry_list(struct fb_entry_list *list,
                                  enum app_fs_file_type type, const char *path)
{
    bool result = false;
    struct fb_entry_item *item = NULL;
    if (!list || !path) {
        return false;
    }
    /* compose an item */
    item = (struct fb_entry_item *)fb_app_malloc(sizeof(struct fb_entry_item));
    if (!item) {
        return false;
    }
    item->next = NULL;
    item->type = type;
    item->name = (char *)fb_app_malloc(strlen(path) + 1);
    if (!item->name) {
        fb_app_free(item);
        return false;
    }
    strcpy(item->name, path);

    /* add to the tail of the list */
    if (NULL == list->first) {
        list->first = item;
        list->last = item;
    } else {
        list->last->next = item;
        list->last = item;
    }
    list->total++;

    return result;
}

#ifndef WIN32
/**
 * app_srv_fb_init - start fb service task
 *
 * Return: NA
 */
void app_srv_fb_init(void)
{
    if (g_fb_srv_ctx.thread != AOS_INVALID_THREAD_ID) {
        /* already inited */
        return;
    }

    g_fb_srv_ctx.queue = aos_create_queue(
        FB_SRV_TASK_QUEUE_NAME, sizeof(aos_msg_ptr), FB_SRV_TASK_QUEUE_NUM);

    if (g_fb_srv_ctx.queue) {
        g_fb_srv_ctx.thread = aos_create_thread(
            "app_fb_service", fb_srv_task_handler, NULL, FB_SRV_TASK_STACK_SIZE,
            FB_SRV_TASK_QUEUE_NUM, FB_SRV_TASK_QUEUE_NAME, FB_SRV_TASK_COM_PRIO,
            AOS_AUTO_START);
    } else {
        app_loge("app_srv_fb_init: failed to create a queue\n");
    }
}

/**
 * app_srv_fb_destroy - destroy fb service task
 *
 * Return: NA
 */
void app_srv_fb_destroy(void)
{
    if (g_fb_srv_ctx.thread != AOS_INVALID_THREAD_ID) {
        int retry_cnt = 5;
        g_fb_srv_ctx.to_stop_task = true;
        while (!g_fb_srv_ctx.task_stopped && retry_cnt) {
            retry_cnt--;
            aos_sleep(10);
        }
        if (0 == retry_cnt) {
            aos_terminate_thread(g_fb_srv_ctx.thread);
        }
        aos_delete_thread(g_fb_srv_ctx.thread);
        g_fb_srv_ctx.thread = AOS_INVALID_THREAD_ID;
        aos_delete_queue(g_fb_srv_ctx.queue);
        g_fb_srv_ctx.queue = NULL;
        g_fb_srv_ctx.to_stop_task = false;
        g_fb_srv_ctx.thread = 0;
    }
}
#endif

#ifdef CONFIG_FB_APP_DEBUG_MEM
/**
 * fb_app_malloc_ex - malloc memory and output debug info
 * @size: [IN] size
 * @name: [IN] filename
 * @line: [IN] line number
 *
 * Return: pointer to allocated memory, NULL for failure case.
 * This is just used for debug purpose
 */
void *fb_app_malloc_ex(size_t size, const char *name, uint32_t line)
{
    void *p = lv_mem_alloc(size);
    if (p) {
        app_logi("fb app lv_mem_alloc: %u in FILE %s - %u\n", p, name, line);
    } else {
        app_loge("failed to alloc memory. size = %u in FILE %s - %u\n", size,
                 name, line);
    }
    return p;
}

/**
 * fb_app_free_ex - free memory and output debug info
 * @x: [IN] pointer to the memory to free
 * @name: [IN] filename
 * @line: [IN] line number
 *
 * Return: NA
 * This is just used for debug purpose
 */
void fb_app_free_ex(void *x, const char *name, uint32_t line)
{
    if (x) {
        lv_mem_free(x);
        app_logi("fb_app lv_mem_free: %u in FILE %s - %u\n", x, name, line);
    } else {
        app_loge("ERROR: try to free and NULL pointer in FILE %s - %u\n", name,
                 line);
    }
}
#endif
