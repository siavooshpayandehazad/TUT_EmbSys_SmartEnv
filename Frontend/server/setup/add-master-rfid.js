'use strict';

var utils = require('../lib/utils');

var db = require('../lib/db')();
var AccessCard = db.models.AccessCard;

var codeArgIndex = process.argv.indexOf('--code') + 1;
var code = codeArgIndex > 0 ? process.argv[codeArgIndex] : null;

if (!code) {
    console.log('Usage: node ./add-master-rfid.js --code <ten byte hex number>');
    process.exit(1);
}

AccessCard
    .create({
        code: code,
        isMaster: true
    })
    .then(function () {
        console.log('Done');
        process.exit();
    })
    .catch(function (err) {
        console.log('Error');
        console.log(err);
        process.exit(2);
    });
