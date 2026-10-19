
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x01da73
#define LOG_MODULE    AMOD_VENDOR
#ifdef CONFIG_FOTA_SUPPORT_ABUP
#include "abup_typedef.h"
#include "abup_debug.h"
#include "abup_net.h"
#include "abup_thread.h"
#include <aic_cmd_api.h>
#include <aic_init.h>
#include <aic_size.h>
#include <aic_osa.h>
#include "abup_flash.h"


#define ABUP_TASK_STACK_SIZE     (20*1024)
#define ABUP_TASK_QUEUE_NUM      50

abup_uint32 s_abup_task_id=-1;

aos_queue_ptr abup_sock_queue;
#define AIC_SOCK_QUEUE_NAME "sock queue"
#define AIC_SOCK_QUEUE_NUM 10
#define AIC_SOCK_WAIT_TIME 20
//#define AT_SOCK_CONNECT_TIMEOUT (15 * 1000)

extern ABUP_SOCKET_MGR_STRUCT abup_socket_mgr;

void abup_task_entry(void *thread_input);

abup_uint32 abup_task_id(void);
extern void abup_upgrade_thread_on(void);
extern void abup_get_new_version(void);
extern void abup_register(void);
extern void Abup_Entry_Downloading(void);
extern void abup_fota_monitor_exit_deal(void);
extern abup_wchar* abup_get_login_file_path(void);
extern abup_int32 Abup_Remove_File(abup_wchar *file_path);

int abup_task_create(void)
{
    int abup_thread_id=0;

    abup_DebugPrint("abup_task_create--->");

    abup_sock_queue = aos_create_queue(AIC_SOCK_QUEUE_NAME,
                                             sizeof(ABUP_SOCK_CB_MSG),
                                             AIC_SOCK_QUEUE_NUM);


    abup_thread_id = aos_create_thread("abup_sock_cb", abup_task_entry, NULL,
                      ABUP_TASK_STACK_SIZE, 0, NULL, 30, AOS_AUTO_START);
    s_abup_task_id = abup_thread_id;

    abup_DebugSwitch(1);
    abup_boot_hal_init();

    return abup_thread_id;
}

abup_uint32 abup_sock_msg_send(ABUP_SOCK_CB_MSG *p_sock_msg, abup_uint32 wait_option)
{
    return aos_send_to_queue(abup_sock_queue, (void *)p_sock_msg,
                             wait_option);
}

abup_uint32 abup_sock_msg_recv(ABUP_SOCK_CB_MSG *p_sock_msg, abup_uint32 wait_option)
{
    return aos_recv_from_queue(abup_sock_queue, (void *)p_sock_msg,
                               wait_option);
}

void abup_task_entry(void *thread_input)
{
    abup_uint32 ret = 0;
    ABUP_SOCK_CB_MSG recv_msg = {0};

    abup_DebugPrint("abup_task_entry!\n");

    while (1) {
        ret = abup_sock_msg_recv(&recv_msg, AOS_WAIT_FOREVER);
        if( AOS_QUEUE_EMPTY == ret ) {
            continue;
        } else {
            switch (recv_msg.msg_id) {
                case ABUP_SOCK_GETHOSTBYNAME:
                    abup_DebugPrint("ABUP_SOCK_GETHOSTBYNAME!\n");
                    abup_GetHostByName_Notify(&recv_msg);
                    break;

                case ABUP_SOCK_SENT:
                    abup_DebugPrint("ABUP_SOCK_SENT!\n");
                    abup_soc_callback_init(&abup_socket_mgr,ABUP_WRITE_NOTIFY);
                    abup_socket_mgr.abup_write_notify(abup_socket_mgr.abup_tcp_socket_id);
                    break;

                case ABUP_SOCK_READ:
                    abup_DebugPrint("ABUP_SOCK_READ!\n");
                    abup_soc_callback_init(&abup_socket_mgr,ABUP_READ_NOTIFY);
                    abup_socket_mgr.abup_read_notify(abup_socket_mgr.abup_tcp_socket_id);
                    break;

                case ABUP_SOCK_REG:
                    abup_DebugPrint("ABUP_SOCK_REG!\n");
                    abup_register();
                    break;

                case ABUP_SOCK_CV:
                    abup_DebugPrint("ABUP_SOCK_CV!\n");
                    abup_get_new_version();
                    break;

                case ABUP_SOCK_DL:
                    abup_DebugPrint("ABUP_SOCK_DL!\n");
                    Abup_Entry_Downloading();
                    break;

                case ABUP_SOCK_RU:
                    abup_DebugPrint("ABUP_SOCK_RU!\n");
                    abup_upgrade_thread_on();
                    break;

                case ABUP_SOCK_ME:
                    abup_DebugPrint("ABUP_SOCK_ME!\n");
                    abup_fota_monitor_exit_deal();
                    break;

                case ABUP_SOCK_CONNECTED:
                    abup_DebugPrint("ABUP_SOCK_CONNECTED!\n");
                    break;

            default:
                abup_DebugPrint("Recive a sock msg which is NOT known \n");
                break;

            }

        }
        aos_sleep(50);
    }


}

