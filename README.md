This is a Linux driver for the 2nd generation Bosto Kingtee 22HD and Bosto Kingtee 14WA tablets. These tablets were released around November 2013 and featured a Hanwang/Hanvon digitiser superior to the earlier tablet versions, but that broke the Wacom compatibility that previously allowed these tablets to work directly in Linux.

Fortunately it was discovered that there was a driver for an older Hanwang tablet already in the kernel source and with a few modifications that driver will work for the newer Kingtee's. Using this driver requires some manual work but is definitely possible right now. We are working to get the drivers to a point where using the tablets under Linux will be automatic and convenient.

Multiple drivers
================

The 14WA tablet requires two different drivers, one for the pen and one for the keys. The pen driver works quite well (with proper tracking and pressure support) but key support is handled by the USBHID driver which treats the tablet like a keyboard. This means the keys emulate numbers and letters. We are working to improve this.

Current Status
==============

* Pen pressure: working
* Pen tracking: working
* Keys: produce numbers like a numeric keypad
* Scroll wheels: produce 'a', 'b', 'c', 'd' like a keyboard

Installation
============

These instructions are for Ubuntu 14.04 but will likely work or be adaptable to other distributions.

**Build and install the driver**

```bash
sudo apt-get install build-essential linux-headers-generic git     # install requirements
cd ~
git clone https://github.com/aidyw/bosto-2g-linux-kernel-module.git
cd bosto-2g-linux-kernel-module
make clean && make
ln -s `pwd`/bosto_2g.ko /lib/modules/`uname -r`
depmod
```

**Load the module**

The USBHID module grabs the pen's USB interface as soon as the tablet is plugged in. It has to be unbound before the driver can work. First find out what the bus ID of the tablet is:

```bash
# unplug the tablet
ls /sys/bus/usb/drivers/usbhid

# take note of the bound interface ID's, numbers like '2-3:1.0'. Or you might not have any.
# plug in the tablet
# now see which numbers were added by the tablet
ls /sys/bus/usb/drivers/usbhid

# the highest new number will be the pen's USB interface.
# set up the id for us to bind/unbind:
USB_BUS_ID="2-3:1.0"    # substitute your own Bus ID here
```

Now unbind and rebind the USB port to the right driver:

```bash
sudo -i                                                           # be careful, admin permissions
modprobe bosto_2g                                                 # load the driver
echo -n "$USB_BUS_ID" > /sys/bus/usb/drivers/usbhid/unbind        # unbind USBHID
echo -n "$USB_BUS_ID" > /sys/bus/usb/drivers/bosto_2g/bind        # bind bosto_2g
exit
```

TODO
====

1. Make the pen driver load automatically when the tablet is plugged in
2. Write another or configure USBHID driver to allow remapping of keys and scroll wheels
3. Try to get the driver updated in the kernel tree so no installation is required in future
