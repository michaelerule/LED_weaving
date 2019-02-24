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

encoded_text = """
Questions
-1-
WHY
The forlorn voice inside me:
"Why?"
And nights and the morning and the noon
"Why?"
The question that eats away the sun
The pain wrapping my skin
A silent exile to the void
"Why?"
A susurrus
From the funeral of answers
"Why? Why? Why?"

-2-
ANATOCISMUS
Why don't you have eyes
When looking at me
Why don't you have ears
When I'm talking
Such a big mouth you have
Is it to eat me?

-3-
ANSWER QUESTION
Not knowing what to ask
You ask
Of the pain whose answer is
Hidden in the question

-4-
QUESTION FREE
There were no questions
Void and pain
Question free darkness
Writhed in void

DENOUMENT

A rippling
Radiance
Knot within me
Come loose
Tears
Running

♥♥♥

"""

# PNG image file containing bitmap font pixel data
# fontfile = "./alphabet.png"
fontfile = "../../Fonts/Pixel Fonts/Height 5/alphabet_extended_turkish.png"
# Magenta color delimits character data
delimit  = (255,0,255)

# Unicode points corresponding to the glyps in the fontfile
mapping =\
    'ABCDEFGHİJKLMNOPQRSTUVWXYZ'+\
    'abcdefghijklmnopqrstuvwxyz'+\
    '1234567890., ;:-+=?!\'*'+\
    '⌶█♥&$%^#[]()@"/\\|<>{}∘÷🙁🙂'+\
    'ŞşÇçıIÖöÜüǦǧ'

# Standard ASCII glyphs
ascii_32_to_126 = r' !"'+r"#$%&'()*+,-./0123456789:;<=>"+\
    r'?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^'+\
    r'_`abcdefghijklmnopqrstuvwxyz{|}~'

# Code points we'll use to store additional glyphs
special_slots = [1,2,3,4,5,6,11,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31]


'''
Imports, with a little ImportError handling in case the user has not
installed the imageio library
'''
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

print('Reading font image file...')
im = imageio.imread(fontfile)
print(im.shape)
height,width,depth = im.shape
print('Font file defines a font for a %d-pixel high marquee.'%height)
if height>8:
    print("At the moment fonts taller than 8 pixels are unsupported")
    print("Exiting...")
    sys.exit(-1)

print('Reading in bitmap font data...')
print('(Final character should be followed by a delimiter pixel)')
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

print('Displaying imported character data:')
NCHARS        = len(characters)
MAX_NON_ASCII = len(special_slots)
ascii     = []
non_ascii = []
print('%d characters read'%NCHARS)
for c,ch in zip(mapping,characters):
    print('%s:\n'%c)
    is_ascii = c in ascii_32_to_126
    if is_ascii:
        print('Ascii code point',ascii_32_to_126.index(c)+32)
        ascii += [c]
    else:
        non_ascii += [c]
        print('Non-ascii, %d total so far out of %d max'%(len(non_ascii),MAX_NON_ASCII))
    for cd in ch:
        s = "{0:b}".format(cd).zfill(height)
        s = s.replace('1','█').replace('0',' ')
        print(s)
    print('\n')

print('Summary of font information:')
N_MISSING_ASCII = len(ascii_32_to_126)-len(ascii)
ascii_missing = sorted(list(set(ascii_32_to_126) - set(ascii)))
print('  %d out of %d ASCII glyphs defined:'%(len(ascii),len(ascii_32_to_126)))
print('    ',''.join(ascii))
print('  Missing %d ascii glyphs:'%N_MISSING_ASCII)
print('    ',''.join(ascii_missing))
print('  %d out of %d possible non-ascii glyph slots used:'%(len(non_ascii),MAX_NON_ASCII))
print('    ',''.join(non_ascii))
if (len(non_ascii)>MAX_NON_ASCII):
    print('Can only support %d non-ascii glyphs, but there are %d'%(MAX_NON_ASCII,len(non_ascii)))
    print("Exiting...")
    sys.exit(-1)
print('\n')

'''
We store the whole font as a bitmap. Character data then consists of 
  (a) an index into this bitmap
  (b) the character width in pixels
'''
widths = np.array(list(map(len,characters)))
print('\nCharacter widths in pixels:')
print(widths)
start_index = np.concatenate([[0],np.cumsum(widths)])
print('\nCharacter data starting indecies:')
print(start_index)
chardata = np.concatenate(characters)
print('\nConcatenated character pixel data:')
print(chardata)
if np.any(widths)<1:
    print('Need minimum width 1 pixel; check image data, 0-width chars found.')
    print("Exiting...")
    sys.exit(-1)
   
