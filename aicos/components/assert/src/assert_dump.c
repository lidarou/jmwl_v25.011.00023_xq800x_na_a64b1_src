/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines assert dump memory interface.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c0bad
#define LOG_MODULE    AMOD_SYS

#include <aic_assert.h>
#include <aic_errno.h>
#include <aic_io.h>
#include <aic_math.h>
#include <aic_osa.h>
#include <assert_adpter.h>
#include <assert_cmd.h>
#include <assert_dump.h>
#include <subsys_api.h>

#ifdef CONFIG_AIC_CTRL_SUPPORT
#include <aic_ctrl.h>
#endif

#define ASSERT_DUMP_BLOCK_SIZE      0xFFF0
#define ASSERT_DUMP_REG_BUF_SIZE    0x100

static u32 assert_dump_buf[ASSERT_DUMP_REG_BUF_SIZE / 4];

static LIST_HEAD(assert_dump_list);

static u32 assert_dump_reg_read(u32 address)
{
    return AIC_READ32(address);
}

static u32 assert_dump_hreg_read(u32 address)
{
    return AIC_READ16(address);
}

#if 0
void assert_dump_hword(diag_recv_cmd_t *diag_cmd, u16 val)
{
    assert_diag_send(diag_cmd, (void *)(&val), 2);
}

void assert_dump_word(diag_recv_cmd_t *diag_cmd, u32 val)
{
    assert_diag_send(diag_cmd, (void *)(&val), 4);
}
#endif

static void assert_dump_data(diag_recv_cmd_t *diag_cmd, void *address,
                          u32 length)
{
    assert_diag_send(diag_cmd, address, length);
}

static void assert_dump_mem_area(diag_recv_cmd_t *diag_cmd,
                                 #ifdef CONFIG_SUBSYS_CTRL_SUPPORT
                                 u8 sys,
                                 u8 mpu_conf,
                                 #endif
                                 u32 address,
                                 u32 length, dump_prepare_f pre_func,
                                 dump_post_f pst_func)
{
    u32 i, block_num, remainder_len;
    u32 start = address;
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    int index;
#endif

    block_num = length / ASSERT_DUMP_BLOCK_SIZE;
    remainder_len = length % ASSERT_DUMP_BLOCK_SIZE;

    if (pre_func)
        pre_func();

#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    if (mpu_conf) {
        index = subsys_mpu_config_sys(sys);
        if (index < 0)
            return;
    } else {
        index = -1;
    }
#endif

    for (i = 0; i < block_num; i++) {
        assert_dump_data(diag_cmd, (void *)start, ASSERT_DUMP_BLOCK_SIZE);
        start += ASSERT_DUMP_BLOCK_SIZE;
    }

    if (remainder_len != 0)
        assert_dump_data(diag_cmd, (void *)start, remainder_len);

#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    if (index >= 0)
        subsys_mpu_unconfig_region(index);
#endif

    if (pst_func)
        pst_func();
}

static void assert_dump_reg_area(diag_recv_cmd_t *diag_cmd, u32 address,
                                 u32 length, dump_read_f func,
                                 dump_prepare_f pre_func, dump_post_f pst_func,
                                 dump_check_f check_func, u32 default_val)
{
    u32 i, j, block_num, remainder_len;
    u32 start = address, reg_addr;
    u32 *read_buf = assert_dump_buf;
    dump_read_f read_func = NULL;
    u32 check_result = 0;

    block_num = length / ASSERT_DUMP_REG_BUF_SIZE;
    remainder_len = length % ASSERT_DUMP_REG_BUF_SIZE;

    if (pre_func)
        pre_func();

    if (func)
        read_func = func;
    else
        read_func = assert_dump_reg_read;

    if (check_func)
        check_result = check_func();
    else
        check_result = 1;

    memset(read_buf, 0x0, ASSERT_DUMP_REG_BUF_SIZE);

    for (i = 0; i < block_num; i++) {
        reg_addr = start;

        for (j = 0; j < (ASSERT_DUMP_REG_BUF_SIZE / 4); j++) {
            read_buf[j] = (check_result) ? (read_func(reg_addr)) : (default_val);
            reg_addr += 4;
        }

        assert_dump_data(diag_cmd, (void *)read_buf, ASSERT_DUMP_REG_BUF_SIZE);
        start += ASSERT_DUMP_REG_BUF_SIZE;

        memset(read_buf, 0x0, ASSERT_DUMP_REG_BUF_SIZE);
    }

    if (remainder_len != 0) {
        reg_addr = start;

        for (j = 0; j < (remainder_len / 4); j++) {
            read_buf[j] = (check_result) ? (read_func(reg_addr)) : (default_val);
            reg_addr += 4;
        }

        assert_dump_data(diag_cmd, (void *)read_buf, remainder_len);
    }

    if (pst_func)
        pst_func();
}

