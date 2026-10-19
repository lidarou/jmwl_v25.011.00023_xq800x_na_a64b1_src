/*****************************************************************************/
/*                                                                           */
/*   Copyright 2022 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.    */
/*                                                                           */
/*****************************************************************************/

/**
 *  DESCRIPTION
 *
 *    This file is download command operate header file, Use for external supply cmd interface and
 *    macro definition related to download command.
 */

#ifndef __DLOAD_CMD_H__
#define __DLOAD_CMD_H__

#define HDLC_FLAG 0x7E
#define HDLC_ESCAPE 0x7D
#define HDLC_ESCAPE_MASK 0x20

#define PKT_FLAG_SIZE 1
#define PKT_CRC_SIZE  2
#define PKT_TYPE_SIZE 1
#define PKT_LEN_SIZE  2

#define PKT_HEADER_SIZE sizeof(pkt_header)
#define DATA_ADDR PKT_HEADER_SIZE

#ifndef CONFIG_HDLC_MAX_FRAME_LENGTH
#define HDLC_MAX_FRAME_LENGTH (1024 + 32)
#else
#define HDLC_MAX_FRAME_LENGTH (CONFIG_HDLC_MAX_FRAME_LENGTH + 32)
#endif

#define PARTITION_SIZE_LENGTH           4
#define MAX_PARTITION_NAME_SIZE         36
#define GAP_SIZE_LENGTH                 8
#define PARTITION_SIZE_LENGTH_V1        8
#define REPARTITION_HEADER_MAGIC        0x3A726170

#define BIT64_IDLEN_DATA_LENGTH         0x58      /* ID(72)+ LEN(8) +Rev(8) [+CS] */
#define BIT32_IDLEN_DATA_LENGTH         0x4C      /* ID(72)+ LEN(4) [+CS] */
#define BIT64_ADDRLEN_DATA_LENGTH       0x10      /* ADDR(8)+ LEN(8) [+CS] */
#define BIT32_ADDRLEN_DATA_LENGTH       0x08      /* ADDR(4)+ LEN(4) [+CS] */
#define BIT64_READ_MIDST_LENGTH         0x0C

#define REPARTITION_UNIT_LENGTH         (MAX_PARTITION_NAME_SIZE *2 + PARTITION_SIZE_LENGTH)
#define REPARTITION_UNIT_LENGTH_V1      (MAX_PARTITION_NAME_SIZE *2 + PARTITION_SIZE_LENGTH_V1 + GAP_SIZE_LENGTH)
#define REF_INFO_OFF                    0XFA000


/*HDLC*/
typedef enum _PACKET_TYPE {
    REP_PACKET_TYPE_0 = 0,
    REP_PACKET_TYPE_1,
    REP_PACKET_TYPE_2,
    REP_PACKET_TYPE_3,
    REP_PACKET_TYPE_4,
    REP_PACKET_TYPE_5,
} PACKET_TYPE;

/**---------------------------------------------------------------------------*
 ** The Follow defines the packet processed result table                      *
 ** packet_protocol:                                                          *
 **   FLAG     PKT_TYPE      DATALENGHT    [DATA]    CRC          FLAG        *
 **   0x7E      MAX:255      MAX:65536      ...      ...          0x7E        *
 **    1           2             2           0        2             1         *
 ** response_packet:                                                          *
 **   0x7E      MAX:255          0           --      ...          0x7E        *
 **    1           2             2           --       2             1         *
 ** response packet length: 8 bytes                                           *
 **---------------------------------------------------------------------------*/
