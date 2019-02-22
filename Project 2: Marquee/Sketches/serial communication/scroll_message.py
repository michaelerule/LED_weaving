#!/usr/bin/env python
# -*- coding: utf-8 -*-
'''
loop_to_serial.py: 

Repeatedly scrolls a message (hard-coded at the top of the file) to the
marquee over serial'
'''
from __future__ import unicode_literals
import serial,os,sys,time,tty,termios
from pylab import *
import codecs


baudrate = 9600#4800
delay = .15
message = "PaperMarquee 0.2       ♥♥♥        "


print('message is:')
print('  ',message)

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


'''
Send to serial
'''
ser=startSerial(baudrate)
print('supported characters:')
print('  ',''.join(mapping))
try:
    while(1):
        for ch in message:
            sys.stdout.write(ch)
            sys.stdout.flush()
            if ch in mapping:
                index = mapping.index(ch)
                if index >=0 and index < len(chars):
                    chardata = chars[index]+[0,];
                    for column in chardata:    
                        ser.write(chr(column))
                        time.sleep(delay)
except:
    pass
finally:   
    ser.close()
    print 'CLOSED'





