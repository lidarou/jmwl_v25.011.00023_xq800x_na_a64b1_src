/*****************************************************************************/
/*                                                                           */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the spi nor flash driver struct.
 */

#ifndef __SNOR_H__
#define __SNOR_H__

#include <aic_type.h>
#include <aic_bitmap.h>
#include <assert.h>
#include <core_op.h>
#include <fmdcore.h>
#include <reg_base.h>

/* flash common commands */
#define SNOR_CMD_WRDI      0x04    /* Write disable */
#define SNOR_CMD_WREN      0x06    /* Write enable */
#define SNOR_CMD_RDSR      0x05    /* Read status register */
#define SNOR_CMD_WRSR      0x01    /* Write status register 1 byte */

#define SNOR_CMD_READ              0x03 /* Read(low frequency) */
#define SNOR_CMD_READ_FAST         0x0b /* Read(high frequency) */
#define SNOR_CMD_READ_DUAL_OUTPUT  0x3b /* Read(Dual Output SPI) */
#define SNOR_CMD_READ_DUAO_IO      0xbb /* Read(Dual I/O SPI) */
#define SNOR_CMD_READ_QUAD_OUTPUT  0x6b /* Read(Quad Output SPI) */
#define SNOR_CMD_READ_QUAD_IO      0xeb /* Read(Quad I/O) */

#define SNOR_CMD_PP             0x02   /* Page program (up to 256 bytes) */
#define SNOR_CMD_PP_QUAD        0x32   /* Quad page program */

#define SNOR_CMD_SE_4K      0x20    /* Erase 4KB sector */
#define SNOR_CMD_BE_32K     0x52    /* Erase 32KB block */
#define SNOR_CMD_BE_64K     0xD8    /* Erase 64KB block */
#define SNOR_CMD_CE         0xc7    /* Erase whole flash */

#define SNOR_CMD_RDID       0x9f    /* Read JEDEC ID */
#define SNOR_CMD_RDSFDP     0x5a    /* Read SFDP */
#define SNOR_CMD_RSTEN      0x66    /* Reset Enable */
#define SNOR_CMD_RST        0x99    /* Reset */

#define SNOR_CMD_RDSR2         0x35    /* Read status register2 */

#define SNOR_CMD_PE_SUSPEND    0x75    /* Program/Erase suspend */
#define SNOR_CMD_PE_RESUME     0x7A    /* Program/Erase resume */

#define SNOR_CMD_DPD        0xB9 /* enter deep power down mode */
#define SNOR_CMD_RPD        0xAB /* release from deep power down mode */

/* status register */
#define SR_WIP          BIT(0)  /* Erase/Write in progress */
#define SR_WEL          BIT(1)  /* Write enable latch */

/* snor signal transmission protocol */
#define SNOR_STP_CMD_MASK     GENMASK(16, 19)
#define SNOR_STP_CMD_SHIFT    16
#define SNOR_STP_CMD(bits)  \
    ((((u64)(bits)) << SNOR_STP_CMD_SHIFT) & \
     SNOR_STP_CMD_MASK)

#define SNOR_STP_ADDR_MASK    GENMASK(12, 15)
#define SNOR_STP_ADDR_SHIFT   12
#define SNOR_STP_ADDR(bits) \
    ((((u64)(bits)) << SNOR_STP_ADDR_SHIFT) & \
     SNOR_STP_ADDR_MASK)

#define SNOR_STP_MODE_MASK    GENMASK(8, 11)
#define SNOR_STP_MODE_SHIFT   8
#define SNOR_STP_MODE(bits) \
    ((((u64)(bits)) << SNOR_STP_MODE_SHIFT) & \
     SNOR_STP_MODE_MASK)

#define SNOR_STP_DUMMY_MASK   GENMASK(4, 7)
#define SNOR_STP_DUMMY_SHIFT  4
#define SNOR_STP_DUMMY(bits) \
    ((((u64)(bits)) << SNOR_STP_DUMMY_SHIFT) & \
     SNOR_STP_DUMMY_MASK)

#define SNOR_STP_DATA_MASK    GENMASK(0, 3)
#define SNOR_STP_DATA_SHIFT   0
#define SNOR_STP_DATA(bits) \
    ((((u64)(bits)) << SNOR_STP_DATA_SHIFT) & \
     SNOR_STP_DATA_MASK)

