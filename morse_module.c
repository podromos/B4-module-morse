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

static char* letter_morse_code[26]=
	{
		".-", 		//A
		"-...",		//B
		"-.-.",		//C
		"-..",		//D
		".",		//E
		"..-.",		//F
		"--.",		//G
		"....",		//H
		"..",		//I
		".---",		//J
		"-.-",		//K
		".-..",		//L
		"--",		//M
		"-.",		//N
		"---",		//O
		".--.",		//P
		"--.-",		//Q
		".-.",		//R
		"...",		//S
		"-",		//T
		"..-",		//U
		"...-",		//V
		".--",		//W
		"-..-",		//X
		"-.--",		//Y
		"--..",		//Z
	}; 

static char* digit_morse_code[10]=
	{
		".----",	//0
		"..---",	//1
		"...--",	//2
		"....-",	//3
		".....",	//4
		"-....",	//5
		"--...",	//6
		"---..",	//7
		"----.",	//8
		"-----",	//9
	};

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
		if(*morse_code=='.'){
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
		if (*morse_code=='.'){
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
	else{
		return NULL;
	}
}

/* Ecriture */
struct write_data{
	/* Buffer des données ascii à écrire */
	char* buffer;
	size_t buffer_capacity;
	size_t buffer_size;
	size_t buffer_pos;
	/* Données d'écriture d'un caractère */
	int is_special;
	char* cur_morse_code;
	uint8_t cur_morse_code_pos;
	uint8_t count;
	/* Données timer */
	struct timer_list timer;
	unsigned long period;
};

static struct write_data w_data; 

int init_write_data(const char *user_buffer,size_t size){
	w_data.buffer=(char*)kmalloc(size,GFP_KERNEL);
	if (w_data.buffer==NULL){
		printk(KERN_INFO "Write Allocation error\n");
		return 0;
	}
	if (copy_from_user(w_data.buffer,user_buffer,size)>0){
		printk(KERN_INFO "Copy from user error\n");
		return 0;
	}
	w_data.buffer_size=size;
	w_data.buffer_pos=0;
	return 1;
} 

void start_writing_morse_char(void){
	char cur_morse_char=w_data.cur_morse_code[w_data.cur_morse_code_pos];
	if (cur_morse_char=='.'){
		w_data.is_special=0;
		w_data.count=1;
	}
	else if(cur_morse_char=='-'){
		w_data.is_special=0;
		w_data.count=3;
	}
	else{
		w_data.is_special=1;
		w_data.count=1;
	}
}

int start_writing_char(void){
	if(w_data.buffer_pos==w_data.buffer_size){
		return 1;
	}

	else if (w_data.buffer[w_data.buffer_pos]==' '){
		w_data.is_special=1;
		w_data.count=4;
		return 0;
	}
	else{
		w_data.cur_morse_code=char_to_morse_code(w_data.buffer[w_data.buffer_pos]);
		w_data.cur_morse_code_pos=0;
		start_writing_morse_char();
		return 0;
	}
}
		
ssize_t dev_write(struct file * fil, const char * buff, size_t len, loff_t* off){
	init_write_data(buff,len);
	start_writing_char();
	mod_timer (&(w_data.timer),jiffies+w_data.period);
	return 1;
}

void write_timer_routine(struct timer_list* timer){
	mod_timer (timer , jiffies + w_data.period);
	if (w_data.is_special){
		printk(KERN_INFO"0");
		if(w_data.count==0){
			w_data.buffer_pos++;
			if(!start_writing_char()){
				mod_timer (timer , 0);
			}
		}
		else{
			w_data.count--;
		}
	}
	else if (w_data.count==0){
		printk(KERN_INFO"0");
		w_data.cur_morse_code_pos++;
		start_writing_morse_char();
	}
	else{
		printk(KERN_INFO"1");
		w_data.count--;
	}
}

static struct file_operations fops =
{
	.write = dev_write ,
};



/* Fonction du module*/

static int __init morse_init(void) {
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

	return 0;
}

static void __exit morse_exit(void) {
	morse_tree_free(morse_tree);
	del_timer (&(w_data.timer));
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(morse_init);
module_exit(morse_exit);

