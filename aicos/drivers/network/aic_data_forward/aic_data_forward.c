/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1ad22e
#define LOG_MODULE    AMOD_DRV

#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aipc_api.h>
#include <aic_cmd.h>
#include <aic_osa.h>
#include <aos_mem.h>
#include <timer_inc.h>
#include <cmux.h>
#include <cmux_cfg.h>
#include <aic_network_pcap.h>

#define AIC_DATA_BLOCK_OFFSET           24
#define AIC_DATA_TASK_PRIORITY          6

#define AIC_DATA_MAX_INSTANCES          8

#define AIC_DATA_NOTIFY_L2_REBUILD      1

enum data_chan {
    NORMAL_DATA,
    HIGH_DATA,
};

typedef struct aos_mbuf_st
{
  struct aos_mbuf_st  *next_ptr;
  unsigned char       *data_ptr;
  unsigned short      data_offset;
  unsigned short      data_len;
  unsigned short      user_num;
  unsigned char       ext;
  unsigned char       cached_b:1,
                      free_data_b:1,
                      prio_b:1,
                      ping_b:1,
                      mem_owner:2,  /* 0:AP;1:CP;2:TC */
                      data_type:2;  /* 0:USER APP;1:ims_signaling;2:ims_voice;3:ims_video */
  unsigned int        ext1;
} aos_mbuf;

struct aic_data_nic_driver
{
    u16 pdp_active_flag;

    u32 s_set;

    volatile u32 s_link_up_status;

    aos_thread_id recv_thread_id;
    aos_thread_id recv_host_thread_id;

    ABRR_HANDLE aipc_recv_handle;
    ABRR_HANDLE aipc_send_handle;

    struct aos_mbuf_st *send_mbuf_head;
    struct aos_mbuf_st *send_mbuf_last;

    void *cmux_handle;

    u32 last_trigger_fast_rebuild;

    u32 current_send_cache_size;
    u32 max_send_cache_size;

    u32 recv_from_host_buff[1500];
};

static struct aic_data_nic_driver aic_data_devices;

static int aic_data_link_up_callback(u8 link_id, link_state_t link_status)
{
    AOS_INTERRUPT_DECLARE
    struct aic_data_nic_driver *nic_dev = &aic_data_devices;

    aicx_info(0x18ce1a, "dd", "[AIC_DATA]callback:link %d state %d\n", link_id, link_status);

    AOS_INTERRUPT_DISABLE
    if (link_status) {
        nic_dev->s_link_up_status |= BIT(link_id);
        AOS_INTERRUPT_RESTORE
    } else {
        nic_dev->s_link_up_status &= ~BIT(link_id);
        AOS_INTERRUPT_RESTORE
    }

    return AIC_SUCCESS;
}

static void aic_data_identify_pri(u8 *data, u32 len, struct aos_mbuf_st *mbuf)
{
    u8 ip_header_len, tcp_header_len;

    if ((*data & 0xF0) == 0x40) {
        if (*(data + 9) == 6) {
            ip_header_len = (*data >> 4) * (*data & 0x0F);
            tcp_header_len = (*(data + ip_header_len + 12) >> 4) * 4;
            if ((ip_header_len + tcp_header_len) == len)
                mbuf->ext = 1;
        }
    } else if ((*data & 0xF0) == 0x60) {
        if (*(data + 6) == 6) {
            ip_header_len = 40;
            tcp_header_len = (*(data + ip_header_len + 12) >> 4) * 4;
            if ((ip_header_len + tcp_header_len) == len)
                mbuf->ext = 1;
        }
    }
}

