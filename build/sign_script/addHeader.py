#!/usr/bin/env python

#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import os
import sys
import click
import argparse
import numpy as np

img_header = {
    'magic_number': 0x55aa7e7e,
    'pubkey_size': 0,
    'image_offset': 0x30,
	'image_size':  0,
    'image_certificates_offset':0,
    'image_certificates_size':0,
	'image_sign_offset': 0,
    'image_sign_size': 0,
    'key_certificates_offset':0,
    'key_certificates_size':0,
	'key_sign_offset': 0,
    'key_sign_size': 0,
}

def header_create():
    header = []
    for key, i in zip(img_header, range(len(img_header))):
        header.append(img_header[key])
    return header

def gen_signed_file(header, payload, outfile):
    file = open(outfile, 'wb')
    file.seek(0, 0)
    np.array([header], dtype=np.int32).tofile(file)
    file.write(payload)
    file.close()

#sign tool的入口函数，参数私钥，密钥，要签名的bin，需要签名后bin的命名方式
@click.command(help='''Add image header\n''')
@click.option('-i', '--infile', metavar='filename')

def main(infile):
    #infile = "P:/00_development/lanqing.liu/work/1020/out/gx316l_h1/cp/bin/aicos_cp_flash.bin"
    #获取image
    file = open(infile, 'rb')
    payload = file.read()
    file.close()
    header = header_create()
    gen_signed_file(header, payload, infile)


if __name__=='__main__':
    main()