'''
To store offsets in 8-bit cars, subtract 1 from every index
Other ways to do this: store a signed offset relative to the average
character start position? 
'''
meanWidth = int(0.5+np.mean(widths))
print('\nTypical character width is %d'%meanWidth)
defaultOffsets = np.arange(len(widths)+1)*meanWidth
startIndex = start_index - defaultOffsets
print('\nRelative starting offsets:')
print(startIndex)
if np.any(startIndex>255):
    print('Adjusted start-indecies of character data > 255; Not supported presently')
    print("Exiting...")
    system.exit(-1)

'''
We need to bulid a mapping from ACII code points to our character data
We don't allow more than 127 characters, so 0xFF is "safe" to use as a 
"null" index (i.e. character slot not used)
'''
# Handle the ascii code points
NONE = 0xFF
location = np.zeros(128,dtype='i')+NONE
for i,ch in enumerate(ascii_32_to_126):
    if ch in mapping:
        location[i+32] = mapping.index(ch)
# Handle the non-ascii code points
# (store these in uncommonly used control characters in the 0-31 range)
for index,ch in zip(special_slots,non_ascii):
    location[index] = mapping.index(ch)
print('\nASCII point to character index (%d=None):'%NONE)
print(location)

print('Reformatting message for display...')
# Remove undefined characters
stripped = ''.join([c for c in encoded_text if c in mapping or c in '\n'])
NREMOVED = len(encoded_text)-len(stripped)
if NREMOVED>0:
    print('Removd %d undefined characters:'%(NREMOVED))
    print('  ',''.join([c for c in encoded_text if not (c in mapping or c in '\n')]))
NMESSAGE = len(stripped)
# Only supports escaped ascii characters and newline at the moment
# Other escape codes not allowed? not handled? How to handle? hmm...
recoded = stripped.replace('\\','\\\\').replace('"','\\"').replace("'","\\'").replace('\n','\\n')
for c in non_ascii:
    recoded = recoded.replace(c,'\\x%02x'%(special_slots[non_ascii.index(c)]))
message = recoded
message_stringform = '\n  '.join(['"%s\\n"'%s for s in message.split('\\n')])
print(message_stringform)


widths     = ','.join(map(str,widths))
startIndex = ','.join(map(str,startIndex))
chardata   = ','.join(map(str,chardata))
location   = ','.join(map(str,location))
message    = ','.join(map(str,message))

program_header = '''

// routines to store/access data from flash (program) memory directly
#include <avr/pgmspace.h>

// fixed numeric type definitions (uint8_t etc.)
#include <stdint.h>

// if the screen width has not otherwise been defined, default it to 5x22
// (the size of a charlieplexed display using 11 control lines)
#ifndef NCOLS
#define NCOLS (22)
#endif
#ifndef NROWS
#define NROWS (5)
#endif

// Relative character offsets (data starts at meanWidth*index+charOffset)
#define NCHARS (%(NCHARS)s)
#define MEANWIDTH (%(meanWidth)d)
const PROGMEM int8_t charOffset[NCHARS+1]={%(startIndex)s};

// Bit-packed character data (lowest order bits used first)
const PROGMEM uint8_t charData[]={%(chardata)s};

// Mapping from ascii character value to index into our character data
#define NONE (0x%(NONE)X)
const PROGMEM uint8_t ASCIIMap[128]={%(location)s};

// Message to be displayed (non-ascii as custom escape codes)
#define NMESSAGE (%(NMESSAGE)s)
const PROGMEM uint8_t message[%(NMESSAGE)s]=%(message_stringform)s;
'''%globals()