static int aic_data_driver_send(struct aos_mbuf_st *mbuf)
{
    int ret, i;
    u8 link_id;
    ABLK_T blk;
    u32 prev_timestamp;
    struct aic_data_nic_driver *nic_dev = &aic_data_devices;

    blk.addr = mbuf;
    blk.size = mbuf->data_len + mbuf->data_offset;

    aic_data_identify_pri(mbuf->data_ptr + mbuf->data_offset, mbuf->data_len,
                          mbuf);
    for (i = 0; i < 32; i++) {
        if (nic_dev->s_link_up_status & BIT(i)) {
            break;
        }
    }

    if (i >= 32) {
        aicx_error(0x0754f6, "x", "[AIC_DATA] link status not ready 0x%x\n",
                   nic_dev->s_link_up_status);
        ablk_recycle_block(nic_dev->aipc_send_handle, &blk);
        return AIC_ENODEV;
    }

    if (mbuf->ext)
        link_id = i + 1;
    else
        link_id = i;

    aic_network_pcap_dump(mbuf->data_ptr + mbuf->data_offset,
                          mbuf->data_len, AIC_PDP_NIC_FLAG);
    prev_timestamp = (u32)aic_get_tick();
    aicx_info(0x050d41, "dd", "[FORWARD] send network data to link id %d len %d\n", link_id, mbuf->data_len);
    ret = ablk_send_block(nic_dev->aipc_send_handle, link_id, &blk,
                          5000);
    if (ret != AIC_SUCCESS) {
       aicx_error(0x121aeb, "dd", "[AIC_DATA] send blk fail %d %d\n", link_id, ret);
       ablk_recycle_block(nic_dev->aipc_send_handle, &blk);
    }

    if (aic_get_tick() - prev_timestamp > 1000)
        aicx_warn(0x1e0fb7, "d", "[AIC_DATA] send data wait too long %d\r\n",
                  (u32)aic_get_tick() - prev_timestamp);

    return AIC_SUCCESS;
}

static int aic_data_recv(struct aic_data_nic_driver *nic_dev, u32 link_id)
{
    ABLK_T blk;
    int ret, i;
    u32 block_cnt;
    size_t size;
    struct aos_mbuf_st *mbuf;

    block_cnt = ablk_get_recv_block_count(nic_dev->aipc_recv_handle, link_id);
    aicx_info(0x12b649, "dd", "[AIC_DATA] recv block cnt %d link_id %d\n", block_cnt, link_id);

    for (i = 0; i < block_cnt; i++) {
        memset(&blk, 0, sizeof(blk));
        ret = ablk_recv_block(nic_dev->aipc_recv_handle, link_id, &blk,
                              AIPC_NO_WAIT);
        mbuf = blk.addr;
        if (!mbuf || ret) {
            aicx_error(0x113816, "d", "[AIC_DATA] recv mbuf is null ret %d\n", ret);
            continue;
        }

        aicx_debug(0x084f9f, "xxxxx", "[AIC_DATA] recv data ptr 0x%x offset 0x%x len 0x%x val 0x%x 0x%x\n",
                   mbuf->data_ptr, mbuf->data_offset, mbuf->data_len,
                   *(u32 *)(mbuf->data_ptr + mbuf->data_offset),
                   *(u32 *)(mbuf->data_ptr + mbuf->data_offset + 4));

        aic_network_pcap_dump(mbuf->data_ptr + mbuf->data_offset,
                              mbuf->data_len, AIC_PDP_NIC_FLAG);
        /* Send to host */
        size = aic_cmux_write(nic_dev->cmux_handle, mbuf->data_ptr + mbuf->data_offset,
                              mbuf->data_len, 1000);
        if (size != mbuf->data_len) {
            aicx_warn(0x0c5b31, "d", "[AI_DATA] cmux write fail %d\n", size);
        }
    }

    return AIC_SUCCESS;
}

static void aic_data_recv_host_task(void *p)
{
    AMSG_T msg;
    int ret;
    ABLK_T blk = {0};
    size_t read_size = 0;
    struct aos_mbuf_st *mbuf;
    struct aic_data_nic_driver *nic_dev = p;

    while (1) {
        read_size = aic_cmux_read(nic_dev->cmux_handle ,
                                  nic_dev->recv_from_host_buff,
                                  1500, AOS_WAIT_FOREVER);
        if (read_size < 20)
            continue;

        aicx_info(0x10b8d8, "d", "[FORWARD] recv %d from cmux\n", read_size);
        ret = ablk_get_block(nic_dev->aipc_send_handle,
                             sizeof(struct aos_mbuf_st) + read_size + AIC_DATA_BLOCK_OFFSET,
                             &blk, 500);
        if ((ret != AIC_SUCCESS) || !blk.addr) {
            if ((aic_get_tick() - nic_dev->last_trigger_fast_rebuild) > 5000) {
                amsg_fill_msg(&msg, AMOD_EL2, AIC_DATA_NOTIFY_L2_REBUILD,
                              AIC_DATA_NOTIFY_L2_REBUILD);
                amsg_send_msg(AIC_SYS_CP, &msg, 0);
                nic_dev->last_trigger_fast_rebuild = aic_get_tick();
            }
        }

        mbuf = blk.addr;
        memset(mbuf, 0, sizeof(struct aos_mbuf_st));
        mbuf->data_len = read_size;
        mbuf->data_offset = sizeof(struct aos_mbuf_st) + AIC_DATA_BLOCK_OFFSET;
        mbuf->data_ptr = blk.addr;
        memcpy(mbuf->data_ptr + mbuf->data_offset, nic_dev->recv_from_host_buff,
               read_size);

        aic_data_driver_send(mbuf);
        memset(&blk, 0, sizeof(blk));
    }
}

