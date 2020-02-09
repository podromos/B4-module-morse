#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h> 
#include <linux/timer.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sheryar Masood, Kossivi Kougblenou & Baptiste Lambert");
MODULE_DESCRIPTION("Driver de communication en code morse");
MODULE_VERSION("0.01");

/* Paramètres */

static uint write_period=1000;
module_param(write_period,uint,0);
MODULE_PARM_DESC(write_period,"Période d'écriture en micro secondes");

/* Conversion MORSE / ASCII */

static char letter_morse_code[][5]=
	{
		{1,2,0,0,0}, 	//A
		{2,1,1,1,0}, 	//B
		{2,1,2,1,0},	//C
		{2,1,1,0,0},	//D
		{1,0,0,0,0},	//E
		{1,1,2,1,0},	//F
		{2,2,1,0,0},	//G
		{1,1,1,1,0},	//H
		{1,1,0,0,0},	//I
		{1,2,2,2,0},	//J
		{2,1,2,0,0},	//K
		{1,2,1,1,0},	//L
		{2,2,0,0,0},	//M
		{2,1,0,0,0},	//N
		{2,2,2,0,0},	//O
		{1,2,2,1,0},	//P
		{2,2,1,2,1},	//Q
		{1,2,1,0,0},	//R
		{1,1,1,0,0},	//S
		{2,0,0,0,0},	//T
		{1,1,2,0,0},	//U
		{1,1,1,2,0},	//V
		{1,2,2,0,0},	//W
		{2,1,1,2,0},	//X
		{2,1,2,2,0},	//Y
		{2,2,1,1,0},	//Z
	}; 

static char digit_morse_code[][6]=
	{
		{1,2,2,2,2,0}, //0
		{1,1,2,2,2,0}, //1
		{1,1,1,2,2,0}, //2
		{1,1,1,1,2,0}, //3
		{1,1,1,1,1,0}, //4
		{2,1,1,1,1,0}, //5
		{2,2,1,1,1,0}, //6
		{2,2,2,1,1,0}, //7
		{2,2,2,2,1,0}, //8
		{2,2,2,2,2,0}, //9
	};

static char space_morse_code[3]=
		{0,0};

struct bit_code{
	unsigned char nb_bits;
	unsigned char bit_code;
};

struct bit_code morse_bit_code[4]=
{
	{.nb_bits=2, .bit_code=0x00}, 	// fin d'un caractère morse
	{.nb_bits=2, .bit_code=0x01},	// tih
	{.nb_bits=4, .bit_code=0x07},	// taah
};

inline static const struct bit_code* get_bit_code(int i){
	return &morse_bit_code[i];
} 

struct node{
	char root;
	struct node* taah_tree;
	struct node* ti_tree;
};

typedef struct node* morse_tree_t;

static morse_tree_t morse_tree_create(void){
	morse_tree_t tree = 
		(morse_tree_t)kmalloc(sizeof(struct node),GFP_KERNEL);

	tree->root=0;
	tree->taah_tree=NULL;
	tree->ti_tree=NULL;

	return tree;
}
static void morse_tree_free(morse_tree_t tree){
	if(tree->ti_tree!=NULL){
		morse_tree_free(tree->ti_tree);
	}
	if (tree->taah_tree!=NULL){
		morse_tree_free(tree->taah_tree);
	}
	kfree(tree);
}

static void morse_tree_add_char(morse_tree_t tree, char* morse_code,char c){
	while(*morse_code!='\0'){
		if(*morse_code==1){
			if(tree->ti_tree==NULL)
				tree->ti_tree=morse_tree_create();
			tree=tree->ti_tree;
		}
		else{
			if(tree->taah_tree==NULL)
				tree->taah_tree=morse_tree_create();
			tree=tree->taah_tree;
		}
		morse_code++;
	}
	tree->root=c;
}

static morse_tree_t morse_tree_init(morse_tree_t tree){
	char c;
	unsigned char i; 

	c='A';
	for (i=0;i<26;i++){
		morse_tree_add_char(tree,letter_morse_code[i],c);
		c++;
	}
	c='0';
	for(i=0;i<10;i++){
		morse_tree_add_char(tree,digit_morse_code[i],c);
		c++;
	}
	return tree;
}

static morse_tree_t morse_tree;

