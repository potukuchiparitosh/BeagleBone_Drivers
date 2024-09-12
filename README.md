# BeagleBone_Drivers
This repo contains beaglebone based drivers for sensors using the SPI, I2C, UART, IIO and char driver subsystems of the linux kernel. These were all tested on the beaglebone black

# Beaglebone : 

My board of choice is the beaglebone black. I explored sensors, GPIO expanders and bluetooth modules and in the process got familiar with writing driver for SPI, I2C, I2C and GPIO devices.

I Worked with **Bosch’s BMP280** temperature and pressure sensor. Programmed an I2C and SPI based driver separately for the sensor. This driver had to loaded using the Device Tree overlays that beaglebone black provides. This work gave an insight into the way beaglebone identifies its hardware. The subsystem used for user interaction was the Industrial IO(iio) subsystem. I used this instead of the normal file_ops IO since this is more standardized for use in professional applications.

# SPI based driver for BMP280:
Two approaches were used to write this driver.
 
    • The first approach was to use the standard spi_write and spi_read functions that the SPI subsystem provides. This abstracts a lot underlying instructions that would otherwise increase the size of the program. 
    • The second approach is to understand how the protocol works and when the clock like needs to be pulled up and down. In this approach we form a message using the SPI subsystem’s function like spi_message_init, spi_message_add_tail and spi_sync. This approach gives more flexibility, but also means that more responsibility lies with the programmer.
Lets go over both the approaches in detail.









**Low Level SPI based driver for BMP280 without using much abstractions**:

This driver was written by manually forming the messages that needed to be transferred for the read operation from the device to happen. The central structure of the program is the bmp280_state structure, which looks like this:

      struct bmp280_state {
            struct spi_device *spi;
            struct spi_transfer ring_xfer[18];
            struct spi_message ring_msg;

            u8 rx_buf[18]; 
            u8 tx_buf[18]; 

      };

The spi_device is the spi subsystem specific device structure which contains a pointer to to struct device as well as many other parameters which help the SPI subsystem setup and use the device.
The spi_transfer structure looks like this:

      struct spi_transfer {
          const void * tx_buf;
          void * rx_buf;
          unsigned len;
          dma_addr_t tx_dma;
          dma_addr_t rx_dma;
          struct sg_table tx_sg;
          struct sg_table rx_sg;
          unsigned cs_change:1;
          unsigned tx_nbits:3;
          unsigned rx_nbits:3;
          #define SPI_NBITS_SINGLE        0x01
          #define SPI_NBITS_DUAL          0x02
          #define SPI_NBITS_QUAD          0x04
          u8 bits_per_word;
          u16 delay_usecs;
          u32 speed_hz;
          struct list_head transfer_list;
      }; 
      
The important fields in this are **tx_buf**, **rx_buf**, **cs_change** and **len**.
**tx_buf** is for the register number that needs to be sent to the device. This is the register whos value needs to be read, and this value would be received in the spi_transfer structure following this structure, in the spi_message. 
Coming to spi_message, this is just a list of spi_transfers added one after another in the order of execution. Generally it looks like TX, RX, TX, RX. This means that the first spi_transfer is for sending which register to send, the second spi_transfer is for receiving the value read from the device, and this would be filled in the rx_buf part of this spi_transfer. Similarly all the instructions are executed in the spi_message. The adding of spi_transfer to the spi_message is done using the function spi_message_add_tail. 
After all the spi_transfer s are added to the spi_message, we need to call the function spi_sync.
This function is performs a blocking and synchronous SPI data transfer. Blocking means that this is not interruptible and has no timeout. Low overhead controller drivers may DMA directly into and out of the message buffers.

**FYI**: The function for performing spi_sync just for spi_transfer instead of spi_message is spi_sync_transfer.

The driver for BMP280 uses the IIO subsystem and the read call is implemented inside the read_raw callback function provided in the structu iio_info bmp280_info structure, which is registered with the driver during the probe function.

