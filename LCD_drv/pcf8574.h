struct pcf8574 {
	struct i2c_client *client;
	struct gpio_chip gpio_chip;
	uint8_t gpio_state;
};