static char morse_code_to_char(morse_tree_t tree,char* morse_code){
	while(*morse_code!='\0'){
		if (tree==NULL){
			return 0;
		}
		if (*morse_code==1){
			tree=tree->ti_tree;
		}
		else{
			tree=tree->taah_tree;
		}
		morse_code++;
	}
	return tree->root;
}

static char* char_to_morse_code(char c){
	if ((c>='A') && (c<='Z')){
		return letter_morse_code[c-'A'];
	}
	else if ((c>='0') && (c<='9')){
		return digit_morse_code[c-'0'];
	}
	else if (c==' '){
		return space_morse_code;
	}
	else{
		return NULL;
	}
}

/* Ecriture */
struct write_data{
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

void init_morse_code(void){
	w_data.cur_morse_code=char_to_morse_code(w_data.buffer[w_data.cur_str_pos]);
}

void init_bit_code(void){
	w_data.cur_bit_code.nb_bits=get_bit_code(w_data.cur_morse_code[w_data.cur_morse_code_pos])->nb_bits;
	w_data.cur_bit_code.bit_code=get_bit_code(w_data.cur_morse_code[w_data.cur_morse_code_pos])->bit_code;
}

int init_write_data(const char *user_buffer,size_t size){
	/*init buffer*/
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
	
	/*init ASCII caractère*/
	init_morse_code();
	w_data.cur_morse_code_pos=0;
	
	/*init morse caractère*/
	init_bit_code();
	return 1;
} 


ssize_t dev_write(struct file * fil, const char * buff, size_t len, loff_t* off){
	printk("DEBUT WRITE\n");
	if (buff==NULL){
		return 0;
	}
	printk(KERN_INFO"DEBUT write = %s\n",buff);
	init_write_data(buff,len);
	printk(KERN_INFO"INIT ok_n\n");
	mod_timer (&(w_data.timer),jiffies+w_data.period);
	return 1;
}

void write_timer_routine(struct timer_list* timer){
	mod_timer (timer , jiffies + w_data.period);
	/* Ecriture */
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
		/* Ecriture du caractères ASCII terminé */
		else{
			w_data.cur_str_pos++;
			/* Il reste des caractères ASCII à écrire */
			if (w_data.cur_str_pos<w_data.str_size){
				init_morse_code();
				init_bit_code();
			}
			/* Ecriture terminée */
			else{
				mod_timer (timer , jiffies);
				printk("FIN ECRITURE\n");
			}
		}	
	}
}

static int dev_open (struct inode * inod , struct file * fil){
	printk(KERN_INFO"OPEN\n");
	return 0;
}

static ssize_t dev_read (struct file * fil, char * buff, size_t len, loff_t * off){
	printk(KERN_INFO"READ\n");
	return 1;
}

static int dev_rls (struct inode * inod, struct file * fil){
	printk(KERN_INFO"RLS\n");
	return 0;
}

static struct file_operations fops =
{
	.read = dev_read ,
	.open = dev_open ,
	.write = dev_write ,
	.release = dev_rls ,
};



/* Fonction du module*/

static int __init morse_init(void) {
	char c,cf;
	char* tmp;
	
	morse_tree=morse_tree_init(morse_tree_create());
	
	/* Déclaration du device associé à fops */
	if (register_chrdev (90 , "mon_device" , &fops )<0) {
		printk (KERN_ALERT "registration failed\n");
	} 
	else {
		printk (KERN_ALERT "registration success\n");
	}
	
	/* initialisation des données d'écriture */
	timer_setup_on_stack(&(w_data.timer),write_timer_routine,0 );
	w_data.period=write_period*1000000/HZ; 
	add_timer (&(w_data.timer));
	
	
	for (c='A';c<='Z';c++){
		tmp=char_to_morse_code(c);
		if (tmp==NULL){
			printk( "error char = %c \n",c);
		}
		cf=morse_code_to_char(morse_tree,tmp);
		if (cf==0){
			printk( "error str = %s \n",tmp);
		}
		printk( "%c = %c \n",c,cf);
	}
	
	return 0;
}

static void __exit morse_exit(void) {
	morse_tree_free(morse_tree);
	unregister_chrdev(90,"mon_device");
	del_timer (&(w_data.timer));
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(morse_init);
module_exit(morse_exit);

