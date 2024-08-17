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
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0
#define INITIAL_NODE_SIZE_USE INIT_PARAM_OFFSET

/* For convenience */
#define INIT_PARAM_OFFSET 9 //Initial node maintainence parameter offset
#define NODE_MID_SIZE (NODE_SIZE/ 2.0)
#define STR_SMALLER -1
#define POSITIVE 10 // This is just any integer, so it returns something 
#define ALLOCATE_OVERHEAD 8

/* Parameter choice */
// #define T_DEGREE 100
#define NODE_SIZE 300
#define WORDS_NUM 100000 // Parameter to control how many words to get from text file 


/* Function Prototypes(main) */
char* B_tree_create(void);
int B_tree_split_child(char*, char*);
int B_tree_insert(char**, const char*);
int B_tree_insert_nonfull(char*, const char*);
int B_tree_search(char*, char*, const char*);

/* More important Function Prototypes */
size_t skip_single_block(char**);
size_t skip_child_ptr(char**);
size_t skip_single_key(char**);
size_t move_ptr_to_ptr(char**, char*);
size_t skip_initial_parameters(char**);
bool node_is_leaf(char*);
size_t skip_block_from_start(char**, int);
size_t skip_key_to_key(char**);
size_t move_mid_node(char**);
int update_node_use(char*, size_t);
int compare_current_string_move(char**, size_t*, char*, const char*);
int compare_second_string_move(char** node_ptr, char*node, size_t* offset_ptr, char* tmp_array, 
                          const char* str_cmp);
int compare_current_string(char* node, char* tmp_array, const char* str_cmp);
int compare_middle_string(char* node, char* block, char* tmp_array, const char* str_cmp);


/* Function Prototypes (supplementary) */
char* initialize_node(bool, size_t);
size_t get_max_block_size(void);
size_t get_node_capacity(char*);
size_t get_node_use(char*);
size_t get_init_param_offset(void);
size_t get_single_key_size(char*);
int get_skip_str_length(char**);
int increment_block_counter(char*);
char* get_child_node(char*);


void print_current_block(char* node, char* node_start, size_t offset);
void check_valid_pointer(char* node);
void print_size(size_t size);
void print_node(char*);
void print_node_lexigraphic_check(char* node);
void print_node_strings(char* node);
void print_node_value_address(char* node);
void print_key_string(char* key);

/* Function Prototypes for text processing */



/* Function Prototypes for testing only */
void print_split_working(void);
void print_non_full_insert_working(void);
void print_node_create_successful(void);

void print_node_address(char* node);
void print_current_block_address(char* node);


/* Possible functions that are not done */
// get_index(char*); //This function would require a change in node structure


/* Just some definitions:
 * Key refers to strlength(int), string(char*) and string counter(int)
 * Block refers to ptr(char*) and key pair 
 * ptr(char*) points to a child node
 * */


/* Some B-tree functions:
 * Tree search, Create empty tree, Split child, Insert key, Insert key non-full, delete key */

/* Problems: Note that if node == nullbyte probably isn't needed in print current block */




static size_t memory_usage = 0;

