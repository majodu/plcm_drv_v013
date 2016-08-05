#! /bin/bash

if [[ $EUID -ne 0 ]]; then
   echo "This script must be run as root!" 
   exit 1
fi
echo "Stop lcd-menu service:"
stop lcd-menu

echo "Compile and install lcd-menu service:"
mv plcm_drv_v013.tar.gz /root/.
cd /root
tar xvfz plcm_drv_v013.tar.gz
cd plcm_drv_v013
make
make boot -i
cp lcd-menu.conf /etc/init/.
echo "Start lcd-menu service:"
start lcd-menu
cd ..



