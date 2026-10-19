/*****************************************************************************/
/*                                                                           */
/*    Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fmd.h>
#include <aic_list.h>
#include <aic_errno.h>
#include <aic_size.h>
#include <cache.h>
#include <hlzma.h>
#include <bspatch.h>
#include <lzma.h>
#include <boot_mode.h>
#include <initcall.h>
#include <psram_mem_config.h>

#include "serial/serial.h"
#include "platform/platform.h"
#include "fota_comm.h"
#include "console/console.h"

/* Note:
 *
 * The process supports power-off protection when both fota and fota_bak
 * partitions are exist, or rundata partition will be used to update without
 * power-off protection.
 *
 * */

#define FOTA_DEBUG
//#define FOTA_EXTEND_CHECK_DEBUG
//#define CONFIG_FOTA_BY_UART
//#define CONFIG_FOTA_BY_MCU

#define fota_urc(process) do {\
        u8 buf[100] = {0}; \
        u8 bottom[10] = {0}; \
        strncpy((char *)buf, atfota_ind, (sizeof(buf) - 1)); \
        __itoa(process, (char *)bottom, 10); \
        strncat((char *)buf, (char *)bottom, (sizeof(buf) - strlen((char *)buf) - 1)); \
        strncat((char *)buf, "\r\n", (sizeof(buf) - strlen((char *)buf) - 1)); \
        if (urc_port) \
            aic_uart_write(urc_port, buf, strlen((char *)buf)); \
    } while(0)

#ifdef FOTA_DEBUG
#define fota_info(fmt, ...) do {\
        log_print("fota: " fmt, ##__VA_ARGS__);\
    } while (0)
#else
#define fota_info(...)
#endif

#define PARSE_DPKG_ERROR -1
#define CHECKSUM_VERIFY_ERROR -2
#define OTHER_ERROR -3

/* info struct of patch for each partition */
typedef struct dpkg_item_s {
    char *name;             /* partition name */
    u32  dpoft;             /* patch offset */
    u32  dpsz;              /* diff patch size in byte */
    u32  oldimgsz;          /* old split block size, for non-split, this is total image size */
    u32  newimgsz;          /* new split block size, for non-split, uncom length for com part */
    u32  newcompsz;         /* size of compressed new image for lzma part */
    u16  oldchksum;         /* old image checksum */
    u16  newchksum;         /* new image checksum */
    u32  splitcnt;          /* split count, 0: not split, others split cnt */
    u32  splitidx;          /* split index, start from 0 */
    struct list_head list;  /* list for items */
} dpkg_item_t;

/* info struct of diff package */
typedef struct dpkg_info_s {
    u32 hdrsz;               /* dpkg header size in byte */
    u8  fmtver;              /* dpkg version number */
    u8  partn;               /* dpkt item number, include split parts */
    u16 checksum;            /* dpkg checksum */
    u32 dpkgsz;              /* dpkg total size in byte include header */
    u32 dpdatasz;            /* diff data size in byte */
    u32 dpkgaddr;            /* dpkg physicial address */
    u32 updateurc;           /* update urc */
    struct list_head items;  /* dpkt item pointer */
    u32 splitsz;             /* splitSz */
} dpkg_info_t;

#define FLAGS_BLK_OFFSET     SIZE_8K
#define FLAGS_BKBLK_OFFSET   SIZE_4K
#define FLAGS_SIZE_IN_BYTE   sizeof(struct fota_param)
#define FOTA_PTNAME          FOTA_EXTIMG_PARTION_NAME
#define FOTA_BK_OFFSET       FOTA_DIFF_PAC_MAX_SIZE
#define LZMA_DESTLEN_OFFSET  5
#define LZMA_DESTLEN_LEN     4
#define LZMA_HEADER_LEN      13
#define BS_NEWFLEN_OFFSET    24
#define BS_NEWFLEN_LEN       8
#define BKEND_LEN            8
#define COMPRESSED_FLAG_LEN  4

#define FAILURE_RETURN do { \
    if (ret < 0) { \
        log_print("fota: Line=%d, FAILURE_RETURN ret=%d!\n", __LINE__, ret); \
        goto err_ret; \
    } \
} while (0);

#define FOTA_EXT_IMG_MAGIC      0xAEAEBFBF
#define FOTA_EXT_IMG_HEADER_LEN 8

const char *atfota_ind = "\r\n+IND:\"FOTA\",\"BASIC\",\"UPDATING\",";
struct fota_param g_fota_para = {0};
dpkg_info_t g_dpkg_info = {0};
/* fota param partition handle */
u32 fotaparamparthdl = 0;
/* fota param partition size */
u32 fotaparampartsz = 0;
/* finished diff bytes */
u32 findpksz = 0;
u32 szperline = 0;
void *urc_port = 0;

#ifndef CONFIG_FOTA_BY_MCU
static void init_urt_port(void)
{
    struct fota_param *param = &g_fota_para;

    fota_info("init_urt_port port(0x%x), baud=%d\n",
              param->input.serial_port_id, param->input.serial_port_baud);
    log_print_r("init_urt_port port(0x%x), baud=%d\n",
                param->input.serial_port_id, param->input.serial_port_baud);
    urc_port = aic_uart_port_open(param->input.serial_port_id);
    if (urc_port)
        aic_uart_baud_set(urc_port, param->input.serial_port_baud);
}
#endif

/* len is bytes of buffer */
u16 fota_checksum(u16 *p, u32 len)
{
    u16 cksum = 0;
    u32 cnt;

    if (!p || len == 0)
        return 0;

    cnt = len / 2;
    while (cnt-- > 0)
        cksum ^= *p++;

    /* if len is odd, the last byte left shift 8bits, and 8 LSB fill 0 */
    if (len % 2)
        cksum ^= ((u16)(*(char *)p) << 8);

    return cksum;
}

/*
 * parse_dpkg - Parse difference package information
 *
 * blkid: 0,success;negative, fail
 *
 */
