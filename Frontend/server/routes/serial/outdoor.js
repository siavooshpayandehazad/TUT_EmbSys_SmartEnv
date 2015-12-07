'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.outdoor.statusFiles
});


module.exports = function (parent) {

    serial.onOpen(function () {

        var pollConf = config.modules.outdoor.poll;

        setTimeout(function () {
            serial.write({
                to: config.modules.outdoor.address,
                data: ['D']
            });

            setInterval(function () {
                serial.write({
                    to: config.modules.outdoor.address,
                    data: ['D']
                });
            }, pollConf.interval);

        }, pollConf.start);

    });

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('S', function (packet, next) {
        packet.log.info({route: 'S', data: packet.data}, 'Handling packet');

        var temperature = packet.data.readInt8(1);
        var humidity = packet.data[2];
        var pressure = packet.data.readUInt16BE(3);
        var light = packet.data[5];
        var batteryLow = !!packet.data[6];

        packet.log.debug({
            temperature: temperature,
            humidity: humidity,
            pressure: pressure,
            light: light,
            batteryLow: batteryLow
        }, 'Setting module states');

        statusFiles.updateMany({
            'p3.temp': temperature,
            'p3.humid': humidity,
            'p3.pressure': pressure,
            'p3.light': light,
            'p3.battery': batteryLow ? 'low' : 'ok'
        });
    });

    return router;
};
