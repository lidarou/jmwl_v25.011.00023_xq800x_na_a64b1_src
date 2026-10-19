#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import os
import argparse
import hashlib

def get_file_hash(bin_file):
    f = open(bin_file,"rb")
    thehash = hashlib.sha256()
    theline = f.readline()

    while(theline):
        thehash.update(theline)
        theline = f.readline()

    return thehash.hexdigest()

def genHashfile(file, out):
    hash_value = get_file_hash(file)
    hash_bin = bytes([int(hash_value[x:x+2],16)for x in range(0,len(hash_value),2)])
    fileOutput = open(out, 'wb')
    fileOutput.write(hash_bin)
    fileOutput.close()
#    print("hash_value",hash_value)
#    print(len(hash_value))
#    print("hash_bin", hash_bin)

    return hash_value

def main():
    bin_file = "D:/workspace/test/pub_key.pem"
    hash_file = "D:/workspace/test/pubkey_hash.bin"
    genHashfile(bin_file, hash_file)

if __name__=='__main__':
    main()