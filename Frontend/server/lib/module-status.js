'use strict';

var async = require('async');
var fs = require('fs');
var path = require('path');

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

module.exports = ModuleStatus;
