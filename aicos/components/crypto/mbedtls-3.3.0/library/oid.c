/**
 * \file oid.c
 *
 * \brief Object Identifier (OID) database
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/* Auto gen by tool, don't change it. */
#define AIC_THIS_FILE    0x100c80
#define LOG_MODULE    AMOD_SYS

#include "common.h"

#if defined(MBEDTLS_OID_C)

#include "mbedtls/oid.h"
#include "mbedtls/rsa.h"
#include "mbedtls/error.h"

#include "mbedtls/legacy_or_psa.h"

#include <stdio.h>
#include <string.h>

#include "mbedtls/platform.h"

/*
 * Macro to automatically add the size of #define'd OIDs
 */
#define ADD_LEN(s)      s, MBEDTLS_OID_SIZE(s)

/*
 * Macro to generate mbedtls_oid_descriptor_t
 */
#if !defined(MBEDTLS_X509_REMOVE_INFO)
#define OID_DESCRIPTOR(s, name, description)  { ADD_LEN(s), name, description }
#define NULL_OID_DESCRIPTOR                   { NULL, 0, NULL, NULL }
#else
#define OID_DESCRIPTOR(s, name, description)  { ADD_LEN(s) }
#define NULL_OID_DESCRIPTOR                   { NULL, 0 }
#endif

/*
 * Macro to generate an internal function for oid_XXX_from_asn1() (used by
 * the other functions)
 */
#define FN_OID_TYPED_FROM_ASN1( TYPE_T, NAME, LIST )                    \
    static const TYPE_T * oid_ ## NAME ## _from_asn1(                   \
                                      const mbedtls_asn1_buf *oid )     \
    {                                                                   \
        const TYPE_T *p = (LIST);                                       \
        const mbedtls_oid_descriptor_t *cur =                           \
            (const mbedtls_oid_descriptor_t *) p;                       \
        if( p == NULL || oid == NULL ) return( NULL );                  \
        while( cur->asn1 != NULL ) {                                    \
            if( cur->asn1_len == oid->len &&                            \
                memcmp( cur->asn1, oid->p, oid->len ) == 0 ) {          \
                return( p );                                            \
            }                                                           \
            p++;                                                        \
            cur = (const mbedtls_oid_descriptor_t *) p;                 \
        }                                                               \
        return( NULL );                                                 \
    }

#if !defined(MBEDTLS_X509_REMOVE_INFO)
/*
 * Macro to generate a function for retrieving a single attribute from the
 * descriptor of an mbedtls_oid_descriptor_t wrapper.
 */
#define FN_OID_GET_DESCRIPTOR_ATTR1(FN_NAME, TYPE_T, TYPE_NAME, ATTR1_TYPE, ATTR1) \
int FN_NAME( const mbedtls_asn1_buf *oid, ATTR1_TYPE * ATTR1 )                  \
{                                                                       \
    const TYPE_T *data = oid_ ## TYPE_NAME ## _from_asn1( oid );        \
    if( data == NULL ) return( MBEDTLS_ERR_OID_NOT_FOUND );            \
    *ATTR1 = data->descriptor.ATTR1;                                    \
    return( 0 );                                                        \
}
#endif /* MBEDTLS_X509_REMOVE_INFO */

/*
 * Macro to generate a function for retrieving a single attribute from an
 * mbedtls_oid_descriptor_t wrapper.
 */
#define FN_OID_GET_ATTR1(FN_NAME, TYPE_T, TYPE_NAME, ATTR1_TYPE, ATTR1) \
int FN_NAME( const mbedtls_asn1_buf *oid, ATTR1_TYPE * ATTR1 )                  \
{                                                                       \
    const TYPE_T *data = oid_ ## TYPE_NAME ## _from_asn1( oid );        \
    if( data == NULL ) return( MBEDTLS_ERR_OID_NOT_FOUND );            \
    *ATTR1 = data->ATTR1;                                               \
    return( 0 );                                                        \
}

