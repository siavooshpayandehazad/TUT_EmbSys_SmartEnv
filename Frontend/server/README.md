
# Node.js server for TUT Home Automation project

## Stack

+ [node.js](https://nodejs.org/)
+ [MySQL](https://www.mysql.com/)


## Install

Install system dependencies

Install mysql with apt-get.

[Install node.js](http://node-arm.herokuapp.com/)


Install global node modules

```
npm install -g bower
npm install -g bunyan
npm install -g forever
```

Clone Frontend files from GitHub

```
cd ~
mkdir home-automation
cd home-automation
git init
git config core.sparseCheckout true
git remote add -f origin https://github.com/siavooshpayandehazad/TUT_EmbSys_SmartEnv
echo Frontend/* > .git/info/sparse-checkout
git checkout master
```

Install server dependencies

```
cd Frontend/server
npm install
bower install
```


## Configure

To override default configuration, add `config/config.dev.json` file (use `config/config.pro.json` for production server).
For more details, see [easy-config](https://github.com/DeadAlready/node-easy-config) documentation.

Configuration is just a JSON with following properties:

```
{
  "name": "tut-home-automation-fe",// Server name

  "log": {// Logging options
    "bunyan": {// Options for logger (See bunyan documentation for details: https://github.com/trentm/node-bunyan)
      "name": "hafe-server",
      "level": "trace"
    },
    "file": "./log/server.log"// Log file location. Needed for debug web interface
  },

  "webInterface": {
    "port": 3000// Port for debug web interface
  },

  "rf": {
    "address": 4// Address of rf module
  },

  "serial": {// Serial options
    "device": "/dev/ttyACM0",
    "options": {
      "baudrate": 9600,
      "dataBits": 8,
      "stopBits": 1,
      "parity": "none"
    },
    "incoming": {// Options for handling incoming packets
      "timeout": 30// Time interval (ms) in which the whole packet must arrive
    },
    "outgoing": {
      "delay": 50,// Delay (ms) between two outgoing packets (measured from start of the packages)
      "timeout": 1000,// Currently not used. Time (ms) in which response to a packet must arrive
      "retry": 3000,// Currently not used. Interval (ms) for retrying sending failed packets
      "retryCount": 3// Currently not used. Number of retries for failed packets
    }
  },

  "hass": {// Options related to Home Assistant
    "bufferFile": "data-from-hass.local"// File that acts as a buffer for getting command from Home Assistant
  },

  "mysql": {// MySQL conf
    "host": "127.0.0.1",
    "port": 3306,
    "user": "root",
    "pass": "",
    "db": "hafe"
  },

  "crypto": {
    "key": "9nmn2oopmnlgzaor"// Key for encrypting sensitive data for storage
  },

  "modules": {// Configuration for connected sensor modules
    "_statusDir": "../hass/data/",// Directory for storing status files. Needed for displaying in Home Assistant

    "fingerprint": {
      "address": 1
    },

    "rfid": {// Module name
      "address": 2,// Rf address of module
      "statusFiles": {// Status files for module in form {"filename": "type", ...}
        "p2.lock": "bool",
        "p2.status": "enum(ok,fault)"
      }
    },

    "outdoor": {
      "address": 3,
      "statusFiles": {
        "p3.temp": "number",
        "p3.humid": "number",
        "p3.pressure": "number",
        "p3.light": "number",
        "p3.battery": "enum(ok,low)"
      },
      "poll": {// Status polling
        "start": 1000,// Time (ms) from starting of the server, when first status request is sent. Should be different for each module to avoid packet collision
        "interval": 60000// Interval (ms) for repeating status request
      }
    },

    "fishFeeder": {
      "address": 5,
      "statusFiles": {
        "p5.light": "enum(on,off)",
        "p5.water": "enum(ok,low)",
        "p5.feed": "enum(ok,low)",
        "p5.temp": "number",
        "p5.temp-state": "enum(low,ok,high)",
        "p5.filter": "enum(ok,dirty)"
      },
      "poll": {
        "start": 1500,
        "interval": 60000
      }
    },

    "flowerBox": {
      "address": 6,
      "statusFiles": {
        "p6.light": "enum(on,off)",
        "p6.temp": "number",
        "p6.moisture": "number",
        "p6.humid": "number"
      },
      "poll": {
        "start": 2000,
        "interval": 60000
      }
    },

    "sunFollower": {
      "address": 12
    },

    "meterReporter": {
      "address": 14,
      "statusFiles": {
        "p14.water": "number",
        "p14.temp": "number"
      }
    },

    "indoorLighting": {
      "address": 15,
      "statusFiles": {
        "p15.led1": "number",
        "p15.led2": "number",
        "p15.led3": "number",
        "p15.led4": "number",
        "p15.led5": "number",
        "p15.led6": "number"
      },
      "poll": {
        "start": 2500
      }
    }
  }
}
```


## Run

Start mysql server

`systemctl start mysqld`

Use `forever` to keep server alive after crashes.

`forever start -l hafe-server.log --append --workingDir ~/home-automation/Frontend/server/ --sourceDir ~/home-automation/Frontend/server/ server.js`


// TODO: running server on system startup
