
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x1e1e26
#define LOG_MODULE    AMOD_NET
#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1

#include "ipsec/ipsecdev.h"
#include "lwip/ip.h"

#if 0
struct orig_fns {
    netif_input_fn orig_input;
    netif_output_fn orig_output;
#if LWIP_IPV6
    netif_output_ip6_fn orig_output_ip6;
#endif
    netif_linkoutput_fn orig_linkoutput;
};

static struct orig_fns orig_fns_data[256];
#endif
extern db_set_netif* db;

struct pbuf * ipsecdev_input(struct pbuf* p, struct netif *inp)
{
    void * pkt;
    int ofs = 0;
    int len = 0;
    int retcode = IPSEC_STATUS_NOT_INITIALIZED;
    spd_entry* spd;
    unsigned char protocol;
    struct pbuf* new_p;

    /* check packet has data */
    if (p == NULL || p->payload == NULL) {
        LWIP_DEBUGF(IPSEC_DEBUG,
            ("ipsecdev_input: Packet has no payload. Can't pass it to higher level protocol stacks."));
        pbuf_free(p);
        return NULL;
    } else {
        if (IP_HDR_GET_VERSION(p->payload) == 6) {
            pkt = (ipsec_ip6_header *)p->payload;
            protocol = ((ipsec_ip6_header *)pkt)->nexthdr;
        } else {
            pkt = (ipsec_ip_header *)p->payload;
            protocol = ((ipsec_ip_header *)pkt)->protocol;
        }

        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: protocol 0x%x", protocol));
        /* subject to IPSec processing? */
        if (protocol == IPSEC_PROTO_ESP || protocol == IPSEC_PROTO_AH) {
            /* handle chained pbufs */
            if (p->next != NULL) {
                new_p = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
                if (!new_p) {
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: alloc pbuf fail %d tot_len %d", retcode, p->tot_len));
                    pbuf_free(p);
                    return NULL;
                }

                retcode = pbuf_copy(new_p, p);
                if (retcode != ERR_OK) {
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: pbuf copy fail %d", retcode));
                    pbuf_free(new_p);
                    pbuf_free(p);
                    return NULL;
                }

                pbuf_free(p);
                p = new_p;
            }
            retcode = ipsec_input_impl(p, p->len, &ofs, &len, db);
            if (retcode == IPSEC_STATUS_SUCCESS) {
                LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: fwd decapsulated IPsec packet"));
                goto EXIT;
            } else {
                LWIP_DEBUGF(IPSEC_DEBUG,
                    ("ipsecdev_input: error on ipsec_input() processing (retcode = %d)",
                     retcode));
                pbuf_free(p);
                return NULL;
            }
        } else {
            /* check what the policy says about non-IPsec traffic */
            spd = ipsec_spd_lookup(pkt, &db->inbound_spd) ;
            if (spd == NULL) {
                LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: no matching SPD policy found"));
                goto EXIT;
            } else {
                switch(spd->policy) {
                case POLICY_APPLY:
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: POLICY_APPLY: got non-IPsec packet which should be one"));
                    goto EXIT;
                case POLICY_DISCARD:
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: POLICY_DISCARD: dropping packet"));
                    pbuf_free(p);
                    return NULL;
                case POLICY_BYPASS:
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: POLICY_BYPASS: forwarding packet to ip_input"));
                    goto EXIT;
                    break;
                default:
                    pbuf_free(p) ;
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: IPSEC_STATUS_FAILURE: dropping packet"));
                    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_input: unknown Security Policy: dropping packet"));
                    return NULL;
                }
            }
        }
    }

EXIT:
    return p;
}

struct pbuf * ipsecdev_output(struct netif* netif, struct pbuf* p, const ip4_addr_t *ipaddr)
{
    spd_entry* relevant_sp;
    int ofs = 0;
    int len = 0;
    ipsec_status status = IPSEC_STATUS_NOT_INITIALIZED;
    ipsec_ip_header* hdr = (ipsec_ip_header*)p->payload;
    struct pbuf *esp_p = NULL;

    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: Inside IPSecdev OUTPUT"));