/*
 * Macro to generate a function for retrieving two attributes from an
 * mbedtls_oid_descriptor_t wrapper.
 */
#define FN_OID_GET_ATTR2(FN_NAME, TYPE_T, TYPE_NAME, ATTR1_TYPE, ATTR1,     \
                         ATTR2_TYPE, ATTR2)                                 \
int FN_NAME( const mbedtls_asn1_buf *oid, ATTR1_TYPE * ATTR1,               \
                                          ATTR2_TYPE * ATTR2 )              \
{                                                                           \
    const TYPE_T *data = oid_ ## TYPE_NAME ## _from_asn1( oid );            \
    if( data == NULL ) return( MBEDTLS_ERR_OID_NOT_FOUND );                 \
    *(ATTR1) = data->ATTR1;                                                 \
    *(ATTR2) = data->ATTR2;                                                 \
    return( 0 );                                                            \
}

/*
 * Macro to generate a function for retrieving the OID based on a single
 * attribute from a mbedtls_oid_descriptor_t wrapper.
 */
#define FN_OID_GET_OID_BY_ATTR1(FN_NAME, TYPE_T, LIST, ATTR1_TYPE, ATTR1)   \
int FN_NAME( ATTR1_TYPE ATTR1, const char **oid, size_t *olen )             \
{                                                                           \
    const TYPE_T *cur = (LIST);                                             \
    while( cur->descriptor.asn1 != NULL ) {                                 \
        if( cur->ATTR1 == (ATTR1) ) {                                       \
            *oid = cur->descriptor.asn1;                                    \
            *olen = cur->descriptor.asn1_len;                               \
            return( 0 );                                                    \
        }                                                                   \
        cur++;                                                              \
    }                                                                       \
    return( MBEDTLS_ERR_OID_NOT_FOUND );                                    \
}

/*
 * Macro to generate a function for retrieving the OID based on two
 * attributes from a mbedtls_oid_descriptor_t wrapper.
 */
#define FN_OID_GET_OID_BY_ATTR2(FN_NAME, TYPE_T, LIST, ATTR1_TYPE, ATTR1,   \
                                ATTR2_TYPE, ATTR2)                          \
int FN_NAME( ATTR1_TYPE ATTR1, ATTR2_TYPE ATTR2, const char **oid ,         \
             size_t *olen )                                                 \
{                                                                           \
    const TYPE_T *cur = (LIST);                                             \
    while( cur->descriptor.asn1 != NULL ) {                                 \
        if( cur->ATTR1 == (ATTR1) && cur->ATTR2 == (ATTR2) ) {              \
            *oid = cur->descriptor.asn1;                                    \
            *olen = cur->descriptor.asn1_len;                               \
            return( 0 );                                                    \
        }                                                                   \
        cur++;                                                              \
    }                                                                       \
    return( MBEDTLS_ERR_OID_NOT_FOUND );                                   \
}

