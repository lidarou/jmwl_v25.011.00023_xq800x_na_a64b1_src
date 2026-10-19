
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1c70cb
#define LOG_MODULE    AMOD_VENDOR
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_net.h"
#include "abup_stdlib.h"
#include "abup_thread.h"
#include "abup_device.h"
#include "abup_timer.h"
#include "abup_flash.h"

#include <aic_net_api.h>
#include <nxd_dns.h>


#ifdef CONFIG_FOTA_SUPPORT_ABUP
#define ABUP_PDP_ACTIVE_RETRY_TIME 4
#define ABUP_PDP_ACTIVE_TIMEOUT_RETRY (30*1000)
ABUP_BOOL s_abup_is_pdp_activated = ABUP_FALSE;
ABUP_BOOL s_abup_is_pdp_active_flag = ABUP_FALSE;
abup_uint8 s_abup_sock_index_activate_pdp = 0;
ABUP_BOOL s_abup_is_dns_act_pdp = ABUP_FALSE;          //action: activate dns pdp
ABUP_BOOL s_abup_is_dns_pdp_actived = ABUP_TRUE;//ABUP_FALSE;      //status: dns pdp is actived or not
abup_common_callback s_abup_pdp_userevt;

static abup_uint32 g_abup_net_id = 0;//5;
ABUP_SOCKET_MGR_STRUCT abup_socket_mgr = {0};

//for  socket
struct abup_socket_info
{
    abup_uint8 sock_index_id;   // add by longting.zhao
    abup_uint8 link_id;
    abup_uint16 lport;
    abup_uint16 fport;
    abup_uint32 fhost;
    //void *socket;
    abup_int32 socket;
    abup_uint32 type;
    abup_uint32 state;
    //TCPIP_HOST_HANDLE sock_request_id;   // add by longting.zhao
    abup_int32 sock_request_id;
    abup_char *addr_str;
};

enum abup_cp_state_e
{
    ABUP_IDLE,
    ABUP_SET,
    ABUP_ACTIVE,
    ABUP_SEND
};

#define  MAX_RECEIVE_DATA_LENGTH   400
#define  MAX_SEND_DATA_LENGTH  1024
#define  UDP_DATACONF   1
#define  TCP_DATACONF   2
#define  MAX_PORT_NUM   65535
#define  MAX_CONNECT_ID_NUMBER  10
#define  default_netid   5     //default nsapi value = 5 when default cid =1
#define  MAX_TX_BUF_LEN   1024
#define  MAX_RX_BUF_LEN    400

#define  ABUP_MAX_TX_BUF_LEN               1024
#define  ABUP_MAX_RX_BUF_LEN               400
#define  ABUP_MAX_SEND_DATA_LENGTH         1024
#define  ABUP_MAX_DATACONF_NUM             10
#define  ABUP_MAX_UDP_SEND_LENGTH          1096 //shijun modify for TFTP client cr35475
#define  ABUP_UDP_DATACONF                 1
#define  ABUP_TCP_DATACONF                 2
#define  ABUP_ATDATALEN                    500

#define  ABUP_DEFAULT_APN                   "CMNET"
#define  ABUP_DEFAULT_CID                   1
#define  ABUP_DEFAULT_NETID                 5     //default nsapi value = 5 when default cid =1
#define  ABUP_MAX_PORT_NUM                  65535

#define  ABUP_MAX_IP_STR_LEN            15  //just for IPV4
#define  ABUP_MAX_NETWORK_ADDR_LEN      255
#define  ABUP_MAX_PDP_ADDRESS_LEN 20
//Hyman we set at pdp max num as MN:MN_GPRS_MAX_PDP_ID_NUM
#define ABUP_MAX_PDP_CONTEXT_COUNT 15
#define ABUP_ERR_INVALID_INDEX 21
#define ABUP_MAX_PCO_LEN 253
#define ABUP_MAX_PDP_TYPE_LEN 7     //longest str: ipv4v6
#define ABUP_MAX_APN_LEN 100
#define ABUP_MAX_PAP_USER_LEN 64
#define ABUP_MAX_PAP_PASSWD_LEN 64

