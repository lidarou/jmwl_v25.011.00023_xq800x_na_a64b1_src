

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x11dd9f
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_define.h"
#include "abup_device.h"
#include "abup_build_datetime.h"
#include "abup_display.h"
#include "abup_timer.h"
#include "abup_stdlib.h"
#include "abup_thread.h"
#include "abup_file.h"
#include "aic_srv_tele_sim.h"
#include <efuse.h>
#include <aic_osa.h>
#include <sysoff.h>
#include <aic_osa_ex.h>
#include <aic_version.h>
#include "abup_flash.h"



const char* const  s_abup_project_name = ABUP_PROJECT_NAME;
const char* const  s_abup_version_info = ABUP_BUILD_TIME;

ABUP_BOOL s_abup_em_mode = ABUP_FALSE;
ABUP_BOOL abup_fota_force_cv = ABUP_FALSE;
ABUP_BOOL abup_fota_force_ru = ABUP_FALSE;


#define ABUP_NET_RETRY_MAXCNT 5
#define ABUP_NET_SEPARATE_SIZE (64*1024)
#define ABUP_MONITOR_REST_MIN_MS    15*1000

abup_extern void abup_get_device_time(abup_time_struct* time);
abup_extern void Abup_CK_confirm_lsk(void);
abup_extern void abup_display_fota_status(abup_int32 status);

abup_extern void abup_fota_monitor_reset(void);
abup_extern void AlertNewVersion(void);
abup_extern abup_int32 abup_recvd_size(void);
abup_extern abup_int32 Abup_Remove_File(abup_wchar *file_path);


//当需要检测和下载， 下载和升级分开时，请开启此宏。
//并在相应的回调函数中做处理。
//#define ADUPS_FOTA_UI_ALERT

static struct fota_ui abup_fota_ui;
struct fota_ui * pfota_ui=&abup_fota_ui;

FotaVersionInfo fota_version;

FotaVersionInfo *pfota_version=&fota_version;

void abup_fota_reset_monitor(void)
{
    abup_DebugPrint("abup_fota_reset_monitor");

    abup_fota_monitor_reset();

    abup_start_timer(ABUP_MONITOR_REST_MIN_MS, abup_fota_reset_monitor, ABUP_FALSE);
}

void abup_fota_start_reset_monitor(void)
{
    abup_start_timer(ABUP_MONITOR_REST_MIN_MS, abup_fota_reset_monitor, ABUP_FALSE);
}

void report_fota_version(char *versionname,abup_int32 filesize)
{
    abup_DebugPrint("%s, versionname = %s, filesize = %d",__FUNCTION__,versionname,filesize);

    pfota_version->versionName=versionname;
    pfota_version->fileSize=filesize;
    if(pfota_ui->notify_version)
        pfota_ui->notify_version(pfota_version);
}

void ui_notify_version(FotaVersionInfo *version)
{
    abup_fota_start_reset_monitor();
    abup_DebugPrint("%s, version->versionName:%s\r\n",__FUNCTION__, version->versionName);

    AlertNewVersion();
}

void ui_notify_restart(void)
{
    abup_DebugPrint("%s reboot",__FUNCTION__);
}

void ui_notify_result(abup_int32 result)
{
    abup_DebugPrint("%s result=%d",__FUNCTION__,result);
}

void ui_notify_curr_download_percent(abup_int32 percent)
{
    abup_DebugPrint("%s, percent=%d\r\n",__FUNCTION__,percent);
}

void ui_fota_bind(
    fota_notify_version notify_version,
    fota_notify_curr_download_percent notify_curr_download_percent,
    fota_notify_restart notify_restart,
    fota_notify_result notify_result)
{

#if defined (ADUPS_FOTA_UI_ALERT)
    pfota_ui->haveui=1;
    pfota_ui->notify_version=notify_version;
    pfota_ui->notify_restart=notify_restart;
#endif

    pfota_ui->notify_curr_download_percent=notify_curr_download_percent;
    pfota_ui->notify_result=notify_result;

    abup_DebugPrint("%s bind success",__func__);
}

void abup_set_ui_alert(abup_int32 haveui)
{
    struct fota_ui * pfota_ui=&abup_fota_ui;
    pfota_ui->haveui=haveui;
}

abup_int32 abup_get_ui_alert(void)
{
    #if 1
    return 0;
    #else
    struct fota_ui * pfota_ui=&abup_fota_ui;
    return pfota_ui->haveui;
    #endif
}

void fota_ui_init(void)
{
    abup_DebugPrint("%s",__func__);

    abup_set_ui_alert(0);
    ui_fota_bind(ui_notify_version,ui_notify_curr_download_percent,ui_notify_restart,ui_notify_result);
}

