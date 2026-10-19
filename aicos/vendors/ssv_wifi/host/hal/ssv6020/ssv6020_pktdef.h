#ifndef _ssv6020_PKTDEF_H_
#define _ssv6020_PKTDEF_H_


#include <ssv_types.h>
//#include "ssv6020_hal.h"

#define u8 ssv_type_u8
#define u16 ssv_type_u16
#define u32 ssv_type_u32
#define u64 ssv_type_u64
#define s8 ssv_type_s8
#define s16 ssv_type_s16
#define s32 ssv_type_s32
#define s64 ssv_type_s64
#ifndef bool
#define bool ssv_type_bool
#endif
#define uint8_t ssv_type_u8
#define uint16_t ssv_type_u16
#define uint32_t ssv_type_u32
#define uint64_t ssv_type_u64
#define int8_t ssv_type_s8
#define int16_t ssv_type_s16
#define int32_t ssv_type_s32
#define int64_t ssv_type_s64

#include <ssv_desc.h>

#define PKT_TxInfo struct ssv6020_tx_desc
#define PKT_RxInfo struct ssv6020_rx_desc


typedef enum __PBuf_Type_E {
    NOTYPE_BUF  = 0,
    TX_BUF      = 1,
    RX_BUF      = 2
} PBuf_Type_E;

/**
 *
 *  Offset Table (register):
 *
 *    c_type            hdr_len
 *  ----------     ----------
 *  M0_TXREQ         8-bytes
 *  M1_TXREQ         12-bytes
 *  M2_TXREQ         sizeof(PKT_TXInfo)
 *  M0_RXEVENT
 *  M1_RXEVENT
 *  HOST_CMD
 *  HOSt_EVENT
 *
 *
 */
#define M0_TXREQ                            0
#define M1_TXREQ                            1
#define M2_TXREQ                            2
#define M0_RXEVENT                          3
#define M2_RXEVENT                          4
#define HOST_CMD                            5
#define HOST_EVENT                          6
#define RATE_RPT                            7


#define IS_RX_PKT(_p)       (((_p)->c_type==M0_RXEVENT) || ((_p)->c_type==M2_RXEVENT))
#define IS_TX_PKT(_p)       (((_p)->c_type<=M2_TXREQ))
#define IS_TX_TEST_CMD(_p)	((_p)->c_type==TEST_CMD)

/*	WMM_Specification_1-1 : Table 14  802.1D Priority to AC mappings

	UP		Access Category
	-------------------------
	1, 2	AC_BK
	0, 3	AC_BE
	4, 5	AC_VI
	6, 7	AC_VO
*/
#define	AC_BK	 0
#define	AC_BE	 1
#define	AC_VI	 2
#define	AC_VO	 3


#define	PBUF_HDR80211(p, i)				(*((ssv_type_u8 *)(p)+(p)->hdr_offset + (i)))

#define	GET_SC_SEQNUM(sc)				(((sc) & 0xfff0) >> 4)
#define	GET_SC_FRAGNUM(sc)				(((sc) & 0x000f)     )

#define GET_QC_TID(qc)					((qc) & 0x000f)
#define	GET_QC_UP(qc)					((qc) & 0x0007)
#define GET_QC_AC(qc) 					((GET_QC_UP(qc) == 0) ? AC_BE : \
										 (GET_QC_UP(qc) == 1) ? AC_BK : \
										 (GET_QC_UP(qc) == 2) ? AC_BK : \
										 (GET_QC_UP(qc) == 3) ? AC_BE : \
										 (GET_QC_UP(qc) == 4) ? AC_VI : \
										 (GET_QC_UP(qc) == 5) ? AC_VI : \
										 (GET_QC_UP(qc) == 6) ? AC_VO : AC_VO)


#define GET_HDR80211_FC(p)				(((p)->f80211==1) ? (((ssv_type_u16)PBUF_HDR80211(p, 1) << 8) | PBUF_HDR80211(p, 0)) : 0)
#define GET_HDR80211_FC_TYPE(p)			((GET_HDR80211_FC(p) & 0x0c) >> 2)
#define GET_HDR80211_FC_TYPE_STR(t)		((t == 0) ? "Mgmt" : ((t == 1) ? "Control" : ((t == 2) ? "Data" : "Reserved")))
#define	GET_HDR80211_FC_VER(p)			((GET_HDR80211_FC(p) & M_FC_VER))
#define GET_HDR80211_FC_TODS(p)			((GET_HDR80211_FC(p) & M_FC_TODS)      >>  8)
#define GET_HDR80211_FC_FROMDS(p)		((GET_HDR80211_FC(p) & M_FC_FROMDS)    >>  9)
#define GET_HDR80211_FC_MOREFRAG(p)		((GET_HDR80211_FC(p) & M_FC_MOREFRAGS) >> 10)
#define	GET_HDR80211_FC_RETRY(p)	    ((GET_HDR80211_FC(p) & M_FC_RETRY)     >> 11)
#define	GET_HDR80211_FC_PWRMGNT(p)		((GET_HDR80211_FC(p) & M_FC_PWRMGMT)   >> 12)
#define	GET_HDR80211_FC_MOREDATA(p)		((GET_HDR80211_FC(p) & M_FC_MOREDATA)  >> 13)
#define	GET_HDR80211_FC_PROTECTED(p)	((GET_HDR80211_FC(p) & M_FC_PROTECTED) >> 14)
#define	GET_HDR80211_FC_ORDER(p)		((GET_HDR80211_FC(p) & M_FC_ORDER)     >> 15)