#define SNOR_STP(cmd, addr, mode, dummy, data) \
    (SNOR_STP_CMD(cmd) | \
     SNOR_STP_ADDR(addr) | \
     SNOR_STP_MODE(mode) | \
     SNOR_STP_DUMMY(dummy) | \
     SNOR_STP_DATA(data))

enum snor_stp {
    SNOR_STP_1_0_0_0_0 = SNOR_STP(1, 0, 0, 0, 0),
    SNOR_STP_1_0_0_0_1 = SNOR_STP(1, 0, 0, 0, 1),
    SNOR_STP_1_0_0_4_4 = SNOR_STP(1, 0, 0, 4, 4),
    SNOR_STP_1_1_0_0_0 = SNOR_STP(1, 1, 0, 0, 0),
    SNOR_STP_1_1_0_0_1 = SNOR_STP(1, 1, 0, 0, 1),
    SNOR_STP_1_1_0_0_4 = SNOR_STP(1, 1, 0, 0, 4),
    SNOR_STP_1_1_0_1_1 = SNOR_STP(1, 1, 0, 1, 1),
    SNOR_STP_1_1_0_1_2 = SNOR_STP(1, 1, 0, 1, 2),
    SNOR_STP_1_1_0_1_4 = SNOR_STP(1, 1, 0, 1, 4),
    SNOR_STP_1_2_2_0_2 = SNOR_STP(1, 2, 2, 0, 2),
    SNOR_STP_1_4_0_4_4 = SNOR_STP(1, 4, 0, 4, 4),
    SNOR_STP_1_4_4_4_4 = SNOR_STP(1, 4, 4, 4, 4),
    SNOR_STP_4_0_0_0_0 = SNOR_STP(4, 0, 0, 0, 0),
    SNOR_STP_4_0_0_0_4 = SNOR_STP(4, 0, 0, 0, 4),
    SNOR_STP_4_4_0_0_0 = SNOR_STP(4, 4, 0, 0, 0),
    SNOR_STP_4_4_0_0_4 = SNOR_STP(4, 4, 0, 0, 4),
    SNOR_STP_4_4_0_4_4 = SNOR_STP(4, 4, 0, 4, 4),
    SNOR_STP_4_4_4_4_4 = SNOR_STP(4, 4, 4, 4, 4),
};

static inline u8 snor_get_stp_cmd_bits(enum snor_stp v)
{
    return ((u64)(v & SNOR_STP_CMD_MASK)) >> SNOR_STP_CMD_SHIFT;
}

static inline u8 snor_get_stp_addr_bits(enum snor_stp v)
{
    return ((u64)(v & SNOR_STP_ADDR_MASK)) >> SNOR_STP_ADDR_SHIFT;
}

static inline u8 snor_get_stp_mode_bits(enum snor_stp v)
{
    return ((u64)(v & SNOR_STP_MODE_MASK)) >> SNOR_STP_MODE_SHIFT;
}

static inline u8 snor_get_stp_dummy_bits(enum snor_stp v)
{
    return ((u64)(v & SNOR_STP_DUMMY_MASK)) >> SNOR_STP_DUMMY_SHIFT;
}

static inline u8 snor_get_stp_data_bits(enum snor_stp v)
{
    return ((u64)(v & SNOR_STP_DATA_MASK)) >> SNOR_STP_DATA_SHIFT;
}

struct snor_dev;

struct snor_phy_ops {
    int (*read_reg)(struct snor_dev *snor, u8 cmd, u8 *buf, u32 from, u32 len);
    int (*write_reg)(struct snor_dev *snor, u8 cmd, u8 *buf, u32 len);
    int (*read)(struct snor_dev *snor, u32 from, u32 len, u8 *buf);
    int (*write)(struct snor_dev *snor, u32 to, u32 len, u8 *buf);
    int (*erase)(struct snor_dev *snor, u32 offs);
    void (*enter_read)(struct snor_dev *snor);
    u32 (*get_flash_base)(struct snor_dev *snor);
    void (*prepare)(struct snor_dev *snor);
    void (*post)(struct snor_dev *snor);
};

struct snor_erase_info {
    u8  cmd;
    u32 size;
};