abup_int32 abup_sim_ready(void)
{
    #if 1
    return E_ABUP_SUCCESS;//not need to check
    #else
    ts_sim_status_e status = TS_SIM_STATUS_NOT_READY;
    if(aic_srv_tele_sim_get_status_sync(0,&status) == 0)//jie kou zhi xing hui dump!
    {
        if(status == TS_SIM_STATUS_READY)
        {
            return E_ABUP_SUCCESS;
        }
        else{
            return E_ABUP_FAILURE;
        }
    }
    else{
        return E_ABUP_FAILURE;
    }
    #endif

}
ABUP_BOOL abup_use_test_mid = ABUP_FALSE;
abup_int32 abup_get_device_mid(abup_char* mid)
{
    ABUP_BOOL ret;
    int len = 0;

    #if 0//test only
    abup_memcpy(mid, "1234567890test1", 15);
    return 0;
    #endif

    if(!mid)
        return E_ABUP_FAILURE;

    if (abup_use_test_mid == ABUP_TRUE)
    {
        abup_memcpy(mid, "123456789", abup_strlen("123456789"));
    }
    else
    {
        if(abup_em_mode() == ABUP_TRUE)
        {
            abup_em_get_mid(mid);
            return E_ABUP_SUCCESS;
        }
        ret = aic_get_uid(mid, &len);
        if(ret)
            return E_ABUP_FAILURE;
    }
#if !defined(ABUP_NO_KEY)
    abup_hal_set_mid(mid);
#endif
    return E_ABUP_SUCCESS;
}


abup_char* abup_get_device_version(void)
{
#ifndef WIN32
    static abup_char verno_str[100];

    abup_memset(verno_str,0,100);
    abup_snprintf(verno_str, 100, "%s",AIC_SW_VERSION);
    return verno_str;

#else
    return "J603_1";
#endif
}

abup_char* abup_get_device_oem(void)
{
    return ABUP_FOTA_SERVICE_OEM;
}

abup_char* abup_get_device_model(void)
{
    return ABUP_FOTA_SERVICE_MODEL;
}

abup_char* abup_get_device_product_id(void)
{
    return ABUP_FOTA_SERVICE_PRODUCT_ID;
}

abup_char* abup_get_device_product_sec(void)
{
    return ABUP_FOTA_SERVICE_PRODUCT_SEC;
}

abup_char* abup_get_device_type(void)
{
    return ABUP_FOTA_SERVICE_DEVICE_TYPE;
}

abup_char* abup_get_device_platform(void)
{
    return ABUP_FOTA_SERVICE_PLATFORM;
}

abup_char* abup_get_device_network_type(void)
{
    return ABUP_DEVICE_NETWORKTYPE;
}

void abup_get_device_time(abup_time_struct* time)
{
    aos_systime sysdate = {0};

    aos_get_systime(&sysdate);
    if(time != NULL)
    {
        time->nYear = sysdate.year;
        time->nMonth = sysdate.month;
        time->nDay = sysdate.day;
        time->nHour = sysdate.hour;
        time->nMin = sysdate.minute;
        time->nSec = sysdate.second;

    }

}

abup_uint32 abup_get_utc_time_sec(abup_time_struct* time)
{
    abup_uint32 utc_time=0;

    if(time)
    {
        utc_time = abup_dt_curtime2utcsec(time);

        abup_DebugPrint("abup timestamp:%d; systime:%04d-%02d-%02d,%02d:%02d:%02d", \
            utc_time,
            time->nYear,time->nMonth,time->nDay,
            time->nHour,time->nMin,time->nSec);
    }
    else
        abup_DebugPrint("abup timestamp: time is null");

    return utc_time;
}

abup_uint8 abup_get_device_battery_level(void)
{
    return 0;
}

void abup_device_start_reboot(void)
{
    if (abup_is_s3_authenticated()) return;

    abup_DebugPrint("abup: reboot, please waiting...");

    sysoff_reset();

}

void* abup_memalloc(abup_int32 size)
{
    void *ptr=NULL;

    ptr = aic_malloc(size);
    return ptr;
}

void abup_memfree(void *p)
{
    if(p != NULL)
    {
        //abup_DebugPrint("abup_memfree:%X", p);
        aic_free(p);
        p = NULL;
    }
}

void* abup_memalloc_ext(abup_int32 size)
{
    void *ptr=NULL;
    ptr = abup_memalloc(size);
    return ptr;
}
void abup_memfree_ext(void *p)
{
    if(p != NULL)
        abup_memfree((void **)&p);
}

