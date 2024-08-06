/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0
#define INITIAL_NODE_SIZE_USE 0

/* For convenience */
#define INIT_PARAM_OFFSET 9 //Initial node maintainence parameter offset
#define NODE_MID_SIZE (NODE_SIZE / 2.0)

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
size_t get_init_param_offset(void);
size_t get_single_key_size(char*);
size_t skip_single_block(char**);
size_t skip_child_ptr(char**);
size_t skip_single_key(char**);
size_t skip_initial_parameters(char**);
bool node_is_leaf(char*);
size_t skip_block_from_start(char**, int);
size_t skip_key_to_key(char**);


/* Just some definitions:
 * Key refers to strlength(int), string(char*) and string counter(int)
 * Block refers to ptr(char*) and key pair 
 * ptr(char*) points to a child node
 * */


/* Some B-tree functions:
 * Tree search, Create empty tree, Split child, Insert key, Insert key non-full, delete key */


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
    char* mid_ptr = node_y;

    move_mid_node(&mid_ptr);

    return 1;
}


/* Function that takes char** node_ptr as input, where *node_ptr points 
 * to the start of the node. The function moves *node_ptr to the middle key 
 * of the node. */
int move_mid_node(char** node_ptr) {

    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* Make a copy to modify */
    char* tmp = *node_ptr;
   
    /* Variables to keep track of offset of closest key to mid point */
    size_t min_offset, tmp_min, min_distance;
    size_t prev_distance, curr_distance;

    /* Go to first and second key for prev_end_offset and curr_start_offset respectively */
    size_t prev_start_offset, prev_key_size;
    prev_start_offset += get_init_param_offset();
    prev_start_offset += sizeof(char*);

    /* Size of previous key to get the index */
    prev_key_size = get_single_key_size(*node_ptr + prev_start_offset);
    
    /* Move tmp pointer to second key of the node */
    size_t curr_start_offset = skip_block_from_start(&tmp, 1);
    curr_start_offset += skip_child_ptr(&tmp);
    
    /* Initialize minimum key offset */
    if ((prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1))) 
    < (curr_distance = fabs(NODE_MID_SIZE - curr_start_offset))) {
        min_distance = prev_distance;
        min_offset = prev_start_offset;
    } else {
        min_offset = curr_start_offset; 
        min_distance = curr_distance;
    }


    /* Go through blocks of the node and find the key_offset that is closest to NODE_MID_SIZE */
    while(prev_distance > curr_distance) {

        /* Get key size of curr_start_offset and update prev_start_offset */
        prev_key_size = get_single_key_size(tmp);
        prev_start_offset = curr_start_offset;

        /* Update curr_start offset to next key */
        curr_start_offset += skip_key_to_key(&tmp);
        
        /* Cacluate the new distances between the keys and the node mid point */
        prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
        curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

        /* Update the min_offset and min_distance accordingly for next while loop */
        if (prev_distance < min_distance) {
            min_distance = prev_distance;
            min_offset = prev_start_offset;
        }

        if (curr_distance < min_distance) {
            min_distance = curr_distance;
            min_offset = curr_start_offset;
        }     
    } 

    /* Update *node_ptr to start of the key that is closest to the NODE_MID_SIZE */
    *node_ptr = *node_ptr + min_offset;

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
size_t skip_single_block(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    
    size_t tmp_offset;

    /* offset pointer to node by one block */
    tmp_offset += skip_child_ptr(node_ptr); // Skip ptr
    tmp_offset += skip_single_key(node_ptr); // Skip key
    
    return tmp_offset;
}

/* Function that takes a double pointer node as input and offsets pointer to node by ptr (child node) */
size_t skip_child_ptr(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* offset by ptr */
    *node_ptr += sizeof(char*);
    return sizeof(char*);
}

/* Function that takes a double pointer node as input and offsets pointer to node 
 * by a single key */
size_t skip_single_key(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    
    /* Offset by strlength(int), string(tmp_length + 1), and str counter(int) */
    int tmp_str_length = *(int*)(*node_ptr); *node_ptr += sizeof(int); 
    *node_ptr += tmp_str_length + 1 + sizeof(int);

    return sizeof(int) + tmp_str_length + 1 + sizeof(int);
}

/* Function that takes a char *node starting at the key as input and calculate the size of the current key */
size_t get_single_key_size(char *node) {
    if (!node) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    int tmp_str_length = *(int*)node;
    return sizeof(int) + tmp_str_length + 1 + sizeof(int);
}

/* Function that takes a double pointer node as input and offsets pointer to 
 * node by initial housekeeping parameters. */
size_t skip_initial_parameters(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    /* Offset by initial node housekeeping paramters */
    *node_ptr += get_init_param_offset();
    return get_init_param_offset();
}

/* Function that takes a double pointer node and index as input. The function 
 * offsets the ptr to node by the number of index blocks to skip and outputs 
 * the offset size */
size_t skip_block_from_start(char** node_ptr, int index) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }


    /* Move pointer past initial parameters and update offset */
    size_t tmp_offset = 0;
    tmp_offset = skip_initial_parameters(node_ptr);

    /* Move pointer past index blocks and calculate total offset */
    for (int i = 0; i < index; i++) {
        tmp_offset += skip_single_block(node_ptr);
    }
    
    return tmp_offset; 
}

/* Function that takes a char** node_ptr as input and assumes *node_ptr 
 * points to the start of the key. This moves the *node_ptr to the start 
 * of the next key and outputs the offset. */
size_t skip_key_to_key(char** node_ptr) {
    /* Sanity Checks */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* Move to next key and output offset */
    size_t next_key_offset = 0;
    next_key_offset += skip_single_key(node_ptr);
    next_key_offset += skip_child_ptr(node_ptr);
    return next_key_offset;
}
