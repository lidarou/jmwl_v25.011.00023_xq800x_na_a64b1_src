
/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is the subsys loader source file.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1bb9b4
#define LOG_MODULE    AMOD_SUBSYS

#include <aic_assert.h>
#include <aic_boot_mode.h>
#include <aic_cmdline.h>
#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_size.h>
#include <busmon_inc.h>
#include <dmc_mpu_inc.h>
#include <drv_file.h>
#include <hlzma.h>
#include <subsys_api.h>
#include <subsys_drv.h>
#ifdef CONFIG_NVRAM_SUPPORT
#include <aic_nvram_ap.h>
#endif
#include <subsys_sec.h>
#include <subsys_header.h>
#include <subsys_loader.h>
#include <subsys_default_ldif.h>
#ifdef CONFIG_SECURE_BOOT
#include <secure_efuse_cfg.h>
#include <secure/secure_boot.h>
#endif

#ifndef WIN32
#include <cache.h>
#include <mpu.h>
#include <slp_inc.h>
#else
#define dcache_clean_by_range(base, size)
#endif

#ifdef CONFIG_MCX_SUPPORT
#include <aic_mcx.h>
#endif

#if defined(CONFIG_CP_PSRAM_REMAP) || (QSPI_NOR_DEVICE_NUM == 2)
#include <psram_mem_config.h>
#endif

#ifdef CONFIG_SECURE_BOOT
#include <efuse.h>
#endif

#define SUBSYS_STRCPY2(str, str1, str2, size) \
    strncpy(str, str1, size); \
    strncpy(str + strlen(str), str2, size - strlen(str))

#define COMPRESSED_FLAG_LEN    4

#ifdef CONFIG_MCX_SUPPORT
static void subsys_loader_enable_mcx(subsys_loader_t *loader, bool enable)
{
    if (loader->sys == AIC_SYS_CP) {
        mcx_eb_set(enable);
        mcx_ap_force_clear_event();
    }
}
#else
#define subsys_loader_enable_mcx(loader, enable)
#endif

static void subsys_loader_image_to_ram(char *filename, bool lzma, u32 offset,
                                       void *load_base, size_t load_size);

int subsys_mpu_config_region(u32 region_attr)
{
    int index;

#ifndef WIN32
    MPU_REGION_T rg;

    rg.ap = RDWR;
    rg.cbtex = MEM_WBWA;
    rg.xn = NO_EXEC;
    rg.rg_stat = RG_ENABLE;


    if (region_attr == SUBSYS_REGION_CPRAM) {
        rg.base_addr = CP_REGION_BASE;
        rg.end_addr = CP_REGION_BASE + CP_REGION_SIZE - 1;
    }
#ifdef CONFIG_SUPPORT_ADSP
    else if (region_attr == SUBSYS_REGION_ADSP_RAM) {
        rg.base_addr = ADSP_REGION_BASE;
        rg.end_addr = ADSP_REGION_BASE + ADSP_REGION_SIZE - 1;
    } else if (region_attr == SUBSYS_REGION_ADSP_DTCM) {
        rg.base_addr = ADSP_DTCM_ADDR;
        rg.end_addr = ADSP_DTCM_ADDR + ADSP_DTCM_SIZE - 1;
    }
#endif
    else {
        return AIC_EINVAL;
    }

    index = mpu_dynamic_conf_region(&rg, FALSE);
    AIC_DUMMY_ASSERT(index >= 0);
#else
    index = 0;
#endif

    return index;
}

static void subsys_loader_adjust(subsys_loader_t *loader)
{
    u32 i, region_num;
    subsys_region_t *tbl;

    region_num = loader->region_num;
    tbl = loader->region_tbl;

    aicx_debug(0x047234, "sd", "%s region num=%d, load info:\n", loader->name, region_num);

    for (i = 0; i < region_num; i++, tbl++) {
#ifdef CONFIG_CP_PSRAM_REMAP
        /* cp psram has remap, need get the original psram addr. */
        if (tbl->region_attr == SUBSYS_REGION_CPRAM)
            tbl->region_base = CP_REMAP_TO_PSRAM(tbl->region_base);
#endif

        aicx_info(0x12515a, "sxxx", "region:%s, attr=0x%08x, base=0x%08x, size=0x%x\n",
                  tbl->region_name, tbl->region_attr,
                  tbl->region_base, tbl->region_size);

    }
}

