/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

/* Global Variables */
#define MAX_STRING_BYTES 64

/* For convenience */
#define STR_SMALLER -1
#define POSITIVE 10 // This is just any integer, so it returns something 
#define LEFT_PTR_OFFSET 0
#define RIGHT_PTR_OFFSET sizeof(char*)
/* Data Volume */
#define WORDS_NUM 1000 // Parameter to control how many words to get from text file 


/* PROBLEMS:
 * DON'T USE STRCASECMP I THINK */

/* Function Prototypes */

/* Main Function Prototypes */
char* create_node(const char* str);
void binary_tree_insert(char** root_ptr, const char* str);
int binary_tree_search(char* node, const char* str);



/* Supplementary Function Prototypes */
void move_to_left_pointer(char** node_ptr);
void move_to_right_pointer(char** node_ptr);
int compare_key_string(char* n1, char* n2);
int compare_str_node(const char* str, char* n1);
void increment_counter(char* node);
void print_binary_node(char* node);




int main(int argc, char** argv) {


    FILE* file;
    char word[100]; int counter = 1;
    
    char word_list[WORDS_NUM][MAX_STRING_BYTES];
    


    file = fopen("wordstream.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }


    char* tree_root; 
    
    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {
        printf("The word is to insert is %s\n", word);
        strcpy(word_list[counter - 1], word);
        counter++;

        if (counter > WORDS_NUM) {
            break;
        }
    }

    printf("Beginning binary tree word insertions\n");
    /* Insert the words into the binary tree */
    for (int i = 0; i < counter - 1; i ++) {
        binary_tree_insert(&tree_root, word_list[i]);
    }


    printf("Beginning binary tree word_list searches\n");
    /* Search for the words in the binary tree */
    for (int i = 0; i < counter - 1; i++) {
        if(!binary_tree_search(tree_root, word_list[i])) {
            /* Word not found for some reason */
            assert(0);
        }
    }



    free(tree_root);
    fclose(file);
    return 0;
}


/* Function that takes a char** root_ptr where *root_ptr points to tree root. The function creates a node for "str", and inserts 
 * it at the appropriate position of the tree. */
void binary_tree_insert(char** root_ptr, const char* str) {
    char* y = NULL;
    char* x = *root_ptr;
    int store;

    while(x != NULL) {
        /* Save previous subroot */
        y = x;

        /* Compare keys to determine which child node to access */
        /* String smaller than current key */
        if ((store = compare_str_node(str, x)) <= 0) {

            /* If same string, increment counter for x */
            if (store == 0) {
                increment_counter(x);
            } else {
                x = *(char**)(x + LEFT_PTR_OFFSET);
            }
            
        /* When the string is larger than the key */
        } else {
            x = *(char**)(x + RIGHT_PTR_OFFSET);
        }
    }

    /* The parent of z is y now, not saved in this code setup */

    /* Create Z node and put it in the correct position. Update sub-parent child pointer node */
    char* z = create_node(str);
    if (y == NULL) {
        *root_ptr = z;
    } else if (compare_key_string(z, y) < 0) {
        *(char**)(y + LEFT_PTR_OFFSET) = z;
    } else {
        *(char**)(y + RIGHT_PTR_OFFSET) = z;
    }
}

/* Function that takes a root node and string to search as input. The function outputs
 * the node if the string is found. Otherwise, it outputs to stdout that string
 * was not found. */
int binary_tree_search(char* root, const char* str) {

    /* Node empty or key found  */
    if (root == NULL) {
        printf("String is not found\n");
        return 0;
    } 
    
    if (compare_str_node(str, root) == 0) {
        printf("String is found\n");
        print_binary_node(root);
        return 1;
    }
    
    /* Search children nodes */
    if (compare_str_node(str, root) < 0) {
        return binary_tree_search(*(char**)(root + LEFT_PTR_OFFSET), str);
    }
    else{
        return binary_tree_search(*(char**)(root + RIGHT_PTR_OFFSET), str);
    }
}



/* Function that takes a constant string as input. The function creates a node of the form
 * left child pointer, right child pointer and key. The key is defined as string length (integer), 
 * string (char*) and string counter (integer). The function outputs the node address of node */
char* create_node(const char* str) {
    /* Get length of string */
    int tmp_length = strlen(str);

    /* Dynamically allocate memory to new node */
    char* tmp = (char*)malloc(2 * sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int));
    
    /* Set left and right child pointer to NULL */
    *(char**)(tmp + LEFT_PTR_OFFSET) = NULL;
    *(char**)(tmp + RIGHT_PTR_OFFSET) = NULL;
    
    return tmp;
}