s32 parse_dpkg(void)
{
    s32 ret = AIC_SUCCESS;
    u32 dp_pos, part_size, totalsz;
    dpkg_item_t *dpitem;
    bool isSplit = false;
    u32 fotaparthdl = 0;

    ret = fmd_open(FOTA_PTNAME, &fotaparthdl);
    if (ret < 0) {
        fota_info("parse_dpkg open fota backup partition failed, errno(%d)!\n",
                  ret);
        goto err_ret;
    }
    ret = fmd_get_physical_address(fotaparthdl, 0, &dp_pos);
    if (ret < 0) {
        fota_info("parse_dpkg get fota partition address failed, errno(%d)!\n",
                  ret);
        goto err_ret;
    }

    totalsz = *(u32 *)(dp_pos + 12);
    part_size = 0;
    fmd_get_partition_size(fotaparthdl, &part_size);

    if (totalsz >= part_size)
    {
        fota_info("parse_dpkg err totalsz=0x%x, part_size(0x%x)!\n",
                  totalsz, part_size);
        ret = AIC_EINVAL;
        goto err_ret;
    }

    fota_info("parse_dpkg start magic(0x%x), end magic(0x%x)!\n",
              *(u32 *)dp_pos,
              *(u32 *)(dp_pos + totalsz - 4));
    if ((*(u8 *)(dp_pos  + 0) != 'D') ||
         (*(u8 *)(dp_pos + 1) != 'I') ||
         (*(u8 *)(dp_pos + 2) != 'F') ||
         (*(u8 *)(dp_pos + 3) != 'S') ||
         (*(u8 *)(dp_pos + totalsz - 4) != 'D') ||
         (*(u8 *)(dp_pos + totalsz - 3) != 'I') ||
         (*(u8 *)(dp_pos + totalsz - 2) != 'F') ||
         (*(u8 *)(dp_pos + totalsz - 1) != 'E')) {
        ret = AIC_EINVAL;
        goto err_ret;
    }
    g_dpkg_info.dpkgaddr = dp_pos;
    /* skip diff pkg start magic */
    dp_pos += 4;
    list_head_init(&g_dpkg_info.items);
    g_dpkg_info.hdrsz = *(u32 *)dp_pos;
    dp_pos += sizeof(g_dpkg_info.hdrsz);
    g_dpkg_info.fmtver = *(u8 *)dp_pos;
    dp_pos += sizeof(g_dpkg_info.fmtver);
    g_dpkg_info.partn = *(u8 *)dp_pos;
    dp_pos += sizeof(g_dpkg_info.partn);
    g_dpkg_info.checksum = *(u16 *)dp_pos;
    dp_pos += sizeof(g_dpkg_info.checksum);
    g_dpkg_info.dpkgsz = *(u32 *)dp_pos;
    dp_pos += sizeof(g_dpkg_info.dpkgsz);
    g_dpkg_info.splitsz = 0;
    /* skip reserve size */
    dp_pos += 32;
    /* parse diff package items */
    for (u32 i = 0; i < g_dpkg_info.partn; i++) {
        u8 buf[BS_NEWFLEN_LEN];

        dpitem = malloc(sizeof(dpkg_item_t));
        list_head_init(&dpitem->list);
        list_add_tail(&dpitem->list, &g_dpkg_info.items);
        if (isSplit) {
            dpkg_item_t *prev;
            /* some information come from prev part */
            prev              = list_prev_entry(dpitem, list, dpkg_item_t);
            dpitem->name      = prev->name;
            dpitem->splitcnt  = prev->splitcnt;
            dpitem->newcompsz = prev->newcompsz;
            dpitem->splitidx  = prev->splitidx + 1;
            if (dpitem->splitidx == (dpitem->splitcnt - 1))
                isSplit = false;
            /* split seg size */
            dpitem->dpsz      = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* old split seg size */
            dpitem->oldimgsz  = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* new split seg size */
            dpitem->newimgsz  = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* split seg offset */
            dpitem->dpoft     = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* old seg checksum */
            dpitem->oldchksum = *(u16 *)dp_pos;
            dp_pos           += 2;
            /* new seg checksum */
            dpitem->newchksum = *(u16 *)dp_pos;
            dp_pos           += 2;
        } else { /* not split */
            dpitem->name      = (char *)dp_pos;
            dp_pos           += 32;
            /* size of difference package of this partition */
            dpitem->dpsz      = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* size of old image */
            dpitem->oldimgsz  = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* size of new image,valid for lzma part */
            dpitem->newcompsz = *(u32 *)dp_pos;
            dp_pos           += 4;
            /* total image checksum, will be changed later if splited */
            dpitem->oldchksum = *(u16 *)dp_pos;
            dp_pos           += 2;
            /* total image checksum, will be changed later if splited */
            dpitem->newchksum = *(u16 *)dp_pos;
            dp_pos           += 2;
            /* difference package offset of cur partition in diff package */
            dpitem->dpoft     = *(u32 *)dp_pos;
            dp_pos           += 4;
            dpitem->splitcnt  = *(u32 *)dp_pos;
            dp_pos           += 4;
            if (dpitem->splitcnt == 0) { /* non split part ? */
                ret = fmd_read(fotaparthdl,
                               dpitem->dpoft + BS_NEWFLEN_OFFSET,
                               BS_NEWFLEN_LEN,
                               buf);
                FAILURE_RETURN
                dpitem->newimgsz  =  (u32)buf[0] |
                                     ((u32)buf[1] << 8) |
                                     ((u32)buf[2] << (8*2)) |
                                     ((u32)buf[3] << (8*3));
            } else {
                dpitem->splitidx   = 0;
                dpitem->dpsz       = *(u32 *)dp_pos;
                dp_pos            += 4;
                dpitem->oldimgsz   = *(u32 *)dp_pos;
                dp_pos            += 4;
                dpitem->newimgsz   = *(u32 *)dp_pos;
                dp_pos            += 4;
                dpitem->dpoft      = *(u32 *)dp_pos;
                dp_pos            += 4;
                /* old seg checksum */
                dpitem->oldchksum = *(u16 *)dp_pos;
                dp_pos           += 2;
                /* new seg checksum */
                dpitem->newchksum = *(u16 *)dp_pos;
                dp_pos           += 2;
                isSplit            = true;
                /* splitsz is same among partitions */
                if (g_dpkg_info.splitsz == 0)
                    g_dpkg_info.splitsz = dpitem->oldimgsz;
            }
        }
        g_dpkg_info.dpdatasz += dpitem->dpsz;
        fota_info("parse_dpkg part num(0x%x),name(%s),oldchecksum(0x%x),"
                  "newchecksum(0x%x)!\n",
                  i,
                  dpitem->name,
                  dpitem->oldchksum,
                  dpitem->newchksum);
        fota_info("splitcnt(0x%x),splitidx(%x),oldsz(0x%x),newsz(0x%x),"
                  "compsz(0x%x),oft(0x%x)!\n",
                  dpitem->splitcnt,
                  dpitem->splitidx,
                  dpitem->oldimgsz,
                  dpitem->newimgsz,
                  dpitem->newcompsz,
                  dpitem->dpoft);
    }

err_ret:
    if (fotaparthdl)
        fmd_close(fotaparthdl);

    return ret;
}

