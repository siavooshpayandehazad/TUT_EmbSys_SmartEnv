'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.fishFeeder.statusFiles
});


module.exports = function (parent) {
    var log = parent._log;

    serial.onOpen(function () {

        var pollConf = config.modules.fishFeeder.poll;

        setTimeout(function () {
            serial.write({
                to: config.modules.fishFeeder.address,
                data: ['D']
            });

            setInterval(function () {
                serial.write({
                    to: config.modules.fishFeeder.address,
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
        var waterLow = !!packet.data[2];
        var feedLow = !!packet.data[3];
        var temperature = packet.readInt8(4);
        var filterCleaning = !!packet.data[5];

        statusFiles.updateMany({
            'p5.light': lightOn ? 'on' : 'off',
            'p5.water': waterLow ? 'low' : 'ok',
            'p5.feed': feedLow ? 'low' : 'ok',
            'p5.temperature': temperature,
            'p5.filter': filterCleaning ? 'dirty' : 'ok'
        });
    });

    return router;
};
