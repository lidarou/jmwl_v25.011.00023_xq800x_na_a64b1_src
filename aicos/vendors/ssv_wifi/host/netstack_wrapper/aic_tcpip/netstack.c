/*
*  (C) Copyright 2014-2016 Shenzhen South Silicon Valley microelectronics co.,limited
*
*  All Rights Reserved
*/
    
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x18e7ee
#define LOG_MODULE    AMOD_VENDOR

#include "porting.h"
#include "host_config.h"
#include "ssv_frame.h"

#include "rtos.h"
#include <host_apis.h>
#include <net_mgr.h>
#include <netstack.h>
#include <ctype.h>
#include <nx_api.h>

#include <aic_ipv6_forward.h>
#include <aic_osa_ex.h>
#include <aic_net_api.h>
#include <aic_packet_management.h>
#include <aic_network_pcap.h>

#include <aic_net_share_common.h>

#define NX_ETHERNET_ARP  0x0806
#define NX_ETHERNET_RARP 0x0835
#define NX_ETHERNET_IP   0x0800
#define NX_ETHERNET_IPV6 0x86DD
//extern int aic_wcn_network_pcap_dump(u8 *packet_ptr, u32 packet_len);


#if CONFIG_STATUS_CHECK
extern ssv_type_u32 g_l2_tx_packets;
extern ssv_type_u32 g_l2_tx_copy;
extern ssv_type_u32 g_notpool;
extern ssv_type_u32 g_dump_tx;
extern ssv_type_u32 g_tx_allocate_fail;
#endif

#define SSV_NETIF_NAME_STA "ssv-sta"
#define SSV_NETIF_NAME_AP "ssv-ap"
#define SSV_NETIF_MAX 2
#define SSV_ETH_MTU 1514

static u32 wifi_netid[SSV_NETIF_MAX] = {0xffffffff, 0xffffffff};

struct eth_hdr
{
    u8 dest[ETH_ALEN];
    u8 src[ETH_ALEN];
    u16 type;
}__attribute__((packed));

struct ip_hdr {
    /* version / header length */
    u8 _v_hl;

    /* type of service */
    u8 _tos;
    /* total length */
    u16 _len;
    /* identification */
    u16 _id;
    /* fragment offset field */
    u16 _offset;
    /* time to live */
    u8 _ttl;
    /* protocol*/
    u8 _proto;
    /* checksum */
    u16 _chksum;
    /* source and destination IP addresses */
    u32 src;
    u32 dest;
}__attribute__((packed));

struct eth_addr {
  u8 addr[ETH_ALEN];
}__attribute__((packed));

struct ip_addr2 {
  u16 addrw[2];
}__attribute__((packed));

struct etharp_hdr {
    u16 hwtype;
    u16 proto;
    u8  hwlen;
    u8  protolen;
    u16 opcode;
    struct eth_addr shwaddr;
    struct ip_addr2 sipaddr;
    struct eth_addr dhwaddr;
    struct ip_addr2 dipaddr;
}__attribute__((packed));

u32 netdev_get_wifi_netid(int index)
{
    if (index >= SSV_NETIF_MAX)
    {
        return 0xffffffff;
    }

    return wifi_netid[index];
}

int netstack_input(void *data, ssv_type_u32 len, ssv_type_u8 vif_idx)
{
    NX_PACKET *pkt;
    int ret = 0;
    static int count=0;
    struct eth_hdr *eth;
    ssv_type_u8 *data_buff = 0;
    struct aic_network_ip_instance *aic_ip_ptr;

    drv_logd("netstack_input tcpip prt=%x, len %d, calc ptr %x, calc len=%d, wifi_netid=%x\n",
             data, len, OS_FRAME_GET_DATA(data), OS_FRAME_GET_DATA_LEN(data),
             wifi_netid[vif_idx]);
    if (!data || len == 0 || (vif_idx >= SSV_NETIF_MAX)) {
        return -1;
    }

    if (wifi_netid[vif_idx] == 0xffffffff) {
        os_frame_free(data);
        return 0;
    }

    data_buff = OS_FRAME_GET_DATA(data);

    eth = (struct eth_hdr *)data_buff;
    if ((eth->type != htons(0x0806U)) && (eth->type != htons(0x0800U)) && 
        (eth->type != htons(0x86ddU))) {
        drv_logi("%s, eth->type=%02x, len=%d\r\n",__func__, eth->type, len);
        os_frame_free(data);
        return 0;
    }

    if (eth->type == htons(0x0800U)) {
        struct ip_hdr *iphdr = (struct ip_hdr *)((char *)eth + sizeof(struct eth_hdr));

        #define IP_PROTO_ICMP 1
        if (iphdr->_proto == IP_PROTO_ICMP) {
            if ((unsigned int)iphdr % 4) {
                ALIGN_ARRAY(4) struct ip_hdr  iphdr1;
                memcpy((void *)&iphdr1, iphdr, sizeof(struct ip_hdr));
                drv_logi("%s, len=%d, ICMP, src=%08x, dst=%08x\r\n", __func__,
                         len, iphdr1.src, iphdr1.dest);
            } else {
                drv_logi("%s, len=%d, ICMP, src=%08x, dst=%08x\r\n", __func__,
                         len, iphdr->src, iphdr->dest);
            }
        }
    }

    if (eth->type == htons(0x0806U)) {
        ALIGN_ARRAY(8) struct etharp_hdr arphdr;
        memcpy(&arphdr, (char *)eth + sizeof(struct eth_hdr), sizeof(struct etharp_hdr));

        //drv_logi("idle_time=%d\r\n", APStaInfo_getSta_idle_time(0));

        drv_logi("%s, ARP code 0x%x,0x%x,0x%x, [%08x]=>[%08x], [%02x:%02x:%02x:%02x:%02x:%02x]=>[%02x:%02x:%02x:%02x:%02x:%02x]\r\n", __func__, 
                 arphdr.proto, arphdr.protolen, arphdr.opcode,
                 (arphdr.sipaddr.addrw[1] << 16) | (arphdr.sipaddr.addrw[0] << 0), 
                 (arphdr.dipaddr.addrw[1] << 16) | (arphdr.dipaddr.addrw[0] << 0), 
                 arphdr.shwaddr.addr[0], arphdr.shwaddr.addr[1],arphdr.shwaddr.addr[2],
                 arphdr.shwaddr.addr[3], arphdr.shwaddr.addr[4],arphdr.shwaddr.addr[5], 
                 arphdr.dhwaddr.addr[0], arphdr.dhwaddr.addr[1], arphdr.dhwaddr.addr[2], 
                 arphdr.dhwaddr.addr[3], arphdr.dhwaddr.addr[4], arphdr.dhwaddr.addr[5]);
    }

    ret = aic_network_alloc_usb_packet(&pkt, len);
    if (ret) {
        drv_loge("[WCN] alloc packet fail\r\n");
        os_frame_free(data);
        return -1;
    }

    memcpy(pkt->nx_packet_prepend_ptr, data_buff, OS_FRAME_GET_DATA_LEN(data));
    //aic_wcn_network_pcap_dump(data_buff, OS_FRAME_GET_DATA_LEN(data));
    pkt->nx_packet_append_ptr = pkt->nx_packet_prepend_ptr + len;
    pkt->nx_packet_length = len - sizeof(struct eth_hdr);
    pkt->nx_packet_prepend_ptr = pkt->nx_packet_prepend_ptr +
        sizeof(struct eth_hdr);
    pkt->nx_packet_ip_interface = (NX_INTERFACE *)wifi_netid[vif_idx];
#ifdef CONFIG_NET_PCAP_DUMP
    aic_network_pcap_dump(pkt->nx_packet_prepend_ptr,
                          pkt->nx_packet_length,
                          AIC_USB_NIC_FLAG);
#endif

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when input\r\n");
        os_frame_free(data);
        return -1;
    }

    if (eth->type == htons(0x0800U)) {
        pkt->nx_packet_ip_version = NX_IP_VERSION_V4;
        _nx_ip_packet_deferred_receive(aic_ip_ptr->ip_ptr, pkt);
    } else if (eth->type == htons(0x08ddU)) {
        pkt->nx_packet_ip_version = NX_IP_VERSION_V6;
        _nx_ip_packet_deferred_receive(aic_ip_ptr->ip_ptr, pkt);
    } else if (eth->type == htons(0x0806U)) {
        _nx_arp_packet_deferred_receive(aic_ip_ptr->ip_ptr, pkt);
    } else if (eth->type == htons(0x0835U)) {
        _nx_rarp_packet_deferred_receive(aic_ip_ptr->ip_ptr, pkt);
    }
    
    os_frame_free(data);
    count++;
    return 0;
}

int netstack_output(void *net_interface, void *data, ssv_type_u32 len)
{
    unsigned char * ucBuffer = NULL;
    ssv_type_u8 * frame = NULL;
    int vif_index = 1;
    ssv_type_u8 retry_cnt = 5;

    if (!net_interface)
        return NS_OK;

    vif_index = *((int *)net_interface);
    if (vif_index >= MAX_VIF_NUM) {
        drv_loge("netstack_output, vif_index(%x) error\n", vif_index);
        return NS_OK;
    }

    drv_logd("==sentton== netdev_netstack_output, len=%d\n", len);
    retry_cnt = 5;
retry:
    frame = NULL;
    ucBuffer = NULL;
    frame = os_frame_alloc(len, FALSE);
    if (frame == NULL) {
        drv_loge("netstack_output, alloc error\n");
        //todo: alloc it after a while
        retry_cnt--;
        if (0 == retry_cnt)
            return -1;

        aos_sleep(10);
        goto retry;
    }

    ucBuffer = OS_FRAME_GET_DATA(frame);
    memcpy(ucBuffer, data, len);
    drv_logd("==sentton== netdev_netstack_output, after copy\n");

#if 1
    {
        struct eth_hdr *eth;
        eth = (struct eth_hdr *)ucBuffer;
        if (eth->type == htons(0x0800U)) {
            struct ip_hdr *iphdr = (struct ip_hdr *)((char *)eth + sizeof(struct eth_hdr));

            #define IP_PROTO_ICMP 1
            if (iphdr->_proto == IP_PROTO_ICMP) {
                if ((unsigned int)iphdr % 4) {
                    ALIGN_ARRAY(4) struct ip_hdr iphdr1;
                    memcpy((void *)&iphdr1, iphdr, sizeof(struct ip_hdr));
                    drv_logi("%s, len=%d, ICMP, src=%08x, dst=%08x, [%02x:%02x:%02x:%02x:%02x:%02x], %p\r\n",
                             __func__, len, iphdr1.src, iphdr1.dest, eth->dest[0],
                             eth->dest[1], eth->dest[2], eth->dest[3],
                             eth->dest[4], eth->dest[5], eth);
                } else {
                    drv_logi("%s, len=%d, ICMP, src=%08x, dst=%08x [%02x:%02x:%02x:%02x:%02x:%02x], %p\r\n",
                             __func__, len, iphdr->src, iphdr->dest,
                             eth->dest[0], eth->dest[1], eth->dest[2],
                             eth->dest[3], eth->dest[4], eth->dest[5], eth);
                }
            }
        }
    }
#endif
    ssv6xxx_wifi_send_ethernet(vif_index, frame, len, 0);
    return NS_OK;
}

