#!/bin/sh

# Script called by udev to initialise bosto_2g module during usb hotplug event.

logger udev rule triggered to load bosto_2g
#Make sure bosto module is not loaded
rmmod bosto_2g
logger removed bosto_2g driver module from kernel

#Detach the usbhid module
/usr/local/bin/detach_usbhid
logger detached usbhid driver module from bosto device

#Now insert Bosto module
insmod /lib/modules/$(uname -r)/bosto_2g.ko
