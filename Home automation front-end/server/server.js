'use strict';

var bunyan = require('bunyan');
var config = require('easy-config');
var SerialPort = require('serialport').SerialPort;

var FileBuffer = require('./lib/file-buffer');

var log = bunyan.createLogger(config.log);

var router = require('./routes')({
    id: 'main',
    log: log
});


var serial;
/**
 * Listen incoming packets from serial
 */
function listenSerial() {
    serial = new SerialPort(config.serial.device, config.serial.options);

    serial.on('open', function () {

        log.info({config: config.serial}, 'Serial port opened');

        // TODO: flush callback
        serial.flush();

        // Packet being received
        var receivePacket = [];

        serial.on('data', function(data) {
            log.info({data: data}, 'Incoming packet');

            router.request(data, function (err) {

            });
        });
    });

    serial.on('error', function (err) {
        log.error({error: err, config: config.serial}, 'Failed to open serial port');
    });
}

listenSerial();


/**
 * Listen commands from Home Assistant front-end
 */
function listenHass() {

    // Listen hass file for data
    var dataFromHass = FileBuffer({
        filename: config.hass.bufferFile
    });

    dataFromHass.on('data', function (data) {
        log.info({data: data}, 'Got command from hass');

        // TODO: use similar router as for serial packets

        var target = data.substr(0, 2);
        var command = data[2];
        var value = (data[3] === '1' ? 1 : 0);

        if (target !== '02' || command !== 'L') {
            return;
        }

        log.debug({value: value}, 'Setting lock');

        serial.write([2, 2, 4, 0, 0, 'L'.charCodeAt(0), value], function (err) {
            if (err) {
                log.error({error: err}, 'Failed to write to serial');
                return;
            }

            log.debug('Data successfully written to serial');
        })

    });
}

listenHass();