#ifdef CONFIG_FOTA_BY_MCU
static void fota_get_the_http_param(struct fota_old_http_input *old_http,
                                    struct fota_http_input *new_http)
{
    memcpy(new_http->url, old_http->url, sizeof(new_http->url));
    memcpy(new_http->user_name, old_http->user_name, sizeof(new_http->user_name));
    memcpy(new_http->password, old_http->password, sizeof(new_http->password));
    new_http->port = old_http->port;
}

static void fota_get_the_input_param(struct fota_old_input *old_input,
                                     struct fota_input *new_input)
{
    /* get input param. */
    fota_get_the_http_param(&old_input->http, &new_input->http);

    /* get crt_url param. */
    memcpy(new_input->crt_url, old_input->crt_url, sizeof(new_input->crt_url));

    /* [crt, mode) is no change, just copy it. */
    memcpy(&new_input->crt, &old_input->crt,
           (u32)&new_input->mode -  (u32)&new_input->crt);

    /* [mode, pkg_size] not in old, don't copy, get the reserved. */
    new_input->reserved = old_input->reserved;
}

static s32 fota_old_to_new_and_save_it(u32 offset, struct fota_old_param *fota_old)
{
    struct fota_param *fota_new;
    s32 ret;

    log_print("fota: conver old to new(offset=0x%x)!\n", offset);

    fota_new = (struct fota_param *)malloc(sizeof(*fota_new));
    if (!fota_new) {
        log_print("fota: malloc fail.\n");
        return -1;
    }

    memset(fota_new, 0, sizeof(*fota_new));

    /* before input member is no change, just cpy it. */
    memcpy((void *)fota_new, (void *)fota_old,
            (u32)(&fota_new->input) - (u32)(fota_new));

    /* the get the input param. */
    fota_get_the_input_param(&fota_old->input, &fota_new->input);

    /* the last 2 member. */
    fota_new->finished_parti_cnt = fota_old->finished_parti_cnt;
    fota_new->magic = fota_old->magic;

    /* save to flash */
    ret = fmd_erase(fotaparamparthdl,
                    offset,
                    SIZE_4K);
    FAILURE_RETURN
    ret = fmd_write(fotaparamparthdl,
                    offset,
                    FLAGS_SIZE_IN_BYTE,
                    (u8 *)fota_new);

err_ret:
    free(fota_new);
}

/*
 * get_fota_old_ver_check - check if the fota param is the
 * old fota verion, if it is the old para, conver it to
 * new version param.
 */
s32 get_fota_old_ver_check(void)
{
    s32 ret;
    struct fota_old_param *fota_old;

    log_print("fota: old ver check.\n");

    fota_old = (struct fota_old_param *)malloc(sizeof(*fota_old));
    if (!fota_old) {
        log_print("fota: malloc fail.\n");
        return -1;
    }

    /* check the fota param. */
    memset(fota_old, 0, sizeof(*fota_old));
    ret = fmd_read(fotaparamparthdl,
                   0,
                   sizeof(*fota_old),
                   (u8 *)fota_old);
    FAILURE_RETURN

    if (fota_old->magic == FOTA_MAGIC_WORD) {
        fota_old_to_new_and_save_it(0, fota_old);
    }

    /* check the fota bak. */
    memset(fota_old, 0, sizeof(*fota_old));
    ret = fmd_read(fotaparamparthdl,
                   FLAGS_BKBLK_OFFSET,
                   sizeof(*fota_old),
                   (u8 *)fota_old);
    FAILURE_RETURN

    if (fota_old->magic == FOTA_MAGIC_WORD) {
        fota_old_to_new_and_save_it(FLAGS_BKBLK_OFFSET, fota_old);
    }

err_ret:
    free(fota_old);
}
#endif

/*
 * get_fota_param - Get fota param from fota partition
 * and store them to g_fota_para or g_fota_parabk
 *
 *
 */
s32 get_fota_param(void)
{
    s32 ret;
    u8 *param = AIC_NULL;
    u32 offset;
    struct fota_param fota_parabk = {0};

    ret = fmd_read(fotaparamparthdl,
                   0,
                   FLAGS_SIZE_IN_BYTE,
                   (u8 *)&g_fota_para);
    FAILURE_RETURN
    ret = fmd_read(fotaparamparthdl,
                   FLAGS_BKBLK_OFFSET,
                   FLAGS_SIZE_IN_BYTE,
                   (u8 *)&fota_parabk);
    FAILURE_RETURN

    if (g_fota_para.magic != FOTA_MAGIC_WORD) {
        if (fota_parabk.magic == FOTA_MAGIC_WORD) {
            offset = 0;
            param  = (u8 *)&fota_parabk;
            memcpy(&g_fota_para, &fota_parabk, sizeof(struct fota_param));
        } else {
            ret = AIC_EPERM;
            goto err_ret;
        }
    }
    if (fota_parabk.magic != FOTA_MAGIC_WORD) {
        if (g_fota_para.magic == FOTA_MAGIC_WORD) {
            offset = FLAGS_BKBLK_OFFSET;
            param  = (u8 *)&g_fota_para;
            memcpy(&fota_parabk, &g_fota_para, sizeof(struct fota_param));
        } else {
            ret = AIC_EPERM;
            goto err_ret;
        }
    }
    if (param != AIC_NULL) {
        ret = fmd_erase(fotaparamparthdl,
                        offset,
                        SIZE_4K);
        FAILURE_RETURN
        ret = fmd_write(fotaparamparthdl,
                        offset,
                        FLAGS_SIZE_IN_BYTE,
                        param);
        FAILURE_RETURN
    }

err_ret:
    if (ret < 0)
        log_print("fota: get_fota_param(0x%x)!\n", ret);
    return ret;
}

/*
 * set_fota_param() - Set fota param from fota partition
 * and store them to g_fota_para or g_fota_parabk
 *
 *
 */
s32 set_fota_param(void)
{
    s32 ret = AIC_SUCCESS;

    ret = fmd_erase(fotaparamparthdl, 0, SIZE_4K);
    FAILURE_RETURN
    ret = fmd_write(fotaparamparthdl, 0, FLAGS_SIZE_IN_BYTE,
                    (u8 *)&g_fota_para);
    FAILURE_RETURN
    ret = fmd_erase(fotaparamparthdl, FLAGS_BKBLK_OFFSET, SIZE_4K);
    FAILURE_RETURN
    ret = fmd_write(fotaparamparthdl, FLAGS_BKBLK_OFFSET, FLAGS_SIZE_IN_BYTE,
                    (u8 *)&g_fota_para);
    FAILURE_RETURN

err_ret:
    if (ret < 0)
        log_print("fota: set_fota_param(0x%x)!\n", ret);
    return ret;
}

