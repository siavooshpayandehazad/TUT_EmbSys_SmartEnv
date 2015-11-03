'use strict';

var Router = require('../lib/router');
var middleware = require('./middleware');

module.exports = function (opts) {
    var log = opts.log;

    var router = new Router(opts);

    router.middleware(middleware.extractCommandByte);

    router.route('S', function (packet, done) {
        log.info({route: 'S', data: packet.data}, 'Packet  routed');
        done();
    });

    return router;
};
