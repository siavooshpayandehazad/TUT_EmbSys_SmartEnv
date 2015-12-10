"use strict";

var utils = require('../../utils');

module.exports = function (sequelize, DataTypes) {
    return sequelize.define('accessCard', {
        id: {type: DataTypes.INTEGER, primaryKey: true, autoIncrement: true},
        type: {type: DataTypes.ENUM('rfid'), allowNull: false, defaultValue: 'rfid'},
        code: {type: DataTypes.STRING, allowNull: false, scopes: false, validate: {
            isHex: function (value) {
                if (this.isNewRecord && !utils.isHexString(value, 10)) {
                    throw new Error('Code must be hex representation of 10-byte integer');
                }
            }
        }},
        isMaster: {type: DataTypes.BOOLEAN, allowNull: false, defaultValue: false}
    }, {
        hooks: {
/*            afterValidate: function afterValidate(accessCard) {
                if (accessCard.isNewRecord) {
                    // TODO: ensure that code cannot be changed for existing records
                    accessCard.code = utils.encrypt(accessCard.code);
                }
            },
            beforeFind: function beforeFind(options) {
                if (options.code) {
                    options.code = utils.encrypt(options.code);
                }
            }*/
        }
    });
};
