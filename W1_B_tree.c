/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define INITIAL_NODE_BYTES 200
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0
#define INITIAL_NODE_USE 0

/* For convenience */

/* Parameter choice */
#define T_DEGREE 100



/* Function Prototypes */
char* initialize_empty_node(int, bool);
char* B_tree_create(void);
char* skip_ptr_block(char**);
char* get_child(char*, int);


/* Plan:    
 * Create empty tree
 * B-tree search
 * B-tree split child
 * B-tree insert and B-tree insert non-full
 * Not yet: B-tree delete key
 *
 * */

/* Problems:
 * Need to think of node structure: num_keys (probably not using), is_leaf (bool is size_t 4, same as int)
 *
 *
 *
 *
 * */

/* Node structure: is_leaf(boolean), number of keys(int), degree of t(int), node_size, node_use, [ptr, block alternating sequence] 
 * where block has structure: string length(int), string(char*), counter(int)
 * */

int main(int argc, char** argv) {
    //printf("bool true and false is size %zu and %zu\n", sizeof(true), sizeof(false));

    /* Initialze B-tree root */
    char* tree_root = B_tree_create();


    return 0;
}

/* Function that builds an empty B-tree (empty root node). */
char* B_tree_create() {
    char* root = initialize_empty_node(T_DEGREE, true);
    return root;
}

/* Function that creates an emtpy node that takes as input degree of T and boolean is_leaf */
char* initialize_empty_node(int degree_t, bool is_leaf) {

    /* Allocate memory for node */
    char* node = (char*)malloc(INITIAL_NODE_BYTES * sizeof(char));

    /* Initialize variables for the node: size = 20 bytes (21 including null-byte) */

    *(bool*)node = is_leaf; node += sizeof(bool); // Initialize is_leaf boolean
    *(int*)(node) = INITIAL_KEYS; node += sizeof(int); // Initialize number of keys
    *(int*)(node) = degree_t; node += sizeof(int); // Initialize degree of node 
    *(int*)(node) = INITIAL_NODE_BYTES ; node += sizeof(int); // Initialize node byte size  
    *(int*)(node) = INITIAL_NODE_USE; node += sizeof(int); // Initialize node byte use
    *node = '\0';

    return node;
}

/* Function that splits a full node in a B tree. It takes parent node and child node index as input */
char* B_tree_split_child(char* node, int index) {
    char* offset = node + 20; // 20 is the sizeof(initialization variables)
    
    /* Search for the ith child (same as read i - 1 keys) */
    char* child_index_address = get_child(node, index);
    char* child = *(char**)(child_index_address); // A bit uncertain: read as a pointer to a pointer to a char
    
    /* First 4 bytes of child is is_leaf boolean */
    int sibling_leaf = *(int*)(child);
    char* new_node = initialize_empty_node(T_DEGREE, sibling_leaf);


    return NULL; // Temporarily here 

}

/* Function that gets the index child of the node (skip first i - 1 keys) */
char* get_child(char* node, int index) {

    char* offset = node + 20; // 20 goes to first pointer block in the node

    /* Skip the first (index - 1) keys so offset pointer is pointer at node.child_index*/
    for (int i = 0; i < index - 1; i++) {
       skip_ptr_block(&offset);
    }

    return offset; // return offset, which now points to the node.child_index 
}

/* Function that skips the current pointer block and changes the offset ptr accordingly */
char* skip_ptr_block(char** offset_ptr) {
    if (**offset_ptr == '\0') {
        printf("Not supposed to happen; empty tree or at the end of ptr block alternating sequence.\n");
        return NULL;
    }

    /* Skip first child pointer, and skip the block (strlength, string and counter) */
    *offset_ptr += sizeof(char*);
    int tmp_str_length = *(int*)(*offset_ptr); (*offset_ptr) += sizeof(int);
    (*offset_ptr) += tmp_str_length + 1 + sizeof(int); // skip string including null-byte and integer counter 

    return NULL;
}