/* Init netstack
 * [in] config: pointer of config
 * Init netstack with specific config
 * You may 
 * 1)init netstack
 * 2)add default net interface
 * 3)connect io of netstack and 
 */
int netstack_init(void *config)
{
    return NS_OK;
}

int netstack_deinit(void *config)
{
    return NS_OK;
}


/*Init device with setting
 * [in] pdev: pointer of config
 * Init netstack with specific config
 * You may 
 * init netdev
 */

int netdev_init(struct netdev *pdev, ssv_type_bool dft_dev,
                ssv_type_bool init_up)
{
    return NS_OK;
}

int netdev_deinit(struct netdev *pdev)
{      
    return NS_OK;
}

//get hw mac
int netdev_set_default(const char *ifname)
{
    int ret = NS_OK;
    return ret;
}

//get hw mac
int netdev_getmacaddr(const char *ifname, ssv_type_u8 *macaddr)
{
    ssv_type_u8 vif_index = 1;
	ssv6xxx_wifi_get_mac(macaddr, vif_index);
    return NS_OK;
}

//get hw mac
int netdev_getmacaddr_vif(int vif_index, ssv_type_u8 *macaddr)
{
	ssv6xxx_wifi_get_mac(macaddr, vif_index);
    return NS_OK;
}

//get ipinfo
int netdev_getipv4info(const char *ifname, ssv_type_u32 *ip, ssv_type_u32 *gw,
                       ssv_type_u32 *netmask)
{
    u32 if_index;
    struct aic_network_ip_instance *aic_ip_ptr;
    if (wifi_netid[0] == 0xffffffff ||wifi_netid[1] == 0xffffffff){
         drv_loge("netdev_getipv4info wifi_netid not init\r\n");

        return NS_OK;
    }

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when get ipv4\r\n");
        return -1;
    }

    if (strcmp(ifname, SSV_NETIF_NAME_STA)) {
        if_index = ((NX_INTERFACE *)(wifi_netid[0]))->nx_interface_index;
    } else if (strcmp(ifname, SSV_NETIF_NAME_AP)) {
        if_index = ((NX_INTERFACE *)(wifi_netid[1]))->nx_interface_index;
    } else {
        drv_loge("[WCN] can't support %s when set ipv4\n", ifname);
        return -1;
    }

    if (ip)
        *ip = aic_ip_ptr->ip_ptr->nx_ip_interface[if_index].nx_interface_ip_address;

    if (netmask)
        *netmask = aic_ip_ptr->ip_ptr->nx_ip_interface[if_index].nx_interface_ip_network_mask;

    if (gw)
        *gw = aic_ip_ptr->ip_ptr->nx_ip_interface[if_index].nx_interface_ip_network;

    return NS_OK;
}

//set ipinfo
int netdev_setipv4info(const char *ifname, ssv_type_u32 ip, ssv_type_u32 gw,
                       ssv_type_u32 netmask)
{
    u32 if_index;
    struct aic_network_ip_instance *aic_ip_ptr;

    drv_logi("[WCN] start set ipv4 info 0x%x 0x%x 0x%x\n", ip, gw, netmask);
    if (wifi_netid[0] == 0xffffffff ||wifi_netid[1] == 0xffffffff){
        drv_loge("[WCN] netdev_setipv4info wifi_netid not init\r\n");
        return NS_OK;
    }

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when set ipv4\r\n");
        return -1;
    }
   

    if (strcmp(ifname, SSV_NETIF_NAME_STA)) {
        if_index = ((NX_INTERFACE *)(wifi_netid[0]))->nx_interface_index;
    } else if (strcmp(ifname, SSV_NETIF_NAME_AP)) {
        if_index = ((NX_INTERFACE *)(wifi_netid[1]))->nx_interface_index;
    } else {
        drv_loge("[WCN] can't support %s when set ipv4\n", ifname);
        return -1;
    }

    _nx_ip_interface_address_set(aic_ip_ptr->ip_ptr, if_index, ip, netmask);
    return NS_OK;
}

//get dns server
//int netdev_get_ipv4dnsaddr(const char *ifname, u32 *dnsserver);
//set dns server
//int netdev_set_ipv4dnsaddr(const char *ifname, const u32 *dnsserver);

//get interface status
int netdev_check_ifup(const char *ifname)
{
    return NS_OK;
}

//set interface up
int netdev_l3_if_up(const char *ifname)
{
    return NS_OK;
}

//set interface down
int netdev_l3_if_down(const char *ifname)
{   
    return NS_OK;
}

//interface link up cb
void netdev_link_up_cb(void *ifname)
{   
   
}

//interface link down cb
void netdev_link_down_cb(void *ifname)
{   
}

//set dhcp client on dev
int dhcpc_wrapper_set(const char *ifname, const ssv_type_bool enable)
{
    return NS_OK;
}

ssv_type_u32 netdev_getallnetdev(struct netdev *pdev, ssv_type_u32 num_of_pdev)
{
    return 0;
}


ssv_type_s32 netdev_get_netdev(struct netdev *pdev, ssv_type_u8 if_idx)
{
    return -1;
}

