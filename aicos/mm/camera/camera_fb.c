/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x02e51e
#define LOG_MODULE    AMOD_CAMERA

#include <aic_osa.h>
#include <aic_errno.h>
#include <tx_api.h>
#include <camera_fb.h>

#define FB_ALIGN_SIZE        32U

/* TX_BLOCK_POOL camera_fb_pool;*/

#if 0
static u32 camera_fb_pool_memory[(CAMERA_FB_NUM * (CAMERA_FB_SIZE + sizeof(VOID *))) / sizeof(u32)];
#endif
static u32 camera_fb_pool_memory[CAMERA_MEM_SIZE / sizeof(u32)] __attribute__((aligned(CACHE_LINE_SIZE))); ;
struct camera_fb_info g_camera_fb_info[CAMERA_BUF_NUM];

/* return the framebuffer number */
u8 camera_fb_alloc(u32 fb_size)
{
    u8 fb_num;
    int i;
    u32 align_sz, ptr = (u32)camera_fb_pool_memory;

    AIC_ASSERT(fb_size <= sizeof(camera_fb_pool_memory));
    if (fb_size % FB_ALIGN_SIZE) {
        align_sz = fb_size / FB_ALIGN_SIZE;
        align_sz = align_sz * FB_ALIGN_SIZE + FB_ALIGN_SIZE;
    } else
        align_sz = fb_size;

    fb_num = sizeof(camera_fb_pool_memory)/fb_size;
    if (fb_num > CAMERA_BUF_NUM) {
        fb_num = CAMERA_BUF_NUM;
    }

    for (i = 0; i < fb_num; i++) {
        g_camera_fb_info[i].fb_addr = ptr;
        g_camera_fb_info[i].fb_size = fb_size;
        g_camera_fb_info[i].fb_empty = TRUE;
        g_camera_fb_info[i].fb_flushing = FALSE;
        ptr = ptr + align_sz;
    }

    aicx_info(0x126285, "", "allocate camera framebuffer success!\n");
    return fb_num;
}

#if 0
int camera_fb_alloc(int fb_num, u32 fb_size)
{
    int i;
    u32 ptr = camera_fb_pool_memory;

    for (i = 0; i < fb_num; i++) {
        g_camera_fb_info[i].fb_addr = ptr;
        g_camera_fb_info[i].fb_size = fb_size;
        g_camera_fb_info[i].fb_empty = TRUE;
        g_camera_fb_info[i].fb_flushing = FALSE;
        ptr = ptr + fb_size;
    }

    aicx_info(0x126285, "", "allocate camera framebuffer success!\n");

    return AIC_SUCCESS;
}

int camera_fb_alloc(int fb_num, u32 fb_size)
{
    int i;
    u32 status;
    unsigned char *ptr;

    status = tx_block_pool_create(&camera_fb_pool, "Camera Framebuffer Pool", CAMERA_FB_SIZE,
                                  camera_fb_pool_memory, sizeof(camera_fb_pool_memory));
    if (status) {
            aicx_error(0x17aec4, "", "create camera pool failed!\n");
        tx_block_pool_delete(&camera_fb_pool);
        return status;
    }

    memset(g_camera_fb_info, 0, sizeof(g_camera_fb_info));

    for (i = 0; i < fb_num; i++) {
        status = tx_block_allocate(&camera_fb_pool, (void **)&ptr, TX_NO_WAIT);
        g_camera_fb_info[i].fb_addr = (u32 *)ptr;
        g_camera_fb_info[i].fb_size = fb_size;
        g_camera_fb_info[i].fb_empty = TRUE;
        g_camera_fb_info[i].fb_flushing = FALSE;
        if (status) {
            aicx_error(0x1a38ff, "d", "allocate block %d from camera pool failed!\n", i);
            goto fb_block_release;
        }
    }

    aicx_info(0x126285, "", "allocate camera framebuffer success!\n");

    return status;

fb_block_release:
    for (; i >= 0; i--)
        tx_block_release((void *)g_camera_fb_info[i].fb_addr);

    return status;
}
#endif

struct camera_fb_info *camera_get_fb_info(int fb_id)
{
    return &g_camera_fb_info[fb_id];
}

