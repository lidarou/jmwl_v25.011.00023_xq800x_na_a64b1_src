/**************************************************************************/
/*                                                                        */
/*    Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.*/
/*                                                                        */
/**************************************************************************/

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x0a16dd
#define LOG_MODULE    AMOD_ALOG

#include <aic_bitmap.h>
#include <aic_type.h>
#include <aic_errno.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_osa.h>
#include <aic_network_pcap.h>
#include <stdlib.h>
#include <aipc_api.h>
#include "../inc/alog_data_format.h"

#include "timer_inc.h"

#ifdef WIN32
#include "stdio.h"
#endif

#ifdef CONFIG_TCPIP_LWIP
#include <lwip/pbuf.h>
#endif

#define ETHERNET_LINK  1
#define PPP_LINK       9
#define RAW_IP_LINK    101
#define PACKET_MAX_LEN 1500
#define AIC_CURRENT_YEAR     1970

/* Define the number of seconds from 01-01-1970 00:00:00 to
   01-01-2022 00:00:00 */
#define AIC_FIXED_YEAR     2022
#define AIC_LOSS_OF_UTC_TIME 0x61CF9980

#ifdef CONFIG_SUBSYS_AP
static u8 apcap_dump_flag = 0;
#endif
#define PCAP_DUMP_MAX_SIZE  80
#define PCAP_START_UTC_TIME 0x61ebe1e1
static u32 pcap_dump_size = 0;
#define AIC_GET_PCAP_DUMP_SIZE() pcap_dump_size ? pcap_dump_size : PCAP_DUMP_MAX_SIZE;

struct time_spec {
    /* Seconds */
    u32 tv_sec;
    /* Nanoseconds */
    u64 tv_nsec;
    u32 tv_sec_offset;
};

struct pcap_hdr {
    /* magic number */
    u32 magic_number;
    /* major version number */
    u16 version_major;
    /* minor version number */
    u16 version_minor;
    /* GMT to local correction */
    int thiszone;
    /* accuracy of timestamps */
    u32 sigfigs;
    /* max length of captured packets, in octets */
    u32 snaplen;
    /* data link type */
    u32 network;
};

struct pcap_packet_header {
    /* timestamp seconds */
    u32 ts_sec;
    /* timestamp microseconds */
    u32 ts_usec;
    /* number of octets of packet saved in file */
    u32 incl_len;
    /* actual length of packet */
    u32 orig_len;
};

#ifndef WIN32
static ABRR_HANDLE pcap_brr;
static int pcap_init_flag = 0;
static u8 cptcpdump = AIC_FALSE;
#endif
static struct time_spec aic_pcap_tv;
#ifdef WIN32
FILE *fp = NULL;
#endif

ulong aic_convert_date_to_seconds(aos_systime *local_time);

#ifndef WIN32
#ifdef CONFIG_SUBSYS_AP

void aic_network_enable_pdp_nic_capdump()
{
    apcap_dump_flag |= AIC_PDP_NIC_FLAG;

    if (!pcap_brr) {
        pcap_brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DEBUG);
        if (!pcap_brr) {
            aicx_error(0x068ebe, "", "[PCAP] request brr fail\r\n");
        }
    }
}

void aic_network_enable_usb_nic_capdump()
{
    apcap_dump_flag |= AIC_USB_NIC_FLAG;

    if (!pcap_brr) {
        pcap_brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DEBUG);
        if (!pcap_brr) {
            aicx_error(0x068ebe, "", "[PCAP] request brr fail\r\n");
        }
    }
}

void aic_network_enable_ppp_nic_capdump()
{
    apcap_dump_flag |= AIC_PPP_NIC_FLAG;

    if (!pcap_brr) {
        pcap_brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DEBUG);
        if (!pcap_brr) {
            aicx_error(0x068ebe, "", "[PCAP] request brr fail\r\n");
        }
    }
}

void aic_network_disable_pdp_nic_capdump()
{
    apcap_dump_flag &= ~AIC_PDP_NIC_FLAG;
}

void aic_network_disable_usb_nic_capdump()
{
    apcap_dump_flag &= ~AIC_USB_NIC_FLAG;
}

void aic_network_disable_ppp_nic_capdump()
{
    apcap_dump_flag &= ~AIC_PPP_NIC_FLAG;
}

void aic_network_config_pcap_size(int len)
{
    if (len > PACKET_MAX_LEN)
        pcap_dump_size = PACKET_MAX_LEN;
    else
        pcap_dump_size = len;
}

