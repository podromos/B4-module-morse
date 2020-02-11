#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h> 
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/wait.h>

#include "conversion.h"
#include "morse_write.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sheryar Masood, Kossivi Kougblenou & Baptiste Lambert");
MODULE_DESCRIPTION("Driver de communication en code morse");
MODULE_VERSION("0.01");

/* Paramètres */

static uint write_period=1000;
module_param(write_period,uint,0);
MODULE_PARM_DESC(write_period,"Période d'écriture en micro secondes");

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
	.write = morse_write ,
	.release = dev_rls ,
};



/* Fonction du module*/

static int __init morse_init(void) {
	char c,cf;
	char* tmp;
	
	morse_tree_init();
	/* initialisation de l'écriture*/
	init_morse_write(write_period*HZ/1000000);

	/* Déclaration du device associé à fops */
	if (register_chrdev (90 , "mon_device" , &fops )<0) {
		printk (KERN_ALERT "registration failed\n");
	} 
	else {
		printk (KERN_ALERT "registration success\n");
	}
	
	for (c='A';c<='Z';c++){
		tmp=char_to_morse_code(c);
		if (tmp==NULL){
			printk( "error char = %c \n",c);
		}
		cf=morse_code_to_char(tmp);
		if (cf==0){
			printk( "error str = %s \n",tmp);
		}
		printk( "%c = %c \n",c,cf);
	}
	
	return 0;
}

static void __exit morse_exit(void) {
	unregister_chrdev(90,"mon_device");
	free_morse_write();
	morse_tree_free();
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(morse_init);
module_exit(morse_exit);

