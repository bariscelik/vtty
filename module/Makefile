KVERSION := $(shell uname -r)

obj-m := vtty.o
vtty-objs := shatty.o chip.o

all:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

load:
	insmod ./vtty.ko

unload:
	rmmod ./vtty.ko