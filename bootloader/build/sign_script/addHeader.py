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
fota_vaild_flag = 0xaae7557e
img_header_size = 0x30
img_header = {
    'magic_number': 0x55aa7e7e,
    'pubkey_size': 0,
    'image_offset': img_header_size,
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

def update_header(infile):
    img_header['pubkey_size'] = 0
    #获取image
    file = open(infile, 'rb')
    payload = file.read()
    file.close()

    img_header['image_size'] = len(payload)
    img_header['image_certificates_offset'] = img_header['image_size']  + img_header_size
    img_header['image_certificates_size'] = 0
    img_header['image_sign_offset'] = img_header['image_size']  + img_header_size
    img_header['image_sign_size'] = 0
    img_header['key_certificates_offset'] = img_header['image_size']  + img_header_size
    img_header['key_certificates_size'] = 0
    img_header['key_sign_offset'] = img_header['image_size']  + img_header_size
    img_header['key_sign_size'] = 0

def header_create():
    header = []
    for key, i in zip(img_header, range(len(img_header))):
        header.append(img_header[key])
    return header

def gen_signed_file(header, payload, infile, outfile):
    file = open(outfile, 'wb')
    file.seek(0, 0)
    np.array([header], dtype=np.int32).tofile(file)
    file.write(payload)
    file.close()

#sign tool的入口函数，参数私钥，密钥，要签名的bin，需要签名后bin的命名方式
@click.command(help='''Add image header\n''')
@click.option('-i', '--infile', metavar='filename')
@click.option('-o', '--outfile', metavar='filename')
@click.option('--fota', default=False, is_flag=True)

def main(infile, outfile, fota):
    #update header, especially img size and other offset, which will be used in mbl
    update_header(infile)

    #获取image
    file = open(infile, 'rb')
    payload = file.read()
    if fota:
        payload = payload + np.uint32(fota_vaild_flag)
    file.close()
    #only add header if secure_boot not enabled
    header = header_create()
    gen_signed_file(header, payload, infile, outfile)

if __name__=='__main__':
    main()