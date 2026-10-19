/* adapter_vex.h
 *
 * The VEX API.
 */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*   Module        : DDK-130_bsd                                              */
/*   Version       : 3.1                                                      */
/*   Configuration : DDK_EIP130_BSD                                           */
/*                                                                            */
/*   Date          : 2022-Feb-14                                              */
/*                                                                            */
/* Copyright (c) 2014-2022 by Rambus, Inc. and/or its subsidiaries.           */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or without         */
/* modification, are permitted provided that the following conditions are     */
/* met:                                                                       */
/*                                                                            */
/* 1. Redistributions of source code must retain the above copyright          */
/* notice, this list of conditions and the following disclaimer.              */
/*                                                                            */
/* 2. Redistributions in binary form must reproduce the above copyright       */
/* notice, this list of conditions and the following disclaimer in the        */
/* documentation and/or other materials provided with the distribution.       */
/*                                                                            */
/* 3. Neither the name of the copyright holder nor the names of its           */
/* contributors may be used to endorse or promote products derived from       */
/* this software without specific prior written permission.                   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR      */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT       */
/* HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT           */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,      */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY      */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE      */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.       */
/* -------------------------------------------------------------------------- */

#ifndef INCLUDE_GUARD_ADAPTER_VEX_H
#define INCLUDE_GUARD_ADAPTER_VEX_H

#include "c_adapter_vex.h"          // configuration

#include "basic_defs.h"             // uint32_t, inline, etc.

/*----------------------------------------------------------------------------
 * VexStatus_t
 */
typedef enum
{
    /* Operation success */
    VEX_SUCCESS = 0,

    /* Not supported */
    VEX_UNSUPPORTED = -1,

    /* No connection with HW available */
    VEX_NOT_CONNECTED = -2,

    /* Device writes (unexpectly) did not work */
    VEX_POWER_STATE_ERROR = -3,

    /* The host is not allowed to do this operation */
    VEX_OPERATION_NOT_ALLOWED = -4,

    /* Operation failed */
    VEX_OPERATION_FAILED = -5,

    /* Invalid parameter */
    VEX_INVALID_OPCODE = -6,

    /* Invalid parameter */
    VEX_INVALID_SUBCODE = -7,

    /* Invalid length */
    VEX_INVALID_LENGTH = -8,

    /* Wrong use; not depending on configuration */
    VEX_BAD_ARGUMENT = -9,

    /* No memory */
    VEX_NO_MEMORY = -10,

    /* No identity */
    VEX_NO_IDENTITY = -11,

    /* No mailbox */
    VEX_NO_MAILBOX = -12,

    /* Mailbox is in use */
    VEX_MAILBOX_IN_USE = -13,

    /* Output token response timeout */
    VEX_REPONSE_TIMEOUT = -14,

    /* Data ready check timeout */
    VEX_DATA_TIMEOUT = -15,

    /* Output token response timeout */
    VEX_LOCK_TIMEOUT = -16,

    /* Internal error */
    VEX_INTERNAL_ERROR = -17
}
VexStatus_t;


/*----------------------------------------------------------------------------
 * VexTokenOpCode_t - Token operation codes
 */
typedef enum
{
    VEXTOKEN_OPCODE_NOP              = 0,
    VEXTOKEN_OPCODE_ENCRYPTION       = 1,
    VEXTOKEN_OPCODE_HASH             = 2,
    VEXTOKEN_OPCODE_MAC              = 3,
    VEXTOKEN_OPCODE_TRNG             = 4,
    VEXTOKEN_OPCODE_SPECIALFUNCTIONS = 5,
    VEXTOKEN_OPCODE_SYMWRAP          = 6,
    VEXTOKEN_OPCODE_ASSETMANAGEMENT  = 7,
    VEXTOKEN_OPCODE_AUTH_UNLOCK      = 8,
    VEXTOKEN_OPCODE_PUBLIC_KEY       = 9,
    VEXTOKEN_OPCODE_EMMC             = 10,
    VEXTOKEN_OPCODE_EXTSERVICE       = 11,
    VEXTOKEN_OPCODE_RESERVED12       = 12,
    VEXTOKEN_OPCODE_RESERVED13       = 13,
    VEXTOKEN_OPCODE_SERVICE          = 14,
    VEXTOKEN_OPCODE_SYSTEM           = 15,
    VEXTOKEN_OPCODE_HARDWARE         = 16,
    VEXTOKEN_OPCODE_CLAIMCONTROL     = 17
} VexTokenOpCode_t;

/*----------------------------------------------------------------------------
 * VexTokenSubCode_t - Token sub codes
 */
