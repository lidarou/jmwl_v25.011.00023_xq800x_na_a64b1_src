/******************************************************************************/
/*                                                                            */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/
/**
 *  DESCRIPTION
 *
 *    This file is enter of factory reset
 */
/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <aic_list.h>
#include "fs_api.h"
#include "aic_md_api.h"
#include "aic_md_id.h"
#include "app_log.h"
#include "aic_nvram_ap.h"
#include "aic_srv_tmr.h"
#include "factoryreset.h"
#include <sys/mount.h>


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
static srv_func_callback g_app_fac_callback = NULL;
static srv_tmr_handle         g_reset_timer = NULL;

/**********************
 *   STATIC FUNCTIONS
 **********************/

static int factoryreset_volume(char *p_volume_name)
{
    int ret = 0;

    struct fs_mount_info dev_info = {0};

    if(NULL == p_volume_name) {
        APP_LOGE("[%s],p_volume_name is null", __func__);
        ret = -1;
        return ret;
    }

    APP_LOGI("[%s],p_volume_name is %s",__func__, p_volume_name);

    fs_getmntinfo(p_volume_name, &dev_info);
    APP_LOGI("[%s],target is %s, devpath = %s, fs_type = %s",__func__, 
            dev_info.target, dev_info.devpath, dev_info.fstype);
    ret = mkfs(dev_info.fstype, dev_info.target);

    APP_LOGI("[%s],format complete,ret value = %d",__func__, ret);

    return ret;
}

static int32_t factoryreset_timer_callback(void *p_param, uint32_t size)
{
    int ret = 0;
    factoryreset_msg_t         ret_msg = {0};

    /* 1. factoryreset nv */
    factoryreset_nv();
    /* 2. foamrat disk C */
    ret = factoryreset_volume("C:");

    APP_LOGI("[%s],format result = %d.", __func__, ret);

    /* if factoryreset complete,call app callback*/
    if(0 == ret) {
        ret_msg.fac_msg = FACTORYRESET_COMPLETE;
    } else {
        ret_msg.fac_msg = FACTORYRESET_FAIL;
    }

    /* notify caller */
    g_app_fac_callback(&ret_msg, sizeof(factoryreset_msg_t));
    g_app_fac_callback = NULL;
    aic_srv_tmr_destroy(g_reset_timer);
    g_reset_timer = NULL;

    APP_LOGI("[%s],factoryreset complete.", __func__);
    return 0;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void factoryreset_nv(void)
{
    /* depends on cp alive*/
    aic_nvram_ap_restore_factory();

    aic_md_restore_factory();
    /*disk c,nv part*/
    aic_md_userdata_restore_factory();
}


/**
 * factoryreset_except_folder(char *p_volume_name,char *p_folder_name).
 * @arg1: volume_name:C disk(C:);D disk(D:).
 * @arg2: file_name:file name,can't delete file.
 * Return: none.
 */
void factoryreset_except_folder(char *p_volume_name,char *p_folder_name)
{
    int ret         = AIC_SUCCESS;
    char def_dir[3] = "C:";
    DIR         *dp = NULL;
    struct dirent *p_dir_entry = NULL;
    struct stat statbuf;

    if(NULL == p_folder_name) {
        APP_LOGE("[%s],folder name is null",__func__);
    } else {
        APP_LOGI("[%s],folder name is %s",__func__,p_folder_name);
    }

    if(NULL == p_volume_name) {
        APP_LOGE("[%s],volume name is null,use C disk",__func__);
    } else {
        strcpy(def_dir,p_volume_name);
    }

    APP_LOGI("[%s],dir name= %s",__func__, def_dir);
    /* don't delete D disk */
    if(0 == strcmp(def_dir, "D:")){
        APP_LOGE("[%s], it is d disk,not factoryreset",__func__);
        return;
    }

    /* open C/E disk */
    dp = opendir(def_dir);
    if(NULL != dp) {
        chdir(def_dir);
        while(NULL != (p_dir_entry = readdir(dp))) {
            if (-1 == stat(p_dir_entry->d_name, &statbuf)) {
                /* no this file */
                ret = AIC_ENOENT;
                APP_LOGE("[%s],folder name is null",__func__);
                break;
            }
            if (S_ISDIR(statbuf.st_mode)) { /* a folder */
                /* Found a directory, but ignore . and ..*/
                if (0 == strcmp(".", p_dir_entry->d_name) ||
                    0 == strcmp("..", p_dir_entry->d_name)) {
                    APP_LOGE("[%s],file name is dot",__func__);
                    continue;
                }

               if((NULL != p_folder_name) && 0 == strcmp(p_dir_entry->d_name,p_folder_name)) {
                    APP_LOGI("[%s],find file name",__func__);
                    continue;
                }else {
                    APP_LOGE("[%s],delete folder",__func__);
                    rmdir(p_dir_entry->d_name);
                }
            }
        }
    }else {
        APP_LOGE("[%s],dp is null", __func__ );
    }

    /* close dir */
    closedir(dp);

    /* create default dir */
    if(0 == strcmp(def_dir,"C:")) {
        /*if reset C disk,need create default dir */
        fs_create_default_dir(def_dir);
    }
}

/**
 * factoryreset_sync(srv_func_callback _fac_callback).
 * @arg1: _fac_callback: callback.
 * Return: none.
 */
void factoryreset_async(srv_func_callback _app_fac_callback)
{
    srv_tmr_param_t reset_timer_param = {0};

    if(NULL == _app_fac_callback) {
        APP_LOGE("[%s],_fac_callback is null", __func__);
        return;
    } else {
        g_app_fac_callback = _app_fac_callback;
    }

    /* 1. create timer */
    reset_timer_param.p_name     = "factoryreset";
    reset_timer_param.is_repeat  = false;
    reset_timer_param.time_out   = 100;
    reset_timer_param.p_usr_data = NULL;
    g_reset_timer = aic_srv_tmr_create(&reset_timer_param, factoryreset_timer_callback);
}
