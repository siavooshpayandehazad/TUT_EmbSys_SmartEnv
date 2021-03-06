'use strict';

var childProcess = require('child_process');
var fs = require('fs');
var path = require('path');

var bunyan = require('bunyan');
var config = require('easy-config');
var express = require('express');

var utils = require('../../lib/utils');

var rootDir = path.join(__dirname, '../..');

module.exports = function () {

    var app = express();

    app.get('/log', function (req, res, next) {

        var level = req.query.level;
        if (parseInt(level, 10)) {
            level = parseInt(level, 10);
        } else {
            level = bunyan.levelFromName[level];
        }

        if (level && typeof level !== 'number') {
            res.status(400).json({
                status: 'fail',
                data: {
                    level: {
                        field: 'level',
                        value: req.query.level,
                        msg: 'Invalid value'
                    }
                }
            });
            return;
        }

        var timeStart = req.query.timeStart;
        if (timeStart && !Date.parse(timeStart)) {
            res.status(400).json({
                status: 'fail',
                data: {
                    level: {
                        field: 'timeStart',
                        value: timeStart,
                        msg: 'Invalid value'
                    }
                }
            });
        }

        var timeEnd = req.query.timeEnd;
        if (timeEnd && !Date.parse(timeEnd)) {
            res.status(400).json({
                status: 'fail',
                data: {
                    level: {
                        field: 'timeEnd',
                        value: timeEnd,
                        msg: 'Invalid value'
                    }
                }
            });
        }

        var cmd = ['bunyan --output bunyan --strict --no-color'];

        if (level) {
            cmd.push('--level', level);
        }

        if (timeStart) {
            cmd.push('--condition', '\'this.time >=', utils.quoteString(timeStart) + '\'');
        }
        if (timeEnd) {
            cmd.push('--condition', '\'this.time <=', utils.quoteString(timeEnd) + '\'');
        }

        cmd.push(path.isAbsolute(config.log.file) ? config.log.file : path.join(rootDir));

        cmd = cmd.join(' ');

        childProcess.exec(cmd, {cwd: rootDir, maxBuffer: 1024*1024}, function (err, stdout, stderr) {
            if (err || stderr) {
                req.log.error({err: err, stderr: stderr}, 'Failed to read log file');
                res.status(400).json({
                    status: 'fail',
                    data: 'Failed to read log files'
                });
                return;
            }

            var logs = stdout
                .trim()
                .split(/[\r\n]+(?=\{)/)
                .filter(function (str) {
                    return !!str;
                })
                .map(function (str) {
                    try {
                        return JSON.parse(str);
                    } catch (err) {
                        req.log.debug({err: err, log: str}, 'Failed to parse log entry');
                    }
                })
                .filter(function (json) {
                    return !!json;
                });

            res.status(200).json(logs);
        });

    });

    return app;
};
