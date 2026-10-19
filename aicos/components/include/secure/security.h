/*****************************************************************************/
/*                                                                           */
/*      Copyright 2021 by AIC Corporation. All rights reserved.              */
/*                                                                           */
/*****************************************************************************/

#ifndef __SECURE_SECURITY_H__
#define __SECURE_SECURITY_H__

#include "aic_io.h"
#ifndef WIN32
#include <secure_cfg.h>
#endif

#define MAX_SEC_VERIFY_IMG_CNT  20

struct subsys_img {
    int num;
    char *name;
    u8 *img[MAX_SEC_VERIFY_IMG_CNT];
    u8 *head[MAX_SEC_VERIFY_IMG_CNT];
    u32 attr[MAX_SEC_VERIFY_IMG_CNT];
};

static inline u32 aic_secboot_header_size(void)
{
#ifndef WIN32
    return IMAGE_HEADER_SIZE;
#else
    return 0;
#endif
}

u32 aic_secboot_header_size(void);
int aic_secboot_auth_img_combo(struct subsys_img *subsys);
int aic_secboot_init(void);
int diag_secure_cmd_init(void);

#if 0
#ifdef CONFIG_SECURE_BOOT_CORE
u32 aic_secboot_header_size(void);
int aic_secboot_auth_img_combo(struct subsys_img *subsys);
#else
static inline u32 aic_secboot_header_size(void) { return 0; }
int aic_secboot_auth_img_combo(struct subsys_img *subsys) { return AIC_SUCCESS; }
#endif
#endif

#endif /* __SECURE_SECURITY_H__ */
