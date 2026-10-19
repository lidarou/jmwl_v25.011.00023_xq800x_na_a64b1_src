/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file include download command callback handler.
 */
#include <aic_errno.h>
#include <stdlib.h>
#include <string.h>
#include "download/dload_cmd.h"
#include "download/dload_kit.h"
#include "download/dload_packet.h"
#include "console/console.h"
#include <charger.h>
#include "initcall.h"
#include <fmd.h>
#include "boot_mode.h"
#include <efuse.h>
#include <pmic_auto_reg.h>
#include <pmic_io.h>
#include <timer.h>
#include <lpr.h>
#include <string.h>
#include <ftlblock.h>
#include <ftlrandom.h>

#ifdef CONFIG_FS
#include "vfs_init.h"
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/syslimits.h>
#include <errno.h>
#endif

#ifdef CONFIG_SSP
#include <ssp.h>
#endif

typedef int (*cmd_operate) (packet_body *);

static DLOAD_CONDITION dload_file;
u32 cur_ptn_handle;

cmd_operate op_table[PCK_CMD_TYPE_MAX - PCK_CMD_TYPE_MIN] = {0, 0};

#define CMD_INDEX(cmd)                ((cmd) - PCK_CMD_TYPE_MIN)
#define CMD_HANDLER(cmd)              (op_table[CMD_INDEX(cmd)])

