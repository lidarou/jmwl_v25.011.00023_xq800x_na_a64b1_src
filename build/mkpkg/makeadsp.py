#!/usr/bin/python
import os
import sys

ptcm_bin = sys.argv[1]
dtcm_bin = sys.argv[2]
single_bin = sys.argv[3]

#Contact dcom and pcom to a single bin
f_dtcm = open(dtcm_bin, "rb")
b_dtcm = f_dtcm.read()
f_dtcm.close()
f_ptcm = open(ptcm_bin, "rb")
b_ptcm = f_ptcm.read()
f_ptcm.close()
f_single = open(single_bin, "wb")
offset = len(b_dtcm) + 4
b_off = offset.to_bytes(4, byteorder = 'little')
f_single.write(b_off)
f_single.write(b_dtcm)
f_single.write(b_ptcm)
f_single.close()
