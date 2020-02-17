obj-m += morse_module.o 
morse_module-objs := morse_module_main.o morse_write.o morse_read_gpio.o conversion.o 

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
