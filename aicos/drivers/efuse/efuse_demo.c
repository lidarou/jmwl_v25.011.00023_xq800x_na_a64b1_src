/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the efuse demo for user
*/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1cdb31
#define LOG_MODULE    AMOD_DRV

#include <aic_errno.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <efuse.h>
#include "efuse_phy.h"

#define AIC_EFUSE_TEST_DATA     0x7F6F5F4F

void demo_efuse_entry(void *input)
{
    u8 uid_demo[50];
    int i, ret, len;
    u32 data;

#ifdef CONFIG_SECURE_BOOT
    #include <secure_efuse_cfg.h>
    u8 pubkey_demo_src[32] = {
                  0x58, 0x69, 0x45, 0x7B, 0x34, 0x67, 0x30, 0x4F,
                  0x40, 0xD9, 0x3B, 0x0C, 0xD0, 0xA9, 0xDF, 0x40,
                  0xBC, 0x9A, 0xB0, 0xBC, 0x78, 0x1A, 0xA8, 0x23,
                  0xB6, 0x49, 0x85, 0xCD, 0x8,  0x54, 0xD2, 0x1E
                  };
    u8 pubkey_demo_dest[32] = {0};

    ret = aic_write_efuse_enob(pubkey_demo_src, PUBKEY_HASH_BLK_IDX, PUBKEY_HASH_BIT_IDX, PUBKEY_HASH_BITS_LENGTH);
    if (ret)
        aicx_error(0x0e8a50, "", "efuse write pubkey hash fail!!\n");

    ret = aic_read_efuse_enob(pubkey_demo_dest, PUBKEY_HASH_BLK_IDX, PUBKEY_HASH_BIT_IDX, PUBKEY_HASH_BITS_LENGTH);
    if (ret)
        aicx_error(0x12cf38, "", "efuse read pubkey hash fail!!\n");

    ret = memcmp(pubkey_demo_src, pubkey_demo_dest, sizeof(pubkey_demo_dest));
    if (!ret) {
        aicx_error(0x11f2bb, "", "efuse pubkey hash write & read consistency, enable secure boot!!\n");
        aic_efuse_block_write(SEC_BOOT_EN_BLK, BIT(SEC_BOOT_EN_BIT));
    } else {
        aicx_error(0x112e20, "", "efuse pubkey hash write & read not match, keep non-secure!!\n");
    }
#endif

    for (i = 0; i < 4; i++) {
#if 0
        ret = aic_efuse_block_write(i, AIC_EFUSE_TEST_DATA);
        if(ret)
            aicx_error(0x091683, "", "efuse write data fail!!\n");
#endif
        ret = aic_efuse_block_read(i, &data);
        if(ret)
            aicx_error(0x091683, "", "efuse write data fail!!\n");

        if(data != AIC_EFUSE_TEST_DATA)
            aicx_error(0x198d3a, "x", "efuse demo write/read fail 0x%x!!\n",data);
    }

    ret = aic_get_uid(uid_demo, &len);
    if (ret)
         aicx_error(0x02aab5, "", "efuse get uid fail!!");
}

int efuse_demo_init(void)
{
    aos_create_thread("demo_efuse", demo_efuse_entry, AOS_NULL, 1024, 8,
                      "demo_efuse", 16, AOS_AUTO_START);

    return 0;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(efuse_demo_init);