    /* chained pbuf check */
    if(p->next != NULL) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: can not handle chained pbuf"));
        goto OUTPUT;
    }

    /* pbuf reference check */
    if(p->ref != 1) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: can not handle pbuf->ref != 1 - p->ref == %d", p->ref));
        goto OUTPUT;
    }

    /* RFC Conform IPSec Processing */
    relevant_sp = ipsec_spd_lookup((unsigned char *)hdr, &(db->outbound_spd));
    if (relevant_sp == NULL) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: no matching SPD policy found,forwarding packet to ip_output"));
        /* free local pbuf here */
        goto OUTPUT;
    }

    switch(relevant_sp->policy) {
    case POLICY_APPLY:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: POLICY_APPLY: processing IPsec packet"));
        esp_p = pbuf_alloc(PBUF_IP, p->tot_len, PBUF_RAM);
        if (!esp_p)
            goto OUTPUT;

        esp_p->flags |= PBUF_FLAG_IPSEC;
        status = (ipsec_status)ipsec_output_impl(p, esp_p, ipsec_ntohs(hdr->len), &ofs, &len, &hdr->src, &hdr->dest, relevant_sp);

        /* todo: check length of output after ipsec is applied and remove unneeded space */
        if (status == IPSEC_STATUS_SUCCESS) {
            /* remove unused space (likely to occur in ESP mode due to variable padding.)
            ipsec_output_impl should guarantee that any extra space is at the FRONT of p->payload */
            LWIP_DEBUGF(IPSEC_DEBUG,
                        ("ipsec_output: fwd IPsec packet to HW mapped device"));
            goto OUTPUT;
        } else {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_output: error on ipsec_output() processing"));
            pbuf_free(esp_p);
            esp_p = NULL;
            return NULL;
        }
    case POLICY_DISCARD:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: POLICY_DISCARD: dropping packet"));
        break;
    case POLICY_BYPASS:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: POLICY_BYPASS: forwarding packet to ip_output"));
        //return data->orig_output(netif, p, ipaddr);
        goto OUTPUT;
    default:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: POLICY_DIRCARD: dropping packet"));
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: unknown Security Policy: dropping packet"));
    }

OUTPUT:
    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output: return = %d", status));
    if (esp_p)
        return esp_p;
    else
        return p;
}

#if LWIP_IPV6
struct pbuf * ipsecdev_output_ip6(struct netif* netif, struct pbuf* p, const ip6_addr_t *ipaddr)
{
    spd_entry* relevant_sp;
    int ofs = 0;
    int len = 0;
    ipsec_status status = IPSEC_STATUS_NOT_INITIALIZED;
    struct ipsec_ip6_addr dest_addr;
    ipsec_ip6_header* hdr = (ipsec_ip6_header*)p->payload;
    struct pbuf *esp_p = NULL;

    /* chained pbuf check */
    if(p->next != NULL) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: can not handle chained pbuf"));
        goto OUTPUT;
    }

    /* pbuf reference check */
    if(p->ref != 1) {
        LWIP_DEBUGF(IPSEC_DEBUG,
            ("ipsecdev_output_ip6: can not handle pbuf->ref != 1 - p->ref == %d", p->ref));
        goto OUTPUT;
    }

    /** backup of physical destination IP address (inner IP header may become encrypted) */
    memcpy(&dest_addr, ipaddr, sizeof(struct ipsec_ip6_addr));

    /* RFC Conform IPSec Processing */
    relevant_sp = ipsec_spd_lookup((unsigned char *)hdr, &(db->outbound_spd));
    if (relevant_sp == NULL) {
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: no matching SPD policy found"));
        goto OUTPUT;
    }

    switch(relevant_sp->policy) {
    case POLICY_APPLY:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: POLICY_APPLY: processing IPsec packet"));
        esp_p = pbuf_alloc(PBUF_IP, p->tot_len, PBUF_RAM);
        if (!esp_p)
            goto OUTPUT;

        esp_p->flags |= PBUF_FLAG_IPSEC;
        status = (ipsec_status)ipsec_output_impl(p, esp_p, ipsec_ntohs(hdr->len) + IPSEC_MIN_IP6HDR_SIZE, &ofs, &len, hdr->src, hdr->dest, relevant_sp);

        /* todo: check length of output after ipsec is applied and remove unneeded space */
        if (status == IPSEC_STATUS_SUCCESS) {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_output_ip6: fwd IPsec packet to HW mapped device"));
            goto OUTPUT;
        } else {
            LWIP_DEBUGF(IPSEC_DEBUG, ("ipsec_output_ip6: error on ipsec_output() processing"));
            pbuf_free(esp_p);
            esp_p = NULL;
            return NULL;
        }
    case POLICY_DISCARD:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: POLICY_DISCARD: dropping packet"));
        break;
    case POLICY_BYPASS:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: POLICY_BYPASS: forwarding packet to ip_output"));
        goto OUTPUT;
    default:
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: POLICY_DIRCARD: dropping packet"));
        LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: unknown Security Policy: dropping packet"));
    }

OUTPUT:
    LWIP_DEBUGF(IPSEC_DEBUG, ("ipsecdev_output_ip6: return = %d", status));
    if (esp_p)
        return esp_p;
    else
        return p;
}
#endif

#if 0
static err_t ipsecdev_linkoutput(struct netif* netif, struct pbuf* p) {
    /* intentionally leaving empty since thats how the original
    EIPS repo implemented it (i.e just forward along to the original linkoutput
    func for this netif.) */
    return netif->linkoutput(netif, p);
}
#endif

void ipsecdev_add(struct netif* netif) {
    int ethr;

    ethr = netif->flags & (NETIF_FLAG_ETHERNET | NETIF_FLAG_ETHARP);

    if (!(netif->flags & NETIF_FLAG_IPSEC))
        netif_set_flags(netif, NETIF_FLAG_IPSEC);

    if (!ethr)
        netif->ipsec_input_process = ipsecdev_input;

#if LWIP_IPV4
    netif->ipsec_output_process = ipsecdev_output;
#endif

#if LWIP_IPV6
    netif->ipsec_output_ip6_process = ipsecdev_output_ip6;
#endif
}

err_t CreateIpsecChannel(struct ipsec_channel *channel)
{
    sad_entry *sa_in, *sa_out;
    spd_entry *spd_in, *spd_out;
    static sad_entry sa1, sa2;

    ipsec_sa_config(&sa1, channel->dest, channel->dest_net, channel->spi_c,
                    IPSEC_PROTO_ESP, channel->mode, channel->seq,
                    channel->enc_alg, channel->enckey, channel->enckey_size,
                    channel->auth_alg, channel->authkey, channel->authkey_size,
                    channel->type);
    ipsec_sa_config(&sa2, channel->src, channel->src_net, channel->spi_s,
                    IPSEC_PROTO_ESP, channel->mode, channel->seq,
                    channel->enc_alg, channel->enckey, channel->enckey_size,
                    channel->auth_alg, channel->authkey, channel->authkey_size,
                    channel->type);
    spd_out = ipsec_spd_add(channel->src, channel->src_net, channel->dest,
                            channel->dest_net, channel->protocol,
                            IPSEC_HTONS(channel->src_port),
                            IPSEC_HTONS(channel->dest_port),
                            POLICY_APPLY, &db->outbound_spd, channel->type);
    if (!spd_out)
        return ERR_MEM;

    spd_in = ipsec_spd_add(channel->dest, channel->dest_net, channel->src,
                           channel->src_net, channel->protocol,
                           IPSEC_HTONS(channel->dest_port),
                           IPSEC_HTONS(channel->src_port),
                           POLICY_APPLY, &db->inbound_spd, channel->type);
    if (!spd_in) {
        ipsec_spd_del(spd_out, &db->outbound_spd);
        return ERR_MEM;
    }

    sa_out = ipsec_sad_add(&sa1, &db->outbound_sad);
    if (!sa_out) {
        ipsec_spd_del(spd_out, &db->outbound_spd);
        ipsec_spd_del(spd_in, &db->inbound_spd);
        return ERR_MEM;
    }

    sa_in = ipsec_sad_add(&sa2, &db->inbound_sad);
    if (!sa_in) {
        ipsec_spd_del(spd_out, &db->outbound_spd);
        ipsec_spd_del(spd_in, &db->inbound_spd);
        ipsec_sad_del(sa_out, &db->outbound_sad);
        return ERR_MEM;
    }

    ipsec_spd_add_sa(spd_out, sa_out);
    ipsec_spd_add_sa(spd_in, sa_in);

    return ERR_OK;
}

err_t DeleteIpsecChannel(struct ipsec_channel *channel)
{
    sad_entry *sa_in, *sa_out;
    spd_entry *spd_in, *spd_out;

    sa_out = ipsec_sad_lookup(channel->dest, channel->protocol, channel->spi_c,
                              &db->outbound_sad, channel->type);
    sa_in = ipsec_sad_lookup(channel->src, channel->protocol, channel->spi_s,
                              &db->inbound_sad, channel->type);
    spd_out = ipsec_spd_get_by_sad(sa_out, &db->outbound_spd);
    spd_in = ipsec_spd_get_by_sad(sa_in, &db->inbound_spd);

    if (sa_out)
        ipsec_sad_del(sa_out, &db->outbound_sad);
    if (sa_in)
        ipsec_sad_del(sa_in, &db->inbound_sad);
    if (spd_out)
        ipsec_spd_del(spd_out, &db->outbound_spd);
    if (spd_in)
        ipsec_spd_del(spd_in, &db->inbound_spd);

    return ERR_OK;
}

#endif /* defined(EIPS) && EIPS == 1 */
