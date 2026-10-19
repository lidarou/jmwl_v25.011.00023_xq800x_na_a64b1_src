#!/usr/bin/env python

#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

import os
import sys
import struct
import argparse
import random
import shutil
import time
import click
import imgsign

def resign(magic, sw_verison, privkey, pubkey, infile, outfile, keysign, next_pubkey):
    fs = open(infile, 'rb')
    full_image = fs.read()
    magic_file, pubkey_size, image_offset= struct.unpack('3I', full_image[ : 12])
    fs.close()
    if magic_file == 0x55aa7e7e:
        mid_file = outfile + '.org'
        fd = open(mid_file, 'wb')
        fd.seek(0, 0)
        fd.write(full_image[image_offset : ])
        fd.close()
 
    if magic_file == 0x55aa7e7e:
        imgsign.sign_main(magic, sw_verison, privkey, pubkey, mid_file, outfile, keysign, next_pubkey)
        os.remove(mid_file)
    else :
        imgsign.sign_main(magic, sw_verison, privkey, pubkey, infile, outfile, keysign, next_pubkey)

@click.argument('outfile')
@click.argument('infile')
@click.option('-m', '--magic', metavar='magic_number')
@click.option('-s', '--sw_verison', metavar='version')
@click.option('-k', '--privkey', metavar='filename')
@click.option('-pub', '--pubkey', metavar='filename')
@click.option('--keysign', default=False, is_flag=True)
@click.option('-nk', '--next_pubkey', metavar='filename')
@click.command(help='''Create a signed or unsigned image\n''')        
def main(magic, sw_verison, privkey, pubkey, infile, outfile, keysign, next_pubkey):
    resign(magic, sw_verison, privkey, pubkey, infile, outfile, keysign, next_pubkey)
if __name__=='__main__':
    main()
