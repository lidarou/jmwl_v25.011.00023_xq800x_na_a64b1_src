/**************************************************************************/
/*                                                                        */
/*  Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.  */
/*                                                                        */
/**************************************************************************/

/**
 *  DESCRIPTION
 *
 *  Aic PDP Network Management Component.
 *  This is the management of PDP network share. After PDP is
 *  activated or deactivated,it will update the info about the PDP,
 *  also update the WAN net and the IP instance,which are used by
 *  apps and network share
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1110d9
#define LOG_MODULE    AMOD_NET

#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <strings.h>
#endif
#include <tx_api.h>
#include <nx_api.h>
#include <nxd_bsd.h>
#include <nxd_dns.h>
#include <aic_size.h>
#include <aic_init.h>
#include <aic_osa.h>
#include <aic_errno.h>
#include <subsys_api.h>
#include <connmgr_api.h>
#include <aic_md_id.h>
#include <aic_net_api.h>
#ifdef CONFIG_NET_NAT64
#include <aic_nat64.h>
#endif
#include "aic_fixed_apns.h"
#include "aic_additional_apns.h"
#include "aic_net_pdp_mgr.h"
#include "aic_at_interact_cp.h"
#include "aic_serial_recv_send.h"
#include "aic_at_urc_handle.h"
#include "aic_at_resp_handle.h"
#include "aic_general_at_handle.h"
#include "aic_at_parse_utils.h"

#ifdef CONFIG_NET_SHARE_COMMON
#include "aic_net_share_common.h"
#endif

#ifdef CONFIG_USB_NET_SHARE
#include "aic_usb_net_share_mgr.h"
#endif

#ifdef CONFIG_PPP_DIAL
#include "aic_ppp_dial_mgr.h"
#endif

#ifdef CONFIG_MISC_SUPPORT
#include <aic_md_api.h>
#endif

#ifdef CONFIG_NET_IPV6_PACKET_ROUTER
#include <aic_ipv6_forward.h>
#endif

#ifdef CONFIG_NET_ALTE_DRIVER
#include <nx_alte_driver.h>
#endif

/* Define internal vars. */
static aic_pdp_state_change_cb pdp_state_cbs[ANET_MAX_PDP_STATE_CBS];
static APN apns_set_by_user[ANET_APN_CNT_USER_SET];
static boolean try_apn_in_process = AIC_FALSE;
/* If apn has been tried,then don't invoke again unless reset. */
static boolean try_apn_done = AIC_FALSE;
static aos_thread_id thread_pdp_mgr;
static APN *all_apns_per_imsi[ANET_APN_CNT_PER_IMSI];
static APN apns_table[] = {
    //{"46000","cmnet","","",0,""},
    {AIC_NULL}
};

/* Define global vars. */
aos_mutex_ptr pdp_info_mutex;
aos_mutex_ptr wan_net_mutex;
aos_event_ptr event_pdp_mgr;

NET_PDP_INFO net_pdp_info_list[ANET_PDP_MAX_NUM];
WAN_NET wan_net_list[ANET_PDP_MAX_NUM];

struct aic_network_ip_instance alte_ip = {0};
struct aic_packet_pool anet_packet_pool = {0};

/* Define internal functions. */
static u32 create_alte_ip(void);
static u32 get_mobile_ifc_index(u16 pdp_cid, u8 *ifc_name);
static u32 config_pdp_interface(u8 wan_index, u16 pdp_index,
                                u32 *interface_index);
static u32 set_ipv6_link_local_addr(WAN_NET *wan_net, u8 *suffix, u32 ifc_index);
static void clear_pdp_info(u16 pdp_cid);
static u32 clear_wan_net_paras(u8 wan_net_index);
static u32 init_wan_net_paras(NET_PDP_INFO *net_pdp_info, u8 *wan_net_index);
static u32 attach_pdp_interface(WAN_NET *wan_net, u32 *ifc_index);
static void get_mobile_ifc_name(u16 pdp_cid, u8 *interface_name);
static void pdp_ipv6_addr_change(NX_IP *ip_ptr, u32 status,
                                 u32 interface_index,
                                 u32 address_index, ulong *ip_address);
static void reset_pdp_and_wan(u8 index);
static u32 delete_pdp_intf_ipv6_addrs(WAN_NET *wan_net_ptr, boolean is_global_only);
static u32 set_wan_global_ipv6_addrs(WAN_NET *wan_net_ptr, ulong *ip_address);
static boolean is_valid_apn_auth_type(u8 auth_type);
static u32 handle_at_urc_cgev(u8 *urc_str, u8 *start_str);
static void end_waiting_pdp_act(u8 pdp_cid);
static u8 get_cid_from_cgcontrdp_cmd(u8 *at_cmd_str);
static void parse_ipv4_addr(u8 *ipv4_addr, u8 *ip_str);
static void parse_ipv6_suffix(u8 *ipv6_suffix, u8 *ip_str, u8 ipv6_format);
static boolean is_ipv4_addr(u8 *ip_addr);
static s32 aic_handle_cgact_exe_resp(AT_REQ_RESP *at_to_ap);
static s32 aic_handle_cgact_query_resp(AT_REQ_RESP *at_to_ap);
static s32 aic_handle_cgcontrdp_set_resp(AT_REQ_RESP *at_to_ap);
static void aic_try_all_apns_by_imsi(void);

static AT_URC_FROM_CP_HANDLE net_urc_handles[] =
{
    {"+CGEV: ", handle_at_urc_cgev},

    {AIC_NULL},
};

/* The handle list of network related at response received from CP. */
static AT_RESP_FROM_CP_HANDLE net_at_resp_handles[] =
{
    {"AT+CGACT", AIC_NULL, aic_handle_cgact_query_resp,
     AIC_NULL, aic_handle_cgact_exe_resp, AIC_NULL},

    {"AT+CGCONTRDP", AIC_NULL, AIC_NULL,
     aic_handle_cgcontrdp_set_resp, AIC_NULL, AIC_NULL},

    {AIC_NULL},
};

#ifdef CONFIG_NET_NAT64
static u8 nat64_ipv4_flag[ANET_PDP_MAX_NUM];
#endif

/* Define external vars. */
extern aos_event_ptr event_send_at;
extern AT_ABUF_LINK_CFG_SET abuf_link_cfg_set;
#ifdef CONFIG_NET_SEMI_TETHER
extern struct aic_network_ip_instance usb_semi_tether_ip;
#endif
#ifdef CONFIG_PPP_DIAL
extern PPP_DIAL_NET_INFO ppp_dial_net_info;
#endif
#ifdef CONFIG_NET_SHARE_COMMON
extern aos_event_ptr event_net_share;
#endif
extern REG_SERVICE_INFO reg_service_info;
extern INIT_NET_CFG init_net_cfg;
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
extern MIPCALL_CFG mipcall_cfg[ANET_PDP_MAX_NUM];
#endif
extern aos_thread_id thread_at_rev_from_cp;

static void aic_pdp_mgr_entry(void *thread_input)
{
    u32 pdp_events = 0;

    while (1) {
        aos_get_event(event_pdp_mgr, 0xFFFFFFFF, AOS_OR_CLEAR, &pdp_events, AOS_WAIT_FOREVER);

        if (pdp_events & AT_EVENT_TRY_ALL_APN)
            aic_try_all_apns_by_imsi();
    }
}

/* Init the pdp managing module. */
s32 aic_pdp_mgr_init(void)
{
    u8 i;

    wan_net_mutex = aos_create_mutex("wan_net_mutex");
    AIC_ASSERT(wan_net_mutex);
    pdp_info_mutex = aos_create_mutex("pdp_info_mutex");
    AIC_ASSERT(pdp_info_mutex);

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        reset_pdp_and_wan(i);
    }

    aic_add_at_urc_handle(&net_urc_handles[0]);
    aic_add_at_resp_handle(&net_at_resp_handles[0]);
    aic_init_user_dns_cfg();

    if (!alte_ip.ip_ptr)
        create_alte_ip();

    event_pdp_mgr = aos_create_event("pdp_mgr");
    AIC_ASSERT(event_pdp_mgr);

    thread_pdp_mgr = aos_create_thread("pdp_mgr",
                                       aic_pdp_mgr_entry,
                                       NULL, SIZE_2K,
                                       0, NULL, 20,
                                       AOS_AUTO_START);
    if (thread_pdp_mgr == AOS_INVALID_THREAD_ID)
        return AOS_INVALID_THREAD_ID;

    return AIC_SUCCESS;
}

/* Create the IP instance for wan net. */
u32 create_alte_ip(void)
{
    u32 status;

    if (!anet_packet_pool.packet_pool) {
        anet_packet_pool.per_packet_size = ANET_PACKET_SIZE;
        anet_packet_pool.packet_pool_size = ANET_PACKET_POOL_SIZE;
        anet_packet_pool.packet_pool_name = "anet_packet_pool";
        aic_create_packet_pool(&anet_packet_pool);
    }

    alte_ip.ip_name = "ALTE_IP_INSTANCE";
    alte_ip.default_net_mask = 0xffffff00UL;
    alte_ip.pri = 7;
    alte_ip.task_stack_size = ANET_IP_TASK_STACK_SIZE;
    alte_ip.bsd_task_stack_size = SIZE_1K;
    alte_ip.bsd_task_pri = 5;
    alte_ip.bsd_owner = TRUE;

#ifdef CONFIG_NET_ALTE_DRIVER
    alte_ip.if_type = AIC_MODEM_NETWORK_LINK;
#endif
    alte_ip.packet_pool_name = "anet_packet_pool";
    alte_ip.packet_pool_size = ANET_PACKET_POOL_SIZE;
    alte_ip.per_packet_size = ANET_PACKET_SIZE;
    alte_ip.packet_pool = anet_packet_pool.packet_pool;

    status = aic_create_ip_instance(&alte_ip);
    if (status)
        return status;

    /* Enable ICMP. */
    nx_icmp_enable((NX_IP *)alte_ip.ip_ptr);
    /* Enable the IPv6 service. */
    nxd_ipv6_enable((NX_IP *)alte_ip.ip_ptr);
    /* Enable ICMPV6. */
    nxd_icmp_enable((NX_IP *)alte_ip.ip_ptr);
    nx_udp_enable((NX_IP *)alte_ip.ip_ptr);
    nx_tcp_enable((NX_IP *)alte_ip.ip_ptr);
    nx_ip_fragment_enable((NX_IP *)alte_ip.ip_ptr);

#ifdef NX_ENABLE_IP_RAW_PACKET_FILTER
    nx_ip_raw_packet_enable((NX_IP *)alte_ip.ip_ptr);
#endif

    return AIC_SUCCESS;
}

void reset_pdp_and_wan(u8 index)
{
    net_pdp_info_list[index].pdp_cid = 0;
    net_pdp_info_list[index].pdp_activated = 0;
    net_pdp_info_list[index].pdp_ifc_index = 0xff;

    aos_memset(&wan_net_list[index], 0, sizeof(WAN_NET));
    wan_net_list[index].wan_ifc_index = 0xff;
}

