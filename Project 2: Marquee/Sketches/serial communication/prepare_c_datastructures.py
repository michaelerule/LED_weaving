#!/usr/bin/python
# -*- coding: UTF-8 -*-
# BEGIN PYTHON 2/3 COMPATIBILITY BOILERPLATE
from __future__ import absolute_import
from __future__ import with_statement
from __future__ import division
from __future__ import nested_scopes
from __future__ import generators
from __future__ import unicode_literals
from __future__ import print_function

'''
prepare_c_datastructures.py

Python script to create C datastructures for embedding the font inside
Arduino sketches directly.

We store the whole font as a bitmap. Character data then consists of 
  (a) an index into this bitmap
  (b) the character width in pixels
'''

fontfile = "./alphabet.png"

# Magenta color delimits character data
delimit  = (255,0,255)

import sys,os
import numpy as np

def needs_imageio():
    print("This script depends on the `imageio` package")
    print('consider "pip install --user imageio"')
    sys.exit(-1)

try:
    import imageio
except ImportError:
    if sys.version_info < (3,0):
        # Workaround for bug on some systems:
        # Sometimes pip installs only for python 3?
        home = os.path.expanduser('~')
        path = home+'/.local/lib/python3.5/site-packages/'
        sys.path.append(path)
        try:
            import imageio
        except ImportError:
            needs_imageio()
    else:
        needs_imageio()

# Read font image file
im = imageio.imread(fontfile)
print(im.shape)
height,width,depth = im.shape
print('Font file defines a font for a %d-pixel high marquee.'%height)
if height>8:
    print("At the moment fonts taller than 8 pixels are unsupported")
    print("Exiting...")
    sys.exit(-1)

# Read in bitmap font data
chardata   = []
characters = []
powersof2  = 2**np.arange(height)
for column in range(width):
    marker = im[0,column,:]
    if np.all(marker==delimit):
        # Character delimiter detected
        # Save current char and prepare to read next one
        characters.append(chardata)
        chardata = []
        continue
    # Character data: get binary black/white values
    bits  = np.int32(im[:,column,0]<128)
    # Pack as integer
    asint = np.sum(bits*powersof2)
    chardata.append(asint)

# Display imported character data
NCHARS = len(characters)
print('%d characters read'%NCHARS)
for ch in characters:
    for cd in ch:
        s = "{0:b}".format(cd).zfill(height)
        s = s.replace('1','█').replace('0',' ')
        print(s)
    print('\n')




mapping = ['A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i',
'j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2',
'3','4','5','6','7','8','9','0','.',',',' ',';',':','-','+','=','?','!',
'\'','*','⌶','█','♥','&','$','%','^','#','[',']','(',')','@','"',
'/','\\','|','<','>','{','}','∘','÷','🙁','🙂']



'''
We'll store a 7-bit ASCII table (128) entries. Non-ascii characters will 
be... discarded? hmm. 
'''


