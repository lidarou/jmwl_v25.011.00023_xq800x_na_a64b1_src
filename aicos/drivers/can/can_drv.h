/*****************************************************************************/
/*                                                                           */
/*    Copyright 2023 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.   */
/*                                                                           */
/*****************************************************************************/

#ifndef __CAN_DRV_H__
#define __CAN_DRV_H__

#include <aic_type.h>

/*
 * Controller Area Network Identifier structure
 *
 * bit 0-28 : CAN identifier (11/29 bit)
 * bit 29   : error message frame flag (0 = data frame, 1 = error message)
 * bit 30   : remote transmission request flag (1 = rtr frame)
 * bit 31   : frame format flag (0 = standard 11 bit, 1 = extended 29 bit)
 */
typedef u32 canid_t;

#define CAN_SFF_ID_BITS 11
#define CAN_EFF_ID_BITS 29

/* CAN payload length and DLC definitions according to ISO 11898-1 */
#define CAN_MAX_DLC 8
#define CAN_MAX_RAW_DLC 15
#define CAN_MAX_DLEN 8

/* CAN FD payload length and DLC definitions according to ISO 11898-7 */
#define CANFD_MAX_DLC 15
#define CANFD_MAX_DLEN 64

#define CAN_CTRLMODE_LOOPBACK       0x01    /* Loopback mode */
#define CAN_CTRLMODE_LISTENONLY     0x02    /* Listen-only mode */
#define CAN_CTRLMODE_3_SAMPLES      0x04    /* Triple sampling mode */
#define CAN_CTRLMODE_ONE_SHOT       0x08    /* One-Shot mode */
#define CAN_CTRLMODE_BERR_REPORTING 0x10    /* Bus-error reporting */
#define CAN_CTRLMODE_FD             0x20    /* CAN FD mode */
#define CAN_CTRLMODE_PRESUME_ACK    0x40    /* Ignore missing CAN ACKs */
#define CAN_CTRLMODE_FD_NON_ISO     0x80    /* CAN FD in non-ISO mode */
#define CAN_CTRLMODE_CC_LEN8_DLC    0x100   /* Classic CAN DLC option */
#define CAN_CTRLMODE_TDC_AUTO       0x200   /* CAN transiver automatically calculates TDCV */
#define CAN_CTRLMODE_TDC_MANUAL     0x400   /* TDCV is manually set up by user */

typedef enum can_workmode_st {
    CAN_WM_LOOPBACK_I,
    CAN_WM_LOOPBACK_E,
    CAN_WM_NORMAL,
    CAN_WM_MAX
} can_workmode;

typedef struct can_bittiming_st {
    u8 s_seg_1;    /* offset: 0xa8 */
    u8 s_seg_2;    /* offset: 0xa9 */
    u8 s_sjw;      /* offset: 0xaa */
    u8 s_presc;    /* offset: 0xab */
    u32 baud;      /* baudrate */
} can_bittiming;

#if 0
/*
 * CAN bit-timing parameters
 *
 * For further information, please read chapter "8 BIT TIMING
 * REQUIREMENTS" of the "Bosch CAN Specification version 2.0"
 * at http://www.semiconductors.bosch.de/pdf/can2spec.pdf.
 */
struct can_bittiming {
    u32 bitrate;      /* Bit-rate in bits/second */
    u32 sample_point; /* Sample point in one-tenth of a percent */
    u32 tq;           /* Time quanta (TQ) in nanoseconds */
    u32 prop_seg;     /* Propagation segment in TQs */
    u32 phase_seg1;   /* Phase buffer segment 1 in TQs */
    u32 phase_seg2;   /* Phase buffer segment 2 in TQs */
    u32 sjw;          /* Synchronisation jump width in TQs */
    u32 brp;          /* Bit-rate prescaler */
};

/*
 * CAN hardware-dependent bit-timing constant
 *
 * Used for calculating and checking bit-timing parameters
 */
struct can_bittiming_const_st {
    char name[16];      /* Name of the CAN controller hardware */
    u32 tseg1_min;    /* Time segment 1 = prop_seg + phase_seg1 */
    u32 tseg1_max;
    u32 tseg2_min;    /* Time segment 2 = phase_seg2 */
    u32 tseg2_max;
    u32 sjw_max;      /* Synchronisation jump width */
    u32 brp_min;      /* Bit-rate prescaler */
    u32 brp_max;
    u32 brp_inc;
};
#endif

/**
 * struct can_frame - Classical CAN frame structure (aka CAN 2.0B)
 * @can_id:   CAN ID of the frame and CAN_*_FLAG flags, see canid_t definition
 * @len:      CAN frame payload length in byte (0 .. 8)
 * @can_dlc:  deprecated name for CAN frame payload length in byte (0 .. 8)
 * @__pad:    padding
 * @__res0:   reserved / padding
 * @len8_dlc: optional DLC value (9 .. 15) at 8 byte payload length
 *            len8_dlc contains values from 9 .. 15 when the payload length is
 *            8 bytes but the DLC value (see ISO 11898-1) is greater then 8.
 *            CAN_CTRLMODE_CC_LEN8_DLC flag has to be enabled in CAN driver.
 * @data:     CAN frame payload (up to 8 byte)
 */
typedef struct can_frame_st {
    u32 can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    u32 len;
    u32 len8_dlc; /* optional DLC for 8 byte payload length (9 .. 15) */
    u32 data[CAN_MAX_DLEN/4];
} can_frame;

