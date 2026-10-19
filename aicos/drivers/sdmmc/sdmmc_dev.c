/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the sdmmc dev driver.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1de7fe
#define LOG_MODULE    AMOD_SDMMC

#include <aic_errno.h>
#include <sdmmc_core.h>
#include <sdmmc_common.h>
#include <aic_init.h>
#include <sdmmc_dev.h>

#define SECTOR_SIZE      512
#define MAX_TRAN_SIZE    (8*1024/SECTOR_SIZE)
//#define SDMMC_DEV_TEST
#ifdef SDMMC_DEV_TEST
void sdmmc_dev_test();
#endif
struct sdmmc_dev_st sdmmc_dev;

int32_t fs_dev_sdmmc_add();
int32_t fs_dev_sdmmc_remove();

static int sdmmc_req_blk(sdmmc_card *card,
                              uint32_t sector,
                              void *buf,
                              size_t blks,
                              uint8_t dir)
{
    sdmmc_cmd cmd, stop;
    sdmmc_data data;
    sdmmc_req req;
    sdmmc_host *host;
    uint32_t r_cmd, w_cmd;

    if (!card)
        return AIC_EINVAL;
    host = card->host;

    sdmmc_host_lock(host);
    memset(&req, 0, sizeof(sdmmc_req));
    memset(&cmd, 0, sizeof(sdmmc_cmd));
    memset(&stop, 0, sizeof(sdmmc_cmd));
    memset(&data, 0, sizeof(sdmmc_data));
    req.cmd = &cmd;
    req.data = &data;

    cmd.arg = sector;
    if (!(card->flags & CARD_FLAG_SDHC))
    {
        cmd.arg <<= 9;
    }
    cmd.flags = RESP_R1 | CMD_ADTC;

    data.blksize = SECTOR_SIZE;
    data.blks  = blks;

    if (blks > 1)
    {
        req.stop = &stop;
        stop.cmd_code = STOP_TRANSMISSION;
        stop.arg = 0;
        stop.flags = RESP_R1B | CMD_AC;
        r_cmd = READ_MULTIPLE_BLOCK;
        w_cmd = WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        req.stop = NULL;
        r_cmd = READ_SINGLE_BLOCK;
        w_cmd = WRITE_BLOCK;
    }

    if (!dir)
    {
        cmd.cmd_code = r_cmd;
        data.flags |= DATA_DIR_READ;
    }
    else
    {
        cmd.cmd_code = w_cmd;
        data.flags |= DATA_DIR_WRITE;
    }

    sdmmc_set_data_timeout(&data, card);
    data.buf = buf;
    sdmmc_send_request(host, &req);

    if (cmd.err || data.err){
        aicx_info(0x12a7b1, "", "sdmmc_req_blk fail!");
        return AIC_EIO;
    }

    if (dir != 0)
    {
        do
        {
            int32_t err;

            cmd.cmd_code = SEND_STATUS;
            cmd.arg = card->rca << 16;
            cmd.flags = RESP_R1 | CMD_AC;
            err = sdmmc_send_cmd(card->host, &cmd, 0);
            if (err)
            {
                aicx_info(0x17fb61, "d", "error %d requesting status", err);
                break;
            }
            /*
             * Some cards mishandle the status bits,
             * so make sure to check both the busy
             * indication and the card state.
             */
         } while (!(cmd.resp[0] & R1_READY_FOR_DATA) ||
            (R1_CURRENT_STATE(cmd.resp[0]) == 7));
    }

    sdmmc_host_unlock(host);

    if (cmd.err || data.err || stop.err)
    {
        aicx_info(0x1190a6, "", "sdmmc request blocks error");
        aicx_info(0x05d5f5, "dddxx", "%d,%d,%d, 0x%08x,0x%08x",
                   cmd.err, data.err, stop.err, data.flags, sector);

        return AIC_EIO;
    }

    return AIC_SUCCESS;
}


static int32_t sdmmc_set_blksize(sdmmc_card *card, u32 size)
{
    sdmmc_cmd cmd;
    int err;

    if (!card)
        return AIC_EINVAL;

    /* Block-addressed cards ignore MMC_SET_BLOCKLEN. */
    if (card->flags & CARD_FLAG_SDHC)
        return 0;

    sdmmc_host_lock(card->host);
    cmd.cmd_code = SET_BLOCKLEN;
    cmd.arg = size;
    cmd.flags = RESP_R1 | CMD_AC;
    err = sdmmc_send_cmd(card->host, &cmd, 0);
    sdmmc_host_unlock(card->host);

    if (err) {
        aicx_info(0x1dafb1, "d", "set block size to %d fail!", cmd.arg, err);
        return AIC_EIO;
    }

    return 0;
}

int sdmmc_open(char *name, u32 *handle)
{
    AOS_INTERRUPT_DECLARE

    AOS_INTERRUPT_DISABLE

    if (!sdmmc_dev.card) {
        aicx_info(0x13da28, "", "fs_sdmmc_open fail, card not exist!");
        AOS_INTERRUPT_RESTORE
        return AIC_EIO;
    }
    sdmmc_dev.ref_count++;
    *handle = (u32)&sdmmc_dev;

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

int sdmmc_close(u32 handle)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;
    AOS_INTERRUPT_DECLARE

    if (!pdev)
        return AIC_EINVAL;

    AOS_INTERRUPT_DISABLE

    pdev->ref_count--;

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

int sdmmc_read(u32 handle, u32 sector_id, u32 sector_num, u8 *buf)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;
    int err = 0;
    size_t offset = 0;
    size_t req_size = 0;
    size_t remain_size = sector_num;
    void *rd_ptr = (void *)buf;

    if (!buf || !pdev)
        return AIC_EINVAL;

    if (sector_id >= pdev->sector_num ||
        sector_num > (pdev->sector_num - sector_id))
        return AIC_EINVAL;

    while (remain_size)
    {
        req_size = (remain_size > pdev->max_req_size) ? pdev->max_req_size : remain_size;
        err = sdmmc_req_blk(pdev->card, sector_id+offset, rd_ptr, req_size, 0);
        if (err != AIC_SUCCESS)
            break;
        offset += req_size;
        rd_ptr = (void *)((uint8_t *)rd_ptr + (req_size << 9));
        remain_size -= req_size;
    }

    return err;

}