# Note: this routine is not finished, use scroll routine 2 instead.
scroll_routine_1 = '''
/** Routine to retrieve next column of pixel data in the scrolling text.
 *  Uses a state machine to decode the stored string data
 *  
 *  Possible better way to implement this? Rather than have a complex
 *  state machine that returns the columns of the scrolling marquee data,
 *  we could use more conventional looping & recursion to extract the 
 *  screen data, and have this routine call a "draw" or "update" function
 *  as it unpacks new data. This drawing function could have an internal
 *  state that causes it to wait a certain amount of time between
 */
volatile int message_index  = 0;
volatile int chardata_index = 0;
volatile int cols_left      = 0;
uint8_t get_next_column() {

    uint8_t pixels;

    if (cols_left>0)
        // next column of character data
        pixels = (uint8_t)pgm_read_byte(&charData[chardata_index]); 
    else
        // blank column between characters
        pixels = 0x00;
        
    cols_left--;
    while (cols_left<0) {
        // Wrap around the message
        message_index = (message_index+1)%%NMESSAGE;
        
        // Get next character from the message string
        uint8_t character = (uint8_t)pgm_read_byte(&message[message_index]); 
        
        // Special characters: these correspond to control actions and not
        // printed display characters. At present, only the newline action
        // is defined. We set it to scroll the text off screen.
        if (character=='\n') {
            // TODO: implement me!
            continue;
        }
        
        // Remap char to an index into the custom font data
        uint8_t chi = (uint8_t)pgm_read_byte(&ASCIIMap[character]);
        
        // It might be NONE? If so, we need to keep reading
        if (chi==NONE) {
            cols_left=-1;
            continue;
        }
        
        // Character is not NONE; pixel data exists, read it...
        
        // Determine the number of columns in character pixel data
        int offset  = (int8_t)pgm_read_byte(&charOffset[chi]);
        int offset2 = (int8_t)pgm_read_byte(&charOffset[chi+1]);
        int width   = offset2-offset+MEANWIDTH;
        
        // Set state machine to read columns from character data
        chardata_index = offset+MEANWIDTH*chi;
        cols_left      = width;
    }
    
    return pixels;
}
'''

# Use this one
scroll_routine_2 = '''
/** Routines to retrieve next column of pixel data in the scrolling text.
 *  
 *  Better implementation:
 * 
 *  Rather than have a complex state machine that returns the columns of 
 *  the scrolling marquee data, we could use more conventional looping & 
 *  recursion to extract the screen data, and have this routine call a 
 *  "draw" function as it unpacks new data. This drawing function could 
 *  have an internal state that causes it to wait a certain amount of time 
 *  between columns for smooth scrolling. 
 */

/** Drawing routine
 *  This function scrolls the display left one pixel, and adds new pixel
 *  data on the right. It waits at least `scan_interval_ms` between calls 
 *  to smooth the scrolling effect (this is a blocking wait).
 */
volatile unsigned long previousMillis   = 0;
const    unsigned long scan_interval_ms = 200;
void draw_column(uint8_t pixel_data) {

  // Wait until next scan interval 
  while (millis()-previousMillis <= scan_interval_ms);
  previousMillis = millis(); 
    
  // Draw pixel data to display
  shift_left();
  for (int col=0; col<NCOLS; col++)
    write_pixel(&display_data[0],0,col,(pixel_data>>col)&1);
}
 
/** Scrolling routine
 *  Unpacks message and font pixel data and sends it to screen
 */
#define GETCHAR(V,I) ((uint8_t)pgm_read_byte(&V[(I)]))
#define GETINT8(V,I) (( int8_t)pgm_read_byte(&V[(I)]))
void scroll_message() {

  for (int messageidx=0; messageidx<NMESSAGE; messageidx++) {
        
    // Get next character from the message string
    uint8_t character = GETCHAR(message,messageidx); 
        
    // Special characters: these correspond to control actions and not
    // printed display characters. 
    // Presently, only the newline action is defined. 
    // We set it to scroll the text off screen.
    if (character=='\\n') {
      for (int i=0; i<NCOLS; i++)
        draw_column(0x00);
      continue;
    }
        
    // Remap char to an index into the custom font data
    // The setup script shouldn't allow undefined characters, 
    // but skip them if we happen to encounter them anyway.
    uint8_t chi = GETCHAR(ASCIIMap,character);
    if (chi==NONE) continue;
        
    // Draw the pixel data
    int stop = GETINT8(charOffset,chi+1)+MEANWIDTH*(chi+1);
    for (int i=GETINT8(charOffset,chi  )+MEANWIDTH*chi; i<stop; i++)
      draw_column(GETCHAR(charData,i)); 
        
    // Draw a blank column between characters
    draw_column(0x00);
  }
}
'''

print(top_comment)
print(program_header)
print(scroll_routine_2)