/* Activate certain PDP. */
u32 aic_enable_data_connection(u8 pdp_cid, u8 *pdp_type, u8 *apn)
{
    u8 cmd_str[20] = {'\0'};

    /** If PDP is not activated,then send AT+CGDCONT and AT+CGACT.
     * Or just send AT+CGACT if PDP is already activated.
     * CP would save the user number of the PDP.
     */
    if (!aic_is_pdp_activated(pdp_cid)) {
        /* For default pdp,not set apn now,in case set it error. */
        if (pdp_cid == ANET_DEFAULT_PDP_CID) {
            if (is_valid_apn_auth_type(reg_service_info.auth_type)) {
                snprintf(cmd_str, sizeof(cmd_str), "AT+CGAUTH=%d,%d", pdp_cid,
                         reg_service_info.auth_type);
                aic_send_cmd_with_callback(cmd_str, AIC_NULL);
            }
        } else {
            /** For non-default pdp,the apn name should be set by the caller. */
            aic_send_cmd_cgdcont(pdp_cid, pdp_type, apn);
        }
    }

    return aic_send_cgact_exe_cmd(ANET_PDP_TO_ACTIVATE, pdp_cid);
}

boolean is_valid_apn_auth_type(u8 auth_type)
{
    return (auth_type >= ANET_APN_AUTH_TYPE_PAP &&
            auth_type <= ANET_APN_AUTH_TYPE_CHAP);
}

s32 aic_disable_data_connection(u8 pdp_cid)
{
    s32 ret;

    if (!aic_is_pdp_activated(pdp_cid)) {
        return AIC_SUCCESS;
    }

    ret = aic_send_cgact_exe_cmd(ANET_PDP_TO_DEACTIVATE, pdp_cid);

    return ret;
}

void aic_attach_packet_domain(void)
{
    aic_send_cmd_with_callback("AT+CGATT=1", AIC_NULL);
}

u32 aic_send_cgact_exe_cmd(u8 is_to_deact, u8 pdp_cid)
{
    u8 cmd_str[20] = {'\0'};

    snprintf(cmd_str, sizeof(cmd_str), "AT+CGACT=%d,%d", is_to_deact, pdp_cid);
    return aic_send_cmd_with_callback(cmd_str, AIC_NULL);
}

u32 aic_send_cgcontrdp_set_cmd(u8 cid)
{
    u8 at_buf[20] = {'\0'};

    snprintf(at_buf, sizeof(at_buf), "AT+CGCONTRDP=%d", cid);
    return aic_send_cmd_with_callback(at_buf, AIC_NULL);
}

void aic_send_cmd_cgdcont(u8 cid, u8 *pdp_type, u8 *apn_name)
{
    u8 cmd_str[80] = {'\0'};

    snprintf(cmd_str, sizeof(cmd_str), "AT+CGDCONT=%d,\"", cid);
    if (pdp_type && strlen(pdp_type) > 0) {
        strncat(cmd_str, pdp_type, ANET_PDP_TYPE_MAX_LEN);
    } else {
        strncat(cmd_str, "IPV4V6", ANET_PDP_TYPE_MAX_LEN);
    }
    strcat(cmd_str, "\",\"");

    if (apn_name && strlen(apn_name) > 0)
        strncat(cmd_str, apn_name, (sizeof(cmd_str) - strlen(cmd_str) - 2));

    strcat(cmd_str, "\"");
    aic_send_cmd_with_callback(cmd_str, AIC_NULL);
}

#ifdef CONFIG_NET_NAT64
static void aic_pdp_enable_nat64(WAN_NET *wan_net)
{
    u32 local_prefix[3];
    u8 external_prefix[12] = {0x00, 0x64, 0xff, 0x9b, 0x00, 0x00, 0x00, 0x00,
                              0x00, 0x00, 0x00, 0x00};
    u8 local_prefix_len = 96, external_prefix_len = 96, i;
    u32 ipv4_addr, ipv4_mask = 0xFFFFFF00, ret;
    u8 last_byte[ANET_PDP_MAX_NUM] = {4,5,6,7,1};

    if (!wan_net)
        return;

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (nat64_ipv4_flag[i] == 0)
            break;
    }

    if (i == ANET_PDP_MAX_NUM)
        return;

    nat64_ipv4_flag[i] = 1;
    ipv4_addr = IP_ADDRESS(192, 0, 0, last_byte[i]);
    wan_net->nat64_ipv4_index = i;
    local_prefix[0] = htonl(wan_net->wan_ipv6_address1[0]);
    local_prefix[1] = htonl(wan_net->wan_ipv6_address1[1]);
    local_prefix[2] = 0;

    if (wan_net->pdp_cid == ANET_DEFAULT_PDP_CID)
        _nx_ip_gateway_address_set(alte_ip.ip_ptr, ipv4_addr);

    ret = aic_nat64_enable(wan_net->wan_ifc_index, ipv4_addr, ipv4_mask, local_prefix_len,
                           (u8 *)local_prefix, external_prefix_len, external_prefix,
                           ANET_NAT64_TABLE_NUM);
    if (ret)
        aicx_error(0x1a0455, "x", "nat64 enable error 0x%x\n", ret);

    return;
}
#endif

/** After receiving RA from core network,then the global ipv6 address is
 * obtained for the PDP.And then the function is called.
 */
void pdp_ipv6_addr_change(NX_IP *ip_ptr, u32 status, u32 interface_index,
                          u32 address_index, ulong *ip_address)
{
    WAN_NET *wan_net_ptr;

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    aicx_info(0x1429a7, "dddxxxx", "ipv6 addr change:status %d,intf index %d,addr index %d, 0x%x 0x%x 0x%x 0x%x\n",
              status, interface_index, address_index, ip_address[0], ip_address[1],
              ip_address[2], ip_address[3]);

    if (status == NX_IPV6_ADDRESS_STATELESS_AUTO_CONFIG) {
        if (ip_address[0] == 0xfe800000)
            AIC_ASSERT(0);

        aos_get_mutex(wan_net_mutex, AOS_WAIT_FOREVER);
        wan_net_ptr = aic_get_wan_net_by_ifc_index(interface_index);
        if (!wan_net_ptr) {
            aos_put_mutex(wan_net_mutex);
            return;
        }

        set_wan_global_ipv6_addrs(wan_net_ptr, ip_address);
        aos_put_mutex(wan_net_mutex);
        if(!aic_boot_mode_is_basic()) {
#ifdef CONFIG_PPP_DIAL
            /* Config stuff about IPv6 if ppp dial is started. */
            aic_ppp_dial_pdp_activated(wan_net_ptr->wan_net_id);
#endif
#ifdef CONFIG_USB_NET_SHARE
            /* Config stuff about IPv6 if usb net share is started. */
            aic_notify_usb_tether_ipv6_change(wan_net_ptr->wan_net_id);
#endif
        }
    }

    if (status == NX_IPV6_ADDRESS_DAD_SUCCESSFUL) {
        if (ip_address[0] == 0xfe800000)
            return;

#ifdef CONFIG_NET_NAT64
        wan_net_ptr = aic_get_wan_net_by_ifc_index(interface_index);
        if (wan_net_ptr && aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_address1) &&
            aic_get_pdp_ip_type(wan_net_ptr->pdp_cid) == ANET_DEFAULT_IP_TYPE_V6) {
            aic_pdp_enable_nat64(wan_net_ptr);
        }
#endif

        if(!aic_boot_mode_is_basic()) {
#ifdef CONFIG_USB_NET_SHARE
            aic_usb_ipv6_dad_success(ip_ptr, interface_index, address_index);
#endif
#ifdef CONFIG_PPP_DIAL
            aic_ppp_ipv6_dad_success(ip_ptr, interface_index, address_index);
#endif
        }
    }
#endif
}

u32 set_wan_global_ipv6_addrs(WAN_NET *wan_net_ptr, ulong *ip_address)
{
    u16 i;
    NET_PDP_INFO *pdp_info;
    u8 ipv6_str[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    ulong ipv6_suffix[4];
    NXD_ADDRESS global_ipv6;
    u32 addr_index = 0xff;
    u32 interface_index = 0xff;
    u32 status;

    pdp_info = aic_get_pdp_info_by_cid(wan_net_ptr->wan_net_id);
    if (!pdp_info) {
        return ANET_ERROR_PDP_NOT_FOUND;
    }

    /* Check if it is the same prefix as the old ipv6 addr. */
    if (wan_net_ptr->wan_ipv6_address1[0] == ip_address[0] &&
        wan_net_ptr->wan_ipv6_address1[1] == ip_address[1]) {
        return AIC_SUCCESS;
    } else if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_address1)) {
        /* Delete the old global ipv6 addresses. */
        delete_pdp_intf_ipv6_addrs(wan_net_ptr, AIC_TRUE);
    }

    /* Not save ipv6 addr generated from all zero MAC addr to wan_ipv6_address2. */
    for (i = 0; i < 2; i++) {
        /* Save the prefix to address1,suffix is from PDP activation. */
        wan_net_ptr->wan_ipv6_address1[i] = ip_address[i];
    }

    /** Generate the other ipv6 global address via suffix got from core net,
     * and set the address to wan_ipv6_address1 and the IP instance.
     * No need to set wan_ipv6_address2 as it is set to the IP instance
     * when process the RA packet.
     */
    if (strlen(pdp_info->pdp_ipv6_suffix_hex) > 0) {
        /** Consider that the prefix length is 64 for default.
         * Maybe should also check the prefix length ??
         */
        strcpy(ipv6_str, "::");
        strncat(ipv6_str, pdp_info->pdp_ipv6_suffix_hex,
                (ANET_MAX_LEN_HEX_IPV6_ADDR - 3));
        inet_pton(AF_INET6, ipv6_str, &ipv6_suffix[0]);
        /* Suffix is stored in ipv6_suffix of index 2 and 3. */
        wan_net_ptr->wan_ipv6_address1[2] = ipv6_suffix[2];
        wan_net_ptr->wan_ipv6_address1[3] = ipv6_suffix[3];
        NX_CHANGE_ULONG_ENDIAN(wan_net_ptr->wan_ipv6_address1[2]);
        NX_CHANGE_ULONG_ENDIAN(wan_net_ptr->wan_ipv6_address1[3]);
    }

    interface_index = wan_net_ptr->wan_ifc_index;
    global_ipv6.nxd_ip_version = NX_IP_VERSION_V6;
    for (i = 0; i < 4; i++) {
        global_ipv6.nxd_ip_address.v6[i] = wan_net_ptr->wan_ipv6_address1[i];
    }

    status = nxd_ipv6_address_set((NX_IP *)alte_ip.ip_ptr,
                                  interface_index,
                                  &global_ipv6, 64, &addr_index);
    if (!status) {
        aicx_info(0x0357c9, "dxxxx", "pdp cid %d,set global v6 addr1 0x%08x 0x%08x 0x%08x 0x%08x\n",
                  wan_net_ptr->wan_net_id,
                  global_ipv6.nxd_ip_address.v6[0],
                  global_ipv6.nxd_ip_address.v6[1],
                  global_ipv6.nxd_ip_address.v6[2],
                  global_ipv6.nxd_ip_address.v6[3]);
    }

    return status;
}

NET_PDP_INFO *aic_get_pdp_info_by_cid(u8 cid)
{
    u16 index = ANET_PDP_MAX_NUM;
    u16 i;

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == cid) {
            index = i;
            break;
        }
    }

    if (index == ANET_PDP_MAX_NUM) {
        return AIC_NULL;
    } else {
        return &net_pdp_info_list[index];
    }
}