/*
 * For X520 attribute types
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    const char          *short_name;
} oid_x520_attr_t;

static const oid_x520_attr_t oid_x520_attr_type[] =
{
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_CN,          "ac",               "CN" ),
        "CN",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_COUNTRY,     "ac",              "C" ),
        "C",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_LOCALITY,    "al",                 "L" ),
        "L",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_STATE,       "as",                    "S" ),
        "ST",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_ORGANIZATION,"ao",         "O" ),
        "O",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_ORG_UNIT,    "aoU",   "OU" ),
        "OU",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS9_EMAIL,    "eA",                   "Ea" ),
        "eA",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_SERIAL_NUMBER,"as",            "Sn" ),
        "sN",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_POSTAL_ADDRESS,"ap",          "Pa" ),
        "pA",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_POSTAL_CODE, "ap",               "Pc" ),
        "p",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_SUR_NAME,    "as",                  "S" ),
        "SN",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_GIVEN_NAME,  "ag",                "Gn" ),
        "GN",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_INITIALS,    "ai",                 "I" ),
        "i",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_GENERATION_QUALIFIER, "ag", "G" ),
        "g",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_TITLE,       "at",                    "T" ),
        "t",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_DN_QUALIFIER,"ad",              "Dnq" ),
        "d",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_PSEUDONYM,   "ap",                "P" ),
        "p",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_UID,            "u",                         "UI" ),
        "u",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DOMAIN_COMPONENT, "dC",           "Dc" ),
        "DC",
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_AT_UNIQUE_IDENTIFIER, "au",    "UI" ),
        "u",
    },
    {
        NULL_OID_DESCRIPTOR,
        NULL,
    }
};

FN_OID_TYPED_FROM_ASN1(oid_x520_attr_t, x520_attr, oid_x520_attr_type)
FN_OID_GET_ATTR1(mbedtls_oid_get_attr_short_name, oid_x520_attr_t, x520_attr, const char *, short_name)

/*
 * For X509 extensions
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    int                 ext_type;
} oid_x509_ext_t;

static const oid_x509_ext_t oid_x509_ext[] =
{
    {
        OID_DESCRIPTOR( MBEDTLS_OID_BASIC_CONSTRAINTS,    "cb",    "BC" ),
        MBEDTLS_OID_X509_EXT_BASIC_CONSTRAINTS,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_KEY_USAGE,            "ck",            "KU" ),
        MBEDTLS_OID_X509_EXT_KEY_USAGE,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EXTENDED_KEY_USAGE,   "ce",         "EKU" ),
        MBEDTLS_OID_X509_EXT_EXTENDED_KEY_USAGE,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_SUBJECT_ALT_NAME,     "cs",      "SAN" ),
        MBEDTLS_OID_X509_EXT_SUBJECT_ALT_NAME,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_NS_CERT_TYPE,         "nc",      "NCT" ),
        MBEDTLS_OID_X509_EXT_NS_CERT_TYPE,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_CERTIFICATE_POLICIES, "cc",  "CP" ),
        MBEDTLS_OID_X509_EXT_CERTIFICATE_POLICIES,
    },
    {
        NULL_OID_DESCRIPTOR,
        0,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_x509_ext_t, x509_ext, oid_x509_ext)
FN_OID_GET_ATTR1(mbedtls_oid_get_x509_ext_type, oid_x509_ext_t, x509_ext, int, ext_type)

#if !defined(MBEDTLS_X509_REMOVE_INFO)
static const mbedtls_oid_descriptor_t oid_ext_key_usage[] =
{
    OID_DESCRIPTOR( MBEDTLS_OID_SERVER_AUTH,      "id-kp-serverAuth",      "TLS Web Server Authentication" ),
    OID_DESCRIPTOR( MBEDTLS_OID_CLIENT_AUTH,      "id-kp-clientAuth",      "TLS Web Client Authentication" ),
    OID_DESCRIPTOR( MBEDTLS_OID_CODE_SIGNING,     "id-kp-codeSigning",     "Code Signing" ),
    OID_DESCRIPTOR( MBEDTLS_OID_EMAIL_PROTECTION, "id-kp-emailProtection", "E-mail Protection" ),
    OID_DESCRIPTOR( MBEDTLS_OID_TIME_STAMPING,    "id-kp-timeStamping",    "Time Stamping" ),
    OID_DESCRIPTOR( MBEDTLS_OID_OCSP_SIGNING,     "id-kp-OCSPSigning",     "OCSP Signing" ),
    OID_DESCRIPTOR( MBEDTLS_OID_WISUN_FAN,        "id-kp-wisun-fan-device", "Wi-SUN Alliance Field Area Network (FAN)" ),
    NULL_OID_DESCRIPTOR,
};

FN_OID_TYPED_FROM_ASN1(mbedtls_oid_descriptor_t, ext_key_usage, oid_ext_key_usage)
FN_OID_GET_ATTR1(mbedtls_oid_get_extended_key_usage, mbedtls_oid_descriptor_t, ext_key_usage, const char *, description)

static const mbedtls_oid_descriptor_t oid_certificate_policies[] =
{
    OID_DESCRIPTOR( MBEDTLS_OID_ANY_POLICY,      "anyPolicy",       "Any Policy" ),
    NULL_OID_DESCRIPTOR,
};

FN_OID_TYPED_FROM_ASN1(mbedtls_oid_descriptor_t, certificate_policies, oid_certificate_policies)
FN_OID_GET_ATTR1(mbedtls_oid_get_certificate_policies, mbedtls_oid_descriptor_t, certificate_policies, const char *, description)
#endif /* MBEDTLS_X509_REMOVE_INFO */

