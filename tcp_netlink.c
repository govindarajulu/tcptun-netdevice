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
	int res;
	struct nlmsghdr *nlhdr;
	nlhdr = (struct nlmsghdr *) recv_skb->data;
	pid = nlhdr->nlmsg_pid;
	printk(KERN_INFO"nlhdr->nlmsg_flags=%d\nnlhdr->nlmsg_len=%d\nnlhdr->nlmsg_seq=%d\nnlhdr->nlmsg_type=%d\n",
	       nlhdr->nlmsg_flags, nlhdr->nlmsg_len
	       , nlhdr->nlmsg_seq, nlhdr->nlmsg_type);
	skb_pull(recv_skb, NLMSG_LENGTH(0));
	recv_skb->dev = tcptun_netdev;
	recv_skb->csum = CHECKSUM_COMPLETE;
	printk(KERN_INFO"received %d bytes\n",recv_skb->len);
	recv_skb->protocol = eth_type_trans(recv_skb, tcptun_netdev);
	//atomic_inc(&recv_skb->users);
	//netif_rx(recv_skb);
	hexprint(recv_skb->data, recv_skb->len);
	tcptun_netdev->last_rx = jiffies;
}



void tcp_netlink_exit(void)
{
	netlink_kernel_release(nl_sock);

}