int main(int argc, char** argv) {
    
    FILE* file;
    char word[100]; int counter = 1;
    
    char word_list[WORDS_NUM][MAX_STRING_BYTES];

    /* Variables for measuring time */
    struct timespec prec_start, prec_end;
    double elapsed1, elapsed2;


    file = fopen("wordstream.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    /* Initialize tree */
    char* tree_root = B_tree_create();
    

    
    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {

        /* Only store strings that aren't too large */
        if (strlen(word) < MAX_STRING_BYTES) {
            strcpy(word_list[counter - 1], word);
            counter++;
        }

        /* Store the specified number of strings only */
        if (counter > WORDS_NUM) {
            break;
        }
    }


    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i ++) {
        printf("Word to insert in tree is %s\n", word_list[i]);
        B_tree_insert(&tree_root, word_list[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed1 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)

    

    printf("Beginning binary tree word_list searches\n");
    /* Search for the words in the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i++) {
        if(!B_tree_search(tree_root, word, word_list[i])) {
            /* Word not found for some reason */
            assert(0);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed2 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)
    

    printf("Inserting all the strings took %.9f seconds. Searching all the strings took %.9f seconds\n", elapsed1, elapsed2);
    printf("The total memory usage was %zu bytes\n", memory_usage);

    /* Free the root node */
    free(tree_root);
    fclose(file);

    return 0;
}

/* Function that takes no input. It outputs the root node of a B-tree(char*) */
char* B_tree_create(void) {
    char* root = initialize_node(true, NODE_SIZE); 
    return root;
}

/* Function that takes the root node as input, a temporary heap array, and search string as input. The function outputs whether 
 * 1 if string is found in B-tree and 0 vice-versa */
int B_tree_search(char* node, char* array_store, const char* str) {
    char* tmp = node; int store;
    size_t offset = skip_initial_parameters(&tmp);

    int key_index = 1;
   
    /* If stop at first block */
    if ((store = compare_current_string(node, array_store, str)) >= 0) {
        if (store == 0) {
            return 1;
        }
        else if (node_is_leaf(node)){
            return 0;
        }
        else {
            return B_tree_search(*(char**)(node + INIT_PARAM_OFFSET), array_store, str);
        }
    /* Do not stop at first block */
    } else {
        while((store = compare_second_string_move(&tmp, node, &offset, array_store, str)) < 0) 
            key_index++;

        if (store == 0) {
            return 1;
        }
        else if (node_is_leaf(node)){
            return 0;
        }
        else {
            return B_tree_search(*(char**)(tmp), array_store, str);
        }
    }
    
    return 1;

}



/* Function that takes as input a non-full internal node x and full child node y of x. 
 * The function splits the child into two and adjusts x so that it has an extra child. */
/* Note that we assume that the parent node can fit at least one maximum size block, so it
 * is not full */

// Node_x is the root, node_y is the child
int B_tree_split_child(char* node_x, char* node_y) {


    /* Create new child node z (sibling of y). Note that node_x is the parent node. node_y and node_z are the child nodes */
    // char* child_y = node_y;
    char* child_z = initialize_node(node_is_leaf(node_y), NODE_SIZE);

    /* Find midpoint of node_y to copy to node_z */
    char* mid_ptr = node_y;

    size_t y_mid_offset = move_mid_node(&mid_ptr); //middle key of y_node

    /* This is the middle key of the node_y */
    int tmp_length = *(int*)mid_ptr; // y_child middle key length
    
    
    size_t key_size = sizeof(int) + tmp_length + 1 + sizeof(int);
    size_t insertion_offset = key_size + sizeof(char*) + sizeof(char*); // Insertion offset to insert in node_x. (block + ptr offset)


    /* Move tmp_x ptr to node_y child address */
    /* Insert middle key of y into x, and shift all blocks of node_x to the right */
    char* tmp_x = node_x;
    size_t key_offset = move_ptr_to_ptr(&tmp_x, node_y) + sizeof(char*); // Go past node_y address in x, to store middle key

    /* Making sure that the pointer finds the correct child node */
    assert(*(char**)tmp_x == node_y);


    /* Refer to Cormen: Move W onwards and leave space for ptr, S, ptr */
    memmove(tmp_x + insertion_offset, tmp_x + sizeof(char*), get_node_use(node_x) + 1 - key_offset);

    // Store key from y to x  
    tmp_x += sizeof(char*); *(int*)tmp_x = tmp_length; // printf("length is %d, should be %d\n", *(int*)(tmp_x), tmp_length);
    tmp_x += sizeof(int); //skip first ptr, key, second pointer // Not supposed to be here I think
    memcpy(tmp_x, mid_ptr + sizeof(int), tmp_length + 1);
    tmp_x += tmp_length + 1; 
    *(int*)tmp_x = *(int*)(mid_ptr + sizeof(int) + tmp_length + 1); 
    tmp_x += sizeof(int); // skip counter
    

    /* Storing child_z ptr into node_x */
    *(char**)tmp_x = child_z;
    update_node_use(node_x, get_node_use(node_x) + key_size + sizeof(char*));

    
    /* Delete middle key in y and move RHS keys to child_z */
    /* Clear memory of middle key and set null-byte */
//    memset(mid_ptr, 0xFF, key_size); 

    *mid_ptr = '\0'; // 

    mid_ptr += key_size; y_mid_offset += key_size;

    /* Move following blocks of node_y to child_z including the null-byte */
    memmove(child_z + get_init_param_offset(), mid_ptr, get_node_use(node_y) + 1 - y_mid_offset);
//    memset(mid_ptr, 0xFF, get_node_use(node_y) + 1 - y_mid_offset);

    update_node_use(child_z, get_node_use(child_z) + (get_node_use(node_y) - y_mid_offset));
    update_node_use(node_y, y_mid_offset - key_size);


    return 1;
}


/* Function that takes a char* subtree root node and key as input. 
 * The function inserts the key into the correct position of the tree */


// Current problem, forgot to update the new root of the tree 
int B_tree_insert(char** root_ptr, const char* str) {
    
    /* Node will be full if we assume a MAX STRING BYTES */
    if (get_node_use(*root_ptr) + 1 + get_max_block_size() > NODE_SIZE) {
        
        
        char* prev_root = *root_ptr; 

        /* allocate s as the new root */
        char* s = initialize_node(false, NODE_SIZE);
        *root_ptr = s; // The new node becomes the new root
        char* tmp = s;
        skip_initial_parameters(&tmp);


        /* S is the new root. First child ptr is node */
        *(char**)tmp = prev_root;
        update_node_use(s, get_node_use(s) + sizeof(char*));
        *(tmp + sizeof(char*)) = '\0'; //Set null-byte at end of new node 
        

        B_tree_split_child(s, prev_root);
        B_tree_insert_nonfull(s, str);

    } else { 

        B_tree_insert_nonfull(*root_ptr, str);
    }

    return 1;
}

/* Function that takes a non-full char* subtree, creates the key, and 
 * inserts it in the correct position */

// WRONG ATM, need to insert key ptr in this order

int B_tree_insert_nonfull(char* node, const char* str) {
    // int index = get_index();
    



    // Create dummy variable and keep track of tmp pointer 
    char *tmp = node; size_t offset = 0;
    offset += skip_initial_parameters(&tmp); //skip initial parameters for tmp 
    

    int str_length = strlen(str);

    int flag = 0; int store; 
    size_t block_size;

    // Create a temporary array rather than dynamic memory allocation (use stack for better
    // performance)
    char tmp_array[MAX_STRING_BYTES];
    
    /* Find the correct position of where to insert key.*/

    /* Compare the first string. Update counter if str match */
    if ((store = compare_current_string(node, tmp_array, str)) >= 0) {

        flag = 1;
        if (store == 0) {
            /* Exit function after inserting string / updating */
            increment_block_counter(tmp);
            return POSITIVE;
        }
    }

    /* Compare every second string, update counter if match. (pointer updated each time) */
    if (!flag) {

        /* Comparing every second string */
        while((store = compare_second_string_move(&tmp, node, &offset, tmp_array, str)) <= 0) {

            /* Repeating String */
            if (store == 0) {
                increment_block_counter(tmp);
                return POSITIVE;
            }
        }
    }


    // Skip child pointers when node is leaf
    if (node_is_leaf(node)) {

        /* For this case, we need to add a child pointer at the start and end */
        if (get_node_use(node) == INIT_PARAM_OFFSET) {
            
            /* Shift everything to the right to fit the new block */
            block_size = sizeof(char*) + sizeof(int) + str_length + 1 + sizeof(int) + sizeof(char*);


            memmove(tmp + block_size, tmp, get_node_use(node) + 1 - offset);
            *(char**)(tmp) = NULL; // castes tmp to double pointer

            tmp += sizeof(char*);
            *(int*)tmp = str_length; tmp += sizeof(int); // Store strlength in key 
            memcpy(tmp, str, str_length + 1); // Store string inside node 

            tmp += str_length + 1;
            *(int*)tmp = INITIAL_COUNT; tmp += sizeof(int);
            *(char**)(tmp) = NULL;

        } else {
            /* Shift everything to the right to fit the new block */
            block_size = sizeof(char*) + sizeof(int) + str_length + 1 + sizeof(int);


            memmove(tmp + block_size, tmp, get_node_use(node) + 1 - offset);
            *(char**)(tmp) = NULL; // castes tmp to double pointer

            tmp += sizeof(char*);
            *(int*)tmp = str_length; tmp += sizeof(int); // Store strlength in key 
            memcpy(tmp, str, str_length + 1); // Store string inside node 

            tmp += str_length + 1;
            *(int*)tmp = INITIAL_COUNT; 
            // tmp += sizeof(int); // This is not needed I think
        }

        
        /* Update currently used space in the node */
        update_node_use(node, get_node_use(node) + block_size);

        return POSITIVE;

    } else {
        
        char* child = get_child_node(tmp);
        
        /* Split the child node if full after inserting key */
        if (get_node_use(child) + get_max_block_size() + 1 > NODE_SIZE) {

            B_tree_split_child(node, child);


            /* If str is larger than current key in parent, shift tmp by one block */
            if (compare_middle_string(node, tmp, tmp_array, str) < 0) {
                skip_single_block(&tmp);

                child = get_child_node(tmp);
            } 
        }
        return B_tree_insert_nonfull(child, str);
    }
}


/* Function that takes char** node_ptr as input, where *node_ptr points 
 * to the start of the node. The function moves *node_ptr to the middle key 
 * of the node. */


// Need to fix, stop at key or ptr
size_t move_mid_node(char** node_ptr) {


    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* Make a copy to modify (not saved) */
    char* tmp = *node_ptr;

   
    /* Variables to keep track of offset of closest key to mid point */
    size_t min_offset, min_distance; // size_t tmp_min; // tmp_min not being used
    size_t prev_distance, curr_distance;

    /* Go to first and second key for prev_end_offset and curr_start_offset respectively */
    size_t prev_start_offset = 0; size_t prev_key_size;
    prev_start_offset += get_init_param_offset();
    prev_start_offset += sizeof(char*);

    /* Size of previous key to get the index */
    prev_key_size = get_single_key_size(*node_ptr + prev_start_offset);
    
    /* The node is not sufficiently full to perform a node-split */
    if (prev_start_offset + prev_key_size + sizeof(char*) + get_max_block_size() >= get_node_use(*node_ptr)) {
        printf("Node size parameter is too small, unable to do child split...\n");
        assert(0);
    }
    
    
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
    *node_ptr += min_offset;

    return min_offset;
}





/* Function that takes as input (is_leaf[boolean], node_size[size_t]). The function ouputs an empty node (char*) */
char* initialize_node(bool is_leaf, size_t node_size) {

    /* Dynamically allocate memory for node */
    char* node = (char*)malloc(node_size * sizeof(char));
    if (!node) {
        fprintf(stderr, "Failed to initialize node...\n");
        return NULL;
    }

    char* tmp = node;

    /* Initialize the housekeeping variables for the node */
    *(bool*)(tmp) = is_leaf; tmp += sizeof(bool); // Initialize leaf parameter 
    *(size_t*)(tmp) = INITIAL_NODE_SIZE_USE; tmp += sizeof(size_t); // Initialize curr_node_size_use parameter 
    *tmp = '\0'; // End node with null-byte
    
    memory_usage += node_size * sizeof(char);
    memory_usage += ALLOCATE_OVERHEAD;

    return node;

}

/* Function that takes a node(char*) as input. The function outputs true 
 * if node is a leaf node, false (0) if node is not a leaf node */
bool node_is_leaf(char* node) {
    return *(bool*)(node);
}


/* Function that takes a node(char*) as input. The function outputs the 
 * the curr_node_size(size_t) used, which can be used as an offset */
size_t get_node_use(char* node) {
    node += sizeof(bool); // add offset to skip is_leaf boolean
    return *(size_t*)(node);
}

/* Function that takes no input and returns the size of initial parameters used for node housekeeping. */
size_t get_init_param_offset(void) {
    return sizeof(bool) + sizeof(size_t); 
}

/* Function that takes no input and returns the maximum size of a block. (Based on MAX_STRING_BYTES key and ptr) */
size_t get_max_block_size(void) {
    /* String length integer + String with null-byte + counter + child_ptr */
    return (sizeof(int) + MAX_STRING_BYTES + 1 + sizeof(int) + sizeof(char*));
}

/* Funtion that takes a pointer to the start of a block in some node as input. The function assumes the pointer
 * currently points to the start of a block and does not modify the pointer. The 
 * function outputs the strlength of the key in the current block. */
int get_str_length(char* node) {
    if (!node) {
        fprintf(stderr, "Null pointer input");
        return -1;
    }

    /* Skip initial parameters, the child pointer and returns the length of the string */
    int tmp_length = *(int*)(node + sizeof(char*));

    return tmp_length;
}


/* Function that takes a char** node_ptr that points to the start of some block
 * as input. The function outputs the length of the current block string and 
 * moves the pointer to the string */

// Skipped in wrong order, should be key then child ptr I think

int get_skip_str_length(char** node_ptr) {
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return -1;
    }

    *node_ptr += sizeof(char*); // Skip the child pointer

    int tmp_length = *(int*)*node_ptr;
    (*node_ptr) += sizeof(int);

    return tmp_length;

}