/*
 * close_fota_param() - Close fota param from fota partition
 *
 */
void close_fota_param(void)
{
    if (fotaparamparthdl != 0) {
        fmd_close(fotaparamparthdl);
        fotaparamparthdl = 0;
    }
}

/* fota is normal mode? */
bool fota_is_normal_mode(void)
{
    if (g_fota_para.magic == 0xFFFFFFFF ||
        (g_fota_para.magic == FOTA_MAGIC_WORD &&
         g_fota_para.stage == FOTA_STAGE_BASIC_DIFF_DL))
        return true;
    else
        return false;
}

/* set ota status flag */
s32 set_otasts(u32 *sts)
{
    s32 ret = AIC_SUCCESS;

    g_fota_para.state = *sts;
    ret = set_fota_param();
    FAILURE_RETURN

err_ret:
    return ret;
}

/* set finished partition count */
s32 set_fincnt(u32 *cnt)
{
    s32 ret = AIC_SUCCESS;

    g_fota_para.finished_parti_cnt = *cnt;
    ret = set_fota_param();
    FAILURE_RETURN

err_ret:
    return ret;
}

/* erase backup partition */
s32 erase_partition(char *partname, u32 offset)
{
    s32 ret = AIC_SUCCESS;
    u32 bkphandle = 0, bkpsize;

    if (!partname)
        goto err_ret;
    ret = fmd_open(partname, &bkphandle);
    FAILURE_RETURN
    ret = fmd_get_partition_size(bkphandle, &bkpsize);
    FAILURE_RETURN
    ret = fmd_erase(bkphandle, offset, bkpsize - offset);
    FAILURE_RETURN
err_ret:
    if (bkphandle != 0)
        fmd_close(bkphandle);
    return ret;
}

/* to avoid too large stack */
u8 bkBuf[SIZE_4K];
/* backup a partition */
s32 bk_partition(char *name, u32 offset, u32 size)
{
    s32 ret = AIC_SUCCESS;
    u32 srcHandle = 0;
    u32 dstHandle = 0;
    u32 dstSz, inc, inc_bk;
    u8 end[BKEND_LEN];

    ret = fmd_open(name, &srcHandle);
    FAILURE_RETURN

    inc = 0;
    inc_bk = FOTA_BK_OFFSET;
    ret = fmd_open(FOTA_PTNAME, &dstHandle);
    FAILURE_RETURN
    ret = fmd_get_partition_size(dstHandle, &dstSz);
    FAILURE_RETURN

    if (size > dstSz - SIZE_4K) {
        log_print("fota: size(0x%x) must < 0x%x.\n", size, dstSz - SIZE_4K);
        goto err_ret;
    }

    ret = fmd_erase(dstHandle, dstSz - SIZE_4K, SIZE_4K);
    FAILURE_RETURN
    while(size >= SIZE_4K) {
        ret = fmd_read(srcHandle, offset + inc, SIZE_4K, bkBuf);
        FAILURE_RETURN
        ret = fmd_erase(dstHandle, inc_bk, SIZE_4K);
        FAILURE_RETURN
        ret = fmd_write(dstHandle, inc_bk, SIZE_4K, bkBuf);
        FAILURE_RETURN
        inc += SIZE_4K;
        inc_bk += SIZE_4K;
        size -= SIZE_4K;
    }
    if (size > 0) {
        ret = fmd_read(srcHandle, offset + inc, size, bkBuf);
        FAILURE_RETURN
        ret = fmd_erase(dstHandle, inc_bk, SIZE_4K);
        FAILURE_RETURN
        ret = fmd_write(dstHandle, inc_bk, size, bkBuf);
        FAILURE_RETURN
        inc += size;
        inc_bk += size;
    }
    /* write backup region end flag */
    end[0] = (u8)(g_fota_para.finished_parti_cnt & 0xFF);
    end[1] = (u8)((g_fota_para.finished_parti_cnt >> 8) & 0xFF);
    end[2] = (u8)((g_fota_para.finished_parti_cnt >> 16) & 0xFF);
    end[3] = (u8)((g_fota_para.finished_parti_cnt >> 24) & 0xFF);
    end[4] = 'e';
    end[5] = 'n';
    end[6] = 'd';
    end[7] = 'b';
    ret = fmd_write(dstHandle, dstSz - SIZE_4K, BKEND_LEN, end);
    FAILURE_RETURN
err_ret:
    if (srcHandle != 0)
        fmd_close(srcHandle);
    if (dstHandle != 0)
        fmd_close(dstHandle);
    if (ret < 0)
        log_print("fota: bk_partition failed, errno(%d)!\n", ret);
    return ret;
}

/* get whether the partition is compressed */
bool part_is_compressed(char *name, u32 offset)
{
    s32 ret = AIC_SUCCESS;
    u32 handle = 0;
    u8 buf[COMPRESSED_FLAG_LEN];

    ret = fmd_open(name, &handle);
    if (ret < 0) {
        fota_info("part_is_compressed open part failed, errno(%d)!\n", ret);
        return false;
    }
    ret = fmd_read(handle, offset, 4, buf);
    if (ret < 0) {
        fota_info("part_is_compressed read part failed, errno(%d)!\n", ret);
        return false;
    }
    if (handle != 0)
        fmd_close(handle);
    /* compressed flag lenc */
    if (buf[0] == 'l' && buf[1] == 'e' && buf[2] == 'n' && buf[3] == 'c')
        return true;
    else
        return false;
}

/* backup region is valid? */
bool partbk_is_valid(void)
{
    s32 ret = AIC_SUCCESS;
    u32 handle = 0;
    u8 end[BKEND_LEN];
    u32 cnt, bkSz;

    ret = fmd_open(FOTA_PTNAME, &handle);
    if (ret < 0) {
        fota_info("partbk_is_valid open part failed, errno(%d)!\n", ret);
        return false;
    }
    ret = fmd_get_partition_size(handle, &bkSz);
    if (ret < 0) {
        fota_info("partbk_is_valid get bk part sz failed, errno(%d)!\n", ret);
        return false;
    }
    ret = fmd_read(handle, bkSz - SIZE_4K, BKEND_LEN, end);
    if (ret < 0) {
        fota_info("partbk_is_valid read bk part end failed, errno(%d)!\n", ret);
        return false;
    }

    if (handle)
        fmd_close(handle);

    cnt =  (u32)end[0] |
          ((u32)end[1] << 8) |
          ((u32)end[2] << (8*2)) |
          ((u32)end[3] << (8*3));
    if (end[4] == 'e' && end[5] == 'n' && end[6] == 'd' && end[7] == 'b' &&
        cnt == g_fota_para.finished_parti_cnt)
        return true;
    else
        return false;
}

