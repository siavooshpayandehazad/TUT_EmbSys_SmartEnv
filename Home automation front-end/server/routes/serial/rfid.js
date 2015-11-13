'use strict';

var Router = require('../../lib/router');
var middleware = require('./middleware');

var db = require('../../lib/db')();
var AccessCard = db.models.AccessCard;

module.exports = function (parent) {
    var log = parent._log;

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('C', function (packet, next) {
        log.info({route: 'C', data: packet.data}, 'Handling RFID card');

        // Take 10-byte code from buffer as hex.
        var code = packet.data
            .slice(1, 11)
            .toString('hex');

        AccessCard
            .find({code: code})
            .then(function (accessCard) {
                console.log(accessCard.toJSON());
                // TODO: unlock/lock
            });
    });

    router.route('S', function (packet, next) {
        log.info({route: 'S', data: packet.data}, 'Packet routed');

        // TODO: handling
    });

    return router;
};
