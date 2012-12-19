#include <linux/etherdevice.h>
#include <linux/string.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include "tcptun.h"


extern int pid;
extern struct sock *nl_sock;
struct sk_buff *que[TCPTUN_QLEN];
int fetch = 0;
int feed = 0;
spinlock_t qlock;
wait_queue_head_t waitq;

/* tcptun_netdev: *tcptun_netdev is dynamically
 *by calling alloc_netdev
 */
struct net_device *tcptun_netdev;

/* tcptun_netdev_ops: used in net_device
 */
struct net_device_ops tcptun_netdev_ops;

/*
 *ethtool_ops for tcptunl driver
 */

int inc_fetchfeed(int i)
{
	i++;
	if(i>=TCPTUN_QLEN)
		i = 0;
	return i;

}

struct ethtool_ops tun_ethtool_ops = {
	.get_drvinfo = tcptun_get_drvinfo
};


void tcptun_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo)
{

	strcpy(drvinfo->driver,TCPTUN_IFNAME);
	strcpy(drvinfo->version,"0.0.1");
}


/* tcptun_setup: passed as argument in alloc_netdev
 */
void tcptun_setup(struct net_device *dev)
{
	ether_setup(dev);
	memset(&tcptun_netdev_ops, 0, sizeof(struct net_device_ops));
	dev->hard_header_len = ETH_HLEN;
	tcptun_netdev_ops.ndo_open = tcptun_open;
	tcptun_netdev_ops.ndo_stop = tcptun_stop;
	tcptun_netdev_ops.ndo_start_xmit = tcptun_tx;
	tcptun_netdev_ops.ndo_tx_timeout = tcptun_tx_timeout;
	dev->netdev_ops = &tcptun_netdev_ops;
	dev->ethtool_ops = &tun_ethtool_ops;
	{
		/* addigning the mac address
		 *mac_addr will be distroyed after this block
		 */
		char mac_addr[6];
		mac_addr[0] = (char)(0);
		mac_addr[1] = (char)(1);
		mac_addr[2] = (char)(2);
		mac_addr[3] = (char)(3);
		mac_addr[4] = (char)(4);
		mac_addr[5] = (char)(5);
		memcpy(dev->dev_addr, mac_addr, ETH_ALEN);
	}

}

int tcptun_open(struct net_device *dev)
{

	netif_start_queue(dev);
	return 0;
}

int tcptun_stop(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

int tcptun_tx(struct sk_buff *skb, struct net_device *dev)
{
	int res;
	return 0;
}

void tcptun_tx_timeout(struct net_device *dev)
{
	printk(KERN_INFO"tx_timeout called \n");
}

int err;
int fd;
struct socket *sock;

int tcptun_init(void)
{
	tcptun_netdev = alloc_netdev(sizeof(struct tcptun_priv),
				     TCPTUN_IFNAME, tcptun_setup);
	if (tcptun_netdev == NULL) {
		printk(KERN_INFO "alloc_netdev failed\n");
		goto goto_alloc_netdev_failed;
	}
	err = register_netdev(tcptun_netdev);
	if (err < 0) {
		printk(KERN_INFO "register_netdev failed\n");
		goto goto_register_netdev_failed;
	}
	if(err)
		goto err_netlink_failed;
	spin_lock_init(&qlock);
	init_waitqueue_head(&waitq);
	return 0; /*RETURN SUCCESS*/

err_netlink_failed:
	unregister_netdev(tcptun_netdev);
	free_netdev(tcptun_netdev);
	return -1;
goto_register_netdev_failed:
	free_netdev(tcptun_netdev);
	return -1;
goto_alloc_netdev_failed:
	return -1;
}