/* get uncompressed partition size */
s32 cpart_orig_len(char *name, u32 *len, u32 offset)
{
    s32 ret = AIC_SUCCESS;
    u32 handle = 0;
    u8 buf[LZMA_DESTLEN_LEN];

    ret = fmd_open(name, &handle);
    FAILURE_RETURN
    ret = fmd_read(handle,
                   LZMA_DESTLEN_OFFSET + COMPRESSED_FLAG_LEN + offset,
                   LZMA_DESTLEN_LEN,
                   buf);
    FAILURE_RETURN
    *len =     (u32)buf[0] |
              ((u32)buf[1] << 8) |
              ((u32)buf[2] << (8*2)) |
              ((u32)buf[3] << (8*3));
err_ret:
    if (handle != 0)
        fmd_close(handle);
    return ret;
}

/* get partition size */
s32 part_len(char *name, u32 *len)
{
    s32 ret = AIC_SUCCESS;
    u32 handle = 0;

    ret = fmd_open(name, &handle);
    FAILURE_RETURN
    ret = fmd_get_partition_size(handle, len);
    FAILURE_RETURN

err_ret:
    if (handle != 0)
        fmd_close(handle);
    return ret;
}

/* get partition phycial address */
s32 part_addr(char *name, u32 *addr, u32 offset)
{
    s32 ret = AIC_SUCCESS;
    u32 handle = 0;

    ret = fmd_open(name, &handle);
    FAILURE_RETURN
    ret = fmd_get_physical_address(handle, offset, addr);
    FAILURE_RETURN
err_ret:
    if (handle != 0)
        fmd_close(handle);
    return ret;
}

/* write patched data back to flash */
s32 part_wb(char *name, u32 offset, u8 *start, u32 size)
{
    s32 ret = AIC_SUCCESS;
    u32 handle;

    ret = fmd_open(name, &handle);
    FAILURE_RETURN
    ret = fmd_erase(handle, offset, ROUND_UP(size, SIZE_4K));
    FAILURE_RETURN
    ret = fmd_write(handle, offset, size, start);
    FAILURE_RETURN

err_ret:
    if (handle != 0)
        fmd_close(handle);
    return ret;
}

#ifdef FOTA_EXTEND_CHECK_DEBUG
void extend_check(void)
{
    u32 ext_bak_addr = FOTA_AOS_BACKUP_BASE;
    u32 ext_img_size = *(u32 *)((u32 *)ext_bak_addr + 1);
    u16 checksum_calc = 0, checksum = 0;

    fota_info("fota: start check exend ap\n");
    /* read the extend system image from sram */
    if (*((u32 *)ext_bak_addr) != FOTA_EXT_IMG_MAGIC) {
        log_print("fota: invalid ext image magic!\n");
        log_print_r("inv ext mgc!\n");
        while(1);
    }

    checksum_calc = fota_checksum((u16 *)(ext_bak_addr + FOTA_EXT_IMG_HEADER_LEN),
                                  ext_img_size);
    checksum = *((u16 *)((u8 *)ext_bak_addr + FOTA_EXT_IMG_HEADER_LEN + ext_img_size));
    if (checksum != checksum_calc) {
        log_print("fota: ext image checksum error, %d, %d, size:%d!\n",
                  checksum, checksum_calc, ext_img_size);
        log_print_r("err ext chksum,%d,%d,%d!\n",
                    checksum, checksum_calc, ext_img_size);
        while(1);
    }
}
#endif

static void fota_try_adjust_the_state(bool dpkg_err)
{
    if (g_fota_para.finished_parti_cnt) {
        /*
         * Once the finished_parti_cnt > 0, mens the basic diff is ok,
         * Can't change the stage again, just wait the update finish.
         */
        if (dpkg_err) {
            /*
             * If the dpkg_err, May the extend img has not full restore,
             * change the stage to FOTA_STAGE_EXTEND_IMG_DL to download
             * the full extend image.
             */
            g_fota_para.stage = FOTA_STAGE_EXTEND_IMG_DL;
            set_fota_param();
        }
        return;
    }

    /*
     * The finished_parti_cnt is 0, may the dpkg has error,
     * change the state to FOTA_STAGE_BASIC_DIFF_DL to
     * redownload the basic diff pkg.
     */
    g_fota_para.stage = FOTA_STAGE_BASIC_DIFF_DL;
    set_fota_param();
}

/**
 * patch_apply() - Process FOTA
 *
 * Update firmware by the diff patch which loaded to local before.
 *
 * Return: 0 means success, others failure
 *
 */
