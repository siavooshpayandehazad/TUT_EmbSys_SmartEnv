'use strict';

var SerialPort = require('serialport').SerialPort;

var serial;
var onOpenQueue = [];


function Serial(opts) {
    if (!(this instanceof Serial)) {
        return new Serial(opts);
    }

    this._log = opts.log;
    this._listener = opts.listener;
    this._config = opts.config;
    this._serial = null;
    this._writeQueue = [];

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
            _this._log.error({err: err, config: _this._config.serial}, 'Failed to open serial port');
            return;
        }

        _this._log.debug({device: _this._config.serial.device}, 'Serial port opened');

        _this._serial.flush();
        _this._serial.on('data', onData);

        if (opts.onOpen) {
            setImmediate(opts.onOpen);
        }
    }

    function onError(err) {
        _this._log.error({err: err}, 'Serial error');
    }

    function onClose() {
        _this._log.warn('Serial closed. Trying to reopen');
        setTimeout(open, 1000);
    }


    var byteCount = 0;
    var packetLength = undefined;
    var packetData = new Buffer(0);
    var timer = null;
    var timeout = opts.config.serial.incoming.timeout;

    function onData(data) {
        _this._log.trace({data: data.inspect()}, 'Incoming data');

        if (byteCount === 0) {
            _this._log.trace('Packet start received');
            // The whole packet must come in specified time frame
            timer = setTimeout(resetPacket, timeout);

            // First byte defines packet length
            packetLength = data[0];
        }

        packetData = Buffer.concat([packetData, data]);
        byteCount += data.length;

        if (byteCount < packetLength) {
            _this._log.trace({data: packetData.inspect()}, 'Partial packet');
            // The whole packet not received yet
            return;
        }

        // Packet received
        clearTimeout(timer);

        // Might be more data than one packet
        while (byteCount && packetLength && byteCount >= packetLength) {

            // Slice packet for listener
            var currentPacket = packetData.slice(0, packetLength);
            _this._log.trace({data: currentPacket.inspect()}, 'Packet received');
            setImmediate(_this._listener, currentPacket);

            // Reorganize remaining data
            packetData = packetData.slice(packetLength);
            byteCount = packetData.length;
            packetLength = packetData[0];
        }

        if (byteCount > 0) {
            _this._log.trace({data: packetData.inspect(), byteCount: byteCount, length: packetLength}, 'Next packet partially received');
            // Some data remained. Assuming it belongs to next packet. Set timeout for next packet
            timer = setTimeout(resetPacket, timeout);
        }

        function resetPacket() {
            _this._log.error({timeout: timeout, data: packetData}, 'Incoming packet timeout');
            packetData = new Buffer(0);
            byteCount = 0;
            packetLength = undefined;
        }
    }
}

Serial.prototype.write = function write(data, opts, cb) {
    if (typeof opts === 'function') {
        cb = opts;
        opts = null;
    }

    var _this = this;

/*    if (opts && opts.checkResponse) {
        // This is really bad logic for checking if we get a response. This should actually be handled on lower level

        // TODO:
        var timeout = checkResponse.timeout || 200;
        var expectedResponse;
    }*/

    if (typeof data === 'object' && !Array.isArray(data)) {
        data = [
            data.data.length + 1,// length does not include length byte itself
            data.to
        ].concat(bytesToInts(data.data));
    } else if (Array.isArray(data)) {
        data = bytesToInts(data);
    }

    if (!Array.isArray(data) || data.length < 3 || data.length > 18 || data[0] !== data.length) {
        if (cb) {
            cb(new TypeError('Invalid data'));
        }
        return;
    }


    if (_this._writeDelayed) {
        _this._writeQueue.push(_this._serial.write.bind(_this._serial, data, serialCb));
        return;
    }

    _this._writeDelayed = true;
    setTimeout(_this._executeWriteQueue.bind(_this), _this._config.serial.outgoing.delay);

    _this._serial.write(data, serialCb);


    function serialCb(err) {
        if (err) {
            _this._log.error({err: err, data: data}, 'Failed to write to serial');

            if (cb) {
                cb(err);
            }
            return;
        }

        _this._log.debug({data: data}, 'Data successfully written to serial');

        if (cb) {
            cb();
        }
    }
};

Serial.prototype.isOpen = function isOpen() {
    return this._serial && this._serial.isOpen();
};

Serial.prototype._executeWriteQueue = function _executeWriteQueue() {
    var _this = this;

    if (!_this._writeQueue || !_this._writeQueue.length) {
        _this._writeDelayed = false;
        return;
    }

    _this._writeQueue.shift()();
    setTimeout(_this._executeWriteQueue.bind(_this), _this._config.serial.outgoing.delay);
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
    write: function write(data, opts, cb) {
        if (!serial) {
            if (cb) {
                cb(new Error('Serial not initialized'));
            }

            return;
        }

        serial.write(data, opts, cb);
    },
    onOpen: function onOpen(fn) {
        if (serial && serial.isOpen()) {
            setImmediate(fn);
        } else {
            onOpenQueue.push(fn);
        }
    }
};
