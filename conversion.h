#include <linux/slab.h>

#ifndef MY_CONVERSION_H
#define MY_CONVERSION_H

struct bit_code{
	unsigned char nb_bits;
	unsigned char bit_code;
};

inline const struct bit_code* get_bit_code(int i);

struct node{
	char root;
	struct node* taah_tree;
	struct node* ti_tree;
};

typedef struct node* morse_tree_t;

void morse_tree_init(void);

void morse_tree_free(void);

char morse_code_to_char(char* morse_code);

char* char_to_morse_code(char c);

#endif