typedef enum
{
    // General operations
    VEXTOKEN_SUBCODE_NOT_USED = 0,

    // Encryption operation
    VEXTOKEN_SUBCODE_ENCRYPT     = 0,
    VEXTOKEN_SUBCODE_AUTHENCRYPT = 1,

    // MAC operations
    VEXTOKEN_SUBCODE_MACUPDATE   = 0,
    VEXTOKEN_SUBCODE_MACGENERATE = 1,
    VEXTOKEN_SUBCODE_MACVERIFY   = 2,

    // TRNG operations
    VEXTOKEN_SUBCODE_RANDOMNUMBER = 0,
    VEXTOKEN_SUBCODE_TRNGCONFIG   = 1,
    VEXTOKEN_SUBCODE_VERIFYDRBG   = 2,
    VEXTOKEN_SUBCODE_VERIFYNRBG   = 3,
    VEXTOKEN_SUBCODE_TRNGRAWDATA  = 4,

    // Special functions operations
    VEXTOKEN_SUBCODE_SF_MILENAGE = 0,

    // Asset Management operations
    VEXTOKEN_SUBCODE_ASSETSEARCH        = 0,
    VEXTOKEN_SUBCODE_ASSETCREATE        = 1,
    VEXTOKEN_SUBCODE_ASSETLOAD          = 2,
    VEXTOKEN_SUBCODE_ASSETDELETE        = 3,
    VEXTOKEN_SUBCODE_PUBLICDATA         = 4,
    VEXTOKEN_SUBCODE_MONOTONICREAD      = 5,
    VEXTOKEN_SUBCODE_MONOTONICINCR      = 6,
    VEXTOKEN_SUBCODE_OTPDATAWRITE       = 7,
    VEXTOKEN_SUBCODE_SECURETIMER        = 8,
    VEXTOKEN_SUBCODE_PROVISIONRANDOMHUK = 9,
    VEXTOKEN_SUBCODE_CPIF_EXPORT        = 10,
    VEXTOKEN_SUBCODE_ASSETSTORERESET    = 15,

    // KeyWrap and Encrypt vector operations
    VEXTOKEN_SUBCODE_SYMKEYWRAP    = 0,
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
    VEXTOKEN_SUBCODE_ENCRYPTVECTOR = 1,
#endif

    // Authenticated Unlock operations
    VEXTOKEN_SUBCODE_AUNLOCKSTART   = 0,
    VEXTOKEN_SUBCODE_AUNLOCKVERIFY  = 1,
    VEXTOKEN_SUBCODE_SETSECUREDEBUG = 2,

    // Public key operations
    VEXTOKEN_SUBCODE_PK_NOASSETS   = 0,
    VEXTOKEN_SUBCODE_PK_WITHASSETS = 1,

    // eMMC operations
    VEXTOKEN_SUBCODE_EMMC_RDREQ      = 0,
    VEXTOKEN_SUBCODE_EMMC_RDVER      = 1,
    VEXTOKEN_SUBCODE_EMMC_RDWRCNTREQ = 2,
    VEXTOKEN_SUBCODE_EMMC_RDWRCNTVER = 3,
    VEXTOKEN_SUBCODE_EMMC_WRREQ      = 4,
    VEXTOKEN_SUBCODE_EMMC_WRVER      = 5,

    // Service operations
    VEXTOKEN_SUBCODE_REGISTERREAD     = 0,
    VEXTOKEN_SUBCODE_REGISTERWRITE    = 1,
    VEXTOKEN_SUBCODE_CLOCKSWITCH      = 2,
    VEXTOKEN_SUBCODE_ZEROOUTMAILBOX   = 3,
    VEXTOKEN_SUBCODE_SELECTOTPZERO    = 4,
    VEXTOKEN_SUBCODE_ZEROIZEOTP       = 5,
    VEXTOKEN_SUBCODE_FIRMWARECHECK    = 6,
    VEXTOKEN_SUBCODE_UPDATEROLLBACKID = 7,

    // System operations
    VEXTOKEN_SUBCODE_SYSTEMINFO            = 0,
    VEXTOKEN_SUBCODE_SELFTEST              = 1,
    VEXTOKEN_SUBCODE_RESET                 = 2,
    VEXTOKEN_SUBCODE_DEFINEUSERS           = 3,
    VEXTOKEN_SUBCODE_SLEEP                 = 4,
    VEXTOKEN_SUBCODE_RESUMEFROMSLEEP       = 5,
    VEXTOKEN_SUBCODE_HIBERNATION           = 6,
    VEXTOKEN_SUBCODE_RESUMEFROMHIBERNATION = 7,
    VEXTOKEN_SUBCODE_SETTIME               = 8,
    VEXTOKEN_SUBCODE_ENTERFIPSMODE         = 10,

    // Hardware related information
    VEXTOKEN_SUBCODE_MODULESTATUS = 0,
    VEXTOKEN_SUBCODE_EIP_OPTIONS  = 1,
    VEXTOKEN_SUBCODE_EIP_VERSION  = 2,

    // Claim control
    VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE          = 0,
    VEXTOKEN_SUBCODE_CLAIM_EXCLUSIVE_USE_OVERRULE = 1,
    VEXTOKEN_SUBCODE_CLAIM_RELEASE                = 2,
} VexTokenSubCode_t;

/*----------------------------------------------------------------------------
 * VexTokenAlgoHash_t - Hash algorithm codes
 */
typedef enum
{
    VEXTOKEN_ALGO_HASH_SHA1 = 1,
    VEXTOKEN_ALGO_HASH_SHA224,
    VEXTOKEN_ALGO_HASH_SHA256,
    VEXTOKEN_ALGO_HASH_SHA384,
    VEXTOKEN_ALGO_HASH_SHA512,
    VEXTOKEN_ALGO_HASH_SM3,
    VEXTOKEN_ALGO_HASH_SHA3_224,
    VEXTOKEN_ALGO_HASH_SHA3_256,
    VEXTOKEN_ALGO_HASH_SHA3_384,
    VEXTOKEN_ALGO_HASH_SHA3_512,
} VexTokenAlgoHash_t;

/*----------------------------------------------------------------------------
 * VexTokenAlgoMac_t - MAC algorithm codes
 */