s32 patch_apply(void)
{
    s32 ret = AIC_SUCCESS;
    u32 i = 0;
    struct list_head *pos;
    dpkg_item_t *itm;
    /* offset of splitted partition */
    u32 split_offset = 0;
#ifndef CONFIG_FOTA_CMIOT_CMD
    /* update progress */
    u32 urc = 0, urc_back = 0;
#ifndef CONFIG_FOTA_BY_UART
    u32 base = 50;
#else
    u32 base = 100;
#endif
#endif
    g_dpkg_info.updateurc = g_fota_para.input.upgrd_urc;
    szperline = g_dpkg_info.dpdatasz / g_dpkg_info.updateurc;

    fota_info("fota_entry diffpkg infos:\n");
    fota_info("hdrsz(0x%x),fmtver(0x%x),partn(0x%x),dpkgsz(0x%x),"
              "dpdatasz(0x%x),upurc(0x%x),splitsz(0x%x)\n",
              g_dpkg_info.hdrsz,
              g_dpkg_info.fmtver,
              g_dpkg_info.partn,
              g_dpkg_info.dpkgsz,
              g_dpkg_info.dpdatasz,
              g_dpkg_info.updateurc,
              g_dpkg_info.splitsz);

    if (g_fota_para.finished_parti_cnt > g_dpkg_info.partn) {
        /* finished partition more than total diff partition */
        ret = -404;
        goto err_ret;
    }
#ifndef CONFIG_FOTA_BY_MCU
    init_urt_port();
#endif
    /* get first partition to update*/
    list_for_each(pos, &g_dpkg_info.items) {
        bool isBloader = false;
        if(i++ < g_fota_para.finished_parti_cnt) {
            itm = list_entry(pos, dpkg_item_t, list);
            /* get finished diff pkg size for updating progress */
            findpksz += itm->dpsz;
            continue;
        }
        itm = list_entry(pos, dpkg_item_t, list);
        if (!strcmp(itm->name, "bl"))
           isBloader = true;
        if (itm->dpsz != 0) {
            u32 origSz;
            u8 *origAddr = NULL;
            u32 diffSz, diffAddr = 0;
            u32 newSz;
            u8 *newAddr =  NULL;
            bool isCompressed;
            char *destPartname;
            u8 *dest = NULL;
            u32 offset = 0;

            destPartname = itm->name;
            ret = part_addr(destPartname, (u32 *)&origAddr, 0);
            FAILURE_RETURN
            if (itm->splitcnt != 0)
                origAddr += itm->splitidx * g_dpkg_info.splitsz;
            origSz   = itm->oldimgsz;
            diffAddr = g_dpkg_info.dpkgaddr + itm->dpoft;
            diffSz   = itm->dpsz;
            newSz    = itm->newimgsz;
            fota_info("fota_entry partition name %s,dpsz(0x%x)!\n",
                      itm->name,
                      itm->dpsz);
            if (newSz == 0) {
                newAddr = NULL;
                log_print("fota: splitidx=%d, newsz is 0, skip it!\n", itm->splitidx);
                goto finish_one_parti;
            }

            newAddr  = malloc(newSz);
            if (newAddr == NULL) {
                log_print("fota: alloc mem(0x%x) failed!\n", newSz);
                ret = -405;
                goto err_ret;
            }
            fota_info("fota_entry malloc, 0x%x ~ 0x%x(0x%x)\n",
                      newAddr,
                      newAddr + newSz -1,
                      newSz);

            if (!partbk_is_valid()) {
                u32 oft;
                fota_info("fota_entry %s backup part is not valid\n",
                            itm->name);
                /* backup original image */
                if (itm->splitcnt == 0)
                    oft = 0;
                else
                    oft = itm->splitidx * g_dpkg_info.splitsz;
                fota_info("fota_entry backup %s start, offset(0x%x), size(0x%x)\n",
                            destPartname,
                            oft,
                            itm->oldimgsz);
                ret = bk_partition(destPartname, oft, itm->oldimgsz);
                if (ret < 0) {
                    goto free_new;
                }
                fota_info("fota_entry backup %s end!\n", destPartname);
            } else {
                /* use backup partiton */
                destPartname = FOTA_PTNAME;
                offset = FOTA_BK_OFFSET;
                /* user backup part for patch */
                ret = part_addr(destPartname, (u32 *)&origAddr, offset);
                if (ret < 0) {
                    goto free_new;
                }
            }

            /* process compressed images */
            isCompressed = part_is_compressed(destPartname, offset);
            if (isCompressed) {
                u8 *srcData = NULL;
                u32 origLen, addr, aclen;

                fota_info("fota_entry %s part is compressed\n", itm->name);
                ret = cpart_orig_len(destPartname, &origLen, offset);
                if (ret < 0) {
                    goto free_new;
                }
                ret = part_addr(destPartname, &addr, offset);
                if (ret < 0) {
                    goto free_new;
                }
                aclen = (origLen + SIZE_4K) & ~(SIZE_4K - 1);
                srcData = malloc(aclen);
                if (srcData == NULL) {
                    log_print("fota: alloc mem(0x%x) failed!\n", aclen);
                    ret = -406;
                    goto free_new;
                }
                fota_info("fota_entry malloc, 0x%x ~ 0x%x(0x%x)\n",
                          srcData,
                          srcData + aclen -1,
                          aclen);
                /* make data clean */
                dcache_invcc_by_range((u32)srcData, aclen);
                ret = hlzma_dec_polling(addr + COMPRESSED_FLAG_LEN,
                                        (u32)srcData);
                if (ret < 0) {
                    free(srcData);
                    srcData = NULL;
                    goto free_new;
                }
                dcache_inv_by_range((u32)srcData, aclen);
                origAddr = srcData;
                origSz = origLen;
            } /* else not compressed region */
            fota_info("fota_entry start patch %s\n", itm->name);
            /* do patch */
            ret = patch_partition(origAddr,
                                  origSz,
                                  (u8 *)diffAddr,
                                  diffSz,
                                  newAddr,
                                  &newSz);
            if (ret < 0) {
                goto free_new;
            }
            fota_info("fota_entry finish patch %s\n", itm->name);
            if (isCompressed) {
                u32 destLen = 0;
                /* free some memory */
                if (origAddr != NULL) {
                    free(origAddr);
                    origAddr = NULL;
                }
                /* allocate memory */
                dest = malloc(itm->newcompsz);
                if (dest == NULL) {
                    log_print("fota: alloc mem(0x%x) failed!\n",
                              itm->newcompsz);
                    ret = -407;
                    goto err_ret;
                }
                fota_info("fota_entry malloc, 0x%x ~ 0x%x(0x%x)\n",
                          dest,
                          dest + itm->newcompsz -1,
                          itm->newcompsz);
                dest[0] = 'l';
                dest[1] = 'e';
                dest[2] = 'n';
                dest[3] = 'c';
                destLen = itm->newcompsz - COMPRESSED_FLAG_LEN - LZMA_HEADER_LEN;
                /* encode a partition with lzma */
                ret = enc_partition(dest + COMPRESSED_FLAG_LEN,
                                    &destLen,
                                    newAddr,
                                    newSz);
                if (ret < 0) {
                    free(dest);
                    dest = NULL;
                }
free_new:
                /* free uncompress area */
                if (newAddr != NULL) {
                    free(newAddr);
                    newAddr = NULL;
                }
                if (ret < 0)
                    goto err_ret;
                newAddr = dest;
                newSz   = itm->newcompsz;
            }
            /* write new image to flash */
            if (itm->splitcnt != 0)
                split_offset = itm->splitidx * g_dpkg_info.splitsz;
            else
                split_offset = 0;
            fota_info("fota_entry start write back %s, addr:%p, size:%d\n", itm->name, newAddr, newSz);
            part_wb(itm->name, split_offset, newAddr, newSz);
            fota_info("fota_entry finish write back %s\n", itm->name);
            if (newAddr != NULL) {
                free(newAddr);
                newAddr = NULL;
            }
        } /* else, dpsz = 0 */
finish_one_parti:
        g_fota_para.finished_parti_cnt++;
        ret = set_fota_param();
        FAILURE_RETURN

#ifndef CONFIG_FOTA_CMIOT_CMD
        /* update progress */
        for (u32 j = findpksz / szperline + 1; j <= (findpksz + itm->dpsz) / szperline; j++) {
            urc = j * szperline * base / g_dpkg_info.dpdatasz;
            if (urc_back != urc) {
                urc_back = urc;
                fota_urc(urc);
            }
        }
#endif
        findpksz += itm->dpsz;
        if (isBloader) {
#ifdef FOTA_EXTEND_CHECK_DEBUG
             extend_check();
#endif
             /* reboot */
             fota_info("fota_entry bootloader updated, reboot now!\n");
             log_print_r("blE,reboot!\n");
             bl_reset_basic();
             while(1);
        }
    }

    fota_info("fota_entry update finish, set finish state: %d\n", g_fota_para.state);

#ifndef CONFIG_FOTA_CMIOT_CMD
    if (urc != base)
#ifndef CONFIG_FOTA_BY_UART
        fota_urc(50);
#else
        fota_urc(100);
#endif
#endif

    ret = erase_partition(RUNDATA_PTNAME, 0);
    FAILURE_RETURN

err_ret:
    if (ret < 0)
        log_print("fota: fota_entry(0x%x)!\n", ret);

    fota_info("fota: patch applied (0x%x)\n", ret);

    return ret;
}

