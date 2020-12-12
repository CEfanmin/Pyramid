#!/bin/bash
# chkconfig: 2345 30 15

cd /home/mosquittoMessage/
mosquitto &
sleep 2
/etc/init.d/kafka startzookeeper &
sleep 2
/etc/init.d/kafka startserver &

