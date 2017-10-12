CC=gcc
INC=-I/usr/include/libusb-1.0
LIB=-L/lib/$(arch)-linux-gnu/libusb-1.0.so.0

.PHONY: all clean archive

obj-m += hid-bosto-2g.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

archive:
	tar f - --exclude=.git -C ../ -c bosto_2g | gzip -c9 > ../hid-bosto-2g-`date +%Y%m%d`.tgz

install:
	cp ./hid-bosto-2g.ko /lib/modules/$(shell uname -r)
	echo hid-bosto-2g >> /etc/modules
	depmod
	modprobe hid-bosto-2g

uninstall:
	/sbin/udevadm control --reload
	rm /lib/modules/$(shell uname -r)/hid-bosto-2g.ko
	sed -i '/hid-bosto-2g/d' /etc/modules
	depmod
	rmmod hid-bosto-2g