int netstack_udp_send(void* data, ssv_type_u32 len, ssv_type_u32 srcip,
                      ssv_type_u16 srcport, ssv_type_u32 dstip,
                      ssv_type_u16 dstport, ssv_type_s16 rptsndtimes)
{
    int err=0;
    return err;
}


int netstack_tcp_send(void* data, ssv_type_u32 len, ssv_type_u32 srcip,
                      ssv_type_u16 srcport, ssv_type_u32 dstip,
                      ssv_type_u16 dstport)
{
    return 0;
}

int netstack_dhcps_info_set(dhcps_info *if_dhcps, dhcps_info *des_if_dhcps,
                            ssv_type_u8 vif_idx)
{
    return 0;
}

int netstack_udhcpd_start(void)
{
	return NS_OK;
}

int netstack_udhcpd_stop(void)
{
    return 0;
}

int netstack_dhcp_ipmac_get(dhcpdipmac *ipmac, int *size_count)
{
    return 0;
}

int netstack_find_ip_in_arp_table(ssv_type_u8 * mac,netstack_ip_addr_t *ipaddr)
{
#undef MAC2STR 
#undef MACSTR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
    int ret;
    u32 ip_address;
    struct aic_network_ip_instance *aic_ip_ptr;
    u32 physical_msw, physical_lsw;

    if (!mac || !ipaddr)
        return 0;

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when input\r\n");
        return 0;
    }

    physical_msw = ((u32)mac[0] << 8) | mac[1];
    physical_lsw = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];

    /* Determine if there is an entry for this IP address.  */
    ret = _nx_arp_ip_address_find(aic_ip_ptr->ip_ptr, (unsigned long *)&ip_address, physical_msw,
                                  physical_lsw);
    if (ret) {
        drv_loge("get station("MACSTR") ip failed!\n", MAC2STR(mac));
    } else {
        drv_loge("station MAC: "MACSTR" ip:%08x\r\n",
                 MAC2STR((unsigned char *)mac), ip_address);
        ipaddr->addr = ip_address;
        return 1;
    }

    return 0;
}

int netstack_find_mac_in_arp_table(ssv_type_u8 * mac,netstack_ip_addr_t *ipaddr)
{
    return 0;
}

int netstack_dhcp_ip_get_by_mac(ssv_type_u8* Mac, ssv_type_u32* ip)
{
    return 0;
}

extern ssv_type_u32 ssv_htonl(ssv_type_u32 n);
extern ssv_type_u16 ssv_htons(ssv_type_u16 n);

int netstack_etharp_unicast (ssv_type_u8 *dst_mac, netstack_ip_addr_t *ipaddr)
{
     struct ssv_ip_addr2 {
       ssv_type_u16 addrw[2];
     } __attribute__ ((packed));
     
     struct ssv_eth_addr {
       ssv_type_u8 addr[6];
     } __attribute__ ((packed));

     struct ssv_eth_hdr {
      struct ssv_eth_addr dest;
      struct ssv_eth_addr src;
      ssv_type_u16 type;
    } __attribute__ ((packed));
    /** the ARP message, see RFC 826 ("Packet format") */
    struct ssv_etharp_hdr {
      ssv_type_u16 hwtype;
      ssv_type_u16 proto;
      ssv_type_u8  hwlen;
      ssv_type_u8  protolen;
      ssv_type_u16 opcode;
      struct ssv_eth_addr shwaddr;
      struct ssv_ip_addr2 sipaddr;
      struct ssv_eth_addr dhwaddr;
      struct ssv_ip_addr2 dipaddr;
    } __attribute__ ((packed));

    ssv_type_u8 local_addr[6] ;
    struct ssv_eth_hdr *ethhdr;
    struct ssv_etharp_hdr *hdr;
    const struct ssv_eth_addr ethzero = {{0,0,0,0,0,0}};
    ssv_type_u8  data[64] = {0};
    int vif_index = 1, ret;
    struct netstack_ip_addr local_ipaddr;
    ssv_type_u32 ip;
    NX_PACKET *packet_ptr;
    struct aic_network_ip_instance *aic_ip_ptr;

    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when send eth arp unicast\n");
        return -1;
    }

    ret = netdev_getipv4info(SSV_NETIF_NAME_AP, &local_ipaddr.addr, NULL, NULL);
    if (ret) {
        drv_loge("[WCN] get ip addr fail when send eth arp unicast\n");
        return -1;
    }

    ip = local_ipaddr.addr;
    //ipaddr->addr = ssv_htonl((ssv_type_u32)ipaddr->addr);
    ethhdr = (struct ssv_eth_hdr *)&data[0];
    hdr = (struct ssv_etharp_hdr *)((ssv_type_u8*)ethhdr + sizeof(struct ssv_eth_hdr));

    hdr->opcode = ssv_htons(0x1);
    netdev_getmacaddr_vif(vif_index, local_addr);
    
    //LOG_PRINTF("local_addr=%02x:%02x:%02x:%02x:%02x:%02x\r\n",local_addr[0], local_addr[1], local_addr[2], 
      //  local_addr[3], local_addr[4], local_addr[5]);
    OS_MemCPY((void*)&hdr->shwaddr, (void*)local_addr, 6);
    OS_MemCPY((void*)&hdr->dhwaddr, (void*)&ethzero, 6);
    /* Write the Ethernet MAC-Addresses */
    OS_MemCPY((void*)&ethhdr->dest, (void*)dst_mac, 6);
    OS_MemCPY((void*)&ethhdr->src, (void*)local_addr, 6);
    /* Copy struct ip_addr2 to aligned ip_addr, to support compilers without
     * structure packing. */ 
    OS_MemCPY((void*)&hdr->sipaddr, (void*)&ip, 4);
    OS_MemCPY((void*)&hdr->dipaddr, (void*)ipaddr, 4);
  
    //LOG_PRINTF("local_ip=%x, ipaddr=%x\r\n",local_ipaddr.ipaddr, ipaddr->addr);
    hdr->hwtype = ssv_htons(0x1);
    hdr->proto = ssv_htons(0x0800U);
    /* set hwlen and protolen */
    hdr->hwlen = 6;
    hdr->protolen = 4;
    ethhdr->type = ssv_htons(0x0806U);

    ret = nx_packet_allocate(aic_ip_ptr->ip_ptr->nx_ip_default_packet_pool,
                             &packet_ptr,
                             ETH_HLEN, AOS_NO_WAIT);
    if (ret != NX_SUCCESS) {
        drv_loge("[WCN] alloc packet fail when send eth arp unicast\n");
        return -1;
    }

    ret = nx_packet_data_append(packet_ptr, data, 64,
                                aic_ip_ptr->ip_ptr->nx_ip_default_packet_pool,
                                AOS_NO_WAIT);
    if (ret != NX_SUCCESS) {
        drv_loge("[WCN] append packet fail when send eth arp unicast\n");
        return -1;
    }

    drv_logi("netstack_etharp_unicast: local_ip=%08x, ipaddr=%08x\r\n",
             local_ipaddr.addr, ipaddr->addr);
    netstack_output(&vif_index, packet_ptr, packet_ptr->nx_packet_length);

    return 0;
}

