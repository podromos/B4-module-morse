#include "morse_write.h"
#include "conversion.h"

#define RPI_GPIO_OUT 24

/* Données d'écriture */
struct write_data{
	/* Flag d'attente */
	int end;
	wait_queue_head_t wait_queue;
	/* Buffer des données ascii à écrire */
	char* buffer;
	size_t str_size;
	size_t cur_str_pos;
	/* Données d'écriture d'un caractère ASCII*/
	const char* cur_morse_code; /* code morse du caractère ascii courant */
 	int cur_morse_code_pos;
 	/* Données d'écriture d'un caractère morse */
 	struct bit_code cur_bit_code;
	/* Données timer */
	struct timer_list timer;
	unsigned long period;
};

static struct write_data w_data; 

static void write_timer_routine(struct timer_list* timer);

void init_morse_write(unsigned long period){
	w_data.period=period;
	gpio_request(RPI_GPIO_OUT,THIS_MODULE->name);
	gpio_direction_output(RPI_GPIO_OUT,0);
	timer_setup_on_stack(&(w_data.timer),write_timer_routine,0 );
}

void free_morse_write(){
	del_timer (&(w_data.timer));
	gpio_free(RPI_GPIO_OUT);
}


static void init_morse_code(void){
	w_data.cur_morse_code=char_to_morse_code(w_data.buffer[w_data.cur_str_pos]);
	w_data.cur_morse_code_pos=0;
}

static void init_bit_code(void){
	w_data.cur_bit_code.nb_bits=get_bit_code(w_data.cur_morse_code[w_data.cur_morse_code_pos])->nb_bits;
	w_data.cur_bit_code.bit_code=get_bit_code(w_data.cur_morse_code[w_data.cur_morse_code_pos])->bit_code;
}

static int init_write_data(const char *user_buffer,size_t size){
	/* init synchro entre la fonction write et la routine du timer*/
	w_data.end=0;
	init_waitqueue_head(&w_data.wait_queue);
	/* init buffer */
	w_data.buffer=(char*)kmalloc(size,GFP_KERNEL);
	if (w_data.buffer==NULL){
		printk(KERN_INFO "Write Allocation error\n");
		return 0;
	}
	if (copy_from_user(w_data.buffer,user_buffer,size)>0){
		printk(KERN_INFO "Copy from user error\n");
		return 0;
	}
	w_data.str_size=size;
	w_data.cur_str_pos=0;
	
	/* init ASCII caractère*/
	init_morse_code();
	w_data.cur_morse_code_pos=0;
	
	/* init morse caractère*/
	init_bit_code();
	return 1;
} 

static void free_write_data(void){
	kfree(w_data.buffer);
}


static void write_timer_routine(struct timer_list* timer){
	if (w_data.end){
		return;
	}
	/* Rechargement du timer */
	mod_timer (timer , jiffies + w_data.period);

	/* Ecriture */
	gpio_set_value(RPI_GPIO_OUT, w_data.cur_bit_code.bit_code&0x01);
	printk(KERN_DEBUG" WRITE = %d\n",w_data.cur_bit_code.bit_code&0x01);
	/* Mise à jour des données d'écriture */
	w_data.cur_bit_code.nb_bits--;
	/* Il reste des bits à écrire */
	if (w_data.cur_bit_code.nb_bits){ 
		w_data.cur_bit_code.bit_code>>=1;
	}
	/* Ecriture du caractère morse courant terminé */
	else{
		/* Il reste des carctères Morse à écrire */
		if(w_data.cur_morse_code[w_data.cur_morse_code_pos]){
			w_data.cur_morse_code_pos++;
			init_bit_code();
		}
		/* Ecriture du caractère ASCII terminé */
		else{
			/* Parcourt de la chaine de caractères jusqu'à trouver */
			/* un caractère ASCII convertible en morse ou la fin de la chaine de caractères */
			while(1){
				w_data.cur_str_pos++;
				/* Si on atteint la fin de la chaine de caractères*/
				if(w_data.cur_str_pos>=w_data.str_size){
					w_data.end=1;
					wake_up_interruptible(&w_data.wait_queue);
					break;
				}
				init_morse_code();
				/* Si le caractère ASCII est reconnu */
				if (w_data.cur_morse_code!=NULL){
					init_bit_code();
					break;
				}
			}
		}
	}
}

ssize_t morse_write(struct file * fil, const char * buff, size_t len, loff_t* off){
	printk("DEBUT WRITE\n");
	if (buff==NULL){
		return 0;
	}
	printk(KERN_INFO"DEBUT write = %.*s\n",(int)len,buff);
	init_write_data(buff,len);
	add_timer (&(w_data.timer));
	/* Attend la fin de l'écriture */
	wait_event_interruptible(w_data.wait_queue,w_data.end);
	free_write_data();
	printk(KERN_INFO"FIN TERMINE\n");
	return len;
}