static void aic_data_recv_task(void *p)
{
    int i;
    u32 links, link_id;
    struct aic_data_nic_driver *nic_dev = p;

    while (1) {
        links = ablk_select(nic_dev->aipc_recv_handle, nic_dev->s_set,
                            AIPC_WAIT_FOREVER);

        for (i = 0; i < AIC_DATA_MAX_INSTANCES; i++) {
            if (links & BIT((i << 1) + HIGH_DATA)) {
                link_id = (i << 1) + HIGH_DATA;
                aicx_debug(0x04388b, "d", "[AIC_DATA] recv from link_id %d\n", link_id);
                aic_data_recv(nic_dev, link_id);
                links &= ~BIT((i << 1) + HIGH_DATA);
            }

            if (links & BIT(i << 1)) {
                link_id = (i << 1);
                aicx_debug(0x04388b, "d", "[AIC_DATA] recv from link_id %d\n", link_id);
                aic_data_recv(nic_dev, link_id);
                links &= ~BIT(i << 1);
            }
        }
    }
}

#ifdef CONFIG_PLAT_CONSOLE
static int aic_data_debug_cmd(int argc, char **argv)
{
    CONS_OUT("[DATA] s_set 0x%x s_link_up_status 0x%x cmux_handle 0x%x\r\n",
             aic_data_devices.s_set, aic_data_devices.s_link_up_status,
             aic_data_devices.cmux_handle);

    return AIC_SUCCESS;
}
CONSOLE_EXPORT_CMD(data_debug, aic_data_debug_cmd, data debug);
#endif

int aic_data_nic_init(void)
{
    int link;
    int status = AIC_SUCCESS;

    memset(&aic_data_devices, 0, sizeof(aic_data_devices));

    aic_data_devices.aipc_recv_handle = ablk_request_brr(AIC_SYS_CP, AIPC_BRR_IP);
    if (!aic_data_devices.aipc_recv_handle) {
        aicx_error(0x1e72f9, "", "[driver] request recv brr fail\n");
        return AIC_EPERM;
    }

    aic_data_devices.aipc_send_handle = ablk_request_brr(AIC_SYS_CP,
                                                         AIPC_BRR_IP);
    if (!aic_data_devices.aipc_send_handle) {
        aicx_error(0x011251, "", "[driver] request send brr fail\n");
        return AIC_EPERM;
    }

    aic_data_devices.recv_thread_id =
        aos_create_thread("data-recv", aic_data_recv_task,
                          (void *)&aic_data_devices, SIZE_2K, 0, NULL,
                          AIC_DATA_TASK_PRIORITY, AOS_AUTO_START);
    if (aic_data_devices.recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x063e03, "", "[driver] create lte recv task fail\n");

        return AIC_EPERM;
    }

    aic_data_devices.recv_host_thread_id =
        aos_create_thread("data-recv-host", aic_data_recv_host_task,
                          (void *)&aic_data_devices, SIZE_2K, 0, NULL,
                          AIC_DATA_TASK_PRIORITY, AOS_AUTO_START);
    if (aic_data_devices.recv_thread_id == AOS_INVALID_THREAD_ID) {
        aicx_error(0x0502ca, "", "[driver] create data-recv-host task fail\n");
        aos_delete_thread(aic_data_devices.recv_thread_id);

        return AIC_EPERM;
    }

    aic_data_devices.s_set = (1 << (AIC_DATA_MAX_INSTANCES * 2)) - 1;
    aic_data_devices.cmux_handle = aic_cmux_open(AIC_CMUX_DATA_COM);

    aic_data_devices.max_send_cache_size = aos_get_heap_size() / 10 * 2;
    for (link = 0; link < ablk_get_link_cnt(aic_data_devices.aipc_send_handle, FALSE);
         link++) {
        if (ablk_get_a_link_state(aic_data_devices.aipc_send_handle, link,
                                  AIC_FALSE) == ALINK_STATE_LINKED)
            aic_data_link_up_callback(link, ALINK_STATE_LINKED);
    }
    ablk_listen_a_blk(aic_data_devices.aipc_send_handle,
                      aic_data_link_up_callback);

    return status;
}
AIC_MODEM_INIT_EXPORT(aic_data_nic_init);

