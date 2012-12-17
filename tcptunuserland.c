#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <error.h>
#include <sys/types.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#define USERSPACE_PROGRAM
#include "tcp_netlink.h"
#define NETLINK_NITRO 17


struct sockaddr_nl s_nladdr, d_nladdr;
struct sockaddr_in s_sockaddr, c_sockaddr;
socklen_t socklen;
pthread_t recv_tcpthread, recv_nlthread;

int res;
int d_ipport = 2551;
char d_ipaddr[16] = "127.0.0.1";
int server = 0;

int netlink_fd, tcpsock_fd, tcpsend_fd;

int readn(int fd, char *data, u_int16_t len);
void read_and_print(int fd, struct sockaddr_nl *sock);
int nlsend_msg(int fd, struct sockaddr_nl *d_nladdr, void *data, int len);
void *read_from_tcpsock(void * nothing);


void read_and_print(int fd, struct sockaddr_nl *sock)
{

	struct nlmsghdr *nl_hdr;
	struct msghdr msg_hdr;
	struct iovec iov;

	nl_hdr = malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nl_hdr, 0, NLMSG_SPACE(MAX_PAYLOAD));
	iov.iov_base = nl_hdr;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

	msg_hdr.msg_name = sock;
	msg_hdr.msg_namelen = sizeof(struct sockaddr_nl);
	msg_hdr.msg_iov = &iov;
	msg_hdr.msg_iovlen = 1;
	recvmsg(fd, &msg_hdr, 0);
	printf("received from kernel = %s", NLMSG_DATA(nl_hdr));
}

int nlsend_msg(int fd, struct sockaddr_nl *d_nladdr, void *data, int len)
{
	struct msghdr msg ;
	struct nlmsghdr *nlh = NULL ;
	struct iovec iov;

	/* Fill the netlink message header */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(len));
	memset(nlh , 0, NLMSG_SPACE(len));
	printf("%d-%d\n",len,NLMSG_SPACE(len));
	memcpy(NLMSG_DATA(nlh),data,len);
	nlh->nlmsg_len = NLMSG_SPACE(len);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 1;
	nlh->nlmsg_type = 0;

	/*iov structure */

	iov.iov_base = (void *)nlh;
	iov.iov_len = NLMSG_SPACE(len);

	/* msg */
	memset(&msg,0,sizeof(msg));
	msg.msg_name = (void *) d_nladdr ;
	msg.msg_namelen = sizeof(struct sockaddr_nl);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	sendmsg(fd, &msg, 0);
	free(nlh);
}

void *read_from_tcpsock(void * nothing)
{
	int res;
	char *data;
	u_int16_t len;
	data = malloc(MAX_PAYLOAD);
	while(1) {
		res = read(tcpsend_fd, &len, sizeof(u_int16_t));
		if(res < 0) {
			perror("read");
			pthread_exit(NULL);
		}
		printf("len = %d",len);
		len = ntohs(len);
		printf("--len = %d\n",len);
		if(len > MAX_PAYLOAD) {
			printf("length exceeds MAX_PAYLOAD");
			exit(EXIT_FAILURE);
		}
		res = readn(tcpsend_fd, data, len);
		if(res != len) {
			perror("readn");
			pthread_exit(NULL);
		}
		printf("received %s",data);
		res = nlsend_msg(netlink_fd, &d_nladdr, data, len);
		if(res < 0 ) {
			perror("write in netlink_fd in read_from_tcpsock");
			pthread_exit(NULL);
		}
	}
}

int readn(int fd, char *data, u_int16_t len)
{
	u_int16_t readlen = 0;
	int res;
	while(readlen < len) {
		res = read(fd, (void *)(data + readlen), len - readlen);
		if(res == -1) {
			perror("readn");
			return EXIT_FAILURE;
		}
		readlen = readlen + res;
	}
	return readlen;
}