/**
 * extend_restore - restore extend system
 *
 * Restore extend system which loaded to local ram before.
 *
 * Return: 0 means success, others failure.
 *
 */
s32 extend_restore(void)
{
    s32 ret = AIC_SUCCESS;
    u32 ext_bak_addr = FOTA_AOS_BACKUP_BASE;
    u32 ext_img_size = *(u32 *)((u32 *)ext_bak_addr + 1);
    u16 checksum_calc = 0, checksum = 0;

#ifdef FOTA_EXTEND_CHECK_DEBUG
    extend_check();
#endif

    fota_info("fota: start restore exend ap\n");
    /* read the extend system image from sram */
    if (*((u32 *)ext_bak_addr) != FOTA_EXT_IMG_MAGIC) {
        log_print("fota: invalid ext image magic!\n");
        log_print_r("inv ext mgc\n");
        return -1;
    }

    checksum_calc = fota_checksum((u16 *)(ext_bak_addr + FOTA_EXT_IMG_HEADER_LEN),
                                  ext_img_size);
    checksum = *((u16 *)((u8 *)ext_bak_addr + FOTA_EXT_IMG_HEADER_LEN + ext_img_size));
    if (checksum != checksum_calc) {
        log_print("fota: ext image checksum error, %d, %d, size:%d!\n",
                  checksum, checksum_calc, ext_img_size);
        log_print_r("err ext chksum,%d,%d,%d!\n",
                    checksum, checksum_calc, ext_img_size);
        return -1;
    }

    ret = part_wb(FOTA_PTNAME, 0, (u8 *)(ext_bak_addr + FOTA_EXT_IMG_HEADER_LEN),
                  ext_img_size);
    if (ret < 0) {
        log_print("fota: ext image restore failed\n");
        log_print_r("err ext rstore\n");
        return -1;
    }

    dcache_invcc_all();

    fota_info("fota: extend ap restore success\n");

    return 0;
}

/**
 * patch_apply() - Process FOTA
 *
 * Update firmware which loaded to local before.
 *
 * Return: None
 *
 */
void fota_entry(void)
{
    fota_info("fota_entry start\n");

#ifdef FOTA_EXTEND_CHECK_DEBUG
    extend_check();
#endif

    if (AIC_SUCCESS == patch_apply()) {
#ifndef CONFIG_FOTA_BY_UART
#ifndef CONFIG_FOTA_BY_MCU
        /*
         * When the func extend_restore is not fully executed,
         * the device suddenly powered off. It will become a
         * scence like this, after finished the basic images update,
         * the fota stage has not been changed.
         */

         /* So, first, set the stage to FOTA_STAGE_EXTEND_IMG_DL.*/
        g_fota_para.stage = FOTA_STAGE_EXTEND_IMG_DL;
        set_fota_param();

        /* Than, after restore succ, change state to FOTA_STAGE_EXTEND_DIFF_DL. */
        if (0 == extend_restore())
            g_fota_para.stage = FOTA_STAGE_EXTEND_DIFF_DL;
#else
        g_fota_para.stage = FOTA_STAGE_EXTEND_IMG_DL;
#endif
        g_fota_para.state = FOTA_STATE_NONE;
        g_fota_para.finished_parti_cnt = 0;

        set_fota_param();
        close_fota_param();
        bl_reset_basic();
#else
        /*
         * TODO:
         * CONFIG_FOTA_BY_UART open, has not extend image temporary.
         * need change the stae FOTA_STAGE_UPDATE_DONE.
         * need repalce it with a more accurate feature.
         */
        g_fota_para.stage = FOTA_STAGE_UPDATE_DONE;
        g_fota_para.state = FOTA_STATE_NONE;
        g_fota_para.finished_parti_cnt = 0;

        set_fota_param();
        close_fota_param();
        bl_reset();
#endif
    } else {
#ifndef CONFIG_FOTA_BY_UART
        /* Apply_patch failed, try adjust the fota stage. */
        fota_try_adjust_the_state(FALSE);
#ifndef CONFIG_FOTA_BY_MCU
        extend_restore();
#endif
#endif
        close_fota_param();
        bl_reset();
    }

    fota_info("fota_entry end\n");
    log_print_r("fota_entry end\n");
}

/**
 * chksum_verify() - Verify image checksum
 *
 * Check checksum of every image to make sure that the
 * difference is valid. For finished partition, compare
 * new checksum, for others, compare old checksum.
 *
 * Return: AIC_SUCCESS verify pass; Others verify fail.
 *
 */
