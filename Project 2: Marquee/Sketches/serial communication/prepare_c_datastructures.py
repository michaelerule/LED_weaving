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
We'll store a 7-bit ASCII table (128) entries.

Dec  Char                           Dec  Char     Dec  Char     Dec  Char
---------                           ---------     ---------     ----------
  0  NUL (null)                      32  SPACE     64  @         96  `
  1  SOH (start of heading)          33  !         65  A         97  a
  2  STX (start of text)             34  "         66  B         98  b
  3  ETX (end of text)               35  #         67  C         99  c
  4  EOT (end of transmission)       36  $         68  D        100  d
  5  ENQ (enquiry)                   37  %         69  E        101  e
  6  ACK (acknowledge)               38  &         70  F        102  f
  7  BEL (bell)                      39  '         71  G        103  g
  8  BS  (backspace)                 40  (         72  H        104  h
  9  TAB (horizontal tab)            41  )         73  I        105  i
 10  LF  (NL line feed, new line)    42  *         74  J        106  j
 11  VT  (vertical tab)              43  +         75  K        107  k
 12  FF  (NP form feed, new page)    44  ,         76  L        108  l
 13  CR  (carriage return)           45  -         77  M        109  m
 14  SO  (shift out)                 46  .         78  N        110  n
 15  SI  (shift in)                  47  /         79  O        111  o
 16  DLE (data link escape)          48  0         80  P        112  p
 17  DC1 (device control 1)          49  1         81  Q        113  q
 18  DC2 (device control 2)          50  2         82  R        114  r
 19  DC3 (device control 3)          51  3         83  S        115  s
 20  DC4 (device control 4)          52  4         84  T        116  t
 21  NAK (negative acknowledge)      53  5         85  U        117  u
 22  SYN (synchronous idle)          54  6         86  V        118  v
 23  ETB (end of trans. block)       55  7         87  W        119  w
 24  CAN (cancel)                    56  8         88  X        120  x
 25  EM  (end of medium)             57  9         89  Y        121  y
 26  SUB (substitute)                58  :         90  Z        122  z
 27  ESC (escape)                    59  ;         91  [        123  {
 28  FS  (file separator)            60  <         92  \        124  |
 29  GS  (group separator)           61  =         93  ]        125  }
 30  RS  (record separator)          62  >         94  ^        126  ~
 31  US  (unit separator)            63  ?         95  _        127  DEL
 
To handle a limited set of non-ascii characters, we remap these characters 
to escape squencies that are not commonly used in the 0-31 range.

We might want to support sending text to the display like it is a terminal,
in which case the newline, backspace, null, LF and CR characters should be
respected, as well as perhaps TAB, since this may occur in normal text
documents. BEL might be fun to implement as well as flashing or some such.
One character perhaps we will use for a terminal "reset"? Perhaps FF (12)

This leaves the following 25 characters free:

01 02 03 04 05 
06 11 14 15 16 
17 18 19 20 21 
22 23 24 25 26 
27 28 29 30 31

This might be enough to support a few Unicode characters, in a non-standard
way. For undefined and reserved characters, we should remember to set their
character data to "null".
'''





