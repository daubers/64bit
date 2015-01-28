#!/usr/bin/env node
var ws = require('ws'),
    serialport = require('serialport')

var id=-1;
var add=-1;

var socket = null,
    ser = new serialport.SerialPort(
    '/dev/ttyUSB0',
    {
        baudrate: 38400,
    },
    function () {
        console.log('Serial open');
        ser.on('data', function (databuf) {
            for (var i = 0, len = databuf.length; i < len; i++) {
                var data = databuf[i];
                if(data & 0x80) {
                    id = data ^ 0x80;
                } else {
                    if(add == -1) {
                        add = data;
                    } else {
                        data = data | (add & 1) << 7;
                        add = (add & 0x70) >> 4;
                        console.log('<', id, add, data);

                        if(socket) {
                            socket.send(JSON.stringify({id: id, add: add, data: data}), function (error) {
                                if(error) {
                                    console.log(error);
                                    socket = null;
                                }
                            });
                        }
                        id = -1;
                        add = -1;
                    }
                }
            }
        });
    }
);

new ws.Server({port: 8080}).on(
    'connection',
    function connection(_socket) {
        console.log('Websocket open');
        socket = _socket;
        socket.on('message', function (message) {
            if(ser.isOpen()) {
                var msg = JSON.parse(message);
                ser.write(String.fromCharCode(0x80 | msg.id, msg.add<<4 | msg.type<<3 | (msg.data & 0x80)>>7, msg.data & ~0x80));
                console.log('>' + [0x80 | msg.id, msg.add<<4 | msg.type<<3 | (msg.data & 0x80)>>7, msg.data & ~0x80]);
            }
        });
    }
);
