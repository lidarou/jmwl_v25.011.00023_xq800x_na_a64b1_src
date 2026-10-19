/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This memory layout of ap sys.
 */
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x05c081
#define LOG_MODULE    AMOD_SYS

#include <aic_errno.h>
#include <aic_osa.h>
#include <aic_seg_alloc.h>
#include <flash_mem_config.h>
#include <mpu.h>
#include <sys_mem_api.h>
#include <sys_mem_config.h>

#ifdef CONFIG_FOTA_SUPPORT
#include <fota.h>

#if IS_CONF_ENABLED(USE_ADSP_REGION_TO_SAVE_DIFF)
/*
 * not define USE_ADSP_REGION_TO_SAVE_DIFF,
 * if (HEAP_REGION_SIZE - BASIC_SYS_MIN_HEAP_SIZE ) > ADSP_REGION_SIZE
 * should not use adsp region to save the basic diff.
 */
#if ((HEAP_REGION_SIZE - BASIC_SYS_MIN_HEAP_SIZE ) > ADSP_REGION_SIZE)
#error should disable the feature USE_ADSP_REGION_TO_SAVE_DIFF
#endif
#else
/*
 * not define USE_ADSP_REGION_TO_SAVE_DIFF,
 * FOTA_DIFF_PAC_MAX_SIZE + BASIC_SYS_MIN_HEAP_SIZE must <= HEAP_REGION_SIZE
 */
#if (FOTA_DIFF_PAC_MAX_SIZE + BASIC_SYS_MIN_HEAP_SIZE > HEAP_REGION_SIZE)
#error FOTA_DIFF_PAC_MAX_SIZE + BASIC_SYS_MIN_HEAP_SIZE must <= HEAP_REGION_SIZE
#endif
#endif /* USE_ADSP_REGION_TO_SAVE_DIFF */

/*
 * FOTA_AOS_BACKUP_SIZE must >= FLASH0_FOTA_BAK_SIZE
 */
#if (FOTA_AOS_BACKUP_SIZE < FLASH0_FOTA_BAK_SIZE + FOTA_EXT_IMG_BAK_ATTACH_SIZE)
#error FOTA_AOS_BACKUP_SIZE must >= FLASH0_FOTA_BAK_SIZE + FOTA_EXT_IMG_BAK_ATTACH_SIZE
#endif

/*
 * FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAMx_SIZE + FOTA_BAK_END_FLAG_SIZE must <= FLASH0_FOTA_BAK_SIZE
 */
#if (FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM1_SIZE + FOTA_BAK_END_FLAG_SIZE > FLASH0_FOTA_BAK_SIZE)
#error FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM1_SIZE + FOTA_BAK_END_FLAG_SIZE must <= FLASH0_FOTA_BAK_SIZE
#endif

#if (FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM2_SIZE + FOTA_BAK_END_FLAG_SIZE > FLASH0_FOTA_BAK_SIZE)
#error FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM2_SIZE + FOTA_BAK_END_FLAG_SIZE must <= FLASH0_FOTA_BAK_SIZE
#endif

#ifdef FLASH0_CPRAM3_SIZE
#if (FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM3_SIZE + FOTA_BAK_END_FLAG_SIZE > FLASH0_FOTA_BAK_SIZE)
#error FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM3_SIZE + FOTA_BAK_END_FLAG_SIZE must <= FLASH0_FOTA_BAK_SIZE
#endif

#if (FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM4_SIZE + FOTA_BAK_END_FLAG_SIZE > FLASH0_FOTA_BAK_SIZE)
#error FOTA_DIFF_PAC_MAX_SIZE + FLASH0_CPRAM4_SIZE + FOTA_BAK_END_FLAG_SIZE must <= FLASH0_FOTA_BAK_SIZE
#endif
#endif /* FLASH0_CPRAM3_SIZE */
#endif /* CONFIG_FOTA_SUPPORT */

#if IS_CONF_DISABLED(USE_ADSP_REGION_TO_SAVE_DIFF)
void *s_save_fota_diff_base;
#endif

extern int __extend_text_start__;
extern int __extend_rodata_end__;
extern int __ROM_segment_used_end__;

static u8 s_sys_heap[HEAP_REGION_SIZE] AIC_SECTION(".sys_heap");
static u8 s_sys_stack[ALL_STACK_SIZE] AIC_SECTION(".sys_stack");

const u32 g_sys_stack_limit = (u32)&s_sys_stack + ALL_STACK_SIZE;
const u32 g_init_heap_start = INIT_HEAP_BASE;
const u32 g_init_heap_limit = (INIT_HEAP_BASE + INIT_HEAP_SIZE - 4);

