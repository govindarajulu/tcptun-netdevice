#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include "tcp_netlink.h"

struct sock *nl_sock = NULL;

int tcp_netlink_init(void)
{
	nl_sock = netlink_kernel_create(&init_net, NETLINK_TCP, 0,
					tcp_netlink_msg, NULL,
					THIS_MODULE);
	if(!nl_sock) {
		printk(KERN_INFO "tcp_netlink: creating netlink failed\n");
		return -1;
	} else {
		printk(KERN_INFO "tcp_netlink: created netlink with id %d\n", NETLINK_TCP);
	}
	return 0;
}


void tcp_netlink_msg(struct sk_buff *recv_skb)
{
	int res;
	struct nlmsghdr *recv_nl_hdr, *send_nl_hdr;
	struct sk_buff *send_skb;
	struct case_data *data;
	recv_nl_hdr = (struct nlmsghdr *) recv_skb->data;
	data = (struct case_data *) NLMSG_DATA(recv_nl_hdr);

	switch (data->cmd) {
	case 0:
		printk(KERN_INFO"in case 0:\n");
		printk(KERN_INFO"received message from pid=%d, %s\n", recv_nl_hdr->nlmsg_pid, CASE_DATA_DATA(data));
		break;
	case 1:
		printk("in case 1:\n");
		struct sk_buff *skb;
		char buff[] = "hello userspace, this is kernel";
		skb = alloc_skb(NLMSG_SPACE(sizeof(buff)), GFP_KERNEL);
		if(!skb) {
			printk(KERN_INFO "alloc_skb failed\n");
			break;
		}
		skb_put(skb, NLMSG_SPACE(sizeof(buff)));
		send_nl_hdr = (struct nlmsghdr*) skb->data;
		send_nl_hdr->nlmsg_len = NLMSG_SPACE(sizeof(buff));
		send_nl_hdr->nlmsg_pid = 0;
		send_nl_hdr->nlmsg_flags = 0;

		memcpy(NLMSG_DATA(send_nl_hdr),buff,sizeof(buff));
		NETLINK_CB(skb).pid = recv_nl_hdr->nlmsg_pid;
		NETLINK_CB(skb).dst_group = 0;
		res = netlink_unicast(nl_sock, skb, NETLINK_CB(skb).pid, MSG_DONTWAIT);
		res = netlink_broadcast
		if(res < 0 )
			printk("netlink_unicast failed\n");
		break;
	case 2:
		printk("in case 2:\n");
		break;
	default:
		printk("in default case\n");

	}
}



void enic_netlink_exit(void)
{
	netlink_kernel_release(nl_sock);

}