/* Function that takes a double pointer node and offsets pointer to node by one block */

// Skipped in wrong order, should be key then child ptr
size_t skip_single_block(char** node_ptr) {
    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    
    size_t tmp_offset = 0;

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

/* Function that takes char** node_ptr and char** node_y_ptr as input, and moves 
 * *node_ptr pointer to the start of node_y address */

// Assumption here is that *node_ptr currently points to the start of the node
size_t move_ptr_to_ptr(char** node_ptr, char* node_to) {
    /* Sanity Check */
    if (!node_ptr || !node_to) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }
    check_valid_pointer(*node_ptr);


    size_t node_used = get_node_use(*node_ptr); 
    size_t tmp_size = 0;

    tmp_size += skip_initial_parameters(node_ptr);
    
    /* Move *node_ptr to point to node_to */
    char* tmp = *(char**)(*node_ptr); // *node_ptr is the first element. char** is 
    // a pointer to the pointer. Dereferencing gives the address 
 
    while(tmp != node_to) {
        /* If at last child ptr of the node, quit the function and return the offset */
        if (tmp_size + sizeof(char*) > node_used) { 

            return tmp_size; 
        }

        tmp_size += skip_single_block(node_ptr);
        tmp = *(char**)(*node_ptr);
    }

    return tmp_size;
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

/* Function that takes a node and size_t node_size as input. 
 * The function updates the current node use to new_size */
int update_node_use(char* node, size_t new_size) {
    if (!node) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    node += sizeof(bool);
    *(size_t*)node = new_size;

    
    return 1;    
}

/* Function that takes a char* node as input. char* node currently points to a block
 * The function outputs the address of the child node in the block. */
char* get_child_node(char* node) {
    return *(char**)node;
}



// THIS FUNCTION IS NOT BEING USED RN
/* Function that compares the string in the current block with str_cmp. It assumes 
 * *node_ptr is pointing to the start of some block
 * The function outputs -1 if string in block smaller than str_cmp, 0 if equal
 * and +1 if string in block is larger than str_cmp. 
 * */
int compare_current_string_move(char** node_ptr, size_t* offset_ptr, char* tmp_array, const char* str_cmp) {
    
    if (!node_ptr || !tmp_array) {
        fprintf(stderr, "Null pointer...\n");
        return -5;
    }

    int tmp_length = get_skip_str_length(node_ptr);
    *offset_ptr += sizeof(char*) + sizeof(int); // add child_ptr and str_length offset

    memcpy(tmp_array, *node_ptr, tmp_length + 1);

    *node_ptr += tmp_length + 1 + sizeof(int); // Skip the string and the counter 
    *offset_ptr += tmp_length + 1 + sizeof(int); // add string and counter offset 

    return strcmp(tmp_array, str_cmp);
    
}


/* Function that compares the string in the second block with strcmp. It 
 * *node_ptr is currently pointer to the start of the first block.
 * The function outputs -1 if string in block smaller than str_cmp, 0 if equal 
 * and +1 if string in block is larger than str_cmp;
 * */


// Current problem when a new string is stored at the end of an array byte
int compare_second_string_move(char** node_ptr, char* node, size_t* offset_ptr, char* tmp_array, 
                          const char* str_cmp) {

    if (!node_ptr || !tmp_array) {
        fprintf(stderr, "Null input...\n");
        return -5;
    }

    /* This condition is required for the start, where only a single pointer*/
    if (*offset_ptr + sizeof(char*) >= get_node_use(node)) {
        printf("Currently at the last block (compare_second_string_move)");
        return POSITIVE;
    }
    

    /* Move *node_ptr to next block */
    int tmp_length = get_skip_str_length(node_ptr); // skip child_ptr and strlength and store strlength

    *node_ptr += tmp_length + 1 + sizeof(int); // Skip string and counter 
    *offset_ptr += sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int); // Move offset forward one block
    

    /* Currently at the second block. Check if this block has a key. Already at the last block
     * we don't modify this */
    if (*offset_ptr + sizeof(char*) >= get_node_use(node)) {
        return POSITIVE;
    }

    // This is equivalent to the above, but the above is safer I think
   // if (*(*node_ptr + sizeof(char*)) == '\0') {
   //     printf("Currently at last block(compare_second_string_move)\n");
   //     return POSITIVE;
   // }

    tmp_length = get_str_length(*node_ptr);
    

    /* Copy the string in the second block to tmp array */
    memcpy(tmp_array, *node_ptr + sizeof(char*) + sizeof(int), tmp_length + 1);

    return strcmp(tmp_array, str_cmp);
}

