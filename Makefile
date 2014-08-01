.PHONY: all clean archive

obj-m += bosto_2g.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

archive:
	tar f - --exclude=.git -C ../ -c bosto_2g | gzip -c9 > ../bosto_2g-`date +%Y%m%d`.tgz

install:
	cp ./bosto_2g.ko /lib/modules/$(shell uname -r)
	depmod
	cp ./insert_bosto_2g /usr/local/bin
	cp ./bosto_2g.rules /etc/udev/rules.d
	/sbin/udevadm control --reload

uninstall:
	rm /usr/local/bin/insert_bosto_2g
	rm /etc/udev/rules.d/bosto_2g.rules
	/sbin/udevadm control --reload
	rm /lib/modules/$(shell uname -r)/bosto_2g.ko
	depmod
