#!/bin/sh

#cat /proc/cpuinfo | grep MHz | head -n 1 | cut -d : -f 2 | cut -d " " -f 2
var=$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq)
echo $((var/1000))
