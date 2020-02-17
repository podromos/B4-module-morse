#include "morse_read_gpio.h"

#define SIZE_OF_BUFFER 100
#define RPI_GPIO_IN 19

struct read_gpio_data{
	struct timer_list timer;
	unsigned long period;
	int end;
	/* buffer dans lequel est stocké la chaine de caractères ASCII lue */
	char buffer[SIZE_OF_BUFFER];
	int cur_buffer_pos;
};

static struct read_gpio_data r_gpio_data;

void init_read_gpio_data(void){
	r_gpio_data.end=0;
	r_gpio_data.cur_buffer_pos=0;
	convert_bit_to_char_online(RESET);
}

static void read_gpio_timer_routine (struct timer_list* timer) {
	char c;
	int val;
	if(r_gpio_data.end){
		printk(KERN_INFO"FIN LECTURE = %s\n",r_gpio_data.buffer);
		enable_irq(gpio_to_irq(RPI_GPIO_IN));
		return; 
	}

	val=gpio_get_value(RPI_GPIO_IN);
	mod_timer (&r_gpio_data.timer , jiffies + r_gpio_data.period );
	c=convert_bit_to_char_online(val);
	/*Si val ne peut être le bit suivant dans le caractère ASCII courant*/
	if(c){
		r_gpio_data.buffer[r_gpio_data.cur_buffer_pos]=c;
		convert_bit_to_char_online(RESET);
		convert_bit_to_char_online(val);
		
		/*Deux espaces successifs sont considérés comme la fin de la transmission*/
		if (((c==' ') && (r_gpio_data.buffer[r_gpio_data.cur_buffer_pos-1]==' '))
		|| (r_gpio_data.cur_buffer_pos+1==SIZE_OF_BUFFER)){
			r_gpio_data.end=1;
		}
		
		r_gpio_data.cur_buffer_pos++;
	}
}

static irqreturn_t gpio_irq_handler(int irq, void * ident){
	disable_irq_nosync(irq);
	mod_timer (&r_gpio_data.timer , jiffies + r_gpio_data.period/2 );
	init_read_gpio_data();
	printk(KERN_INFO"FIN IRQ READ");
	return IRQ_HANDLED;
}

void init_read_gpio(unsigned long read_period){
	r_gpio_data.period=read_period;
	
	gpio_request(RPI_GPIO_IN,THIS_MODULE->name);
	gpio_direction_input(RPI_GPIO_IN);
	
	if (request_irq(gpio_to_irq(RPI_GPIO_IN), gpio_irq_handler,
					IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
					THIS_MODULE->name, THIS_MODULE->name)){
		printk(KERN_ALERT"Problème utilisation de la l'interruption");
	}
	printk(KERN_INFO"FIN READ GPIO INIT\n");
	timer_setup(&r_gpio_data.timer,read_gpio_timer_routine,0);
	
}

void free_read_gpio(void){
	del_timer (&(r_gpio_data.timer));
	free_irq(gpio_to_irq(RPI_GPIO_IN), THIS_MODULE->name);
	gpio_free(RPI_GPIO_IN);
}
