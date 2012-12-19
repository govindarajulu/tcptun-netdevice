#ifndef CHAR_H
#define CHAR_H

#define MAJORR 59
#define COUNT 2

#define MY_IOC_MAGIC 'k'
#define MY_IOC_PRINT1 _IO(MY_IOC_MAGIC, 0)
#define MY_IOC_PRINT_STRING _IOWR(MY_IOC_MAGIC, 1, char)


int fops_myopen(struct inode *myinode, struct file *myfile);
int fops_myrelease(struct inode *myinode, struct file *myfile);
ssize_t fops_myread(struct file *filep, char __user *buf, size_t count, loff_t *f_pos);
ssize_t fops_mywrite(struct file *filep, char __user *buf, size_t count, loff_t *f_pos);
int fops_myioctl (struct inode *myinode, struct file *filep, unsigned int mycmd, unsigned long myarg);
int char_init(void);


#endif // CHAR_H
