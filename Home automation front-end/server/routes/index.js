'use strict';

var Router = require('../lib/router.js');

module.exports = function (opts) {
    var router = new Router(opts);

    router.middleware(function (packet) {
        packet.length = packet.data[0];
        packet.from = packet.data[1];
        packet.to = packet.data[2];
        packet.route = packet.from;
        packet.data = packet.data.slice(3);
    });

    router.route(2, require('./rfid')({log: opts.log}));

    return router;
};
