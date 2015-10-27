'use strict';

var bunyan = require('bunyan');
var config = require('easy-config');
var RSMQWorker = require('rsmq-worker');


var log = bunyan.createLogger({
    name: 'hafe-server',
    level: config.log.level || 'trace'
});

var router = require('./routes')({
    id: 'main',
    log: log
});

var packetProcessor = new RSMQWorker('packets', {
    redisPrefix: 'rsmq'
});

packetProcessor.on('message', function (packet, done) {
    log.info({packet: packet}, 'Incoming packet');

    try {
        packet = JSON.parse(packet);
    } catch (err) {
        log.error({error: err}, 'Failed to parse packet');
        done(err);
        return;
    }

    router.request(packet, done);
});

// TODO: Error listeners
packetProcessor.on('error', function( err, msg ){
    console.log( "ERROR", err, msg.id );
});
packetProcessor.on('exceeded', function( msg ){
    console.log( "EXCEEDED", msg.id );
});
packetProcessor.on('timeout', function( msg ){
    console.log( "TIMEOUT", msg.id, msg.rc );
});

packetProcessor.start();
log.info('Listening for packets');
