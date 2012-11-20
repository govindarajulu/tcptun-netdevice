#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/socket.h>
#include "tcptun.h"
#include "main.h"
#include "tcpsock.h"

static __init int modinit(void)
{
	int err;


	tcpsock_init();

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
	return 0;

goto_alloc_netdev_failed:
	return -1;
goto_register_netdev_failed:
	free_netdev(tcptun_netdev);
	return -1;
} /* end of static __init int modinit(void)*/


static __exit void modexit(void)
{
	kernel_sock_shutdown(sock,0);
	unregister_netdev(tcptun_netdev);
	free_netdev(tcptun_netdev);

}

module_init(modinit);
module_exit(modexit);
MODULE_LICENSE("GPL");
module_param(sock_type, int, S_IRUSR);
module_param(sock_port, int, S_IRUSR);
