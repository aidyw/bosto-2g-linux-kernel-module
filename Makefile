.PHONY: all clean archive

obj-m += bosto_2g.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
 
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

archive:
	tar f - --exclude=.git -C ../ -c bosto_2g | gzip -c9 > ../bosto_2g-`date +%Y%m%d`.tgz

install:
	cp bosto_2g.ko /lib/modules/$(shell uname -r)
	depmod