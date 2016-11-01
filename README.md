This is a Linux driver for the 2nd generation Bosto Kingtee 22HD and Bosto Kingtee 14WA tablets.
These tablets were released around November 2013.

Using this driver requires some manual work but is definitely possible right now. We are
working to get the drivers to a point where using these tablets under Linux will be automatic
and convenient.

Multiple drivers
================

The 14WA tablet requires two different drivers, one for the pen and one for the keys.
The pen driver works quite well (with proper tracking and pressure support) but key support
is handled by the USBHID driver which treats the tablet like a keyboard. This means the keys
emulate numbers and letters. We are working to improve this.
The pen driver has a landing spot in a switch statement to handle buttons events from the tablet. This is a work in progress.
Hardware needed such as 22HD+ with tablet buttons.

Current Status
==============

* Pen pressure: working (pressure magnified by pen tool button. Usage: Ignore the right click response, maintain the button pressed, and continue to draw with magnified pressure sensitivity.)
* Pen tracking: working
* Keys: produce numbers like a numeric keypad
* Scroll wheels: produce 'a', 'b', 'c', 'd' like a keyboard
* Removed fuzz factor from driver when reporting absolute position x & y. This is not a joystick and we should have negligable jitter on the reported position.
* ~~Added delay before pressure reports when tool is presented for the first time.~~
* Added conditional for origin 0,0, and don't report. This prevents lines drawn to the absolute origin when pen first in or when stylus button presses during contact. We loose one pixel. I'm not loosing any sleep.

**Tested programs**

Tracking and pressure sensitivity working on:

* GIMP 2.8
* MyPaint 1.1.0
* Inkscape 0.48
* Synfig 0.64.1
* Krita 2.9.7 (Nov 2016 this is the distro version on Ubuntu) 
* Blender 2.76b

In each program, you will need to find the "Input Devices" configuration and select the Bosto (change 'disabled' to 'screen') and you might need to map axis 3 to pressure (but that's usually the default).

* Krita 2.7.2 has tracking but it's unknown right now whether pressure works.
* Krita 2.8.1 doesn't seem to work at all.
* Krita 3+ works without pressure. Working on this with krita guys?

**Distributions**

The installation has been tested on Ubuntu 13.10, 14.04, 16.04 LTS. Please let us know your experiences on other distributions.


In each program, you will need to find the "Input Devices" configuration and select the Bosto (change 'disabled' to 'screen') and you might need to map axis 3 to pressure (but that's usually the default).

Krita 2.8.1 doesn't seem to work properly, though a newer version is known to work. Later versions 3+ have major rework within Krita, its broken something. We're looking into it.

Installation
============

**Build and install the driver**

```bash
sudo apt-get install libusb-1.0.0-dev build-essential linux-headers-generic git     # install requirements
cd ~
git clone https://github.com/aidyw/bosto-2g-linux-kernel-module.git
cd bosto-2g-linux-kernel-module
git checkout <branch>   # only if you want to change the branch
make clean && make
sudo make install
```

**Load the module**
Now if you plug in the tablet, the pen should work right away. If not, please post an issue and we'll try to improve the code.

If you have a multiscreen setup you can automagically! adjust the transform matrix on X using the wonderful little script written by Leslie  'inputtransform'.
This helps you identify the Tablet monitor and adjust accordingly.

The "git checkout" to change the branch allows you to select a different stream of development.
The "master" branch is the default and should be the right choice for the official release version.
Keep an eye on "testing" any modifications are happening here and will merge as appropriate into master.


TODO
====

1. Make the pen driver load automatically when the tablet is plugged in  <-- done (22HD (master branch) now uses usblib based userspace code to detach the generic kernel module. 14WA using unbind.
2. Write another or configure USBHID driver to allow remapping of keys and scroll wheels
3. Try to get the driver updated in the kernel tree so no installation is required in future
4. Understand and improve the mapping of the rubber tool, to a working software interface. Krita etc ? Probably need to report each tool as a different device?
5. Consider implementing fuzz factor for the position reports when eraser tool is presented. The eraser tool currently reports with a great deal of jitter. (Hardware)

Diagnostics
===========

After running modprobe, check if the module was loaded properly with dmesg.
"Bosto 2nd Generation USB Driver module being initialised." should appear in the listing.

lsmod should also contain `bosto_2g` in its listing: lsmod | grep bosto_2g

Debug ouput now pattern matched to entries in the /sys/kernel/debug/dynamic_debug/control file


For example to see each time the driver detects a TOOL event, echo the following:

echo -n 'format "Bosto TOOL" +p' > /sys/kernel/debug/dynamic_debug/control

Then view the debug output in syslog. To switch debug output off again:

echo -n 'format "Bosto TOOL" -p' > /sys/kernel/debug/dynamic_debug/control

check the 'dev_dbg' lines in bosto_2g.c for other debug pattern matches that are possible.

Another possibility based on per line number in the source file.
(See https://www.kernel.org/doc/Documentation/dynamic-debug-howto.txt )
echo -n 'file ./<path to source>/bosto_2g.ko line 230 +p' > <debugfs>/control

Known Hardware Issues
=====================

The Bosto 22HD (v1) Shipped with a firmware issue. If the USB port is detached and then re-attached, the data coming from the tablet becomes corrupted when pressure exceeds a certain level.
To resolve the issue it is necessary to power cycle the tablet totally, removing both power input and USB cable, in some cases even a DVI connector.
The tablet will remain internally powered from any available source and all power sources need to be removed to make the USB firmware reboot.

Feedback
========

The best place for feedback is probably the Bosto community Google Group:
https://groups.google.com/forum/#!categories/bosto-user-group/mac--linux-discussion

