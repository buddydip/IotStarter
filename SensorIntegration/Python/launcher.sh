#!/bin/sh
# launcher.sh
cd /
sudo service grafana-server start
cd /home/pi/IoT/Smarty/Python
sudo python3 SmartyMQTTDataBridge.py
cd /

