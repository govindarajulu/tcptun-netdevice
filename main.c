#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include <linux/ethtool.h>
#include "tcptun.h"
#include "main.h"
#include "tcp_netlink.h"

static __init int modinit(void)
{
	int err;
	int fd;
	struct socket *sock;


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
	err = tcp_netlink_init();
	if(err)
		goto err_netlink_failed;
	return 0; /*RETURN SUCCESS*/

err_netlink:
	unregister_netdev(tcptun_netdev);
	free_netdev(tcptun_netdev);
	return -1;
goto_register_netdev_failed:
	free_netdev(tcptun_netdev);
	return -1;
goto_alloc_netdev_failed:
	return -1;
} /* end of static __init int modinit(void)*/


static __exit void modexit(void)
{
	tcp_netlink_exit();
	unregister_netdev(tcptun_netdev);
	free_netdev(tcptun_netdev);

}

module_init(modinit);
module_exit(modexit);
MODULE_LICENSE("GPL");