static void assert_dump_hreg_area(diag_recv_cmd_t *diag_cmd, u32 address,
                                  u32 length, dump_read_f func,
                                  dump_prepare_f pre_func, dump_post_f pst_func)
{
    u32 i, j, block_num, remainder_len;
    u32 start = address, reg_addr;
    u16 *read_buf = (u16 *)&assert_dump_buf[0];
    dump_read_f read_func = NULL;

    block_num = length / ASSERT_DUMP_REG_BUF_SIZE;
    remainder_len = length % ASSERT_DUMP_REG_BUF_SIZE;

    if (pre_func)
        pre_func();

    if (func)
        read_func = func;
    else
        read_func = assert_dump_hreg_read;

    memset(read_buf, 0x0, ASSERT_DUMP_REG_BUF_SIZE);

    for (i = 0; i < block_num; i++) {
        reg_addr = start;

        for (j = 0; j < (ASSERT_DUMP_REG_BUF_SIZE / 2); j++) {
            read_buf[j] = read_func(reg_addr) & 0xFFFF;
            reg_addr += 2;
        }

        assert_dump_data(diag_cmd, (void *)read_buf, ASSERT_DUMP_REG_BUF_SIZE);
        start += ASSERT_DUMP_REG_BUF_SIZE;

        memset(read_buf, 0x0, ASSERT_DUMP_REG_BUF_SIZE);
    }

    if (remainder_len != 0) {
        reg_addr = start;

        for (j = 0; j < (remainder_len / 2); j++) {
            read_buf[j] = read_func(reg_addr) & 0xFFFF;
            reg_addr += 2;
        }

        assert_dump_data(diag_cmd, (void *)read_buf, remainder_len);
    }

    if (pst_func)
        pst_func();
}

void assert_dump_handle(diag_recv_cmd_t *diag_cmd)
{
    struct assert_dump_info *dump;
    struct assert_dump_head head;

    list_for_each_entry(dump, &assert_dump_list, node,
                                            struct assert_dump_info) {
        memset(&head, 0x0, sizeof(struct assert_dump_head));
        head.magic = ASSERT_DUMP_MAGIC_NUM;
        head.type = dump->type;
        head.address = dump->address;
        head.length = dump->length;

        assert_dump_data(diag_cmd, (void *)&head, sizeof(head));

        if (dump->type == DUMP_MEM)
            assert_dump_mem_area(diag_cmd,
                                 #ifdef CONFIG_SUBSYS_CTRL_SUPPORT
                                 dump->sys, dump->need_conf_mpu,
                                 #endif
                                 dump->address, dump->length,
                                 dump->pre_f, dump->pst_f);
        else if(dump->type == DUMP_REG)
            assert_dump_reg_area(diag_cmd, dump->address, dump->length,
                                 dump->read, dump->pre_f, dump->pst_f,
                                 dump->check_f, dump->default_val);
        else if(dump->type == DUMP_RF_WREG)
            assert_dump_reg_area(diag_cmd, dump->address, dump->length,
                                 dump->read, dump->pre_f, dump->pst_f,
                                 dump->check_f, dump->default_val);
        else if(dump->type == DUMP_RF_HREG)
            assert_dump_hreg_area(diag_cmd, dump->address, dump->length,
                                 dump->read, dump->pre_f, dump->pst_f);
        else
            AIC_ASSERT(0);
    }

    /* give a dump finsh cmd to pc. */
    diag_cmd->subcmd = ASSERT_DUMP_FINISH;
    assert_diag_send(diag_cmd, NULL, 0);

#ifdef CONFIG_SUBSYS_AP
#ifdef CONFIG_AIC_CTR_DBG_ON_CMUX
    aic_ctrl_str_notify(DUMP_FINISH);
#endif
#else
    /* dump finsh notify to ap. */
    cp_assert_notify_ap(DUMP_FINISH);
#endif
}

void assert_dump_region_register_ex(enum assert_dump_type type,
                                    #ifdef CONFIG_SUBSYS_CTRL_SUPPORT
                                    u8 sys,
                                    u8 mpu_conf,
                                    #endif
                                    u32 address, u32 length,
                                    dump_read_f func,
                                    dump_prepare_f pre_func,
                                    dump_post_f pst_func,
                                    dump_check_f check_func, u32 default_val)
{
    struct assert_dump_info *dump;

    dump = aos_malloc(sizeof(*dump));
    AIC_ASSERT(dump != NULL);

    list_head_init(&dump->node);
    dump->type = type;
#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
    dump->sys = sys;
    dump->need_conf_mpu = mpu_conf;
#endif
    dump->address = address;
    dump->length = length;
    dump->read = func;
    dump->pre_f = pre_func;
    dump->pst_f = pst_func;
    dump->check_f = check_func;
    dump->default_val = default_val;

    list_add_tail(&dump->node, &assert_dump_list);
}

void assert_dump_region_register(enum assert_dump_type type, u32 address,
                                 u32 length, dump_read_f func,
                                 dump_prepare_f pre_func,
                                 dump_post_f pst_func)
{
    assert_dump_region_register_ex(type,
                                  #ifdef CONFIG_SUBSYS_CTRL_SUPPORT
                                  AIC_THIS_SYS, 0,
                                  #endif
                                  address, length,
                                  func, pre_func, pst_func, NULL, 0x0);
}

#ifdef CONFIG_SUBSYS_CTRL_SUPPORT
void assert_dump_region_register_mem(u8 sys, u8 mpu_conf,
                                     u32 address, u32 length)
{
    assert_dump_region_register_ex(DUMP_MEM, sys, mpu_conf, address, length,
                                   NULL, NULL, NULL, NULL, 0x0);
}
#endif

