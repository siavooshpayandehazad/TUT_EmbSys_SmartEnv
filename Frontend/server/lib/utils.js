'use strict';

var crypto = require('crypto');
var config = require('easy-config');

/**
 * Checks if given string is hex number. Optionally check if has specified length in bytes.
 * @param str {string}
 * @param bytes {number}
 * @returns {boolean}
 */
module.exports.isHexString = function (str, bytes) {
    if (!str || typeof str !== 'string') {
        return false;
    }

    if (!/^[0-9a-f]+$/i.test(str)) {
        return false;
    }

    if (bytes) {
        return str.length === 2 * bytes;
    }

    return true;
};

module.exports.encrypt = function (str) {
    // TODO: This can be cracked in matter of minutes. Use stronger encryption
    return crypto.createHmac('sha256', config.crypto.key).update(str).digest('base64');
};

module.exports.quoteString = function quoteString(str, quote) {
    quote = quote || '"';

    return quote + str.replace(quote, '\\' + quote) + quote;
};
