/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0
#define INITIAL_NODE_SIZE_USE 0

/* For convenience */
#define INIT_PARAM_OFFSET 9 //Initial node maintainence parameter offset

/* Parameter choice */
#define T_DEGREE 100
#define NODE_SIZE 500


/* Function Prototypes(main) */
char* B_tree_create(void);
int B_tree_split_child(char*, char*);

/* Function Prototypes (supplementary) */
char* initialize_node(bool, size_t);
size_t get_node_capacity(char*);
size_t get_node_use(char*);
int skip_child_ptr(char**);
int skip_single_key(char**);
bool node_is_leaf(char* node);


/* Just some definitions:
 * Key refers to strlength(int), string(char*) and string counter(int)
 * Block refers to ptr(char*) and key pair 
 * ptr(char*) points to a child node
 * */

int main(int argc, char** argv) {

    printf("bool, size_t and int is size %zu, %zu and %zu\n", sizeof(bool), sizeof(size_t), sizeof(int));
    
    /* Initialize tree */
    char* tree_root = B_tree_create();
    

    /* Free the root node */
    free(tree_root);
    return 0;
}

/* Function that takes no input. It outputs the root node of a B-tree(char*) */
char* B_tree_create(void) {
    char* root = initialize_node(true, NODE_SIZE);
    return root;
}


/* Function that takes as input a non-full internal node x and full child node y of x. 
 * The function splits the child into two and adjusts x so that it has an extra child. */
int B_tree_split_child(char* node_x, char* node_y) {

    /* Create new child node z (sibling of y) */
    char* child_y = node_y;
    char* child_z = initialize_node(node_is_leaf(node_y), NODE_SIZE);

    /* Find midpoint of node_y to copy to node_z */


    return 1;
}








/* Function that takes as input (is_leaf[boolean], node_size[size_t]). The function ouputs an empty node (char*) */
char* initialize_node(bool is_leaf, size_t node_size) {

    /* Dynamically allocate memory for node */
    char* node = (char*)malloc(NODE_SIZE * sizeof(char));
    char* tmp = node;

    /* Initialize the housekeeping variables for the node */
    *(bool*)(tmp) = is_leaf; tmp += sizeof(bool); // Initialize leaf parameter 
    *(size_t*)(tmp) = INITIAL_NODE_SIZE_USE; tmp += sizeof(size_t); // Initialize curr_node_size_use parameter 
    *tmp = '\0'; // End node with null-byte

    return node;

}

/* Function that takes a node(char*) as input. The function outputs true 
 * if node is a leaf node, false (0) if node is not a leaf node */
bool node_is_leaf(char* node) {
    return *(bool*)(node);
}


/* Function that takes a node(char*) as input. The function outputs the 
 * the curr_node_size(size_t) used, which can be used as an offset */
size_t get_curr_node_use(char* node) {
    node += sizeof(bool); // add offset to skip is_leaf boolean
    return *(size_t*)(node);
}

/* Function that takes no input and returns the size of initial parameters used for node housekeeping. */
size_t get_init_param_offset(void) {
    return sizeof(bool) + sizeof(size_t); 
}

/* Function that takes a double pointer node and offsets pointer to node by one block */
int skip_single_block(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* offset pointer to node by one block */
    char** tmp_ptr = node_ptr;
    skip_child_ptr(tmp_ptr); // Skip ptr
    skip_single_key(tmp_ptr); // Skip key
    
    return 1;
}

/* Function that takes a double pointer node as input and offsets pointer to node by ptr (child node) */
int skip_child_ptr(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* offset by ptr */
    node_ptr += sizeof(char*);
    return 1;
}

/* Function that takes a double pointer node as input and offsets pointer to node 
 * by a single key */
int skip_single_key(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    
    /* Offset by strlength(int), string(tmp_length + 1), and str counter(int) */
    int tmp_str_length = *(int*)(*node_ptr); *node_ptr += sizeof(int); 
    *node_ptr += tmp_str_length + 1 + sizeof(int);

    return 1;
}

/* Function that takes a double pointer node as input and offsets pointer to 
 * node by initial housekeeping parameters. */
int skip_initial_parameters(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    /* Offset by initial node housekeeping paramters */
    *node_ptr += get_init_param_offset();
    return 1;
}