static AIC_SECTION(".extend_header") const EXT_IMG_HEADER s_extend_header =
{
    .rsrv_checksum = 0,
    .header_size = EXTEND_IMG_HEADER_SIZE,
    .magic = EXTEND_IMG_MAGIC,
    .extend_size = (u32)&__extend_rodata_end__ - FLASH0_AP_EXTEND_BASE,
    .max_diff_size = FOTA_DIFF_PAC_MAX_SIZE,
    .date = __DATE__,
    .time = __TIME__,
};

int sys_get_psram_region(PSRAM_RGT_T type, RG_DATA_T *rg)
{
    int ret = AIC_SUCCESS;

    switch (type) {
        case PSRAM_TOTAL_RG:
        rg->base = PSRAM_REGION_BASE;
        rg->size = PSRAM_REGION_SIZE;
        break;

        case PSRAM_SHM_RG:
        rg->base = SHMEM_REGION_BASE;
        rg->size = SHMEM_REGION_SIZE;
        break;

        case PSRAM_SHM_NV_RG:
        rg->base = SHM_NV_BASE;
        rg->size = SHM_NV_SIZE;
        break;

        case PSRAM_SHM_CMDLINE_RG:
        rg->base = SHM_CMDLINE_BASE;
        rg->size = SHM_CMDLINE_SIZE;
        break;

        case PSRAM_AP_RG:
        rg->base = AP_REGION_BASE;
        rg->size = AP_REGION_SIZE;
        break;

        case PSRAM_CP_RG:
        rg->base = CP_REGION_BASE;
        rg->size = CP_REGION_SIZE;
        break;

        case PSRAM_ADSP_RG:
        rg->base = ADSP_REGION_BASE;
        rg->size = ADSP_REGION_SIZE;
        break;

        default:
        ret = AIC_ESRCH;
    }

    return ret;
}

int sys_get_aoniram_region(AONIRAM_RGT_T type, RG_DATA_T *rg)
{
    int ret = AIC_SUCCESS;

    switch (type) {
        case AON_TOTAL_RG:
        rg->base = AONRAM_REGION_BASE;
        rg->size = AONRAM_REGION_SIZE;
        break;

        case AON_APVEC_RG:
        rg->base = AP_VECTOR_REGION_BASE;
        rg->size = AP_VECTOR_REGION_SIZE;
        break;

        case AON_CPVEC_RG:
        rg->base = CP_VECTOR_REGION_BASE;
        rg->size = CP_VECTOR_REGION_SIZE;
        break;

        case AON_SHM_RG:
        rg->base = AON_SHMEM_REGION_BASE;
        rg->size = AON_SHMEM_REGION_SIZE;
        break;

        case AON_ADSP_RG:
        rg->base = AON_ADSP_BASE;
        rg->size = AON_ADSP_SIZE;
        break;

        default:
        ret = AIC_ESRCH;
    }

    return ret;
}


int sys_get_sysmem_region(SYSMEM_RGT_T type, RG_DATA_T *rg)
{
    int ret = AIC_SUCCESS;

    switch (type) {
        case MEM_CODE_RG:
        rg->base = CODE_REGION_BASE;
        rg->size = CODE_REGION_SIZE;
        break;

        case MEM_HEAP_RG:
        rg->base = HEAP_REGION_BASE;
        rg->size = HEAP_REGION_SIZE;
        break;

        case MEM_RW_RG:
        rg->base = RW_REGION_BASE;
        rg->size = RW_REGION_SIZE;
        break;

        case MEM_SYS_STACK_RG:
        rg->base = AP_SYSSTACK_REGION_BASE;
        rg->size = AP_SYSSTACK_REGION_SIZE;
        break;

        default:
        ret = AIC_ESRCH;
    }

    return ret;
}

void sys_reconfig_extend_image(void)
{
    int ret;
    MPU_REGION_T rg =
        {MPU_RG(FLASH0_FOTA_BAK_BASE,  FLASH0_FOTA_BAK_SIZE),
         RDONLY, MEM_WTNWA,  NO_EXEC,   RG_ENABLE};

    ret = mpu_reconfig_region_by_area(&rg);
    AIC_ASSERT(ret == AIC_SUCCESS);
}

#ifdef CONFIG_FOTA_SUPPORT
bool sys_is_belong_extend_image(void *func)
{
    return (((ulong)(func) >= FLASH0_FOTA_BAK_BASE) &&
    ((ulong)(func) < (FLASH0_FOTA_BAK_BASE + FLASH0_FOTA_BAK_SIZE)));
}
#endif

void sys_get_extend_image_area(addr_t *addr_ptr, size_t *size_ptr)
{
    *addr_ptr = (addr_t)FLASH0_FOTA_BAK_BASE;
    *size_ptr = FLASH0_FOTA_BAK_SIZE;
}

