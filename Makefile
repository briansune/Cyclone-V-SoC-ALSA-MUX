
obj-m := i2s_mic_amp.o

KDIR := /home/intel/rootfs/lib/modules/6.12.11-g2f0b0270dbab-dirty/build
CROSS_COMPILE := /usr/bin/arm-linux-gnueabihf-
ARCH := arm

all:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


