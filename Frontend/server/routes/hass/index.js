'use strict';

var config = require('easy-config');

var Router = require('../../lib/router.js');
var serial = require('../../lib/serial.js');

var indoorLighting = require('../serial/indoor-lighting')('api');

module.exports = function (opts) {
    var router = new Router({log: opts.log.child({sub: 'hass-router'})});
    var log = opts.log;

    var moduleMap = {};
    Object.keys(config.modules).forEach(function (key) {
        if (key[0] === '_') {
            return;
        }
        var moduleAddress = config.modules[key].address;
        moduleMap[moduleAddress] = key;
    });

    router.middleware(function (packet) {
        packet.parts = packet.raw.split(' ');
        packet.route = packet.parts[0];
        packet.data = packet.parts
            .slice(1)
            .map(function (part) {
                var number = parseInt(part, 10);
                return isNaN(number) ? part : number;
            });

        packet.log = log.child({module: moduleMap[packet.route] || packet.route});
        packet.log.debug({data: packet.data}, 'Outgoing packet from frontend to serial');
    });

    router.route('DEBUG', function (packet, next) {
        // Write raw data
        packet.debug({data: packet.data}, 'Passing DEBUG packet from frontend to serial');
        serial.write(packet.data);
    });

    router.route('15', function (packet, next) {
        var command = packet.data[0];

        if (command === 'L') {
            // Convert percentage values to bytes
            var sendData = ['L'];
            for (var i = 1; i <= 6; ++i) {
                sendData[i] = Math.round(packet.data[i] * 255 / 100);
            }

            // Send command
            serial.write({
                to: 15,
                data: sendData
            });

            // Update status files
            indoorLighting.updateStatusFiles(packet.data.slice(1), true);

            return;
        }

        packet.log.error({data: packet.data}, 'Handler not implemented for this frontend packet');
        next(new Error('Unhandled packet from hass'));
    });

    // Default router
    router.route(function (packet, next) {
        // Just pass command to serial
        serial.write({
            to: parseInt(packet.route, 10),
            data: packet.data
        });
    });

    return router;
};