struct snor_erase_cmd {
    u32 size;
    u32 size_mask;
    u8  cmd;
};

#define SNOR_ERASE_CMD_MAX 4
struct snor_erase_table {
    u32 erase_mask;
    struct snor_erase_cmd erase_cmd[SNOR_ERASE_CMD_MAX];
};

struct snor_dev_ops {
    void (*quad_enable)(struct snor_dev *snor);
    void (*qpi_enter)(struct snor_dev *snor);

    int (*lock)(struct snor_dev *snor, u32 ofs, u32 len);
    int (*unlock)(struct snor_dev *snor, u32 ofs, u32 len);
    boolean (*is_locked)(struct snor_dev *snor, u32 ofs, u32 len);
};

#define SNOR_QUAD_READ      BIT(1)  /* Flash supports Quad Read */
#define SNOR_QUAD_PP        BIT(2)  /* Flash supports Quad PP */
#define SNOR_HAS_QPI_MODE   BIT(3)  /* Flash supports QPI mode */
#define SNOR_HAS_DTR_MODE   BIT(4)  /* Flash supports DTR mode */
#define SNOR_HIGH_VOLTAGE   BIT(5)  /* Flash work on high voltage */

struct snor_dev_info {
    u16 did;

    u16 max_vol;
    u16 min_vol;

    u32 size;
    u16 min_erase_size;

    u16 page_size;
    u16 addr_width;

    u32 max_sclk;
    u32 flags;

    u8 dummy_clocks;
    u8 dummy_val;

    u32 lock_density;

    struct snor_erase_info *erase_info;
    u32 erase_num;

    struct snor_dev_ops *dev_ops;
};

#define READ_INFO(clocks, val) \
        .dummy_clocks = (clocks), \
        .dummy_val = (val)

struct snor_vendor {
    u8 mid;
    struct snor_dev_info* (*get_dev_info)(struct snor_dev *snor, u16 did);
};

#define SNOR_BUF_SIZE       (512)
struct snor_dev {
    struct fmd_dev fmd;

    u8 mid;
    struct snor_dev_info *info;

    u32 cs;

    /* read command paramters */
    u8 read_cmd;
    u8 read_dummy;
    enum snor_stp read_stp;

    /* write command paramters */
    u8  write_cmd;
    enum snor_stp write_stp;

    /* erase command paramters */
    u32 erase_size;
    u8  erase_cmd;
    enum snor_stp erase_stp;

    /* register related command paramters */
    enum snor_stp reg_stp;

    struct snor_erase_table erase_table;

    u8  work_mode;
#define SNOR_SPI_MODE  0
#define SNOR_QPI_MODE  1

    const struct snor_phy_ops *phy_ops;

    boolean init_done;
    u32 buf[SNOR_BUF_SIZE / 4];

    void *priv;
};

/* vendor devices. */
extern struct snor_vendor g_gigadevice;
extern struct snor_vendor g_winbond;
extern struct snor_vendor g_xmc;
extern struct snor_vendor g_puya;

void snor_read_sr1(struct snor_dev *snor, u8 *sr);
void snor_read_sr2(struct snor_dev *snor, u8 *sr);
void snor_write_sr1(struct snor_dev *snor, u8 sr1);
void snor_write_sr2(struct snor_dev *snor, u8 sr2);

static inline struct snor_dev *fmd_to_snor(struct fmd_dev *fmd)
{
    return fmd->priv;
}

#define FREERUN_CNT_L       (REG_BASE_AON_SYSTMR + 0x0)
#define FREERUN_CNT_H       (REG_BASE_AON_SYSTMR + 0x4)
#define TIMER_FREQ_32K      (30)

u64 snor_get_utime(void);
void snor_udelay(u32 usec);

int snor_probe(struct snor_dev *snor);
void snor_setup(struct snor_dev *snor);
void snor_sleep_resume(struct snor_dev *snor);

#define SNOR_INT_SAVE        AIC_INT_SAVE_AREA
#define SNOR_INT_DISABLE     AIC_INT_DISABLE
#define SNOR_INT_RESTORE     AIC_INT_RESTORE

#define SNOR_ASSERT          ASSERT
#define SNOR_ASSERT_INFO     ASSERT_INFO

#endif /* __SNOR_H__ */