typedef enum
{
#ifdef VEX_ENABLE_SYM_ALGO_SHA3
    /* for SHA-3 support */
    VEXTOKEN_ALGO_MAC_HMAC_SHA1 = 1,
    VEXTOKEN_ALGO_MAC_HMAC_SHA224,
    VEXTOKEN_ALGO_MAC_HMAC_SHA256,
    VEXTOKEN_ALGO_MAC_HMAC_SHA384,
    VEXTOKEN_ALGO_MAC_HMAC_SHA512,
    VEXTOKEN_ALGO_MAC_HMAC_SM3,
    VEXTOKEN_ALGO_MAC_POLY1305,
    VEXTOKEN_ALGO_MAC_AES_CMAC,
    VEXTOKEN_ALGO_MAC_AES_CBC_MAC,
    VEXTOKEN_ALGO_MAC_ARIA_CMAC,
    VEXTOKEN_ALGO_MAC_ARIA_CBC_MAC,
    VEXTOKEN_ALGO_MAC_HMAC_SHA3_224,
    VEXTOKEN_ALGO_MAC_HMAC_SHA3_256,
    VEXTOKEN_ALGO_MAC_HMAC_SHA3_384,
    VEXTOKEN_ALGO_MAC_HMAC_SHA3_512,
#else
    VEXTOKEN_ALGO_MAC_HMAC_SHA1 = 1,
    VEXTOKEN_ALGO_MAC_HMAC_SHA224,
    VEXTOKEN_ALGO_MAC_HMAC_SHA256,
    VEXTOKEN_ALGO_MAC_HMAC_SHA384,
    VEXTOKEN_ALGO_MAC_HMAC_SHA512,
    VEXTOKEN_ALGO_MAC_HMAC_SM3,
    VEXTOKEN_ALGO_MAC_AES_CMAC = 8,
    VEXTOKEN_ALGO_MAC_AES_CBC_MAC,
    VEXTOKEN_ALGO_MAC_ARIA_CMAC,
    VEXTOKEN_ALGO_MAC_ARIA_CBC_MAC,
    VEXTOKEN_ALGO_MAC_POLY1305 = 14,
#endif
} VexTokenAlgoMac_t;

/*----------------------------------------------------------------------------
 * VexTokenAlgoCipher_t - Cipher algorithm codes
 */
typedef enum
{
    VEXTOKEN_ALGO_CIPHER_AES = 0,
    VEXTOKEN_ALGO_CIPHER_DES,
    VEXTOKEN_ALGO_CIPHER_3DES,
    VEXTOKEN_ALGO_CIPHER_CHACHA20 = 7,
    VEXTOKEN_ALGO_CIPHER_SM4,
    VEXTOKEN_ALGO_CIPHER_ARIA,
} VexTokenAlgoCipher_t;

/*----------------------------------------------------------------------------
 * VexTokenModeHashMAC_t - Hash and MAC mode codes
 */
typedef enum
{
    VEXTOKEN_MODE_HASH_MAC_INIT2FINAL = 0,
    VEXTOKEN_MODE_HASH_MAC_CONT2FINAL,
    VEXTOKEN_MODE_HASH_MAC_INIT2CONT,
    VEXTOKEN_MODE_HASH_MAC_CONT2CONT,
} VexTokenModeHashMAC_t;

/*----------------------------------------------------------------------------
 * VexTokenModeCipher_t - Cipher mode codes
 */
typedef enum
{
    VEXTOKEN_MODE_CIPHER_ECB = 0,
    VEXTOKEN_MODE_CIPHER_CBC,
    VEXTOKEN_MODE_CIPHER_CTR,
    VEXTOKEN_MODE_CIPHER_ICM,
    VEXTOKEN_MODE_CIPHER_F8,
    VEXTOKEN_MODE_CIPHER_CCM,
    VEXTOKEN_MODE_CIPHER_XTS,
    VEXTOKEN_MODE_CIPHER_GCM,
    VEXTOKEN_MODE_CIPHER_CHACHA20_ENC = 0,
    VEXTOKEN_MODE_CIPHER_CHACHA20_AEAD,
} VexTokenModeCipher_t;

/*----------------------------------------------------------------------------
 * VexTokenMilenageOperation_t - Milenage operation codes
 */
typedef enum
{
    VEXTOKEN_MILENAGE_OP_SQNADMINCREATE = 0,
    VEXTOKEN_MILENAGE_OP_AUTNVERIFICATION,
    VEXTOKEN_MILENAGE_OP_AUTSGENERATION,
    VEXTOKEN_MILENAGE_OP_SQNADMINEXPORT,
    VEXTOKEN_MILENAGE_OP_CONFORMANCECHECK,
} VexTokenMilenageOperation_t;

/*----------------------------------------------------------------------------
 * VexTokenAssetLoad_t - Asset Load codes
 */
typedef enum
{
    VEXTOKEN_ASSETLOAD_IMPORT = 0,
    VEXTOKEN_ASSETLOAD_DERIVE,
    VEXTOKEN_ASSETLOAD_PLAINTEXT,
    VEXTOKEN_ASSETLOAD_RANDOM,
    VEXTOKEN_ASSETLOAD_SYMUNWRAP,
} VexTokenAssetLoad_t;

/*----------------------------------------------------------------------------
 * VexTokenPkAsset_t - PK with Assets operation codes
 */
