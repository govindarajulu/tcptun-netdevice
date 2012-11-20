#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/slab.h>
#include "tcpsock.h"

int sock_type = TCP_SERVER; /* module is server or client*/
int sock_port = TCP_PORT; /*tcp port*/
struct socket *sock, *client;
struct sockaddr_in *server, *cl_addr;
struct iovec *iov;
struct msghdr *sock_msg;
char *data;
int tcpsock_init(void)
{
	int err;

	iov = (struct iovec *) kmalloc(sizeof( struct iovec), GFP_KERNEL);
	sock_msg = (struct msghdr *) kmalloc(sizeof(struct msghdr), GFP_KERNEL);
	server = (struct sockaddr_in *) kmalloc(sizeof(struct sockaddr_in), GFP_KERNEL);
	//client = (struct socket *) kmalloc(sizeof(struct socket), GFP_KERNEL);
	cl_addr = (struct sockaddr_in *) kmalloc(sizeof(struct sockaddr_in), GFP_KERNEL);
	data = (char *) kmalloc(3, GFP_KERNEL);
	err = sock_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
	if (err) {
		printk(KERN_INFO "sock_create failed\n");
		return -1;
	}

	/* initialize server source address*/
	server->sin_family = AF_INET;
	server->sin_port = htons(TCP_PORT);
	server->sin_addr.s_addr = htonl(INADDR_ANY);

	err = kernel_bind(sock, (struct sockaddr *) server, sizeof(struct sockaddr_in));

	if (err) {
		printk(KERN_INFO "bind failed\n");
		return -1;
	}

	err = kernel_listen(sock, 1024);

	if (err) {
		printk(KERN_INFO "listen failed\n");
		return -1;
	}
	kernel_accept(sock,&client,0);

	return 0;
}
