#!/usr/bin/env python

'''

quick script to convert a bmp image to the format required by the teensyview memory.

try and crop and resize your image to closely match 128px x 32px.

@author: Guil-T

'''

from scipy import misc
import os

path = './'
#image = misc.imread(os.path.join(path, 'tesla_logo_PNG21_lcd.bmp'), flatten=1)
image = misc.imread(os.path.join(path, 'esidewinder.bmp'), flatten=1)

print image

oledmem = bytearray([0 for i in range(128*4)])

for i in xrange(len(image)):
    for y in xrange(len(image[i])):
        if image[i][y] < 255:
			if (y < 128) and (i < 32):
				oledmem[((i/8)*128+y)] |= 1 << (i%8)
			
print "paste this array..."
	

for i in xrange(len(oledmem)):
	if (i % 128) == 0:
		print ""
	print "0x%02x, " % oledmem[i],

	