void subsys_loader_reorder_tbl(int num, loader_region_t *src, loader_region_t **array)
{
    int i, j;

    /* copy the first array element. */
    array[0] = src++;

    for (i = 1; i < num; i++, src++) {
        /* move to the last array element */
        j = i - 1;

        /* find the src insert pos. */
        while (j >= 0 && src->region_base > array[j]->region_base) {
            /* move back array[j] */
            array[j + 1] = array[j];
            j--;
        }

        /* insert src */
        array[j + 1] = src;
    }
}

#ifdef CONFIG_SUPPORT_ADSP
subsys_load_info * subsys_get_adsp_ldif(char *head_bin, subsys_loader_t *loader)
{
    return NULL;
}
#endif

subsys_load_info *subsys_get_cp_ldif(char *head_bin, subsys_loader_t *loader)
{
    size_t sec_hd_size = subsys_sec_get_head_size();
    subsys_load_info *header;
    u32 offset = 0;
    u8 *buffer;
    size_t head_size;
    bool lzma;

    aicx_debug(0x0db829, "s", "correct cp loader info, file=%s.\n", head_bin);

    /* try get cp  header first. */
    head_size = sizeof(*header) + sec_hd_size;
    buffer = (u8 *)CP_REGION_BASE;

#ifdef CP_MAIN_IMG_IS_LZMA
    lzma = TRUE;
#else
    lzma = FALSE;
#endif

#ifdef CONFIG_SECURE_BOOT
#ifdef CONFIG_SEC_COMP_FIRST_THEN_SIGN
    /* must pass the sec header. */
    if (lzma)
        offset += subsys_sec_get_head_size();
#endif
#endif

    subsys_loader_image_to_ram(head_bin, lzma, offset, buffer, head_size);

    /* header = buffer or header = bufer +  sec_hd_size. */
    header = (subsys_load_info *)(buffer + sec_hd_size);
    if (SUBSYS_HEAD_MAGIC != header->magic)
        header = (subsys_load_info *)buffer;

    /* check magic fail, assert */
    AIC_ASSERT(SUBSYS_HEAD_MAGIC == header->magic);

    return header;
}


subsys_load_info * subsys_get_ldif_from_header(u8 sys, subsys_loader_t *loader)
{
   switch (sys) {
    case AIC_SYS_CP:
        return subsys_get_cp_ldif(CP_MAIN_IMG_BIN, loader);

#ifdef CONFIG_SUPPORT_ADSP
    case AIC_SYS_ADSP:
        return subsys_get_adsp_ldif(ADSP_MAIN_IMG_BIN, loader);
#endif

    default:
        break;
    }

    return NULL;
}

void subsys_loader_create_ldif(u8 sys, subsys_loader_t *loader, subsys_load_info *ldif)
{
    subsys_region_t *tbl;
    loader_region_t *load_tbl = NULL;
    u32 *boot_code = NULL;
    u16 tbl_size, boot_size = 0;
    u8 index, region_num = 0;
    loader_region_t *order_tbl[MAX_REGIN_NUM];

    switch (sys) {
    case AIC_SYS_CP:
        AIC_DUMMY_ASSERT(ldif);
        region_num = ldif->region_num;
        load_tbl = ldif->region;
        boot_code = ldif->boot_code;
        while (boot_code[boot_size] && boot_size < MAX_BOOT_CODE)
            boot_size++;

        boot_size = boot_size *sizeof(u32);
        break;

#ifdef CONFIG_SUPPORT_ADSP
    case AIC_SYS_ADSP:
        load_tbl = adsp_load_tab;
        region_num = ARRAY_SIZE(adsp_load_tab);
        boot_code = NULL;
        boot_size = 0;
        break;
#endif

    default:
        return;
    }

    AIC_ASSERT(region_num <= MAX_REGIN_NUM);

    /*
     * because some bin has a sec header, may override the the lower addr region
     * so must reorder the load table before loader every region,
     * the higher addr region need to be load firstly,
     */
    subsys_loader_reorder_tbl((int)region_num, load_tbl, order_tbl);

    /* malloc a table and init it. */
    tbl_size = region_num * sizeof(*tbl);
    tbl = aos_zalloc(tbl_size);
    AIC_DUMMY_ASSERT(tbl);
    loader->region_tbl = tbl;
    loader->bootcode_size = boot_size;
    loader->region_num = 0;

    /* copy region tbl */
    for (index = 0; index < region_num; index++, tbl++) {
        load_tbl = order_tbl[index];
        memcpy(tbl->region_name, load_tbl->region_name, sizeof(tbl->region_name));
        tbl->region_base = load_tbl->region_base;
        tbl->region_size = load_tbl->region_size;
        tbl->region_attr = load_tbl->region_attr;
        loader->region_num++;
    }

    /* copy boot code */
    if (boot_code) {
        loader->bootcode = aos_malloc(boot_size);
        AIC_DUMMY_ASSERT(loader->bootcode);
        memcpy(loader->bootcode, boot_code, boot_size);
    } else {
        loader->bootcode = NULL;
    }
}

#define subsys_loader_bootcode(subsys, loader) \
    subsys_write_region(subsys, (u8 *)loader->bootcode, loader->bootcode_size)

static void subsys_loader_image_to_ram(char *filename, bool lzma, u32 offset,
                                       void *load_base, size_t load_size)
{
    u32 handle;
    int ret;

    ret = fmd_open(filename, &handle);
    if (ret != AIC_SUCCESS) {
        aicx_error(0x12a544, "s", "%s is not exsist.\n", filename);
        return;
    }

    if (lzma) {
        u32 in_addr;

        /* skip the offset, get the file memory addr */
        fmd_get_physical_address(handle, offset, &in_addr);

#if (QSPI_NOR_DEVICE_NUM == 2) && defined(CP_COMPRESS_TO_EXT_FLASH)
        u32 part_size;

        fmd_get_partition_size(handle, &part_size);
#endif

        /* Auto gen by tool, don't change it. */
        aicx_info(0x11a319, "sxxx", "LZMA load %s, load_base=0x%x, offset=0x%x, in_addr=0x%x.\n",
                  filename, load_base, offset, in_addr);

#if (QSPI_NOR_DEVICE_NUM == 2) && defined(CP_COMPRESS_TO_EXT_FLASH)
        /* copy data from ext flash to cp rw ram */
        in_addr = CP_LOAD_BASE;
        AIC_ASSERT(AIC_SUCCESS == fmd_read(handle, offset, part_size, (u8 *)in_addr));
        dcache_clean_by_range((u32)in_addr, part_size);
#endif

#ifdef CONFIG_DRIVER_HLZMA
        /*
         * Before lzma load, invalid the load region cache to ensure
         * the data consistency between the lzma and the cpu.
         */
        in_addr += COMPRESSED_FLAG_LEN;
        dcache_inv_by_range(DCLINE_ALIGN((u32)load_base),
                            DCLINE_GRANULARITY(load_base, load_size));
        AIC_ASSERT(AIC_SUCCESS == hlzma_dec_polling(in_addr, (u32)load_base));
#endif
    }
    else {
        /* skip the offset */
        AIC_ASSERT(AIC_SUCCESS == fmd_read(handle, offset, load_size, load_base));
        /* here must flush the cache. */
        dcache_clean_by_range((u32)load_base, load_size);
    }

    (void)fmd_close(handle);

}