/* Function that takes a node, array from stack, and string to compare as input. 
 * The function outputs < 0 if string in block smaller than str_cmp, 0 if equal, 
 * > 0 if string in block is larger than str_cmp. NOTE THAT THIS FUNCTION DOESN'T
 * AlTER THE PARENT NODE_PTR
 * */
int compare_current_string(char* node, char* tmp_array, const char* str_cmp) {
    if (!node || !tmp_array) {
        fprintf(stderr, "Emtpy pointer input...\n");
        return POSITIVE;
    }
    
    // THis shouldn't even work...
    /* If start of current string is nullbyte, should only happen in the start. When node has no keys */
    if (get_node_use(node) == INITIAL_NODE_SIZE_USE) {
        return POSITIVE;
    }



    /* Get the string from current block and store in temporary array */
    int tmp_length = get_str_length(node + INIT_PARAM_OFFSET);
    memcpy(tmp_array, node + INIT_PARAM_OFFSET + sizeof(char*) + sizeof(int), tmp_length + 1);

    /* return comparison results of two strings */
    return strcmp(tmp_array, str_cmp);
}

//THIS FUNCTION IS STILL BEING TESTED
/* Function that compares the string in the middle of some block */
int compare_middle_string(char* node, char* block, char* tmp_array, const char* str_cmp) {
    if (!block|| !tmp_array || !node) {
        fprintf(stderr, "Emtpy pointer input...\n");
        return POSITIVE;
    }
    
    // THis shouldn't even work...
    /* If start of current string is nullbyte, should only happen in the start. Node has no keys */
    if (get_node_use(node) == INITIAL_NODE_SIZE_USE) {
        return POSITIVE;
    }

    /* Get the string from current block and store in temporary array */
    int tmp_length = get_str_length(block);
    memcpy(tmp_array, block + sizeof(char*) + sizeof(int), tmp_length + 1);

    
    /* return comparison results of two strings */
    return strcmp(tmp_array, str_cmp);
}