int abup_fota_test_main(int argc, char **argv)
{
    abup_DebugPrint("abup fota\n");
    abup_task_create();
    return 0;
}
CONSOLE_EXPORT_CMD(fota, abup_fota_test_main, fota test);

int abup_fota_remove_register(int argc, char **argv)
{
    abup_DebugPrint("abup fota_remove_register\n");
    Abup_Remove_File(abup_get_login_file_path());
    return 0;
}
CONSOLE_EXPORT_CMD(RM_RG, abup_fota_remove_register, RM_RG test);

int abup_fota_remove_update_info(int argc, char **argv)
{
    abup_DebugPrint("abup fota_remove_update_info\n");
    abup_get_flash_info();
    abup_hal_rst_fota();
    return 0;
}
CONSOLE_EXPORT_CMD(RM_INFO, abup_fota_remove_update_info, RM_INFO test);

int abup_fota_clr_key(int argc, char **argv)
{
    abup_DebugPrint("abup_fota_clr_key\n");
    abup_get_flash_info();
    abup_rst_key();
    return 0;
}
CONSOLE_EXPORT_CMD(RESET_KEY, abup_fota_clr_key, RESET_KEY test);
extern ABUP_BOOL abup_use_test_mid;
int abup_fota_reset_mid(int argc, char **argv)
{
    abup_DebugPrint("abup_fota_reset_mid\n");
    if (abup_use_test_mid == ABUP_FALSE)
        abup_use_test_mid = ABUP_TRUE;
    else
        abup_use_test_mid = ABUP_FALSE;
    return 0;
}
CONSOLE_EXPORT_CMD(RESET_MID, abup_fota_reset_mid, RESET_MID test);

abup_uint32 abup_task_id(void)
{
    return (abup_uint32)s_abup_task_id;
}

void abup_task_uninit(void)
{
    abup_DebugPrint("abup_task_uninit: task_id = 0x%x", s_abup_task_id);
    if (s_abup_task_id != -1)
    {
        #if 0
        MMIAPIPDP_Deactive(MMI_MODULE_COMMON);
        #endif
        s_abup_task_id = -1;
    }
}

void abup_upgrade_thread_on_ex(void)
{
    abup_int32 ret = -1;
    ABUP_SOCK_CB_MSG recv_msg = {0};

    recv_msg.msg_id = ABUP_SOCK_RU;
    ret = abup_sock_msg_send(&recv_msg, AOS_NO_WAIT);
    abup_DebugPrint_Ext("[ABUP_RU]:","send msg ret = %d!",ret);
}

void abup_get_new_version_ex(void)
{
    abup_int32 ret = -1;
    ABUP_SOCK_CB_MSG recv_msg = {0};

    recv_msg.msg_id = ABUP_SOCK_CV;
    ret = abup_sock_msg_send(&recv_msg, AOS_NO_WAIT);
    abup_DebugPrint_Ext("[ABUP_CV]:","send msg ret = %d!",ret);

}
void abup_register_ex(void)
{
    abup_int32 ret = -1;
    ABUP_SOCK_CB_MSG recv_msg = {0};

    recv_msg.msg_id = ABUP_SOCK_REG;
    ret = abup_sock_msg_send(&recv_msg, AOS_NO_WAIT);
    abup_DebugPrint_Ext("[ABUP_REG]:","send msg ret = %d!",ret);

}

#endif