Inside the **read_raw** function there is a function argument called channel. This is used to expose different files to read from in the IIO subsystem inside /sys/class… . This means that if there are two channels declared using the iio_chan_spec structure(that contains the names pressure and temp, as below):

        static struct iio_chan_spec bmp280_channels[] = { 
                {   
                        .type = IIO_PRESSURE,
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                },  
                {
                        .type = IIO_TEMP,
                        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
                },  
        };

This is given as input into the iio subsystem during the probe function registration when the iio subsystem is also being initialized using the structure iio_dev.

Example from code:

        struct iio_dev *iio_dev;
        iio_dev = devm_iio_device_alloc(&spi->dev, sizeof(struct bmp280_state));
        iio_dev->info = &bmp280_info;
        iio_dev->channels = chip_info->channels;
        iio_dev->num_channels = chip_info->num_channels;
        iio_dev->name = "bmp280";
        iio_dev->modes = INDIO_DIRECT_MODE;
        iio_dev->dev.parent = &spi->dev;
        iio_dev->dev.of_node = spi→dev.of_node;

The calculations for each temperature and pressure are done by looking through the data sheet and using the formulas given to calculate the values using the registers read through the previoud operations involving sending spi_transfer and spi_message.

The device tree is used by first disabling the pins used to connect the device to the Beaglebone black. This disabling is done inside the main device tree that is used by beaglebone when it is booted. The other option is to overlay the device tree with the SPI pins being used for BMP280 and mark them as disabled. This method is much easier and should be used in production.

# High Level SPI based driver for BMP280 with abstractions:

This driver is much simpler than the one explained above, since there is not concept of spi_transfer, spi_message. Every value that needs to be written to the device is written using the function spi_write. An example of how this function is used is shown below:




        u8 buffer[2];

        buffer[0] = 0x75;
        buffer[1] = 0xA0;

        ret = spi_write(bmp280->spi, buffer, 2);  

The above code snippet is the only time a write function is actually used in the code since this sensor needs to be read from. Therefore the function used after this is:
spi_w8r8. This writes 1 byte and reads 1 byte. It is used to specify the register to be read from and the value of the register is read and given out as the return value of the function. An example from the code is:

                        buf[0] = spi_w8r8(bmp280->spi, 0xF7);                                                                                                                           
                        buf[1] = spi_w8r8(bmp280->spi, 0xF8);
                        buf[2] = spi_w8r8(bmp280->spi, 0xF9);

The device tree used for this driver is also the same as the Low level SPI driver.

# PCF8574 I2C to GPIO expander: 

I also worked with PCF8574, which is an I2C to GPIO expander.  This meant that access to the expander was performed using the kernel’s I2C subsystem. The user interface was provided using the GPIO subsystem, which involves registering a gpio_chip structure with the device, and in the process associating the device specific structure with it, in this case the private structure for pcf8574 that I defined.

**Structure of the driver program for PCF8574 I2C to GPIO**:
        struct pcf8574 {
             struct i2c_client *client;
             struct gpio_chip gpio_chip; 
             uint8_t gpio_state; 
        };

This is the main structure around which everything revolves inside this driver. The first field is the i2c_client itself. The second field contains information about the GPIO subsystem and lets the program use the GPIO subsystem to be used. This gpio_chip structure needs to filled as follows(in the probe function):

         //Initialize the GPIO chip
         pcf8574->gpio_chip.label = client->name;
         pcf8574->gpio_chip.parent = &client->dev;
         pcf8574->gpio_chip.owner = THIS_MODULE;
         pcf8574->gpio_chip.base = -1;
         pcf8574->gpio_chip.ngpio = 8;
         pcf8574->gpio_chip.can_sleep = true;
         pcf8574->gpio_chip.get = pcf8574_gpio_get;
         pcf8574->gpio_chip.set = pcf8574_gpio_set;
         pcf8574->gpio_chip.direction_input = pcf8574_gpio_direction_input;
         pcf8574->gpio_chip.direction_output = pcf8574_gpio_direction_output;

 

         ret = devm_gpiochip_add_data(&client->dev, &pcf8574->gpio_chip, pcf8574);
         //This function is used to register the GPIO chip and associate it with a device structure. The last parameter is the device specific info, in this case the structure pcf8574