/* Function that takes a char* node pointer. The function assumes the node points
 * to the start of the block. The function increments the counter of part if the key 
 * by 1 (+= 1) */
int increment_block_counter(char* node) {
    if (!node) {
        fprintf(stderr, "Null input...\n");
        return 0;
    }

    /* Skip past child pointer, str_length, string so the pointer points to counter */
    node += sizeof(char*);
    int tmp_length = *(int*)node; node += sizeof(int) + tmp_length + 1;
    
    *(int*)(node) += 1; //Increment node key counter by 1

    return 1;
}

/* Function that takes a ptr to a node starting at a block. It prints the str length, string and counter without 
 * modifying the node. It assumes that the current block is a non-emtpy block */
void print_current_block(char* node, char* node_start, size_t offset) {
    check_valid_pointer(node);

    if (get_node_use(node_start) == INIT_PARAM_OFFSET) {
        printf("Current node is emtpy, nothing to print\n");
        return;
    }

    if (offset + sizeof(char*) >= get_node_use(node_start)) {
        printf("Currently node is at the end\n");
        return; 
    }

    node += sizeof(char*);

    // End of node. I don't think this is needed. The prior if condition is the same 
    if (*node == '\0') {
        printf("Currently at end of node (print_current_block)\n");
        return;
    }

    char word[MAX_STRING_BYTES + 1];
    int tmp_length = *(int*)node; node += sizeof(int);


    memcpy(word, node, tmp_length + 1); node += tmp_length + 1;
    int tmp_counter = *(int*)node;

    printf("Block string length is %d, Block string is %s, Block string counter is %d\n", tmp_length, word, tmp_counter);
    return;
}

