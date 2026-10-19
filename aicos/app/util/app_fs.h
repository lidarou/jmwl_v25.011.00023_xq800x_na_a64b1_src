/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) to fs operation.
 * All service prototypes and data structure definitions are defined in this
 * file.
 */

#ifndef __APP_FS_H__
#define __APP_FS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#ifndef WIN32
#include <aic_osa_ex.h>
#include <fs_api.h>
#else
#include <stdio.h>
#endif
#include <lvgl.h>
#include <stdbool.h>
#include <time.h>

/*******************************************
 *              DEFINES
 ******************************************/
#ifdef WIN32
#ifndef APP_FS_LOCAL_ROOT
#define APP_FS_LOCAL_ROOT "C:/"
#endif
#ifndef APP_FS_SD_ROOT
#define APP_FS_SD_ROOT "E:/"
#endif
#define app_logi printf
#define app_loge printf
#define APP_FS_VOLUME_LEN 10
#else
#define APP_FS_VOLUME_LEN FS_VOLUME_NAME_MAX_LEN
#endif
#define APP_FS_FILE_SEPERATOR "/"
#define APP_FS_FILE_SEPERATOR_CHR '/'
#define app_fs_malloc(x) lv_mem_alloc((size_t)(x))
#define app_fs_free(x) lv_mem_free((void *)(x))
#define APP_FS_SHORT_PATH 10
#define APP_FS_MAX_PATH 256
#define APP_FS_NOTIFY_SD_MOUNT_MSG 1001 /* To be changed */

/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * enum - app fs device IDs
 *
 * @APP_FS_DEV_ID_LOCAL: local disk.
 * @APP_FS_DEV_ID_SD: sd card.
 */
enum app_fs_dev_id {
    APP_FS_DEV_ID_LOCAL,
    APP_FS_DEV_ID_SD,

    APP_FS_DEV_ID_CNT,
    APP_FS_DEV_ID_UNKNOWN = 0xFFFF
};

/**
 * enum - file types
 *
 * @APP_FS_FILE_TYPE_FOLDER: folder
 * @APP_FS_FILE_TYPE_FILE: file
 */
enum app_fs_file_type {
    APP_FS_FILE_TYPE_FOLDER,
    APP_FS_FILE_TYPE_FILE,

    APP_FS_FILE_TYPE_UNKNOWN = 0xFFFF
};

/**
 * struct app_fs_scan_list_item - file node for scanning result
 * @type: type, file or folder
 * @m_time: file-modified time
 * @next: pointer to the next file
 * @name: full path name of the file/subfolder
 *
 * Note: to optimize the memory allocation, name should be put as the last
 * member, because if length of the target name is larger than the default size,
 * the whole structure will be reallocate a compatible size
 */
struct app_fs_scan_list_item {
    enum app_fs_file_type type;
    time_t m_time;
    struct app_fs_scan_list_item *next;
    /* WARNNING: tricky size. see comments above*/
    char name[APP_FS_SHORT_PATH];
};

/**
 * struct app_fs_scan_list_entry - file node for scanning result
 * @total: total number of files/subfolders
 * @first: pointer to the first item
 * @last: pointer to the last item
 */
struct app_fs_scan_list_entry {
    int total;
    struct app_fs_scan_list_item *first;
    struct app_fs_scan_list_item *last;
};

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * app_fs_sd_mount_listener_register - add listener to sd mount event
 * @listener: callback of listener
 * @user_data: user data
 *
 * Return: NA
 */
void app_fs_sd_mount_listener_register(void (*listener)(bool mount,
                                                        void *user_data),
                                       void *user_data);

/**
 * app_fs_sd_mount_listener_deregister - remove listener to sd mount event
 * @listener: callback of listener
 *
 * Return: NA
 */
void app_fs_sd_mount_listener_deregister(void (*listener)(bool mount,
                                                          void *user_data));
/**
 * app_fs_register_mount_hook - register fs mount hook into OS
 *
 * Return: NA
 */
void app_fs_register_mount_hook(void);

/**
 * app_fs_deregister_mount_hook - deregister fs mount hook into OS
 *
 * Return: NA
 */
void app_fs_deregister_mount_hook(void);

/**
 * app_fs_get_volume - Get the volume name based on the device id
 * @device_id: [IN] the id of the device, local disk or SD
 * @volume_name: [OUT] the text string of the volume name
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_fs_get_volume(enum app_fs_dev_id device_id, char *volume_name);

/**
 * app_fs_is_sd_mounted - Check if sd is mounted or not
 *
 * Return: true for mounted, false for not mounted
 */
bool app_fs_is_sd_mounted(void);

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
int app_fs_scan_dir_ex(const char *src, bool is_resur,
                       struct app_fs_scan_list_entry *list,
                       bool (*file_filter)(enum app_fs_file_type type,
                                           const char *name));

/**
 * app_fs_scan_dir - Scan a folder to list the files and subfolders
 * @src: [IN] path of the folder
 * @is_recur: [IN] true to scan subfolders recursively, false for current folder
 * only
 * @list: [OUT] the scan result of list, including files and subfolders
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_fs_scan_dir(const char *src, bool is_resur,
                    struct app_fs_scan_list_entry *list);

/**
 * app_fs_free_scan_list - free scan list
 * @list: [IN] the scan list to free
 *
 * Return: NA
 *
 * Note: this function will free the child nodes only, list itself will
 * not be freed
 */
void app_fs_free_scan_list(struct app_fs_scan_list_entry *list);

/**
 * app_fs_is_folder - check if the given path is folder or not
 * @path: [IN] file/folder full path
 *
 * Return: true for folder, false for not
 */
bool app_fs_is_folder(const char *path);

/**
 * app_fs_remove_file - remove a file
 * @path: [IN] file full path
 *
 * Return: true for success, false for failure
 */
bool app_fs_remove_file(const char *path);

/**
 * app_fs_remove_folder - remove a folder
 * @path: [IN] folder full path
 *
 * Return: true for success, false for failure
 */
bool app_fs_remove_folder(const char *path);

/**
 * app_fs_is_entry_exist - check if the entry already exists or not
 * @path: [IN] full path of the entry
 *
 * Return: true for yes, false for no
 */
bool app_fs_is_entry_exist(const char *path);

/**
 * app_fs_copy_file - copy a file
 * @src: [IN] full path of src file
 * @dest: [IN] full path of dest file
 *
 * Return: true for success, false for failure
 */
bool app_fs_copy_file(const char *src, const char *dest);

/**
 * app_fs_move_file - move a file
 * @src: [IN] full path of src file
 * @dest: [IN] full path of dest file
 *
 * Return: true for success, false for failure
 */
bool app_fs_move_file(const char *src, const char *dest);

/**
 * app_fs_copy_folder - copy a folder
 * @src: [IN] full path of src folder
 * @dest: [IN] full path of dest folder
 *
 * Return: true for success, false for failure
 */
bool app_fs_copy_folder(const char *src, const char *dest);

/**
 * app_fs_move_folder - move a folder
 * @src: [IN] full path of src folder
 * @dest: [IN] full path of dest folder
 *
 * Return: true for success, false for failure
 */
bool app_fs_move_folder(const char *src, const char *dest);

/**
 * app_fs_get_file_name_ext - get the extenion name of a file
 * @file_name: [IN] name of a file
 *
 * Return: extension name
 *
 */
const char *app_fs_get_file_name_ext(const char *file_name);

/**
 * app_fs_handle_sd_mount_msg - handle sd mount msg
 *
 * Return: NA
 */
void app_fs_handle_sd_mount_msg(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __APP_FS_H__ */
