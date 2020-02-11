#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h> 
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/wait.h>


#ifndef MY_MORSE_WRITE_H
#define MY_MORSE_WRITE_H

void init_morse_write(unsigned long period);

ssize_t morse_write(struct file * fil, const char * buff, size_t len, loff_t* off);

void free_morse_write(void);
#endif
