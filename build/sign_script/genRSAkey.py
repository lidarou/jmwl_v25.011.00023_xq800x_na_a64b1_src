#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import Crypto.PublicKey.RSA
import Crypto.Random
import rsa
import os
import sys
import numpy as np

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

def genrsakey(name, path):
    x = Crypto.PublicKey.RSA.generate(3072)
    a = x.exportKey("PEM")  # 生成私钥 DER PEM PVK
    b = x.publickey().exportKey("DER")   # 生成公钥

    #获取e值，len做byte对齐
    e = x.publickey().e
    len = e.bit_length()
    len = (len + 0x1f) & ~0x1f
    len = int(len / 8)
    aic_pubkey['e'] = e.to_bytes(len, byteorder='little')
    aic_pubkey['e_len'] = len.to_bytes(4, byteorder = 'little')

    n = x.publickey().n
    n_len = int(n.bit_length()/8) + 1
    n_array_len = (n_len + 3) & ~0x3
    n_array = n.to_bytes(n_array_len, byteorder = 'little')
    aic_pubkey['n'] = n_array
    aic_pubkey['n_len'] = n_len.to_bytes(4, byteorder = 'little')

    file = open(path + "/" + name + "_pub_key.pem", "wb")
    for key in aic_pubkey :
        file.write(aic_pubkey[key])
    file.close()
    print(aic_pubkey)

    isExists = os.path.exists(path)
    if not isExists:
        os.makedirs(path)
    with open(path + "/" + name + "_priv_key.pem", "wb") as x:
        x.write(a)

    #返回pubkey文件路径
    return path + "/" + name + "_pub_key.pem"

def main():
    key_name = sys.argv[1]
#    key_name = "bl"
    key_path = "./out"
    isExists = os.path.exists(key_path)
    if not isExists:
        os.makedirs(key_path)

    if len(sys.argv) > 2 :
        key_path = sys.argv[2]

    pubkey_path = genrsakey(key_name, key_path)
    hash_file =  key_path + "/" + key_name + "_pubkey_hash.bin"
    genHashfile(pubkey_path, hash_file)

if __name__ == '__main__':
    main()