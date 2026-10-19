/*****************************************************************************/
/*                                                                           */
/*Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.       */
/*                                                                           */
/*****************************************************************************/

#ifndef __PLATFORM_PLATFORM_H__
#define __PLATFORM_PLATFORM_H__

#include "inttypes.h"
#include "io.h"
#include "secure/secure_boot.h"

struct platform_core;
struct boot_parameters;

typedef int(* BOOT_MOD_ENTERPOINT)(void);
typedef int (* SECBOOT_AUTH)(struct platform_core *p);
typedef int (* SECBOOT_IMGATTR_CHECK)(char *);

enum download_type {
    DL_SPI,
    DL_USB,
    DL_UART0,
    DL_UART1,
    DL_MAX,
};

/*****************************************************************************************
* Note:
* If update BOOT_MOD here, May update BOOT_REASON_E in aic_boot_mode.h.
******************************************************************************************/
enum BOOT_MOD {
    DOWNLOAD_BOOT = 0,
    NORMAL_BOOT,
    FOTA_BOOT,
    BASIC_BOOT,
    CHARGE_BOOT,
    GXEM_BOOT,
    SYSDUMP_BOOT,
    ALARM_UPS_BOOT,
    ALARM_RTC_BOOT,
    WARMRST_BOOT,
    AUTODL_BOOT,
    LP_SYSDUMP_BOOT,
    BOOT_MOD_MAX,
};

struct image_header {
    uint32_t magic_number;
    uint32_t pubkey_size;
    uint32_t image_offset;
    uint32_t image_size;
    uint32_t image_certificates_offset;
    uint32_t image_certificates_size;
    uint32_t image_sign_offset;
    uint32_t image_sign_size;
    uint32_t key_certificates_offset;
    uint32_t key_certificates_size;
    uint32_t key_sign_offset;
    uint32_t key_sign_size;
};

struct secure_boot {
    bool secure_boot_enable;
    SECBOOT_AUTH secboot_authorization;
    SECBOOT_IMGATTR_CHECK img_attr_check;
    struct image_header *header;
    void *pubkey;
};

struct boot_parameters {
    uint32_t boot_mode;
    uint32_t dl_type;
    uint8_t pubkey_hash[HASH_BYTE_LEN];
    uint32_t SW_version;
    uint32_t version_id_check;
};

struct platform_core {
    struct boot_parameters *bp;
    char *name;
    uint32_t boot_mode;
    uint32_t boot_mode_cnt;
    uint8_t *next_img;
    BOOT_MOD_ENTERPOINT boot_mode_enter[BOOT_MOD_MAX];
    struct secure_boot secure_boot;
    uint32_t timer_count;
    u32 (*get_tick)(void);
    u32 next_img_compressed;
};

static inline u32 systick_default_get(void)
{
    return 0;
}

extern struct platform_core platform;

int platform_tick_register(u32 (*tick)(void));
int platform_boot_mode_enterpoint_register(
                    BOOT_MOD_ENTERPOINT enter,
                    enum BOOT_MOD mod);
uint32_t platform_get_dl_type(void);
int platform_secboot_init(SECBOOT_AUTH auth);
int platform_do_boot(void);
void platform_init(void);

#endif /* __PLATFORM_PLATFORM_H__ */