boolean aic_is_pdp_activated(u16 cid)
{
    boolean is_activated = AIC_FALSE;
    u8 i;
    NET_PDP_INFO *pdp_info;

    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == cid) {
            pdp_info = &net_pdp_info_list[i];
            if (pdp_info->pdp_activated == ANET_PDP_ACTIVATED &&
                (strlen(pdp_info->pdp_ipv4_address) > 0 ||
                strlen(pdp_info->pdp_ipv6_suffix_hex) > 0)) {
                is_activated = AIC_TRUE;
                break;
            }
        }
    }
    aos_put_mutex(pdp_info_mutex);

    return is_activated;
}

boolean aic_is_default_pdp_activated(void)
{
    u8 i;

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == ANET_DEFAULT_PDP_CID &&
            net_pdp_info_list[i].pdp_activated == ANET_PDP_ACTIVATED &&
            (strlen(net_pdp_info_list[i].pdp_ipv4_address) > 0 ||
            strlen(net_pdp_info_list[i].pdp_ipv6_suffix_hex) > 0)) {
            return AIC_TRUE;
        }
    }

    return AIC_FALSE;
}

/** The format of decimal_ipv6 is like this:
 * 10.36.125.47.00.00.172.245.0.0.58.0.0.0.253.1
 */
u32 ipv6_decimal_to_hex(u8 *dst_ptr, u8 *decimal_ipv6)
{
    u8 *tmp_ptr1, *tmp_ptr2;
    u8 decimal_number, count = 0;
    char *save_ptr;

    if (!decimal_ipv6 || strlen(decimal_ipv6) == 0) {
        return AIC_SUCCESS;
    }

    tmp_ptr2 = dst_ptr;
    tmp_ptr1 = strtok_r(decimal_ipv6, ".", &save_ptr);
    while (tmp_ptr1) {
        count++;
        decimal_number = atoi(tmp_ptr1);
        snprintf(tmp_ptr2, 3, "%02x", decimal_number);
        tmp_ptr2 += 2;
        if (count % 2 == 0) {
            *tmp_ptr2 = ':';
            tmp_ptr2++;
        }

        tmp_ptr1 = strtok_r(AIC_NULL, ".", &save_ptr);
    }

    if (*(tmp_ptr2 - 1) == ':') {
        //alog_info("%s:the last char is : \n", __func__);
        *(tmp_ptr2 - 1) = '\0';
    }

    return AIC_SUCCESS;
}

static void set_gateway_via_activated_pdp(void)
{
    u8 i;
    WAN_NET *wan_net;

    wan_net = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (wan_net && wan_net->pdp_activated == ANET_PDP_ACTIVATED) {
        alte_ip.ip_ptr->nx_ip_gateway_address = wan_net->wan_ipv4_gateway;
        alte_ip.ip_ptr->nx_ip_gateway_interface =
            &(alte_ip.ip_ptr->nx_ip_interface[wan_net->wan_ifc_index]);
    } else if (!alte_ip.ip_ptr->nx_ip_gateway_address) {
        wan_net = aic_get_all_wan_net();
        for (i = 0; i < aic_get_max_wan_net_num(); i++) {
             if (wan_net && wan_net->pdp_activated == ANET_PDP_ACTIVATED) {
                 alte_ip.ip_ptr->nx_ip_gateway_address = wan_net->wan_ipv4_gateway;
                 alte_ip.ip_ptr->nx_ip_gateway_interface =
                     &(alte_ip.ip_ptr->nx_ip_interface[wan_net->wan_ifc_index]);
                 break;
             }

            wan_net++;
        }
    }
}

/** Set PDP info when PDP is activated.
 * It should be called when pdp is activated, not only for net share,
 * but also for internal use.
 */
u32 aic_set_pdp_activated(u16 pdp_cid, u8 *ipv4_addr, u8 *ipv4_dns1,
                          u8 *ipv4_dns2, u8 *ipv6_suffix, u8 *ipv6_dns1,
                          u8 *ipv6_dns2, u8 ipv6_format)
{
    u8 i, pdp_info_index = ANET_PDP_MAX_NUM;
    u32 interface_index = NX_MAX_PHYSICAL_INTERFACES, status;
    u8 wan_net_index = ANET_PDP_MAX_NUM;
    NET_PDP_INFO *pdp_info_ptr;
    WAN_NET *wan_net_ptr;
    //long global_ipv6[4] = {0x20010000,0x00,0x05,0x0f};

    if (!aic_is_valid_pdp_cid(pdp_cid)) {
        return ANET_ERROR_INVALID_PDP;
    }

    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == pdp_cid) {
            aos_put_mutex(pdp_info_mutex);
            return AIC_SUCCESS;
        }
    }

    /* Look up the idle pdp_info. */
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == 0) {
            pdp_info_index = i;
            break;
        }
    }
    if (pdp_info_index == ANET_PDP_MAX_NUM) {
        aicx_error(0x175e47, "", "no empty pdp info\n");
        aos_put_mutex(pdp_info_mutex);
        return ANET_ERROR_PDP_OVERFLOW;
    }

    pdp_info_ptr = &net_pdp_info_list[pdp_info_index];
    pdp_info_ptr->pdp_cid = pdp_cid;
    pdp_info_ptr->pdp_activated = ANET_PDP_ACTIVATED;

    if (ipv4_addr && strlen(ipv4_addr) > 0) {
        strncpy(pdp_info_ptr->pdp_ipv4_address, ipv4_addr,
                (ANET_MAX_LEN_IPV4_ADDR - 1));
        if (ipv4_dns1)
            strncpy(pdp_info_ptr->pdp_ipv4_dns1_address, ipv4_dns1,
                    (ANET_MAX_LEN_IPV4_ADDR - 1));
        if (ipv4_dns2)
            strncpy(pdp_info_ptr->pdp_ipv4_dns2_address, ipv4_dns2,
                    (ANET_MAX_LEN_IPV4_ADDR - 1));
    }

    if (ipv6_suffix && strlen(ipv6_suffix) > 0) {
        if (ipv6_format == ANET_IPV6_FORMAT_DECIMAL) {
            ipv6_decimal_to_hex(pdp_info_ptr->pdp_ipv6_suffix_hex, ipv6_suffix);
            ipv6_decimal_to_hex(pdp_info_ptr->pdp_ipv6_dns1_address, ipv6_dns1);
            ipv6_decimal_to_hex(pdp_info_ptr->pdp_ipv6_dns2_address, ipv6_dns2);
        } else {
            strncpy(pdp_info_ptr->pdp_ipv6_suffix_hex, ipv6_suffix,
                    (sizeof(pdp_info_ptr->pdp_ipv6_suffix_hex) - 1));
            strncpy(pdp_info_ptr->pdp_ipv6_dns1_address, ipv6_dns1,
                    (sizeof(pdp_info_ptr->pdp_ipv6_dns1_address) - 1));
            strncpy(pdp_info_ptr->pdp_ipv6_dns2_address, ipv6_dns2,
                    (sizeof(pdp_info_ptr->pdp_ipv6_dns2_address) - 1));
        }
    }
    aos_put_mutex(pdp_info_mutex);

    aos_get_mutex(wan_net_mutex, AOS_WAIT_FOREVER);
    status = init_wan_net_paras(pdp_info_ptr, &wan_net_index);
    if (status) {
        aicx_error(0x0ef82b, "d", "init wan paras error %d\n", status);
        aos_put_mutex(wan_net_mutex);
        goto CONFIG_ERROR;
    }

    if (!alte_ip.ip_ptr) {
        status = create_alte_ip();
        if (status) {
            aicx_error(0x182486, "d", "create ip instance error--%d\n", status);
            aos_put_mutex(wan_net_mutex);
            goto CONFIG_ERROR;
        }
    }

    status = config_pdp_interface(wan_net_index, pdp_info_index, &interface_index);
    if (status || interface_index == NX_MAX_PHYSICAL_INTERFACES) {
        aicx_error(0x0868b8, "d", "config wan interface error %d\n", status);
        aos_put_mutex(wan_net_mutex);
        goto CONFIG_ERROR;
    }

    wan_net_ptr = &wan_net_list[wan_net_index];
    net_pdp_info_list[pdp_info_index].pdp_ifc_index = interface_index;
    wan_net_ptr->wan_ip_instance = (NX_IP *)alte_ip.ip_ptr;
    set_gateway_via_activated_pdp();
    aos_put_mutex(wan_net_mutex);

    if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_local_ipv6_addr)) {
        /** To be notified when ipv6 global addr is changed via auto config,
         * due to receiving RA from core network.
         */
        nxd_ipv6_address_change_notify((NX_IP *)alte_ip.ip_ptr,
                                       pdp_ipv6_addr_change);
    }

    if (!alte_ip.bsd_task_stack)
        aic_init_bsd(&alte_ip);

#ifdef CONFIG_NX_DNS
    aic_update_dns_servers(wan_net_ptr);
#endif

    return AIC_SUCCESS;

CONFIG_ERROR:
    clear_pdp_info(pdp_cid);
    clear_wan_net_paras(wan_net_index);

    return status;
}

/** Reset PDP info and wan net when PDP is deactivated.
 * It should be called when pdp is deactivated, not only for net share,
 * but also for internal use.
 */
u32 aic_set_pdp_deactivated(u16 pdp_cid)
{
    u8 wan_index, i;
    u32 status, ifc_index;
    AT_ABUF_LINK_CFG *abuf_link_cfg;

    if (!aic_is_pdp_activated(pdp_cid)) {
        aicx_info(0x0d49f2, "", "set pdp deact:already deact\n");
        return AIC_SUCCESS;
    }

    clear_pdp_info(pdp_cid);

    aos_get_mutex(wan_net_mutex, AOS_WAIT_FOREVER);
    wan_index = aic_get_wan_net_by_cid(pdp_cid);
    if (wan_index == ANET_PDP_MAX_NUM) {
        aos_put_mutex(wan_net_mutex);
        return ANET_ERROR_WAN_NOT_FOUND;
    }

    ifc_index = wan_net_list[wan_index].wan_ifc_index;
#ifdef CONFIG_NET_NAT64
    if (aic_get_pdp_ip_type(pdp_cid) == ANET_DEFAULT_IP_TYPE_V6) {
        aicx_info(0x0ed743, "d", "disable nat64 for intertf %d\n", ifc_index);
        aic_nat64_disable(ifc_index);
        nat64_ipv4_flag[wan_net_list[wan_index].nat64_ipv4_index] = 0;
    }
#endif
    /* Detach will set the interface to invalid,so just delete the ipv6 addrs. */
    delete_pdp_intf_ipv6_addrs(&wan_net_list[wan_index], AIC_FALSE);

    /* basic mode can't call it. */
    if (!aic_boot_mode_is_basic()) {
        status = nx_ip_interface_detach((NX_IP *)alte_ip.ip_ptr, ifc_index);
        if (status)
            aicx_error(0x1c0f18, "dx", "interf index %d detach error 0x%x\n", ifc_index, status);
    }

    status = clear_wan_net_paras(wan_index);
    set_gateway_via_activated_pdp();
    aos_put_mutex(wan_net_mutex);

    for (i = AT_LINK0; i < AT_LINK_CNT; i++) {
        abuf_link_cfg = &(abuf_link_cfg_set.at_abuf_link_list[i]);
        if (abuf_link_cfg->activated_pdp_cid == pdp_cid &&
            abuf_link_cfg->resp_complete == AT_RESP_BLOCKED) {
            aos_get_mutex(abuf_link_cfg->abuf_link_mutex, AOS_WAIT_FOREVER);
            aic_set_abuf_link_idle(abuf_link_cfg);
            aos_put_mutex(abuf_link_cfg->abuf_link_mutex);
        }
    }

    if (pdp_cid != ANET_DEFAULT_PDP_CID)
        aic_dns_delete_socket_by_cid(pdp_cid);

    return status;
}

