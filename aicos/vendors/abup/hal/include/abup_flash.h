#ifndef _ABUP_FLASH_H_
#define _ABUP_FLASH_H_


#define ABUP_BLOCK_SIZE  0x1000

#define ABUP_DELTA_PART_NAME  "fota"

typedef struct ABUP_FLASH_PARTATION
{
    abup_uint32 sect_size;
    abup_uint32 SploadSize;
    abup_uint32 Spload_Addr;
} ABUP_FLASH_PARTATIONT_T,*ABUP_FLASH_PARTATION_PTR;

//
#ifndef ABUP_MID_MAXLEN
#define ABUP_MID_MAXLEN            31
#endif
#define ABUP_CODE_MAXLEN           32
#define ABUP_DEVICEID_MAX_LEN      31
#define ABUP_DEVICESECRET_MAX_LEN  63

#if defined(ABUP_SHA256 )
#define ABUP_DOWNLOAD_CODE_LEN     32
#else
#define ABUP_DOWNLOAD_CODE_LEN     16
#endif
#define ABUP_ALIGN(x) __attribute__ ((aligned (x)))

typedef struct
{
    abup_uint8 inited:4;
    abup_uint8 serverNO:4;
    abup_uint16 update_result;
    abup_uint8 mid[ABUP_MID_MAXLEN + 1];
    abup_uint8 abup_code[ABUP_CODE_MAXLEN];
#if defined(ABUP_UB)||defined(ABUP_BOOT0_ENABLE)&&(ABUP_BOOT0_ENABLE == 1)
    abup_uint32 crc;
#endif
#if defined(ABUP_UB)||defined(ABUP_UA)||defined(ABUP_OPENFLASH)||defined(ABUP_OPENFOTA)||defined(ABUP_TLV_SERVER)||defined(ABUP_BOOT0_ENABLE)&&(ABUP_BOOT0_ENABLE == 1)
    void* ShareFunc[8];
#endif
    abup_int8 bootupdate;
#if defined(ABUP_TLV_SERVER)
    abup_backup_struct backup;
#endif
    abup_uint8 deviceId[ABUP_DEVICEID_MAX_LEN + 1];
    abup_uint8 deviceSecret[ABUP_DEVICESECRET_MAX_LEN + 1];
    abup_uint32 index;
    abup_uint32 index_max;
    abup_uint32 delta_id;
    abup_char download_code[ABUP_DOWNLOAD_CODE_LEN + 1];
    abup_uint32 default_module;
#if defined(ABUP_WIFI_SSID_PWD)
    abup_char wifi_ssid[ABUP_WIFI_SSID_MAXLEN + 1];
    abup_char wifi_pwd[ABUP_WIFI_PWD_MAXLEN + 1];
#endif
#if defined(ABUP_AUTO_TEST_FOTA)
    abup_autotest_struct autotest;
#endif
} ABUP_ALIGN(1) abup_update_struct;



typedef enum {
    ABUP_FILETYPE_APP,
    ABUP_FILETYPE_PATCH,
    ABUP_FILETYPE_BACKUP,
    ABUP_FILETYPE_PATCH_INFO,
    ABUP_FILETYPE_APP_BACKUP,
    ABUP_FILETYPE_BL,
    ABUP_FILETYPE_BL_BACKUP,
    ABUP_FILETYPE_END
} ABUP_PEER_FILETYPE;

abup_extern abup_int32 abup_write_flash(abup_uint32 start,abup_uint8* data_ptr, abup_uint32 len);
abup_extern ABUP_BOOL AbupUseFlash(void);
abup_extern  abup_int32 abup_read_flash(abup_uint32 addr,abup_uint8* data_ptr, abup_uint32 len);
abup_extern  abup_uint32 abup_addr_to_block(abup_uint32 addr);
abup_extern  abup_uint32 abup_init_flash(void);
abup_extern  void abup_init_block(abup_uint32 block_idx);
abup_extern void abup_erase_delata(void);
abup_extern void abup_erase_update_info(void);
abup_extern abup_int32 abup_read_update_info_flash(abup_uint32 addr,abup_uint8* data_ptr, abup_uint32 len);
abup_extern abup_uint32 AbupGetBlockSize(void);
abup_update_struct* abup_hal_rst_fota(void);
ABUP_BOOL abup_hal_set_update(abup_update_struct *abup_update);
abup_extern void abup_hal_set_download_result(abup_uint16 result);
abup_uint8* abup_hal_get_mid(void);
void abup_hal_set_mid(abup_char *value);
abup_uint16 abup_hal_get_update_result(void);
void abup_rst_key(void);
abup_extern void abup_get_flash_info(void);
abup_extern abup_update_struct* abup_hal_init_update(void);
abup_extern void abup_flash_get_partation(void);
abup_extern abup_uint32 abup_flash_delta_size(void);

#endif