/*
 * For SignatureAlgorithmIdentifier
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_md_type_t           md_alg;
    mbedtls_pk_type_t           pk_alg;
} oid_sig_alg_t;

static const oid_sig_alg_t oid_sig_alg[] =
{
#if defined(MBEDTLS_RSA_C)
#if defined(MBEDTLS_HAS_ALG_MD5_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_MD5,        "mWR",     "RwM" ),
        MBEDTLS_MD_MD5,      MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_MD5_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_SHA1,       "s1wR",   "RWS1" ),
        MBEDTLS_MD_SHA1,     MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_224_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_SHA224,     "s2WR",  "RwS2" ),
        MBEDTLS_MD_SHA224,   MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_224_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_SHA256,     "s3WR",  "RwS3" ),
        MBEDTLS_MD_SHA256,   MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_SHA384,     "s4WR",  "Rws4" ),
        MBEDTLS_MD_SHA384,   MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_SHA512,     "s5WR",  "RwS5" ),
        MBEDTLS_MD_SHA512,   MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_RSA_SHA_OBS,      "sWR",   "RWS" ),
        MBEDTLS_MD_SHA1,     MBEDTLS_PK_RSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA */
#endif /* MBEDTLS_RSA_C */
#if defined(MBEDTLS_ECDSA_C)
#if defined(MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_ECDSA_SHA1,       "eWS",      "EwS" ),
        MBEDTLS_MD_SHA1,     MBEDTLS_PK_ECDSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_224_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_ECDSA_SHA224,     "ewS",    "EWS2" ),
        MBEDTLS_MD_SHA224,   MBEDTLS_PK_ECDSA,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_ECDSA_SHA256,     "eWS",    "EWS" ),
        MBEDTLS_MD_SHA256,   MBEDTLS_PK_ECDSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_ECDSA_SHA384,     "ews3",    "EWS3" ),
        MBEDTLS_MD_SHA384,   MBEDTLS_PK_ECDSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_ECDSA_SHA512,     "ews5",    "EWS5" ),
        MBEDTLS_MD_SHA512,   MBEDTLS_PK_ECDSA,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA */
#endif /* MBEDTLS_ECDSA_C */
#if defined(MBEDTLS_RSA_C)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_RSASSA_PSS,        "RP",           "RP" ),
        MBEDTLS_MD_NONE,     MBEDTLS_PK_RSASSA_PSS,
    },
#endif /* MBEDTLS_RSA_C */
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_MD_NONE, MBEDTLS_PK_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_sig_alg_t, sig_alg, oid_sig_alg)

#if !defined(MBEDTLS_X509_REMOVE_INFO)
FN_OID_GET_DESCRIPTOR_ATTR1(mbedtls_oid_get_sig_alg_desc, oid_sig_alg_t, sig_alg, const char *, description)
#endif

FN_OID_GET_ATTR2(mbedtls_oid_get_sig_alg, oid_sig_alg_t, sig_alg, mbedtls_md_type_t, md_alg, mbedtls_pk_type_t, pk_alg)
FN_OID_GET_OID_BY_ATTR2(mbedtls_oid_get_oid_by_sig_alg, oid_sig_alg_t, oid_sig_alg, mbedtls_pk_type_t, pk_alg, mbedtls_md_type_t, md_alg)

