#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
pipe_to_serial.py: 

Gets characters typed at the command line and sends them to the marquee
over a serial communication. Uses `marquee_font.txt` for the marquee font.
'''
from __future__ import unicode_literals
import serial,os,sys,time,tty,termios
from pylab import *
import codecs

baudrate = 9600#4800
delay = .08
fontfile = 'marquee_font.txt'


'''
Load the font information
'''
mapping = ['A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i',
'j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','1','2',
'3','4','5','6','7','8','9','0','.',',',' ',';',':','-','+','=','?','!',
'\'','*','⌶','█','♥','&','$','%','^','#','[',']','(',')','@','"',
'/','\\','|','<','>','{','}','∘','÷','🙁','🙂']

chars = None
with open(fontfile,'r') as f:
    data = ''.join(f.readlines())
    chars = [map(eval,s.split()) for s in data.split('-')]
print chars



'''
For grabbing chars from the command line
'''
class _Getch:
    """Gets a single character from standard input.  Does not echo to the screen."""
    def __init__(self):
        try:
            self.impl = _GetchWindows()
        except ImportError:
            self.impl = _GetchUnix()
    def __call__(self): return self.impl()
    
class _GetchUnix:
    def __init__(self):
        import tty, sys
    def __call__(self):
        import sys, tty, termios
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        try:
            tty.setraw(sys.stdin.fileno())
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        return ch

class _GetchWindows:
    def __init__(self):
        import msvcrt
    def __call__(self):
        import msvcrt
        return msvcrt.getch()



'''        
Serial communication
'''        
devs = [s for s in os.listdir('/dev') if 'ttyUSB' in s or 'ttyACM' in s]
if len(devs)<1:
	print "no serial devices found!"
	sys.exit(0)
port = '/dev/'+devs[0]

def startSerial(baudrate = 9600):
    os.system('stty -F %s %d cs8 cread'%(port,baudrate))
    ser = serial.Serial(port,baudrate,timeout=0)
    time.sleep(.1)
    if not ser.isOpen():
        print 'CONNECTION FAILED'
        sys.exit(0)
    print 'SUCCESS, USING PORT %s AT %d'%(port,baudrate)
    return ser
    
def closeSerial(ser):
    ser.close()
    print 'CLOSED'



'''
Send to serial
'''
gg = _Getch()
ser=startSerial(baudrate)
print('supported characters:')
print('  ',''.join(mapping))
while(1):
    ch = gg()
    if ch in ['\x1Bc','\x03','\x04','\x0C','\x0A','\x33']:
        print('\n\n Exiting...')
        sys.exit(0)
    sys.stdout.write(ch)
    sys.stdout.flush()
    if ch in mapping:
        index = mapping.index(ch)
        if index >=0 and index < len(chars):
            chardata = chars[index]+[0,];
            for column in chardata:    
                ser.write(chr(column))
                time.sleep(delay)

closeSerial(ser)