typedef enum
{
    VEXTOKEN_PKASSET_ECDH_ECDSA_KEYCHK = 0x01,
    VEXTOKEN_PKASSET_DH_DSA_KEYCHK,
    VEXTOKEN_PKASSET_DSA_SIGN = 0x04,
    VEXTOKEN_PKASSET_DSA_VERIFY,
    VEXTOKEN_PKASSET_ECDSA_SIGN = 0x06,
    VEXTOKEN_PKASSET_ECDSA_VERIFY,
    VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN = 0x08,
    VEXTOKEN_PKASSET_RSA_PKCS1V1_5_VERIFY,
    VEXTOKEN_PKASSET_RSA_PKCS1V1_5_SIGN_CRT,
    VEXTOKEN_PKASSET_RSA_PSS_SIGN = 0x0C,
    VEXTOKEN_PKASSET_RSA_PSS_VERIFY,
    VEXTOKEN_PKASSET_RSA_PSS_SIGN_CRT,
    VEXTOKEN_PKASSET_DH_GEN_PUBKEY = 0x10,
    VEXTOKEN_PKASSET_DH_GEN_KEYPAIR,
    VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_SKEYPAIR,
    VEXTOKEN_PKASSET_DH_GEN_SHARED_SECRET_DKEYPAIR,
    VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_PUBKEY,
    VEXTOKEN_PKASSET_ECDH_ECDSA_GEN_KEYPAIR,
    VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_SKEYPAIR,
    VEXTOKEN_PKASSET_ECDH_GEN_SHARED_SECRET_DKEYPAIR,
    VEXTOKEN_PKASSET_RSA_OAEP_WRAP_STRING = 0x18,
    VEXTOKEN_PKASSET_RSA_OAEP_WRAP_HASH,
    VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_STRING,
    VEXTOKEN_PKASSET_RSA_OAEP_UNWRAP_HASH,
    VEXTOKEN_PKASSET_RSA_PKCS1V15_UNWRAP = 0x21,
    VEXTOKEN_PKASSET_RSA_PKCS1V15_WRAP,
    VEXTOKEN_PKASSET_ECC_ELGAMAL_ENCRYPT = 0x24,
    VEXTOKEN_PKASSET_ECC_ELGAMAL_DECRYPT,
    VEXTOKEN_PKASSET_CURVE25519_GEN_PUBKEY = 0x28,
    VEXTOKEN_PKASSET_CURVE25519_GEN_KEYPAIR,
    VEXTOKEN_PKASSET_CURVE25519_GEN_SHARED_SECRET,
    VEXTOKEN_PKASSET_EDDSA_GEN_PUBKEY,
    VEXTOKEN_PKASSET_EDDSA_GEN_KEYPAIR,
    VEXTOKEN_PKASSET_EDDSA_SIGN_INITIAL,
    VEXTOKEN_PKASSET_EDDSA_SIGN_UPDATE,
    VEXTOKEN_PKASSET_EDDSA_SIGN_FINAL,
    VEXTOKEN_PKASSET_EDDSA_VERIFY_INITIAL,
    VEXTOKEN_PKASSET_EDDSA_VERIFY_FINAL,
    VEXTOKEN_PKASSET_SM2DSA_SIGN,
    VEXTOKEN_PKASSET_SM2DSA_VERIFY,
    VEXTOKEN_PKASSET_SM2ENC_WRAP,
    VEXTOKEN_PKASSET_SM2ENC_UNWRAP,
    VEXTOKEN_PKASSET_SM2KEX_GEN_SHARED_SECRET,
    VEXTOKEN_PKASSET_ECIESWRAP = 0x38,
    VEXTOKEN_PKASSET_ECIESUNWRAP,
} VexTokenPkAsset_t;

/*----------------------------------------------------------------------------
 * VexTokenPkOperation_t - PK without Assets operation codes
 */
typedef enum
{
    VEXTOKEN_PK_OP_NUMLOAD = 0x01,
    VEXTOKEN_PK_OP_NUMSETN = 0x03,
    VEXTOKEN_PK_OP_MODEXPE = 0x04,
    VEXTOKEN_PK_OP_MODEXPD,
    VEXTOKEN_PK_OP_MODEXPCRT,
    VEXTOKEN_PK_OP_ECMONTMUL = 0x0A,
    VEXTOKEN_PK_OP_ECCMUL,
    VEXTOKEN_PK_OP_ECCADD,
    VEXTOKEN_PK_OP_DSASIGN,
    VEXTOKEN_PK_OP_DSAVERIFY,
    VEXTOKEN_PK_OP_ECDSASIGN,
    VEXTOKEN_PK_OP_ECDSAVERIFY,
} VexTokenPkOperation_t;


/*----------------------------------------------------------------------------
 * VexTokenSecureTimer_t - Secure Timer operation codes
 */
typedef enum
{
    VEXTOKEN_SECURETIMER_START = 0,     // (Re)Start a timer
    VEXTOKEN_SECURETIMER_STOP,          // Read elapsed time and stop the timer
    VEXTOKEN_SECURETIMER_READ,          // Read elapsed time
} VexTokenSecureTimer_t;

/*----------------------------------------------------------------------------
 * Token/HW/Algorithm related limits
 */
#define VEXTOKEN_DMA_MAXLENGTH           0x001FFFFF  // 2 MB - 1 bytes
#define VEXTOKEN_DMA_TOKENID_SIZE        4           // bytes

/*----------------------------------------------------------------------------
 * DMA data block must be an integer multiple of a work block size (in bytes)
 */
#define VEXTOKEN_DMA_ALGO_BLOCKSIZE_HASH 64
#define VEXTOKEN_DMA_ALGO_BLOCKSIZE_AES  16
#define VEXTOKEN_DMA_ALGO_BLOCKSIZE_DES  8
#define VEXTOKEN_DMA_ALGO_BLOCKSIZE_ARC4 4
#define VEXTOKEN_DMA_ALGO_BLOCKSIZE_NOP  4

/*----------------------------------------------------------------------------
 * Protected Application related limits
 */
#define PROT_APP_MAX_DATA_SIZE   (256 - (3*4))  // Mailbox Size - 3 reserved words (word 0, Identity, Prot. App Data Size)

/*----------------------------------------------------------------------------
 * Maximums for items in VexToken_Command_t
 */
#define VEXTOKEN_MAX_ASSOC_DATA_SZ  224
#define VEXTOKEN_MAX_PK_NUM_ASSETS  24

/*----------------------------------------------------------------------------
 * VexToken_Command_t - Command Token
 */
