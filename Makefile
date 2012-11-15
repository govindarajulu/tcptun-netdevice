obj-m := tcptun.o
tcptun-objs := main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules
