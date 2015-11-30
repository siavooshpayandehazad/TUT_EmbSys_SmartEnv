#!/bin/bash

startAll() {
    systemctl start mysqld
}
stopAll() {
    systemctl stop mysqld
}
statusAll() {
    systemctl status mysqld
    echo ""
}

command="start"
if [ -n "$1" ]; then
    command=$1
fi

case $command in
"start")
    startAll
    ;;
"stop")
    stopAll
    ;;
"restart")
    stopAll
    startAll
    ;;
"status")
    statusAll
    ;;
*)
    echo "Usage: $0 {start|stop|restart|status}"
esac