#define ABUP_MAX_IMSI_LEN 16
#define ABUP_LACCELL_UPDATE_MAXNUM 5

#ifdef TCPIP_SUPPORT
LOCAL int32 ATC_CreateAndConnectTCPSocket(ATC_CONFIG_T *atc_config_ptr, unsigned long ip_addr, uint8 sock_index_id);
LOCAL int32 ATC_CreateUDPSocket(unsigned long ip_addr, uint8 sock_index_id);
LOCAL void ATC_SocketSend(ATC_CONFIG_T *atc_config_ptr,uint32 sock_id, uint8 sock_index);
LOCAL void ATC_CloseSocket(ATC_CONFIG_T *atc_config_ptr, uint32 id);
#endif

abup_uint8 s_abup_imsi[ABUP_MAX_IMSI_LEN+1] = {0};
static abup_uint8 abup_pdp_active_times = 0;

abup_uint32 abup_net_pdp_active(abup_common_callback userevt);
extern void abup_get_new_version_ex(void);
extern void abup_upgrade_thread_on_ex(void);
extern ABUP_BOOL  abup_fota_running(void);
extern void abup_register_ex(void);
extern void ABUP_reset_stop_downlaod(void);
extern ABUP_BOOL abup_get_force_exit(void);
static void abup_net_pdp_reactive(void);
extern void abup_upgrade_thread_on(void);
extern void abup_get_new_version(void);
extern void abup_save_download_ip(abup_uint8 addr1,abup_uint8 addr2,abup_uint8 addr3,abup_uint8 addr4);
extern abup_uint8 abup_get_fota_state(void);
extern void abup_Debug_DeltaNew(void);

void abup_net_set_netid(abup_uint32 netid)
{
    g_abup_net_id = netid;
}

abup_uint32 abup_net_get_netid(void)
{
    return g_abup_net_id;
}

abup_uint32 abup_encode_dataaccount_id(void)
{
    static abup_uint32 account_id=0;

    abup_memset(&abup_socket_mgr, 0, sizeof(abup_socket_mgr));
    return account_id;
}

ABUP_BOOL abup_GetHostByName_Notify(void *msg_ptr)
{
    ABUP_SOCK_CB_MSG *dns_ind = (ABUP_SOCK_CB_MSG*)msg_ptr;
    abup_uint32 addr_buf=0;
    abup_int32 addr_len=0;
    abup_uint16 port=0;
    abup_int32 result=0;
    abup_uint8 s_addr[4]={0};

    //abup_DebugPrint("abup_GetHostByName_Notify add= 0x%04x ",dns_ind->server_addr.sin_addr.s_addr);
    abup_DebugPrint("abup_GetHostByName_Notify ip= %s ",inet_ntoa(*( (struct in_addr*) (&dns_ind->server_addr.sin_addr))));

    addr_len = 4;
    abup_memset(s_addr,0,4);

    s_addr[0] = (dns_ind->server_addr.sin_addr.s_addr&0x000000ff);
    s_addr[1] = (dns_ind->server_addr.sin_addr.s_addr&0x0000ff00)>>8;
    s_addr[2] = (dns_ind->server_addr.sin_addr.s_addr&0x00ff0000)>>16;
    s_addr[3] = (dns_ind->server_addr.sin_addr.s_addr&0xff000000)>> 24;

    abup_DebugPrint("abup_GetHostByName_Notify dns_ind->addr=%d.%d.%d.%d", s_addr[0],s_addr[1],s_addr[2],s_addr[3]);

    if(addr_len == 4)
    {
        abup_save_download_ip(s_addr[0],\
            s_addr[1],\
            s_addr[2],\
            s_addr[3]);
    }

    if (dns_ind)
    {
        abup_net_set_state(NET_STAT_CONN_REQ);
        addr_buf = (abup_uint32)dns_ind->server_addr.sin_addr.s_addr;

        if(abup_get_fota_state() != STATE_DL)
        {
            //abup_set_ipaddr(addr_buf);
        }

        if (abup_get_fota_state() == STATE_DL)
        {
            port = abup_get_fota_dl_port();
        }
        else
        {
            port = abup_get_fota_port();
        }

        result=abup_connect(abup_socket_mgr.abup_request_id, addr_buf, addr_len, port);

        if(result != E_ABUP_SUCCESS)
        {
            abup_DebugPrint_Ext("[ABUP_DNS]:","abup_GetHostByName_Notify result!=E_abup_SUCCESS");
            abup_soc_callback_init(&abup_socket_mgr,ABUP_ERROR_NOTIFY);

            abup_socket_mgr.abup_net_error_notify(abup_socket_mgr.abup_tcp_socket_id);
            abup_net_set_state(NET_STAT_CONN_ERR);
        }

        abup_net_set_state(NET_STAT_CONN_SUCC);
        return ABUP_TRUE;
    }
    else    // dns analyse error.
    {
        abup_DebugPrint_Ext("[ABUP_DNS]:","abup_GetHostByName_Notify dns_ind->dns analyse error");
        abup_soc_callback_init(&abup_socket_mgr,ABUP_ERROR_NOTIFY);
        abup_socket_mgr.abup_net_error_notify(abup_socket_mgr.abup_tcp_socket_id);
        abup_net_set_state(NET_STAT_DNS_ERR);
        //display dns error
    }

    return ABUP_FALSE;
}


