'use strict';

var bunyan = require('bunyan');
var config = require('easy-config');
var express = require('express');

var FileBuffer = require('./lib/file-buffer');
var logUtils = require('./lib/log');
var serial = require('./lib/serial');

var log = bunyan.createLogger(config.extend(config.log.bunyan, {
    serializers: config.extend(bunyan.stdSerializers, logUtils.serializers)
}));


// Router for incoming packets from serial
var serialRouter = require('./routes/serial')({
    log: log
});

serial.init({
    config: config,
    log: log.child({sub: 'serial'}),
    listener: function (data) {
        log.info({data: data}, 'Incoming packet');

        serialRouter.request(data, function (err) {
            if (err) {
                log.error({err: err, data: data}, 'Failed to process packet from serial');
            }
        });
    }
});


// Router for incoming data from Home Assistant frontend
var hassRouter = require('./routes/hass')({
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

    app.use(function (req, res, next) {
        req.log = log.child({sub: 'debug-interface'});
        next();
    });

    app.use('/api', require('./routes/web')());

    app.use(function (err, req, res, next) {
        log.error({err: err}, 'Error on debug interface');

        res.status(500).send('<div>Unexpected error occured</div>');
    });

    app.listen(config.webInterface.port, function () {
        log.info('Web interface running on port ' + config.webInterface.port);
    });
}
