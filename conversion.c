#include "conversion.h"

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

static char space_morse_code[2]=
		{3,0};

static struct bit_code morse_bit_code[4]=
{
	{.nb_bits=2, .bit_code=0x00}, 	// fin d'un caractère morse
	{.nb_bits=2, .bit_code=0x01},	// tih
	{.nb_bits=4, .bit_code=0x07},	// taah
	{.nb_bits=2, .bit_code=0x00}, 	// espace
};

static morse_tree_t morse_tree;

const struct bit_code* get_bit_code(int i){
	return &morse_bit_code[i];
}

static morse_tree_t morse_tree_create(void){
	morse_tree_t tree = 
		(morse_tree_t)kmalloc(sizeof(struct node),GFP_KERNEL);

	tree->root=-1;
	tree->one=NULL;
	tree->zero=NULL;

	return tree;
}

static void morse_tree_free_aux(morse_tree_t tree){
	if(tree->zero!=NULL){
		morse_tree_free_aux(tree->zero);
	}
	if(tree->one!=NULL){
		morse_tree_free_aux(tree->one);
	}
	kfree(tree);
}

void morse_tree_free(void){
	if(morse_tree!=NULL){
		morse_tree_free_aux(morse_tree);
	}
}

static int morse_tree_add_morse_char(morse_tree_t* ptree,int morse_char,char c){

	int count=morse_bit_code[morse_char].nb_bits;
	unsigned char bits = morse_bit_code[morse_char].bit_code;
	morse_tree_t tree=*ptree;

	while (count>0){
		if (bits&0x01){
			if (tree->one==NULL){
				tree->one=morse_tree_create();
			}
			tree=tree->one;
		}
		else{
			if (tree->zero==NULL){
				tree->zero=morse_tree_create();
			}
			tree=tree->zero;
		}
		count--;
		bits>>=1;
	}
	if (morse_char==0){
		tree->root=c;
		return 1;
	}
	*ptree=tree;
	return 0;
}

static void morse_tree_add_char(char* morse_code,char c){
	morse_tree_t tree=morse_tree;
	int i=0;
	while(!morse_tree_add_morse_char(&tree,morse_code[i],c)){
		i++;
	}
}

char convert_bit_to_char_online(int bit){
	static morse_tree_t tree;
	switch(bit){
		case RESET:
			tree=morse_tree;
			return 0;
			break;
		case 1:
			if (tree->one==NULL){
				return tree->root;
			}
			else{
				tree=tree->one;
				return 0;
			}
			break;
		case 0:
			if (tree->zero==NULL){
				return tree->root;
			}
			else{
				tree=tree->zero;
				return 0;
			}
			break;
		default:
			return -1;
	}
}



void morse_tree_init(void){
	char c;
	unsigned char i; 
	morse_tree=morse_tree_create();
	c='A';
	for (i=0;i<26;i++){
		morse_tree_add_char(letter_morse_code[i],c);
		c++;
	}
	c='0';
	for(i=0;i<10;i++){
		morse_tree_add_char(digit_morse_code[i],c);
		c++;
	}
	morse_tree_add_char(space_morse_code,' ');
}


char* char_to_morse_code(char c){
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