typedef enum _CMD_TYPE {
    /* msg packet*/
    /* Start cmd from pc */
    PCK_CMD_TYPE_MIN = 0,
    /* download protocol*/
    PCK_CMD_CONNECT = 0,                /* 0x0 */
    PCK_CMD_START_DATA,                 /* 0x1 */
    PCK_CMD_MIDST_DATA,                 /* 0x2 */
    PCK_CMD_END_DATA,                   /* 0x3 */
    PCK_CMD_EXEC_DATA,                  /* 0x4 */
    /* reset device */
    PCK_CMD_NORMAL_RESET,               /* 0x5 */
    PCK_CMD_WRITE_SN,                   /* 0x6*/
    PCK_CMD_CHANGE_BAUD = 0x8,          /* 0x8*/
    /* erase flash */
    PCK_CMD_ERASE_FLASH = 0x9,          /* 0x9 */
    PCK_CMD_ERASE_PARTITION = 0xA,      /* 0xA */
    /* read flash */
    PCK_CMD_READ_NOR_FLASH_START= 0xB,  /* 0xB */
    PCK_CMD_READ_NOR_FLASH_MIDST,       /* 0xC */
    PCK_CMD_READ_NOR_FLASH_END,         /* 0xD */
    PCK_CMD_UPDATE_PTABLE,              /* 0xE*/
    PCK_CMD_CHECK_PTABLE,               /* 0xF*/
    /* reset device*/
    PCK_CMD_SYS_RESET = 0x10,           /* 0x10 */
    /* reset device*/
    PCK_CMD_CHG_PD,                     /* 0x11 */
    PCK_CMD_GET_UID,                    /* 0x12 */
    PCK_CMD_SYS_PD,                     /* 0X13 */
    /* backup read partiton */
    PCK_CMD_BACKUP_READ_START= 0x20,    /* 0x20 */
    PCK_CMD_BACKUP_READ_MIDST,          /* 0x21 */
    PCK_CMD_BACKUP_READ_END,            /* 0x22 */
    /* fs operation */
    PCK_CMD_FS_WRITE_START= 0x30,       /* 0x30 */
    PCK_CMD_FS_WRITE_MIDST,             /* 0x31 */
    PCK_CMD_FS_WRITE_END,               /* 0x32 */
    PCK_CMD_FS_READ_START,              /* 0x33 */
    PCK_CMD_FS_READ_MIDST,              /* 0x34 */
    PCK_CMD_FS_READ_END,                /* 0x35 */
    /* ssp info record */
    PCK_CMD_SSP_INFO_RECORD,            /* 0x36 */

    PCK_CMD_TYPE_MAX,
    /* Start ack from device*/
    /* success  ack */
    PCK_REP_ACK = 0x80,                 /* 0x80 */
    PCK_REP_VER,                        /* 0x81 */
    PCK_REP_READ_NOR_FLASH,             /* 0x82 */
    PCK_REP_GET_UID,                    /* 0x83 */
    PCK_REP_READ_FS,                    /* 0x84 */
    /* fail ack */
    PCK_REP_INVALID_CMD = 0x85,         /* 0x85 */
    PCK_REP_OPERATION_FAILED,           /* 0x86 */
    PCK_REP_WAIT_INPUT_TIMEOUT,         /* 0x87 */
    PCK_REP_REPEAT_CONTINUE,            /* 0x88 */
    PCK_REP_INCOMPATIBLE_PARTITION,     /* 0x89 */
    PCK_REP_EEROR_CRC,                  /* 0x8a */
    PCK_REP_EEROR_CHECKSUM,             /* 0x8b */
    /* Start message from device */
    /* success message */
    PCK_PHONE_SUCCEED = 0xa0,           /* 0xa0 */
    /* fail message */
    PCK_PHONE_INVALID_FLASH,            /* 0xa1 */
    PCK_PHONE_NOT_ENOUGH_MEMORY,        /* 0xa2 */
    PCK_PHONE_FLASH_DENSITY_NOT_ENOUGH, /* 0xa3 */
    PCK_PHONE_PSRAM_DENSITY_NOT_ENOUGH, /* 0xa4 */
    /* ptable error */
    PCK_PTABLE_EMPTY = 0xb0,            /* 0xb0 */
    PCK_PTABLE_NOT_MATCH,               /* 0xb1 */
    PCK_PTABLE_CORRUPTION_ERR,          /* 0xb2 */
    PCK_PTABLE_HEADER_CRC_ERR,          /* 0xb3 */
    PCK_PTABLE_CRC_ERR,                 /* 0xb4 */
    PCK_PTABLE_PARTITION_NOT_FIND_ERR,  /* 0xb5 */
    PCK_PTABLE_SECTOR_SIZE_NOT_MATCH,   /* 0xb6 */
    PCK_PKT_TYPE_MAX
} CMD_TYPE;

typedef struct pkt_header_tag {
    unsigned short type;
    unsigned short length;
} pkt_header, *pkt_header_ptr;

typedef struct msg_header_tag {
    CMD_TYPE msg;
    char info[80];
} msg_header, *msg_header_ptr;

typedef struct packet_body
{
    unsigned short type;
    unsigned short size;
    unsigned char content[HDLC_MAX_FRAME_LENGTH];
}packet_body;

typedef struct _DLOAD_CONDITION {
    unsigned int start_address;
    unsigned long long total_size;
    unsigned long long recv_size;
    unsigned int next_address;
    unsigned char ptn_name[36];
    packet_body *p_read_ptn;
} DLOAD_CONDITION, *PDLOAD_CONDITION;

void dl_cmd_operate(CMD_TYPE cmd,packet_body *packet);

#endif /*__DLOAD_CMD_H__*/
