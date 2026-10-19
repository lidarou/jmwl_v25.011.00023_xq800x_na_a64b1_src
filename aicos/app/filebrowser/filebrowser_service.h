/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) to file
 *    browser(FB) app.  All service prototypes and data structure
 *    definitions are defined in this file.
 */

#ifndef __FILEBROWSER_SERVCIE_H__
#define __FILEBROWSER_SERVCIE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <app_fs.h>

/*******************************************
 *              DEFINES
 ******************************************/
#ifdef WIN32
#define FB_FILE_SEPERATOR APP_FS_FILE_SEPERATOR
#define FB_FILE_SEPERATOR_CHR APP_FS_FILE_SEPERATOR_CHR
#else
#define FB_FILE_SEPERATOR "/"
#define FB_FILE_SEPERATOR_CHR '/'
#endif

#define FB_MAX_PATH APP_FS_MAX_PATH

#ifdef CONFIG_FB_APP_DEBUG_MEM /*debug only*/
extern void *fb_app_malloc_ex(size_t x, const char *name, uint32_t line);
extern void fb_app_free_ex(void *x, const char *name, uint32_t line);
#define fb_app_malloc(x) fb_app_malloc_ex(x, __FILE__, __LINE__)
#define fb_app_free(x) fb_app_free_ex(x, __FILE__, __LINE__)
#else
#define fb_app_malloc(x) lv_mem_alloc((size_t)(x))
#define fb_app_free(x) lv_mem_free((void *)(x))
#endif
/*******************************************
 *              TYPEDEFS
 ******************************************/
/**
 * enum - fb progress phases
 *
 * @FB_PROGRESS_COUNTING: counting the job
 * @FB_PROGRESS_EXECUTING: executing the job
 * @FB_PROGRESS_DONE: finished the job successfully.
 * @FB_PROGRESS_FAILED: the job was failed and about to quit.
 */
enum fb_progress {
    FB_PROGRESS_COUNTING,
    FB_PROGRESS_EXECUTING,
    FB_PROGRESS_DONE,
    FB_PROGRESS_FAILED,

    FB_PROGRESS_UNKNOWN = 0xFFFF
};

/**
 * struct fb_volume_detail - details of volume
 * @device: local device or sd
 * @capacity: capacity in bytes
 * @used: used bytes
 * @avail: available bytes
 */
struct fb_volume_detail {
    enum app_fs_dev_id device;
    uint64_t capacity;
    uint64_t used;
    uint64_t avail;
};

/**
 * struct fb_folder_detail - details of a folder
 * @size: used size
 * @file_cnt: count of files
 * @subfolder_cnt: count of subfolders
 */
struct fb_folder_detail {
    uint32_t size;
    int file_cnt;
    int subfolder_cnt;
};

/**
 * struct fb_file_detail - details of a file
 * @size: used size
 * @is_img: image file or not
 * @img_w: image width
 * @img_h: image height
 */
struct fb_file_detail {
    uint32_t size;
    bool is_img;
    uint16_t img_w;
    uint16_t img_h;
};

/**
 * struct fb_entry_item - entry (file/folder) item
 * @type: type, file or folder
 * @next: pointer to the next file
 * @name: full path name of the file/subfolder
 */
struct fb_entry_item {
    enum app_fs_file_type type;
    struct fb_entry_item *next;
    char *name;
};

/**
 * struct fb_entry_list - entry(file/folder) list
 * @ref： reference cnt of the list
 * @total: total number of files/subfolders
 * @first: pointer to the first item
 * @last: pointer to the last item
 */
struct fb_entry_list {
    int ref;
    int total;
    struct fb_entry_item *first;
    struct fb_entry_item *last;
};
/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
#ifndef WIN32
/**
 * app_srv_fb_init - start fb service task
 *
 * Return: NA
 */
void app_srv_fb_init(void);

/**
 * app_srv_fb_destroy - destroy fb service task
 *
 * Return: NA
 */
