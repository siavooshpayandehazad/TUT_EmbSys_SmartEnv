
# Node.js server for TUT Home Automation project

## Stack

+ [node.js](https://nodejs.org/)
+ [MySQL](https://www.mysql.com/)


## Install

Install system dependencies

[Install node.js](http://node-arm.herokuapp.com/)
// TODO: add commands for installing node.js and mysql on raspbian

Install global node modules

`npm install -g bower`
`npm install -g forever`

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

Following example shows fields that should be overridden.

```
{
  "log": {
    "bunyan": {
      "name": "hafe-server",// Logger name
      "level": "trace"// Logging level
    },
    "file": "~/.forever/server.log"// Path to log file
  },

  "serial": {
    "device": "/dev/ttyACM0"// Path to serial device
  },

  "hass": {
    "bufferFile": "data-from-hass.local"// File used to read commands from HomeAssistant (user interface)
  },

  "mysql": {// Mysql server details
    "host": "127.0.0.1",
    "port": 3306,
    "user": "root",
    "pass": "",
    "db": "hafe"
  },

  "crypto": {
    "key": "lrx9mcl0ol9jm7vi"// Key used to encrypt sensitive data
  },

  "modules": {// Home automation components
    "_statusDir": "../hass/data/",// Path to component states data folder
  }
}
```


## Run

Start mysql server

`systemctl start mysqld`

Use `forever` to keep server alive after crashes.

`forever start -l hafe-server.log --append --workingDir ~/home-automation/Frontend/server/ --sourceDir ~/home-automation/Frontend/server/ server.js`


// TODO: running server on system startup
