#!/bin/sh

cat /proc/cpuinfo | grep MHz | head -n 1 | cut -d : -f 2 | cut -d " " -f 2
