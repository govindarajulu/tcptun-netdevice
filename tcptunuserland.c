
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
	int cmd[5];
	struct sockaddr_nl s_nladdr, d_nladdr;
	char data[]="abcdefghijklmnopqrstuvwxyz1234567890";
	char name[] = "hi kernel, this is userspace";
	int fd = socket(AF_NETLINK ,SOCK_RAW , NETLINK_NITRO );
	struct cmd_alloc *cmdalloc = NULL;
	struct cmd_free *cmdfree = NULL;
	const char *short_opt="afw:r:c:i:d:";
	const struct option long_option[]={
		{"alloc",0,NULL,'a'},
		{"free",0,NULL,'f'},
		{"wq",1,NULL,'w'},
		{"rq",1,NULL,'r'},
		{"cq",1,NULL,'c'},
		{"intr",1,NULL,'i'},
		{"device",1,NULL,'d'}
	};
	int next_arg;

	memset(cmd,0,sizeof(cmd));
/*

	do{
		next_arg=getopt_long(argc,argv,short_opt,long_option,NULL);

		switch(next_arg)
		{
		case 'a':
			cmd[CMD_ALLOC] = 1;
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			break;
		case 'f':
			cmd[CMD_FREE] = 1;
			if(cmdfree == NULL) {
				cmdfree = malloc(sizeof(struct cmd_free));
				memset(cmdfree,0,sizeof(struct cmd_free));
			}
			break;
		case 'w':
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			cmdalloc->wq = atoi(optarg);
			break;
		case 'r':
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			cmdalloc->rq = atoi(optarg);
			break;
		case 'c':
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			cmdalloc->cq = atoi(optarg);
			break;
		case 'i':
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			cmdalloc->intr = atoi(optarg);
			break;
		case 'd':
			if(cmd[CMD_ALLOC]) {
				if(cmdalloc == NULL ) {
					cmdalloc = malloc(sizeof(struct cmd_alloc));
					memset(cmdalloc,0,sizeof(struct cmd_alloc));
				}
				strcpy(cmdalloc->ifname,optarg);
				break;
			} else if(cmd[CMD_FREE]) {
				if(cmdfree == NULL) {
					cmdfree = malloc(sizeof(struct cmd_free));
					memset(cmdfree,0,sizeof(struct cmd_free));
				}
				strcpy(cmdfree->ifname,optarg);
				break;
			}
			if(cmdalloc == NULL ) {
				cmdalloc = malloc(sizeof(struct cmd_alloc));
				memset(cmdalloc,0,sizeof(struct cmd_alloc));
			}
			strcpy(cmdalloc->ifname,optarg);
			if(cmdfree == NULL) {
				cmdfree = malloc(sizeof(struct cmd_free));
				memset(cmdfree,0,sizeof(struct cmd_free));
			}
			strcpy(cmdfree->ifname,optarg);
			break;
		case -1:
			break;
		case '?':
			break;
		}
	}while(next_arg!=-1);
*/
	/* source address */
	memset(&s_nladdr, 0 ,sizeof(s_nladdr));
	s_nladdr.nl_family = AF_NETLINK ;
	s_nladdr.nl_pad = 0;
	s_nladdr.nl_pid = getpid();
	bind(fd, (struct sockaddr*)&s_nladdr, sizeof(s_nladdr));

	/* destination address */
	memset(&d_nladdr, 0, sizeof(d_nladdr));
	d_nladdr.nl_family = AF_NETLINK ;
	d_nladdr.nl_pad = 0;
	d_nladdr.nl_pid = 0; /* destined to kernel */

	nlsend_msg(fd, &d_nladdr, data, strlen(data));







//	/*case 0 */
//	data = case0();
//	nlsend_msg(fd, &d_nladdr, data, CASE_DATA_LENGTH(data));
//	free(data);
//
//	/*case 1 */
//	data = case1();
//	nlsend_msg(fd, &d_nladdr, data, CASE_DATA_LENGTH(data));
//	free(data);

	close(fd);
	return (EXIT_SUCCESS);

}
