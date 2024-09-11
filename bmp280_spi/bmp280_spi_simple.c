#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/i2c.h>
#include<linux/regmap.h>
#include<linux/iio/iio.h>
#include<linux/iio/sysfs.h>
#include<linux/types.h>
#include<linux/spi/spi.h>
#define BMP280_REG_PRESSURE 0xF7
#define BMP280_DIG_P 0x8E
#define BMP280_REG_TEMP 0xFA
#define BMP280_DIG_T 0x88


typedef int32_t BMP280_S32_t;

struct bmp280 {
        struct spi_device *spi;
};


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


static int bmp280_read_raw(struct iio_dev *iio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask) {
        struct bmp280 *bmp280 = iio_priv(iio_dev);
        int ret;
        char dig_p[18];
        u8 buf[3];
        u8 dig_t[6];
        u8 addr_reg_temp[3] = {0xFA, 0xFB, 0xFC};
        u8 addr_dig_temp[6] = {0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D};
        int jj;
        int k;

        u32 adc_T,adc_P,p;
        u16 dig_T1, dig_T2, dig_T3;
        u16 dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
        int32_t var1,var2,T,t_fine = 0;

        printk(KERN_ALERT "inside bmp280_read_raw\n");
        switch(chan->type) {
                case IIO_PRESSURE:

printk("Hello pressure\n");


                        buf[0] = spi_w8r8(bmp280->spi, 0xF7);
                        buf[1] = spi_w8r8(bmp280->spi, 0xF8);
                        buf[2] = spi_w8r8(bmp280->spi, 0xF9);

                        adc_P = ((u32)(unsigned char)buf[0] << 12) |
                                ((u32)(unsigned char)buf[1] << 4) |
                                ((u32)(unsigned char)buf[2] >> 4);


                        dig_p[0] = spi_w8r8(bmp280->spi, 0x8E);
                        dig_p[1] = spi_w8r8(bmp280->spi, 0x8F);
                        dig_p[2] = spi_w8r8(bmp280->spi, 0x90);
                        dig_p[3] = spi_w8r8(bmp280->spi, 0x91);
                        dig_p[4] = spi_w8r8(bmp280->spi, 0x92);
                        dig_p[5] = spi_w8r8(bmp280->spi, 0x93);
                        dig_p[6] = spi_w8r8(bmp280->spi, 0x94);
                        dig_p[7] = spi_w8r8(bmp280->spi, 0x95);
                        dig_p[8] = spi_w8r8(bmp280->spi, 0x96);
                        dig_p[9] = spi_w8r8(bmp280->spi, 0x97);
                        dig_p[10] = spi_w8r8(bmp280->spi, 0x98);
                        dig_p[11] = spi_w8r8(bmp280->spi, 0x99);
                        dig_p[12] = spi_w8r8(bmp280->spi, 0x9A);
                        dig_p[13] = spi_w8r8(bmp280->spi, 0x9B);
                        dig_p[14] = spi_w8r8(bmp280->spi, 0x9C);
                        dig_p[15] = spi_w8r8(bmp280->spi, 0x9D);
                        dig_p[16] = spi_w8r8(bmp280->spi, 0x9E);
                        dig_p[17] = spi_w8r8(bmp280->spi, 0x9F);




                        dig_P1 = ((u16)(unsigned char)dig_p[1] << 8) |
                                ((u16)(unsigned char)dig_p[0]);

                        dig_P2 = ((u16)(unsigned char)dig_p[3] << 8) |
                                ((u16)(unsigned char)dig_p[2]);

                        dig_P3 = ((u16)(unsigned char)dig_p[5] << 8) |
                                ((u16)(unsigned char)dig_p[4]);

                        dig_P4 = ((u16)(unsigned char)dig_p[7] << 8) |
                                ((u16)(unsigned char)dig_p[6]);

                        dig_P5 = ((u16)(unsigned char)dig_p[9] << 8) |
                                ((u16)(unsigned char)dig_p[8]);

                        dig_P6 = ((u16)(unsigned char)dig_p[11] << 8) |
                                ((u16)(unsigned char)dig_p[10]);

                        dig_P7 = ((u16)(unsigned char)dig_p[13] << 8) |
                                ((u16)(unsigned char)dig_p[12]);

                        dig_P8 = ((u16)(unsigned char)dig_p[15] << 8) |
                                ((u16)(unsigned char)dig_p[14]);

                        dig_P9 = ((u16)(unsigned char)dig_p[17] << 8) |
                                ((u16)(unsigned char)dig_p[16]);



                        var1 = ((int32_t)t_fine/2) - 64000;
                        var2 = var1 * var1 * ((u32)dig_P6) / 32768;
                        var2 = var2 + var1 * ((u32)dig_P5) * 2;
                        var2 = (var2/4)+(((u32)dig_P4) * 65536);
                        var1 = (((u32)dig_P3) * var1 * var1 / 524288 + ((u32)dig_P2) * var1) / 524288;
                        var1 = (1 + var1 / 32768)*((u32)dig_P1);
                        if (var1 == 0)
                        {
                                *val= 0;
                                return IIO_VAL_INT; // avoid exception caused by division by zero
                        }
                        p = 1048576 - (u32)adc_P;
                        p = (p - (var2 / 4096)) * 6250 / var1;
                        var1 = ((u32)dig_P9) * p * p / 2147483648;
                        var2 = p * ((u32)dig_P8) / 32768;
                        p = p + (var1 + var2 + ((u32)dig_P7)) / 16;

                        *val = p;

                        return IIO_VAL_INT;
#if 0
                        st->tx_buf[0] = BMP280_REG_PRESSURE;

                        st->ring_xfer[0].tx_buf = &st->tx_buf[0];
                        st->ring_xfer[0].rx_buf = NULL;
                        st->ring_xfer[0].len = 1;
                        st->ring_xfer[0].cs_change = 0;

                        st->ring_xfer[1].tx_buf = NULL;
                        st->ring_xfer[1].rx_buf = &st->rx_buf[0];
                        st->ring_xfer[1].len = 3;
                        st->ring_xfer[1].cs_change = 1;


                        spi_message_init(&st->ring_msg);
                        spi_message_add_tail(&st->ring_xfer[0], &st->ring_msg);
                        spi_message_add_tail(&st->ring_xfer[1], &st->ring_msg);

                        ret = spi_sync(st->spi, &st->ring_msg);
                        if(ret < 0) {
                                printk(KERN_INFO "Something wrong here buddy\n");
                                return ret;
                        }

                        for( jj = 0; jj < 3; jj++) {
                                buf[jj] = st->rx_buf[jj];
                        }

                        adc_P = ((u32)(unsigned char)buf[0] << 16) |
                                ((u32)(unsigned char)buf[1] << 8) |
                                (((u32)(unsigned char)buf[2] >> 4) << 4);
                        adc_P = adc_P >> 4;



                        st->tx_buf[0] = BMP280_DIG_P;

                        st->ring_xfer[0].tx_buf = &st->tx_buf[0];
                        st->ring_xfer[0].rx_buf = NULL;
                        st->ring_xfer[0].len = 1;
                        st->ring_xfer[0].cs_change = 0;

                        st->ring_xfer[1].tx_buf = NULL;
                        st->ring_xfer[1].rx_buf = &st->rx_buf[0];
                        st->ring_xfer[1].len = 18;
                        st->ring_xfer[1].cs_change = 1;


                        spi_message_init(&st->ring_msg);
                        spi_message_add_tail(&st->ring_xfer[0], &st->ring_msg);
                        spi_message_add_tail(&st->ring_xfer[1], &st->ring_msg);

                        ret = spi_sync(st->spi, &st->ring_msg);
                        if(ret < 0) {
                                printk(KERN_INFO "Something wrong here buddy\n");
                                return ret;
                        }

                        for( jj = 0; jj < 18; jj++) {
                                dig_p[jj] = st->rx_buf[jj];
                        }


                        dig_P1 = ((u16)(unsigned char)dig_p[1] << 8) |
                                ((u16)(unsigned char)dig_p[0]);

                        dig_P2 = ((u16)(unsigned char)dig_p[3] << 8) |
                                ((u16)(unsigned char)dig_p[2]);

                        dig_P3 = ((u16)(unsigned char)dig_p[5] << 8) |
                                ((u16)(unsigned char)dig_p[4]);

                        dig_P4 = ((u16)(unsigned char)dig_p[7] << 8) |
                                ((u16)(unsigned char)dig_p[6]);

                        dig_P5 = ((u16)(unsigned char)dig_p[9] << 8) |
                                ((u16)(unsigned char)dig_p[8]);

                        dig_P6 = ((u16)(unsigned char)dig_p[11] << 8) |
                                ((u16)(unsigned char)dig_p[10]);

                        dig_P7 = ((u16)(unsigned char)dig_p[13] << 8) |
                                ((u16)(unsigned char)dig_p[12]);

                        dig_P8 = ((u16)(unsigned char)dig_p[15] << 8) |
                                ((u16)(unsigned char)dig_p[14]);

                        dig_P9 = ((u16)(unsigned char)dig_p[17] << 8) |
                                ((u16)(unsigned char)dig_p[16]);



                        var1 = ((int32_t)t_fine/2) - 64000;
                        var2 = var1 * var1 * ((u32)dig_P6) / 32768;
                        var2 = var2 + var1 * ((u32)dig_P5) * 2;
                        var2 = (var2/4)+(((u32)dig_P4) * 65536);
                        var1 = (((u32)dig_P3) * var1 * var1 / 524288 + ((u32)dig_P2) * var1) / 524288;
                        var1 = (1 + var1 / 32768)*((u32)dig_P1);
                        if (var1 == 0)
                        {
                                *val= 0;
                                return IIO_VAL_INT; // avoid exception caused by division by zero
                        }
                        p = 1048576 - (u32)adc_P;
                        p = (p - (var2 / 4096)) * 6250 / var1;
                        var1 = ((u32)dig_P9) * p * p / 2147483648;
                        var2 = p * ((u32)dig_P8) / 32768;
                        p = p + (var1 + var2 + ((u32)dig_P7)) / 16;

                        *val = p;

                        return IIO_VAL_INT;
#endif
                case IIO_TEMP:
#if 0
                        /*Setting up the transfer structures*/
                        for (k = 0; k < 6; k=k+2) {
                                printk(KERN_INFO "Going through each transfer structure population for the adc_T buf calculation\n");

                                st->tx_buf[k] = addr_reg_temp[k/2];

                                printk(KERN_INFO "Sending command: %02x\n", st->tx_buf[k]);
                                st->ring_xfer[k].tx_buf = &st->tx_buf[k];
                                st->ring_xfer[k].rx_buf = NULL;
                                st->ring_xfer[k].len = 1;
                                st->ring_xfer[k].cs_change = 0;

                                st->ring_xfer[k+1].tx_buf = NULL;
                                st->ring_xfer[k+1].rx_buf = &st->rx_buf[k+1];
                                st->ring_xfer[k+1].len = 1;
                                st->ring_xfer[k+1].cs_change = 1;

                        }
                        /*Setting up the message structure with the transfers from above*/
                        spi_message_init(&st->ring_msg);
                        for(k = 0; k < 6 ;k++) {
                                printk(KERN_INFO "Going through addin thte message tail for each transfer structure into the message structure\n");
                                spi_message_add_tail(&st->ring_xfer[k], &st->ring_msg);
                        }

                        printk(KERN_INFO "BEFORE sync : The value of buf is %d %d %d\n",buf[0],buf[1],buf[2]);
                        printk(KERN_INFO "XIMPORTANT BEFORE sync : The value of rx_buf is %d %d %d\n",st->rx_buf[1],st->rx_buf[3],st->rx_buf[5]);

                        ret = spi_sync(st->spi, &st->ring_msg);
                        if(ret < 0) {
                                printk(KERN_INFO "Something wrong here buddy\n");
                                return ret;
                        }
                        printk(KERN_ALERT "SYNC DONE\n");
                        printk(KERN_INFO "XIMPORTANT : SPI received data: rx_buf[1] %02x rx_buf[3] %02x rx_buf[5] %02x\n", st->rx_buf[1], st->rx_buf[3], st->rx_buf[5]);


                        buf[0] = st->rx_buf[1];buf[1] = st->rx_buf[3];buf[2] = st->rx_buf[5];

                        printk(KERN_INFO "rx_buf ------> buf for the three items was just done\n");
                        printk(KERN_INFO "XIMPORTANT : SPI received data:buf[0] %02x buf[1] %02x buf[2] %02x\n", buf[0], buf[1], buf[2]);
                        //*val = be16_to_cpu(regval);
                        adc_T = ((u32)(unsigned char)buf[0] << 12) |
                                ((u32)(unsigned char)buf[1] << 4) |
                                ((u32)(unsigned char)buf[2] >> 4);

                        // Check if the values are correctly processed
                        printk(KERN_INFO "Processed adc_T: %d\n", adc_T);
                        printk(KERN_INFO "AFTER sync and adc_T calculation : The value of rx_buf is rx_buf[1] %d rx_buf[3] %d rx_buf[5] %d\n",st->rx_buf[1],st->rx_buf[3],st->rx_buf[5]);
                        printk(KERN_INFO "The value of buf finally is %d %d %d\n",buf[0],buf[1],buf[2]);
                        printk(KERN_INFO "The value of adc_T is %d\n", adc_T);



                        /*Setting up the transfer structures*/
                        for (k = 0; k < 12; k=k+2) {
                                printk(KERN_INFO "Going through each transfer structure for dig values\n");

                                st->tx_buf[k] = addr_dig_temp[k/2];

                                st->ring_xfer[k].tx_buf = &st->tx_buf[k];
                                st->ring_xfer[k].rx_buf = NULL;
                                st->ring_xfer[k].len = 1;
                                st->ring_xfer[k].cs_change = 0;

                                st->ring_xfer[k+1].tx_buf = NULL;
                                st->ring_xfer[k+1].rx_buf = &st->rx_buf[k+1];
                                st->ring_xfer[k+1].len = 1;
                                st->ring_xfer[k+1].cs_change = 1;

                        }

                        /*Setting up the message structure with the transfer structures from above*/
                        spi_message_init(&st->ring_msg);
                        for(k = 0; k < 12 ;k++) {
                                printk(KERN_INFO "Going through each transfer structure for dig message structures\n");
                                spi_message_add_tail(&st->ring_xfer[k], &st->ring_msg);
                        }


                        ret = spi_sync(st->spi, &st->ring_msg);
                        if(ret < 0) {
                                printk(KERN_INFO "Something is wrong here too buddy\n");
                                return ret;
                        }


                        for( k = 0; k <6;k++) {
                                dig_t[k] = st->rx_buf[2*k + 1];
                        }

                        dig_T1 = ((u16)(unsigned char)dig_t[1] << 8) |
                                ((u16)(unsigned char)dig_t[0]);

                        dig_T2 = ((u16)(unsigned char)dig_t[3] << 8) |
                                ((u16)(unsigned char)dig_t[2]);

                        dig_T3 = ((u16)(unsigned char)dig_t[5] << 8) |
                                ((u16)(unsigned char)dig_t[4]);
                        printk(KERN_INFO "The value of dig_t is %d%d%d%d%d%d\n",dig_t[0],dig_t[1],dig_t[2],dig_t[3],dig_t[4],dig_t[5]);


                        var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
                        var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
                        t_fine = var1 + var2;
                        T = (t_fine * 5 + 128) >> 8;

                        *val = T;

                        printk(KERN_ALERT "The temperature is %d\n",T);
                        //                    *val = (u32)(unsigned char)buf[0];
                        return IIO_VAL_INT;
#endif

#if 0
                        ret = spi_w8r8(bmp280->spi, 0XFA);
                        if(ret < 0) {
                                printk(KERN_INFO "the return value of w8r8 is less than 0\n");
                                return ret;
                        }
#endif
                        buf[0] = spi_w8r8(bmp280->spi, 0xFA);
                        buf[1] = spi_w8r8(bmp280->spi, 0xFB);
                        buf[2] = spi_w8r8(bmp280->spi, 0xFC);


                        dig_t[0] = spi_w8r8(bmp280->spi, 0x88);
                        dig_t[1] = spi_w8r8(bmp280->spi, 0x89);
                        dig_t[2] = spi_w8r8(bmp280->spi, 0x8A);
                        dig_t[3] = spi_w8r8(bmp280->spi, 0x8B);
                        dig_t[4] = spi_w8r8(bmp280->spi, 0x8C);
                        dig_t[5] = spi_w8r8(bmp280->spi, 0x8D);



                        adc_T = ((u32)(unsigned char)buf[0] << 12) |
                                ((u32)(unsigned char)buf[1] << 4) |
                                ((u32)(unsigned char)buf[2] >> 4);

                        printk(KERN_INFO "XIMPORTANT : SPI buf data: buf[0] %02x buf[1] %02x buf[2] %02x\n", buf[0],buf[1], buf[2]);
                        printk(KERN_INFO "Processed adc_T: %d\n", adc_T);

                        dig_T1 = ((u16)(unsigned char)dig_t[1] << 8) |
                                ((u16)(unsigned char)dig_t[0]);

                        dig_T2 = ((u16)(unsigned char)dig_t[3] << 8) |
                                ((u16)(unsigned char)dig_t[2]);

                        dig_T3 = ((u16)(unsigned char)dig_t[5] << 8) |
                                ((u16)(unsigned char)dig_t[4]);
                        printk(KERN_INFO "The value of dig_t is %d%d%d%d%d%d\n",dig_t[0],dig_t[1],dig_t[2],dig_t[3],dig_t[4],dig_t[5]);


                        var1 = ((((adc_T>>3) - ((BMP280_S32_t)dig_T1<<1))) * ((BMP280_S32_t)dig_T2)) >> 11;
                        var2 = (((((adc_T>>4) - ((BMP280_S32_t)dig_T1)) * ((adc_T>>4) - ((BMP280_S32_t)dig_T1))) >> 12) * ((BMP280_S32_t)dig_T3)) >> 14;
                        t_fine = var1 + var2;
                        T = (t_fine * 5 + 128) >> 8;

                        *val = T;

                        printk(KERN_ALERT "The temperature is %d\n",T);
                        return IIO_VAL_INT;
                default:
                        return -EINVAL;
        }
}
static const struct iio_info bmp280_info = {
        .read_raw = bmp280_read_raw,
};
static int bmp280_probe(struct spi_device *spi) {
        int ret;
        struct iio_dev *iio_dev;
        struct bmp280 *bmp280;
        u8 buffer[2];

        printk(KERN_ALERT "BMP280 :  We have entered the probe function\n");

        iio_dev = devm_iio_device_alloc(&spi->dev, sizeof(struct bmp280));
        if(!iio_dev) {
                printk(KERN_INFO "iio_dev could not be allocated\n");
                return -1;
        }

        bmp280 = iio_priv(iio_dev);
        bmp280->spi = spi;


        iio_dev->info = &bmp280_info;
        iio_dev->channels = bmp280_channels;
        iio_dev->num_channels = 2;
        iio_dev->name = "bmp280";
        iio_dev->modes = INDIO_DIRECT_MODE;
        iio_dev->dev.parent = &spi->dev;
        iio_dev->dev.of_node = spi->dev.of_node;

        spi->mode = SPI_MODE_0;
        spi->max_speed_hz = 10000000;

        ret = spi_setup(spi);
        if(ret < 0) {
                printk(KERN_ALERT "spi setup failed\n");
                return ret;
        }

#if 1
        buffer[0] = 0x75;
        buffer[1] = 0xA0;
#endif
#if 0
        buffer[0] = 0xF1;
        buffer[1] = 0xA0;
#endif
        ret = spi_write(bmp280->spi, buffer, 2);
        if(ret < 0) {
                printk(KERN_INFO "Could not write to 75\n");
                return ret;
        }
#if 1
        buffer[0] = 0x74;
        buffer[1] = 0xB7;
#endif
#if 0
        buffer[0] = 0xF1;
        buffer[1] = 0xA0;
#endif
        ret = spi_write(bmp280->spi, buffer, 2);
        if(ret < 0) {
                printk(KERN_INFO "Could not write to 74\n");
                return ret;
        }

        spi_set_drvdata(spi, iio_dev);
        
        printk(KERN_ALERT "PROBE COMPLETED\n");
        ret = devm_iio_device_register(&spi->dev, iio_dev);
        if(ret < 0) {
                printk("Could not register iio_device \n");
                return ret;
        }



}
static int bmp280_remove(struct spi_device *spi) {
        //struct iio_dev *iio_dev = spi_get_drvdata(spi);

       // iio_device_unregister(iio_dev);
        return 0;

}

static const struct spi_device_id bmp280_id[] = {
        { "bmp280", 0},
        { }
};

MODULE_DEVICE_TABLE(spi, bmp280_id);

static const struct of_device_id bmp280_match_table[] = {
        {.compatible = "bmp280,bmp280_spi"},
        { },
};
MODULE_DEVICE_TABLE(of,bmp280_match_table);

static struct spi_driver bmp280_driver = {
        .driver = {
                .name = "bmp280",
                .of_match_table = bmp280_match_table,
        },
        .probe = bmp280_probe,
        .remove = bmp280_remove,
        .id_table = bmp280_id,
};

module_spi_driver(bmp280_driver);


MODULE_AUTHOR("Paritosh Potukuchi");
MODULE_DESCRIPTION("I2C driver for BMP280 sensor");
MODULE_LICENSE("GPL");
