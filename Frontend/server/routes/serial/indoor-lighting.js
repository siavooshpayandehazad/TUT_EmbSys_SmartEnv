'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.indoorLighting.statusFiles
});

function updateStatusFiles(leds, noConversion) {
    // argument must be an array of 6 numbers

    var stateMap = {};

    leds.forEach(function (value, index) {
        // Convert byte values of brightness to percentage
        stateMap['p15.led' + (index + 1)] = noConversion ? value : Math.round(value * 100 / 255);
    });

    statusFiles.updateMany(stateMap);
}

module.exports = function (parent) {
    if (parent === 'api') {
        return {
            statusFiles: statusFiles,
            updateStatusFiles: updateStatusFiles
        };
    }

    serial.onOpen(function () {

        var pollConf = config.modules.indoorLighting.poll;

        setTimeout(function () {

            // TODO: read actual values from files instead of zeros

            serial.write({
                to: config.modules.indoorLighting.address,
                data: ['L', 0, 0, 0, 0, 0]
            });

        }, pollConf.start);

    });

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('S', function (packet, next) {
        packet.log.info({route: 'S', data: packet.data}, 'Handling packet');

        updateStatusFiles(packet.data.slice(1));
    });

    return router;
};
