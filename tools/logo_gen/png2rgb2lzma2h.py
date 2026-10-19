# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.
import os
import sys
import struct
from PIL import Image

def main():
	infile = "./image.png" # png of logo which you want to use
	outfile = "logo.h" # output file name
	im=Image.open(infile)
	im.show()
	print("read %s\nImage Width:%d Height:%d" % (infile, im.size[0], im.size[1]))

	f = open("temp.rgb", "wb")
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
	print("write to %s" % outfile)

	os.system('lzma.exe e temp.rgb temp.lzma -d12 -lc0 -lp0 -pb0')
	os.system('xxd -i temp.lzma ' + outfile)

if __name__ == "__main__":
	sys.exit(main())