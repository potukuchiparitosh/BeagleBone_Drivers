#include<linux/init.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/cdev.h>
/*
 * Buffer for data
 * 
 */
static char buffer[255];
static int buffer_pointer;

/*
 * Variables for device and device class
 */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

#define DRIVER_NAME "dummydriver"
#define DRIVER_CLASS "MyModuleClass"

/*
 *@brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *f, char __user *user_buffer, size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;
	/*Get amount of data to copy*/
	to_copy = min(count, buffer_pointer);

	/*Copy data to user*/
	not_copied = copy_to_user(user_buffer, buffer, to_copy);

	/* Calculate data*/
	delta = to_copy - not_copied;
	return delta;

}
/*
 *@brief Write data to buffer
 */
static ssize_t driver_write(struct file *f, const char __user *user_buffer,size_t count, loff_t *offs)
{
	int to_copy, not_copied, delta;

	to_copy = min(count, sizeof(buffer));
	not_copied = copy_from_user(buffer, user_buffer, to_copy);
	buffer_pointer = to_copy;

	delta = to_copy - not_copied;
	return delta;
}
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

static struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = device_open,
	.release = device_close,
	.read = driver_read,
	.write = driver_write,
};
#define MYMAJOR 160
static int __init Init_module(void)
{
	int ret;
	printk("Init module\n");
	/* Allocate a device number */
	if(alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
		printk("Could not be allocated!\n");
		return -1;
	}
	printk("read_write - Device number Major %d Minor %d was registered \n",my_device_nr , my_device_nr & 0xfffff);

	/* Create device class*/
	if((my_class = class_create(THIS_MODULE,DRIVER_CLASS)) == NULL) {
		printk("Cannot create class\n");
		goto ClassError;
	}
	/*Create device file*/
	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
		printk("Cannot create device file\n");
		goto FileError;
	}
	/* Initialize device file*/
	cdev_init(&my_device, &my_fops);

	/*Registering device to kernel*/
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("Registering of device to kernel failed\n");
		goto AddError;
	}
AddError:
	device_destroy(my_class, my_device_nr);

FileError:
	class_destroy(my_class);
ClassError : 
	unregister_chrdev(my_device_nr, DRIVER_NAME);
	return 0;
}
static void __exit Exit_module(void)
{
	cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev(MYMAJOR,"my_chr_dev");
	printk("Exit module\n");
}
module_init(Init_module);
module_exit(Exit_module);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Johannes 4 GNU/Linux");
MODULE_AUTHOR("Paritosh Potukuchi");
