#!/bin/sh

project=$1
type=$2
value=$3

echo "$1$2$3" >> "../../server/data-from-hass.local"
