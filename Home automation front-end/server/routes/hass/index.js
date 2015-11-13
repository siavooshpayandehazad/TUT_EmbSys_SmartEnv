'use strict';

var Router = require('../../lib/router.js');
var serial = require('../../lib/serial.js');

module.exports = function (opts) {
    var router = new Router(opts);

    router.middleware(function (packet) {
        packet.parts = packet.raw.split(' ');
        packet.route = packet.parts[0];
        packet.data = packet.parts
            .slice(1)
            .map(function (part) {
                var number = parseInt(part, 10);
                return isNaN(number) ? part : number;
            });
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
