# Kernel Version Check
KVER  := $(shell uname -r | cut -c1-3 | sed 's/2\.[56]/2\.6/')
KVERSUB := $(shell uname -r | cut -c5-6)
KVER3 :=$(shell uname -r | cut -c1-1)
# Makefile for a basic kernel module - 2.4.x
KINC=
ifeq ($(KINC),)
KINC  := $(shell uname -r)
endif
CC    := gcc
MODCFLAGS := -DMODULE -D__KERNEL__ -DLINUX -DOLDKERNEL -O -I/usr/src/linux-$(KINC)/include

# Makefile for a basic kernel module - 2.6.x
obj-m := plcm_drv.o
KDIR  := /lib/modules/$(shell uname -r)/build
PWD   := $(shell pwd)

# MODULE -- This tells the header files to give the appropriate
# definitions for a kernel module.
#
# __KERNEL__ -- This tells the header files that this code will
# be run in kernel mode not as part of a user process.
#
# LINUX -- Technically speaking, this is not necessary. However,
# if you ever want to write a serious kernel module which will
# compile on more than one operating system, you'll be happy you
# did. This will allow you to do conditional compilation on the
# parts which are OS dependant.
#
default:
	gcc -O2 ppdev_test.c -o ppdev_test
	gcc -O2 plcm_test.c -o plcm_test
	gcc -O2 plcm_cursor_char.c -o plcm_cursor_char
	gcc -g info_disp.c menu_objs.c -o lcd-menu  

boot:
ifeq ($(KVER3),3)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif
ifeq ($(KVER),2.6)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
endif
ifeq ($(KVER),2.4)
	$(CC) $(MODCFLAGS) -c plcm_drv.c
endif

ifeq ($(wildcard lcd-menu),)
	gcc -O2 ppdev_test.c -o ppdev_test
	gcc -O2 plcm_test.c -o plcm_test
	gcc -O2 plcm_cursor_char.c -o plcm_cursor_char
	gcc -O2 info_disp.c menu_objs.c -o lcd-menu  
endif
	rmmod plcm_drv
ifeq ($(wildcard /dev/plcm_drv),) 
	mknod /dev/plcm_drv c 248 0
endif	
	insmod plcm_drv.ko	
	install -m 755 lcd-menu /usr/bin/.
clean:
	rm -f plcm_test
	rm -f plcm_cursor_char
	rm -f ppdev_test
	rm -f lcd-menu 
	
ifeq ($(KVER3),3)
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
ifeq ($(KVER),2.6)
	$(MAKE) -C $(KDIR) M=$(PWD) clean
endif
ifeq ($(KVER),2.4)
	rm -f *.o *.ko
endif
