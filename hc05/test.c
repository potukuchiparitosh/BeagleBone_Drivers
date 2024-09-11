#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple loopback driver for an UART port");

/* Declate the probe and remove functions */
static int hc05_probe(struct serdev_device *serdev);
static void hc05_remove(struct serdev_device *serdev);

static struct of_device_id hc05_ids[] = {
	{
		.compatible = "hc05,hc05_test",
	}, { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, hc05_ids);

static struct serdev_device_driver hc05_driver = {
	.probe = hc05_probe,
	.remove = hc05_remove,
	.driver = {
		.name = "hc05",
		.of_match_table = hc05_ids,
	},
};

/**
 * @brief Callback is called whenever a character is received
 */
static int hc05_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size) {
	printk("hc05 - Received %ld bytes with \"%s\"\n", size, buffer);
        //return 	serdev_device_write_buf(serdev, buffer, size);
}

static const struct serdev_device_ops hc05_ops = {
	.receive_buf = hc05_recv,
};

/**
 * @brief This function is called on loading the driver 
 */
static int hc05_probe(struct serdev_device *serdev) {
	int status;
	printk("hc05 - Now I am in the probe function!\n");

	serdev_device_set_client_ops(serdev, &hc05_ops);
	status = serdev_device_open(serdev);
	if(status) {
		printk("hc05 - Error opening serial port!\n");
		return -status;
	}

	serdev_device_set_baudrate(serdev, 9600);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev, SERDEV_PARITY_NONE);

	//status = serdev_device_write_buf(serdev, "Type something: ", sizeof("Type something: "));
	//printk("hc05 - Wrote %d bytes.\n", status);
     printk("Ended probe function\n");

	return 0;
}

/**
 * @brief This function is called on unloading the driver 
 */
static void hc05_remove(struct serdev_device *serdev) {
	printk("hc05 - Now I am in the remove function\n");
	serdev_device_close(serdev);
}

/**
 * @brief This function is called, when the module is loaded into the kernel
 */
static int __init my_init(void) {
	printk("hc05 - Loading the driver...\n");
	if(serdev_device_driver_register(&hc05_driver)) {
		printk("hc05 - Error! Could not load driver\n");
		return -1;
	}
	return 0;
}

/**
 * @brief This function is called, when the module is removed from the kernel
 */
static void __exit my_exit(void) {
	printk("hc05 - Unload driver");
	serdev_device_driver_unregister(&hc05_driver);
}

module_init(my_init);
module_exit(my_exit);


