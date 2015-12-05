'use strict';

var bunyan = require('bunyan');
var config = require('easy-config');
var express = require('express');

var FileBuffer = require('./lib/file-buffer');
var serial = require('./lib/serial');

var log = bunyan.createLogger(config.extend(config.log, {
    serializers: bunyan.stdSerializers
}));


// Router for incoming packets from serial
var serialRouter = require('./routes/serial')({
    id: 'serial',
    log: log
});

serial.init({
    config: config,
    log: log,
    listener: function (data) {
        log.info({data: data}, 'Incoming packet');

        serialRouter.request(data, function (err) {
            if (err) {
                log.error({err: err, packetData: data}, 'Failed to process packet from serial');
            }
        });
    }
});


// Router for incoming data from Home Assistant frontend
var hassRouter = require('./routes/hass')({
    id: 'hass',
    log: log
});

/**
 * Listen commands from Home Assistant front-end
 */
function listenHass() {

    // Listen hass file for data
    var dataFromHass = FileBuffer({
        filename: config.hass.bufferFile
    });

    dataFromHass.on('data', function (data) {
        log.info({data: data}, 'Incoming command from hass');

        hassRouter.request(data, function (err) {
            if (err) {
                log.error({err: err, packetData: data}, 'Failed to process packet from serial');
            }
        });

    });
}

listenHass();

// Web interface for debugging

if (config.isEnvironment('dev')) {
    var app = express();

    app.use('/', express.static('./public'));
    app.use('/bower_components', express.static('./bower_components'));

    app.use('/api', require('./routes/web')());

    app.use(function (err, req, res, next) {
        log.error({err: err}, 'Error on debug interface');

        res.status(500).send('<div>Unexpected error occured</div>');
    });

    app.listen(config.webInterface.port, function () {
        log.info('Web interface running on port ' + config.webInterface.port);
    });
}
