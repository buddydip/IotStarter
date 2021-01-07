#!/bin/sh
# launcher.sh

sleep 60

cd /
cd /home/pi/IoT/Smarty
sudo python3 SmartyMQTTInfluxDBBridge.py
cd /