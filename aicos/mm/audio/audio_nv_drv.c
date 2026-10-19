/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**---------------------------------------------------------------------------*
 ** Description
 **
 ** This file defines the audio nv base driver interface.
 **---------------------------------------------------------------------------*/
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e00d7
#define LOG_MODULE    AMOD_AUDIO

#include "alog_api.h"
#include "aic_errno.h"
#include "aic_init.h"
#include "audio_nv_drv.h"

#define INVALID_TIMESTAMP       0xFFFFFFFF
#define INVALID_ITEM_ID         0xFFFF
#define RESERVED_ITEM_ID        0xFFFE

/**---------------------------------------------------------------------------*
 ** Struct
 **---------------------------------------------------------------------------*/
struct audio_nv_header{
    u16 id;
    u16 size;
    u16 checksum;
    u8  category;
    u8  status;
};

struct audio_nv_object {
    u32 timestamp;
    u8  *addr;
    u32 size;
    u16 max_id;
};

#define AUDIO_NV_BIN_MAX    0x8000  /* data + sizeof(struct audio_nv_header) */
#define AUDIO_NV_DATA_MAX   (AUDIO_NV_BIN_MAX - sizeof(struct audio_nv_header))

struct audio_nv_object audio_nv_object;

#ifdef CONFIG_AUDIO_NV_TEST
static const u8 s_nvram_nv[AUDIO_NV_BIN_MAX] =
{
    0xDC, 0xFA, 0x7B, 0x5D, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x01, 0x01, 0x41, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x42, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x43, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x44, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x45, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0xFF,
    0x01, 0x00, 0x44, 0x00, 0x00, 0x00, 0x01, 0x01, 0x4F, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x50, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x51, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x52,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x53, 0x01, 0x00, 0x01,
    0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF
};
#else
#ifndef CONFIG_AUDIO_NV_PARTION_ADDR
#define CONFIG_AUDIO_NV_PARTION_ADDR   0
#endif
#endif

/**
 * audio_nv_init - init audio nv partition.
 *
 * Return:
 *    0 is success, other fail.
 */
static int audio_nv_drv_init(void)
{
#ifdef CONFIG_AUDIO_NV_TEST
    audio_nv_object.addr = (u8 *)s_nvram_nv;
#else
    audio_nv_object.addr = (u8 *)CONFIG_AUDIO_NV_PARTION_ADDR;
#endif
    audio_nv_object.size = AUDIO_NV_BIN_MAX;
    audio_nv_object.timestamp = *(u32 *)audio_nv_object.addr;

    if (INVALID_TIMESTAMP == audio_nv_object.timestamp ||
        0 == audio_nv_object.timestamp) {
        aicx_error(0x10aa30, "x", "invalid timestamp 0x%x\n", audio_nv_object.timestamp);
        return AIC_EPERM;
    }

    return AIC_SUCCESS;
}
AIC_NORMAL_COMP_P1_INIT_EXPORT(audio_nv_drv_init);

#ifdef CONFIG_AUDIO_NV_CHECK
/**
 * audio_nv_chk_item - check audio nv item.
 *
 * @header: item header.
 * @offset: item offset.
 * @object: item obejct
 *
 * Return:
 *    @status: 0 is success, other fail.
 */
static int audio_nv_chk_item(struct audio_nv_header *header, u32 offset,
                             struct audio_nv_object *object)
{
    if ((INVALID_ITEM_ID == header->id) ||
        (RESERVED_ITEM_ID == header->id) ||
        (AUDIO_NV_DATA_MAX < header->size) ||
        (offset + header->size > object->size)) {
        aicx_error(0x0c57bf, "xxx", "id(0x%x),size(0x%x),offset(0x%x) invalid.\n",
                   header->id, header->size, offset);
        return EINVAL;
    }

    return AIC_SUCCESS;
}
#endif

/**
 * audio_nv_get_offset - get audio nv item offset.
 *
 * @id: nv item id.
 *
 * Return:
 *    @offset: audio nv item id offset.
 */
u32 audio_nv_get_offset(u16 item_id)
{
    struct audio_nv_object *object = &audio_nv_object;
    struct audio_nv_header *header;
    u32 offset = 0;

    /* Skip timestamp */
    offset += sizeof(object->timestamp);

    while (1) {
        if (offset + sizeof(struct audio_nv_header) > object->size) {
            aicx_error(0x039470, "d", "Audio nv get offset=%d, exceeded!\n", offset);
            return 0;
        }
        header = (struct audio_nv_header *)(object->addr + offset);

#ifdef CONFIG_NVRAM_CHECK
        if (!nvram_chk_item(header, offset +
                            sizeof(struct audio_nv_header), object)) {
            aicx_error(0x1589f3, "d", "Audio nv check item failed, status=%d\n", status);
            break;
        }
#endif

        /* if match, break and return current offset */
        if (header->id == item_id) {
            offset += sizeof(struct audio_nv_header);
            aicx_info(0x0c6497, "x", "Audio nv offset got, offset=0x%x\n", offset);
            break;
        } else {
            /* not match, go on next nv id */
            offset += sizeof(struct audio_nv_header) + header->size;
            /* Every item starts on the boundary of 4 alignment */
            offset = (offset + 3) & 0xFFFFFFFC;
        }
    }

    return offset;
}

/**
 * audio_nv_get_base - get audio nv base address.
 *
 * Return:
 *    audio nv base address.
 */
char *audio_nv_get_base(void)
{
    return (char *)audio_nv_object.addr;
}

/**
 * audio_nv_readi - read nv component.
 *
 * Return:
 *    read result
 */
int audio_nv_readi(u32 nvid, u32 index, void *buf, u32 nvid_size)
{
    struct audio_nv_header *header;
    u32 offset;
    char *nv_base;

    offset = audio_nv_get_offset(nvid);
    nv_base = audio_nv_get_base();

    header = (struct audio_nv_header *)(nv_base + offset);
    if (nvid_size * index >= header->size) {
        aicx_error(0x135538, "dd", "nv_read error, nvid=%d, index=%d\n", nvid, index);
        return AIC_EINVAL;
    }
    /* offset is nvid's, should add each item's offset */
    memcpy(buf, nv_base + offset + nvid_size * index, nvid_size);

    return 0;
}

/**
 * audio_nv_writei - write nv component.
 *
 * Return:
 *    write result
 */
int audio_nv_writei(u32 nvid, u32 index, void *buf, u32 nvid_size)
{
    struct audio_nv_header *header;
    u32 offset;
    char *nv_base;

    offset = audio_nv_get_offset(nvid);
    nv_base = audio_nv_get_base();

    header = (struct audio_nv_header *)nv_base + offset;
    if (nvid_size * index >= header->size) {
        aicx_error(0x135538, "dd", "nv_read error, nvid=%d, index=%d\n", nvid, index);
        return AIC_EINVAL;
    }
    /* TODO: change flash write API */
    /* offset is nvid's, should add each item's offset */
    memcpy(nv_base + offset + nvid_size * index, buf, nvid_size);

    return 0;
}
