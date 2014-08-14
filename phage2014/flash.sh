#!/bin/sh
killall screen
sleep .5
../../stm32flash/stm32flash -b 115200 -w build/phage2014.hex /dev/ttyUSB0