typedef struct
{
    uint32_t OpCode;                    // Service group
    uint32_t SubCode;                   // Specific service code in the group
    uint32_t fFromUserSpace;            // From User Space indication

    union
    {
        struct
        {
            const uint8_t * In_p;
            uint8_t * Out_p;
            uint32_t Size;
        } Nop;
        struct
        {
            const uint8_t * SrcData_p;
            uint8_t * DstData_p;
            uint32_t Algorithm;
            uint32_t Mode;
            uint32_t TempAssetId;
            uint32_t KeyAssetId;
            uint32_t DataSize;
            uint32_t KeySize;
            uint8_t Key[64];            // AES-XTS requires 512bit (2 * 256bit)
            uint8_t IV[16];
            uint8_t Parameter[16];      // System parameter
            uint8_t f8_IV[16];          // IV for AES f8 mode
            uint8_t f8_SaltKey[16];     // Salt key for AES f8 mode
            uint8_t f8_SaltKeySize;     // Salt key size for AES f8 mode
            uint8_t NonceLength;        // Nonce length for ChaCha20
            uint8_t fEncrypt;
            uint8_t fParameter;         // Write system parameter
        } Cipher;
        struct
        {
            const uint8_t * AAD_p;
            const uint8_t * SrcData_p;
            uint8_t * DstData_p;
            uint32_t Algorithm;
            uint32_t Mode;
            uint32_t KeyAssetId;
            uint32_t AADSize;
            uint32_t DataSize;
            uint32_t KeySize;
            uint8_t Key[32];
            uint8_t NonceHashKey[16];
            uint8_t IV[16];
            uint8_t Tag[16];
            uint8_t fEncrypt;
            uint8_t fNoRandomIV;
            uint8_t NonceHashKeySize;  // Actual Nonce/HashKey size
            uint8_t GCMMode;           // GCMMode
            uint8_t TagSize;           // Tag size
        } CipherAE;
        struct
        {
            const uint8_t * Data_p;
            uint64_t TotalMessageLength;
            uint32_t Algorithm;
            uint32_t Mode;
            uint32_t DataSize;
            uint32_t TempAssetId;
            uint8_t Digest[64];
        } Hash;
        struct
        {
            const uint8_t * Data_p;
            uint64_t TotalMessageLength;
            uint32_t Algorithm;
            uint32_t Mode;
            uint32_t TempAssetId;
            uint32_t KeyAssetId;
            uint32_t MacAssetId;
            uint32_t DataSize;
            uint32_t KeySize;
            uint32_t MacSize;
            uint8_t Key[128];
            uint8_t Mac[64];
            uint8_t TempMac[64];
        } Mac;
        struct
        {
            const uint8_t * SrcData_p;
            uint8_t * DstData_p;
            uint32_t KeyAssetId;
            uint32_t SrcDataSize;
            uint32_t DstDataSize;
            uint32_t KeySize;
            uint8_t Key[32];
            uint8_t fWrap;
            uint8_t Algorithm;
        } KeyWrap;
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
        struct
        {
            uint8_t * Data_p;
            uint32_t AssetId;
            uint32_t DataSize;
            uint16_t AssetNumber;
        } EncVector;
#endif
        struct
        {
            uint16_t FroBlockKey;
            uint16_t SampleCycles;
            uint16_t AdaptProp512Cutoff;
            uint8_t NoiseBlocks;
            uint8_t SampleDiv;
            uint8_t AutoSeed;
            uint8_t Scale;
            uint8_t RepCntCutoff;
            uint8_t AdaptProp64Cutoff;
            uint8_t fLoadStart;
            uint8_t fReseed;
            uint8_t fAria;
            uint8_t fDRBGForceSeed;
            uint8_t Seed[48];
        } TrngConfig;
        struct
        {
            uint8_t * Data_p;
            uint16_t Size;
        } Random;
        struct
        {
            const uint8_t * InputData_p;
            uint8_t * OutputData_p;
            uint32_t InputDataSize;
            uint32_t OutputDataSize;
            uint16_t Size;
            uint16_t Pattern;
            uint8_t Test;
        } DrbgPP;
        struct
        {
            const uint8_t * InputData_p;
            uint16_t Size;
            uint16_t AdaptProp512Cutoff;
            uint16_t AdaptProp512Count;
            uint8_t AdaptProp512Data;
            uint8_t AdaptProp64Cutoff;
            uint8_t AdaptProp64Count;
            uint8_t AdaptProp64Data;
            uint8_t RepCntCutoff;
            uint8_t RepCntCount;
            uint8_t RepCntData;
        } NrbgVerify;
        struct
        {
            uint32_t Operation;
            uint32_t AssetId;
            uint16_t AssetNumber;
            uint8_t fAMF_SBtest;
            uint8_t RAND[16];
            uint8_t AUTN[16];
            uint8_t SQN[6];
            uint8_t AMF[2];
            uint8_t K[16];
            uint8_t OP[16];
        } SF_Milenage;
        struct
        {
            uint32_t Operation;
            uint32_t AssetId;
            uint8_t * DataBlob_p;
            uint32_t KekAssetId;
            uint32_t DataBlobSize;
            uint32_t AssociatedDataSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } SF_MilenageExport;
        struct
        {
            uint64_t Policy;
            uint32_t Size;
            uint32_t Lifetime;
            uint8_t fAllHosts;
            uint8_t fRemoveSecure;
            uint8_t LifetimeUse;
            uint8_t fLifetimeRelative;
            uint8_t fLifetimeNoLoad;
        } AssetCreate;
        struct
        {
            uint32_t AssetId;
        } AssetDelete;
        struct
        {
            uint16_t Number;
        } AssetSearch;
        struct
        {
            uint32_t Method;
            const uint8_t * KeyBlob_p;
            uint32_t AssetId;
            uint32_t KekAssetId;
            uint32_t KeyBlobSize;
            uint32_t AssociatedDataSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } AssetLoadImport;
        struct
        {
            uint32_t Method;
            const uint8_t * Salt_p;
            const uint8_t * IV_p;
            uint32_t AssetId;
            uint32_t KdkAssetId;
            uint32_t AssociatedDataSize;
            uint32_t SaltSize;
            uint32_t IVSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
            uint8_t fCounter;
            uint8_t fRFC5869;
        } AssetLoadDerive;
        struct
        {
            uint32_t Method;
            const uint8_t * Data_p;
            uint8_t * KeyBlob_p;
            uint32_t AssetId;
            uint32_t KekAssetId;
            uint32_t DataSize;
            uint32_t AssociatedDataSize;
            uint32_t KeyBlobSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } AssetLoadPlaintext;
        struct
        {
            uint32_t Method;
            uint8_t * KeyBlob_p;
            uint32_t AssetId;
            uint32_t KekAssetId;
            uint32_t AssociatedDataSize;
            uint32_t KeyBlobSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } AssetLoadRandom;
        struct
        {
            uint32_t Method;
            const uint8_t * KeyBlob_p;
            uint32_t AssetId;
            uint32_t KekAssetId;
            uint32_t KeyBlobSize;
            uint8_t  Algorithm;
        } AssetLoadSymUnwrap;
        struct
        {
            uint32_t AssetId;
            uint16_t Address;
            uint16_t CPxSelectionBits;
        } AssetExportCPIF;
        struct
        {
            const uint8_t * KeyBlob_p;
            uint32_t KeyBlobSize;
            uint32_t AssociatedDataSize;
            uint16_t AssetNumber;
#ifdef EIP130_FW_ASSETPOLICY_V2
            uint64_t Policy;
#else
            uint16_t PolicyNumber;
#endif
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
            uint8_t fAddCRC;
        } OTPDataWrite;
        struct
        {
            const uint8_t * KeyBlob_p;
            uint32_t Identity;
            uint32_t KeyBlobSize;
            uint32_t AssociatedDataSize;
            uint16_t AssetNumber;
            uint16_t SampleCycles;
            uint16_t AdaptProp512Cutoff;
            uint8_t NoiseBlocks;
            uint8_t SampleDiv;
            uint8_t AutoSeed;
            uint8_t Scale;
            uint8_t RepCntCutoff;
            uint8_t AdaptProp64Cutoff;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
            uint8_t f128bit;
            uint8_t fAddCRC;
            uint8_t fAria;
        } ProvisionRandomHUK;
        struct
        {
            uint32_t Operation;
            uint32_t AssetId;
            uint32_t fSecond;
        } SecureTimer;
        struct
        {
            const uint8_t * Data_p;
            uint32_t AssetId;
            uint32_t DataSize;
        } PublicData;
        struct
        {
            const uint8_t * Data_p;
            uint32_t AssetId;
            uint32_t DataSize;
        } MonotonicCounter;
        struct
        {
            uint32_t Method;
            const uint8_t * HashData_p;
            uint8_t * Sign_p;
            uint64_t TotalMessageSize;
            uint32_t ModulusSizeInBits;
            uint32_t DomainAssetId;
            uint32_t KeyAssetId;
            uint32_t DigestAssetId;
            uint32_t HashDataSize;
            uint32_t SaltSize;          // RSA-PSS only
            uint32_t SignSize;
            uint32_t ExplicitDigestSize;
            uint8_t ExplicitDigest[64];
        } PkAssetSignVerify;
        struct
        {
            uint32_t Method;
            const  uint8_t * SrcData_p;
            const uint8_t * DstData_p;
            uint32_t ModulusSizeInBits;
            uint32_t DomainAssetId;
            uint32_t KeyAssetId;
            uint32_t SrcDataSize;
            uint32_t DstDataSize;
        } PkAssetEncrypt;
        struct
        {
            uint32_t Method;
            const uint8_t * KeyBlob_p;
            uint8_t * PubKey_p;
            uint32_t ModulusSizeInBits;
            uint32_t DivisorSizeInBits;
            uint32_t DomainAssetId;
            uint32_t PubKeyAssetId;
            uint32_t PrivKeyAssetId;
            uint32_t KekAssetId;
            uint32_t AssociatedDataSize;
            uint32_t KeyBlobSize;
            uint32_t PubKeySize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } PkAssetGenKey;
        struct
        {
            uint32_t Method;
            uint32_t ModulusSizeInBits;
            uint32_t DivisorSizeInBits;
            uint32_t DomainAssetId;
            uint32_t PubKeyAssetId;
            uint32_t PrivKeyAssetId;
        } PkAssetKeyCheck;
        struct
        {
            uint32_t Method;
            const uint8_t * OtherInfo_p;
            const uint8_t * LocalEphPubKey;
            uint32_t ModulusSizeInBits;
            uint32_t DivisorSizeInBits;
            uint32_t DomainAssetId;
            uint32_t PubKeyAssetId;
            uint32_t PrivKeyAssetId;
            uint32_t PubKey2AssetId;
            uint32_t PrivKey2AssetId;
            uint32_t OtherInfoSize;
            uint32_t LocalEphPubKeySize;
            uint32_t AssetIdListSize;
            uint32_t AssetIdList[VEXTOKEN_MAX_PK_NUM_ASSETS];
            uint8_t fSaveSharedSecret;
        } PkAssetGenSharedSecret;
        struct
        {
            uint32_t Method;
            const uint8_t * Data_p;
            uint32_t ModulusSizeInBits;
            uint32_t DomainAssetId;
            uint32_t KeyAssetId;
            uint32_t AssetId;
            uint32_t DataSize;
            uint32_t AdditionalInputSize;
            uint8_t AdditionalInput[208];
        } PkAssetWrap;
        struct
        {
            uint32_t Method;
            const uint8_t * Data_p;
            const uint8_t * EphPubKey_p;
            const uint8_t * OtherInfo_p;
            uint32_t ModulusSizeInBits;
            uint32_t DataSize;
            uint32_t EphPubKeySize;
            uint32_t OtherInfoSize;
            uint32_t DomainAssetId;
            uint32_t KeyAssetId;
            uint32_t EphPubKeyAssetId;
            uint32_t AssetDataId;
        } PkEcies;
        struct
        {
            const uint8_t * InData_p;
            const uint8_t * OutData_p;
            uint32_t Operation;
            uint32_t PublicExponent;
            uint32_t InDataSize;
            uint32_t OutDataSize;
            uint8_t Nwords;
            uint8_t Mwords;
            uint8_t Mmask;
            uint8_t Index;
        } PkOperation;
        struct
        {
            const uint8_t * Sign_p;
            uint32_t AuthStateAssetId;
            uint32_t AuthKeyAssetId;
            uint32_t SignSize;
            uint8_t Nonce[16];
        } AuthUnlock;
        struct
        {
            uint32_t AuthStateAssetId;
            uint32_t fSet;
        } SecureDebug;
        struct
        {
            uint32_t AssetId;
        } eMMCRdRequest;
        struct
        {
            const uint8_t * Data_p;
            uint32_t StateAssetId;
            uint32_t DataSize;
            uint32_t MacSize;
            uint8_t Mac[32];
        } eMMCRdVerifyWrReqVer;
        struct
        {
            const uint8_t * Data_p;
            uint32_t Algorithm;
            uint32_t KeyAssetId;
            uint32_t StateAssetId;
            uint32_t KeySize;
            uint32_t DataSize;
            uint32_t AssociatedDataSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } ExtService;
        struct
        {
            uint32_t Address;
            uint32_t Value;
        } Register;
        struct
        {
            uint32_t User1;
            uint32_t User2;
            uint32_t User3;
            uint32_t User4;
        } SystemDefineUsers;
        struct
        {
            uint32_t Seconds;
        } SystemSetTime;
        struct
        {
            const uint8_t * DataBlob_p;
            uint32_t StateAssetId;
            uint32_t KeyAssetId;
            uint32_t AssociatedDataSize;
            uint32_t DataBlobSize;
            uint8_t AssociatedData[VEXTOKEN_MAX_ASSOC_DATA_SZ];
        } SystemHibernation;
        struct
        {
            uint16_t On;
            uint16_t Off;
        } ClockSwitch;
        struct
        {
            const uint8_t * FirmwareImage_p;
            uint32_t FirmwareImageSize;
        }FirmwareCheck;
        struct
        {
            uint16_t RollbackID;
        } UpdateRollbackID;
    } Service;
} VexToken_Command_t;