/* Function that takes a node_ptr and moves the position of *node_ptr so that it points to the left child pointer.
 * The function is just for convenient naming. Don't need to move the pointer */
void move_to_left_pointer(char** node_ptr) {
    return;
}

/* Function that takes a node_ptr and moves the position of *node_ptr so that it points to the right child pointer. */
void move_to_right_pointer(char** node_ptr) {
    *node_ptr += sizeof(char*);
    return;
}

/* Function that takes a node_ptr and moves the position of *node_ptr so that it points the the start of string 
 * outputs the string length of the key. */
int move_to_string(char** node_ptr) {
    /* Skip the left and right child pointers */
    *node_ptr += 2 * sizeof(char*);

    int tmp_length = *(int*)(*node_ptr); *node_ptr += sizeof(int);

    return tmp_length;
}

/* Function that takes two nodes as input, and compares the two strings lexigraphically. The function outputs <0 if n1 string 
 * is less than n2 string , 0 if n1 string and n2 string are equal, and >0 if n1 string is larger than n2 string. */
int compare_key_string(char* n1, char* n2) {
    char buffer_1[MAX_STRING_BYTES + 1];
    char buffer_2[MAX_STRING_BYTES + 1];

    int n1_length = move_to_string(&n1);
    int n2_length = move_to_string(&n2);

    memcpy(buffer_1, n1, n1_length + 1);
    memcpy(buffer_2, n2, n2_length + 1);


    return strcasecmp(n1, n2);
}


/* Function that takes a string and node as input, and compares the two strings (const string and string in node key)
 * lexigraphically. The function outputs <0 if constant string is less than n1 string , 0 if 
 * constant string and n1 string are equal, and >0 if constant string is larger than n1 string. */
int compare_str_node(const char* str, char* n1) {
    char buffer_1[MAX_STRING_BYTES + 1];

    int n1_length = move_to_string(&n1);
    memcpy(buffer_1, n1, n1_length + 1);


    return strcasecmp(str, n1);
}

/* Function that takes a char* node as input, and increases the counter by 1 */
void increment_counter(char* node){
    /* Skip child pointers */
    node += 2 * sizeof(char*);

    /* Move pointer to counter position */
    int tmp_length = *(int*)node; node += sizeof(int) + tmp_length + 1;
    
    /* Increase the counter by 1 */
    *(int*)node += 1;

}

/* Function that takes a node as input and outputs all relevant data */
void print_binary_node(char* node) {
    if (!node) {
        printf("The node is emtpy\n");
        assert(0);
    }

    /* Prints the address of the child pointers */
    // THIS MIGHT BE WRONG. NO GUARENTEE THAT CHILD POINTERS ARE NON-EMPTY
    printf("The left child pointer is %p. The right child pointer is %p\n", 
           *(char**)(node + LEFT_PTR_OFFSET), *(char**)(node + RIGHT_PTR_OFFSET));


    /* Prints the key of the node: length, string and counter */
    node += RIGHT_PTR_OFFSET + sizeof(char*);
    int tmp_length = *(int*)node; node += sizeof(int);

    char tmp_word[MAX_STRING_BYTES];
    memcpy(tmp_word, node, tmp_length + 1); node += tmp_length + 1;
    
    int tmp_counter = *(int*)node;

    printf("Length: %d String: %s Count: %d", tmp_length, tmp_word, tmp_counter);

    return;
}