#define SET_HDR80211_FC_MOREFRAG(p)		(PBUF_HDR80211(p, 1) |= 0x04)
#define UNSET_HDR80211_FC_MOREFRAG(p)	(PBUF_HDR80211(p, 1) &= 0xfb)

#define GET_HDR80211_SC(p)				((ssv_type_u16)PBUF_HDR80211(p, 23) << 8 | (PBUF_HDR80211(p, 22)))
#define GET_HDR80211_SC_SEQNUM(p)		((GET_HDR80211_SC(p) & 0xfff0) >> 4)
#define GET_HDR80211_SC_FRAGNUM(p)		((GET_HDR80211_SC(p) & 0x000f))

//
//	Function			ToDS	FromDS	Addr1	Addr2	Addr3	Addr4
//	-------------------------------------------------------------------------
//	IBSS				0		0		DA		SA		BSSID	Not_Used
//	To AP (infra.)		1		0		BSSID	SA		DA		Not_Used
//	From AP (infra.)	0		1		DA		BSSID	SA		Not_Used
//	WDS (bridge)		1		1		RA		TA		DA		SA
#define HAS_HDR80211_ADDRESS_4(p)		(GET_HDR80211_FC_TODS(p) & GET_HDR80211_FC_FROMDS(p))

// QoS Control Field
#define	GET_HDR80211_QC(p)				(((p)->qos == 1) ? (((ssv_type_u16)PBUF_HDR80211(p, 25 + (HAS_HDR80211_ADDRESS_4(p)*6)) << 8) | PBUF_HDR80211(p, 24 + (HAS_HDR80211_ADDRESS_4(p)*6))) : 0)
#define GET_HDR80211_ADDRESS_1(a, p)	OS_MemCPY((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset +  4), 6)
#define GET_HDR80211_ADDRESS_2(a, p)	OS_MemCPY((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset + 10), 6)
#define GET_HDR80211_ADDRESS_3(a, p)	OS_MemCPY((a), ((ssv_type_u8 *)(p)+(p)->hdr_offset + 16), 6)


/* 
 * struct hci_tx_aggr_info - HCI TX Aggregation Format description
 */
struct hci_tx_aggr_info {
    ssv_type_u32 jmp_len0:16;
    ssv_type_u32 jmp_len1:16;
    ssv_type_u32 extra_info;
    
#ifdef NOT_SUPPORT_ZERO_ARRAY
#else
     ssv_type_u8      dat8[0];
#endif
};


#define SSV62XX_TX_MAX_RATES    3

#define ssv6020_HOST_TXREQ0         PKT_TxInfo
#define ssv6020_HOST_RXPKT          PKT_RxInfo
#define PKT_INFO_DATA_TYPE          PKT_Info
#define PKT_TXINFO_DATA_TYPE        PKT_TxInfo
#define PKT_RXINFO_DATA_TYPE        PKT_RxInfo

/**
 * Define constants for do_rts_cts field of PKT_TxInfo structure
 *
 * @ TX_NO_RTS_CTS
 * @ TX_RTS_CTS
 * @ TX_CTS
 */
#define TX_NO_RTS_CTS                   0
#define TX_RTS_CTS                      1
#define TX_CTS                          2

#ifdef THROUGHPUT_TEST

typedef struct SDIO_THROUGHPUT_COMMAND_st
{
	ssv_type_u32		transferCount;
	ssv_type_u16		transferLength;
	ssv_type_u16		resverd;
} SDIO_THROUGHPUT_COMMAND;


typedef struct THROUGHPUT_COMMAND_st
{
    ssv_type_u8		mode;
	ssv_type_u8		rateIndex;//0-38
	ssv_type_u8      noack;
	ssv_type_u16		transferLength;
	ssv_type_u16      qos;
	ssv_type_u32		transferCount;
} THROUGHPUT_COMMAND;

#endif

enum fcmd_seek_type {
    FCMD_SEEK_PREV  = 0,
    FCMD_SEEK_CUR,
    FCMD_SEEK_NEXT
};

#define M0_HDR_LEN							4
#define M1_HDR_LEN							8
#define M2_HDR_LEN							16

#define RX_M0_HDR_LEN						ssv6020_RXINFO_SIZE

#endif	/* _ssv6020_PKTDEF_H_ */

