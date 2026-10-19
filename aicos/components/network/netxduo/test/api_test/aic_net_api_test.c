
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x09b1bb
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>

#include "aic_net_api.h"

static aos_thread_id thread_test_id;

static int aic_ip_instance_test()
{
    int ret;
    struct aic_network_ppp aic_ppp = {0};
    struct aic_network_ip_instance aic_ip = {0};

    aic_ip.ip_name = "aic_ip_instance";
    aic_ip.default_ip_address = IP_ADDRESS(1, 2, 3, 4);
    aic_ip.default_net_mask = 0xFFFFFF00;
    aic_ip.arp_cache_mem_size = 0x400;
    aic_ip.if_type = AIC_RAM_NETWORK_LINK;
    aic_ip.packet_pool_name = "aic packet pool";
    aic_ip.packet_pool_size = 0x4000;
    aic_ip.per_packet_size = 1536 + sizeof(NX_PACKET);
    aic_ip.pri = 3;
    aic_ip.task_stack_size = SIZE_2K;
    aic_ip.bsd_owner = 1;
    aic_ip.bsd_task_pri = 2;
    aic_ip.bsd_task_stack_size = SIZE_2K;
    ret = aic_create_ip_instance(&aic_ip);
    aicx_info(0x05abbe, "pd", "[ip create] ip ptr 0x%p ret %d ", aic_ip.ip_ptr, ret);
    aicx_info(0x0c946c, "ppd", "packet pool and buf 0x%p 0x%p packet pool owner %d",
              aic_ip.packet_pool, aic_ip.packet_pool_buf,
              aic_ip.packet_owner);
    aicx_info(0x099923, "p", " bsd stack 0x%p\n", aic_ip.bsd_task_stack);
    aic_ip.arp_cache_mem_size = 0x400;
    ret = aic_enable_arp(&aic_ip);
    aicx_info(0x041c68, "dp", "[ARP CREATE] ret %d arp_cache_mem 0x%p\n",
              ret, aic_ip.arp_cache_mem);
    aic_ppp.nx_ip_ptr = aic_ip.ip_ptr;
    aic_ppp.packet_pool = aic_ip.packet_pool;
    aic_ppp.nx_ppp_name = "aic ppp test";
    aic_ppp.stack_size = SIZE_2K;
    aic_ppp.thread_priority = 6;
    aic_create_ppp(&aic_ppp);
    aicx_info(0x165235, "pp", "[AIC PPP] ppp ptr %p stack ptr %p\n", aic_ppp.nx_ppp,
              aic_ppp.stack_memory_ptr);
    aic_delete_ppp(&aic_ppp);
    aic_free_arp_cache(&aic_ip);
    ret =  aic_delete_ip_instance(&aic_ip);
    aicx_info(0x18422c, "d", "[ip delete] ret %d\n", ret);

    return AIC_SUCCESS;
}

static int aic_packet_pool_test()
{
    int ret;
    struct aic_packet_pool aic_pool = {0};

    aic_pool.per_packet_size = 1536;
    aic_pool.packet_pool_size = 0x4000;
    aic_pool.packet_pool_name = "aic packet test";
    ret = aic_create_packet_pool(&aic_pool);
    aicx_info(0x02acd5, "dpp", "[PACKET CREATE] ret %d packet pool 0x%p pool 0x%p\n",
              ret, aic_pool.packet_pool, aic_pool.packet_pool_buf);

    ret = aic_delete_packet_pool(&aic_pool);
    aicx_info(0x1a1c7f, "d", "[PACKET DELETE] ret %d\n", ret);

    return ret;
}

static void aic_net_api_test_task(void *input)
{
    aic_ip_instance_test();
    aic_packet_pool_test();
}

int aic_net_api_test(void)
{
    thread_test_id = aos_create_thread("nic api test",
                                       aic_net_api_test_task,
                                       NULL, SIZE_1K, 0, NULL, 30,
                                       AOS_AUTO_START);

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_net_api_test);

