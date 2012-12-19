#ifndef TCPTUN_H
#define TCPTUN_H

#include <linux/netdevice.h>
#include <linux/semaphore.h>

#define TCPTUN_IFNAME "tcptunl%d"


struct tcptun_priv {
	int status;
	struct semaphore lock;
};

void tcptun_setup(struct net_device *dev);
int tcptun_open(struct net_device *dev);
int tcptun_stop(struct net_device *dev);
int tcptun_tx(struct sk_buff *skb, struct net_device *dev);
void tcptun_tx_timeout(struct net_device *dev);
void tcptun_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *drvinfo);
int tcptun_init(void);


#endif // TCPTUN_H
