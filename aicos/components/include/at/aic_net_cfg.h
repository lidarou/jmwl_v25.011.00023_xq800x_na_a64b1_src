#ifndef __AIC_NET_CFG_H__
#define __AIC_NET_CFG_H__

#pragma pack(push,1)

typedef struct _NET_CFG{
    unsigned char auto_start_ps;    /* auto start ps */
    unsigned char cfun_pre_rst;    /* cfun value before reset */
    unsigned char auto_cfg_pdp_intf;    /* auto config pdp intf */
    unsigned char auto_start_usb_net;    /* auto start usb net */
    unsigned char reg_mode;    /* register mode */
    unsigned char urc_port_name[12];    /* urc port name */
    unsigned char ri_signal_type[12];    /* ri signal type */
    unsigned char m_intf_mode;    /* customer usb intf mode */
    unsigned char m_work_mode;    /* customer usb share mode */
    unsigned char m_auto_flag;    /* customer auto connect to wan */
    unsigned char pdp_type[8];    /* pdp type */
    unsigned char default_apn[40];    /* default apn name */
    unsigned char cus_dial_mode;    /* customer dial mode */
    unsigned char cus_intf_mode;    /* customer intf mode */
    unsigned char cus_ip_mode;    /* customer ip mode */
    unsigned char cus_third_byte_set;    /* customer if set third byte */
    unsigned char cus_dhcp_start_set;    /* customer if set dhcp start ip */
    unsigned char cus_dhcp_third_byte;    /* customer dhcp third byte */
    unsigned char cus_dhcp_ip_start;    /* customer dhcp start ip */
    unsigned char cus_dhcp_ip_end;    /* customer dhcp end ip */
    unsigned char cereg_mode;    /* cereg mode */
    unsigned char ctzu_mode;    /* ctzu mode */
    unsigned char d2d_lte_mode;    /* d2d or lte mode */
    unsigned char d2d_pkt_dump;    /* enable d2d pkt dump */
    unsigned int d2d_pkt_size;    /* d2d raw pkt size */
    unsigned int d2d_at_com_id;    /* d2d at com id */
    unsigned char d2d_role;    /* d2d role */
    unsigned char d2d_msg_head_enabled;    /* d2d msg head enabled */
    unsigned char d2d_com_type[8];    /* d2d com type */
    unsigned char reserved[2];    /* 4byte alignment */
}NET_CFG;

#define AIC_NET_CFG_STRUCT_SIZE    112

#pragma pack(pop)

#endif /* __AIC_NET_CFG_H__ */
