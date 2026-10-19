/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This memory api header file.
 */

#ifndef __SYS_MEM_API_H__
#define __SYS_MEM_API_H__

#include <aic_type.h>
#include <sys_mem_config.h>

typedef enum {
    PSRAM_TOTAL_RG = 0,
    PSRAM_SHM_RG,
    PSRAM_SHM_NV_RG,
    PSRAM_SHM_CMDLINE_RG,
    PSRAM_AP_RG,
    PSRAM_CP_RG,
    PSRAM_ADSP_RG,
} PSRAM_RGT_T;

typedef enum {
    AON_TOTAL_RG = 0,
    AON_APVEC_RG,
    AON_CPVEC_RG,
    AON_SHM_RG,
    AON_ADSP_RG,
    AON_PHYBUF_RG,
} AONIRAM_RGT_T;

typedef enum {
    MEM_CODE_RG = 0,
    MEM_HEAP_RG,
    MEM_RW_RG,
    MEM_SYS_STACK_RG,
} SYSMEM_RGT_T;

typedef struct {
    addr_t base;
    size_t size;
} RG_DATA_T;

#define EXTEND_DATE_MAX_LEN         12
#define EXTEND_TIME_MAX_LEN         8

typedef struct __attribute__((packed)) {
    u16     rsrv_checksum; /* reserve for diff.exe add check sum here. */
    u16     header_size;
    u32     magic;
    u32     extend_size;
    u32     max_diff_size;
#ifdef CONFIG_COMPRESS_EXTEND_AP
    u32     load_address;
#endif
    char    date[EXTEND_DATE_MAX_LEN];
    char    time[EXTEND_TIME_MAX_LEN];
} EXT_IMG_HEADER;

#define EXTEND_IMG_MAGIC            0xABCDDCBA
#define EXTEND_IMG_HEADER_SIZE      sizeof(EXT_IMG_HEADER)
#define EXTEND_RSRV_CHECKSUM_SIZE   2

int sys_get_psram_region(PSRAM_RGT_T type, RG_DATA_T *rg);
int sys_get_aoniram_region(AONIRAM_RGT_T type, RG_DATA_T *rg);
int sys_get_sysmem_region(SYSMEM_RGT_T type, RG_DATA_T *rg);

/*
 * The extend images contains all the images in fota bakup group.
 */

/**
 * sys_is_belong_extend_image -
 * if the specified addr is belong the extend image range
 *
 * @addr: any addr.
 *
 * @return: TRUE ar FALSE
 */
#ifdef CONFIG_FOTA_SUPPORT
bool sys_is_belong_extend_image(void *func);
#else
#define sys_is_belong_extend_image(addr) (FALSE)
#endif

/**
 * sys_reconfig_extend_image - Reconfig the mpu attribute of the ext image,
 * In basic mode, The extend image can't be exe again, can be read and write.
 */
void sys_reconfig_extend_image(void);

/**
 * sys_get_extend_image_area -
 * Get the extend image area.
 */
void sys_get_extend_image_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_get_extend_img_actual_area -
 * Get the extend image actual area..
 */
void sys_get_extend_img_actual_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_get_ap_extend_img_actual_area -
 * Get the ap_extend.bin image actual area.
 */
void sys_get_ap_extend_img_actual_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_get_extend_img_cheksum_in_base -
 * Get the checksum of extend image that save in base image.
 */
u16 sys_get_extend_img_cheksum_in_base(void);

/**
 * sys_get_ap_img_actual_area -
 * Get the aicos_ap.bin image actual area.
 */
void sys_get_ap_img_actual_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_reserve_save_basic_diff_image_ram -
 * reserve a area on ram for save basic diff image.
 *
 */
void sys_reserve_save_basic_diff_image_ram(void);

/**
 * sys_get_backup_fota_diff_img_area -
 * Get the reserve area for backup the basic diff image.
 *
 */
void sys_get_backup_fota_diff_img_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_get_backup_extend_img_area -
 * Get the reserve area for backup extend images except the basic diff image.
 *
 */
void sys_get_backup_extend_img_area(addr_t *addr_ptr, size_t *size_ptr);

/**
 * sys_create_basic_ext_heap -
 * Will use a area of cp to create a mem pool.
 *
 * Notify: only can be called after stop cp in basic mode.
 */
void sys_create_basic_ext_heap(void);

void *sys_basic_ext_heap_malloc(size_t size);
void sys_basic_ext_heap_free(void *addr);

#ifdef CONFIG_FOTA_BY_MCU
void sys_get_save_mcu_fota_pkg_area(addr_t *addr_ptr, size_t *size_ptr);
#endif

/**
 * fota_ext_malloc -
 * malloc a mem from ext heap pool.
 *
 * Notify: only can be used after stop cp in basic mode.
 */
#define fota_ext_malloc(size)   sys_basic_ext_heap_malloc(size)

/**
 * fota_ext_free -
 * put a mem back to ext heap pool.
 *
 * Notify: only can be used after stop cp in basic mode.
 */
#define fota_ext_free(addr)     sys_basic_ext_heap_free(addr)

#endif /* __SYS_MEM_API_H__ */

