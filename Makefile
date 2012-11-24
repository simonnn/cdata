obj-m := cdata.o

KDIR := /usr/src/linux-source-2.6.24
PWD := $(shell pwd)
EXTRA_CFLAGS := -Wall

default:
	$(MAKE) $(EXTRA_CFLAGS) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -rf *.o *.ko .*cmd modules.* Module.* .tmp_versions *.mod.c
