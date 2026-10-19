#ifndef __AIC_NET_DIRECT_FORWARD_H__
#define __AIC_NET_DIRECT_FORWARD_H__

#include "aic_net_share_port_mgr.h"

#define DIRECT_FORWARD_BYPASS               0
#define DIRECT_FORWARD_CONSUMER             1

#define AIC_NET_MAX_FRAGS                   8

struct aic_net_fragment_forward {
    u8 in_use;

    NET_UP_PKT_DIRECT up_direct;
    NET_DOWN_PKT_DIRECT down_direct;

    u32 src_addr;
    u32 dst_addr;
    u8 proto;
    u16 ip_id;

    u32 init_timestamp;

    u8 up_frag_in_progress;
    u8 down_frag_in_progress;
};

struct aic_net_direct_forward_dev {
    struct aic_net_fragment_forward frag_arrays[AIC_NET_MAX_FRAGS];
};

int aic_net_down_link_direct_dispatch(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                      u8 netid);

void aic_net_down_link_update_dst_port(NX_PACKET *packet_ptr, u8 netid);

int aic_net_up_link_direct_dispatch(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                    u8 netid);

#endif