/*
 * For PublicKeyInfo (PKCS1, RFC 5480)
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_pk_type_t           pk_alg;
} oid_pk_alg_t;

static const oid_pk_alg_t oid_pk_alg[] =
{
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS1_RSA,           "r",    "R" ),
        MBEDTLS_PK_RSA,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_ALG_UNRESTRICTED, "e",   "E" ),
        MBEDTLS_PK_ECKEY,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_ALG_ECDH,         "e",          "E" ),
        MBEDTLS_PK_ECKEY_DH,
    },
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_PK_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_pk_alg_t, pk_alg, oid_pk_alg)
FN_OID_GET_ATTR1(mbedtls_oid_get_pk_alg, oid_pk_alg_t, pk_alg, mbedtls_pk_type_t, pk_alg)
FN_OID_GET_OID_BY_ATTR1(mbedtls_oid_get_oid_by_pk_alg, oid_pk_alg_t, oid_pk_alg, mbedtls_pk_type_t, pk_alg)

#if defined(MBEDTLS_ECP_C)
/*
 * For namedCurve (RFC 5480)
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_ecp_group_id        grp_id;
} oid_ecp_grp_t;

static const oid_ecp_grp_t oid_ecp_grp[] =
{
#if defined(MBEDTLS_ECP_DP_SECP192R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP192R1, "s",    "s" ),
        MBEDTLS_ECP_DP_SECP192R1,
    },
#endif /* MBEDTLS_ECP_DP_SECP192R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP224R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP224R1, "s2",    "s2" ),
        MBEDTLS_ECP_DP_SECP224R1,
    },
#endif /* MBEDTLS_ECP_DP_SECP224R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP256R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP256R1, "s3",    "s3" ),
        MBEDTLS_ECP_DP_SECP256R1,
    },
#endif /* MBEDTLS_ECP_DP_SECP256R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP384R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP384R1, "s3",    "s3" ),
        MBEDTLS_ECP_DP_SECP384R1,
    },
#endif /* MBEDTLS_ECP_DP_SECP384R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP521R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP521R1, "s5",    "s5" ),
        MBEDTLS_ECP_DP_SECP521R1,
    },
#endif /* MBEDTLS_ECP_DP_SECP521R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP192K1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP192K1, "s1k",    "s1k" ),
        MBEDTLS_ECP_DP_SECP192K1,
    },
#endif /* MBEDTLS_ECP_DP_SECP192K1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP224K1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP224K1, "s2k",    "s2k" ),
        MBEDTLS_ECP_DP_SECP224K1,
    },
#endif /* MBEDTLS_ECP_DP_SECP224K1_ENABLED */
#if defined(MBEDTLS_ECP_DP_SECP256K1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_SECP256K1, "s3k",    "s3k" ),
        MBEDTLS_ECP_DP_SECP256K1,
    },
#endif /* MBEDTLS_ECP_DP_SECP256K1_ENABLED */
#if defined(MBEDTLS_ECP_DP_BP256R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_BP256R1,   "bp2","bp2" ),
        MBEDTLS_ECP_DP_BP256R1,
    },
#endif /* MBEDTLS_ECP_DP_BP256R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_BP384R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_BP384R1,   "bp3","bp3" ),
        MBEDTLS_ECP_DP_BP384R1,
    },
#endif /* MBEDTLS_ECP_DP_BP384R1_ENABLED */
#if defined(MBEDTLS_ECP_DP_BP512R1_ENABLED)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_EC_GRP_BP512R1,   "bp5","bp5" ),
        MBEDTLS_ECP_DP_BP512R1,
    },
