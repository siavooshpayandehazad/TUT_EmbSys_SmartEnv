#!/bin/python3

import serial
import time

def init():
    return serial.Serial(
        port = '/dev/ttyACM0',
        baudrate = 9600,
        bytesize = serial.EIGHTBITS,
        parity = serial.PARITY_NONE,
        stopbits = serial.STOPBITS_ONE
    )

def listen(com):
    char = com.read().decode('utf-8')
    print('received ' + char)
    return char

def sendByte(com, byte):
    print('sending ' + byte.decode('utf-8'))
    com.write(byte)

sendChar = b'A';
bytesRead = 0

com = init()

while 1:
    listen(com);
    bytesRead += 1;

    if bytesRead % 5 == 0:
        sendByte(com, sendChar)
        sendChar = bytes([sendChar[0] + 1])

    time.sleep(0.05)
