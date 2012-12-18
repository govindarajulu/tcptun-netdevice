#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <net/net_namespace.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include "tcp_netlink.h"

struct sock *nl_sock = NULL;
int pid = 0;
extern struct net_device *tcptun_netdev;


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


void hexprint(char *data, int len)
{
	int i;
	printk(KERN_INFO"\n------------------\n");
	for(i = 0; i < len; i++) {
		printk(KERN_INFO"%x",data[i]);
	}
	printk(KERN_INFO"\n------------------\n");

}

void tcp_netlink_msg(struct sk_buff *recv_skb)
{
	int res, len;
	struct nlmsghdr *nlhdr;
	struct sk_buff *skb;

	nlhdr = (struct nlmsghdr *) recv_skb->data;
	pid = nlhdr->nlmsg_pid;
	len = nlhdr->nlmsg_len - NLMSG_LENGTH(0);
	hexprint(NLMSG_DATA(nlhdr), len);
	skb = alloc_skb(len,GFP_KERNEL);
	if(!skb) {
		printk(KERN_INFO"skb is NULL\n");
		return;
	}
	skb->dev = tcptun_netdev;
	skb->csum = CHECKSUM_COMPLETE;
	memcpy(skb->data, NLMSG_DATA(nlmsg), len);
	skb->len = len;
	skb->protocol = eth_type_trans(skb, tcptun_netdev);
	printk(KERN_INFO"received %d bytes\n",recv_skb->len);
	//netif_rx(skb);
	tcptun_netdev->last_rx = jiffies;
}



void tcp_netlink_exit(void)
{
	netlink_kernel_release(nl_sock);

}
