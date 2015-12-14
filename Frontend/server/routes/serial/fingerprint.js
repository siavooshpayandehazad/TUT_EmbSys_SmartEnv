'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');

var door = require('./rfid')('api').door;

var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.fingerprint.statusFiles
});


module.exports = function (parent) {

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('S', function (packet, next) {
        packet.log.info({route: 'S', data: packet.data}, 'Handling packet');

        var fingerId = packet.data[1];

        packet.log.debug({fingerId: fingerId}, 'Toggle door lock');
        door.lockRequest(!door.isLocked(), door.isMasterMode(), false);

        packet.log.debug({fingerId: fingerId}, 'Saving id in state file');
        statusFiles.update('p1.id', fingerId, function (err, cb) {
            if (err) {
                packet.log.error({err: err}, 'Failed to write fingerprint id');
            }
        });
    });

    return router;
};
