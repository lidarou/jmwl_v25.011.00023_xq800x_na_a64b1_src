#!/usr/bin/env python3
#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

#import Crypto.PublicKey.RSA
#import Crypto.Random
#import rsa
import os
import sys
import numpy as np
from cryptography.hazmat.backends.openssl.backend import backend
from cryptography.hazmat.primitives.serialization import load_pem_public_key
from Crypto.PublicKey import RSA
from Crypto.Util import asn1
from Crypto.PublicKey import RSA
from base64 import b64decode
from genHash import genHashfile

RSA_KEY_LEN = 3072
RSA_KEY_LEN_BYTE = 384

aic_pubkey = {
    'n_len': 1,
    'e_len': 0,
    'e': 0,
    'n': 1,
}

def convert_pubkey_to_aic_pubkey(in_path, out_path):
    #x = Crypto.PublicKey.RSA.generate(3072)
    with open(in_path, 'rb') as f:
        key = RSA.import_key(f.read())

    #获取e值，len做byte对齐
    e = key.e
    len = e.bit_length()
    len = (len + 0x1f) & ~0x1f
    len = int(len / 8)
    aic_pubkey['e'] = e.to_bytes(len, byteorder='little')
    aic_pubkey['e_len'] = len.to_bytes(4, byteorder = 'little')

    n = key.n
    n_len = int(n.bit_length()/8) + 1
    n_array_len = (n_len + 3) & ~0x3
    n_array = n.to_bytes(n_array_len, byteorder = 'little')
    aic_pubkey['n'] = n_array
    aic_pubkey['n_len'] = n_len.to_bytes(4, byteorder = 'little')

    file = open(out_path, "wb")
    for key in aic_pubkey :
        file.write(aic_pubkey[key])
    file.close()
    print(aic_pubkey)

    #返回pubkey文件路径
    return out_path

##参数1：输入的pubkey，参数2：输出转换后的pubkey, 参数3：转换后pubkey的hash值
def main():
    if len(sys.argv) < 3 :
        return
    pubkey_path = convert_pubkey_to_aic_pubkey(sys.argv[1], sys.argv[2])
    hash_file =  sys.argv[3]
    genHashfile(pubkey_path, hash_file)

if __name__ == '__main__':
    main()