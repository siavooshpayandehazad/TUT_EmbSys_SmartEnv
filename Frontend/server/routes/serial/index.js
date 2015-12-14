'use strict';

var config = require('easy-config');

var Router = require('../../lib/router.js');

module.exports = function (opts) {
    var router = new Router({log: opts.log.child({sub: 'serial-router'})});

    var moduleMap = {};
    Object.keys(config.modules).forEach(function (key) {
        if (key[0] === '_') {
            return;
        }
        var moduleAddress = config.modules[key].address;
        moduleMap[moduleAddress] = key;
    });

    router.middleware(function (packet) {
        packet.dataLength = packet.raw[0];
        packet.to = packet.raw[1];
        packet.from = packet.raw[2];
        packet.route = packet.from;
        packet.data = packet.raw.slice(5);

        packet.log = opts.log.child({module: moduleMap[packet.from]}, true);
        packet.log.info('Incoming packet');
    });

    router.route(2, require('./rfid')(router));
    router.route(3, require('./outdoor')(router));
    router.route(5, require('./fish-feeder')(router));
    router.route(6, require('./flower-box')(router));
    router.route(14, require('./meter-reporter')(router));

    return router;
};