int aic_network_pcap_time_init(void)
{
    aic_pcap_tv.tv_sec = PCAP_START_UTC_TIME;
    aic_pcap_tv.tv_nsec = 0;

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_network_pcap_time_init);

int aic_network_data_dump(u8 *packet_ptr, u32 packet_len)
{
    int ret;
    ABLK_T blk;
    u32 cap_size, log_header_size;
    AOS_INTERRUPT_DECLARE

    if (!packet_len)
        return AIC_EINVAL;

    if (!pcap_brr) {
        aicx_error(0x02ec80, "", "[PCAP] pcap_brr is null\n");
        return AIC_EMFILE;
    }

    cap_size = packet_len;

    log_header_size = alog_data_get_header_size(ALOG_DATA_CAP);

    ret = ablk_get_block(pcap_brr, log_header_size + cap_size,
                         &blk, 0);
    if (ret) {
        aicx_info(0x02ccae, "d", "[PCAP] log lost, get blk ret=%d.\n", ret);
        return AIC_ENOMEM;
    }

    memcpy((u8*)blk.addr + log_header_size, (u8 *)packet_ptr, cap_size);
    AOS_INTERRUPT_DISABLE
    alog_data_format(ALOG_DATA_CAP, INVALID_DATA_LINK, blk.addr,
                     log_header_size + cap_size, TRUE);

    blk.size = log_header_size + cap_size;
    ret = ablk_send_block(pcap_brr, ALDATA_LINK, &blk, 0);
    if (ret) {
        AOS_INTERRUPT_RESTORE
        aicx_info(0x070b93, "d", "[PCAP] log lost, send blk ret = %d.\n", ret);
        ablk_recycle_block(pcap_brr, &blk);
        return AIC_ENOBUFS;
    }

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

int aic_network_pcap_dump_do(u8 *packet_ptr, u32 packet_len, u8 type)
{
    int ret;
    ABLK_T blk;
    u32 cap_size, log_header_size, size;
    aos_systime time;
    AOS_INTERRUPT_DECLARE
    struct pcap_packet_header packet_header;

    if (!(apcap_dump_flag & type))
        return AIC_ENXIO;

    if (!packet_len)
        return AIC_EINVAL;

    if (!pcap_brr) {
        aicx_error(0x02ec80, "", "[PCAP] pcap_brr is null\n");
        return AIC_EMFILE;
    }

    size = AIC_GET_PCAP_DUMP_SIZE();
    if (packet_len > size)
        cap_size = size;
    else
        cap_size = packet_len;

    log_header_size = alog_data_get_header_size(ALOG_DATA_CAP);

    ret = ablk_get_block(pcap_brr,
                         log_header_size + cap_size + sizeof(packet_header),
                         &blk, 0);
    if (ret) {
        aicx_info(0x02ccae, "d", "[PCAP] log lost, get blk ret=%d.\n", ret);
        return AIC_ENOMEM;
    }

#if 0
    packet_header.ts_sec = (aic_pcap_tv.tv_nsec + aic_get_timestamp()) / 1000000;
    packet_header.ts_usec = (aic_pcap_tv.tv_nsec + aic_get_timestamp()) % 1000000;
#endif
    AOS_INTERRUPT_DISABLE
    aos_get_utctime(&time);
    packet_header.ts_sec = aic_convert_date_to_seconds(&time);
    packet_header.ts_usec = time.msecond * 1000 + time.usecond;
    packet_header.incl_len = cap_size;
    packet_header.orig_len = packet_len;

    memcpy((u8*)blk.addr + log_header_size,
           (u8 *)&packet_header, sizeof(packet_header));
    memcpy((u8*)blk.addr + sizeof(packet_header) + log_header_size,
           (u8 *)packet_ptr, cap_size);
    alog_data_format(ALOG_DATA_CAP, INVALID_DATA_LINK, blk.addr,
                     log_header_size + sizeof(packet_header) + cap_size, TRUE);

    blk.size = log_header_size + sizeof(packet_header) + cap_size;
    ret = ablk_send_block(pcap_brr, ALDATA_LINK, &blk, 0);
    if (ret) {
        AOS_INTERRUPT_RESTORE
        aicx_info(0x070b93, "d", "[PCAP] log lost, send blk ret = %d.\n", ret);
        ablk_recycle_block(pcap_brr, &blk);
        return AIC_ENOBUFS;
    }

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}
#endif

int aic_network_pcap_update_time_do(u32 seconds, u32 milliseconds)
{
    //aic_pcap_tv.tv_sec = seconds;
    aic_pcap_tv.tv_nsec = (u64)seconds * 1000000 + (u64)milliseconds * 1000 -
                          aic_get_timestamp();
    //aic_pcap_tv.tv_sec_offset = aic_get_timestamp();

    return AIC_SUCCESS;
}

ulong aic_convert_date_to_seconds(aos_systime *local_time)
{
    int i, leaps, years = AIC_CURRENT_YEAR;
    ulong total_seconds = 0;

    if (AIC_CURRENT_YEAR > local_time->year) {
        aicx_error(0x1ab731, "d", "[TIME] get utc time error, year is %d\r\n",
                   local_time->year);
        return aic_pcap_tv.tv_sec + (aic_get_timestamp() / 1000000);;
    }

    if (local_time->year >= AIC_FIXED_YEAR) {
        years = AIC_FIXED_YEAR;
        total_seconds += AIC_LOSS_OF_UTC_TIME;
    }

    for (i = years; i < local_time->year; i++) {
        leaps = (!(i & 0x3) && (i % 100)) || (!(i % 400));
        if (leaps)
            total_seconds += 86400 * 366;
        else
            total_seconds += 86400 * 365;
    }

    leaps = (!(local_time->year & 0x3) && (local_time->year % 100)) ||
            (!(local_time->year % 400));

    if (local_time->month > 12)
        return aic_pcap_tv.tv_sec + (aic_get_timestamp() / 1000000);

    for (i = 1; i < local_time->month; i++) {
        switch (i) {
        case 1:
            total_seconds += 31 * 86400;
            break;
        case 2:
            if (!leaps)
                total_seconds += 28 * 86400;
            else
                total_seconds += 29 * 86400;
            break;
        case 3:
            total_seconds += 31 * 86400;
            break;
        case 4:
            total_seconds += 30 * 86400;
            break;
        case 5:
            total_seconds += 31 * 86400;
            break;
        case 6:
            total_seconds += 30 * 86400;
            break;
        case 7:
            total_seconds += 31 * 86400;
            break;
        case 8:
            total_seconds += 31 * 86400;
            break;
        case 9:
            total_seconds += 30 * 86400;
            break;
        case 10:
            total_seconds += 31 * 86400;
            break;
        case 11:
            total_seconds += 30 * 86400;
            break;
        case 12:
            total_seconds += 31 * 86400;
            break;
        }
    }

    total_seconds += ((local_time->day - 1) * 86400);
    total_seconds += (local_time->hour * 3600);
    total_seconds += (local_time->minute * 60);
    total_seconds += local_time->second;

    return total_seconds;
}

int aic_network_cpcap_dump(u8 *packet_ptr, u32 packet_len)
{
    int ret;
    ABLK_T blk;
    u32 cap_size, log_header_size, size;
    aos_systime time;
    AOS_INTERRUPT_DECLARE
    struct pcap_packet_header packet_header;

    if (cptcpdump == AIC_FALSE)
        return AIC_ENXIO;

    if (!packet_len)
        return AIC_EINVAL;

    if (!pcap_brr) {
        aicx_error(0x02ec80, "", "[PCAP] pcap_brr is null\n");
        return AIC_EMFILE;
    }

    if (!pcap_init_flag) {
        pcap_init_flag = 1;

        aic_pcap_tv.tv_sec = PCAP_START_UTC_TIME;
        aic_pcap_tv.tv_nsec = 0;
    }

    size = AIC_GET_PCAP_DUMP_SIZE();
    if (packet_len > size)
        cap_size = size;
    else
        cap_size = packet_len;

    log_header_size = alog_data_get_header_size(ALOG_DATA_CAP);

    ret = ablk_get_block(pcap_brr,
                         log_header_size + cap_size + sizeof(packet_header),
                         &blk, 0);
    if (ret) {
        aicx_info(0x02ccae, "d", "[PCAP] log lost, get blk ret=%d.\n", ret);
        return AIC_ENOMEM;
    }

    aos_get_utctime(&time);
    packet_header.ts_sec = aic_convert_date_to_seconds(&time);
    packet_header.ts_usec = time.msecond * 1000 + time.usecond;
    packet_header.incl_len = cap_size;
    packet_header.orig_len = packet_len;

    memcpy((u8*)blk.addr + log_header_size,
           (u8 *)&packet_header, sizeof(packet_header));
    memcpy((u8*)blk.addr + sizeof(packet_header) + log_header_size,
           (u8 *)packet_ptr, cap_size);
    AOS_INTERRUPT_DISABLE
    alog_data_format(ALOG_DATA_CAP, INVALID_DATA_LINK, blk.addr,
                     log_header_size + sizeof(packet_header) + cap_size, TRUE);

    blk.size = log_header_size + sizeof(packet_header) + cap_size;
    ret = ablk_send_block(pcap_brr, ALDATA_LINK, &blk, 0);
    if (ret) {
        AOS_INTERRUPT_RESTORE
        aicx_info(0x070b93, "d", "[PCAP] log lost, send blk ret = %d.\n", ret);
        ablk_recycle_block(pcap_brr, &blk);
        return AIC_ENOBUFS;
    }

    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}

__MCX_CODE void aic_network_parse_cp_at(u8 *string)
{
    int str_len = 0, data_len = 0;
#ifdef CONFIG_NET_PCAP_DUMP
    str_len = strlen(string);
    if (str_len > 8) {
        data_len = atoi(string + 8);
        if (data_len > PACKET_MAX_LEN)
            pcap_dump_size = PACKET_MAX_LEN;
        else
            pcap_dump_size = data_len;
    } else {
        pcap_dump_size = 0;
    }

    if (strncmp(string, "cpcapon", 7) == 0 ||
        strncmp(string, "CPCAPON", 7) == 0) {
        aic_network_cpcap_enable();
        if (!pcap_brr) {
            pcap_brr = ablk_request_brr(AIC_SYS_AP, AIPC_BRR_DEBUG);
            if (!pcap_brr) {
                aicx_error(0x068ebe, "", "[PCAP] request brr fail\r\n");
                aic_network_cpcap_disable();
            }
        }
    }

    if (strncmp(string, "cpcapoff", 8) == 0 ||
        strncmp(string, "CPCAPOFF", 8) == 0) {
        aic_network_cpcap_disable();
    }
#endif
}

__MCX_CODE void aic_network_cpcap_enable(void)
{
    cptcpdump = AIC_TRUE;
}

__MCX_CODE void aic_network_cpcap_disable(void)
{
    cptcpdump = AIC_FALSE;
}

#else
void aic_network_pcap_init(void)
{
    struct pcap_hdr pcap_header;
    size_t size;

    if (fp != NULL) {
        aicx_info(0x17d9bc, "", "pcap file already open\n");
        return;
    }

    fopen_s(&fp, "../tcpdump.pcap", "wb+");
    if (fp == NULL) {
        aicx_error(0x1085ab, "", "pcap file open fail\n");
        return;
    }

    memset(&pcap_header, 0, sizeof(struct pcap_hdr));
    pcap_header.magic_number = 0xa1b2c3d4;
    pcap_header.version_major = 0x02;
    pcap_header.version_minor = 0x04;
    pcap_header.thiszone = 0;
    pcap_header.sigfigs = 0;
    pcap_header.snaplen = 0xffff;
    pcap_header.network = RAW_IP_LINK;
    //memcpy(aic_pcap_prepend_ptr, &pcap_header, sizeof(struct pcap_hdr));

    aic_pcap_tv.tv_sec = PCAP_START_UTC_TIME;
    aic_pcap_tv.tv_nsec = 0;

    size = fwrite(&pcap_header, 1, sizeof(struct pcap_hdr), fp);
    aicx_info(0x114bd9, "d", "write %d size to pcap file\n", size);
}

int aic_network_pcap_dump(u8 *packet_ptr, u32 packet_len)
{
    u32 cap_size;
    AOS_INTERRUPT_DECLARE
    struct pcap_packet_header packet_header;
    size_t size;

    if (!packet_len)
        return AIC_EINVAL;

    if (packet_len > PCAP_DUMP_MAX_SIZE)
        cap_size = PCAP_DUMP_MAX_SIZE;
    else
        cap_size = packet_len;

    packet_header.ts_sec = aic_pcap_tv.tv_sec + (aos_get_tickcount() / 1000);
    packet_header.ts_usec = (aos_get_tickcount() % 1000) * 1000;
    packet_header.incl_len = cap_size;
    packet_header.orig_len = packet_len;

    AOS_INTERRUPT_DISABLE
    //memcpy(aic_pcap_prepend_ptr, &packet_header, sizeof(packet_header));
    size = fwrite(&packet_header, 1, sizeof(struct pcap_packet_header), fp);
    //alog_info("write %d size to pcap file\n", size);

    //memcpy(aic_pcap_prepend_ptr, packet_ptr->nx_packet_prepend_ptr, cap_size);
    size = fwrite(packet_ptr, 1, cap_size, fp);
    //alog_info("write %d size to pcap file\n", size);
    fflush(fp);
    AOS_INTERRUPT_RESTORE

    return AIC_SUCCESS;
}
#endif


