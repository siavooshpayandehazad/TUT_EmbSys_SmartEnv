"use strict";

module.exports = function (sequelize, DataTypes) {
    return sequelize.define('rfidCard', {
        id: {type: DataTypes.INTEGER, primaryKey: true, autoIncrement: true},
        code: {type: DataTypes.STRING, allowNull: false},
        isMaster: {type: DataTypes.BOOLEAN, allowNull: false, defaultValue: false}
    });
};