#define ssv_ip4_addr_set_u32(dest_ipaddr, src_u32) //((dest_ipaddr)->addr = (src_u32))
#define ssv_ip4_addr_get_u32(src_ipaddr)  src_ipaddr//((src_ipaddr)->addr)

/**
 * Convert an ssv_type_u16 from host- to network byte order.
 *
 * @param n ssv_type_u16 in host byte order
 * @return n in network byte order
 */
ssv_type_u16
ssv_htons(ssv_type_u16 n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an ssv_type_u16 from network- to host byte order.
 *
 * @param n ssv_type_u16 in network byte order
 * @return n in host byte order
 */
ssv_type_u16
ssv_ntohs(ssv_type_u16 n)
{
  return 0; //lwip_htons(n);
}

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
ssv_type_u32
ssv_htonl(ssv_type_u32 n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
ssv_type_u32
ssv_ntohl(ssv_type_u32 n)
{
  //return lwip_htonl(n);
  return 0;
}

/**
 * Same as ipaddr_ntoa, but reentrant since a user-supplied buffer is used.
 *
 * @param addr ip address in network order to convert
 * @param buf target buffer where the string is stored
 * @param buflen length of buf
 * @return either pointer to buf which now holds the ASCII
 *         representation of addr or NULL if buf was too small
 */
char *ssv_ipaddr_ntoa_r(const /*ip_addr_t*/char  *addr, char *buf, int buflen)
{
  ssv_type_u32 s_addr;
  char inv[3];
  char *rp;
  ssv_type_u8 *ap;
  ssv_type_u8 rem;
  ssv_type_u8 n;
  ssv_type_u8 i;
  int len = 0;

  s_addr = (ssv_type_u32)ssv_ip4_addr_get_u32(addr);

  rp = buf;
  ap = (ssv_type_u8 *)&s_addr;
  for(n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (ssv_type_u8)10;
      *ap /= (ssv_type_u8)10;
      inv[i++] = '0' + rem;
    } while(*ap);
    while(i--) {
      if (len++ >= buflen) {
        return NULL;
      }
      *rp++ = inv[i];
    }
    if (len++ >= buflen) {
      return NULL;
    }
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return buf;
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int
ssv_ipaddr_aton(const char *cp, netstack_ip_addr_t *addr)
{
  ssv_type_u32 val;
  ssv_type_u8 base;
  char c;
  ssv_type_u32 parts[4];
  ssv_type_u32 *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!ssv_isdigit(c))
      return (0);
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else
        base = 8;
    }
    for (;;) {
      if (ssv_isdigit(c)) {
        val = (val * base) + (int)(c - '0');
        c = *++cp;
      } else if (base == 16 && ssv_isxdigit(c)) {
        val = (val << 4) | (int)(c + 10 - (ssv_islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else
        break;
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3) {
        return (0);
      }
      *pp++ = val;
      c = *++cp;
    } else
      break;
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && !ssv_isspace(c)) {
    return (0);
  }
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  switch (pp - parts + 1) {

  case 0:
    return (0);       /* initial nondigit */

  case 1:             /* a -- 32 bits */
    break;

  case 2:             /* a.b -- 8.24 bits */
    if (val > 0xffffffUL) {
      return (0);
    }
    val |= parts[0] << 24;
    break;

  case 3:             /* a.b.c -- 8.8.16 bits */
    if (val > 0xffff) {
      return (0);
    }
    val |= (parts[0] << 24) | (parts[1] << 16);
    break;

  case 4:             /* a.b.c.d -- 8.8.8.8 bits */
    if (val > 0xff) {
      return (0);
    }
    val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
    break;
  default:
    //LWIP_ASSERT("unhandled", 0);
    break;
  }
  if (addr) {
    ssv_ip4_addr_set_u32(addr, ssv_htonl(val));
  }
  return (1);
}

char *
ssv_ipaddr_ntoa(const /*ip_addr_t*/void *addr)
{
  //static char str[16];
  //return ssv_ipaddr_ntoa_r(addr, str, 16);
  return "0";
}


/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
ssv_type_u32 netstack_ipaddr_addr(const char *cp)
{
  #if 0   
  netstack_ip_addr_t val;

  if (ssv_ipaddr_aton(cp, &val)) {
    return ssv_ip4_addr_get_u32(&val);
  }
  #endif
  return (0xffffffffUL);
}

char *netstack_inet_ntoa(netstack_ip_addr_t addr)
{
    //return ssv_ipaddr_ntoa((ip_addr_t*)&(addr));
    return "0";
}

/* Get one byte from the 4-byte address */
#define ssv_ip4_addr1(ipaddr) (((ssv_type_u8*)(ipaddr))[0])
#define ssv_ip4_addr2(ipaddr) (((ssv_type_u8*)(ipaddr))[1])
#define ssv_ip4_addr3(ipaddr) (((ssv_type_u8*)(ipaddr))[2])
#define ssv_ip4_addr4(ipaddr) (((ssv_type_u8*)(ipaddr))[3])
/* These are cast to ssv_type_u16, with the intent that they are often arguments
 * to printf using the U16_F format from cc.h. */
#define ssv_ip4_addr1_16(ipaddr) ((ssv_type_u16)ssv_ip4_addr1(ipaddr))
#define ssv_ip4_addr2_16(ipaddr) ((ssv_type_u16)ssv_ip4_addr2(ipaddr))
#define ssv_ip4_addr3_16(ipaddr) ((ssv_type_u16)ssv_ip4_addr3(ipaddr))
#define ssv_ip4_addr4_16(ipaddr) ((ssv_type_u16)ssv_ip4_addr4(ipaddr))

ssv_type_u16 netstack_ip4_addr1_16(ssv_type_u32* ipaddr)
{
    return ssv_ip4_addr1_16(ipaddr);
}
ssv_type_u16 netstack_ip4_addr2_16(ssv_type_u32* ipaddr)
{
    return ssv_ip4_addr2_16(ipaddr);
}
ssv_type_u16 netstack_ip4_addr3_16(ssv_type_u32* ipaddr)
{
    return ssv_ip4_addr3_16(ipaddr);
}
ssv_type_u16 netstack_ip4_addr4_16(ssv_type_u32* ipaddr)
{
    return ssv_ip4_addr4_16(ipaddr);
}

void netstack_wifi_link_register_cb(netdev_link_callback_t link_up_cb, netdev_link_callback_t link_down_cb)
{
    netmgr_wifi_link_register_cb(link_up_cb, link_down_cb);
}

#if 0
static void wifi_mem_free(void *buf)
{
	TCPIP_PACKET_INFO_T *pkt = (TCPIP_PACKET_INFO_T *)buf;

	if (pkt && pkt->data_ptr) 
    {
		//LOG_PRINTF("%s %p %p %d\n", __func__,pkt, pkt->data_ptr,pkt->data_len);
		free(pkt->data_ptr);
		pkt->data_ptr = NULL;
		free(pkt);
		pkt =  NULL;
	}
}

static void wifi_free_daps_mem_zerocpy(void *buf)
{
}
#endif

int netdev_netstack_output(NX_PACKET *packet_ptr)
{
    int vif_index = 0;

    if (packet_ptr->nx_packet_next)
        aic_assert(FALSE);

    if ((u32)packet_ptr->nx_packet_ip_interface == wifi_netid[0])
        vif_index = 0;
    else if ((u32)packet_ptr->nx_packet_ip_interface == wifi_netid[1])
        vif_index = 1;

    drv_logd("[WCN] %s output len %d\n",
             packet_ptr->nx_packet_ip_interface->nx_interface_name,
             packet_ptr->nx_packet_length);
    //aic_wcn_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr, packet_ptr->nx_packet_length);
    netstack_output((void *)&vif_index, packet_ptr->nx_packet_prepend_ptr,
                    packet_ptr->nx_packet_length);
    nx_packet_transmit_release(packet_ptr);

    return NS_OK;
}
//void wifi_adap_report_open_sta_success(int id_);
//void wifi_adap_report_open_ap_success(int id_);

void wcn_network_driver_entry(NX_IP_DRIVER *nx_ip_driver)
{
    AOS_INTERRUPT_DECLARE
    NX_IP *nx_ip;
    NX_PACKET *packet_ptr;
    u32 *ethernet_frame_ptr;
    NX_INTERFACE *nx_interface_ptr;

    /* Get the pointer to the NX_IP instance. */
    nx_ip = nx_ip_driver->nx_ip_driver_ptr;

    /* Set the default status return. */
    nx_ip_driver->nx_ip_driver_status = NX_NOT_SUCCESSFUL;

    /* Get the pointer to the interface in local variable. */
    nx_interface_ptr = nx_ip_driver->nx_ip_driver_interface;

    /* Is this the ATTACH command? */
    if (nx_ip_driver->nx_ip_driver_command == NX_LINK_INTERFACE_ATTACH) {
        /* Critical section. */
        AOS_INTERRUPT_DISABLE

        if (!strcmp(nx_interface_ptr->nx_interface_name, SSV_NETIF_NAME_STA))
            wifi_netid[0] = (u32)nx_interface_ptr;
        else if (!strcmp(nx_interface_ptr->nx_interface_name, SSV_NETIF_NAME_AP))
            wifi_netid[1] = (u32)nx_interface_ptr;

        nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;

        /* Unprotect the critical section. */
        AOS_INTERRUPT_RESTORE
    } else {
        /* Identify command. */
        switch (nx_ip_driver->nx_ip_driver_command) {
        case NX_LINK_INITIALIZE:
            /* INIT command, set the interface parameters.  */
            nx_interface_ptr->nx_interface_valid = NX_TRUE;
            nx_interface_ptr->nx_interface_address_mapping_needed = NX_TRUE;
            nx_interface_ptr->nx_interface_ip_mtu_size = 1500;

            /* Set the link to down for now.  */
            nx_interface_ptr->nx_interface_link_up = NX_FALSE;

            /* Operation is successful.  */
            nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;
            break;
        case NX_LINK_ENABLE:
            /* Set the link state to UP.  */
            nx_interface_ptr->nx_interface_link_up = NX_TRUE;
            nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;
            break;
        case NX_LINK_DISABLE:
            /* Set the link down.  */
            if (nx_ip->ipv6_forward_release_packet)
                nx_ip->ipv6_forward_release_packet(nx_interface_ptr->nx_interface_index);

            nx_interface_ptr->nx_interface_link_up = NX_FALSE;
            nx_ip_driver->nx_ip_driver_status =  NX_SUCCESS;        
            break;
        case NX_LINK_PACKET_SEND:
        case NX_LINK_PACKET_DIRECT_SEND:
        case NX_LINK_PACKET_BROADCAST:
        case NX_LINK_ARP_SEND:
        case NX_LINK_ARP_RESPONSE_SEND:
        case NX_LINK_RARP_SEND:
            /* Place the ethernet frame at the front of the packet.  */
            packet_ptr = nx_ip_driver->nx_ip_driver_packet;
#ifdef CONFIG_NET_PCAP_DUMP
            aic_network_pcap_dump(packet_ptr->nx_packet_prepend_ptr,
                                  packet_ptr->nx_packet_length,
                                  AIC_USB_NIC_FLAG);
#endif
            /* Adjust the prepend pointer.  */
            packet_ptr->nx_packet_prepend_ptr = packet_ptr->nx_packet_prepend_ptr - sizeof(struct eth_hdr);

            /* Adjust the packet length.  */
            packet_ptr->nx_packet_length = packet_ptr->nx_packet_length + sizeof(struct eth_hdr);
            
            /* Setup the ethernet frame pointer to build the ethernet frame.
               Back up another 2 bytes to get 32-bit word alignment. */
            ethernet_frame_ptr = (u32 *)(packet_ptr->nx_packet_prepend_ptr - 2);

            /* Build the ethernet frame.  */
            *ethernet_frame_ptr = nx_ip_driver->nx_ip_driver_physical_address_msw;
            *(ethernet_frame_ptr + 1) = nx_ip_driver->nx_ip_driver_physical_address_lsw;
            *(ethernet_frame_ptr + 2) = (nx_interface_ptr->nx_interface_physical_address_msw << 16) |
                (nx_interface_ptr->nx_interface_physical_address_lsw >> 16);
            *(ethernet_frame_ptr + 3) = (nx_interface_ptr->nx_interface_physical_address_lsw << 16);

            if ((nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_SEND)||
               (nx_ip_driver->nx_ip_driver_command == NX_LINK_ARP_RESPONSE_SEND)) {
                *(ethernet_frame_ptr + 3) |= NX_ETHERNET_ARP;
            } else if (nx_ip_driver->nx_ip_driver_command == NX_LINK_RARP_SEND) {
                *(ethernet_frame_ptr + 3) |= NX_ETHERNET_RARP;
            } else {
                if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V4)
                    *(ethernet_frame_ptr + 3) |= NX_ETHERNET_IP;
#ifdef FEATURE_NX_IPV6
                else if (packet_ptr->nx_packet_ip_version == NX_IP_VERSION_V6)
                    *(ethernet_frame_ptr + 3) |= NX_ETHERNET_IPV6;
#endif /* FEATURE_NX_IPV6 */
                else 
                {
                    /* Unknown IP version */
                    /* free the packet that we will not send */
                    nx_packet_release(packet_ptr);
                    nx_ip_driver  -> nx_ip_driver_status =  NX_NOT_SUCCESSFUL;
                    break;
                }
            }

            /* Endian swapping if NX_LITTLE_ENDIAN is defined.  */
            NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr));
            NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 1));
            NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 2));
            NX_CHANGE_ULONG_ENDIAN(*(ethernet_frame_ptr + 3));

            /* Write the packet or queue it. */
            nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;
            netdev_netstack_output(packet_ptr);
            break;
        case NX_LINK_UNINITIALIZE:
            break;
        case NX_LINK_INTERFACE_DETACH:
            break;
        case NX_LINK_MULTICAST_JOIN:
        case NX_LINK_MULTICAST_LEAVE:
            nx_ip_driver->nx_ip_driver_status = NX_SUCCESS;
            break;
        case NX_LINK_GET_STATUS:
        case NX_LINK_GET_ERROR_COUNT:
        case NX_LINK_GET_RX_COUNT:
        case NX_LINK_GET_TX_COUNT:
        case NX_LINK_GET_ALLOC_ERRORS:
        case NX_LINK_GET_SPEED:
        case NX_LINK_GET_DUPLEX_TYPE:
        case NX_LINK_USER_COMMAND:
        default:
            /* Invalid driver request.  */
            nx_ip_driver->nx_ip_driver_status =  NX_UNHANDLED_COMMAND;
            /* Return the link status in the supplied return pointer. */
            if (nx_ip_driver->nx_ip_driver_return_ptr)
                *(nx_ip_driver->nx_ip_driver_return_ptr) = (u32)0;
            break;
        }
    }

    /* We are done here. */
    return;
}

