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
        packet.log.info({route: 'S', data: packet.data}, 'Handling packet');

        var temperatureStates = {
            '0': 'low',
            '1': 'ok',
            '2': 'high'
        };

        var lightOn = !!packet.data[1];
        var waterLevelOk = !!packet.data[2];
        var feedLevelOk = !!packet.data[3];
        var temperature = packet.data.readInt8(4);
        var temperatureState = temperatureStates[packet.data[5]];
        var filterClean = !!packet.data[6];

        packet.log.debug({
            lightOn: lightOn,
            waterLevelOk: waterLevelOk,
            feedLevelOk: feedLevelOk,
            temperature: temperature,
            temperatureState: temperatureState,
            filterClean: filterClean
        }, 'Setting module states');

        statusFiles.updateMany({
            'p5.light': lightOn ? 'on' : 'off',
            'p5.water': waterLevelOk ? 'ok' : 'low',
            'p5.feed': feedLevelOk ? 'ok' : 'low',
            'p5.temp': temperature,
            'p5.temp-state': temperatureState,
            'p5.filter': filterClean ? 'ok' : 'dirty'
        });
    });

    return router;
};