u32 delete_pdp_intf_ipv6_addrs(WAN_NET *wan_net_ptr, boolean is_global_only)
{
    u32 index = NX_MAX_IPV6_ADDRESSES, ret = AIC_SUCCESS;
    u8 i;
    NXD_ADDRESS ipv6_addr;

    ipv6_addr.nxd_ip_version = NX_IP_VERSION_V6;
    if (!is_global_only) {
        if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_local_ipv6_addr)) {
            for (i = 0; i < 4; i++) {
                ipv6_addr.nxd_ip_address.v6[i] = wan_net_ptr->wan_local_ipv6_addr[i];
            }
            aic_get_ipv6_addr_index(&alte_ip, &ipv6_addr, &index);
            if (index < NX_MAX_IPV6_ADDRESSES) {
                ret = nxd_ipv6_address_delete(alte_ip.ip_ptr, index);
                if (ret)
                    aicx_error(0x020525, "x", "delete pdp local addr error 0x%x\n", ret);
            }
        }
    }

    if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_address1)) {
        index = NX_MAX_IPV6_ADDRESSES;
        for (i = 0; i < 4; i++) {
            ipv6_addr.nxd_ip_address.v6[i] = wan_net_ptr->wan_ipv6_address1[i];
        }
        aic_get_ipv6_addr_index(&alte_ip, &ipv6_addr, &index);

        if (index < NX_MAX_IPV6_ADDRESSES) {
            aicx_info(0x0ead41, "dd", "delete ipv6 global addr 1 index %d,interf index %d\n",
                      index, wan_net_ptr->wan_ifc_index);
            ret = nxd_ipv6_address_delete(alte_ip.ip_ptr, index);
            if (ret)
                aicx_error(0x15bfd7, "x", "delete pdp global ipv6 addr1 error 0x%x\n", ret);
        }
    }

    if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_address2)) {
        index = NX_MAX_IPV6_ADDRESSES;
        for (i = 0; i < 4; i++) {
            ipv6_addr.nxd_ip_address.v6[i] = wan_net_ptr->wan_ipv6_address2[i];
        }
        aic_get_ipv6_addr_index(&alte_ip, &ipv6_addr, &index);
        if (index < NX_MAX_IPV6_ADDRESSES) {
            aicx_info(0x07087b, "dd", "delete ipv6 global addr 2 index %d,interf index %d\n",
                      index, wan_net_ptr->wan_ifc_index);
            ret = nxd_ipv6_address_delete(alte_ip.ip_ptr, index);
            if (ret)
                aicx_error(0x1bce1a, "x", "delete pdp global ipv6 addr2 error 0x%x\n", ret);
        }
    }

    return ret;
}

struct aic_network_ip_instance *aic_get_pdp_ip_instance(void)
{
    return &alte_ip;
}

boolean all_pdp_deactivated(void)
{
    u16 i;

    /* Clear pdp_info struct. */
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid > 0 &&
            net_pdp_info_list[i].pdp_activated == ANET_PDP_ACTIVATED) {
            return FALSE;
        }
    }

    return TRUE;
}

u32 config_pdp_interface(u8 wan_index, u16 pdp_index, u32 *interface_index)
{
    u16 pdp_cid;
    u32 status;
    WAN_NET *wan_net_ptr;
    NX_INTERFACE *nx_intf_ptr;
    struct NXD_IPV6_ADDRESS_STRUCT *ipv6_addr;
    NET_PDP_INFO *pdp_info_ptr;

    pdp_info_ptr = &net_pdp_info_list[pdp_index];
    pdp_cid = pdp_info_ptr->pdp_cid;
    if (!aic_is_valid_pdp_cid(pdp_cid)) {
        return ANET_ERROR_INVALID_PDP;
    }

    wan_net_ptr = &wan_net_list[wan_index];
    status = attach_pdp_interface(wan_net_ptr, interface_index);
    if (status) {
        aicx_error(0x1587f2, "x", "attach pdp interface error 0x%x\n", status);
        return status;
    }

    /* Set up the ipv6 link local address. */
    if (strlen(pdp_info_ptr->pdp_ipv6_suffix_hex) > 0) {
        set_ipv6_link_local_addr(wan_net_ptr,
                                 pdp_info_ptr->pdp_ipv6_suffix_hex,
                                 *interface_index);

        nx_intf_ptr = &alte_ip.ip_ptr->nx_ip_interface[*interface_index];
        ipv6_addr = nx_intf_ptr->nxd_interface_ipv6_address_list_head;
        while (ipv6_addr) {
            if (ipv6_addr->nxd_ipv6_address[0] &&
                ipv6_addr->nxd_ipv6_address[0] != 0xfe800000) {
                set_wan_global_ipv6_addrs(wan_net_ptr, ipv6_addr->nxd_ipv6_address);
                break;
            }

            ipv6_addr = ipv6_addr->nxd_ipv6_address_next;
        }
    }

    /* Set MTU size. */
    status = nx_ip_interface_mtu_set((NX_IP *)alte_ip.ip_ptr,
                                     *interface_index,
                                     ANET_PDP_MTU_SIZE);
    if (status) {
        aicx_error(0x15c201, "x", "set wan interf mtu error 0x%x\n", status);
    }

    return AIC_SUCCESS;
}

u32 set_ipv6_link_local_addr(WAN_NET *wan_net, u8 *suffix, u32 ifc_index)
{
    u8 ipv6_str[ANET_MAX_LEN_HEX_IPV6_ADDR] = {'\0'};
    NXD_ADDRESS link_local_addr = {0};
    u16 i;
    u32 status;
    u32 addr_index;

    link_local_addr.nxd_ip_version = NX_IP_VERSION_V6;
    if (suffix && strlen(suffix) > 0) {
        strncpy(ipv6_str, "fe80::", strlen("fe80::"));
        strncat(ipv6_str, suffix,
                (ANET_MAX_LEN_HEX_IPV6_ADDR - strlen("fe80::") -1));
        inet_pton(AF_INET6, ipv6_str, &wan_net->wan_local_ipv6_addr[0]);
    }

    for (i = 0; i < 4; i++) {
        NX_CHANGE_ULONG_ENDIAN(wan_net->wan_local_ipv6_addr[i]);
        link_local_addr.nxd_ip_address.v6[i] = wan_net->wan_local_ipv6_addr[i];
    }

    /* Prefix length of link local addr is 10. */
    status = nxd_ipv6_address_set(alte_ip.ip_ptr, ifc_index,
                                  &link_local_addr, 10, &addr_index);

    if (status) {
        aicx_error(0x0ef054, "x", "set pdp ipv6 local addr error 0x%x\n", status);
        return status;
    }
    aicx_info(0x1d7a3e, "xxxx", "pdp link local addr 0x%08x 0x%08x 0x%08x 0x%08x\n",
              wan_net->wan_local_ipv6_addr[0],
              wan_net->wan_local_ipv6_addr[1],
              wan_net->wan_local_ipv6_addr[2],
              wan_net->wan_local_ipv6_addr[3]);

    return AIC_SUCCESS;
}

u32 attach_pdp_interface(WAN_NET *wan_net, u32 *ifc_index)
{
    u16 pdp_cid;
    u32 status;

    pdp_cid = wan_net->pdp_cid;
    /* Assign interface index in _nx_ip_interface_attach. */
    status = nx_ip_interface_attach(alte_ip.ip_ptr, wan_net->wan_ifc_name,
                                    wan_net->wan_ipv4_address,
                                    wan_net->wan_ipv4_net_mask,
#ifdef WIN32
                                    AIC_NULL
#else
#ifdef CONFIG_NET_ALTE_DRIVER
                                    nx_alte_nic_driver_entry
#else
                                    AIC_NULL
#endif
#endif
                                    );

    if (status) {
        *ifc_index = NX_MAX_PHYSICAL_INTERFACES;
        return status;
    }

    /* Get interface index via interface name set above. */
    *ifc_index = get_mobile_ifc_index(pdp_cid, wan_net->wan_ifc_name);
    if (*ifc_index == NX_MAX_PHYSICAL_INTERFACES) {
       status = ANET_ERROR_INVALID_IFC_INDEX;
       return status;
    }

    aicx_info(0x17147d, "d", "interface attached to index %d\n", (*ifc_index));
    alte_ip.ip_ptr->nx_ip_interface[*ifc_index].nx_interface_name = wan_net->wan_ifc_name;
    wan_net->wan_ifc_index = *ifc_index;

    return AIC_SUCCESS;
}

/* It is called after PDP is activated, to set paras of WAN net. */
u32 init_wan_net_paras(NET_PDP_INFO *net_pdp_info, u8 *wan_net_index)
{
    u8 i, pdp_cid = 0;
    u8 wan_index = ANET_PDP_MAX_NUM;
    u8 ip_num[4] = {0};
    WAN_NET *wan_net_ptr;

    pdp_cid = net_pdp_info->pdp_cid;
    wan_index = aic_get_wan_net_by_cid(pdp_cid);
    if (wan_index == ANET_PDP_MAX_NUM) {
        for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
            if (wan_net_list[i].pdp_cid == 0) {
                wan_index = i;
                *wan_net_index = i;
                break;
            }
        }
    } else {
        *wan_net_index = wan_index;
    }

    if (wan_index == ANET_PDP_MAX_NUM) {
        return ANET_ERROR_PDP_OVERFLOW;
    }

    wan_net_ptr = &wan_net_list[wan_index];
    wan_net_ptr->wan_net_id = pdp_cid;
    wan_net_ptr->pdp_cid = pdp_cid;
    wan_net_ptr->pdp_activated = net_pdp_info->pdp_activated;
    get_mobile_ifc_name(pdp_cid, wan_net_ptr->wan_ifc_name);

    if (strlen(net_pdp_info->pdp_ipv4_address) > 0) {
        /*inet_pton(AF_INET, net_pdp_info->pdp_ipv4_address,
                  &wan_net_list[wan_index].wan_ipv4_address);

        inet_pton(AF_INET, net_pdp_info->pdp_ipv4_dns1_address,
                    &wan_net_list[wan_index].wan_ipv4_dns1_address);

        inet_pton(AF_INET, net_pdp_info->pdp_ipv4_dns2_address,
                  &wan_net_list[wan_index].wan_ipv4_dns2_address);*/
        get_numeric_ipv4_addr(ip_num, net_pdp_info->pdp_ipv4_address);
        wan_net_ptr->wan_ipv4_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        /* The last byte set to 1,as the gateway. */
        ip_num[3] = 1;
        wan_net_ptr->wan_ipv4_gateway =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        aos_memset(ip_num, 0, sizeof(ip_num));
        get_numeric_ipv4_addr(ip_num, net_pdp_info->pdp_ipv4_dns1_address);
        wan_net_ptr->wan_ipv4_dns1_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        aos_memset(ip_num, 0, sizeof(ip_num));
        get_numeric_ipv4_addr(ip_num, net_pdp_info->pdp_ipv4_dns2_address);
        wan_net_ptr->wan_ipv4_dns2_address =
            IP_ADDRESS(ip_num[0], ip_num[1], ip_num[2], ip_num[3]);

        wan_net_ptr->wan_ipv4_net_mask = ANET_IPV4_DEFAULT_NET_MASK;
    }

    if (strlen(net_pdp_info->pdp_ipv6_suffix_hex) > 0) {
        if (strlen(net_pdp_info->pdp_ipv6_dns1_address) == 0) {
            strncpy(net_pdp_info->pdp_ipv6_dns1_address,
                    ANET_DEFAULT_IPV6_DNS_SERVER1,
                    (ANET_MAX_LEN_HEX_IPV6_ADDR - 1));
            strncpy(net_pdp_info->pdp_ipv6_dns2_address,
                    ANET_DEFAULT_IPV6_DNS_SERVER2,
                    (ANET_MAX_LEN_HEX_IPV6_ADDR - 1));
        }

        inet_pton(AF_INET6, net_pdp_info->pdp_ipv6_dns1_address,
                  &wan_net_ptr->wan_ipv6_dns1_address[0]);
        inet_pton(AF_INET6, net_pdp_info->pdp_ipv6_dns2_address,
                  &wan_net_ptr->wan_ipv6_dns2_address[0]);
        for (i = 0; i < 4; i++) {
            NX_CHANGE_ULONG_ENDIAN(wan_net_ptr->wan_ipv6_dns1_address[i]);
            NX_CHANGE_ULONG_ENDIAN(wan_net_ptr->wan_ipv6_dns2_address[i]);
        }
    }

    return AIC_SUCCESS;
}