int main(int argc, char **argv)
{
	const char *short_opt = "d:p:s";
	const struct option long_option[] = {
		{"dest", 1, NULL, 'd'},
		{"port", 1, NULL, 'p'},
		{"serv", 0, NULL, 's'}
	};
	int next_arg;

	do{
		next_arg = getopt_long(argc,argv,short_opt,long_option,NULL);

		switch(next_arg)
		{
		case 'd':
			strcpy(d_ipaddr,optarg);
			break;
		case 'p':
			d_ipport = atoi(optarg);
			break;
		case 's':
			server = 1;
			break;
		case -1:
			break;
		case '?':
			break;
		}
	}while(next_arg != -1);

	netlink_fd = socket(AF_NETLINK ,SOCK_RAW , NETLINK_NITRO );
	if (netlink_fd < 0) {
		printf("error in socket\n");
		perror("socket: netlink_fd");
		exit(EXIT_FAILURE);
	}


	/* source address */
	memset(&s_nladdr, 0 ,sizeof(s_nladdr));
	s_nladdr.nl_family = AF_NETLINK ;
	s_nladdr.nl_pad = 0;
	s_nladdr.nl_pid = getpid();
	res = bind(netlink_fd, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));

	if (res < 0 ) {
		printf("error in bind\n");
		perror("bind netlink_fd");
		exit(EXIT_FAILURE);
	}

	/* destination address */
	memset(&d_nladdr, 0, sizeof(d_nladdr));
	d_nladdr.nl_family = AF_NETLINK ;
	d_nladdr.nl_pad = 0;
	d_nladdr.nl_pid = 0; /* destined to kernel */

	tcpsock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(tcpsock_fd < 0 ) {
		printf("error in socket\n");
		perror("socket tcpsock_fd");
		exit(EXIT_FAILURE);
	}
	bzero(&s_sockaddr, sizeof(s_sockaddr));
	bzero(&c_sockaddr, sizeof(c_sockaddr));

	s_sockaddr.sin_family = AF_INET;
	s_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_sockaddr.sin_port = htons(d_ipport);

	c_sockaddr.sin_family = AF_INET;
	c_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	c_sockaddr.sin_port = htons(INADDR_ANY);


	if(server) {

		res = bind(tcpsock_fd, (struct sockaddr *) &s_sockaddr, sizeof(s_sockaddr));
		if (res < 0) {
			printf("error in bind\n");
			perror("bind s_sockaddr");
			exit(EXIT_FAILURE);
		}
		res = listen(tcpsock_fd, 1024);
		if(res < 0) {
			perror("listen");
			exit(EXIT_FAILURE);
		}
		socklen = sizeof(c_sockaddr);
		tcpsend_fd = accept(tcpsock_fd, (struct sockaddr *)&c_sockaddr, &socklen);
		if(tcpsend_fd < 0) {
			printf("error in accept\n");
			perror("accept");
			exit(EXIT_FAILURE);
		}
	} else {
		//struct hostent *ipaddr;
		//ipaddr = gethostbyname(d_ipaddr);
		//if(ipaddr == NULL) {
		//	printf("error in gethostbyname\n");
		//	perror("gethostbyname:");
		//	exit(EXIT_FAILURE);
		//}

		//bcopy(ipaddr->h_addr,&c_sockaddr.sin_addr.s_addr, ipaddr->h_length);
		c_sockaddr.sin_addr.s_addr = inet_addr(d_ipaddr);
		c_sockaddr.sin_port = htons(d_ipport);
		res = connect(tcpsock_fd, (struct sockaddr *)&c_sockaddr, sizeof(c_sockaddr));
		if(res < 0) {
			printf("error in connect\n");
			perror("connect:");
			exit(EXIT_FAILURE);
		}
		tcpsend_fd = tcpsock_fd;
	}

	//nlsend_msg(fd, &d_nladdr, data, strlen(data));
	//read_and_print(fd,&d_nladdr);
	res = pthread_create(&recv_tcpthread, NULL, read_from_tcpsock, NULL);
	pthread_join(recv_tcpthread, NULL);

	if(server)
		close(tcpsend_fd);
	close(tcpsock_fd);
	close(netlink_fd);
	return (EXIT_SUCCESS);

}
