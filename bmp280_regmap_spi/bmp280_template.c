#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define BMP280_REG_TEMP 0xFA // Example register addresses
#define BMP280_REG_PRESS 0xF7

struct bmp280_data {
    struct regmap *regmap;
    struct device *dev;
};

static const struct regmap_config bmp280_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
};

static int bmp280_read_raw(struct iio_dev *indio_dev, struct iio_chan_spec const *chan, int *val, int *val2, long mask) {
    struct bmp280_data *data = iio_priv(indio_dev);
    int ret;
    __be16 regval;

    switch (chan->type) {
    case IIO_PRESSURE:
        ret = regmap_bulk_read(data->regmap, BMP280_REG_PRESS, &regval, 2);
        if (ret < 0)
            return ret;
        *val = be16_to_cpu(regval);
        return IIO_VAL_INT;
    case IIO_TEMP:
        ret = regmap_bulk_read(data->regmap, BMP280_REG_TEMP, &regval, 2);
        if (ret < 0)
            return ret;
        *val = be16_to_cpu(regval);
        return IIO_VAL_INT;
    default:
        return -EINVAL;
    }
}

static const struct iio_info bmp280_info = {
    .read_raw = bmp280_read_raw,
};

static const struct iio_chan_spec bmp280_channels[] = {
    {
        .type = IIO_PRESSURE,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
    {
        .type = IIO_TEMP,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
    },
};

static int bmp280_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct iio_dev *indio_dev;
    struct bmp280_data *data;
    int ret;

    indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
    if (!indio_dev)
        return -ENOMEM;

    data = iio_priv(indio_dev);
    data->dev = &client->dev;

    data->regmap = devm_regmap_init_i2c(client, &bmp280_regmap_config);
    if (IS_ERR(data->regmap))
        return PTR_ERR(data->regmap);

    indio_dev->info = &bmp280_info;
    indio_dev->channels = bmp280_channels;
    indio_dev->num_channels = ARRAY_SIZE(bmp280_channels);
    indio_dev->name = "bmp280";
    indio_dev->modes = INDIO_DIRECT_MODE;

    ret = iio_device_register(indio_dev);
    if (ret)
        return ret;

    i2c_set_clientdata(client, indio_dev);
    return 0;
}

static int bmp280_remove(struct i2c_client *client) {
    struct iio_dev *indio_dev = i2c_get_clientdata(client);

    iio_device_unregister(indio_dev);
    return 0;
}

static const struct i2c_device_id bmp280_id[] = {
    { "bmp280", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, bmp280_id);

static struct i2c_driver bmp280_driver = {
    .driver = {
        .name = "bmp280",
    },
    .probe = bmp280_probe,
    .remove = bmp280_remove,
    .id_table = bmp280_id,
};

module_i2c_driver(bmp280_driver);

MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("I2C driver for BMP280 sensor");
MODULE_LICENSE("GPL");

