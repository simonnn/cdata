obj-m := cdata.o

KDIR := /usr/src/linux-source-2.6.24
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .*cmd modules.* Module.* .tmp_versions *.mod.c