int netdev_register_net_dev(u8 *mac, int mode)
{
    //struct wifi_device *wifi_dev = (struct wifi_device *)dev;
    u32 ret;
    u32 physical_msw, physical_lsw;
    struct aic_network_ip_instance *aic_ip_ptr;

    drv_logi("netdev_register_net_dev, mode = %d\n", mode);
    aic_ip_ptr = aic_get_default_ip_instance();
    if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
        drv_loge("[WCN] get ip ptr fail when input\r\n");
        return 0;
    }

    switch (mode/*wifi_dev->mode*/) {
    case 0: //SSV6XXX_HWM_STA
        if (wifi_netid[0] != 0xffffffff) {
            drv_loge("wifi_netid[0]=%x, already register\n", wifi_netid[0]);
            return -1;
        }

        ret = _nx_ip_interface_attach(aic_ip_ptr->ip_ptr, SSV_NETIF_NAME_STA,
                                      IP_ADDRESS(192, 168, 20, 1), 0xFFFFFF00UL,
                                      wcn_network_driver_entry);
        if (ret != NX_SUCCESS) {
            drv_loge("attach %s fail\n", SSV_NETIF_NAME_STA);
            return -1;
        }

        if (wifi_netid[0] == 0xffffffff) {
            drv_loge("attach %s netid fail\n", wifi_netid[0]);
            return -1;
        }

        physical_msw = ((u32)mac[0] << 8) | mac[1];
        physical_lsw = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];
        _nx_ip_interface_physical_address_set(aic_ip_ptr->ip_ptr, ((NX_INTERFACE *)(wifi_netid[0]))->nx_interface_index,
                                              physical_msw, physical_lsw,
                                              FALSE);

        drv_logi("netdev_register_net_dev[STA], wifi_netid = 0x%x\n", wifi_netid[0]);
        //sci_setip6autoip(wifi_dev->net_id, FALSE, 0, 0);
    //    wifi_adap_report_open_sta_success(wifi_netid[0]);
        break;

    case 1://SSV6XXX_HWM_AP
        if (wifi_netid[1] != 0xffffffff) {
            drv_loge("wifi_netid[1]=%x, already register\n", wifi_netid[1]);
            return -1;
        }

        ret = aic_config_softap_intf(SSV_NETIF_NAME_AP, wcn_network_driver_entry);
        if (ret != NX_SUCCESS) {
            drv_loge("config softap intf %s fail\n", SSV_NETIF_NAME_AP);
            return -1;
        }

        if (wifi_netid[1] == 0xffffffff) {
            drv_loge("attach %s netid fail\n", wifi_netid[0]);
            return -1;
        }

        physical_msw = ((u32)mac[0] << 8) | mac[1];
        physical_lsw = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) | ((u32)mac[4] << 8) | mac[5];
        _nx_ip_interface_physical_address_set(aic_ip_ptr->ip_ptr, ((NX_INTERFACE *)(wifi_netid[1]))->nx_interface_index,
                                              physical_msw, physical_lsw,
                                              FALSE);

        drv_logi("netdev_register_net_dev[AP], wifi_netid id = 0x%x \n",
                 wifi_netid[1]);

       // wifi_adap_report_open_ap_success(wifi_netid[1]);
        break;
    default:
        //LOG_PRINTF("Not supported mode: %d\n", wifi_dev->mode);
        break;
    }

    return 0;
}

