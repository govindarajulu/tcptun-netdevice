#ifndef TCPTUN_H
#define TCPTUN_H

#include <linux/netdevice.h>
#include <linux/semaphore.h>

#define TCPTUN_IFNAME "tcptap%d"

struct tcptun_priv {
	int status;
	struct semaphore lock;
};

#endif // TCPTUN_H
