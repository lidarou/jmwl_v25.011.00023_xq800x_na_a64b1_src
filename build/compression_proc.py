#!/usr/bin/python
import os,sys,platform,subprocess

# param0: python script
# param1: binary file path
# param2: output pah

of_postfix = '_compressed.'
para_num = len(sys.argv)
build_path = os.path.dirname(sys.argv[0])
if_fullpath = sys.argv[1]
if_dir = os.path.dirname(if_fullpath)
if_fullname = os.path.basename(if_fullpath)
if_basename = if_fullname.split('.')[0]
if_postfix = if_fullname.split('.')[1]
if para_num < 3 :
    of_name = if_dir + '/' + if_basename + of_postfix + if_postfix
else :
    of_name = sys.argv[2]

os_name = platform.system()
if os_name.lower() == "windows":
    commd = build_path + '/tools/lzma/lzma.exe e ' + if_fullpath + ' ' + of_name + ' -d12 -lc0 -lp0 -pb0'
    r_v = os.system(commd)
else:
    commd = build_path + '/tools/lzma/lzma'
    r_v = subprocess.run([commd, 'e', if_fullpath, of_name, '-d12', '-lc0', '-lp0', '-pb0'], capture_output=True)

# Add "lenc" header to to compressed image

comp_file = open(of_name, 'rb')
comp_data = comp_file.read()
comp_file.close()
new_file = open(of_name, 'wb')
header_str = "lenc"
new_file.write(header_str.encode("ASCII"))
new_file.write(comp_data)
new_file.close()
