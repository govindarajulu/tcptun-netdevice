#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/kdev_t.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
//#include <asm-generic/uaccess.h>

#include "tcptun.h"
#include "char.h"

extern struct net_device *tcptun_netdev;
extern struct sk_buff *que[TCPTUN_QLEN];
extern int fetch;
extern int feed;
extern spinlock_t qlock;
extern wait_queue_head_t waitq;

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
	struct sk_buff *skb;
	int res;
	spin_lock(&qlock);
	while(fetch == feed) { /* TRUE = queue empty */
		spin_unlock(&qlock);
		res = wait_event_interruptible(waitq, fetch != feed);
		spin_lock(&qlock);
	}
	skb = que[fetch];
	if(count < skb->len) {
		printk("no enough space in userspace\n");
	}
	copy_to_user(buf, skb->data, count);
	*f_pos = *f_pos + skb->len;
	fetch = inc_fetchfeed(fetch);

	spin_unlock(&qlock);
	//copy_to_user(buf, "linux", 6);
	kfree_skb(skb);
	return skb->len;
}

ssize_t fops_mywrite(struct file *filep, char __user *buf,
		     size_t count, loff_t *f_pos)
{
	struct sk_buff *skb;
	char *buffer;
	buffer = kmalloc(count+1, GFP_KERNEL);
	copy_from_user(buffer, buf, count);
	buffer[count] = '\0';
	//printk(KERN_INFO "read- %s\n", buffer);
	*f_pos = *f_pos + count;
	kfree(buffer);


	skb = alloc_skb(count, GFP_KERNEL);
	if(!skb) {
		printk(KERN_INFO"skb alloc failed\n");
		return -1;
	}
	skb_put(skb, count);
	copy_from_user(skb->data, buf, count);
	skb->dev = tcptun_netdev;
	skb->csum = CHECKSUM_COMPLETE;
	skb->protocol = eth_type_trans(skb, tcptun_netdev);
	netif_rx(skb);

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