/*----------------------------------------------------------------------------
 * VexTokenResult_t - Result codes
 */
typedef enum
{
    VEXTOKEN_RESULT_NO_ERROR              =  (0x00), // No error or warning
    VEXTOKEN_RESULT_SEQ_INVALID_TOKEN     = -(0x01), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_PARAMETER = -(0x02), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_KEYSIZE   = -(0x03), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_LENGTH    = -(0x04), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_LOCATION  = -(0x05), // Error
    VEXTOKEN_RESULT_SEQ_CLOCK_ERROR       = -(0x06), // Error
    VEXTOKEN_RESULT_SEQ_ACCESS_ERROR      = -(0x07), // Error
    VEXTOKEN_RESULT_SEQ_UNWRAP_ERROR      = -(0x0A), // Error
    VEXTOKEN_RESULT_SEQ_DATA_OVERRUN      = -(0x0B), // Error
    VEXTOKEN_RESULT_SEQ_ASSET_CHECKSUM    = -(0x0C), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_ASSET     = -(0x0D), // Error
    VEXTOKEN_RESULT_SEQ_OVERFLOW_FULL     = -(0x0E), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_ADDRESS   = -(0x0F), // Error
    VEXTOKEN_RESULT_SEQ_Z1USED            =  (0x10), // Warning
    VEXTOKEN_RESULT_SEQ_INVALID_MODULUS   = -(0x11), // Error
    VEXTOKEN_RESULT_SEQ_VERIFY_ERROR      = -(0x12), // Error
    VEXTOKEN_RESULT_SEQ_INVALID_STATE     = -(0x13), // Error
    VEXTOKEN_RESULT_SEQ_OTP_WRITE_ERROR   = -(0x14), // Error
    VEXTOKEN_RESULT_SEQ_PANIC             = -(0x1F)  // Error
} VexTokenResult_t;

