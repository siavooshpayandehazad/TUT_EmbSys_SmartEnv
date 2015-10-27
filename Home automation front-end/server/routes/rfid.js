'use strict';

var Router = require('../lib/router');

module.exports = function (opts) {
    var log = opts.log;

    var router = new Router(opts);

    router.middleware(function (packet) {
        packet.route = String.fromCharCode(packet.data.shift());
    });

    router.route('S', function (packet, done) {
        log.info({route: 'S', data: packet.data}, 'Packet  routed');
        done();
    });

    return router;
};