/* Get numeric ipv4 addr from string. Save it in ip_num. */
void get_numeric_ipv4_addr(u8 *ip_num, u8 *ipv4_str)
{
    u8 i = 0;
    u8 *tmp_ptr;
    char *save_ptr;
    u8 tmp_ipv4[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};

    if (strlen(ipv4_str) == 0)
        return;

    strncpy(tmp_ipv4, ipv4_str, (ANET_MAX_LEN_IPV4_ADDR - 1));

    tmp_ptr = strtok_r(tmp_ipv4, ".", &save_ptr);
    while (tmp_ptr) {
        ip_num[i] = atoi(tmp_ptr);
        i++;
        if (i > 3) {
            break;
        }
        tmp_ptr = strtok_r(AIC_NULL, ".", &save_ptr);
    }
}

u32 clear_wan_net_paras(u8 wan_net_index)
{
    if (wan_net_index >= ANET_PDP_MAX_NUM) {
        return ANET_ERROR_WAN_NOT_FOUND;
    }

    aos_memset(&wan_net_list[wan_net_index], 0, sizeof(WAN_NET));
    wan_net_list[wan_net_index].wan_ifc_name[0] = '\0';

    return AIC_SUCCESS;
}

void clear_pdp_info(u16 pdp_cid)
{
    u8 i;

    /* Clear pdp_info struct. */
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid == pdp_cid) {
            aos_memset(&net_pdp_info_list[i], 0, sizeof(NET_PDP_INFO));
            break;
        }
    }
}

u8 aic_get_wan_net_by_cid(u8 cid)
{
    u8 i;

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (wan_net_list[i].pdp_cid == cid) {
            return i;
        }
    }

    return ANET_PDP_MAX_NUM;
}

WAN_NET *aic_get_wan_net_per_pdp(u8 pdp_cid)
{
    u8 index;

    if (!aic_is_valid_pdp_cid(pdp_cid))
        return AIC_NULL;

    index = aic_get_wan_net_by_cid(pdp_cid);
    if (index != ANET_PDP_MAX_NUM) {
        return &wan_net_list[index];
    } else {
        return AIC_NULL;
    }
}

/**
 * If the default PDP is activated,then return the WAN_NET of default PDP.
 * If the default PDP is deactivated,then return NULL.
 */
WAN_NET *aic_get_activated_default_wan(void)
{
    WAN_NET *wan_net;

    wan_net = aic_get_wan_net_per_pdp(ANET_DEFAULT_PDP_CID);
    if (wan_net && wan_net->pdp_activated == ANET_PDP_ACTIVATED)
        return wan_net;

    return AIC_NULL;
}

WAN_NET *aic_get_wan_net_by_ifc_index(u32 ifc_index)
{
    u32 index = ANET_PDP_MAX_NUM;
    u16 i;

    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (wan_net_list[i].wan_ifc_index == ifc_index) {
            index = i;
            break;
        }
    }

    if (index == ANET_PDP_MAX_NUM) {
        return AIC_NULL;
    } else {
        return &wan_net_list[index];
    }
}

void get_mobile_ifc_name(u16 pdp_cid, u8 *interface_name)
{
    u8 cid_str[3] = {'\0'};

    if(interface_name) {
        strcpy(interface_name, ANET_PDP_IFC_NAME);
        snprintf(cid_str, sizeof(cid_str), "%d", (pdp_cid - 1));
        strncat(interface_name, cid_str, strlen(cid_str));
    }
}

u32 get_mobile_ifc_index(u16 pdp_cid, u8 *ifc_name)
{
    u32 index = NX_MAX_PHYSICAL_INTERFACES;
    u16 i;
    NX_INTERFACE *nx_interface;
    NX_IP *alte_ip_ptr;

    for(i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++) {
        alte_ip_ptr = (NX_IP *)alte_ip.ip_ptr;
        nx_interface = &(alte_ip_ptr->nx_ip_interface[i]);
        if (strcmp(ifc_name, nx_interface->nx_interface_name) == 0) {
            /* Find a valid index. */
            index = i;
            break;
        }
    }

    return index;
}

boolean aic_is_valid_ipv4_addr(char *ipv4_address)
{
    int i = 0, j = 0;
    int cnt = 0;
    int addr_len;
    u8 ipv4_addr[16] = {'\0'};

    if (!ipv4_address)
        return AIC_FALSE;

    addr_len = strlen(ipv4_address);
    if (addr_len > 15)
        return AIC_FALSE;
    strncpy(ipv4_addr, ipv4_address, addr_len);
    for (i = 0; i < addr_len; i++) {
        if (ipv4_addr[i] == '.') {
            ipv4_addr[i] = '\0';
            if (ipv4_addr[j] == '\0' ||
                (ipv4_addr[j] == '0' && strlen(&ipv4_addr[j]) > 1) ||
                !(atoi(&ipv4_addr[j]) <= 255 && atoi(&ipv4_addr[j]) >= 0))
                return AIC_FALSE;

            j = i + 1;
            if (j >= addr_len)
                return AIC_FALSE;
            cnt++;
        } else if (!(ipv4_addr[i] >= '0' && ipv4_addr[i] <= '9')) {
            return AIC_FALSE;
        }
    }

    if (cnt != 3)
        return AIC_FALSE;

    return AIC_TRUE;
}

boolean aic_is_valid_ipv6_addr(ulong *ipv6_addr)
{
    boolean ret = AIC_FALSE;

    if (!ipv6_addr)
        return AIC_FALSE;

    if (!(ipv6_addr[0] == 0 && ipv6_addr[1] == 0))
        return AIC_TRUE;

    return ret;
}

u32 aic_get_ipv6_dns_server(ulong *dns_server_ptr, u32 wan_intf_index)
{
    WAN_NET *wan_net_ptr = AIC_NULL;
    u8 i;

    if (!dns_server_ptr || wan_intf_index >= NX_MAX_PHYSICAL_INTERFACES) {
        return ANET_ERROR_INVALID_IFC_INDEX;
    }

    wan_net_ptr = aic_get_wan_net_by_ifc_index(wan_intf_index);
    if (wan_net_ptr && aic_is_valid_ipv6_addr(wan_net_ptr->wan_ipv6_dns1_address)) {
        for (i = 0; i < 4; i++) {
            dns_server_ptr[i] = wan_net_ptr->wan_ipv6_dns1_address[i];
        }
        return AIC_SUCCESS;
    } else {
        return ANET_ERROR_NO_IPV6_DNS_SERVER;
    }
}

u32 aic_register_pdp_state_change_cb(aic_pdp_state_change_cb cb)
{
    u8 i;

    for (i = 0; i < ANET_MAX_PDP_STATE_CBS;i++) {
        if (!pdp_state_cbs[i]) {
            pdp_state_cbs[i] = cb;
            return AIC_SUCCESS;
        }
    }

    return ANET_ERROR_PDP_CBS_OVERFLOW;
}

void aic_unregister_pdp_state_change_cb(aic_pdp_state_change_cb cb)
{
    u8 i;

    for (i = 0; i < ANET_MAX_PDP_STATE_CBS;i++) {
        if (pdp_state_cbs[i] == cb) {
            pdp_state_cbs[i] = AIC_NULL;
            return;
        }
    }
}

void aic_notify_pdp_state_change(u8 pdp_cid, u8 pdp_state)
{
    u8 i;

    for (i = 0; i < ANET_MAX_PDP_STATE_CBS;i++) {
        if (pdp_state_cbs[i]) {
            pdp_state_cbs[i](pdp_cid, pdp_state);
        }
    }
}

boolean aic_is_valid_pdp_type(u8 *pdp_type_str)
{
    if (!pdp_type_str || strlen(pdp_type_str) < 2 ||
        !strcasestr(pdp_type_str, "IP"))
        return AIC_FALSE;

    /* Ensure it contains "IP" above. */
    if (strlen(pdp_type_str) == 2)
        return AIC_TRUE;

    if (strlen(pdp_type_str) == strlen(ANET_PDP_TYPE_IPV4V6) &&
        strcasestr(pdp_type_str, ANET_PDP_TYPE_IPV4V6))
        return AIC_TRUE;

    if (strlen(pdp_type_str) == strlen(ANET_PDP_TYPE_IPV6) &&
        strcasestr(pdp_type_str, ANET_PDP_TYPE_IPV6))
        return AIC_TRUE;

    return AIC_FALSE;
}

void aic_handle_pdp_deact(u8 cid, boolean is_flight_mode)
{
    if (aic_get_flight_mode() == ANET_IN_FLIGHT_MODE) {
        return;
    }

    /* Pdp is deactivated. */
    aic_set_pdp_deactivated(cid);
    if(!aic_boot_mode_is_basic())
    {
#ifdef CONFIG_USB_NET_SHARE
        aic_usbtether_pdp_deactivated(cid);
#endif

#ifdef CONFIG_PPP_DIAL
        if (cid == aic_get_ppp_dial_cid())
            aos_set_event(event_net_share, NET_EVENT_STOP_PPP_DIAL, AOS_OR);
#endif
    }

    aic_notify_pdp_state_change(cid, ANET_PDP_DEACTIVATED);
}

