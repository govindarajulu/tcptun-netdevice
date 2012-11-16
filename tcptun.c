#include <linux/etherdevice.h>
#include <linux/string.h>
#include "tcptun.h"

struct net_device *tcptun_netdev;

void tcptun_setup(struct net_device *dev)
{
	ether_setup(dev);
	dev->open = tcptun_open;
	dev->release = tcptun_release;
}

int tcptun_open(struct net_device *dev)
{
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
	netif_start_queue(dev);
	return 0;
}

int tcptun_release(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}
