'use strict';

var SerialPort = require('serialport').SerialPort;

var serial;
var onOpenQueue = [];

// TODO: send [length, to, payload max 16bytes]

function Serial(opts) {
    if (!(this instanceof Serial)) {
        return new Serial(opts);
    }

    this._log = opts.log;
    this._listener = opts.listener;
    this._config = opts.config;
    this._serial = null;

    var _this = this;

    open();

    function open() {
        if (!_this._serial) {
            _this._serial = new SerialPort(_this._config.serial.device, _this._config.serial.options);
            _this._serial.on('open', onOpen);
            _this._serial.on('error', onError);
            _this._serial.on('close', onClose);
        } else {
            _this._serial.open(onOpen);
        }
    }

    function onOpen(err) {
        if (err) {
            _this._log.error({error: err, config: _this._config.serial}, 'Failed to open serial port');
            return;
        }

        _this._log.debug({device: _this._config.serial.device}, 'Serial port opened');

        _this._serial.flush();
        _this._serial.on('data', _this._listener);

        if (opts.onOpen) {
            setImmediate(opts.onOpen);
        }
    }

    function onError(err) {
        _this._log.error({error: err}, 'Serial error');
    }

    function onClose() {
        _this._log.warn('Serial closed. Trying to reopen');
        setTimeout(open, 1000);
    }
}

Serial.prototype.write = function write(data, cb) {
    var _this = this;

    if (typeof data === 'object' && !Array.isArray(data)) {
        data = [
            data.data.length,
            data.to,
            _this._config.rf.address,
            0,
            0
        ].concat(bytesToInts(data.data));
    } else if (Array.isArray(data)) {
        data = bytesToInts(data);
    }

    _this._serial.write(data, function (err) {
        if (err) {
            _this._log.error({error: err, data: data}, 'Failed to write to serial');

            if (cb) {
                cb(err);
            }
            return;
        }

        _this._log.debug({data: data}, 'Data successfully written to serial');

        if (cb) {
            cb();
        }
    });
};

Serial.prototype.isOpen = function isOpen() {
    return this._serial && this._serial.isOpen();
};

function bytesToInts(array) {
    if (!Array.isArray(array)) {
        throw TypeError('bytesToInts: argument must be an Array');
    }

    return array.map(function (byte) {
        var type = typeof byte;
        if (type !== 'string' && type !== 'number') {
            throw TypeError('bytesToInts: Cannot convert to integers: Byte array must contain only characters or integers');
        }
        return (type === 'string') ? byte.charCodeAt(0) : byte;
    });
}

module.exports = {
    init: function init(opts) {

        if (onOpenQueue.length) {
            var original = opts.onOpen;
            opts.onOpen = function () {
                onOpenQueue.forEach(function (fn) {
                    setImmediate(fn);
                });
                onOpenQueue = [];

                if (original) {
                    original();
                }
            };
        }

        serial = new Serial(opts);
        return serial;
    },
    write: function write(data, cb) {
        if (!serial) {
            if (cb) {
                cb(new Error('Serial not initialized'));
            }

            return;
        }

        serial.write(data, cb);
    },
    onOpen: function onOpen(fn) {
        if (serial && serial.isOpen()) {
            setImmediate(fn);
        } else {
            onOpenQueue.push(fn);
        }
    }
};