/* Sanity Check functions */

/* Function that checks whether the node input is emtpy */
void check_valid_pointer(char* node) {
    if (!node) {
        fprintf(stderr, "Node is emtpy\n");
        return;
    }
    return;
}

/* Function that takes a pointer to the start of the key as input. 
 * The function outputs the related key data of the key. */
void print_key_string(char* key) {
    check_valid_pointer(key);

    int stored_length, stored_counter;
    char word[MAX_STRING_BYTES];

    char* ptr = key;
    
    stored_length = *(int*)ptr; ptr += sizeof(int);
    memcpy(word, ptr, stored_length + 1); ptr += stored_length + 1;
    stored_counter = *(int*)ptr; 

    printf("Printing Key...\n");
    fprintf(stdout, "Length of string: %d\n", stored_length);
    fprintf(stdout, "String: %s\n", word);
    fprintf(stdout, "String counter: %d\n", stored_counter);


}

/* Print all blocks from the start of the node */
void print_node(char* node) {
    check_valid_pointer(node);
    printf("\n\nPrinting the whole node\n\n");
    

    size_t node_space_used = get_node_use(node);
    printf("Find initial node size: %zu\n", node_space_used);

    char* ptr = node;
    size_t offset = 0; offset += skip_initial_parameters(&ptr);

    /* Temp variatbles to output data to stdout */
    int stored_length, stored_counter; 
    char stored_string[MAX_STRING_BYTES];


    
    // Note that memcpy is used to indicate that overlapping memory isn't the intention 
    while(offset < node_space_used) {
        printf("%p  ", (void*)*(char**)ptr);
        ptr += sizeof(char*); offset += sizeof(char*);

        /* Last child ptr */
        if (offset >= node_space_used) {
            printf("End of node\n");
            return;
        }
        
        /* Store the strings in temporary variables */
        stored_length = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);

        /* Storing string and String counter*/
        memcpy(stored_string, ptr, stored_length + 1);
        ptr += stored_length + 1; offset += stored_length + 1;

        stored_counter = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);
        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length:%d  ", stored_length);
        fprintf(stdout, "String:%s  ", stored_string);
        fprintf(stdout, "Counter:%d  ", stored_counter);

    }
    printf("\n");
    
}


