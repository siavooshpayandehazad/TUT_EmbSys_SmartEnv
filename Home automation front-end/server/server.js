'use strict';

var bunyan = require('bunyan');
var config = require('easy-config');
var SerialPort = require('serialport').SerialPort;


var log = bunyan.createLogger(config.log);

var router = require('./routes')({
    id: 'main',
    log: log
});


var serial = new SerialPort(config.serial.device, config.serial);

serial.on('open', function (err) {
    if (err) {
        log.error({error: err, config: config.serial}, 'Failed to open serial port. Exiting');
        process.exit(1);
        return;
    }

    log.info({config: config.serial}, 'Serial port opened');

    // Packet being received
    var receivePacket = [];

    serial.on('data', function(data) {
        log.info({data: data}, 'Incoming packet');



/*
        router.request(packet, function (err) {

        });
*/
    });
});