Information about the GPIO chip itself and the ops that we define are listed over here, and the last function dev_gpiochip_add_data is used to connect the device structure exposed by the i2c_client to the gpio_chip structure as well as associate everything with the private data structure that we use for the pcf8574(which actually contains all of this).

The gpio_state field just holds the current value of the GPIO pins. The way this is done is that before we do a write onto the chip, the value is first written into this variable and then from this variable it is written to the i2c device using the function  i2c_smbus_write_byte(pcf8574->client, pcf8574→gpio_state);

As for the device tree, it uses a similar concept to the SPI example described above wherein, whatever pins we use to connect this device need to be disabled from the main device tree, or we can overlay a fragment in our device tree overlay where we mark those pins as disabled.

 The device tree looks something like this:

```/dts-v1/;                                                                                                                                                                               
/plugin/;

/{ /* this is our device tree overlay root node */

  compatible = "ti,beaglebone", "ti,beaglebone-black";
  part-number = "BBB-I2C1"; // you can choose any name here but it should be memorable
  version = "00A0";

  fragment@0 {
    target = <&am33xx_pinmux>; // this is a link to an already defined node in the device tree, so that node is overlayed with our modification

    __overlay__ {
      i2c2_pins: pinmux_i2c2_pins {
        pinctrl-single,pins = < 
          0x17c 0x73 /* spi0_d1.i2c1_sda */ 
          0x178 0x73 /* spi0_cs0.i2c1_sdl */
        >;
      };  
    };  
  };  

  fragment@1 {
    target = <&i2c2>;

    __overlay__ {
      pinctrl-0 = <&i2c2_pins>;
      #address-cells = <1>;
      #size-cells = <0>;

      clock-frequency = <100000>;
      status = "okay";

      pcf: pcf@27 { /* the real time clock defined as child of the i2c1 bus */
        compatible = "pcf8574,pcf8574_i2c";
        reg = <0x27>;
      };  
    };  
  };  
}; /* root node end */
```                        

# HC05 Bluetooth Module Serial Device Subsystem based Driver(UART):

One of the interesting things that I worked on was the HC05 bluetooth sensor module. This module has the ability to connect to, lets say a smartphone, over which AT commands as well as serial ASCII characters can be sent and received. I implemented the RX side of the driver, to be able to receive instructions and data payload from a smartphone connected to the HC05 module.
The subsystem used for this work was the serial bus, serdev subsystem, and the data received was directed towards stdout.
The serial bus subsystem was chosen since, for the case of sensors like HC05 it abstracts a lot of initialization that would have to be done manually if the uart subsystem, that has been there in the kernel for a very long time, had been used.


The structure of the driver is as follows:

We have a driver structure which contains the probe and remove function along with the id_table and and name. In the probe function we register the ops, called serdev_device_ops which looks like this for the hc05 driver:

        static const struct serdev_device_ops hc05_ops = {
             .receive_buf = hc05_recv,
        };

The probe function also registers various values to the device like the baud rate, flowcontrol, and parity. This initializes the UART pins on the beaglebone to be ready to receive data a certain way. These parameters depend on the device and need to be derived from the datasheet.


Inside the serdev_device_ops, the receive function looks like this:

        static int hc05_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size)


The buffer variable in this function contains the message that is being sent from someother device to the HC05 module(lets say from a smartphone to the HC05 module). Since the module uses the UART protocol, the receive function is called n number of times where n is the length of the message being sent.
The device tree in this case it pretty simple and similar to SPI, I2C, wherein we just need to disable the UART pins in the main device tree before using it in ours.
