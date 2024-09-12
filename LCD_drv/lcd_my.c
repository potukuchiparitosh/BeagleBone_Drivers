#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include<linux/gpio.h>
#include<linux/version.h>
#include<linux/delay.h>


/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("Registers a device nr. and implement some callback functions");

/* Variables for device and device class */
static dev_t my_device_nr;
static struct class *my_class;
static struct cdev my_device;

#define DRIVER_NAME "lcd_driver"
#define DRIVER_CLASS "LCDModuleClass"

/*LCD char buffer*/
static char lcd_buffer[17];

/* Pinout for LCD display */

#if 0
#define ENABLE 3; /*Enable Pin */
#define REGISTER_SELECT	2; /*Register select Pin*/
#define DATA0	4; /*Data Pin 0*/
#define DATA1	17; /*Data Pin 1*/
#define DATA2	17; /*Data Pin 2*/
#define DATA3	17; /*Data Pin 3*/
#define DATA4	17; /*Data Pin 4*/
#define DATA5	17; /*Data Pin 5*/
#define DATA6	17; /*Data Pin 6*/
#define DATA7	17; /*Data Pin 7*/
#endif
int gpios[] = {
	60, /*Enable Pin */
	30, /*Register select Pin*/
	66, /*Data Pin 0*/
	67, /*Data Pin 1*/
	69, /*Data Pin 2*/
	68, /*Data Pin 3*/
	45, /*Data Pin 4*/
	44, /*Data Pin 5*/
	23, /*Data Pin 6*/
	26 /*Data Pin 7*/
};
#define ENABLE_PIN gpios[0]
#define REGISTER_SELECT gpios[1]

/*
 *@brief generates a pulse on the enable signal
 */
static void lcd_enable(void) {
	gpio_set_value(ENABLE_PIN, 1);
	msleep(5);
	gpio_set_value(ENABLE_PIN, 0);
}
/*
 * @brief set the 8 bit data bus
 * @param data : Data to set
 */

void lcd_send_byte(char data) {
	int i;
	for(i=0;i<8;i++) {
		gpio_set_value(gpios[i+2],((data & (1<<i)) >> i));
	}
	lcd_enable();
	msleep(5);
}

/*
 *@brief send a command to the LCD
 *@param data : command to send
 *
 */
void lcd_command(uint8_t data) {
	gpio_set_value(REGISTER_SELECT,0); /*RS to command*/
	lcd_send_byte(data);
}

void lcd_data(uint8_t data) {
	gpio_set_value(REGISTER_SELECT, 1);  /*RS to data*/
	lcd_send_byte(data);
}



/**
 * @brief Read data out of the buffer
 */
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta;
	char tmp[3] = " \n";

	printk("DRIVER_READ triggered\n");
	/* Get amount of data to copy */
	to_copy = min(count, sizeof(lcd_buffer));


	/* Copy data to user */
	not_copied = copy_to_user(user_buffer, &tmp, to_copy);

	/* Calculate data */
	delta = to_copy - not_copied;

	return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
	int to_copy, not_copied, delta,i;
	printk("DRIVER_WRITE triggered\n");

	/* Get amount of data to copy */
	to_copy = min(count, sizeof(lcd_buffer));


	/* Copy data to user */
	not_copied = copy_from_user(lcd_buffer, user_buffer, to_copy);



	/* Calculate data */
	delta = to_copy - not_copied;

	lcd_command(0x1);

	for(i = 0;i<to_copy;i++) {
		lcd_data(lcd_buffer[i]);
	}


	return delta;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_open(struct inode *device_file, struct file *instance) {
	printk("dev_nr - open was called!\n");
	return 0;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int driver_close(struct inode *device_file, struct file *instance) {
	printk("dev_nr - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init ModuleInit(void) {
	int retval,i;
	printk("Hello, Kernel!\n");
	char *names[] = {"ENABLE", "REGISTER_SELECT", "DATA0", "DATA1", "DATA2", "DATA3", "DATA4", "DATA5", "DATA6","DATA7"};

	/* Allocate a device nr */
	if( alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
		printk("Device Nr. could not be allocated!\n");
		return -1;
	}
	printk("read_write - Device Nr. Major: %d, Minor: %d was registered!\n", my_device_nr >> 20, my_device_nr & 0xfffff);

	/* Create device class */
	if((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
		printk("Device class can not be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
		printk("Can not create device file!\n");
		goto FileError;
	}

	/* Initialize device file */
	cdev_init(&my_device, &fops);

	/* Regisering device to kernel */
	if(cdev_add(&my_device, my_device_nr, 1) == -1) {
		printk("Registering of device to kernel failed!\n");
		goto AddError;
	}
	/*Initialize GPIOS*/
	printk("lcd -driver Init GPIOS\n");
	for ( i = 0;i < 10;i++) {
		if(gpio_request(gpios[i],names[i])) {
			printk("Cannot initialize gpio\n");
			goto GpioInitError;
		}

	}

	printk("lcd-driver GPIO Init\n");
	for( i = 0 ; i < 10;i++) {
		if(gpio_direction_output(gpios[i], 0)) {
			printk("lcd-driver Error setting GPIO %d\n to output\n",gpios[i]);
			goto GpioDirectionError;
		}
	}

	/*Init the display*/
	lcd_command(0x30); /*Set the display for 8 bit data interface*/

	lcd_command(0xf); /*Turn display on, turn cursor on, set cursor blinking*/

	lcd_command(0x1);

	char text[] = "Hello World!\n";
	for(i = 0;i<sizeof(text);i++) {
		lcd_data(text[i]);
	}


	return 0;
GpioDirectionError:
	i = 9;
GpioInitError:
	i--;
	for(;i>=0;i--) {
		gpio_free(gpios[i]);
	}	
AddError:
	device_destroy(my_class, my_device_nr);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_device_nr, 1);
	return -1;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit ModuleExit(void) {
	//gpio_set_value(GPIO_W, 0);
	int i;
	lcd_command(0x1);
	for ( i = 0;i< 10; i++) {
		gpio_free(gpios[i]);
	}
	cdev_del(&my_device);
	device_destroy(my_class, my_device_nr);
	class_destroy(my_class);
	unregister_chrdev_region(my_device_nr, 1);
	printk("Goodbye, Kernel\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);


