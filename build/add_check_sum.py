#!/usr/bin/env python
# -*- coding: utf-8 -*-

import os,sys,struct
import array

def get_file_checsum(path, offset):
    last = array.array('B')
    buffer = array.array('H')
    total = os.path.getsize(path)
    sum = 0
    index = offset // 2
    cksum = 0
    cnt = total // 2
    #print("index = 0x%d." % index)
    #print("cnt = 0x%d." % cnt)

    file= open(path, 'rb')
    buffer.fromfile(file, cnt)
    #print(buffer)

    cnt -= index
    total -= offset

    while (cnt > 0):
        data = buffer[index]
        cksum ^= data
        #print("buffer[%d] = 0x%x, cksum = 0x%x." % (index, cksum, data))
        cnt -= 1
        index += 1

    # if len is odd, the last byte left shift 8bits, and 8 LSB fill 0.
    if total % 2:
        last.fromfile(file, 1)
        data = last[0]
        #print("data = 0x%x, data<<8 = 0x%x." % (data, data << 8))
        cksum ^= (data << 8)

    return cksum

if __name__ == '__main__':
    input_path = sys.argv[1]
    input_offset = 0;

    #print("Add checksum for " + input_path + ".")
    if len(sys.argv) >= 3:
        input_offset = int(sys.argv[2])
        #print("input_offset = %d." % input_offset)

    if not os.path.exists(input_path):
        print(input_path + " file is not exit!")
        exit(1)

    size = os.path.getsize(input_path)
    #print("size = %d." % size)

    #if len is odd, must pad 1 byte.
    if size % 2:
        f1 = open(input_path, 'ab+')
        f1.write(struct.pack('B', 0))
        size += 1
        f1.close()

    ck_sum = get_file_checsum(input_path, input_offset)
    f = open(input_path, 'ab+')
    f.write(struct.pack('H', ck_sum))
    f.close()
    print("Add checksum 0x%x for " % ck_sum + input_path + ", size:%d offset:%d." % (size, input_offset))

    if len(sys.argv) < 4:
        exit(0)

    base_path = sys.argv[3]

    if not os.path.exists(base_path):
        print(base_path + " file is not exit!")
        exit(1)

    #size = os.path.getsize(base_path)
    #if len is odd, must pad 1 byte.
    #if size % 2:
        #f = open(base_path, 'ab+')
        #f.write(struct.pack('B', 0))
        #f.close()
        #size += 1

    # Add extend bin checksum to the base bin tail(save the ~ck_sum to the tail).
    ck_sum = (~ck_sum & 0xffff)
    f = open(base_path, 'ab+')
    f.write(struct.pack('H', ck_sum))
    f.close()
    print("Add extend ~checksum 0x%x to tail of " % ck_sum + base_path)

    #Just print the base path checksum for debug
    size = os.path.getsize(base_path)
    base_cksum = get_file_checsum(base_path, 0)
    print("Print the checksum 0x%x of " % base_cksum + base_path + ", size:%d offset:%d." % (size, 0))