void subsys_loader_normal(subsys_region_t *region, char *filename)
{
    bool lzma = region->region_attr & SUBSYS_REGION_ATTR_LZMA;
#ifdef CONFIG_SECURE_BOOT
    size_t sec_tail_size = subsys_sec_get_tail_size();
#endif
    size_t sec_hd_size = subsys_sec_get_head_size();
    bool be_memmove = FALSE;
    size_t offset = 0;
    u32 sys_base = 0;
    size_t load_size;
    u8 *load_base;

    /* init load base and load size. */
#ifdef WIN32
    load_base = aos_malloc(region->region_size);
    AOS_ASSERT(load_base);
#else
    load_base = (u8 *)region->region_base;
#endif
    load_size = region->region_size;

    /* adjust the load_base and load size for sec. */
    if (region->region_attr & SUBSYS_REGION_ATTR_SEC_VERIFY) {
        load_base -= sec_hd_size;
        load_size += sec_hd_size;
        if (region->region_attr & SUBSYS_REGION_ATTR_CP)
            sys_base = CP_REGION_BASE;
#ifdef CONFIG_SUPPORT_ADSP
        else if (region->region_attr & SUBSYS_REGION_ATTR_ADSP)
            sys_base = ADSP_REGION_BASE;
#endif

#if defined (CONFIG_MEMMOVE_BASE_REGION_IMG) || defined(CONFIG_SECURE_BOOT)
        /* sys base region, must load twice. */
        if (region->region_base == sys_base) {
            be_memmove = TRUE;
            load_base = (u8 *)region->region_base;
        }
#endif

#ifdef CONFIG_SECURE_BOOT
        load_size += sec_tail_size;

#ifndef CONFIG_SECURE_VERIFY_ON_RAM
        /* has verify on flash, need skip the sec header to load. */
        be_memmove = FALSE;
        offset += sec_hd_size;
        load_size -= sec_hd_size;
        load_base = (u8 *)region->region_base;
#endif
#endif
    }

    /* first load */
    subsys_loader_image_to_ram(filename, lzma, offset, load_base, load_size);

    /* save img sec header. */
#ifdef CONFIG_SECURE_VERIFY_ON_RAM
    if (region->region_attr & SUBSYS_REGION_ATTR_SEC_VERIFY) {
        aicx_debug(0x0434bf, "s", "%s: pre papare the sec header.\n", region->region_name);
        region->sec_head = aos_malloc(sec_hd_size);
        if (region->sec_head)
            memcpy(region->sec_head, load_base, sec_hd_size);
    }
#endif

    /* memmove the image pass the sec header. */
    if (be_memmove) {
#ifdef CONFIG_SECURE_BOOT
        struct image_header *sec_head = (struct image_header *)load_base;
        u32 cpy_size = sec_head->image_size + sec_tail_size;
#else
        u32 cpy_size = load_size - sec_hd_size;
#endif

        aicx_info(0x0d87ad, "xx", "memmove base=0x%x, size=0x%x.\n", load_base, cpy_size);
        memmove((void *)load_base, (void *)(load_base + sec_hd_size), cpy_size);
        dcache_clean_by_range((u32)load_base, cpy_size);
    }

#ifdef WIN32
    aos_free(load_base);
#endif
}

void *subsys_loader_create(u8 sys, char *name)
{
    subsys_loader_t *loader;
    subsys_load_info *ldif;
    int index;

    aicx_debug(0x098a3d, "s", "loader create %s\n", name);

    loader = aos_zalloc(sizeof(*loader));
    AIC_DUMMY_ASSERT(loader);

    loader->sys = sys;
    loader->name = name;

    index = subsys_mpu_config_sys(sys);

#ifdef WIN32
    ldif = &s_cp_load_info;
#else
    ldif = subsys_get_ldif_from_header(sys, loader);
#endif

    /* create loader info. */
    subsys_loader_create_ldif(sys, loader, ldif);

    AIC_DUMMY_ASSERT(index >= 0);
    subsys_mpu_unconfig_region(index);

    /* adjust the laoder info for verify image. */
    subsys_loader_adjust(loader);

    /* create subsys */
    loader->subsys = subsys_create(sys, loader->region_num, loader->region_tbl);
    AIC_DUMMY_ASSERT(loader->subsys);

    return loader;
}

void subsys_loader_release(subsys_loader_t *loader)
{
    subsys_destroy(loader->subsys);
    aos_free(loader->region_tbl);
    if (loader->bootcode)
        aos_free(loader->bootcode);
    aos_free(loader);
}

