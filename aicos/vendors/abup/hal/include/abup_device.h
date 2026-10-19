#ifndef _ABUP_DEVICE_H_
#define _ABUP_DEVICE_H_

typedef struct{
    char * versionName; //version name
    abup_int32   fileSize;
} FotaVersionInfo;
#ifndef ABUP_KEY_VERSION
#define ABUP_KEY_VERSION            2
#endif
typedef void(*fota_notify_version)(FotaVersionInfo *version);
typedef void(*fota_notify_curr_download_percent)(abup_int32 percent);
typedef void(*fota_notify_restart)(void);
typedef void(*fota_notify_result)(abup_int32 result);

struct fota_ui{
    abup_int32 haveui;
    fota_notify_version notify_version;
    fota_notify_curr_download_percent notify_curr_download_percent;
    fota_notify_restart notify_restart;
    fota_notify_result notify_result;
};

void ui_fota_bind(
    fota_notify_version notify_version,
    fota_notify_curr_download_percent notify_curr_download_percent,
    fota_notify_restart notify_restart,
    fota_notify_result notify_result);

extern struct fota_ui * pfota_ui;
extern FotaVersionInfo *pfota_version;

void abup_set_ui_alert(abup_int32 haveui);
abup_int32 abup_get_ui_alert(void);
void fota_ui_init(void);


abup_extern  abup_int32 abup_sim_ready(void);

abup_extern abup_int32 abup_get_device_mid(abup_char* mid);

abup_extern abup_char* abup_get_device_version(void);

abup_extern abup_char* abup_get_device_oem(void);

abup_extern abup_char* abup_get_device_model(void);

abup_extern abup_char* abup_get_device_product_id(void);

abup_extern abup_char* abup_get_device_product_sec(void);

abup_extern abup_char* abup_get_device_type(void);

abup_extern abup_char* abup_get_device_platform(void);

abup_extern abup_char* abup_get_device_network_type(void);

abup_extern void abup_get_device_time(abup_time_struct* time);
abup_extern abup_uint32 abup_get_utc_time_sec(abup_time_struct* time);


abup_extern abup_uint8 abup_get_device_battery_level(void);

abup_extern void abup_device_start_reboot(void);

abup_extern void* abup_memalloc(abup_int32 size);
abup_extern void abup_memfree(void *p);
abup_extern void* abup_memalloc_ext(abup_int32 size);
abup_extern void abup_memfree_ext(void *p);

abup_extern void abup_get_device_data_point1(abup_char** name,abup_char** value);
abup_extern void abup_get_device_data_point2(abup_char* name,abup_char* value);
abup_extern void abup_get_device_data_point3(abup_char* name,abup_char* value);
abup_extern void abup_get_device_data_point4(abup_char* name,abup_char* value);
abup_extern void abup_get_device_data_point5(abup_char* name,abup_char* value);

abup_extern void abup_process_status_cb(abup_int32 status,abup_uint8 fota_status);
abup_extern void abup_em_set_mode(ABUP_BOOL mode);
abup_extern ABUP_BOOL abup_em_mode(void);
abup_extern void abup_em_set_mid(abup_char* mid);
abup_extern void abup_em_get_mid(abup_char* mid);
abup_extern abup_uint32 abup_net_separate_size(void);
abup_extern ABUP_BOOL abup_is_s3_authenticated(void);
abup_extern abup_uint32 abup_get_fota_port(void);
abup_extern abup_char* abup_get_fota_server(void);
abup_extern abup_uint32 abup_get_fota_dl_port(void);
abup_extern abup_char* abup_get_fota_dl_server(void);
abup_extern void abup_download_ratio(abup_int32 percent);
abup_extern abup_int32 abup_get_upgrade_result(abup_int32 up_result);
abup_extern void report_fota_version(char *versionname,abup_int32 filesize);
abup_extern void abup_exit_restore_context(void);
abup_extern abup_uint32 abup_entry_critical_timeout(void);
abup_extern void abup_fota_atcmd_custom(abup_char *cmd, abup_char *para0, abup_char *para1,abup_char *para2, abup_write func);
abup_extern void abup_set_force_cv_flag(ABUP_BOOL flag);
abup_extern ABUP_BOOL abup_get_force_cv_flag(void);
abup_extern void abup_set_force_ru_flag(ABUP_BOOL flag);
abup_extern ABUP_BOOL abup_get_force_ru_flag(void);
abup_int32 abup_hal_get_key_version(void);

#endif
