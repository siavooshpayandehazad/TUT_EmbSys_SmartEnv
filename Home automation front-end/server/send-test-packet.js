'use strict';

var RedisSMQ = require('rsmq');

var rsmq = new RedisSMQ({
    host: "127.0.0.1",
    port: 6379,
    ns: "rsmq"
});

console.log('creating queue');
rsmq.createQueue({qname:"myqueue"}, function (err, resp) {
    if (resp===1) {
        console.log("queue created");

        rsmq.sendMessage({qname: "packets", message: "[7, 2, 4, 83, 97, 98, 99]"}, function (err, resp) {
            console.log('message sent');
            console.log(err);
            console.log(resp);
            if (resp) {
                console.log("Message sent. ID:", resp);
                process.exit();
            }
        });
    }
});