s32 chksum_verify(void)
{
    s32 ret = AIC_SUCCESS;
    struct list_head *pos;
    dpkg_item_t *itm;
    u32 i = 0;
    u32 staddr;
    u16 imgchksum, calchksum;
    u32 chklen, offset = 0;
    char *name;

    list_for_each(pos, &g_dpkg_info.items) {
        itm = list_entry(pos, dpkg_item_t, list);
        /* a new image start, need verify checksum */
        name = itm->name;
        if (itm->splitcnt != 0)
            offset = itm->splitidx * g_dpkg_info.splitsz;
        else
            offset = 0;

        if ((i == g_fota_para.finished_parti_cnt) &&
            partbk_is_valid()) {
            name = FOTA_PTNAME;
            /* offset should be FOTA_BK_OFFSET, if use bk part */
            offset = FOTA_BK_OFFSET;
        }
        ret = part_addr(name, &staddr, 0);
        FAILURE_RETURN
        if (i < g_fota_para.finished_parti_cnt) {
            imgchksum = itm->newchksum;
            if (part_is_compressed(name, offset))
                chklen = itm->newcompsz;
            else
                chklen = itm->newimgsz;
        } else {
            imgchksum = itm->oldchksum;
            chklen    = itm->oldimgsz;
        }
        fota_info("chksum_verify staddr 0x%x, len 0x%x, offset 0x%x\n",
                  staddr, chklen, offset);
        calchksum = fota_checksum((u16 *)(staddr + offset), chklen);
        fota_info("chksum_verify part %s, imgchksum 0x%x, calchksum 0x%x,"
                  "part num %d\n",
                  itm->name, imgchksum, calchksum, i);
        if (imgchksum != calchksum) {
            ret = AIC_EINVAL;
            goto err_ret;
        }
        i++;
        if (i == g_dpkg_info.partn)
            ret = AIC_SUCCESS;
    }

    if (i != g_dpkg_info.partn)
        fota_info("chksum_verify failed, i(%d) != g_dpkg_info.partn(%d).\n",
                  g_dpkg_info.partn, i);
err_ret:
    return ret;
}

/**
 * fota_check() - check fota upgrade data is valid or not
 *
 * Check ota status flag and backup ota status flag, if any of
 * them equals s1(0x6f746173),return true, or return false.
 *
 * Return: 0 if fota check passed; -1 if fota parse pkg or checksum vertify failed; -2 others' failure.
 *
 */
s32 fota_check()
{
    s32 ret = AIC_SUCCESS;

    fota_info("fota_check start!\n");

    ret = fmd_open(FOTA_PARA_PTNAME, &fotaparamparthdl);
    if (ret != AIC_SUCCESS) {
        fota_info("fota_check open fota param partition failed, errno(%d)!\n", ret);
        ret = OTHER_ERROR;
        goto err_ret;
    }
    ret = fmd_get_partition_size(fotaparamparthdl, &fotaparampartsz);
    if (ret != AIC_SUCCESS) {
        fota_info("fota_check get fota param partition size failed, errno(%d)!\n",
                  ret);
        ret = OTHER_ERROR;
        goto err_ret;
    }

#ifdef CONFIG_FOTA_BY_MCU
    (void)get_fota_old_ver_check();
#endif

    ret = get_fota_param();
    if (ret != AIC_SUCCESS) {
        fota_info("fota_check get fota param failed, errno(%d)!\n",
                  ret);
        ret = OTHER_ERROR;
        goto err_ret;
    }
    fota_info("fota_check fota state(0x%x),fincnt(0x%x),stage(0x%x)\n", g_fota_para.state,
        g_fota_para.finished_parti_cnt, g_fota_para.stage);

    if (g_fota_para.stage == FOTA_STAGE_BASIC_UPDATE &&
        g_fota_para.state == FOTA_STATE_DONE) {
        /* get difference package information */
        ret = parse_dpkg();
        if (ret != AIC_SUCCESS) {
            ret = PARSE_DPKG_ERROR;
            goto err_ret;
        }
        ret = chksum_verify();
        if (ret != AIC_SUCCESS) {
            ret = CHECKSUM_VERIFY_ERROR;
            goto err_ret;
        }
    } else if ((g_fota_para.stage == FOTA_STAGE_EXTEND_DIFF_DL &&
                g_fota_para.state != FOTA_STATE_NONE) ||
               (g_fota_para.stage == FOTA_STAGE_EXTEND_IMG_DL &&
                g_fota_para.state != FOTA_STATE_NONE)) {
        g_fota_para.state = FOTA_STATE_NONE;
        set_fota_param();
        close_fota_param();
        bl_reset_basic();
    } else {
        ret = OTHER_ERROR;
    }

err_ret:
    return ret;
}

/**
 * fota_enter() - Do fota if fota check passed.
 *
 * Do fota if fota check passed.
 *
 * Return: NONE.
 *
 */
#if defined(CONFIG_BL) && defined(CONFIG_GRAPHICS)
#include <display.h>
#include "cache.h"
#include <display_cfg.h>
#include <timer.h>

#define __CACHE_LINE_SIZE             32U

char fota_state[] __attribute__((aligned(__CACHE_LINE_SIZE)))= {
    #include "pciture_160_128.h"
};

int fota_show_state(void)
{
    char *fb;
    s32 ret;

    fb = (char *)malloc(LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);
    if (!fb) {
        log_print("fota: malloc fail.\n");
        return -1;
    }

    dcache_invcc_by_range((u32)fb, LCD_PANEL_WIDTH * LCD_PANEL_HEIGHT * 2);

    ret = hlzma_dec_polling((u32)fota_state, (u32)fb);
    if (ret)
        log_print("decompress picture failed, 0x%s\n", ret);
    aic_mdelay(50);
    display_refresh((u32)fb);
    aic_bl_brightness_set(80);
    free(fb);

    return ret;
}
#else
int fota_show_state(void)
{}
#endif

s32 fota_enter(void)
{
    s32 ret = AIC_SUCCESS;
    fota_info("enter fota boot check...\n");

    ret = fota_check();
    if (AIC_SUCCESS == ret) {
        fota_show_state();
        fota_entry();
    } else if ((PARSE_DPKG_ERROR == ret) ||
               (CHECKSUM_VERIFY_ERROR == ret)) {
        /* The dpkg err, try adjust the fota stage. */
        fota_try_adjust_the_state(TRUE);
#if (!defined(CONFIG_FOTA_BY_UART) && !defined(CONFIG_FOTA_BY_MCU))
        extend_restore();
#endif
        close_fota_param();
        bl_reset();
    }

    close_fota_param();
    fota_info("normal boot\n");
    return AIC_SUCCESS;
}

/**
 * fota_boot_register() - Register fota boot mode.
 *
 * Register fota entry.
 *
 * Return: always success.
 *
 */
static s32 fota_boot_register(void)
{
    platform_boot_mode_enterpoint_register(fota_enter, FOTA_BOOT);
    /* after fota, the system boot as normal */
    platform.boot_mode = NORMAL_BOOT;
    return AIC_SUCCESS;
}
driver_init_late(fota_boot_register);
