'use strict';

var Router = require('../../lib/router.js');

module.exports = function (opts) {
    var router = new Router(opts);

    router.middleware(function (packet) {
        packet.dataLength = packet.raw[0];
        packet.to = packet.raw[1];
        packet.from = packet.raw[2];
        packet.route = packet.from;
        packet.data = packet.raw.slice(5);
    });

    router.route(2, require('./rfid')(router));
    router.route(3, require('./outdoor')(router));
    router.route(5, require('./fish-feeder')(router));

    return router;
};
