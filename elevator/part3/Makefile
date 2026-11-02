# Builds elevator.ko in this directory from the sources in src/.
# The path to this directory must not contain spaces: kbuild cannot handle them.

obj-m := elevator.o
elevator-y := src/elevator_main.o src/elevator_sched.o src/elevator_proc.o
ccflags-y := -Wno-error

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

.PHONY: all clean