void sys_get_ap_extend_img_actual_area(addr_t *addr_ptr, size_t *size_ptr)
{
    const EXT_IMG_HEADER *header = (const EXT_IMG_HEADER *)FLASH0_AP_EXTEND_BASE;
    size_t adjust_size;

    /* must skip the rsrv_checksum member. */
    *addr_ptr = (addr_t)FLASH0_AP_EXTEND_BASE;

    /* check if the extend header is valid, else size must set to 0. */
    if (header->magic == EXTEND_IMG_MAGIC &&
        header->extend_size < FLASH0_AP_EXTEND_SIZE) {
        adjust_size = (size_t)header->extend_size;
        /* size is odd, must pad 1 byte. */
        if (adjust_size % 2)
            adjust_size += 1;

        *size_ptr = adjust_size;
    } else {
        adjust_size = 0;
        *size_ptr = adjust_size;
    }
}

u16 sys_get_extend_img_cheksum_in_base(void)
{
    /* save on the last word of the aicos_ap.bin. */
    u32 addr = (u32)&__ROM_segment_used_end__;
    u16 checksum = *(u16 *)(addr);

    /* the tail checksum must ~, than cmpare with calcsum. */
    checksum = (~checksum & 0xffff);
    return checksum;
}

void sys_get_extend_img_actual_area(addr_t *addr_ptr, size_t *size_ptr)
{
#if FOTA_ONLY_BACK_UP_EXTEND_IMAGE
    sys_get_ap_extend_img_actual_area(addr_ptr, size_ptr);
    /* the real bin size need add 4 byte for fota backup. */
    *size_ptr += 4;
#else
    *addr_ptr = (addr_t)FLASH0_FOTA_BAK_BASE;
    *size_ptr = FLASH0_FOTA_BAK_SIZE;
#endif
}

void sys_get_ap_img_actual_area(addr_t *addr_ptr, size_t *size_ptr)
{
    /* save on the last word of the aicos_ap.bin. */
    u32 addr = (u32)&__ROM_segment_used_end__;

    *addr_ptr = (addr_t)FLASH0_AP_BASE;
#ifdef CONFIG_MULTI_BIN_SUPPORT
    *size_ptr = (addr - FLASH0_AP_BASE) + 2;
#else
    *size_ptr = (addr - FLASH0_AP_BASE);
#endif
}

void sys_reserve_save_basic_diff_image_ram(void)
{
#if IS_CONF_ENABLED(USE_ADSP_REGION_TO_SAVE_DIFF)
    /* use adsp region, do nothing. */
#else
    s_save_fota_diff_base = aos_dma_malloc(FOTA_DIFF_PAC_MAX_SIZE);
    AIC_ASSERT(s_save_fota_diff_base);
#endif
}

void sys_get_backup_fota_diff_img_area(addr_t *addr_ptr, size_t *size_ptr)
{
#if IS_CONF_ENABLED(USE_ADSP_REGION_TO_SAVE_DIFF)
    *addr_ptr = (addr_t)ADSP_REGION_BASE;
    *size_ptr = ADSP_REGION_SIZE;
#else
    *addr_ptr = (addr_t)s_save_fota_diff_base;
    *size_ptr = FOTA_DIFF_PAC_MAX_SIZE;
#endif
}

void sys_get_backup_extend_img_area(addr_t *addr_ptr, size_t *size_ptr)
{
    *addr_ptr = (addr_t)FOTA_AOS_BACKUP_BASE;
    *size_ptr = FOTA_AOS_BACKUP_SIZE;
}

#define MEM_128B_SHIFT  (7)
#define MEM_64B_SHIFT   (6)
#define MEM_32B_SHIFT   (5)
#define MEM_16B_SHIFT   (5)

struct aic_pool *s_exe_heap_pool;

void sys_create_basic_ext_heap(void)
{
    s_exe_heap_pool = aic_pool_create(MEM_32B_SHIFT);

    AIC_DUMMY_ASSERT(s_exe_heap_pool);

    /* add ext heap area to the pool. */
    aic_pool_add(s_exe_heap_pool, FOTA_AOS_EXT_HEAP_BASE, FOTA_AOS_EXT_HEAP_SIZE);

#if (!USE_ADSP_REGION_TO_SAVE_DIFF)
    aic_pool_add(s_exe_heap_pool, ADSP_REGION_BASE, ADSP_REGION_SIZE);
#endif
}

void *sys_basic_ext_heap_malloc(size_t size)
{
    void *mem;

    if (size == 0)
        return NULL;

    mem = (void*)aic_pool_alloc(s_exe_heap_pool, size);
    if (!mem)
        mem = aos_malloc(size);

    return mem;
}

void sys_basic_ext_heap_free(void *addr)
{
    if (aos_is_heap_ptr((u8 *) addr))
        aos_free(addr);
    else
        aic_pool_free(s_exe_heap_pool, (addr_t)addr);
}

