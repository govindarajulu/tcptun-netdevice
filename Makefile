obj-m := tcptunl.o
tcptunl-objs := tcptun.o char.o main.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=`pwd` modules
	gcc -g -lpthread tcptunuserland.c -o tcptunuserland
userland:
	gcc -g -lpthread tcptunuserland.c -o tcptunuserland
