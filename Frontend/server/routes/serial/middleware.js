'use strict';

module.exports.extractCommandByte = function (packet) {
    packet.route = String.fromCharCode(packet.data[0]);
};
