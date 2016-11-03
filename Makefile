CC=gcc
INC=-I/usr/include/libusb-1.0 
LIB=-L/lib/$(arch)-linux-gnu/libusb-1.0.so.0

.PHONY: all clean archive

obj-m += bosto_2g.o
 
all:
	$(CC) detach_usbhid.c $(INC) $(LIB) -lusb-1.0 -o detach_usbhid
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm detach_usbhid

archive:
	tar f - --exclude=.git -C ../ -c bosto_2g | gzip -c9 > ../bosto_2g-`date +%Y%m%d`.tgz

install:
	cp ./bosto_2g.ko /lib/modules/$(shell uname -r)
	echo bosto_2g >> /etc/modules
	depmod
	cp ./load_bosto_2g.sh /usr/local/bin
	cp ./detach_usbhid /usr/local/bin
	cp ./load_bosto_2g.rules /etc/udev/rules.d
	/sbin/udevadm control --reload
	modprobe bosto_2g

uninstall:
	rm /usr/local/bin/load_bosto_2g.sh
	rm /usr/local/bin/detach_usbhid
	rm /etc/udev/rules.d/load_bosto_2g.rules
	/sbin/udevadm control --reload
	rm /lib/modules/$(shell uname -r)/bosto_2g.ko
	sed -i '/bosto_2g/d' /etc/modules
	depmod
	rmmod bosto_2g
