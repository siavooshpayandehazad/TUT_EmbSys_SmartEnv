'use strict';

var serializers = {
    data: function (data) {
        if (!(data instanceof Buffer) && !(data instanceof Array)) {
            return data;
        }

        var bytes = [];

        for (var i = 0, l = data.length; i < l; ++i) {
            bytes.push(parseInt(data[i], 10) || data[i]);
        }

        return {
            dec: bytes.join(' '),
            hex: bytes
                .map(function (byte) {
                    return byte.toString(16);
                })
                .join(' ')
        };
    },
    packet: function (packet) {
        var reduced = {};

        Object.keys(packet).forEach(function (key) {
            if (key === 'log') {
                return;
            }

            if (['data', 'raw'].indexOf(key) !== -1) {
                reduced[key] = serializers.data(packet[key]);
                return;
            }

            reduced[key] = packet[key];
        });

        return reduced;
    }
};

module.exports.serializers = serializers;
