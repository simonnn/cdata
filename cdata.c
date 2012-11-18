#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <asm/io.h>
/* for copy_to_user()/copy_from_user() */
#include <asm/uaccess.h>
/* for ioctl */
#include "cdata_ioctl.h"

#ifdef CONFIG_SMP
#define __SMP__
#endif

#define	CDATA_MAJOR 121 
#define BUFFER_SIZE 1024


struct cdata_t {
	char data[BUFFER_SIZE];
	int index;
};


static int cdata_open(struct inode *inode, struct file *filp)
{
	int minor;
	struct cdata_t *cdata;

	minor = MINOR(inode->i_rdev);
	//printk(KERN_ALERT "cdata: open(minor=%d)\n", minor);
	//printk(KERN_ALERT "cdata: open(file struct address=%p)\n", filp);

	/* initial cdata structure */
	cdata = (struct cdata_t *)kmalloc(sizeof(struct cdata_t), GFP_KERNEL);
	memset(cdata->data, '\0', BUFFER_SIZE);
	cdata->index = 0;
	
	filp->private_data = (void *)cdata;
	return 0;
}

static int cdata_ioctl(struct inode *inode, struct file *filp, 
			unsigned int cmd, unsigned long arg)
{
	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;

	//printk(KERN_ALERT "cdata: ioctl(cmd=%d)\n", cmd);
	switch (cmd) {
	case IOCTL_EMPTY:
		//printk(KERN_ALERT "cdata: ioctl(IOCTL_EMPTY)\n");
		memset(cdata->data, '\0', BUFFER_SIZE);
		cdata->index = 0;
		break;
	case IOCTL_SYNC:
		//printk(KERN_ALERT "cdata: ioctl(IOCTL_SYNC)\n");
		printk(KERN_ALERT "cdata: ioctl(data:%s, index:%d)\n", cdata->data, cdata->index);
		break;
	default:
		return -ENOTTY;
	}

	return 0;
}

static ssize_t cdata_read(struct file *filp, char *buf, 
				size_t size, loff_t *off)
{
	printk(KERN_ALERT "cdata: read\n");
	return 0;
}

static ssize_t cdata_write(struct file *filp, const char *buf, 
				size_t size, loff_t *off)
{
	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	int i;

	if (size == 0) return 0;

	for (i = 0; i < size; i++) {
		if (cdata->index >= BUFFER_SIZE)
			return -EFAULT;

		if (copy_from_user(&cdata->data[cdata->index++], &buf[i], 1))
			return -EFAULT;
	}
	//printk(KERN_ALERT "cdata: write(%s)\n", buf);
	return 0;
}

static int cdata_release(struct inode *inode, struct file *filp)
{
	printk(KERN_ALERT "cdata: release\n");

	if (!filp->private_data)
		kfree(filp->private_data);
	return 0;
}

static struct file_operations cdata_fops = {	
	owner:		THIS_MODULE,
	open:		cdata_open,
	release:	cdata_release,
	ioctl:		cdata_ioctl,
	read:		cdata_read,
	write:		cdata_write,
};

int my_init_module(void)
{
	register_chrdev(CDATA_MAJOR, "cdata", &cdata_fops);
	printk(KERN_ALERT "cdata module: registered.\n");

	return 0;
}

void my_cleanup_module(void)
{
	unregister_chrdev(CDATA_MAJOR, "cdata");
	printk(KERN_ALERT "cdata module: unregisterd.\n");
}

module_init(my_init_module);
module_exit(my_cleanup_module);

MODULE_LICENSE("GPL");
