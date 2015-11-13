'use strict';

var Router = require('../../lib/router');
var middleware = require('./middleware');

var db = require('../../lib/db')();
var RfidCard = db.models.RfidCard;

module.exports = function (parent) {
    var log = parent._log;

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('C', function (packet, next) {
        log.info({route: 'C', data: packet.data}, 'Handling RFID card');

        // TODO: handling
    });

    router.route('S', function (packet, next) {
        log.info({route: 'S', data: packet.data}, 'Packet routed');

        // TODO: handling
    });

    return router;
};