#if 0
struct can_frame {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    union {
        /* CAN frame payload length in byte (0 .. CAN_MAX_DLEN)
         * was previously named can_dlc so we need to carry that
         * name for legacy support
         */
        u8 len;
        u8 can_dlc; /* deprecated */
    } __attribute__((packed)); /* disable padding added in some ABIs */
    u8 __pad; /* padding */
    u8 __res0; /* reserved / padding */
    u8 len8_dlc; /* optional DLC for 8 byte payload length (9 .. 15) */
    u8 data[CAN_MAX_DLEN] __attribute__((aligned(8)));
};
#endif
/*
 * defined bits for canfd_frame.flags
 *
 * The use of struct canfd_frame implies the FD Frame (FDF) bit to
 * be set in the CAN frame bitstream on the wire. The FDF bit switch turns
 * the CAN controllers bitstream processor into the CAN FD mode which creates
 * two new options within the CAN FD frame specification:
 *
 * Bit Rate Switch - to indicate a second bitrate is/was used for the payload
 * Error State Indicator - represents the error state of the transmitting node
 *
 * As the CANFD_ESI bit is internally generated by the transmitting CAN
 * controller only the CANFD_BRS bit is relevant for real CAN controllers when
 * building a CAN FD frame for transmission. Setting the CANFD_ESI bit can make
 * sense for virtual CAN interfaces to test applications with echoed frames.
 *
 * The struct can_frame and struct canfd_frame intentionally share the same
 * layout to be able to write CAN frame content into a CAN FD frame structure.
 * When this is done the former differentiation via CAN_MTU / CANFD_MTU gets
 * lost. CANFD_FDF allows programmers to mark CAN FD frames in the case of
 * using struct canfd_frame for mixed CAN / CAN FD content (dual use).
 * N.B. the Kernel APIs do NOT provide mixed CAN / CAN FD content inside of
 * struct canfd_frame therefore the CANFD_FDF flag is disregarded by Linux.
 */
#define CANFD_BRS 0x01 /* bit rate switch (second bitrate for payload data) */
#define CANFD_ESI 0x02 /* error state indicator of the transmitting node */
#define CANFD_FDF 0x04 /* mark CAN FD for dual use of struct canfd_frame */

/**
 * struct canfd_frame - CAN flexible data rate frame structure
 * @can_id: CAN ID of the frame and CAN_*_FLAG flags, see canid_t definition
 * @len:    frame payload length in byte (0 .. CANFD_MAX_DLEN)
 * @flags:  additional flags for CAN FD
 * @__res0: reserved / padding
 * @__res1: reserved / padding
 * @data:   CAN FD frame payload (up to CANFD_MAX_DLEN byte)
 */
typedef struct canfd_frame_st {
    canid_t can_id;  /* 32 bit CAN_ID + EFF/RTR/ERR flags */
    u8    len;     /* frame payload length in byte */
    u8    flags;   /* additional flags for CAN FD */
    u8    __res0;  /* reserved / padding */
    u8    __res1;  /* reserved / padding */
    u8    data[CANFD_MAX_DLEN] __attribute__((aligned(8)));
} canfd_frame;

typedef struct can_dev_st {
    u32 reg_base;
    unsigned int tx_mode;
    aos_event_ptr can_event;
#if 0
    unsigned int tbsel; /*tbsel=0 choose PTB,tbsel=1 choose STB*/
    struct can_bittiming_const_st *bittiming_const, *data_bittiming_const;
    struct can_bittiming bittiming, data_bittiming;
#endif
    u8 can_or_canfd;
} can_dev;

typedef enum can_baud_st {
    CAN_BAUD_1M_0        = 0x00,
    CAN_BAUD_1M_1,
    CAN_BAUD_500K_0,
    CAN_BAUD_500K_1,
    CAN_BAUD_250K_0,
    CAN_BAUD_250K_1,
    CAN_BAUD_125K_0,
    CAN_BAUD_125K_1,
} can_baud;

/**
 * @brief CAN acceptance filter modes
 */
typedef enum _can_filter_mode {
    can_filter_id_mode_both_frames,                /**< Accept both standard frame and extended frame */
    can_filter_id_mode_standard_frames,            /**< Accept standard frame only */
    can_filter_id_mode_extended_frames,            /**< Accept extended frame only */
} can_filter_mode;

/**
 * @brief CAN acceptance filter configuration
 */
typedef struct {
    u16 index;                            /**< CAN filter index */
    can_filter_mode id_mode;              /**< CAN filter id mode */
    BOOL enable;                          /**< CAN filter enable flag */
    u32 code;                             /**< CAN filter code */
    u32 mask;                             /**< CAN filter mask */
} can_filter_cfg;


/*transmit mode*/
#define XMIT_FULL                   0
#define XMIT_SEP_FIFO               1
#define XMIT_SEP_PRIO               2
#define XMIT_PTB_MODE               3
#define XMIT_STB_TSONE_MODE         4
#define XMIT_STB_TSALL_MODE         5

int can_init(void);
void can_set_bit_timing(can_dev *pdev, u32 baudrate);
int can_set_power(boolean b_on);
int can_recv(can_dev *pdev, can_frame *cf);
int can_send(can_dev *pdev, can_frame *cf);
int can_send_high_priority(can_dev *pdev, can_frame *cf);
int can_set_filter(can_dev *pdev, can_filter_cfg *config);

#endif // __CAN_DRV_H__
