
# include <sys/types.h>
# include <stdio.h>
# include <sys/socket.h>
# include <linux/netlink.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>
# include <getopt.h>
# define USERSPACE_PROGRAM
# include "tcp_netlink.h"
# define NETLINK_NITRO 17
# define MAX_PAYLOAD 500

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

int main(int argc, char **argv)
{
	int d_ipport=2551;
	char d_ipaddr[16];
	int server=0;
	struct sockaddr_nl s_nladdr, d_nladdr;
	int netlink_fd = socket(AF_NETLINK ,SOCK_RAW , NETLINK_NITRO );

	const char *short_opt="d:p:s";
	const struct option long_option[]={
		{"dest",1,NULL,'d'},
		{"port",1,NULL,'p'},
		{"serv",0,NULL,'s'}
	};
	int next_arg;

	do{
		next_arg=getopt_long(argc,argv,short_opt,long_option,NULL);

		switch(next_arg)
		{
		case 'd':
			strcpy(d_ipaddr,optarg);
			break;
		case 'p':
			d_ipport = atoi(optarg);
			break;
		case 's':
			server =1;
			break;
		case -1:
			break;
		case '?':
			break;
		}
	}while(next_arg!=-1);

	/* source address */
	memset(&s_nladdr, 0 ,sizeof(s_nladdr));
	s_nladdr.nl_family = AF_NETLINK ;
	s_nladdr.nl_pad = 0;
	s_nladdr.nl_pid = getpid();
	bind(netlink_fd, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));

	/* destination address */
	memset(&d_nladdr, 0, sizeof(d_nladdr));
	d_nladdr.nl_family = AF_NETLINK ;
	d_nladdr.nl_pad = 0;
	d_nladdr.nl_pid = 0; /* destined to kernel */

	//nlsend_msg(fd, &d_nladdr, data, strlen(data));
	//read_and_print(fd,&d_nladdr);

	close(netlink_fd);
	return (EXIT_SUCCESS);

}