int sdmmc_write(u32 handle, u32 sector_id, u32 sector_num, u8 *buf)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;
    int err = 0;
    size_t offset = 0;
    size_t req_size = 0;
    size_t remain_size = sector_num;
    void *wr_ptr = (void *)buf;

    if (!buf || !pdev)
        return AIC_EINVAL;

    if (sector_id >= pdev->sector_num ||
        sector_num > (pdev->sector_num - sector_id))
        return AIC_EINVAL;

    while (remain_size)
    {
        req_size = (remain_size > pdev->max_req_size) ? pdev->max_req_size : remain_size;
        err = sdmmc_req_blk(pdev->card, sector_id+offset, wr_ptr, req_size, 1);
        if (err != AIC_SUCCESS)
            break;
        offset += req_size;
        wr_ptr = (void *)((uint8_t *)wr_ptr + (req_size << 9));
        remain_size -= req_size;
    }

    return err;

}

int sdmmc_release(u32 handle, u32 sector_id, u32 sector_num)
{
    return AIC_SUCCESS;
}

int sdmmc_get_sector_size(u32 handle, u32 *retsize)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retsize = pdev->sector_size;

    return AIC_SUCCESS;
}

int sdmmc_get_sector_num(u32 handle, u32 *retnum)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retnum = pdev->sector_num;

    return AIC_SUCCESS;
}

int sdmmc_get_partition_size(u32 handle, u32 *retsize)
{
    struct sdmmc_dev_st *pdev = (struct sdmmc_dev_st *)handle;

    if (!pdev)
        return AIC_EINVAL;

    *retsize = pdev->total_size;

    return AIC_SUCCESS;
}

int sdmmc_add(sdmmc_card *card)
{
    int err = 0;

    err = sdmmc_set_blksize(card, SECTOR_SIZE);
    if(err)
    {
        return err;
    }

    sdmmc_dev.card = card;
    sdmmc_dev.max_req_size = MAX_TRAN_SIZE;
    sdmmc_dev.total_size = ((u64)card->card_capacity) * 1024;
    sdmmc_dev.sector_size = SECTOR_SIZE;
    sdmmc_dev.sector_num = sdmmc_dev.total_size / SECTOR_SIZE;

#ifdef SDMMC_DEV_TEST
    sdmmc_dev_test();
#endif

    fs_dev_sdmmc_add();
    sdmmc_dev.card_status = 0x1;

    return 0;
}

int sdmmc_remove(sdmmc_card *card)
{
    /* invalid card return directly */
    if (!sdmmc_dev.card_status)
        return 0;

    AIC_ASSERT(sdmmc_dev.card == card);

    sdmmc_dev.card = AIC_NULL;
    sdmmc_dev.max_req_size = 0;

    fs_dev_sdmmc_remove();
    sdmmc_dev.card_status = 0x0;

    return 0;
}

u32 sdmmc_get_card_status()
{
    return sdmmc_dev.card_status;
}


#ifdef SDMMC_DEV_TEST
#define TEST_LEN 8192
#define TEST_LOOP 200
u8 rd_buf[TEST_LEN];
u8 wr_buf[TEST_LEN];
void sdmmc_dev_test()
{
    u32 handle = 0;
    u32 i=0, j=0;
    u32 time, time_w, time_r;
    int err;

    sdmmc_open("D:", &handle);

    fill_buffer(wr_buf, TEST_LEN, RND_TYPE_INC_B, 1);
    fill_buffer(rd_buf, TEST_LEN, RND_TYPE_CONST, 0);

    while(1) {

        aicx_info(0x0eff86, "u", "SDMMC R/W %u test...\r\n", j++);

        time = aos_get_tickcount();
        for (i=0; i<TEST_LOOP; i++) {
            //alog_info("Write data...\r\n");
            err = sdmmc_write(handle, i*TEST_LEN/512, TEST_LEN/512, wr_buf);
            AIC_ASSERT(!err);
        }
        time_w = aos_get_tickcount() - time;

        time = aos_get_tickcount();
        for (i=0; i<TEST_LOOP; i++) {
            //alog_info("Read data...\r\n");
            err = sdmmc_read(handle, i*TEST_LEN/512, TEST_LEN/512, rd_buf);
            AIC_ASSERT(!err);
        }
        time_r = aos_get_tickcount() - time;

        //alog_info("Compare buffers\r\n");
        if (compare_buffers(wr_buf, rd_buf, TEST_LEN)) {
            aicx_error(0x020956, "", "SDMMC test failed\r\n");
            sdmmc_close(handle);
            AIC_ASSERT(0);
            return;
        }

        aicx_info(0x0926f9, "uu", "SDMMC R/W Test, W: %u Byte/S, R: %u Byte/S !\r\n",
                   (TEST_LEN*TEST_LOOP*1000)/time_w, (TEST_LEN*TEST_LOOP*1000)/time_r);

        aicx_info(0x0728d2, "", "Test passed\r\n\r\n");
    }

    sdmmc_close(handle);

}
#endif