/*----------------------------------------------------------------------------
 * VexToken_Result_t - Result token
 */
typedef struct
{
    int Result;

    union
    {
        struct
        {
            uint8_t IV[16];
        } Cipher;
        struct
        {
            uint8_t IV[16];
            uint8_t Tag[16];
        } CipherAE;
        struct
        {
            uint8_t Digest[64];
        } Hash;
        struct
        {
            uint8_t Mac[64];
        } Mac;
        struct
        {
            uint16_t AdaptProp512Cutoff;
            uint16_t AdaptProp512Count;
            uint8_t AdaptProp512Data;
            uint8_t AdaptProp512Fail;
            uint8_t AdaptProp64Cutoff;
            uint8_t AdaptProp64Count;
            uint8_t AdaptProp64Data;
            uint8_t AdaptProp64Fail;
            uint8_t RepCntCutoff;
            uint8_t RepCntCount;
            uint8_t RepCntData;
        } NrbgVerify;
        struct
        {
            uint8_t Token[64];
        } SF_General;
        struct
        {
            uint32_t OutputDataSize;
            uint8_t OutputData[PROT_APP_MAX_DATA_SIZE];
        } SF_ProtectedApp;
        struct
        {
            uint8_t MacKey[16];
        } SF_BluetoothLEf5;
        struct
        {
            uint32_t Size;
        } SF_Coverage;
        struct
        {
            uint32_t AssetId;
            uint8_t RES[8];
            uint8_t CK[16];
            uint8_t IK[16];
            uint8_t SQN[6];
            uint8_t AMF[2];
            uint8_t MACA[8];
            uint8_t MACS[8];
            uint8_t AUTS[14];
            uint8_t AK[6];
            uint8_t AKstar[6];
            uint8_t OPc[16];
            uint8_t EMMCause;
        } SF_Milenage;
        struct
        {
            uint32_t DataBlobSize;
        } SF_MilenageExport;
        struct
        {
            uint32_t Size;
        } KeyWrap;
#ifdef VEX_ENABLE_ENCRYPTED_VECTOR
        struct
        {
            uint32_t DataSize;
        } EncVector;
#endif
        struct
        {
            uint32_t AssetId;
        } AssetCreate;
        struct
        {
            uint32_t AssetId;
            uint32_t Size;
        } AssetSearch;
        struct
        {
            uint32_t KeyBlobSize;
        } AssetLoad;
        struct
        {
            uint32_t KeyBlobSize;
        } ProvisionRandomHUK;
        struct
        {
            uint32_t AssetId;
            uint32_t ElapsedTime;
        } SecureTimer;
        struct
        {
            uint32_t DataSize;
        } PublicData;
        struct
        {
            uint32_t DataSize;
        } MonotonicCounter;
        struct
        {
            uint32_t StateAssetId;
        } PkAssetSignVerify;
        struct
        {
            uint32_t WrappedDataSize;
        } PkAssetWrap;
        struct
        {
            uint8_t Nonce[16];
        } AuthUnlock;
        struct
        {
            uint32_t AssetId;
            uint8_t Nonce[16];
        } eMMCRdRequest;
        struct
        {
            uint8_t Mac[32];
        } eMMCRdVerifyWrReqVer;
        struct
        {
            uint8_t Mac[32];
        } ExtService;
        struct
        {
            uint32_t Value;
        } Register;
        struct
        {
            struct
            {
                uint16_t MemorySizeInBytes;
                uint8_t Major;          // 0..9
                uint8_t Minor;          // 0..9
                uint8_t Patch;          // 0..9
            } Hardware;
            struct
            {
                uint8_t RollbackId;     // 0..32
                uint8_t Major;          // 0..9
                uint8_t Minor;          // 0..9
                uint8_t Patch;          // 0..9
            } Firmware;
            struct
            {
                uint32_t Identity;
                uint8_t HostID;         // 0..7
                uint8_t NonSecure;      // 0=Secure, !0=Non-Secure
                uint8_t CryptoOfficer;  // 0=Not available, !0=Available
                uint8_t Mode;           // 0=Non-FIPS mode, 4..6=Error mode, 15=FIPS mode
                uint8_t ErrorTest;      // Self-test error (only valid when Fatal Error is caused by a self-test)
            } Self;
            struct
            {
                uint16_t ErrorLocation;
                uint8_t ErrorCode;
            } OTP;
        } SystemInfo;
        struct
        {
            uint32_t DataBlobSize;
        } SystemHibernation;
        struct
        {
            uint8_t FIPSmode;
            uint8_t NonFIPSmode;
            uint8_t CRC24Ok;
            uint8_t CRC24Busy;
            uint8_t CRC24Error;
            uint8_t FwImageWritten;
            uint8_t FwImageCheckDone;
            uint8_t FwImageAccepted;
            uint8_t FatalError;
        } ModuleStatus;
        struct
        {
            uint16_t StandardEngines;
            uint16_t CustomEngines;
            uint16_t MailboxSize;
            uint8_t nMailboxes;
            uint8_t HostId;
            uint8_t SecureHostId;
            uint8_t MasterId;
            uint8_t MyHostId;
            uint8_t Protection;
            uint8_t ProtectionAvailable;
        } EipOptions;
        struct
        {
            uint8_t MajorVersion;
            uint8_t MinorVersion;
            uint8_t PatchLevel;
            uint8_t EipNumber;
        } EipVersion;
    } Service;
} VexToken_Result_t;


/*----------------------------------------------------------------------------
 * vex_LogicalToken
 *
 * This function handles the logical tokens coming from the VAL API level.
 *
 * CommandToken_p
 *     Pointer to the buffer with the service request.
 *
 * ResultToken_p
 *     Pointer to the buffer in which the service result needs to be returned.
 *
 * Return Value:
 *     One of the VexStatus_t values.
 */
VexStatus_t
vex_LogicalToken(
        VexToken_Command_t * const CommandToken_p,
        VexToken_Result_t * const ResultToken_p);


/*----------------------------------------------------------------------------
 * vex_Init
 *
 * This function initializes the VEX Adapter.
 *
*
 * Return Value:
 *     0 if successful, otherwise <0
 */
int
vex_Init(void);


/*----------------------------------------------------------------------------
 * vex_UnInit
 *
 * This function un-initializes the VEX Adapter.
 *
 * Return Value:
 *     None
 */
void
vex_UnInit(void);


#endif /* INCLUDE_GUARD_ADAPTER_VEX_H */

/* end of file adapter_vex.h */