void abup_memclean(void)
{
#ifdef ABUP_DMEM_DEBUG
    abup_uint16 idx=0;
    abup_uint32 cursize=0;
    for(idx=0;idx<ABUP_DMEM_MAXNUM;idx++)
    {
        if(s_dmem_info.dmem[idx].addr != NULL)
        {
            abup_DebugPrint("abup_memclean curfind:0x%X, size:%d, idx:%d", \
                s_dmem_info.dmem[idx].addr, s_dmem_info.dmem[idx].size, idx);

            abup_memfree(s_dmem_info.dmem[idx].addr);

            cursize = s_dmem_info.dmem[idx].size;
            s_dmem_info.totalsize -= cursize;
            s_dmem_info.totalnum --;
            if(s_dmem_info.totalnum < 0)
                s_dmem_info.totalnum = 0;

            s_dmem_info.dmem[idx].addr = NULL;
            s_dmem_info.dmem[idx].size = 0;
        }
    }

    abup_DebugPrint("abup_memclean: totalnum:%d,totalsize:%d", \
        s_dmem_info.totalnum, s_dmem_info.totalsize);
#endif
}

void abup_meminfo(void)
{
#ifdef ABUP_DMEM_DEBUG
    abup_DebugPrint("abup_meminfo: totalnum:%d,totalsize:%d", \
        s_dmem_info.totalnum, s_dmem_info.totalsize);
#endif
}

abup_uint32 abup_entry_critical_timeout(void)
{
    return 30*1000;
}

void abup_entry_save_context(void)
{
    abup_DebugPrint_Ext("[ABUP_COMMON]:","abup_entry_save_context");
}
extern abup_uint8 abup_get_dl_state(void);
extern abup_uint8 abup_get_fota_state(void);

void abup_exit_restore_context(void)
{

    abup_DebugPrint_Ext("[ABUP_COMMON]:","abup_exit_restore_context abup_get_dl_state() is %d",abup_get_dl_state());
    if(abup_get_dl_state()==ABUP_DL_SUCCESS)
    {
        if (!abup_is_s3_authenticated()) abup_device_start_reboot();
    }
    else
    {
        abup_memclean();
    }
    abup_DebugPrint("abup_exit_restore_context");
    if(abup_fs_pathorfile_exist(abup_get_version_file_path(), 0)&&abup_get_fota_state()==STATE_RU)
    {
        abup_DebugPrint("restart RU timer!");
        abup_start_timer(60*60*1000, abup_upgrade_thread_on_ex, ABUP_FALSE);
        return;
    }
    abup_task_uninit();
}

void abup_get_device_data_point1(abup_char** name,abup_char** value)
{
    //static abup_char areacode[10]={'\0'};
    *name=NULL;//"areaCode";
    *value=NULL;//areacode;
}
/*****************************
**up_result 输入，标识升级结果
**:1   升级成功
**:其他  升级失败
******************************/
abup_int32 abup_get_upgrade_result(abup_int32 up_result)
{
    abup_DebugPrint("%s,up_result:%d",__FUNCTION__,up_result);

    if(pfota_ui->notify_result)
        pfota_ui->notify_result(up_result);
    return 0;
}

abup_uint32 abup_str2hex(const abup_char* pszStr, abup_uint32 u4Len)
{
    abup_uint32 u4Idx;
    abup_uint32 u4ReturnValue = 0;

    if ((pszStr == NULL) || (u4Len == 0))
    {
        return 0;
    }

    u4Len = (u4Len > 8) ? 8 : u4Len;

    for (u4Idx = 0;
        u4Idx < u4Len;
        u4Idx++)
    {
        if ((pszStr[u4Idx] >= '0') && (pszStr[u4Idx] <= '9'))
        {
            u4ReturnValue = u4ReturnValue << 4;
            u4ReturnValue += (abup_uint32)(abup_uint8)(pszStr[u4Idx] - '0');
        }
        else
        if ((pszStr[u4Idx] >= 'A') && (pszStr[u4Idx] <= 'F'))
        {
            u4ReturnValue = u4ReturnValue << 4;
            u4ReturnValue += (abup_uint32)(abup_uint8)(pszStr[u4Idx] - 'A' ) + 10;
        }
        else
        if ((pszStr[u4Idx] >= 'a') && (pszStr[u4Idx] <= 'f'))
        {
            u4ReturnValue = u4ReturnValue << 4;
            u4ReturnValue += (abup_uint32)(abup_uint8)(pszStr[u4Idx] - 'a') + 10;
        }
        else
        {
            return 0;
        }
    }

    return u4ReturnValue;
}

