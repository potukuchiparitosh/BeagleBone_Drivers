obj-m +=bmp280_spi_simple.o
#obj-m +=bmp280_spi.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