abup_int32 Abup_GetHostByName(abup_int32 request_id, const abup_char* domain, abup_uint32* addr_buf, abup_uint32 nwk_account_id)
{
    abup_int32 ret = -1;
    abup_int32 result = -1;
    ABUP_SOCK_CB_MSG recv_msg = {0};
    struct addrinfo *res;
    struct addrinfo hints;
    NX_DNS *dns_ptr;

    abup_memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;


    dns_ptr = aic_get_dns_client();
    if(dns_ptr == NULL)
    {
        abup_DebugPrint("abup_GetHostByName: dns not activated!");
        return E_ABUP_FAILURE;
    }
    abup_DebugPrint("abup_GetHostByName nx_dns_id=%d, nx_dns_domain=%s",dns_ptr->nx_dns_id,dns_ptr->nx_dns_domain);

    abup_strncpy(abup_socket_mgr.domain, domain, ABUP_DOMAIN_STRLEN - 1);
    abup_net_set_state(NET_STAT_DNS_REQ);

    abup_DebugPrint("abup_GetHostByName domain:%s", domain);
    ret = getaddrinfo(domain, NULL, &hints, &res);

    abup_socket_mgr.abup_request_id = ret;

    abup_DebugPrint_Ext("[ABUP_DNS]:","abup_config_fota_addr getaddrinfo return =%d", ret);

    if(ret == 0 && res)
    {
        abup_DebugPrint_Ext("[ABUP_DNS]:","abup_config_fota_addr getaddrinfo suc");

        if (ret == 0 && res) {
            if (res->ai_family == AF_INET) {
                recv_msg.server_addr.sin_family = AF_INET;
                recv_msg.server_addr.sin_port = htons(recv_msg.port);
                recv_msg.server_addr.sin_addr.s_addr =
                    ((struct sockaddr_in *)res->ai_addr)->sin_addr.s_addr;
            }
        }
        abup_net_set_state(NET_STAT_DNS_SUCC);
        recv_msg.msg_id = ABUP_SOCK_GETHOSTBYNAME;
        ret = abup_sock_msg_send(&recv_msg, AOS_NO_WAIT);
        abup_DebugPrint_Ext("[ABUP_DNS]:","abup_sock_msg_send ret:%d",ret);

        result = E_ABUP_SUCCESS;
    }
    else
    {
        abup_DebugPrint_Ext("[ABUP_DNS]:","abup_config_fota_addr getaddrinfo ERROR:%d",errno);
        abup_net_set_state(NET_STAT_DNS_ERR);
        result = E_ABUP_FAILURE;

    }

    freeaddrinfo(res);

    return result;
}

