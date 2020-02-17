#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h> 
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include "conversion.h"

#ifndef MY_MORSE_READ_GPIO_H
#define MY_MORSE_READ_GPIO_H

void init_read_gpio(unsigned long read_period);

void free_read_gpio(void);

#endif
