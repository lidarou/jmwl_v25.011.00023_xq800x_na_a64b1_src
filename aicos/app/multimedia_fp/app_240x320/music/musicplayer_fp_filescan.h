

/*****************************************************************************/
/*                                                                           */
/*   Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file declare api about music files scan.
 */
#ifndef __MUSICPLAYER_FP_FILESCAN_H__
#define __MUSICPLAYER_FP_FILESCAN_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define     FILE_SCAN_URL_MAX_LENGTH (256)
    typedef void(*scan_cb_t) (void * data,char * url);

    void scan_dir_with_ext_filter(const char * path,const char * file_extension[],
        int         filter_num,scan_cb_t cb,void * cb_data);

#ifdef __cplusplus
} /*extern "C"*/


#endif

#endif /*__MUSICPLAYER_FP_FILESCAN_H__*/