abup_int32 abup_open_socket(void)
{
    abup_int32 soc_id = 0;

    abup_net_set_state(NET_STAT_NEWSOC_REQ);
    soc_id = socket(AF_INET, SOCK_STREAM, 0);

    if(soc_id == -1)
    {
        abup_DebugPrint_Ext("[ABUP_SOC]:","open_socket: soc_create failed, sockid = %d", soc_id);
        abup_net_set_state(NET_STAT_NEWSOC_ERR);
        return 0;
    }

    abup_socket_mgr.abup_tcp_socket_id= soc_id;
    abup_DebugPrint_Ext("[ABUP_SOC]:","connect_tcp-->soc_id = %d", soc_id);

    //result = anet_fcntl(soc_id, F_SETFL, O_NONBLOCK);
    //abup_DebugPrint("[ABUP_SOC]:open_socket set non block: %s", (result==0)?"OK":"FAIL!!!");

    abup_net_set_state(NET_STAT_NEWSOC_SUCC);

    return (abup_int32)soc_id;
}

abup_int32 abup_connect(abup_int32 request_id,abup_uint32 addr_buf,abup_uint16 addr_len,abup_uint16 port)
{
    struct sockaddr_in addr;
    abup_socint soc_id;
    abup_int8 result=-1;
    abup_int32 error_code = 0;
    ABUP_SOCK_CB_MSG recv_msg = {0};
    abup_int32 ret = 0;

    abup_DebugPrint("[ABUP_SOC] connect: addr_buf=0x%08x,addr_len=%d,port:%d,reqid:%d",addr_buf,addr_len,port,request_id);
    abup_memset(&addr, 0x00, sizeof(struct sockaddr_in));

    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = addr_buf;//htonl(addr_buf);

    soc_id = abup_open_socket();
    abup_DebugPrint("[ABUP_SOC] connect: addr.ip_addr=0x%08x,addr.port:%d,len:%d. oport:%d",\
        addr.sin_addr.s_addr, addr.sin_port, sizeof(addr), port);

    abup_net_set_state(NET_STAT_CONN_REQ);
    result = connect(soc_id,(struct sockaddr *)&addr,sizeof(addr));

    abup_DebugPrint_Ext("[ABUP_SOC]:","socket connect return %d ", result);

    if (result == 0)
    {
        abup_DebugPrint_Ext("[ABUP_SOC]:","socket connect   SOC_SUCCESS ");
        abup_net_set_state(NET_STAT_CONN_SUCC);
        abup_soc_callback_init(&abup_socket_mgr,ABUP_WRITE_NOTIFY);
        abup_socket_mgr.abup_write_notify(soc_id);
    }
    else if (result < 0)
    {
        error_code = errno;
        abup_DebugPrint("abup: socket connect error_code=%d ", error_code);
        if((error_code==EWOULDBLOCK) || (error_code==EINPROGRESS))
        {
            abup_DebugPrint_Ext("[ABUP_SOC]:","socket connect   SOC_WOULDBLOCK ");
            abup_net_set_state(NET_STAT_CONNECTING);
        }
        else
        {
            abup_DebugPrint_Ext("[ABUP_SOC]:","socket connect   SOC_ERROR ");
            abup_net_set_state(NET_STAT_CONN_ERR);
            abup_soc_close(soc_id);
            abup_soc_callback_init(&abup_socket_mgr,ABUP_CLOSE_NOTIFY);
            abup_socket_mgr.abup_close_notify(soc_id);
            return E_ABUP_FAILURE;
        }

    }
    result = E_ABUP_SUCCESS;

    if(result == 0){
        recv_msg.msg_id = ABUP_SOCK_READ;
        recv_msg.sock_id = soc_id;
        ret = abup_sock_msg_send(&recv_msg, AOS_NO_WAIT);
        abup_DebugPrint_Ext("[ABUP_CONN]:","ret = %d!",ret);
    }
    return result;
}