/* Print all blocks from the start of the node */
void print_node_lexigraphic_check(char* node) {
    check_valid_pointer(node);
    printf("\n\nPrinting the whole node\n\n");
    

    size_t node_space_used = get_node_use(node);

    if (node_space_used == INIT_PARAM_OFFSET) {
        return;
    }


    printf("Find initial node size: %zu\n", node_space_used);

    char* ptr = node;
    size_t offset = 0; offset += skip_initial_parameters(&ptr);

    /* Temp variatbles to output data to stdout */
    int stored_length, stored_counter; 
    char stored_string[MAX_STRING_BYTES];


    /* Null character has the smallest ASCII value */
    char prev_store[MAX_STRING_BYTES] = "\0";
    
    // Note that memcpy is used to indicate that overlapping memory isn't the intention 
    while(offset < node_space_used) {
        printf("%p  ", (void*)*(char**)ptr);
        ptr += sizeof(char*); offset += sizeof(char*);

        /* Last child ptr */
        if (offset >= node_space_used) {
            printf("End of node\n");
            assert(offset == node_space_used);
            return;
        }
        
        /* Store the strings in temporary variables */
        stored_length = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);

        /* Storing string and String counter*/
        memcpy(stored_string, ptr, stored_length + 1);
        // printf("Previous string is %s, current string is %s\n", prev_store, stored_string);





        assert(strcmp(prev_store, stored_string) <= 0);
        strcpy(prev_store, stored_string);


        ptr += stored_length + 1; offset += stored_length + 1;

        stored_counter = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);
        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length:%d  ", stored_length);
        fprintf(stdout, "String:%s  ", stored_string);
        fprintf(stdout, "Counter:%d  ", stored_counter);


    }
    printf("\n");
    
}

