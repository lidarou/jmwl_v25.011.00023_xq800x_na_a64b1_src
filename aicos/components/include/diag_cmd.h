/******************************************************************************/
/*                                                                            */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                            */
/******************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file defines the diag cmds(not contain the subcmds define).
 *    The cmds range is [0, 255], CP diag cmds range is [0, 191]
 *    AP diag cmds range is [192, 211], [212, 255] is reserved.
 *
 */
#ifndef __DIAG_CMD_H__
#define __DIAG_CMD_H__
#include <aic_sys.h>

/*
 * The diag cmd define at here:
 * 1. cmd range is 0 ~0xff,
 * 2. new cmd add before DIAG_CMD_END and close to DIAG_CMD_END,
 *    DIAG_CMD_END must at the end.
 */
enum {
    DIAG_CMD_VERSION                = 0x00,
    DIAG_CMD_GXEM                   = 0x01,
    DIAG_CMD_AT_CMD                 = 0x02,
    DIAG_CMD_CONSOLE                = 0x03,
    DIAG_CMD_MISC                   = 0x04,
    DIAG_CMD_CALI_DATA              = 0x05,
    DIAG_CMD_NVRAM                  = 0x06,
    DIAG_CMD_FS                     = 0x07,
    DIAG_CMD_ASSERT                 = 0x08,
    DIAG_CMD_UID                    = 0x09,
    DIAG_CMD_POWEROFF               = 0x0A,
    DIAG_CMD_IMEI                   = 0x0B,
    DIAG_CMD_SN                     = 0x0C,
    DIAG_CMD_MAC                    = 0x0D,
    DIAG_CMD_WIFIMAC                = 0x0E,
    DIAG_CMD_BTMAC                  = 0x0F,
    DIAG_CMD_SIMLOCK                = 0x10,
    DIAG_CMD_DCXO_CALI              = 0X11,
    DIAG_CMD_LOG                    = 0X12,
    DIAG_CMD_AUDIO                  = 0x13,
    DIAG_CMD_MEMDFS                 = 0x14,
    DIAG_CMD_STATION                = 0x15,
    DIAG_CMD_CHG_OFF                = 0x16,
    DIAG_CMD_BAND                   = 0x17,
    DIAG_CMD_PMIC_ID                = 0x18,
    DIAG_CMD_PUBKEY_HASH            = 0x19,

    DIAG_CMD_LTE_CALI               = 0x30,
    DIAG_CMD_LTE_NST                = 0x31,
    DIAG_CMD_RFIC_REGISTER_RW       = 0x32,
    DIAG_CMD_MIPI_REGISTER_RW       = 0x33,
    DIAG_CMD_PWRUP_CALI             = 0x34,
    DIAG_CMD_TX_DC_CALI             = 0x35,
    DIAG_CMD_REG198                 = 0x36,

    DIAG_CMD_END
};

#define DIAG_CMD_CNT            DIAG_CMD_END

/*
 * The diag cmd errno define at here:
 */
enum {
    DIAG_RSP_SUCC = 0x00,
    DIAG_RSP_TAIL_CHECK_FAIL,
    DIAG_RSP_NO_FOUND_SYS,
    DIAG_RSP_NO_FOUND_CMD,
    DIAG_RSP_NO_FOUND_CMD_HANDLER,
    DIAG_RSP_NO_FOUND_SUBCMD,
    DIAG_RSP_DATA_ERR,

    /* use define err base 0x80. */
    DIAG_RSP_USE_ERR = 0x80,
};

/*
 * The specail sn define :
 * The specail sn range [0x00, 0xff], It is the special sn that produce by UE,
 * The normal sn is produce by pc tool, base from 0x100.
 */
enum {
    DIAG_SPEC_SN_CONSOLE = 0x00,
    DIAG_SPEC_SN_AT = 0x01,
    DIAG_SPEC_SN_ASSERT = 0x02,
};

/*
 * Add some sub cmd define here.
 */
#define SUB_CMD_GET_VERSION         0x00
#define SUB_CMD_GET_MSG_VERSION     0x01

#endif /* __DIAG_CMD_H__ */
