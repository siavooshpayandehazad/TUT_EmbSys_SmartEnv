'use strict';

var fs = require('fs');

function FileBuffer(opts) {
    if (!(this instanceof FileBuffer)) {
        return new FileBuffer(opts);
    }

    if (!opts || !opts.filename) {
        throw TypeError('Failed to initialize FileBuffer: opts.filename must be specified');
    }

    // TODO: create file if not exists
    this._filename = opts.filename;
    this._handlers = {
        data: []
    };

    this.init();
}

FileBuffer.prototype.init = function init() {
    var _this = this;

    // TODO: make it possible to stop watching
    // TODO: use promises

    fs.watch(this._filename, function (event, filename) {
        if (event !== 'change') {
            return;
        }

        fs.readFile(_this._filename, {encoding: 'utf-8'}, function (err, contents) {
            if (!contents) {
                // File is empty.
                return;
            }

            fs.truncate(_this._filename, 0, function (err) {

                var lines = contents.split(/[\n\r]+/);

                lines.forEach(function (line) {
                    if (!line) {
                        // Ignore empty lines
                        return;
                    }

                    _this._handlers.data.forEach(function (handler) {
                        setImmediate(handler, line);
                    });
                });

            })
        });
    });
};

FileBuffer.prototype.on = function on(event, handler) {
    this._handlers[event].push(handler);
};

module.exports = FileBuffer;
