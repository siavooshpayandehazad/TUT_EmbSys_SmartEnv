'use strict';

var fs = require('fs');

var config = require('easy-config');
var express = require('express');

module.exports = function () {

    var app = express();

    app.use('/debug', require('./debug')());

    return app;
};
