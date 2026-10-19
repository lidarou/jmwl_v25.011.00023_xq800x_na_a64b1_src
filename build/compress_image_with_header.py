#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,struct
import array


if __name__ == '__main__':

    if_fullpath = sys.argv[1]
    if len(sys.argv) >= 3:
        header_size = int(sys.argv[2])
    else:
        header_size = 0

    print("Compress image: {0} and add {1} bytes header.".format(if_fullpath, header_size))

    if not os.path.exists(if_fullpath):
        print(if_fullpath + " file is not exit!")
        exit(1)

    of_postfix = '_compressed.'
    build_path = os.path.dirname(sys.argv[0])
    if_dir = os.path.dirname(if_fullpath)
    if_fullname = os.path.basename(if_fullpath)
    if_basename = if_fullname.split('.')[0]
    if_postfix = if_fullname.split('.')[1]
    if len(sys.argv) < 4 :
        of_name = if_dir + '/' + if_basename + of_postfix + if_postfix
    else :
        of_name = sys.argv[3]

    commd = build_path + '/tools/lzma/lzma.exe e ' + if_fullpath + ' ' + of_name + ' -d12 -lc0 -lp0 -pb0'
    r_v = os.system(commd)
    # Add "lenc" header to to compressed image

    comp_file = open(of_name, 'rb')
    comp_data = comp_file.read()
    comp_file.close()
    ori_header = bytes()
    if header_size > 0:
        ori_file = open(if_fullpath, 'rb')
        ori_header = ori_file.read(header_size)
        ori_file.close()

    new_file = open(of_name, 'wb')
    header_str = "lenc"
    new_file.write(ori_header)
    new_file.write(header_str.encode("ASCII"))
    new_file.write(comp_data)
    new_file.close()

