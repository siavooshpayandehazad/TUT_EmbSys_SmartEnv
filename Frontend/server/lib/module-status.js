'use strict';

var async = require('async');
var fs = require('fs');
var path = require('path');

// TODO: Use promises instead of callbacks

function ModuleStatus(opts) {
    if (!(this instanceof ModuleStatus)) {
        return new ModuleStatus(opts);
    }

    if (!opts || !opts.dir || !opts.files || !Object.keys(opts.files).length) {
        throw TypeError('Failed to initialize StatusFile: opts.dir and opts.files are required');
    }

    var _this = this;

    _this._dir = opts.dir;
    _this._files = {};

    Object.keys(opts.files).forEach(function (filename) {
        var typeDefinition = opts.files[filename];
        var type = typeDefinition.match(/^\w+/g);
        type = type && type[0];

        if (ModuleStatus.types.indexOf(type) === -1) {
            throw new TypeError('Invalid type defined for ' + filename);
        }

        _this._files[filename] = {
            name: filename,
            type: type
        };

        if (type === 'enum') {
            var enumValues = typeDefinition.match(/^enum\((.+)\)/);
            enumValues = enumValues && enumValues[1];
            enumValues.split(',');

            if (!enumValues || !enumValues.length) {
                throw new TypeError('Failed to parse enum values for ' + filename);
            }

            _this._files[filename].values = enumValues.split(',');
        }
    });
}

// TODO: named bool would be nice to use, instead of two-value enums
ModuleStatus.types = ['bool', 'number', 'enum'];

ModuleStatus.prototype.update = function update(name, value, cb) {

    var file = this._files[name];

    if (!file) {
        if (cb) {
            cb(new Error('No such state file defined: ' + name));
        }
        return;
    }

    if (file.type === 'enum' && file.values.indexOf(value) === -1) {
        if (cb) {
            cb(new TypeError('Value not defined in enum ' + name + ': ' + value));
        }
        return;
    }

    if (file.type === 'number' && typeof value !== 'number') {
        if (cb) {
            cb(new TypeError('State file ' + name + ': Expecting number value'));
        }
        return;
    }

    if (file.type === 'bool') {
        value = value ? '1' : '0';
    }

    if (file.type !== 'number' && typeof value !== 'string') {
        if (cb) {
            cb(new TypeError('State file ' + name + ': Expecting string value'));
        }
        return;
    }

    value = value.toString();

    fs.writeFile(path.join(this._dir, name), value, {encoding: 'utf8'}, cb);
};

ModuleStatus.prototype.updateMany = function updateMany(map, cb) {
    var _this = this;

    async.each(Object.keys(map), function (key, cb) {
        _this.update(key, map[key], cb);
    }, function (err) {
        if (cb) {
            cb(err);
        }
    });
};

ModuleStatus.prototype.read = function read(name, cb) {

    var file = this._files[name];

    if (!file) {
        setImmediate(cb, new TypeError('No such state file defined: ' + name));
        return;
    }

    var filePath = path.join(this._dir, name);

    fs.readFile(filePath, {encoding: 'utf8'}, function (err, data) {
        // Ignore ENOENT errors (No such file or directory)
        if (err && err.code !== 'ENOENT') {
            setImmediate(cb, err);
            return;
        }

        var value = data;

        switch (file.type) {
            case 'number':
                value = parseInt(value, 10) || 0;
                break;
            case 'bool':
                value = !!(parseInt(value, 10) || value === 'true');
                break;
        }

        setImmediate(cb, null, value);
    });
};

ModuleStatus.prototype.readMany = function readMany(names, cb) {

    var _this = this;

    var missing = [];

    names.forEach(function (name) {
        if (!_this._files[name]) {
            missing.push(name);
        }
    });

    if (missing.length) {
        setImmediate(cb, new TypeError('No such file(s) defined: ' + missing.join(', ')));
        return;
    }

    async.map(names, function (name, cb) {
        _this.read(name, cb);
    }, function (err, results) {
        if (err) {
            cb(err);
            return;
        }

        var map = {};

        names.forEach(function (name, index) {
            map[name] = results[index];
        });

        cb(null, map);
    });

};

module.exports = ModuleStatus;