void abup_soc_close(abup_socint soc_id)
{
    abup_int32 result = 0;
    abup_DebugPrint("abup soc_close: soc_id=%d, g_tcp_socket_id=%d", soc_id, abup_socket_mgr.abup_tcp_socket_id);

    if (abup_pdp_active_times > 0)
    {
        abup_pdp_active_times = 0;
        abup_stop_timer(abup_net_pdp_reactive);
    }
    if(soc_id == -1)
        return;

    if(soc_id != abup_socket_mgr.abup_tcp_socket_id )
        return;

    abup_net_set_state(NET_STAT_CLOSE_REQ);

    //result = close(soc_id);
    result = soc_close(soc_id);
    abup_socket_mgr.abup_tcp_socket_id=-1;
    abup_DebugPrint("abup soc_close: result: %d! line=%d",result,__LINE__);

    if(result < 0)
    {
        abup_DebugPrint("abup soc_close   unsuccess! errno:%d", errno);
        abup_net_set_state(NET_STAT_CLOSE_ERR);
    }
    else
    {
        abup_DebugPrint("abup soc_close   success");
        abup_net_set_state(NET_STAT_CLOSE_SUCC);
    }
}


abup_int32 abup_soc_send(abup_socint soc_id,Request* req )
{
    abup_int32 result;
    abup_int32 error_code = 0;

    abup_DebugPrint("abup_soc_send  soc_id = %d,g_tcp_socket_id:%d", soc_id,abup_socket_mgr.abup_tcp_socket_id);
    abup_net_set_state(NET_STAT_SEND_REQ);

       result = send(soc_id, req->header.data, strlen(req->header.data),0);

    abup_DebugPrint(" abup_soc_send -------------  strlen(req->header.data) =%d",  strlen(req->header.data));
    abup_DebugPrint(" abup_soc_send ------------- header->data =\r\n%s", req->header.data);
    abup_DebugPrint("+70 %s", req->header.data+70);
    abup_DebugPrint("+200 %s", req->header.data+200);
    if(strlen(req->header.data) > 300)
        abup_DebugPrint(  " %s", req->header.data+300);
    abup_DebugPrint_Ext("[ABUP_SOC]:","abup_soc_send num of byte sent = %d", result);

    if (result > 0)
    {
        abup_DebugPrint("abup_soc_send   SOC_SUCCESS ");
        abup_net_set_state(NET_STAT_SEND_SUCC);
        return E_ABUP_SUCCESS;
    }
    else// if(result == SOC_WOULDBLOCK)
    {
         error_code = errno;
         abup_DebugPrint("abup_soc_send: error_code=%d ", error_code);

         //????????????????
         //if(EPIPE==error_code || ENOBUFS == error_code || EWOULDBLOCK == error_code)
         if(error_code == EWOULDBLOCK)
         {
            abup_DebugPrint("abup_soc_send   SOC_WOULDBLOCK ");
            abup_net_set_state(NET_STAT_SENDING);
            //????????????????        abup_socket_mgr.abup_soc_state=SOC_WRITE;
            return E_ABUP_WOULDBLOCK;
         }
         else
         {
            abup_DebugPrint("abup_soc_send     SOC_ERROR ");
            abup_net_set_state(NET_STAT_SEND_ERR);

            return E_ABUP_FAILURE;
         }
     }

}



abup_int32 abup_soc_recv(abup_socint soc_id,abup_char* buf,abup_int32 len,abup_uint32 flag)
{
    abup_int32 ret = -1;
    abup_int32 error_code = 0;

    abup_DebugPrint("[ABUP_RECV]:soc_recv------>");
    abup_net_set_state(NET_STAT_RECV_REQ);

    ret = recv(soc_id, buf, len, flag);
    abup_DebugPrint("[ABUP_RECV]: ret=%d,buf=%s",ret,buf+20);
    abup_DebugPrint("[ABUP_RECV]: buf=+120:%s",buf+120);
    abup_DebugPrint("[ABUP_RECV]: buf=+200:%s",buf+200);
    abup_DebugPrint("[ABUP_RECV]: buf=+300:%s",buf+300);
    if (ret > 0)
    {
        abup_net_set_state(NET_STAT_RECV_SUCC);
        return     ret;
    }
    else
    {
        error_code =errno;
        abup_DebugPrint("[ABUP_RECV]:abup soc_rev: error_code:%d line = %d", error_code,__LINE__);
        if(error_code == EWOULDBLOCK)
        {
            abup_DebugPrint("[ABUP_RECV]:abup_soc_rev   SOC_WOULDBLOCK ");
            abup_net_set_state(NET_STAT_RECVING);
            return E_ABUP_WOULDBLOCK;
        }
        else
        {
            abup_net_set_state(NET_STAT_RECV_ERR);
            return E_ABUP_FAILURE;
        }
    }
}

