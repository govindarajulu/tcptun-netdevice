obj-m := tcptunl.o
tcptunl-objs := tcptun.o tcp_netlink.o main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules
