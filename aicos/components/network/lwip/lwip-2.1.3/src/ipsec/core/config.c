
/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x14ba60
#define LOG_MODULE    AMOD_NET
#include "lwip/opt.h"

#if defined(EIPS) && EIPS == 1

#include "ipsec/config.h"

/* -------- put statically initialized SA/SP entries and tables here -------- */
spd_entry outbound_spd[IPSEC_MAX_SPD_ENTRIES] = {
    /*           src          src mask    dst         dst mask    inner packet protocol   src port    dst port    IPSec processing flag   SPI */
#if defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1
    {SPD_ENTRY( 127,0,0,1,   0,0,0,0,    127,0,0,1,  0,0,0,0,    IPSEC_PROTO_TCP,        0,          0,          POLICY_APPLY,           0)},
    {SPD_ENTRY( 127,0,0,1,   0,0,0,0,    127,0,0,1,  0,0,0,0,    IPSEC_PROTO_UDP,        0,          0,          POLICY_APPLY,           0)},
#endif /* defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1 */
    0
    /* {SPD_ENTRY( ... )},
    * ...
    */
};
spd_entry inbound_spd[IPSEC_MAX_SPD_ENTRIES] = {
    /*           src          src mask    dst         dst mask    inner packet protocol   src port    dst port    IPSec processing flag   SPI */
#if defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1
    {SPD_ENTRY( 127,0,0,1,   0,0,0,0,    127,0,0,1,  0,0,0,0,    IPSEC_PROTO_TCP,        0,          0,          POLICY_APPLY,           0)},
    {SPD_ENTRY( 127,0,0,1,   0,0,0,0,    127,0,0,1,  0,0,0,0,    IPSEC_PROTO_UDP,        0,          0,          POLICY_APPLY,           0)},
#endif /* defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1 */
    0
    /* {SPD_ENTRY( ... )},
    * ...
    */
};

sad_entry outbound_sad[IPSEC_MAX_SAD_ENTRIES] = {
#if defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1
    {SAD_ENTRY(
        127,0,0,1,              /* destination address          */
        0,0,0,0,                /* destination network mask     */
        0x1010,                 /* Security Parameter Index     */
        IPSEC_PROTO_ESP,         /* IPSec Protocol (AH or ESP)   */
        IPSEC_TRANSPORT,        /* IPSec Mode                   */
        IPSEC_AES_128_CBC,      /* Encryption Algorithm (followed by encryption key bytes) */
        0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33, 0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C,
        IPSEC_HMAC_MD5,         /* Authentication Algorithm (followed by authentication key bytes) */
        0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F, 0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
        0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C, 0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
        0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B, 0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
        IPV4
    )},
#endif /* defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1 */
    0
    /* {SAD_ENTRY( ... )},
    * ...
    */
};
sad_entry inbound_sad[IPSEC_MAX_SAD_ENTRIES] = {
#if defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1
    {SAD_ENTRY(
        127,0,0,1,              /* destination address          */
        0,0,0,0,                /* destination network mask     */
        0x1010,                 /* Security Parameter Index     */
        IPSEC_PROTO_ESP,        /* IPSec Protocol (AH or ESP)   */
        IPSEC_TRANSPORT,        /* IPSec Mode                   */
        IPSEC_AES_128_CBC,      /* Encryption Algorithm (followed by encryption key bytes) */
        0x02, 0xCF, 0x1E, 0x1D, 0x97, 0x96, 0xAD, 0x33, 0x95, 0xD8, 0x12, 0x67, 0x57, 0xB6, 0xFC, 0x7C,
        IPSEC_HMAC_MD5,         /* Authentication Algorithm (followed by authentication key bytes) */
        0xBF, 0x8D, 0x54, 0x47, 0x0B, 0x5F, 0x36, 0x5F, 0x14, 0xF8, 0x4C, 0x0E, 0x04, 0x6D, 0xE4, 0x05,
        0x3D, 0xE7, 0xA0, 0x5D, 0xB8, 0x47, 0xCD, 0x2C, 0x98, 0x44, 0x63, 0x20, 0x82, 0xF5, 0x88, 0x60,
        0x2E, 0xA3, 0x88, 0x5D, 0xEF, 0x9C, 0xA2, 0x1B, 0x8B, 0x46, 0xAA, 0x7C, 0xB9, 0x58, 0x89, 0x38,
        IPV4
    )},
#endif /* defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1 */
    0
    /* {SAD_ENTRY( ... )},
    * ...
    */
};

db_set_netif* db;

/* ------------------------------------------------------------------------- */

/* (owebb@umich.edu) Initialize the Embedded IPSec Security Policies and
 * Associations database. Statically defined policies and associations should
 * be added above */
void sa_sp_db_init(void) {
    spd_entry* free_inbound_sp;
    sad_entry* free_inbound_sa;
    spd_entry* free_outbound_sp;
    sad_entry* free_outbound_sa;

    /* Add any necessary initialization code for user-defined SA/SP entries here *
     * At the least, each SA should be associated with an SP. */
     (void)free_outbound_sp;
     (void)free_outbound_sa;
     (void)free_inbound_sp;
     (void)free_inbound_sa;
    /*
     * ...
     * ipsec_spd_add_sa(..., ...)
     * ...
     * --------------------------------------------------------------------------*/

    db = ipsec_spd_load_dbs(&inbound_spd[0], &outbound_spd[0], &inbound_sad[0], &outbound_sad[0]);

#if defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1
    ipsec_spd_add_sa(&(db->inbound_spd.table[0]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[0]), &(db->outbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->inbound_spd.table[1]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[1]), &(db->outbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->inbound_spd.table[2]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[2]), &(db->outbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->inbound_spd.table[3]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[3]), &(db->outbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->inbound_spd.table[4]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[4]), &(db->outbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->inbound_spd.table[5]), &(db->inbound_sad.table[0]));
    ipsec_spd_add_sa(&(db->outbound_spd.table[5]), &(db->outbound_sad.table[0]));
#endif /* defined(DUMMY_LOOPBACK_SA_SP) && DUMMY_LOOPBACK_SA_SP == 1 */
}

#endif /* defined(EIPS) && EIPS == 1 */