abup_socint abup_net_socid(void)
{
    return abup_socket_mgr.abup_tcp_socket_id;
}
void abup_net_set_state(abup_int32 netsm)
{
    //abup_DebugPrint("abup_net:set sm: %d", netsm);
    abup_socket_mgr.abup_net_sm = netsm;
}
abup_int32 abup_net_get_state(void)
{
    abup_DebugPrint("abup_net:get sm: %d", abup_socket_mgr.abup_net_sm);
    return abup_socket_mgr.abup_net_sm;
}

ABUP_BOOL abup_net_pdp_is_active(void)
{
    return s_abup_is_dns_pdp_actived;
}

void abup_net_pdp_userevt_exe(void)//abup_uint8 simid
{
    if(s_abup_pdp_userevt)
        s_abup_pdp_userevt();
}

static void abup_net_pdp_reactive(void)
{
    if(abup_net_pdp_is_active() == ABUP_FALSE && s_abup_pdp_userevt)
    {
        abup_net_pdp_active(s_abup_pdp_userevt);
    }
}




abup_uint32 abup_net_pdp_active(abup_common_callback userevt)
{
    //add deal if need

    return ABUP_ERR_SUCCESS;
}

/******************************************************************************/
//  DESCRIPTION:This function handles command AT+NETAPN, set net access
//  Global resource dependence :
//  AUTHOR:
//  Note :
/******************************************************************************/
//ABUP_BOOL Abup_ProcessNETAPN(
//        ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct
//        ATC_CUR_INFO_LIST_T  *cur_cmd_info    // point to the atc cmd info
//)
ABUP_BOOL Abup_ProcessNETAPN(abup_char *apn, abup_char *uname, abup_char *upwd)
{
    //add deal  if need

    return ABUP_TRUE;//S_ATC_SUCCESS;
}


/******************************************************************************/
//  DESCRIPTION:This function handles command AT+SOCKET,
//  Global resource dependence :
//  AUTHOR:
//  Note :
/******************************************************************************/
abup_uint32 Abup_ProcessSOCKET(abup_uint8 sidx, abup_char *type, abup_char *domain, abup_uint16 port)
{
    //add deal  if need

    return 0;
}


/******************************************************************************/
//  DESCRIPTION:This function handles command AT+IPOPEN, activate PDP firstly if it is
//                         not  activated, if PDP is activated already, DNS directly
//  Global resource dependence :
//  AUTHOR:
//  Note :
/******************************************************************************/
abup_uint32 abup_ProcessIPOPEN(abup_uint8 sidx)
{
    //add deal  if need

     return 0;
}

abup_int32 abup_get_rcvd_block_size(void)
{
    return RECV_BLOCK_SIZE;
}

void abup_boot_hal_init(void)
{
    abup_DebugPrint_Ext("[ABUP_COMMON]:","abup curr ver: %s", abup_get_device_version());
    abup_get_flash_info();
    abup_hal_init_update();
    /*abup_DebugPrint("update->abup_code:");
    for (abup_uint8 cnt=0; cnt < 16;cnt++)
    {
        abup_DebugPrint("%x", update->abup_code[cnt]);
    }
    abup_DebugPrint("\r\n");*/

    abup_Debug_DeltaNew();//test only

    //abup_start_timer(5*1000, abup_register_ex, ABUP_FALSE);
    abup_start_timer(5*1000, abup_upgrade_thread_on_ex, ABUP_FALSE);
    abup_start_timer(20*1000, abup_get_new_version_ex, ABUP_FALSE);

}


void abup_write_delta_to_flash(void)
{
    abup_DebugPrint("abup_write_delta_to_flash");
    //add deal

}

#endif