/* Print all blocks from the start of the node */
void print_node_strings(char* node) {
    check_valid_pointer(node);
    size_t node_space_used = get_node_use(node);
    char* ptr = node;
    size_t offset = 0; offset += skip_initial_parameters(&ptr);


    /* Temp variatbles to output data to stdout */
    int stored_length, stored_counter; 
    char stored_string[MAX_STRING_BYTES];

    
    // Note that memcpy is used to indicate that overlapping memory isn't the intention 
    while(offset < node_space_used) {
        printf("%p  ", (void*)*(char**)ptr);
        ptr += sizeof(char*); offset += sizeof(char*);

        /* Last child ptr */
        if (offset >= node_space_used) {
            printf("End of node\n");
            return;
        }
        
        /* Store the strings in temporary variables */
        stored_length = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);

        /* Storing string and String counter*/
        memcpy(stored_string, ptr, stored_length + 1);
        ptr += stored_length + 1; offset += stored_length + 1;

        stored_counter = *(int*)ptr; ptr += sizeof(int); offset += sizeof(int);
        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length of string: %d\n", stored_length);
        fprintf(stdout, "String: %s\n", stored_string);
        fprintf(stdout, "String counter: %d\n", stored_counter);

    }
    
}



/* The following functions are used for string processing from the textfile */





/* The following functions are used purely for testing */

/* Function prints B_tree_split child is working */
void print_split_working(void) {
    printf("Node split child working \n");
    return;
}

/* Function prints that B_tree_insert_nonfull function is working */
void print_non_full_insert_working(void) {
    printf("Node B-tree insert non-full working\n");
    return;
}

/* Function that prints if node_created successfully */
void print_node_create_successful(void) {
    printf("Node successfully created \n");
    return;
}


/* Function that prints the node address of the input */
void print_node_address(char* node) {
    printf("The address of the node is: %p\n", (void*)node);
    return;
}

/* Funciton that prints the address value of the node pointer */
void print_node_value_address(char* node) {
    printf("The address stored where node points to is %p\n", *(char**)node);
    return;
}


/* Function that prints the address of the current block */
void print_current_block_address(char* node) {
    print_node_address(*(char**)node);
    return;
}

/* Function that prints the size */
void print_size(size_t size) {
    printf("The size offset is %zu\n", size);
    return;
}
