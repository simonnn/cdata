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
/* for wait queue */
#include <linux/sched.h>
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
	wait_queue_head_t	wait;
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
	init_waitqueue_head(&cdata->wait);
	
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

/* reentrnat issue due to
 * 1. first open()
 * 2. second fork()
 * 3. based on multi-processor(SMP)
 * 4. based on single-processor
 * 		- with kernel API that put current process to sleep
 * 		- means with process scheduling
 *
 * Becuase there's only one file struct.
 * 1. SMP must be with reentrant issue.
 * 2. For single processor cdata_write() for p2,
 *    it will run only when p1 sleep.
 *    That is 
 * 	    - (O) cdata_write() return
 * 	    - (X) p1 state is set to sleep by process scheduling before return */

/* Set process sleep API
 * 	- copy_from_user: sometimes user space memory is invalid
 * 					  due to swap to disk
 * 	- copy_to_user
 * 	- kmalloc: to retrieve lack memory page
 * 	- vmalloc: to retrieve lack memory page */
static ssize_t cdata_write(struct file *filp, const char *buf, 
				size_t size, loff_t *off)
{

	struct cdata_t *cdata = (struct cdata_t *)filp->private_data;
	DECLARE_WAITQUEUE(wait, current);
	int i;

	if (size == 0) return 0;
	
	
	/* reentrant protect start */
	//mutex_lock(...);
	
	for (i = 0; i < size; i++) {
		if (cdata->index >= BUFFER_SIZE) {
			add_wait_queue(&cdata->wait, &wait);
			current->state = TASK_UNINTERRUPTIBLE;
			schedule(); /* call scheduler to continue scheduling */

			/* NO need to change 'current->state = TASK_RUNNING'
			 * Because there must already be done somewhere */
			current->state = TASK_RUNNING;
			remove_wait_queue(&cdata->wait, &wait);
		}
		if (copy_from_user(&cdata->data[cdata->index++], &buf[i], 1))
			return -EFAULT;
	}
	//printk(KERN_ALERT "cdata: write(%s)\n", buf);
	
	//mutex_unlock(...);
	/* reentrant protect end */
	
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
