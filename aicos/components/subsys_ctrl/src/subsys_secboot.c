
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the subsys secboot source file.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0cbf07
#define LOG_MODULE    AMOD_SUBSYS

#include <aic_assert.h>
#include <aic_bitmap.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <fmd.h>
#include <subsys_api.h>
#include <subsys_sec.h>
#include <secure/security.h>

size_t subsys_get_head_size(void)
{
#ifdef CONFIG_SECURE_BOOT
    return aic_secboot_header_size();
#else
    return 0;
#endif
}

size_t subsys_sec_get_head_size(void)
{
    return aic_secboot_header_size();
}

#define SEC_BOOT_TAIL_SIZE      1024

size_t subsys_sec_get_tail_size(void)
{
    return SEC_BOOT_TAIL_SIZE;
}

#ifdef CONFIG_SECURE_BOOT
void subsys_sec_verify_and_start(u8 sys, u32 region_num, subsys_region_t *region_tab)
{
#ifndef CONFIG_SECURE_VERIFY_ON_RAM
    u32 handle = 0;
#endif
    struct subsys_img *sys_img;
    u32 i, cnt = 0, img_addr = 0;
    subsys_region_t *tbl;
    u8 *sec_head = NULL;
    int ret;

    sys_img = aos_malloc(sizeof(*sys_img));
    AIC_DUMMY_ASSERT(sys_img)

    for (i = 0, tbl = region_tab; i < region_num; i++, tbl++) {
        /* skip the no sec verify img. */
        if (!(SUBSYS_REGION_ATTR_SEC_VERIFY & tbl->region_attr))
            continue;

        AIC_ASSERT(cnt < MAX_SEC_VERIFY_IMG_CNT);
        sys_img->attr[cnt] = tbl->region_attr;

#ifdef CONFIG_SECURE_VERIFY_ON_RAM
        img_addr = (u32)tbl->region_base;
        sec_head = tbl->sec_head;
#else
        ret = fmd_open(tbl->region_name, &handle);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

        ret = fmd_get_physical_address(handle, 0, &img_addr);
        AIC_DUMMY_ASSERT(ret == AIC_SUCCESS);

        (void)fmd_close(handle);
        sec_head = NULL;
#endif /* CONFIG_SECURE_VERIFY_ON_RAM */

        sys_img->img[cnt] = (u8 *)img_addr;
        sys_img->head[cnt] = sec_head;

        aicx_info(0x1cb37b, "sx", "sec verify add %s, addr=0x%x\n", tbl->region_name, img_addr);
        cnt++;
    }

    sys_img->num = cnt;
    ret = aic_secboot_auth_img_combo(sys_img);
    aicx_info(0x08d30b, "d", "aic_secboot verify ret = %d\n", ret);

    /* free the head arr */
    for (i = 0; i < cnt; i++) {
        if (sys_img->head[i])
            aos_free(sys_img->head[i]);
    }
    aos_free(sys_img);
    if (ret == AIC_SUCCESS)
        return;

    while (1) {
        if (aos_is_user_thread_context()) {
            aos_sleep(1000*3600);
        }
    }
}


int subsys_sec_init(void)
{
    return aic_secboot_init();
}
AIC_EARLY_INIT_EXPORT(subsys_sec_init);

int subsys_sec_diag_cmd_init(void)
{
    return diag_secure_cmd_init();
}
AIC_COMP_P2_INIT_EXPORT(subsys_sec_diag_cmd_init);

#endif /* CONFIG_SECURE_BOOT */

