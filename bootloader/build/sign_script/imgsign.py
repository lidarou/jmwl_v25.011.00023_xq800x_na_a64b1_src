#!/usr/bin/env python

#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AICXTEK TECHNOLOGIES CO.,LTD. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import os
import sys
import click
import argparse
import hashlib
import numpy as np
from signtool import sign

fota_vaild_flag = 0xaae7557e
#image 头的描述
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

#image的安全证书
img_certificate = {
    'version': 0,
    'type': 1,
    'hash': 0,
    'pubkey': 0,
}

#key的安全证书
key_certificate = {
    'type': 0,
    'hash' : 0,
}

#签名sign固件
sign_context_img = [0]
sign_context_key = [0]
sign_context = {
    'img': sign_context_img,
    'key': sign_context_key
}

def get_file_hash(bin_file):
    f = open(bin_file,"rb")
    thehash = hashlib.sha256()
    theline = f.readline()

    while(theline):
        thehash.update(theline)
        theline = f.readline()

    return thehash.hexdigest()

def key_sign(pubkey, privkey):
    file = open(pubkey, 'rb')
    pubkey_value = file.read()
    sign_context['key'] = sign(pubkey_value, privkey)
    hash_value = get_file_hash(pubkey)
    key_certificate['hash'] = bytes([int(hash_value[x:x+2],16)for x in range(0,len(hash_value),2)])

def image_sign(privkey, pubkey, infile):
    # 初始化证书pubkey值
    file = open(pubkey, 'rb')
    img_certificate['pubkey'] = file.read()
    img_header['pubkey_size'] = len(img_certificate['pubkey'])
    file.close()
 
    #获取image
    file = open(infile, 'rb')
    payload = file.read()
    #获取image的大小
    align4B_len = (len(payload) + 3) & ~0x3
    for i in range(align4B_len - len(payload)) :
        payload = payload + np.uint8(0)
    img_header['image_size'] = align4B_len
    img_header['image_certificates_offset'] = img_header['image_size']  + img_header_size
    file.close()

    payload = payload + np.uint32(img_certificate['version'])
    # 计算img hash值
    h = hashlib.sha256()
    h.update(payload)
    hash_value = h.hexdigest()
    img_certificate['hash'] = bytes([int(hash_value[x:x+2],16)for x in range(0,len(hash_value),2)])

    #签名img
    sign_context['img'] = sign(payload, privkey)
    payload = payload + np.uint32(img_certificate['type'])
    payload = payload + img_certificate['hash']
    payload = payload + img_certificate['pubkey']

    sum_len = len(bytes(img_certificate['type'])) + len(img_certificate['hash']) + len(bytes(img_certificate['version'])) + len(img_certificate['pubkey'])
    img_header['image_sign_offset'] = len(payload) + img_header_size
    img_header['image_sign_size'] = len(sign_context['img'])
    img_header['image_certificates_size'] = img_header['image_sign_offset'] - img_header['image_certificates_offset']

    payload = payload + sign_context['img']
    img_header['key_certificates_offset'] = len(payload) + img_header_size
    return payload

def header_create():
    header = []
    for key, i in zip(img_header, range(len(img_header))):
        header.append(img_header[key])
    return header

def gen_signed_file(header, payload, outfile):
    file = open(outfile, 'wb')
    file.seek(0, 0)
    np.array([header]).tofile(file)
    file.write(payload)
    file.close()

#sign tool的入口函数，参数私钥，密钥，要签名的bin，需要签名后bin的命名方式
@click.argument('outfile')
@click.argument('infile')
@click.option('-m', '--magic', metavar='magic_number')
@click.option('-s', '--sw_verison', metavar='version')
@click.option('-k', '--privkey', metavar='filename')
@click.option('-pub', '--pubkey', metavar='filename')
@click.option('--keysign', default=False, is_flag=True)
@click.option('-nk', '--next_pubkey', metavar='filename')
@click.option('--fota', default=False, is_flag=True)
@click.command(help='''Create a signed or unsigned image\n''')

def main(magic, sw_verison, privkey, pubkey, infile, outfile, keysign, next_pubkey, fota):
    print('magic number: ', magic)
    print('version: ', sw_verison)
    img_header['magic_number'] = int(magic, 16)
    img_certificate['version'] = int(sw_verison, 16)

    payload = image_sign(privkey, pubkey, infile)

    if keysign:
        key_sign(next_pubkey, privkey)
        payload = payload + np.uint32(key_certificate['type'])
        payload = payload + key_certificate['hash']
        img_header['key_certificates_size'] = len(key_certificate['hash']) + 4
        img_header['key_sign_offset'] = len(payload) + img_header_size
        payload = payload + sign_context['key']
        img_header['key_sign_size'] = len(sign_context['key'])

    if fota:
        payload = payload + np.uint32(fota_vaild_flag)

    header = header_create()
    gen_signed_file(header, payload, outfile)

    print('pubkey_size','0x%x'%img_header['pubkey_size'])
    print('image_offset','0x%x'%img_header['image_offset'])
    print('image_size','0x%x'%img_header['image_size'])
    print('image_certificates_offset', '0x%x'%img_header['image_certificates_offset'])
    print('image_certificates_size','0x%x'%img_header['image_certificates_size'])
    print('image_sign_offset', '0x%x'%img_header['image_sign_offset'])
    print('image_sign_size', '0x%x'%img_header['image_sign_size'])
    print('key_certificates_offset', '0x%x'%img_header['key_certificates_offset'])
    print('key_certificates_size','0x%x'%img_header['key_certificates_size'])
    print('key_sign_offset', '0x%x'%img_header['key_sign_offset'])
    print('key_sign_size', '0x%x'%img_header['key_sign_size'])

if __name__=='__main__':
    main()