/*data channel connect packet callback*/
int packet_connect(struct packet_body *pkt)
{
    u32 val, pmic_id;
    u32 pmic_sw_register = REG_GLB_UPS_PMIC_SW_REGISTER;

    pmic_id = (pmic_readl(REG_GLB_PD_CHIP_ID_VERSION) >> 16) & 0xFFFF;
    if (pmic_id == 0x201) {
        pmic_sw_register = REG_GLB_UPS_PMIC_SW_REGISTER;
    } else if (pmic_id == 0x202) {
        pmic_sw_register = 0x30;
    }

    val = pmic_readl(pmic_sw_register);
    pmic_writel(pmic_sw_register, val | BIT_15);

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

static void packet_data_parse(struct packet_body *pkt, uint8_t *partition_name, uint64_t *size, uint32_t *checksum)
{
    uint16_t *data = (uint16_t *) (pkt->content);
    uint32_t  i = 0;

    /* legacy format */
    if (pkt->size < BIT32_IDLEN_DATA_LENGTH) {
        partition_name[i] = '\0';
        *size = 0;
        return;
    }

    /* 64 proc */
    if (BIT64_IDLEN_DATA_LENGTH <= pkt->size) {
        *size = *(uint64_t *) (data + MAX_PARTITION_NAME_SIZE);
        /* ID(72)+ LEN(8) +Rev(8) [+CS] */
        if ((NULL != checksum) && (BIT64_IDLEN_DATA_LENGTH < pkt->size))
            *checksum = *(uint32_t *) (data + MAX_PARTITION_NAME_SIZE + 4 + 4);
    } else {/* 32 proc */
        *size = (uint64_t)*(uint32_t *) (data + MAX_PARTITION_NAME_SIZE);
        /* ID(72)+ LEN(4) [+CS] */
        if ((NULL != checksum) && (BIT32_IDLEN_DATA_LENGTH < pkt->size))
            *checksum = *(uint32_t *) (data + MAX_PARTITION_NAME_SIZE + 2);
    }

    log_print("packet->body.size:0x%x, image size:0x%llx\n", pkt->size, *size);

    while (('\0' != *(data+i)) && (i < (MAX_PARTITION_NAME_SIZE-1))){
        partition_name[i] = *(data+i) & 0xFF;
        i++;
    }
    partition_name[i] = '\0';

    return;
}

#ifdef CONFIG_FS_DL
#define FS_MAX_DL_SIZE  0x80000
u32 fmd_handle, ftl_handle;
u32 fs_dl_offset = 0;

boolean partition_is_fs(const char *ptn_name)
{
    if ((strcmp(ptn_name, "C") == 0) || (strcmp(ptn_name, "E") == 0))
        return true;

    return false;
}

int fs_partition_init(const char *ptn_name)
{
    u32 size;
    int ret;

    ret = fmd_open((char *)ptn_name, &fmd_handle);
    if (ret < 0) {
        log_print("open fs partiton(%s) failed, errno(%d)!\n", ptn_name, ret);
        return ret;
    }

    ret = fmd_get_partition_size(fmd_handle, &size);
    if (ret) {
        log_print("get fs partiton(%s) size failed, errno(%d)!\n", ptn_name, ret);
        return ret;
    }

    ret = fmd_erase(fmd_handle, 0, size);
    if (ret) {
        log_print("erase fs partition(%s) failed, errno(%d)!\n", ptn_name, ret);
        return ret;
    }

#ifdef CONFIG_FATFS_DL
    ret = fmd_close(fmd_handle);
    if (ret) {
        log_print("close fs partition(%s) failed, errno(%d)!\n", ptn_name, ret);
        return ret;
    }

    ret = ftl_init();
    if (ret != AIC_SUCCESS) {
        log_print("fs ftl init fail, ret=%d.\n", ret);
        return ret;
    }

    ret = ftlrandom_open((char *)ptn_name, &ftl_handle);
    if (ret != AIC_SUCCESS) {
        log_print("fs ftl random open fail, ret=%d.\n", ret);
        return ret;
    }
#endif
    return 0;
}

int fs_partition_write(u32 offset, u32 size, u8 *buf)
{
    int ret;

#ifdef CONFIG_FATFS_DL
    ret = ftlrandom_write(ftl_handle, offset, size, buf);
#else
    ret = fmd_write(fmd_handle, offset, size, buf);
#endif

    if (ret != AIC_SUCCESS) {
        log_print("fs partition write fail, ret=%d.\n", ret);
    }

    return ret;
}

int fs_partition_close(const char *ptn_name)
{
    int ret;

#ifdef CONFIG_FATFS_DL
    ret = ftlrandom_close(ftl_handle);
#else
    ret = fmd_close(fmd_handle);
#endif

    if (ret != AIC_SUCCESS) {
        log_print("fs partition(%s) close fail, ret=%d.\n", ptn_name, ret);
    } else {
        log_print("download fs partition(%s) successful!\n", ptn_name);
    }

    return ret;
}
#endif

bool is_ptn_need_verify(unsigned char *ptn_name)
{
    bool ret = false;

    if (!strcmp((char *)ptn_name, "mbl") ||
        !strcmp((char *)ptn_name, "bl") ||
        !strcmp((char *)ptn_name, "ap") ||
        !strcmp((char *)ptn_name, "ap_extend")
        )
        ret = true;

    return ret;
}

int packet_start(struct packet_body *pkt)
{
    unsigned char *p_recv = NULL;
    uint32_t nv_checksum, msize, ptn_size;
    uint64_t size;
    int ret;

    packet_data_parse(pkt, dload_file.ptn_name, &size, &nv_checksum);

#ifdef HDLC
    start_addr = EndianConv_32(start_addr);
    file_size  = EndianConv_32(file_size);
#endif

    msize = size;
    if (is_ptn_need_verify(dload_file.ptn_name)) {
        p_recv = (unsigned char *)malloc(sizeof(unsigned char) * msize);
        if (p_recv == NULL) {
            log_print("%s(%d) not enought space to alloc!!\n", __func__, __LINE__);
            send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
            return 0;
        }
        memset((void *)p_recv, 0, sizeof(unsigned char) * msize);
#ifdef CONFIG_FS_DL
    } else if (partition_is_fs((const char *)dload_file.ptn_name)) {
        if (size > FS_MAX_DL_SIZE) {
            msize = FS_MAX_DL_SIZE;
        }

        p_recv = (unsigned char *)malloc(sizeof(unsigned char) * msize);
        if (p_recv == NULL) {
            log_print("%s(%d) not enought space to alloc!!\n", __func__, __LINE__);
            send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
            return 0;
        }
        memset((void *)p_recv, 0, sizeof(unsigned char) * msize);

        int ret = fs_partition_init((const char *)dload_file.ptn_name);
        if (ret != 0) {
            send_ack_packet(PCK_REP_INCOMPATIBLE_PARTITION);
            return 0;
        }
#endif
    } else {
        ret = fmd_open((char*)dload_file.ptn_name, &cur_ptn_handle);
        if (ret < 0) {
            send_ack_packet(PCK_REP_OPERATION_FAILED);
            log_print("open partiton failed, errno(%d)!\n", ret);
            return ret;
        }

        ret = fmd_get_partition_size(cur_ptn_handle, (u32 *)&ptn_size);
        if (ret) {
            send_ack_packet(PCK_REP_OPERATION_FAILED);
            log_print("get partiton size failed, errno(%d)!\n", ret);
            return ret;
        }

        ret = fmd_erase(cur_ptn_handle, 0, ptn_size);
        if (ret) {
            send_ack_packet(PCK_REP_OPERATION_FAILED);
            log_print("erase partition failed, errno(%d)!\n", ret);
            return ret;
        }
    }
    log_print("partition name:%s size:0x%lx\n", dload_file.ptn_name, size);

    /*temp receive download img start address*/
    dload_file.start_address = (unsigned int)p_recv;
    dload_file.total_size = size;
    dload_file.recv_size = 0;
    dload_file.next_address = dload_file.start_address;

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_change_baud(struct packet_body *pkt)
{
    uint32_t *data = (uint32_t *)pkt->content;
    uint32_t baudrate;

    baudrate = *data;

    log_print("dload uart baudrate set:%d\n", baudrate);
    if(cur_dl_chan->baudrate_set) {
        send_ack_packet(PCK_REP_ACK);
        cur_dl_chan->baudrate_set(cur_dl_chan->data, baudrate);
    } else {
        log_print("current channel don't support change baudrate!\n");
        send_ack_packet(PCK_REP_INVALID_CMD);
    }

    return 0;
}

/*middle packet callback*/
int packet_middle(struct packet_body *pkt)
{
    unsigned short data_len = pkt->size;
    int ret;

    if (is_ptn_need_verify(dload_file.ptn_name)) {
        memcpy((void *)dload_file.next_address, (const void *)(pkt->content), data_len);
#ifdef CONFIG_FS_DL
    } else if (partition_is_fs((const char *)dload_file.ptn_name)) {
        if ((dload_file.recv_size + data_len) > FS_MAX_DL_SIZE) {
            int ret = fs_partition_write(fs_dl_offset,
                                        dload_file.recv_size,
                                        (unsigned char *)dload_file.start_address);
            if (ret != 0) {
                send_ack_packet(PCK_REP_OPERATION_FAILED);
                return 0;
            }
            fs_dl_offset += dload_file.recv_size;
            dload_file.next_address = dload_file.start_address;
            dload_file.recv_size = 0;
        } else {
            memcpy((void *)dload_file.next_address, (const void *)(pkt->content), data_len);
        }
#endif
    } else {
        ret = fmd_write(cur_ptn_handle, dload_file.recv_size, data_len, (u8 *)(pkt->content));
        if (ret) {
            log_print("write image to partition fail!\n");
            send_ack_packet(PCK_REP_OPERATION_FAILED);
            return 0;
        }
    }

    dload_file.next_address += data_len;
    dload_file.recv_size += data_len;

    send_ack_packet(PCK_REP_ACK);
    return 0;
}


static int packet_write_partition(char *ptn_name, unsigned char *img_buf,
                                            unsigned int img_size)
{
    u32 img_ptn_handle;
    u32 size;
    int ret;

    ret = fmd_open(ptn_name, &img_ptn_handle);
    if (ret < 0) {
        log_print("open partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    lpr_disable_kick();

    ret = fmd_get_partition_size(img_ptn_handle, &size);
    if (ret) {
        log_print("get partiton size failed, errno(%d)!\n", ret);
        return ret;
    }

    lpr_disable_kick();

    ret = fmd_erase(img_ptn_handle, 0, size);
    if (ret) {
        log_print("erase partition failed, errno(%d)!\n", ret);
        return ret;
    }

    lpr_disable_kick();

    ret = fmd_write(img_ptn_handle, 0, img_size, img_buf);
    if (ret < 0) {
        log_print("write partition failed, errno(%d)!\n", ret);
        return ret;
    }

    lpr_disable_kick();

    log_print("download write ptn(%s) successful!\n",ptn_name);
    return 0;
}

static int packet_read_partition(char *ptn_name, u32 offset, u32 size,
                                 unsigned char *img_buf)
{
    u32 img_ptn_handle;
    int ret;

    lpr_disable_kick();

    ret = fmd_open(ptn_name, &img_ptn_handle);
    if (ret < 0) {
        log_print("open partiton failed, errno(%d)!\n", ret);
        return ret;
    }

    ret = fmd_read(img_ptn_handle, offset, size, img_buf);
    if (ret < 0) {
        log_print("read partion failed, errno(%d)!\n", ret);
        return ret;
    }

    lpr_disable_kick();

    ret = fmd_close(img_ptn_handle);
    if (ret < 0) {
        log_print("close partion failed, errno(%d)!\n", ret);
        return ret;
    }

    return 0;
}

int packet_end(struct packet_body *pkt)
{
    struct secure_boot *sec_boot = &platform.secure_boot;
    int ret;

#ifdef CONFIG_FS_DL
    if (partition_is_fs((const char *)dload_file.ptn_name)) {
        if (dload_file.recv_size) {
            ret = fs_partition_write(fs_dl_offset, dload_file.recv_size, (unsigned char *)dload_file.start_address);
            if (ret != 0) {
                send_ack_packet(PCK_REP_OPERATION_FAILED);
                goto exit;
            }
        }

        ret = fs_partition_close((const char *)dload_file.ptn_name);
        if (ret) {
            send_ack_packet(PCK_REP_OPERATION_FAILED);
        } else {
            send_ack_packet(PCK_REP_ACK);
        }

        goto exit;
    } else
#endif
    if (!is_ptn_need_verify(dload_file.ptn_name)) {
        send_ack_packet(PCK_REP_ACK);
//        if (!strcmp((char *)dload_file.ptn_name, DOWNLOAD_LAST_PARTITION))
//            lpr_disable_loop();
        return 0;
    }

    if (sec_boot->secure_boot_enable &&
        sec_boot->img_attr_check((char *)dload_file.ptn_name)) {
        /*reset security verify image address*/
        platform.next_img = (uint8_t *)dload_file.start_address;
        ret = sec_boot->secboot_authorization(&platform);
        if (ret) {
            log_print("secboot authorization fail!\n");
            send_ack_packet(PCK_REP_OPERATION_FAILED);
            goto exit;
        }
    }

    ret = packet_write_partition((char*)dload_file.ptn_name,
                                (unsigned char *)dload_file.start_address,
                                (u32)dload_file.total_size);
    if(ret) {
        log_print("write image to partition fail!\n");
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        goto exit;
    }

    send_ack_packet(PCK_REP_ACK);

exit:
    free((unsigned char *)dload_file.start_address);

//    if (!strcmp((char *)dload_file.ptn_name, DOWNLOAD_LAST_PARTITION))
//        lpr_disable_loop();

    return ret;
}

int packet_exec(struct packet_body *pkt)
{
    /*set PC to target binary start address, will complete later*/
    send_ack_packet(PCK_REP_ACK);
    //never return
    return 0;
}

int sysoff_reset_dl(struct packet_body *pkt)
{
    bl_reset();
    return 0;
}

int sysoff_poweroff_dl(struct packet_body *pkt)
{
    aic_delay(5);
    bl_shutdown();
    return 0;
}


int charger_powerdonw_dl(struct packet_body *pkt)
{
    charger_force_powerdown();

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_get_uid(struct packet_body *pkt)
{
    packet_body *packet_ptr;
    int ret, len;

    packet_ptr = (packet_body *)malloc(sizeof(packet_body));
    if (packet_ptr == NULL) {
        log_print("alloc memory fail!\n");
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return AIC_ENOSPC;
    }

    memset(packet_ptr, 0, sizeof(packet_body));

    ret =get_uid(packet_ptr->content, &len);
    if(ret) {
        log_print("get uid failed!\n");
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        free(packet_ptr);
        return ret;
    }

    packet_ptr->type = PCK_REP_GET_UID;
    packet_ptr->size = len;
    send_packet(packet_ptr);
    free(packet_ptr);

    return ret;
}

int packet_read_start(struct packet_body *pkt)
{
    uint32_t nv_checksum;
    uint64_t size;

    packet_data_parse(pkt, dload_file.ptn_name, &size, &nv_checksum);

    log_print("read partition name:%s total_size:%d\n", dload_file.ptn_name, size);

    dload_file.total_size = size;
    dload_file.start_address = 0;

    dload_file.p_read_ptn = (packet_body *)malloc(sizeof(packet_body));
    if( dload_file.p_read_ptn == NULL ) {
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return AIC_ENOBUFS;
    }

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

int packet_read_middle(struct packet_body *pkt)
{
    unsigned int *data = (unsigned int *)(pkt->content);
    unsigned int read_size = *data;
    packet_body *packet_ptr;
    int ret;

#ifdef HDLC
    read_size  = EndianConv_32(read_size);
#endif

    packet_ptr = dload_file.p_read_ptn;

    memset(packet_ptr, 0, sizeof(packet_body));
    ret = packet_read_partition((char *)dload_file.ptn_name, dload_file.start_address, read_size,
                                packet_ptr->content);
    if(ret) {
        log_print("read data from Flash partition fail!\n");
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        free(dload_file.p_read_ptn);
        return ret;
    }

    dload_file.start_address += read_size;
    packet_ptr->type = PCK_REP_READ_NOR_FLASH;
    packet_ptr->size = read_size;
    send_packet(packet_ptr);

    dload_file.total_size -= read_size;
    return ret;
}

int packet_read_end(struct packet_body *pkt)
{
    if (dload_file.total_size != 0)
        log_print("read total size not match with Host requset!!!\n");

    free(dload_file.p_read_ptn);
    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_backup_read_start(struct packet_body *pkt)
{
    packet_body *packet_ptr;
    unsigned int paddr, psize;
    uint64_t dummy1;
    uint32_t dummy2;
    int ret;

    packet_data_parse(pkt, dload_file.ptn_name, &dummy1, &dummy2);

    log_print("read back partition name:%s\n", dload_file.ptn_name);

    ret = fmd_get_old_partition_info((char *)(dload_file.ptn_name),
                                        &paddr, &psize);
    if (ret) {
        send_ack_packet(ret);
        return -1;
    }

    packet_ptr = (packet_body *)malloc(sizeof(packet_body));
    if (packet_ptr == NULL) {
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return AIC_ENOBUFS;
    }

    dload_file.p_read_ptn = packet_ptr;
    dload_file.total_size = psize;
    dload_file.start_address = paddr;

    memset(packet_ptr, 0, sizeof(packet_body));
    packet_ptr->type = PCK_REP_READ_NOR_FLASH;
    packet_ptr->size = sizeof(psize);
    memcpy(packet_ptr->content, &psize, sizeof(psize));
    send_packet(packet_ptr);

    return 0;
}

int packet_backup_read_middle(struct packet_body *pkt)
{
    unsigned int *data = (unsigned int *)(pkt->content);
    unsigned int read_size = *data;
    packet_body *packet_ptr;

#ifdef HDLC
    read_size  = EndianConv_32(read_size);
#endif

    packet_ptr = dload_file.p_read_ptn;
    memset(packet_ptr, 0, sizeof(packet_body));
    memcpy(packet_ptr->content, (void *)(dload_file.start_address), read_size);

    dload_file.start_address += read_size;
    packet_ptr->type = PCK_REP_READ_NOR_FLASH;
    packet_ptr->size = read_size;
    send_packet(packet_ptr);

    dload_file.total_size -= read_size;

    return 0;
}

int packet_backup_read_end(struct packet_body *pkt)
{
    if (dload_file.total_size != 0)
        log_print("read total size not match with Host requset!!!\n");

    free(dload_file.p_read_ptn);
    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_update_ptable(struct packet_body *pkt)
{
    int ret;

    ret = fmd_partition_table_update();
    if (ret) {
        log_print("fmd update partition table failed(0x%x)!!!\n", ret);
        send_ack_packet(ret);
        return -1;
    }

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

int packet_check_ptable(struct packet_body *pkt)
{
    int ret;

    ret = fmd_partition_table_check();
    if (ret) {
        send_ack_packet(ret);
        return -1;
    }

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

int packet_erase_flash(struct packet_body *pkt)
{
    fmd_erase_all_chips();

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

int packet_erase_partition(struct packet_body *pkt)
{
    uint64_t dummy1;
    uint32_t dummy2;
    u32 handle, size;
    int ret;

    packet_data_parse(pkt, dload_file.ptn_name, &dummy1, &dummy2);

    log_print("erase partition:%s\n", dload_file.ptn_name);

    ret = fmd_open((char *)dload_file.ptn_name, &handle);
    ASSERT(ret == AIC_SUCCESS);

    ret = fmd_get_partition_size(handle, &size);
    ASSERT(ret == AIC_SUCCESS);

    ret = fmd_erase(handle, 0, size);
    ASSERT(ret == AIC_SUCCESS);

    ret = fmd_close(handle);
    ASSERT(ret == AIC_SUCCESS);

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

#ifdef CONFIG_FS
int packet_fs_write_start(struct packet_body *pkt)
{
    unsigned char *p_recv = NULL;
    uint32_t nv_checksum;
    uint64_t size;
    int ret;

    packet_data_parse(pkt, dload_file.ptn_name, &size, &nv_checksum);

#ifdef HDLC
    size  = EndianConv_32(size);
#endif

    ret = vfs_init();
    if (ret != AIC_SUCCESS) {
        log_print("vfs init fail!\n");
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return ret;
    }

    p_recv = (unsigned char *)malloc(sizeof(unsigned char) * size);
    if (p_recv == NULL) {
        log_print("%s(%d) not enought space to alloc!!\n", __func__, __LINE__);
        send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
        return 0;
    }

    log_print("file name:%s size:0x%lx\n", dload_file.ptn_name, size);

    memset((void *)p_recv, 0, sizeof(unsigned char) * size);
    /*temp receive download img start address*/
    dload_file.start_address = (unsigned int)p_recv;
    dload_file.total_size = size;
    dload_file.recv_size = 0;
    dload_file.next_address = dload_file.start_address;

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_fs_write_middle(struct packet_body *pkt)
{
    unsigned short data_len = pkt->size;

    memcpy((void *)dload_file.next_address, (const void *)(pkt->content), data_len);

    dload_file.next_address += data_len;
    dload_file.recv_size += data_len;

    send_ack_packet(PCK_REP_ACK);
    return 0;
}

int packet_fs_write_end(struct packet_body *pkt)
{
    ssize_t rsize;
    int ret, fd;
    char *name;

    name = malloc(PATH_MAX);
    if (name == NULL) {
        log_print("malloc size fail.\n", PATH_MAX);
        send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
        return AIC_ENOMEM;
    }
    memset(name, 0x0, PATH_MAX);
    snprintf(name, PATH_MAX, "usr/share/firmware/%s.bin", dload_file.ptn_name);

    ret = unlink(name);
    if (ret != AIC_SUCCESS) {
        log_print("file %s unlink fail, ret=%d.\n", dload_file.ptn_name, -errno);
    }

    fd = open(name, O_RDWR | O_CREAT);
    free(name);
    if (fd < 0) {
        log_print("file %s open fail, ret=%d.\n", dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    rsize = write(fd, (unsigned char *)dload_file.start_address,
                  dload_file.total_size);
    if (rsize < 0) {
        close(fd);
        log_print("file %s write fail, ret=%d.\n", dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    ret = close(fd);
    if (ret != AIC_SUCCESS) {
        log_print("file %s close fail, ret=%d.\n",
                  dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    if (rsize != dload_file.total_size) {
        log_print("file %s write size(0x%x 0x%x) not match with Host requset.\n",
                  dload_file.ptn_name, rsize, dload_file.total_size);
        send_ack_packet(PCK_REP_EEROR_CRC);
        return AIC_EINVAL;
    }

    log_print("download write file(%s) successful!\n", dload_file.ptn_name);

    send_ack_packet(PCK_REP_ACK);
    return AIC_SUCCESS;
}

int packet_fs_read_start(struct packet_body *pkt)
{
    packet_body *packet_ptr;
    uint64_t dummy1;
    uint32_t dummy2;
    struct stat st;
    int ret, fd;
    char *name;
    off_t pos;

    packet_data_parse(pkt, dload_file.ptn_name, &dummy1, &dummy2);

    log_print("read fs file name:%s\n", dload_file.ptn_name);

    ret = vfs_init();
    if (ret != AIC_SUCCESS) {
        log_print("fs init fail, ret=%d.\n", ret);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return ret;
    }

    name = malloc(PATH_MAX);
    if (name == NULL) {
        log_print("malloc size fail.\n", PATH_MAX);
        send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
        return AIC_ENOMEM;
    }
    memset(name, 0x0, PATH_MAX);
    snprintf(name, PATH_MAX, "etc/%s.bin", dload_file.ptn_name);

    fd = open(name, O_RDONLY);
    if (fd < 0) {
        log_print("file %s open fail, ret=%d.\n", name, -errno);
        free(name);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    free(name);

    pos = lseek(fd, 0, 0);
    if (pos != 0) {
        close(fd);
        log_print("file %s lseek fail, ret=%d.\n",
                  dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    ret = fstat(fd, &st);
    if (ret != 0) {
        close(fd);
        log_print("file %s get fstat fail, ret=%d.\n",
                  dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    packet_ptr = (packet_body *)malloc(sizeof(packet_body));
    if (packet_ptr == NULL) {
        close(fd);
        log_print("malloc size fail.\n", sizeof(packet_body));
        send_ack_packet(PCK_PHONE_NOT_ENOUGH_MEMORY);
        return AIC_ENOMEM;
    }

    dload_file.start_address = (unsigned int)fd;
    dload_file.p_read_ptn = packet_ptr;
    dload_file.total_size = st.st_size;

    memset(packet_ptr, 0, sizeof(packet_body));
    packet_ptr->type = PCK_REP_READ_FS;
    packet_ptr->size = sizeof(st.st_size);
    memcpy(packet_ptr->content, &st.st_size, sizeof(st.st_size));
    send_packet(packet_ptr);

    return 0;
}

int packet_fs_read_middle(struct packet_body *pkt)
{
    unsigned int *data = (unsigned int *)(pkt->content);
    unsigned int read_size = *data;
    packet_body *packet_ptr;
    ssize_t rsize;
    int fd;

#ifdef HDLC
    read_size  = EndianConv_32(read_size);
#endif

    packet_ptr = dload_file.p_read_ptn;
    memset(packet_ptr, 0, sizeof(packet_body));

    fd = dload_file.start_address;
    rsize = read(fd, packet_ptr->content, read_size);
    if (rsize < 0) {
        log_print("file %s read size(0x%x) fail, ret=%d.\n",
                  dload_file.ptn_name, read_size, -errno);
        close(fd);
        free(dload_file.p_read_ptn);
        send_ack_packet(PCK_REP_EEROR_CRC);
        return -errno;
    }

    packet_ptr->type = PCK_REP_READ_FS;
    packet_ptr->size = rsize;
    send_packet(packet_ptr);

    dload_file.total_size -= rsize;

    return 0;
}

int packet_fs_read_end(struct packet_body *pkt)
{
    int ret, fd;

    if (dload_file.total_size != 0) {
        log_print("file %s read total size not match with Host requset!!!\n",
                  dload_file.ptn_name);
    }

    free(dload_file.p_read_ptn);

    fd = dload_file.start_address;
    ret = close(fd);
    if (ret < 0) {
        log_print("file %s close fail, ret=%d.\n",
                  dload_file.ptn_name, -errno);
        send_ack_packet(PCK_REP_OPERATION_FAILED);
        return -errno;
    }

    log_print("download read file(%s) successful!\n", dload_file.ptn_name);

    send_ack_packet(PCK_REP_ACK);
    return 0;
}
#endif

int packet_ssp_info_record(struct packet_body *pkt)
{
#ifdef CONFIG_SSP
    int ret;

    ret = ssp_parse_cfg_info(pkt->content);
    if (ret) {
        log_print("ssp info parse failed(0x%x)!!!\n", ret);
        send_ack_packet(ret);
        return -1;
    }
#endif

    send_ack_packet(PCK_REP_ACK);

    return 0;
}

void dload_cmd_register (CMD_TYPE cmd, cmd_operate proc)
{
    CMD_HANDLER(cmd) = proc;
}

void dl_cmd_operate(CMD_TYPE cmd,packet_body *packet)
{
    if (!CMD_HANDLER(cmd)) {
        send_ack_packet(PCK_REP_INVALID_CMD);
        lpr_disable_kick();
        return;
    }

    CMD_HANDLER(cmd)(packet);
    lpr_disable_kick();
}

static int dload_cmd_init(void)
{
    dload_cmd_register(PCK_CMD_CONNECT, packet_connect);
    dload_cmd_register(PCK_CMD_START_DATA, packet_start );
    dload_cmd_register(PCK_CMD_MIDST_DATA, packet_middle);
    dload_cmd_register(PCK_CMD_END_DATA, packet_end);
    dload_cmd_register(PCK_CMD_EXEC_DATA, packet_exec);
    dload_cmd_register(PCK_CMD_READ_NOR_FLASH_START, packet_read_start);
    dload_cmd_register(PCK_CMD_READ_NOR_FLASH_MIDST, packet_read_middle);
    dload_cmd_register(PCK_CMD_READ_NOR_FLASH_END, packet_read_end);
    dload_cmd_register(PCK_CMD_CHANGE_BAUD, packet_change_baud);
    dload_cmd_register(PCK_CMD_UPDATE_PTABLE, packet_update_ptable);
    dload_cmd_register(PCK_CMD_CHECK_PTABLE, packet_check_ptable);
    dload_cmd_register(PCK_CMD_ERASE_FLASH, packet_erase_flash);
    dload_cmd_register(PCK_CMD_ERASE_PARTITION, packet_erase_partition);
    dload_cmd_register(PCK_CMD_BACKUP_READ_START, packet_backup_read_start);
    dload_cmd_register(PCK_CMD_BACKUP_READ_MIDST, packet_backup_read_middle);
    dload_cmd_register(PCK_CMD_BACKUP_READ_END, packet_backup_read_end);
    dload_cmd_register(PCK_CMD_SYS_RESET, sysoff_reset_dl);
    dload_cmd_register(PCK_CMD_CHG_PD, charger_powerdonw_dl);
    dload_cmd_register(PCK_CMD_SYS_PD, sysoff_poweroff_dl);
    dload_cmd_register(PCK_CMD_GET_UID, packet_get_uid);
#ifdef CONFIG_FS
    dload_cmd_register(PCK_CMD_FS_WRITE_START, packet_fs_write_start);
    dload_cmd_register(PCK_CMD_FS_WRITE_MIDST, packet_fs_write_middle);
    dload_cmd_register(PCK_CMD_FS_WRITE_END, packet_fs_write_end);
    dload_cmd_register(PCK_CMD_FS_READ_START, packet_fs_read_start);
    dload_cmd_register(PCK_CMD_FS_READ_MIDST, packet_fs_read_middle);
    dload_cmd_register(PCK_CMD_FS_READ_END, packet_fs_read_end);
#endif
    dload_cmd_register(PCK_CMD_SSP_INFO_RECORD, packet_ssp_info_record);

    return 0;
}

driver_init(dload_cmd_init);
