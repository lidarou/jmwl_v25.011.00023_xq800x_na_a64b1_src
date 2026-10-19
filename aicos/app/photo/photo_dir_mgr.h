/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

#ifndef __PHOTO_DIR_MGR_H__
#define __PHOTO_DIR_MGR_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include "aic_type.h"

/**
 *  DESCRIPTION
 *
 *    This file defines the apis of photo directory manager.
 */

/*********************
 *      DEFINES
 *********************/
/* photo directory */
#define PHOTO_DISK_LETTER "C:"
#define PHOTO_DIR "/Images/"
#define PHOTO_DIR_LEN 24
#define PHOTO_NAME_LEN 12
#define PHOTO_PATH_NAME_LEN (PHOTO_DIR_LEN + PHOTO_NAME_LEN)

/**
 * typdef (void *scan_nofity_calllback)() - callback prototype for scanning
 *                                          result
 * notification of photo directory.
 */
typedef void (*photo_nofity_callback)(void *file_data, void *user_data);

/**
 * photo_dir_init() - initialize the photo directory according to the
 *                    specified path.
 * @path: path the full path of image file stored
 * @cb: callback for photo files scanning result notification
 * Return: 0 means success, -1 means failure.
 */
int photo_dir_init(char *path, photo_nofity_callback cb);

/**
 * photo_dir_deinit() - release the resource in memory for managing current
 *                      directory.
 * @arg1: none.
 * @arg2: none.
 * Return: none.
 */
void photo_dir_deinit(void);

/**
 * photo_delete() - delete a specified photo from the directory.
 * @name: name of file to delete from the directory.
 * @arg2: none.
 * Return:  0 means success, -1 means failure.
 */
int photo_delete(const char *name);

/**
 * photo_rename() - rename a specified photo file.
 * @name: the old name of image file to rename.
 * @new_name: the new name of image file to rename.
 * Return:  0 means success, -1 means failure.
 */
int photo_rename(const char *name, const char *new_name);

/**
 * photo_write() - write a new photo to the directory.
 * @name: name of file to write to the directory.
 * @data: image data.
 * @len: length of data
 * Return:  0 means success, -1 means failure.
 */
int photo_write(const char *name, const u8 *data, u32 len);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __PHOTO_DIR_MGR__ */
