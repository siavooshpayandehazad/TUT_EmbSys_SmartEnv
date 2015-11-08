"use strict";

var config = require('easy-config');
var Sequelize = require('sequelize');

var sequelize = new Sequelize(config.mysql.db, config.mysql.user, config.mysql.pass, {
    host: config.mysql.host,
    port: config.mysql.port,
    dialect: 'mysql'
});


var models = {
    RfidCard: sequelize.import(__dirname + '/models/rfid-card.js')
};

sequelize.sync();

module.exports = function () {
    return {
        db: sequelize,
        models: models
    };
};
