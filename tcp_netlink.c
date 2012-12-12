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
		printk(KERN_INFO "enic_netlink: creating netlink failed\n");
		return -1;
	} else {
		printk(KERN_INFO "enic_netlink: created netlink with id %d\n", NETLINK_TCP);
	}
	return 0;
}


void tcp_netlink_msg(struct sk_buff *recv_skb)
{
	int res;
	struct nlmsghdr *recv_nl_hdr, *send_nl_hdr;
	struct sk_buff *send_skb;
	struct case_data *data;
	struct cmd_alloc *cmdalloc;
	struct cmd_free *cmdfree;
	recv_nl_hdr = (struct nlmsghdr *) recv_skb->data;
	data = (struct case_data *) NLMSG_DATA(recv_nl_hdr);

	switch (data->cmd) {
	case 0:
		cmdalloc = NLMSG_DATA(recv_nl_hdr);
		printk(KERN_INFO"--alloc received from pid %d\n",recv_nl_hdr->nlmsg_pid);
		printk(KERN_INFO"wq=%d\nrq=%d\ncq=%d\nintr=%d\ndevice=%s",
		       cmdalloc->wq,
		       cmdalloc->rq,
		       cmdalloc->cq,
		       cmdalloc->intr,
		       cmdalloc->ifname);
		break;
	case 1:
		cmdfree = NLMSG_DATA(recv_nl_hdr);
		printk(KERN_INFO"--free received from pid %d\n for device %s",
		       recv_nl_hdr->nlmsg_pid,
		       cmdfree->ifname);
		break;
	case 2:
		printk("in case 2:\n");
		break;
	default:
		printk("in default case\n");

	}
}



void tcp_netlink_exit(void)
{
	netlink_kernel_release(nl_sock);

}
