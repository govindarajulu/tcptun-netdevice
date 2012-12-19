#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/kdev_t.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/cdev.h>
//#include <asm-generic/uaccess.h>

#include "char.h"

extern struct net_device *tcptun_netdev;
dev_t chrdev;
struct cdev *mycdev;
const struct file_operations fop = {
	.owner = THIS_MODULE,
	.open  = fops_myopen,
	.release = fops_myrelease,
	.read = fops_myread,
	.write = fops_mywrite
};


int fops_myopen(struct inode *myinode, struct file *myfile)
{
	return 0;
}

int fops_myrelease(struct inode *myinode, struct file *myfile)
{
	return 0;
}
ssize_t fops_myread(struct file *filep, char __user *buf,
		    size_t count, loff_t *f_pos)
{
	copy_to_user(buf, "linux", 6);
	*f_pos = *f_pos + 6;
	return 6;
}

ssize_t fops_mywrite(struct file *filep, char __user *buf,
		     size_t count, loff_t *f_pos)
{
	char *buffer;
	buffer = kmalloc(count+1, GFP_KERNEL);
	copy_from_user(buffer, buf, count);
	buffer[count] = '\0';
	printk(KERN_INFO "read- %s\n", buffer);
	*f_pos = *f_pos + count;
	kfree(buffer);
	return count;
}

int char_init(void)
{
	int err;
	chrdev = MKDEV(MAJORR, 0);
	printk(KERN_INFO "------------------------------\n");
	printk(KERN_INFO "inserting module\n");
	printk(KERN_INFO "trying registering chrdev:%s-MAJOR=%d,COUNT=%d\n",
	       tcptun_netdev->name, MAJOR(chrdev), COUNT);
	err = register_chrdev_region(chrdev, COUNT, tcptun_netdev->name);
	if (err < 0) {
		printk(KERN_INFO "chr register failed with register_chrdev_region, trying alloc_chrdev\n");
		goto try_alloc_chrdev_region;
	}
	printk(KERN_INFO "registration successfull\n");
	goto goto_cdev_init;
try_alloc_chrdev_region:
	err = alloc_chrdev_region(&chrdev, 0, COUNT, tcptun_netdev->name);
	if (err < 0)
		goto goto_error;
	printk(KERN_INFO "registed chrdev:%s-MAJOR=%d,COUNT=%d\n",
	       tcptun_netdev->name, MAJOR(chrdev), COUNT);
	goto goto_cdev_init;
goto_error:
	return -1;
goto_cdev_init:
	mycdev = cdev_alloc();
	mycdev->ops = &fop;
	mycdev->owner = THIS_MODULE;
	err = cdev_add(mycdev, chrdev, COUNT);
	if (err < 0) {
		printk(KERN_INFO "error in cdev_add\n");
		goto goto_unregister_chrdev_region;
	}
	printk(KERN_INFO "cdev_add successfull\n");
	goto goto_success;
goto_success:
	return 0;
goto_unregister_chrdev_region:
	unregister_chrdev_region(chrdev, COUNT);
	goto goto_error;

}
