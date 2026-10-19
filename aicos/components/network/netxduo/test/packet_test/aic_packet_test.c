
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1532b1
#define LOG_MODULE    AMOD_NET
#include <aic_osa.h>
#include <aic_init.h>
#include <aic_errno.h>
#include <aic_size.h>

#include "aic_packet_management.h"

static aos_thread_id thread_test_id;

static void aic_packet_test_task(void *input)
{
    NX_PACKET *packet_ptr1, *packet_ptr2;

    aic_packet_init(AIC_PACKET_MAX_NUM);
    packet_ptr1 = aic_alloc_packet();
    packet_ptr2 = aic_alloc_packet();

    aicx_info(0x0bdc57, "pp", "packet ptr %p %p\n", packet_ptr1, packet_ptr2);
    aic_free_packet(packet_ptr1);
    aic_free_packet(packet_ptr2);
    aic_free_packet_pool();
}

int aic_packet_test(void)
{
    thread_test_id = aos_create_thread("aic packet test",
                                       aic_packet_test_task,
                                       NULL, SIZE_1K, 0, NULL, 30,
                                       AOS_AUTO_START);

    return AIC_SUCCESS;
}
AIC_NORMAL_APP_P2_INIT_EXPORT(aic_packet_test);