#endif /* MBEDTLS_ECP_DP_BP512R1_ENABLED */
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_ECP_DP_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_ecp_grp_t, grp_id, oid_ecp_grp)
FN_OID_GET_ATTR1(mbedtls_oid_get_ec_grp, oid_ecp_grp_t, grp_id, mbedtls_ecp_group_id, grp_id)
FN_OID_GET_OID_BY_ATTR1(mbedtls_oid_get_oid_by_ec_grp, oid_ecp_grp_t, oid_ecp_grp, mbedtls_ecp_group_id, grp_id)
#endif /* MBEDTLS_ECP_C */

#if defined(MBEDTLS_CIPHER_C)
/*
 * For PKCS#5 PBES2 encryption algorithm
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_cipher_type_t       cipher_alg;
} oid_cipher_alg_t;

static const oid_cipher_alg_t oid_cipher_alg[] =
{
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DES_CBC,              "dC",       "DC" ),
        MBEDTLS_CIPHER_DES_CBC,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DES_EDE3_CBC,         "d3", "D3" ),
        MBEDTLS_CIPHER_DES_EDE3_CBC,
    },
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_CIPHER_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_cipher_alg_t, cipher_alg, oid_cipher_alg)
FN_OID_GET_ATTR1(mbedtls_oid_get_cipher_alg, oid_cipher_alg_t, cipher_alg, mbedtls_cipher_type_t, cipher_alg)
#endif /* MBEDTLS_CIPHER_C */

/*
 * For digestAlgorithm
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_md_type_t           md_alg;
} oid_md_alg_t;

static const oid_md_alg_t oid_md_alg[] =
{
#if defined(MBEDTLS_HAS_ALG_MD5_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_MD5,       "m",       "M" ),
        MBEDTLS_MD_MD5,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_SHA1,      "s1",      "S1" ),
        MBEDTLS_MD_SHA1,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_224_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_SHA224,    "s2",    "S2" ),
        MBEDTLS_MD_SHA224,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_SHA256,    "s3",    "S3" ),
        MBEDTLS_MD_SHA256,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_SHA384,    "s4",    "S4" ),
        MBEDTLS_MD_SHA384,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_SHA512,    "s5",    "S5" ),
        MBEDTLS_MD_SHA512,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_RIPEMD160_VIA_LOWLEVEL_OR_PSA)
    {
     OID_DESCRIPTOR( MBEDTLS_OID_DIGEST_ALG_RIPEMD160, "r", "R6" ),
        MBEDTLS_MD_RIPEMD160,
    },
#endif
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_MD_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_md_alg_t, md_alg, oid_md_alg)
FN_OID_GET_ATTR1(mbedtls_oid_get_md_alg, oid_md_alg_t, md_alg, mbedtls_md_type_t, md_alg)
FN_OID_GET_OID_BY_ATTR1(mbedtls_oid_get_oid_by_md, oid_md_alg_t, oid_md_alg, mbedtls_md_type_t, md_alg)

/*
 * For HMAC digestAlgorithm
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_md_type_t           md_hmac;
} oid_md_hmac_t;

static const oid_md_hmac_t oid_md_hmac[] =
{
#if defined(MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_HMAC_SHA1,      "hs",      "h1"/*"hmacSHA1",      "HMAC-SHA-1"*/ ),
        MBEDTLS_MD_SHA1,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_1_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_224_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_HMAC_SHA224,    "hs",    "h2"/*"hmacSHA224",    "HMAC-SHA-224"*/ ),
        MBEDTLS_MD_SHA224,
    },
#endif
#if defined(MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_HMAC_SHA256,    "hs",    "h3"/*"hmacSHA256",    "HMAC-SHA-256"*/ ),
        MBEDTLS_MD_SHA256,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_256_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_HMAC_SHA384,    "hs",    "h4"/*"hmacSHA384",    "HMAC-SHA-384"*/ ),
        MBEDTLS_MD_SHA384,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_384_VIA_LOWLEVEL_OR_PSA */
#if defined(MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA)
    {
        OID_DESCRIPTOR( MBEDTLS_OID_HMAC_SHA512,    "hs",    "h5"/*"hmacSHA512",    "HMAC-SHA-512"*/ ),
        MBEDTLS_MD_SHA512,
    },
