#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>

static int device_open(struct inode *inode, struct file* instance)
{
	printk("The device_open method has been invoked\n");
	return 0;
}
static int device_close(struct inode *inode, struct file *instance)
{
	printk("The device_close method has been invoked\n");
	return 0;
}

struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
};
#define MYMAJOR 160
static int __init Init_module(void)
{
	int ret;
	printk("Init module\n");
	ret = register_chrdev(MYMAJOR,"my_chr_dev", &my_fops);
	if(ret == 0) {
		printk("Found the major number requested\n");
	} else if(ret > 0) {
		printk("Found the major number requested by with a twist. major is %d and minor is %d\n",MYMAJOR>>20, MYMAJOR&0xfffff);
	} else {
		printk("Was not able to assign the major number\n");
		return -1;
	}
	return 0;
}
static void __exit Exit_module(void)
{
	unregister_chrdev(MYMAJOR,"my_chr_dev");
	printk("Exit module\n");
}
module_init(Init_module);
module_exit(Exit_module);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Johannes 4 GNU/Linux");
MODULE_AUTHOR("Paritosh Potukuchi");
