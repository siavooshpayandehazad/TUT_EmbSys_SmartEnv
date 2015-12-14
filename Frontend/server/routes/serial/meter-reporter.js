'use strict';

var config = require('easy-config');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


var statusFiles = ModuleStatus({
    dir: config.modules._statusDir,
    files: config.modules.meterReporter.statusFiles
});


module.exports = function (parent) {

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('D', function (packet, next) {
        packet.log.info({route: 'D', data: packet.data}, 'Handling packet');

        var waterAmount = packet.data.readInt16BE(1);// In millilitres
        var temperature = packet.data[3];

        statusFiles.read('p14.water', function (err, prevWaterAmount) {
            if (err) {
                packet.log.error({err: err}, 'Failed to read previous water amount');
                return;
            }

            packet.log.debug({
                temperature: temperature,
                waterAmount: prevWaterAmount + waterAmount
            }, 'Setting module states');

            statusFiles.updateMany({
                'p14.water': prevWaterAmount + waterAmount,
                'p14.temp': temperature
            });
        });
    });

    return router;
};
