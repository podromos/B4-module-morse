#include <stdlib.h>
#include <stdio.h>

static char* letter_morse_code[26]=
	{
		".-", 	//A
		"-...",	//B
		"-.-.",	//C
		"-..",	//D
		".",	//E
		"..-.",	//F
		"--.",	//G
		"....",	//H
		"..",	//I
		".---",	//J
		"-.-",	//K
		".-..",	//L
		"--",	//M
		"-.",	//N
		"---",	//O
		".--.",	//P
		"--.-",	//Q
		".-.",	//R
		"...",	//S
		"-",	//T
		"..-",	//U
		"...-",	//V
		".--",	//W
		"-..-",	//X
		"-.--",	//Y
		"--..",	//Z
	}; 

static char* digit_morse_code[10]=
	{
		".----",
		"..---",
		"...--",
		"....-",
		".....",
		"-....",
		"--...",
		"---..",
		"----.",
		"-----",
	};

struct node{
	char root;
	struct node* taah_tree;
	struct node* ti_tree;
};

typedef struct node* morse_tree_t;

static morse_tree_t morse_tree_create(void){
	morse_tree_t tree = 
		(morse_tree_t)malloc(sizeof(struct node));

	tree->root='0';
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
	free(tree);
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



int main(){
	char c,cf;
	char* tmp;
	printf("Hello, World!\n");
	
	morse_tree=morse_tree_init(morse_tree_create());
	
	for (c='A';c<='Z';c++){
		tmp=char_to_morse_code(c);
		if (tmp==NULL){
			printf( "error char = %c \n",c);
		}
		cf=morse_code_to_char(morse_tree,tmp);
		if (cf==0){
			printf( "error str = %s \n",tmp);
		}
		printf( "%c = %c \n",c,cf);
	}
	morse_tree_free(morse_tree);
	printf( "Goodbye, World!\n");
	return 0;
}