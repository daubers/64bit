#! /usr/bin/env python2

DEBUG = True

import serial
import time
import random

s = serial.Serial('/dev/ttyUSB0', baudrate=38400, timeout=.5, parity=serial.PARITY_EVEN)

boards = {
    42: [
        [ # current
            0, 0, 0, 0, 0, 0, 0, 0, #Outputs
            0, 0, 0, 0, 0, 0, 0, 0, #Inputs
        ],
        [ # next
            0, 0, 0, 0, 0, 0, 0, 0, #Outputs
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, #Inputs
        ],
        [ # status
            0, 0, 0, 0, 0, 0 ,0 ,0
        ]
    ]
}

def escape(data):
    s.write(chr(data))
    if data == 0xFF:
        s.write(chr(data))

def send(id, address, data):
    if DEBUG:
        print("[debug %d] %02X %02X" % (id, address, data))
    s.write(chr(0))
    s.write(chr(0xFF))
    s.write(chr(id))
    s.write(chr(address))
    escape(data)
    escape(id ^ address ^ data)
    try:
        rlen = ord(s.read()) - 0x82
    except:
        print("[ERROR %d] no reply" % id)
        time.sleep(1)
        return False

    if rlen < 0 or rlen > 128:
        print("[ERROR %d] invalid length" % id)
        return False
    checksum = ord(s.read())
    data = [ord(d) for d in s.read(rlen)]
    if DEBUG:
        print("[DEBUG %d] %d: %s" % (id, rlen, " ".join(['%02X' % d for d in data])))
    if len(data) != rlen:
        print("[ERROR %d] sort response %d" % (id, len(data)))
        return False
    mysum = 0
    for d in data:
        mysum ^= d
    if mysum != checksum or (mysum == 0xFF and checksum == 0):
        print("[ERROR %d] checksum %d != %d" % (id, checksum, mysum))
        return False
    else:
        for add, data in zip(data[::2], data[1::2]):
            if add == 0x7F:
                if chr(data) == 'C':
                    print("[error %d] checksum" % id)
                elif chr(data) == 'F':
                    print("[error %d] frame" % id)
                elif chr(data) == 'P':
                    print("[error %d] parity" % id)
                elif chr(data) == 'E':
                    print("[error %d] bad escape" % id)
                elif chr(data) == 'O':
                    print("[error %d] overflow" % id)
                else:
                    print("[error %d] %d" % (id, data))
                return False
            elif add == 0x7E:
                if data == id:
                    if DEBUG:
                        print("[DEBUG %d] Wakeup %d" % (data, id))
                    boards[id][0] = [0] * 16
                else:
                    print("[ERROR %d] Bad wakeup %d" % (data, id))
                    return False
            else:
                boards[id][2][add] = data
    time.sleep(.050)
    return True

while True:
    for board, data in boards.items():
        sent = False
        for i in range(16):
            if data[0][i] != data[1][i]:
                sent = True
                if send(board, i, data[1][i]):
                    data[0][i] = data[1][i]
        if not sent: #If the board is ideal resend a random item so it can reply
            i = random.randint(0, 15)
            send(board, i, data[0][i])
        print(data[2])
    time.sleep(.5)
