#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/net.h>
#include "tcpsock.h"

int sock_type; /* module is server or client*/
struct socket *sock, *client;
struct sockaddr_in *server, *cl_addr;
struct iovec *iov;
struct msghdr *sock_msg;
int tcpsock_init()
{
	int err;
	err=sock_create(AF_INET, SOCK_STREAM, AF_INET, &sock);
	if (err) {
		printk(KERN_INFO "sock_create failed\n");
	}
	return 0;
}