/*boolean is_non_default_pdp_acted(void)
{
    u8 i;

    aos_get_mutex(pdp_info_mutex, AOS_WAIT_FOREVER);
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        if (net_pdp_info_list[i].pdp_cid != ANET_DEFAULT_PDP_CID &&
            net_pdp_info_list[i].pdp_activated == ANET_PDP_ACTIVATED) {
            aos_put_mutex(pdp_info_mutex);
            return AIC_TRUE;
        }
    }
    aos_put_mutex(pdp_info_mutex);

    return AIC_FALSE;
}*/

u8 aic_get_pdp_ip_type(u8 cid)
{
    WAN_NET *wan_net_ptr = AIC_NULL;
    u8 ip_type = ANET_DEFAULT_IP_TYPE_NONE;

    wan_net_ptr = aic_get_wan_net_per_pdp(cid);
    if (!wan_net_ptr || wan_net_ptr->pdp_activated != ANET_PDP_ACTIVATED)
        return ANET_DEFAULT_IP_TYPE_NONE;

    if (wan_net_ptr->wan_ipv4_address != 0) {
        if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_local_ipv6_addr))
            ip_type = ANET_DEFAULT_IP_TYPE_V4V6;
        else
            ip_type = ANET_DEFAULT_IP_TYPE_V4;
    } else {
        if (aic_is_valid_ipv6_addr(wan_net_ptr->wan_local_ipv6_addr))
            ip_type = ANET_DEFAULT_IP_TYPE_V6;
        else
            ip_type = ANET_DEFAULT_IP_TYPE_NONE;
    }

    return ip_type;
}

/** URC is Unsolicited Result Code from CP.
 * We handle the +CGEV URC about PDP activation and deactivation.
 * For example:
 * +CGEV: ME DEACT <PDP_type>,<PDP_addr>,[<cid>]
 * +CGEV: NW DEACT <PDP_type>,<PDP_addr>,[<cid>]
 * +CGEV: ME PDN DEACT <cid>
 * +CGEV: NW PDN DEACT <cid>
 * +CGEV: ME PDN ACT <cid>
 * +CGEV: NW PDN ACT <cid>
 */
u32 handle_at_urc_cgev(u8 *urc_str, u8 *start_str)
{
    u16 cid = 0xff, paras_cnt = 0;
    u32 status = AIC_SUCCESS;
    ulong *paras_arr;
    u8 *tmp_ptr;
    u8 pdp_state = ANET_PDP_ACTIVATED;
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    u8 mipcall_urc[30] = {'\0'};
#endif

    tmp_ptr = urc_str;
    /* PDP is deactivated passively */
    if (strcasestr(urc_str, "NW PDN DEACT ") ||
        strcasestr(urc_str, "ME PDN DEACT ")) {
        tmp_ptr += strlen("+CGEV: NW PDN DEACT ");
        cid = atoi(tmp_ptr);
        pdp_state = ANET_PDP_DEACTIVATED;
    } else if (strcasestr(urc_str, "NW DEACT ") ||
               strcasestr(urc_str, "ME DEACT ")) {
        tmp_ptr += strlen("+CGEV: NW ");
        /* Get all paras after +CGEV:,cid is the third para. */
        paras_arr = aic_split_paras_to_str_array(tmp_ptr, "DEACT ", &paras_cnt);
        if (paras_arr && (paras_cnt == 3)) {
            cid = atoi((u8 *)paras_arr[2]);
        }
        aic_free_paras_mem(paras_arr, paras_cnt);
        pdp_state = ANET_PDP_DEACTIVATED;
    } else if (strcasestr(urc_str, "NW PDN ACT ") ||
               strcasestr(urc_str, "ME PDN ACT ")) {
        tmp_ptr += strlen("+CGEV: NW PDN ACT ");
        cid = atoi(tmp_ptr);
        pdp_state = ANET_PDP_ACTIVATED;
    }

    if (!aic_is_valid_pdp_cid(cid))
        return AIC_SUCCESS;

#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    if (pdp_state == ANET_PDP_DEACTIVATED && mipcall_cfg[cid - 1].urc_via_cgev &&
        mipcall_cfg[cid - 1].act_by_mipcall && !mipcall_cfg[cid - 1].act_as_ims &&
        !aic_is_powering_off()) {
        mipcall_cfg[cid - 1].act_by_mipcall = AIC_FALSE;
        snprintf(mipcall_urc, sizeof(mipcall_urc), "+MIPCALL: %d,0", cid);
        if (conn_get_uart_at_conid != AIC_ENODEV)
            aic_transmit_at_cmd_urc(conn_get_uart_at_conid, mipcall_urc, strlen(mipcall_urc));

        if (conn_get_usb_at_conid != AIC_ENODEV)
            aic_transmit_at_cmd_urc(AT_USB_AT_PORT_ID, mipcall_urc, strlen(mipcall_urc));
    } else if (pdp_state == ANET_PDP_ACTIVATED) {
        mipcall_cfg[cid - 1].urc_via_cgev = AIC_TRUE;
        if (aic_is_ims_pdp_cid(cid))
            mipcall_cfg[cid - 1].act_as_ims = AIC_TRUE;
        else
            mipcall_cfg[cid - 1].act_as_ims = AIC_FALSE;
    }
#endif
    if (pdp_state == ANET_PDP_DEACTIVATED && aic_is_pdp_activated(cid)) {
        aic_handle_pdp_deact(cid, AIC_FALSE);
    } else if (pdp_state == ANET_PDP_ACTIVATED && !aic_is_ims_pdp_cid(cid) && !aic_is_pdp_activated(cid)) {
        status = aic_send_cgcontrdp_set_cmd(cid);
    }

    return status;
}

void end_waiting_pdp_act(u8 pdp_cid)
{
#ifdef CONFIG_USB_NET_SHARE
    USB_TETHER_NET_INFO *usb_net = AIC_NULL;

    usb_net = aic_get_usb_net_waiting_pdp(pdp_cid);
    if (usb_net) {
        aicx_info(0x0eabc9, "", "set usb share not waiting for pdp\n");
        usb_net->waiting_pdp_act = ANET_NOT_WAIT_PDP_ACTIVATE;
    }
#endif

#ifdef CONFIG_PPP_DIAL
    if (ppp_dial_net_info.wan_net_id == pdp_cid &&
        ppp_dial_net_info.waiting_pdp_act)
        ppp_dial_net_info.waiting_pdp_act = ANET_NOT_WAIT_PDP_ACTIVATE;
#endif
}

/* Get paras of AT+CGACT cmd. */
static u32 get_cgact_exe_paras(AT_REQ_RESP *at_to_ap, u8 *pdp_cid, u8 *pdp_act)
{
    ulong *para_arr;
    u16 paras_count;
    u32 status = AIC_SUCCESS;

    para_arr = aic_split_paras_to_str_array(at_to_ap->at_req,
                                            "AT+CGACT=",
                                            &paras_count);
    if (para_arr && paras_count == 2) {
        *pdp_act = atoi((u8 *)para_arr[0]);
        *pdp_cid = atoi((u8 *)para_arr[1]);
    } else {
        status = AT_ERROR_INVALID_PARAS;
    }
    aic_free_paras_mem(para_arr, paras_count);

    if (!aic_is_valid_pdp_cid(*pdp_cid)) {
        status = AT_ERROR_INVALID_PARAS;
    }

    return status;
}

/** The response may be OK, or ERROR or +CME ERROR:<err> for CGACT exe cmd.
 * Perhaps several lines, including +CGEV intermediate result code.
 */
s32 aic_handle_cgact_exe_resp(AT_REQ_RESP *at_to_ap)
{
    u8 cid = 0xff, pdp_act = 0xff;
    u32 status;

    status = get_cgact_exe_paras(at_to_ap, &cid, &pdp_act);
    if (status) {
        aicx_info(0x144f7d, "", "cgact resp:get no paras\n");
        return status;
    }

    /* The cmd is deactivating cmd. */
    if (pdp_act == 0) {
        /* Send "AT+CGACT?" to check if the pdp is actually deactivated or not. */
        if (strcasestr(at_to_ap->at_resp, AT_RESULT_OK))
            aic_send_cmd_with_callback("AT+CGACT?", AIC_NULL);
    } else if (pdp_act == 1) {
        /** For CGACT activating cmd AT+CGACT=1,cid.If OK,then get ip addrs.
         * There may be +CGEV intermediate result code.
         */
        if (!strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
            if (!aic_is_pdp_activated(cid))
                aic_send_cgcontrdp_set_cmd(cid);
        } else {
            if(!aic_boot_mode_is_basic())
                end_waiting_pdp_act(cid);
        }
    }

    return status;
}

/** Request:AT+CGACT?
 * Response:
 * +CGACT:<cid>,<state>
 * +CGACT:<cid>,<state>
 * [...]
 * OK
 */
s32 aic_handle_cgact_query_resp(AT_REQ_RESP *at_to_ap)
{
    u8 *tmp_ptr;
    u16 cid = 0xff, pdp_state = 0xffff;
    u16 i, paras_count;
    u8 line_cnt;
    ulong *line_arr, *para_arr;
    WAN_NET *wan_net;

    if (at_to_ap->is_external_cmd == AT_FROM_EXTERNAL ||
        strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR)) {
        return AIC_SUCCESS;
    }

    line_cnt = at_to_ap->resp_line_count;
    line_arr = aic_get_each_line_from_resp(at_to_ap->at_resp, line_cnt);
    if (!line_arr)
        return AIC_ENOMEM;

    tmp_ptr = strcasestr((u8 *)line_arr[line_cnt - 1], "OK");
    if (tmp_ptr) {
        /* The result is OK,then check the states of all cids. */
        for (i = 0; i < (line_cnt - 1); i++) {
            para_arr = aic_split_paras_to_str_array((u8 *)line_arr[i],
                                                    "+CGACT: ", &paras_count);
            if (!para_arr || paras_count != 2) {
                aic_free_paras_mem(para_arr, paras_count);
                continue;
            }

            cid = atoi((u8 *)para_arr[0]);
            pdp_state = atoi((u8 *)para_arr[1]);
            aic_free_paras_mem(para_arr, paras_count);
            if (aic_is_ims_pdp_cid(cid))
                continue;

            /** Just handle pdp deactivated here. Handle pdp activated
             * when receive response of AT+CGCONTRDP.
             */
            wan_net = aic_get_wan_net_per_pdp(cid);
            if (wan_net) {
                if (pdp_state == wan_net->pdp_activated)
                    continue;
                else if (pdp_state == ANET_PDP_DEACTIVATED)
                    aic_handle_pdp_deact(cid, AIC_FALSE);
            } else {
                if (pdp_state == ANET_PDP_ACTIVATED)
                    aic_send_cgcontrdp_set_cmd(cid);
            }
        }
    }

    aic_free_paras_mem(line_arr, line_cnt);
    return AIC_SUCCESS;
}

u8 get_cid_from_cgcontrdp_cmd(u8 *at_cmd_str)
{
    u8 cid = 0xff;
    u16 paras_count = 0;
    ulong *paras_arr;

    if (!at_cmd_str)
        return cid;

    paras_arr = aic_split_paras_to_str_array(at_cmd_str,
                                             "AT+CGCONTRDP=",
                                             &paras_count);
    if (paras_count == 1)
        cid = atoi((u8 *)paras_arr[0]);

    aic_free_paras_mem(paras_arr, paras_count);

    return cid;
}

/** Query ip and dns addresses via AT+CGCONTRDP=<cid>.
 * +CGCONTRDP:<cid>,<bearer_id>,<apn>,<local_addr and subnet_mask>,
 * <gw_addr>,<DNS_prim_addr>,<DNS_second_addr>
 * [...]
 * OK
 */
s32 aic_handle_cgcontrdp_set_resp(AT_REQ_RESP *at_to_ap)
{
    u8 ipv4_addr[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};
    u8 ipv4_prim_dns[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};
    u8 ipv4_sec_dns[ANET_MAX_LEN_IPV4_ADDR] = {'\0'};
    u8 ipv6_suffix[ANET_MAX_LEN_DEC_IPV6_ADDR] = {'\0'};
    u8 ipv6_prim_dns[ANET_MAX_LEN_DEC_IPV6_ADDR] = {'\0'};
    u8 ipv6_sec_dns[ANET_MAX_LEN_DEC_IPV6_ADDR] = {'\0'};
    u8 cid = 0, line_cnt, ipv6_format = ANET_IPV6_FORMAT_DECIMAL;
    u16 i, paras_count;
    ulong *line_arr, *para_arr;
    boolean is_ipv4;
    u32 ret;
    u32 old_pri;

    line_cnt = at_to_ap->resp_line_count;
    if (at_to_ap->is_external_cmd == AT_FROM_EXTERNAL || line_cnt <= 1 ||
        strcasestr(at_to_ap->at_resp, AT_RESULT_ERROR))
        return AIC_SUCCESS;

    cid = get_cid_from_cgcontrdp_cmd(at_to_ap->at_req);
#if CONFIG_CUSTOMER_CM_AT || CONFIG_GX_AT_V2
    if (!mipcall_cfg[cid - 1].mipcall_operation && !init_net_cfg.net_cfg.auto_cfg_pdp_intf) {
        aicx_info(0x1a1ba7, "", "Not config pdp intf due to autoconn 0\n");
        return AIC_SUCCESS;
    }
#endif

    if(!aic_boot_mode_is_basic())
        end_waiting_pdp_act(cid);

    line_arr = aic_get_each_line_from_resp(at_to_ap->at_resp, line_cnt);
    if (!line_arr)
        return AIC_ENOMEM;

    /* The result is OK,then check the states of all cids. */
    for (i = 0; i < line_cnt - 1; i++) {
        para_arr = aic_split_paras_to_str_array((u8 *)line_arr[i],
                                                 "+CGCONTRDP: ",
                                                 &paras_count);
        if (!para_arr)
            continue;

        /** +CGCONTRDP:<cid>,<bearer_id>,<apn>,<local_addr and subnet_mask>,
         * <gw_addr>,<DNS_prim_addr>,<DNS_second_addr>
         */
        cid = atoi((u8 *)para_arr[0]);
        if (cid == ANET_DEFAULT_PDP_CID && paras_count >= 3) {
            aos_memset(reg_service_info.default_apn_name, 0, ANET_APN_NAME_MAX_LEN);
            strncpy(reg_service_info.default_apn_name, (u8 *)para_arr[2],
                    (ANET_APN_NAME_MAX_LEN - 1));
        }

        if (paras_count >= 6) {
            is_ipv4 = is_ipv4_addr((u8 *)para_arr[3]);
            if (is_ipv4) {
                parse_ipv4_addr(ipv4_addr, (u8 *)para_arr[3]);
                if (strlen((u8 *)para_arr[5]) > 0) {
                    strcpy(ipv4_prim_dns, (u8 *)para_arr[5]);
                } else {
                    strcpy(ipv4_prim_dns, ANET_DEFAULT_IPV4_DNS_SERVER1);
                }
            } else {
                if (strchr((u8 *)para_arr[3], ':'))
                    ipv6_format = ANET_IPV6_FORMAT_HEX;
                else
                    ipv6_format = ANET_IPV6_FORMAT_DECIMAL;

                parse_ipv6_suffix(ipv6_suffix, (u8 *)para_arr[3], ipv6_format);
                if (strlen((u8 *)para_arr[5]) > 0)
                    strncpy(ipv6_prim_dns, (u8 *)para_arr[5], (sizeof(ipv6_prim_dns) - 1));
            }

            /* There is secondary DNS addr. */
            if (paras_count >= 7) {
                if (is_ipv4) {
                    if (strlen((u8 *)para_arr[6]) > 0) {
                        strncpy(ipv4_sec_dns, (u8 *)para_arr[6], (ANET_MAX_LEN_IPV4_ADDR - 1));
                    } else {
                        strcpy(ipv4_sec_dns, ANET_DEFAULT_IPV4_DNS_SERVER2);
                    }
                } else {
                    if (strlen((u8 *)para_arr[6]) > 0)
                        strncpy(ipv6_sec_dns, (u8 *)para_arr[6], (sizeof(ipv6_sec_dns) - 1));
                }
            }
        }

        aic_free_paras_mem(para_arr, paras_count);
    }

    aic_free_paras_mem(line_arr, line_cnt);
    /* Parsing paras succeed. */
    if (aic_is_valid_pdp_cid(cid) && !aic_is_pdp_activated(cid)) {
        /* Change priority in case receive and handle RA when local ipv6
         * addr hasn't been set. */
        aos_change_thread_pri(thread_at_rev_from_cp,
                              ALTE_TASK_HIGH_PRIORITY, &old_pri);
        /* Init pdp info,wan net,and set interface of IP instance. */
        ret = aic_set_pdp_activated(cid, ipv4_addr, ipv4_prim_dns, ipv4_sec_dns,
                                    ipv6_suffix, ipv6_prim_dns, ipv6_sec_dns, ipv6_format);
        aos_change_thread_pri(thread_at_rev_from_cp,
                              AT_REV_FROM_CP_TASK_PRIO, &old_pri);

        if (!ret) {
#if (CONFIG_PDP_MGR) && (CONFIG_NX_DNS)
            if (cid == ANET_DEFAULT_PDP_CID)
                aic_init_dns_client();
#endif
            /* Notify the callbacks registered before. */
            aic_notify_pdp_state_change(cid, ANET_PDP_ACTIVATED);

#ifdef CONFIG_USB_NET_SHARE
            /* Can't get the usb intf here. */
            if(!aic_boot_mode_is_basic())
            {
                aic_notify_usbtether_pdp_activated(cid);
                if (cid == ANET_DEFAULT_PDP_CID)
                    aos_set_event(event_net_share, NET_EVENT_AUTO_USB_TETHER, AOS_OR);
            }
#endif

#ifdef CONFIG_SOFTAP_SHARE
            if (cid == ANET_DEFAULT_PDP_CID)
                aic_notify_softap_pdp_activated();
#endif
        }
    }

    return AIC_SUCCESS;
}

/**
 * Just get ipv4 addr from the response of AT+CGCONTRDP,ignore subnet mask.
 * IPv4:xx.xx.xx.xx.xx.xx.xx.xx
 * IPv6:xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.
 *      xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx
 */
void parse_ipv4_addr(u8 *ipv4_addr, u8 *ip_str)
{
    u8 *tmp_ptr;
    u8 count = 0, len = 0;

    if (!ip_str) {
        return;
    }

    /* IPv4 addr is the leading 4 numbers. */
    tmp_ptr = ip_str;
    while (*tmp_ptr) {
        if (*tmp_ptr == '.') {
            count++;
            if (count == 4) {
                break;
            }
        }
        tmp_ptr++;
    }

    len = tmp_ptr - ip_str;
    strncpy(ipv4_addr, ip_str, len);

    return;
}

/** IPv4 and subnet mask:xx.xx.xx.xx.xx.xx.xx.xx
 * IPv6 and subnet mask:xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.
 *                      xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx
 */
boolean is_ipv4_addr(u8 *ip_addr)
{
    //u8 dot_count = 0;
    boolean is_ipv4 = AIC_TRUE;

    if (strchr(ip_addr, ':') || strlen(ip_addr) > (ANET_MAX_LEN_IPV4_ADDR * 2)) {
        is_ipv4 = AIC_FALSE;
    }

    return is_ipv4;
}

/** Just get ipv6 suffix from the response of AT+CGCONTRDP,
 * ignore prefix and subnet mask.
 * IPv6:xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.
 *      xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx.xx
 */
void parse_ipv6_suffix(u8 *ipv6_suffix, u8 *ip_str, u8 ipv6_format)
{
    u8 *tmp_ptr = AIC_NULL, *start_ptr;
    u8 count = 0, len, i;
    u32 ipv6_numeric[4] = {0};
    u8 ipv6_str[ANET_MAX_LEN_HEX_IPV6_ADDR] = {0};

    if (!ip_str)
        return;

    /* Two formats of local addr and subnet mask:
     * 0:0:0:0:2:3:456:789 ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff
     * 0:0:0:0:2:3:456:789/128 */
    if (ipv6_format == ANET_IPV6_FORMAT_HEX) {
        tmp_ptr = strchr(ip_str, ' ');
        if (!tmp_ptr)
            tmp_ptr = strchr(ip_str, '/');

        if (tmp_ptr && (tmp_ptr - ip_str))
            aos_memcpy(ipv6_str, ip_str, (tmp_ptr - ip_str));
        else
            return;

        if (strlen(ipv6_str)) {
            inet_pton(AF_INET6, ipv6_str, ipv6_numeric);
            tmp_ptr = ipv6_suffix;
            /* The last two u32 is ipv6 suffix. */
            for (i = 4; i < 8; i++) {
                snprintf(tmp_ptr, 5, "%04x", NX_CHANGE_USHORT_ENDIAN(*((u16 *)ipv6_numeric + i)));
                tmp_ptr += 4;
                if (i != 7)
                    *tmp_ptr = ':';

                tmp_ptr++;
            }
        }
    } else {
        /* IPv6 suffix is the 9th to the 16th numbers. */
        tmp_ptr = ip_str;
        start_ptr = ip_str;

        while (*tmp_ptr) {
            if (*tmp_ptr == '.') {
                count++;
                if (count == 8) {
                    start_ptr = tmp_ptr + 1;
                }
                if (count == 16) {
                    break;
                }
            }
            tmp_ptr++;
        }

        len = tmp_ptr - start_ptr;
        if (len < ANET_MAX_LEN_DEC_IPV6_ADDR)
            strncpy(ipv6_suffix, start_ptr, len);
    }

    return;
}

void aic_deact_all_pdp(boolean is_flight_mode)
{
    u8 i, cid;

    /* Set all PDP to deactivated state. */
    for (i = 0; i < ANET_PDP_MAX_NUM; i++) {
        cid = wan_net_list[i].wan_net_id;
        if (aic_is_valid_pdp_cid(cid)) {
            aic_handle_pdp_deact(cid, is_flight_mode);
        }
    }
}

/* Get and update all pdp states in order to keep up with CP. */
void aic_check_all_pdp_state(void)
{
    aic_send_cmd_with_callback("AT+CGACT?", AIC_NULL);
}

/* Return starting pointer of all wan net. Demo:
 * WAN_NET *wan_net;
 * wan_net = aic_get_all_wan_net();
 * for (i = 0; i < aic_get_max_wan_net_num(); i++) {
 *     if (wan_net->pdp_activated == ANET_PDP_ACTIVATED) {
 *         //To get ip addr from wan_net
 *     }
 *     wan_net++;
 * }
 */
WAN_NET *aic_get_all_wan_net(void)
{
    return &wan_net_list[0];
}

u8 aic_get_max_wan_net_num(void)
{
    return ANET_PDP_MAX_NUM;
}

u8 aic_get_pdp_intf_index(u8 pdp_cid)
{
    WAN_NET *wan_net_ptr;

    wan_net_ptr = aic_get_wan_net_per_pdp(pdp_cid);
    if (wan_net_ptr)
        return wan_net_ptr->wan_ifc_index;
    else
        return 0xff;
}

static boolean aic_check_apn_matched(APN *apn_ptr, u8 *sim_imsi, u8 *sim_iccid)
{
    u16 imsi_len = 0, iccid_cmp_len;

    if (!apn_ptr || !sim_imsi)
        return AIC_FALSE;

    if (apn_ptr->imsi)
        imsi_len = strlen(apn_ptr->imsi);

    if (imsi_len && strncasecmp(apn_ptr->imsi, sim_imsi, imsi_len) == 0) {
        if (!apn_ptr->iccid || strlen(apn_ptr->iccid) == 0)
            return AIC_TRUE;

        iccid_cmp_len = strlen(apn_ptr->iccid);
        if (strncasecmp(apn_ptr->iccid, sim_iccid, iccid_cmp_len) == 0)
            return AIC_TRUE;
    }

    return AIC_FALSE;
}

static u32 aic_read_all_apns_by_imsi(u8 *matched_imsi, u8 *matched_iccid)
{
    u16 i = 0;
    APN *apn_ptr;

    if (!matched_imsi || strlen(matched_imsi) == 0) {
        alog_error("error,no imsi str\n");
        return ANET_ERROR_APN_NOT_FOUND;
    }

    aos_memset(all_apns_per_imsi, 0, sizeof(all_apns_per_imsi));
    apn_ptr = &apns_table[0];
    while (apn_ptr && apn_ptr->imsi) {
        if (aic_check_apn_matched(apn_ptr, matched_imsi, matched_iccid)) {
            all_apns_per_imsi[i] = apn_ptr;
            i++;
            if (i >= ANET_APN_CNT_PER_IMSI) {
                alog_error("read all apn error,list full\n");
                break;
            }
        }

        apn_ptr++;
    };

    if (i == 0)
        return ANET_ERROR_APN_NOT_FOUND;
    else
        return AIC_SUCCESS;
}

/* If APN is set by user,then try the apn first. */
u32 aic_set_apn_by_user(APN *apn_ptr)
{
    u8 i = 0;

    if (apn_ptr && apn_ptr->imsi) {
        alog_info("set apn by user for imsi %s\n", apn_ptr->imsi);
        for (i = 0; i < ANET_APN_CNT_USER_SET; i++) {
            if (!apns_set_by_user[i].imsi) {
                apns_set_by_user[i].imsi = aos_zalloc(strlen(apn_ptr->imsi) + 1);
                strcpy(apns_set_by_user[i].imsi, apn_ptr->imsi);

                if (apn_ptr->apn) {
                    apns_set_by_user[i].apn = aos_zalloc(strlen(apn_ptr->apn) + 1);
                    strcpy(apns_set_by_user[i].apn, apn_ptr->apn);
                }

                if (apn_ptr->userid) {
                    apns_set_by_user[i].userid = aos_zalloc(strlen(apn_ptr->userid) + 1);
                    strcpy(apns_set_by_user[i].userid, apn_ptr->userid);
                }

                if (apn_ptr->password) {
                    apns_set_by_user[i].password = aos_zalloc(strlen(apn_ptr->password) + 1);
                    strcpy(apns_set_by_user[i].password, apn_ptr->password);
                }

                apns_set_by_user[i].auth_prot = apn_ptr->auth_prot;
                if (apn_ptr->iccid) {
                    apns_set_by_user[i].iccid = aos_zalloc(strlen(apn_ptr->iccid) + 1);
                    strcpy(apns_set_by_user[i].iccid, apn_ptr->iccid);
                }

                break;
            }
        }
    }

    if (!apn_ptr || !apn_ptr->imsi || i == ANET_APN_CNT_USER_SET)
        return ANET_ERROR_SET_APN_FAIL;

    return AIC_SUCCESS;
}

u32 get_full_imsi_sync(u8 *imsi_ptr)
{
    ulong *line_arr;
    u32 ret = ANET_ERROR_GET_SIM_INFO_FAIL;
    AT_RESP_FROM_CP resp_to_save;

    resp_to_save.resp_buf = aos_zalloc_relaxed(AT_MAX_AP_RESP_LENGTH);
    if (!resp_to_save.resp_buf)
        return ret;

    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait("AT+CIMI", &resp_to_save);
    if (resp_to_save.resp_line_cnt > 1) {
        line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (line_arr) {
            snprintf(imsi_ptr, ANET_FULL_IMSI_LEN, "%s", (u8 *)line_arr[0]);
            ret = AIC_SUCCESS;
        }

        aic_free_paras_mem(line_arr, resp_to_save.resp_line_cnt);
    }

    aos_free(resp_to_save.resp_buf);
    return ret;
}

u32 get_sim_iccid_sync(u8 *iccid_ptr)
{
    ulong *line_arr;
    u32 ret = ANET_ERROR_GET_SIM_INFO_FAIL;
    AT_RESP_FROM_CP resp_to_save;
    u8 *tmp_ptr;

    resp_to_save.resp_buf = aos_zalloc_relaxed(AT_MAX_AP_RESP_LENGTH);
    if (!resp_to_save.resp_buf)
        return ret;

    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait("AT^DCID", &resp_to_save);
    if (resp_to_save.resp_line_cnt > 1) {
        line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (line_arr) {
            tmp_ptr = strcasestr((u8 *)line_arr[0], "^DCID: ");
            if (tmp_ptr) {
                snprintf(iccid_ptr, ANET_FULL_ICCID_LEN, "%s", (tmp_ptr + strlen("^DCID: ")));
                alog_info("get iccid %s\n", iccid_ptr);
                ret = AIC_SUCCESS;
            }
        }

        aic_free_paras_mem(line_arr, resp_to_save.resp_line_cnt);
    }

    aos_free(resp_to_save.resp_buf);
    return ret;
}

u8 query_cereg_state_sync(void)
{
    u16 paras_cnt = 0;
    ulong *line_arr, *paras_arr;
    u8 cereg_state = 0;
    AT_RESP_FROM_CP resp_to_save;

    resp_to_save.resp_buf = aos_zalloc_relaxed(AT_MAX_AP_RESP_LENGTH);
    if (!resp_to_save.resp_buf)
        return cereg_state;

    resp_to_save.buf_len = AT_MAX_AP_RESP_LENGTH;
    aic_send_cmd_and_wait("AT+CEREG?", &resp_to_save);
    if (resp_to_save.resp_line_cnt > 1) {
        line_arr = aic_get_each_line_from_resp(resp_to_save.resp_buf, resp_to_save.resp_line_cnt);
        if (line_arr) {
            paras_arr = aic_split_paras_to_str_array((u8 *)line_arr[0], "+CEREG: ", &paras_cnt);
            if (paras_arr && paras_cnt >= 2)
                cereg_state = atoi((u8 *)paras_arr[1]);

            aic_free_paras_mem(paras_arr, paras_cnt);
        }

        aic_free_paras_mem(line_arr, resp_to_save.resp_line_cnt);
    }

    aos_free(resp_to_save.resp_buf);
    return cereg_state;
}

static u8 set_apn_and_restart_radio(APN *apn_ptr)
{
    u8 i = 0;
    static u8 try_cnt = 3;
    u8 cereg_state;
    u8 at_cmd_str[80] = {0};

    aic_send_cmd_and_wait("AT+CFUN=4", AIC_NULL);
    aos_memset(at_cmd_str, 0, sizeof(at_cmd_str));
    snprintf(at_cmd_str, sizeof(at_cmd_str), "AT+CGDCONT=1,\"IPV4V6\",\"%s\"", apn_ptr->apn);
    aic_send_cmd_and_wait(at_cmd_str, AIC_NULL);
    aos_memset(at_cmd_str, 0, sizeof(at_cmd_str));
    snprintf(at_cmd_str, sizeof(at_cmd_str), "AT+CGAUTH=1,%d,\"%s\",\"%s\"",
             apn_ptr->auth_prot, apn_ptr->userid, apn_ptr->password);
    aic_send_cmd_and_wait(at_cmd_str, AIC_NULL);
    aic_send_cmd_and_wait("AT+CFUN=1", AIC_NULL);

    do {
        aos_sleep(1000);
        cereg_state = query_cereg_state_sync();
        alog_info("set_apn_and_restart_radio cereg_state = %d", cereg_state);

        i++;
        if (i > try_cnt) {
            try_cnt++;
            if (try_cnt > 8)
                try_cnt = 3;

            break;
        }
    } while (cereg_state == 2);

    return cereg_state;
}

void aic_try_all_apns_by_imsi(void)
{
    u8 i, len, cereg_state = 0;
    u8 imsi_to_look_up[ANET_FULL_IMSI_LEN] = {0};
    u8 sim_iccid[ANET_FULL_ICCID_LEN] = {0};

    /* Get imsi first. */
    get_full_imsi_sync(imsi_to_look_up);
    if (strlen(imsi_to_look_up) == 0) {
        alog_error("try apn error,no imsi\n");
        return;
    }

    get_sim_iccid_sync(sim_iccid);
    alog_info("try apn for imsi %s\n", imsi_to_look_up);
    try_apn_in_process = AIC_TRUE;
    for (i = 0; i < ANET_APN_CNT_USER_SET; i++) {
        if (!apns_set_by_user[i].imsi)
            break;

        len = strlen(apns_set_by_user[i].imsi);
        if (len == 0)
            break;

        if (aic_check_apn_matched(&apns_set_by_user[i], imsi_to_look_up, sim_iccid)) {
            cereg_state = set_apn_and_restart_radio(&apns_set_by_user[i]);
            if (cereg_state != 3)
                goto TRY_DONE;
        }
    }

    aic_read_all_apns_by_imsi(imsi_to_look_up, sim_iccid);
    for (i = 0; i < ANET_APN_CNT_PER_IMSI; i++) {
        if (!all_apns_per_imsi[i] || !all_apns_per_imsi[i]->imsi)
            break;

        len = strlen(all_apns_per_imsi[i]->imsi);
        if (len == 0)
            break;

        cereg_state = set_apn_and_restart_radio(all_apns_per_imsi[i]);
        if (cereg_state != 3)
            goto TRY_DONE;
    }

TRY_DONE:
    alog_info("try all apn done,state %d\n", cereg_state);
    try_apn_in_process = AIC_FALSE;
    try_apn_done = AIC_TRUE;
    return;
}

boolean aic_need_try_saved_apns(void)
{
    alog_info("try_apn_in_process %d,try_apn_done %d\n", try_apn_in_process, try_apn_done);
    if (!try_apn_in_process && !try_apn_done)
        return AIC_TRUE;

    return AIC_FALSE;
}

/* Start the pdp init function. */
DIS_CHARGE_APP_P2_INIT_EXPORT(aic_pdp_mgr_init);