void subsys_loader_load(subsys_loader_t *loader)
{
    bool task_context = aos_is_user_thread_context();
    subsys_region_t *tbl;
    void *subsys = loader->subsys;
    int index;
    u32 i;
#ifdef CONFIG_SECURE_BOOT
    u32 secure_bit = 0;
#endif

    if (task_context) {
#ifndef WIN32
        aic_get_wakelock();
#endif
        /* basic mode can't call it. */
        if (!aic_boot_mode_is_basic()) {
            gx_monitor_suspend();
            gx_mpu_mon_suspend();
        }

        (void)subsys_write_lock(subsys);
    }

#ifdef CONFIG_SECURE_BOOT
    aic_efuse_block_read(SEC_BOOT_EN_BLK, &secure_bit);
    aicx_info(0x0de11d, "x", "secure_bit = 0x%x.\n", secure_bit);
#endif

#ifdef CONFIG_SEC_COMP_FIRST_THEN_SIGN
    /* verify on flash befor loader to ram. */
    if (secure_bit & BIT(SEC_BOOT_EN_BIT))
        subsys_sec_verify_and_start(loader->sys, loader->region_num,
                                    loader->region_tbl);
#endif

    for (i = 0, tbl = loader->region_tbl; i < loader->region_num; i++, tbl++) {
        /* skip the noload img. */
        if (SUBSYS_REGION_ATTR_NOLOAD & tbl->region_attr) {
#ifdef CONFIG_SECURE_VERIFY_ON_RAM
            if (tbl->region_attr & SUBSYS_REGION_ATTR_SEC_VERIFY) {
                size_t sec_hd_size = subsys_sec_get_head_size();

                aicx_debug(0x0434bf, "s", "%s: pre papare the sec header.\n", tbl->region_name);
                tbl->sec_head = aos_malloc(sec_hd_size);
                if (tbl->sec_head) {
                    memcpy(tbl->sec_head, 
                           (void *)(tbl->region_base - sec_hd_size), sec_hd_size);
                }
            }
#endif
            continue;
        }

        /* set write region */
        (void)subsys_set_write_region(subsys, tbl->region_name);

        index = subsys_mpu_config_region(tbl->region_attr);

        if (SUBSYS_REGION_ATTR_NV & tbl->region_attr) {
            //subsys_nv_process(tbl, tbl->region_name);
#ifdef CONFIG_NVRAM_SUPPORT
            nvram_ap_load_data();
#endif
        } else if (SUBSYS_REGION_ATTR_BOOTCODE & tbl->region_attr) {
            subsys_loader_bootcode(subsys, loader);
        } else {
            subsys_loader_normal(tbl, tbl->region_name);
        }

        if (index >= 0)
            subsys_mpu_unconfig_region(index);
    }

#ifdef CONFIG_SECURE_VERIFY_ON_RAM
    /* verify on ram after loader to ram. */
    if (secure_bit & BIT(SEC_BOOT_EN_BIT))
        subsys_sec_verify_and_start(loader->sys, loader->region_num,
                                    loader->region_tbl);
#endif

    /* flush the cmdline befor start cp. */
    aic_cmdline_flush();

    if (task_context) {
        (void)subsys_write_unlock(subsys);
        /* basic mode can't call it. */
        if (!aic_boot_mode_is_basic()) {
            gx_monitor_resume(NULL);
            gx_mpu_mon_resume(NULL);
        }
#ifndef WIN32
        aic_put_wakelock();
#endif
    }
}

void subsys_loader_start(subsys_loader_t *loader)
{
    subsys_loader_enable_mcx(loader, TRUE);
    (void)subsys_drv_start(loader->subsys);
}

void subsys_loader_power_on(subsys_loader_t *loader)
{
    subsys_loader_enable_mcx(loader, TRUE);
    (void)subsys_drv_poweron(loader->subsys);
}

void subsys_loader_power_off(subsys_loader_t *loader)
{
    (void)subsys_drv_poweroff(loader->subsys);
    subsys_loader_enable_mcx(loader, FALSE);
}

static void subsys_loader_dump_tabl(u8 sys, const subsys_dump_t *dump_table, u8 num)
{
    const subsys_dump_t *tbl = dump_table;
    int index;
    u8 i;

    for (i = 0; i < num; i++, tbl++) {
        index = subsys_mpu_config_region(tbl->region_attr);
        alog_dump_start(sys, tbl->region_name, tbl->region_base, tbl->region_size);
        alog_dump(sys, (void *)tbl->region_base, tbl->region_size);
        alog_dump_finish(sys);
        if (index >= 0)
            subsys_mpu_unconfig_region(index);
    }
}

void subsys_loader_dump_sys(u8 sys)
{
    const subsys_dump_t *dump_table;
    u8 num;

    switch(sys) {
    case AIC_SYS_CP:
        dump_table = cp_dump_tab;
        num = ARRAY_SIZE(cp_dump_tab);
        break;

#ifdef CONFIG_SUPPORT_ADSP
    case AIC_SYS_ADSP:
        dump_table = adsp_dump_tab;
        num = ARRAY_SIZE(adsp_dump_tab);
        break;
#endif

    default:
        return;
    }

    subsys_loader_dump_tabl(sys, dump_table, num);
}

int subsys_mpu_config_sys(u8 sys)
{
    int index;

    switch(sys) {
    case AIC_SYS_CP:
        index = subsys_mpu_config_region(SUBSYS_REGION_CPRAM);
        break;

#ifdef CONFIG_SUPPORT_ADSP
    case AIC_SYS_ADSP:
        index = subsys_mpu_config_region(SUBSYS_REGION_ADSP_RAM);
        break;
#endif

     default:
        index = -1;
        break;
    }

    return index;
}