void app_srv_fb_destroy(void);
#endif
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
                      void (*notify_cb)(int result));

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
                                              const char *name));

/**
 * app_srv_fb_scandir - Scan a folder to list the files and subfolders
 * @src: [IN] path of the folder
 * @is_recur: [IN] true to scan subfolders recursively, false for current folder
 * only
 * @list: [OUT] the scan result of list, including files and subfolders
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_scandir(const char *src, bool is_recur,
                       struct app_fs_scan_list_entry *list);

/**
 * app_srv_fb_free_scan_list - free scan list
 * @list: [IN] the scan list to free
 *
 * Return: NA
 *
 * Note: this function will free the child nodes only, list itself will
 * not be freed
 */
void app_srv_fb_free_scan_list(struct app_fs_scan_list_entry *list);

/**
 * app_srv_fb_create_path_node_list - create a node list which saves paths
 *
 * Return: NA
 * Note: each node saves a full path of a file/folder.
 */
lv_ll_t *app_srv_fb_create_path_node_list(void);

/**
 * app_srv_fb_path_node_list_add - add a path into path node list
 *
 * Return: NA
 */
bool app_srv_fb_path_node_list_add(lv_ll_t *list, const char *path);

/**
 * app_srv_fb_free_path_node_list - free the memory used by path node list
 *
 * Return: NA
 */
void app_srv_fb_free_path_node_list(lv_ll_t *list);

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
                    void *user_data);

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
                    void *user_data);

/**
 * app_srv_fb_rename - Rename a file or folder
 * @old: [IN] the full path of the source file or folder
 * @new: [IN] new name of the file or folder
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_rename(const char *old, const char *new);

/**
 * app_srv_fb_remove - Remove a file or folder
 * @path: [IN] the full path of the file or folder
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_remove(const char *path);

/**
 * app_srv_fb_is_valid_folder - Check if the folder is valid
 * @path: [IN] the full path of the folder
 *
 * Return: true for valid, false for not
 * Note: internal use only
 */
bool app_srv_fb_is_valid_folder(const char *path);

/**
 * app_srv_fb_get_volume_detail - get detail of the volume
 * @device_id: [IN] id
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: AIC_SUCCESS for success, otherwise to refer to aic_errno.h
 */
int app_srv_fb_get_volume_detail(enum app_fs_dev_id device_id,
                                 struct fb_volume_detail *detail);

/**
 * app_srv_fb_get_folder_detail - get detail of a folder
 * @path: [IN] the full path of the folder
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_get_folder_detail(const char *path,
                                  struct fb_folder_detail *detail);

/**
 * app_srv_fb_get_file_size - get size of a file
 * @path: [IN] the full path of the file
 * @detail: [OUT] pointer to a buffer to get detail
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_get_file_detail(const char *path,
                                struct fb_file_detail *detail);

/**
 * app_srv_fb_create_entry_list - create an entry list
 *
 * Return: pointer to the created entry list, NULL for failure
 */
struct fb_entry_list *app_srv_fb_create_entry_list(void);

/**
 * app_srv_fb_create_entry_list - create an entry list
 *
 * Return: the new reference count, -1 for failure
 */
int app_srv_fb_entry_list_add_ref(struct fb_entry_list *list);

/**
 * app_srv_fb_entry_list_release - release entry list
 * @list: [IN] the entry list to release
 *
 * Return: NA
 */
void app_srv_fb_entry_list_release(struct fb_entry_list *list);

/**
 * app_srv_fb_add_to_entry_list - add an entry into list
 * @list: [IN] entry list
 * @type: [IN] type of the entry
 * @path: [IN] full path of the entry
 *
 * Return: true for success, false for failure
 */
bool app_srv_fb_add_to_entry_list(struct fb_entry_list *list,
                                  enum app_fs_file_type type, const char *path);
#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __FILEBROWSER_SERVCIE_H__ */
