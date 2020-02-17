#include <linux/slab.h>

#ifndef MY_CONVERSION_H
#define MY_CONVERSION_H

#define RESET -1

struct bit_code{
	unsigned char nb_bits;
	unsigned char bit_code;
};

const struct bit_code* get_bit_code(int i);

struct node{
	char root;
	struct node* one;
	struct node* zero;
};

typedef struct node* morse_tree_t;

void morse_tree_init(void);

void morse_tree_free(void);

char convert_bit_to_char_online(int bit);

char* char_to_morse_code(char c);

#endif