/****************************
*download_status  网络状态 DL_NETWORK_ERROR 下载出错  DL_NO_SPACE 空间不足  ABUP_DL_SUCCESS 下载成功
*abupstatus CK_NEW_VERSION表示有新的版本   UP_SUCCESS 表示上报成功
*abupfileSize  差分包的大小
* abupstate fota state -->STATE_CV  表示检测版本 STATE_DL 下载差分包版本  STATE_RD  上报下载差分包下载结果  STATE_RU 上报升级结果
*abupDlResult 下载差分包结果 abup_DL_SUCCESS 表示下载成功
*version_name 最新版本的版本名字
**************************/
void GetMainStates(abup_char *version_name,abup_int32 download_status,abup_int32 abupstatus,abup_int32 abupfileSize,abup_int32 abupstate,abup_uint8 abupDlResult)
{
    if((abupstate == STATE_DL)&&(abupfileSize == abup_recvd_size()))
    {
        //进入这里面表示下载差分包成功的
    }

    if((abupstate == STATE_DL)&&(abupDlResult != ABUP_DL_SUCCESS && abupDlResult != 0))
    {
        //进入这里面表示下载差分包失败的
        abup_display_fota_status(FOTA_STATE_ERROR);
    }

    if((abupstate == STATE_CV)&&(abupstatus==FOTA_SUCCESS))
    {
        //进入这里面表示有新的版本
    }

    if((abupstate == STATE_CV)&&(abupstatus==FOTA_CV_LAST_VERSION))
    {
        //进入这里面表示当前版本是最新版本
        abup_display_fota_status(FOTA_STATE_LAST_VER);
    }
    if(version_name !=NULL)
    abup_DebugPrint("GetMainStates,version_name:%s",version_name);
}

void abup_notify_fotastate(abup_int32 status)
{
    FotaDisplayState state = 0;
    abup_DebugPrint("abup_notify_fotastate: %d", status);
    if (status == STATE_CV)
        state = FOTA_STATE_CV;
    else if (status == STATE_DL)
        state = FOTA_STATE_DL;
    else if (status == STATE_RD)
        state = FOTA_STATE_RD;
    else if (status == STATE_RU)
        state = FOTA_STATE_RU;
    else if (status == STATE_RG)
        state = FOTA_STATE_RG;
    else if (status == STATE_KY)
        state = FOTA_STATE_KY;
    else
        return;
    abup_display_fota_status(state);
}

void abup_process_status_cb(abup_int32 status,abup_uint8 fota_status)
{
    abup_DebugPrint("abup_process_status_cb: %d,%d", status,fota_status);

    switch(status)
    {
        case FOTA_SUCCESS:
            break;

        case FOTA_RG_SIGN_ERROR:
        case FOTA_CV_UNREG_DEVICE:
            Abup_Remove_File(abup_get_login_file_path());
            break;

        default:
            break;
    }
}

void abup_fota_atcmd_custom(abup_char *cmd, abup_char *para0, abup_char *para1,abup_char *para2, abup_write func)
{
}

void abup_em_set_mode(ABUP_BOOL mode)
{
    s_abup_em_mode = mode;
}

ABUP_BOOL abup_em_mode(void)
{
    return s_abup_em_mode;
}

void abup_em_set_mid(abup_char* mid)
{
}

void abup_em_get_mid(abup_char* mid)
{
}

abup_int16 abup_net_retry_maxtime(void)
{
    return ABUP_NET_RETRY_MAXCNT;
}

abup_uint32 abup_net_separate_size(void)
{
    return ABUP_NET_SEPARATE_SIZE;
}

void abup_download_ratio(abup_int32 percent)
{
    if(pfota_ui->notify_curr_download_percent)
    {
        pfota_ui->notify_curr_download_percent(percent);
    }

    abup_DebugPrint("%s,percent=%d\r\n",__FUNCTION__, percent);
}

ABUP_BOOL abup_is_s3_authenticated(void)
{
    return ABUP_FALSE;
}

abup_char* abup_get_fota_server(void)
{
    return "iotapi.abupdate.com";
}

abup_uint32 abup_get_fota_port(void)
{
    return 80;
}

abup_char* abup_get_fota_dl_server(void)
{
    return "";
}

abup_uint32 abup_get_fota_dl_port(void)
{
    return 80;
}

void abup_set_force_cv_flag(ABUP_BOOL flag)
{
    abup_fota_force_cv = flag;
}
ABUP_BOOL abup_get_force_cv_flag(void)
{
    return abup_fota_force_cv;
}

void abup_set_force_ru_flag(ABUP_BOOL flag)
{
    abup_fota_force_ru = flag;
}
ABUP_BOOL abup_get_force_ru_flag(void)
{
    return abup_fota_force_ru;
}
abup_int32 abup_hal_get_key_version(void)
{
    return ABUP_KEY_VERSION;
}


