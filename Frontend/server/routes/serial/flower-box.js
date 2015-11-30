'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.flowerBox.statusFiles
});


module.exports = function (parent) {
    var log = parent._log;

    serial.onOpen(function () {

        var pollConf = config.modules.flowerBox.poll;

        setTimeout(function () {
            serial.write({
                to: config.modules.flowerBox.address,
                data: ['D']
            });

            setInterval(function () {
                serial.write({
                    to: config.modules.flowerBox.address,
                    data: ['D']
                });
            }, pollConf.interval);

        }, pollConf.start);

    });

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('S', function (packet, next) {
        log.info({route: 'S', data: packet.data}, 'Packet routed');

        var lightOn = !!packet.data[1];
        var temperature = packet.readInt8(2);
        var moisture = packet.data[3];
        var humidity = packet.data[4];

        statusFiles.updateMany({
            'p6.light': lightOn ? 'on' : 'off',
            'p6.temperature': temperature,
            'p6.moisture': moisture,
            'p6.humidity': humidity
        });
    });

    return router;
};