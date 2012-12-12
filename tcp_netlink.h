#ifndef TCP_NETLINK_H
#define TCP_NETLINK_H



#include <linux/types.h>
#ifndef USERSPACE_PROGRAM
#include <linux/skbuff.h>
#endif

#define NETLINK_TCP 17
#ifndef USERSPACE_PROGRAM
int tcp_netlink_init(void);
void tcp_netlink_msg(struct sk_buff *recv_skb);
void tcp_netlink_exit(void);
#endif
struct case_data {
	int cmd;
	int len;
};

struct cmd_alloc {
	int cmd;
	char ifname[20];
	int wq;
	int rq;
	int cq;
	int intr;
};

struct cmd_free {
	int cmd;
	char ifname[20];
};

#define CASE_DATA_DATA(data) (void*) (((char*)data) + sizeof(struct case_data))
#define CASE_DATA_SPACE(len) (len + sizeof(struct case_data))
#define CASE_DATA_LENGTH(data) (sizeof(struct case_data) + data->len)

#define CMD_ALLOC 0
#define CMD_FREE 1


#endif // TCP_NETLINK_H
