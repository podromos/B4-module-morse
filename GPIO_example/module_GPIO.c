#include <linux/module.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/interrupt.h>

#define RPI_GPIO_IN  23
#define RPI_GPIO_OUT 24

static struct timer_list timer;

static irqreturn_t rpi_gpio_handler(int irq, void * ident){
	int value = gpio_get_value(RPI_GPIO_IN);
	printk (KERN_INFO "recu = %d \n",value);
	return IRQ_HANDLED;
}

static void do_something (struct timer_list* timer) {
	printk (KERN_ALERT "Yop !");
	static int value = 0;
	value=!value;
	printk(KERN_INFO"ecriture = %d\n",value);
	gpio_set_value(RPI_GPIO_OUT, value);
	mod_timer (timer , jiffies + HZ ); //réarmement du Timer
}

static int __init hello_start (void) {
	gpio_request(RPI_GPIO_IN,THIS_MODULE->name);
	gpio_direction_input(RPI_GPIO_IN);
	gpio_request(RPI_GPIO_OUT,THIS_MODULE->name);	
	gpio_direction_output(RPI_GPIO_OUT,1);

	request_irq(gpio_to_irq(RPI_GPIO_IN), rpi_gpio_handler,IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, THIS_MODULE->name, THIS_MODULE->name)

	timer_setup_on_stack(& exp_timer,do_something,0);
	mod_timer (timer , jiffies + HZ );
}
static void __exit hello_end (void) {
	free_irq(gpio_to_irq(RPI_GPIO_IN), THIS_MODULE->name);
	gpio_free(RPI_GPIO_IN);
	gpio_free(RPI_GPIO_OUT);
}
module_init (hello_start);
module_exit (hello_end);




