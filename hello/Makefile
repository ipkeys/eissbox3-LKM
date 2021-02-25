obj-m+=hello.o

all:
	make ARCH="${ARCH}" CROSS_COMPILE="${CC}" -C /home/yuriy/Projects/eissbox3-kernel/KERNEL/ M=$(PWD) modules
clean:
	make ARCH="${ARCH}" CROSS_COMPILE="${CC}" -C /home/yuriy/Projects/eissbox3-kernel/KERNEL/ M=$(PWD) clean
