This is a Linux driver for the 2nd generation Bosto Kingtee 22HD and Bosto Kingtee 14WA tablets.
These tablets were released around November 2013 and featured a Hanwang/Hanvon digitiser superior
to the earlier tablet versions, but that broke the Wacom compatibility that previously allowed
these tablets to work directly in Linux.

Fortunately it was discovered that there was a driver for an older Hanwang tablet already in
the kernel source and with a few modifications that driver will work for the newer Kingtee's.
Using this driver requires some manual work but is definitely possible right now. We are
working to get the drivers to a point where using the tablets under Linux will be automatic
and convenient.

Multiple drivers
================

The 14WA tablet requires two different drivers, one for the pen and one for the keys.
The pen driver works quite well (with proper tracking and pressure support) but key support
is handled by the USBHID driver which treats the tablet like a keyboard. This means the keys
emulate numbers and letters. We are working to improve this.

Current Status
==============

* Pen pressure: working
* Pen tracking: working
* Keys: produce numbers like a numeric keypad
* Scroll wheels: produce 'a', 'b', 'c', 'd' like a keyboard

**Tested programs**

Tracking and pressure sensitivity working on:

* GIMP 2.8
* MyPaint 1.1.0

In each program, you will need to find the "Input Devices" configuration and select the Bosto (change 'disabled' to 'screen') and you'll need to map axis 6 to pressure. 

Krita 2.8.1 doesn't seem to work properly, though a newer version is known to work. We're looking into it.

Inkscape 0.48's new Device configuration seems to lack the ability to map axes to functions, so pressure is not working. We're looking into it.

**Distributions**

The installation has been tested on Ubuntu 13.10 and Ubuntu 14.04. Please let us know your experiences on other distributions.

Installation
============

**Build and install the driver**

```bash
sudo apt-get install build-essential linux-headers-generic git     # install requirements
cd ~
git clone https://github.com/aidyw/bosto-2g-linux-kernel-module.git
cd bosto-2g-linux-kernel-module
git checkout <branch>   # only if you want to change the branch
make clean && make
sudo make install
```

Now if you plug in the tablet, the pen should work right away. If not, please post an issue and we'll try to improve the code.

The "git checkout" to change the branch allows you to select a different stream of development. Currently the "master" branch is mostly focused on getting the 22HD working well with Krita. The "bosto_14wa" branch is focused on getting the 14WA to work with GIMP, MyPaint, Synfig and Inkscape. The "master" branch reports the name of the tablet as a "Hanwang Art Master III" which is currently a work-around to get Krita working.

TODO
====

1. Make the pen driver load automatically when the tablet is plugged in
2. Write another or configure USBHID driver to allow remapping of keys and scroll wheels
3. Try to get the driver updated in the kernel tree so no installation is required in future

Diagnostics
===========

After running modprobe, check if the module was loaded properly with dmesg.
"Bosto 2nd Generation USB Driver module being initialised." should appear in the listing.

lsmod should also contain `bosto_2g` in its listing: lsmod | grep bosto_2g

Debug ouput now pattern matched to entries in the /sys/kernel/debug/dynamic_debug/control file
For example to see each time the driver detects a PEN_IN event, echo the following:

echo -n 'format "PEN_IN" +p' > <debugfs>/control
and off again
echo -n 'format "PEN_IN" -p' > <debugfs>/control

Another possibility based on per line number in the source file.
(See https://www.kernel.org/doc/Documentation/dynamic-debug-howto.txt )
echo -n 'file ./<path to source>/bosto_2g.ko line 230 +p' > <debugfs>/control

Feedback
========

The best place for feedback is probably the Bosto community Google Group:
https://groups.google.com/forum/#!categories/bosto-user-group/mac--linux-discussion
