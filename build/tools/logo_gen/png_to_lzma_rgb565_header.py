import os
import sys
import struct
import subprocess
from PIL import Image

def png2rgb565bin(infile, outfile):
	im=Image.open(infile)
#	im.show()
	print("read %s\nImage Width:%d Height:%d" % (infile, im.size[0], im.size[1]))

	f = open(outfile, "wb")
	pix = im.load()  #load pixel array
	for h in range(im.size[1]):
		for w in range(im.size[0]):
			R = pix[w, h][0] >> 3
			G = pix[w, h][1] >> 2
			B = pix[w, h][2] >> 3
			rgb = (R << 11) | (G << 5) | B
			f.write(struct.pack('B', rgb & 255))
			f.write(struct.pack('B', (rgb >> 8) & 255))

	f.close()
#	print("write to %s" % outfile)

def save_pure_data(header_file):
    temp_file = "temp.txt"
    flag1 = 'unsigned'
    flag2 = '};'
    with open(header_file, 'r') as file:
        with open(temp_file, "w") as temp:
            for line in file:
                if (flag1 in line) or (flag2 in line):
                    continue
                else:
                    temp.write(line)
    os.replace(temp_file, header_file)

def convert_all_png2rgb565_h(file_path):
    for root,dirname,filenames in os.walk(file_path):
        for filename in filenames:
            if os.path.splitext(filename)[1] == '.png':
                header_file_name = os.path.splitext(filename)[0]+".bin"
                with open(header_file_name, 'w') as header_file:
                    png2rgb565bin(filename, header_file_name)
                    lzma_shell_cmd = './../lzma/lzma.exe e -d12 -lc0 -lp0 -pb0' + " "+header_file_name+" "+os.path.splitext(filename)[0] + '_lzma.bin'
                subprocess.run(lzma_shell_cmd)
                os.remove(header_file_name)
                lzma_head = "xxd.exe -i " + os.path.splitext(filename)[0] + '_lzma.bin' + ' ' + os.path.splitext(filename)[0] + '.h'
                subprocess.run(lzma_head)
                os.remove(os.path.splitext(filename)[0] + '_lzma.bin')
                target = os.path.splitext(filename)[0] + '.h'
                save_pure_data(target)

if __name__ == '__main__':
    path = os.getcwd()
    convert_all_png2rgb565_h(path)