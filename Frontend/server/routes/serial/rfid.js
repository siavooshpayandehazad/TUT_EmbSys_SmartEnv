'use strict';

var config = require('easy-config');
var path = require('path');

var Router = require('../../lib/router');
var middleware = require('./middleware');

var db = require('../../lib/db')();
var AccessCard = db.models.AccessCard;

var serial = require('../../lib/serial');
var ModuleStatus = require('../../lib/module-status');


function Door(opts) {
    if (!(this instanceof Door)) {
        return new Door();
    }

    opts = opts || {};

    this._logger = opts.log;

    // Default values should be updated as soon as 'S' packet is received
    this._locked = true;
    this._masterMode = false;
    this._error = false;

    this._statusFiles = ModuleStatus({
        dir: config.modules._statusDir,
        files: config.modules.rfid.statusFiles
    });
}

Door.prototype._log = function _log(level) {
    if (!this._logger) {
        return;
    }

    return this._logger[level].apply(this._logger, arguments.slice(1));
};

Door.prototype.isLocked = function isLocked() {
    return this._locked;
};

Door.prototype.setLocked = function setLocked(locked) {
    var _this = this;
    _this._locked = !!locked;

    _this._statusFiles.update('p2.lock', _this._locked, function (err) {
        if (err) {
            _this._log('error', {error: err, statusFile: 'p2.lock'}, 'Failed to update');
        }
    });
};

Door.prototype.hasError = function hasError() {
    return this._error;
};

Door.prototype.setError = function setError(error) {
    var _this = this;
    _this._error = !!error;

    _this._statusFiles.update('p2.status', _this._error ? 'fault': 'ok', function (err) {
        if (err) {
            _this._log('error', {error: err, statusFile: 'p2.status'}, 'Failed to update');
        }
    });
};

Door.prototype.isMasterMode = function isMasterMode() {
    return this._masterMode;
};

Door.prototype.setMasterMode = function masterMode(masterMode) {
    this._masterMode = !!masterMode;
};

Door.prototype.lockRequest = function lockRequest(lock, master, wrong) {
    // Command to component
    // TODO: timeouts for locking and master mode
    serial.write({
        to: config.modules.rfid.address,
        data: [
            'L',
            lock ? 1 : 0,
            master ? 1 : 0,
            wrong ? 1 : 0
        ]
    });

    // Update local statuses
    this.setLocked(lock);
    this.setMasterMode(master);
};

Door.prototype.getStatus = function getStatus() {
    return [this.isLocked(), this.isMasterMode(), this.hasError()];
};


module.exports = function (parent) {
    var log = parent._log;

    var door = Door({log: log});

    serial.onOpen(function () {

        serial.write({
            to: config.modules.rfid.address,
            data: ['R']
        }, function (err) {
            if (err) {
                log.error({error: err}, 'Failed to send status request to rfid component');
                // TODO: retry
                // TODO: should be possible to check if 'S' packet actually arrived
                return;
            }
        });

    });

    var router = new Router(parent);

    router.middleware(middleware.extractCommandByte);

    router.route('C', function (packet, next) {
        log.info({route: 'C'}, 'Handling RFID card');
        log.debug({route: 'C', data: packet.data}, 'Handling RFID card');

        // Take 10-byte code from buffer as hex.
        var code = packet.data
            .slice(1, 11)
            .toString('hex');

        AccessCard
            .find({code: code})
            .then(function (accessCard) {
                if (!accessCard) {
                    if (door.isMasterMode()) {
                        log.debug({code: code}, 'Card not found and door in master mode, adding new card');
                        AccessCard.create({code: code, isMaster: false});
                        return;
                    }

                    log.debug({code: code}, 'Card not found and door in normal mode, doing nothing');
                    door.lockRequest(door.isLocked(), false, true);

                    return;
                }

                log.debug({card: accessCard.toJSON()}, 'Found accessCard from database');

                if (door.isMasterMode()) {
                    if (accessCard.isMaster) {
                        log.debug('Ending master mode');
                        door.lockRequest(door.isLocked(), false, false);
                        return;
                    }
                    log.debug('Card already added, nothing to do');
                    // TODO: this response should be revisited
                    door.lockRequest(door.isLocked(), true, false);
                    return;
                }

                if (accessCard.isMaster) {
                    log.debug('Starting master mode');
                    // Should be ended with timeout
                    door.lockRequest(door.isLocked(), true, false);
                    return;
                }

                door.lockRequest(false, false, false);
            })
            .catch(function (err) {
                log.error({error: err}, 'Failed to process rfid card');
            });
    });

    router.route('S', function (packet, next) {
        log.info({route: 'S', data: packet.data}, 'Packet routed');

        var linkRssi = packet.data[1];
        var nfcRssiOfLast = packet.data[2];
        var locked = !!packet.data[3];
        var sysError = !!packet.data[4];

        door.setLocked(locked);
        door.setError(sysError);

    });

    return router;
};
