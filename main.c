#include <linux/kernel.h>
#include <linux/module.h>
#include "tcptun.h"

static __init int modinit(void)
{
	return 0;
}

static __exit void modexit(void)
{

}

module_init(modinit);
module_exit(modexit);