#endif /* MBEDTLS_HAS_ALG_SHA_512_VIA_LOWLEVEL_OR_PSA */
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_MD_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_md_hmac_t, md_hmac, oid_md_hmac)
FN_OID_GET_ATTR1(mbedtls_oid_get_md_hmac, oid_md_hmac_t, md_hmac, mbedtls_md_type_t, md_hmac)

#if defined(MBEDTLS_PKCS12_C)
/*
 * For PKCS#12 PBEs
 */
typedef struct {
    mbedtls_oid_descriptor_t    descriptor;
    mbedtls_md_type_t           md_alg;
    mbedtls_cipher_type_t       cipher_alg;
} oid_pkcs12_pbe_alg_t;

static const oid_pkcs12_pbe_alg_t oid_pkcs12_pbe_alg[] =
{
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS12_PBE_SHA1_DES3_EDE_CBC, "pSD", "PS3"/* "pbeWithSHAAnd3-KeyTripleDES-CBC", "PBE with SHA1 and 3-Key 3DES" */),
        MBEDTLS_MD_SHA1,      MBEDTLS_CIPHER_DES_EDE3_CBC,
    },
    {
        OID_DESCRIPTOR( MBEDTLS_OID_PKCS12_PBE_SHA1_DES2_EDE_CBC, "pSD", "PS3"/*"pbeWithSHAAnd2-KeyTripleDES-CBC", "PBE with SHA1 and 2-Key 3DES"*/ ),
        MBEDTLS_MD_SHA1,      MBEDTLS_CIPHER_DES_EDE_CBC,
    },
    {
        NULL_OID_DESCRIPTOR,
        MBEDTLS_MD_NONE, MBEDTLS_CIPHER_NONE,
    },
};

FN_OID_TYPED_FROM_ASN1(oid_pkcs12_pbe_alg_t, pkcs12_pbe_alg, oid_pkcs12_pbe_alg)
FN_OID_GET_ATTR2(mbedtls_oid_get_pkcs12_pbe_alg, oid_pkcs12_pbe_alg_t, pkcs12_pbe_alg, mbedtls_md_type_t, md_alg, mbedtls_cipher_type_t, cipher_alg)
#endif /* MBEDTLS_PKCS12_C */

#define OID_SAFE_SNPRINTF                               \
    do {                                                \
        if( ret < 0 || (size_t) ret >= n )              \
            return( MBEDTLS_ERR_OID_BUF_TOO_SMALL );    \
                                                        \
        n -= (size_t) ret;                              \
        p += (size_t) ret;                              \
    } while( 0 )

/* Return the x.y.z.... style numeric string for the given OID */
int mbedtls_oid_get_numeric_string( char *buf, size_t size,
                            const mbedtls_asn1_buf *oid )
{
    int ret = MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED;
    size_t i, n;
    unsigned int value;
    char *p;

    p = buf;
    n = size;

    /* First byte contains first two dots */
    if( oid->len > 0 )
    {
        ret = mbedtls_snprintf( p, n, "%d.%d", oid->p[0] / 40, oid->p[0] % 40 );
        OID_SAFE_SNPRINTF;
    }

    value = 0;
    for( i = 1; i < oid->len; i++ )
    {
        /* Prevent overflow in value. */
        if( ( ( value << 7 ) >> 7 ) != value )
            return( MBEDTLS_ERR_OID_BUF_TOO_SMALL );

        value <<= 7;
        value += oid->p[i] & 0x7F;

        if( !( oid->p[i] & 0x80 ) )
        {
            /* Last byte */
            ret = mbedtls_snprintf( p, n, ".%u", value );
            OID_SAFE_SNPRINTF;
            value = 0;
        }
    }

    return( (int) ( size - n ) );
}

#endif /* MBEDTLS_OID_C */
