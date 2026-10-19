/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the basic Application Interface (API) of file
 *    browser(FB) app.  All app prototypes and data structure
 *    definitions are defined in this file.
 */

#ifndef __FILEBROWSER_H__
#define __FILEBROWSER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************
 *              INCLUDES
 ******************************************/
#include <lvgl.h>

/*******************************************
 *              DEFINES
 ******************************************/
#ifndef FB_APP_NAME
#define FB_APP_NAME "filebrowser"
#endif

/*******************************************
 *              GLOBAL PROTOTYPES
 ******************************************/
/**
 * register_app_filebrowser - register file browser app into app manager
 *
 * Return: NA
 */
void register_app_filebrowser(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* __FILEBROWSER_H__ */
