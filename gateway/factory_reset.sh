#!/bin/sh

rm -rf /gl/etc/FactorySetting
touch /gl/etc/FactoryReset-`date +%Y%m%d%H%M%S`

echo "zigbee reset"
echo 1 > /sys/class/gpio/gpio45/value
echo 0 > /sys/class/gpio/gpio47/value
sleep 1
echo 0 > /sys/class/gpio/gpio45/value
sleep 3
echo 1 > /sys/class/gpio/gpio47/value
sleep 1
echo 1 > /sys/class/gpio/gpio45/value
sleep 1
echo 0 > /sys/class/gpio/gpio45/value

echo "clear setting xml"
rm -rf /mnt/jffs2/*
cp -rf /gl/backup/* /mnt/jffs2/
systemctl stop gl-HA_Daemon.service
killall HA_Daemon
rm -rf /tmp/xml/*
echo "clear video db"
rm -rf /gl/etc/video/video_conf.db
echo "clear application db"
rm -rf /gl/etc/database/application.db
echo "clear rf_dev.json"
rm -rf /gl/etc/rf_dev.json
echo "clear bind list"
rm -rf /gl/etc/bindlist.json /gl/etc/binddev.json

echo "reset setting files"
cp -Rf /gl/etc/default/* /gl/etc/

sync
