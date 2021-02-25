obj-m += ebox3driver.o

KERNEL_ARCH = arm
KERNEL_DIR ?= /home/yuriy/Projects/eissbox3-kernel/KERNEL

all:
	make -C $(KERNEL_DIR) \
		ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(CC) \
		M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) \
		ARCH=$(KERNEL_ARCH) CROSS_COMPILE=$(CC) \
		M=$(PWD) clean
