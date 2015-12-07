'use strict';

module.exports.serializers = {
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
    }
};
