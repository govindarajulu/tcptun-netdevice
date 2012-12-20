#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include <linux/ethtool.h>
#include <linux/cdev.h>
#include "tcptun.h"
#include "main.h"
#include "char.h"


extern struct net_device *tcptun_netdev;
extern dev_t chrdev;
extern struct cdev *mycdev;

static __init int modinit(void)
{
	int res;
	res = tcptun_init();
	if(res)
		return res;
	res = char_init();
	if(res) {
		unregister_netdev(tcptun_netdev);
		free_netdev(tcptun_netdev);
		return res;
	}
	return 0;
} /* end of static __init int modinit(void)*/


static __exit void modexit(void)
{
	cdev_del(mycdev);
	unregister_chrdev_region(chrdev, COUNT);
	unregister_netdev(tcptun_netdev);
	free_netdev(tcptun_netdev);

}

module_init(modinit);
module_exit(modexit);
MODULE_LICENSE("GPL");