void netdev_unregister_net_dev(int index)
{
    struct aic_network_ip_instance *aic_ip_ptr;

    if (index >= SSV_NETIF_MAX) {
        drv_loge("netdev_unregister_net_dev, vif = 0x%x err!!!!\n", index);
        return;
    }

    if (wifi_netid[index] != 0xffffffff) {
        aic_ip_ptr = aic_get_default_ip_instance();
        if (!aic_ip_ptr || !aic_ip_ptr->ip_ptr) {
            drv_loge("[WCN] get ip ptr fail when unregister\r\n");
            return;
        }

        aic_delete_softap_intf();
        wifi_netid[index] = 0xffffffff;
    }
}
#if 1
int netdev_wifi_evt_stack_pm(int is_pm)
{
    return 0;
}
#endif
ssv_type_u32 netdev_wifi_get_ipaddr(int index)
{
    return ((NX_INTERFACE *)(wifi_netid[index]))->nx_interface_ip_address;
}

void netdev_wifi_process_disassoc_ind_event(ssv_wifi_event_data *enData)
{
    int ret;
    ssv_type_u8 mac[6];
    netstack_ip_addr_t ipaddr;

    if (!enData)
        return;

    mac[0] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[0];
    mac[1] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[1];
    mac[2] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[2];
    mac[3] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[3];
    mac[4] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[4];
    mac[5] = (ssv_type_u8)enData->data.auth_deauth_data.reserved[5];
    ret = netstack_find_ip_in_arp_table(mac, &ipaddr);
    if (ret) {
        drv_logi("[NET_WIFI] get ip addr 0x%x when process dis event\n",
                 ipaddr.addr);
        aic_handle_client_disassoc(ipaddr.addr);
    } else {
        drv_logi("[NET_WIFI] find ip in arp table fail\n");
    }
}

