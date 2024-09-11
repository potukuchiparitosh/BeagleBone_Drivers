#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/i2c.h>

/*Meta Information*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paritosh Potukuchi");
MODULE_DESCRIPTION("A device tree based driver for my I2C PCF8574");

static struct i2c_client *adc_client;

static int my_adc_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int my_adc_remove(struct i2c_client *client);

static struct of_device_id my_driver_ids[] = {
        {
                .compatible = "brightlight, myadc",
        }, { /*sentinel*/ }
};

MODULE_DEVICE_TABLE(of, my_driver_ids);

static struct i2c_device_id my_adc[] = {
        {"my_adc", 0},
        { },
};

MODULE_DEVICE_TABLE(i2c, my_adc);

static struct i2c_driver my_driver = {
        .probe = my_adc_probe,
        .remove = my_adc_remove,
        .id_table = my_adc,
        .driver = {
                .name = "my_adc",
                .of_match_table = my_driver_ids,
        },
};

static struct proc_dir_entry *proc_file;

/*
 *@brief Update timing between to ADC reads
 */

static ssize_t my_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
        u8 val = 0xff;
        i2c_smbus_write_byte(adc_client, val);
        return 1;
}

static ssize_t my_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
        u8 val;
        val = i2c_smbus_read_byte(adc_client);
        return 1;
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .write = my_write,
        .read = my_read,
};

static int my_adc_probe(struct i2c_client *client, const struct i2c_device_id *id) {
        printk("dt_i2c Now I am in the probe function!\n");

        if(client->addr != 0x10) {
                printk("dt_i2c - Wrong I2C address!\n");
                return -1;
        }
        adc_client = client;

        /*Creating procfs file*/
        proc_file = proc_create("my_adc", 0666, NULL, &fops);

        printk("Reached this part\n");
        if(proc_file == NULL) {
                printk("dt_i2c - Error creating /proc/myadc \n");
                return -ENOMEM;
        }
        return 0;
}

/*
 *@brief This function is called on unloading the driver
 */

static int my_adc_remove(struct i2c_client *client) {
        printk("dt_i2c Now I am in the remove function!\n");
        proc_remove(proc_file);
        return 0;
}

/*This will create the init and exit function automatically*/
module_i2c_driver(my_driver);






















