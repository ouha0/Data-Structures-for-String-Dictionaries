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
#define INIT_VAR_SIZE 28

/* Parameter choice */
#define T_DEGREE 100



/* Function Prototypes */
char* initialize_node(int, bool, int);
char* B_tree_create(void);
char* skip_ptr_block(char**);
char* get_child(char*, int);
void read_ptr_block(char*, char*, int);
char* skip_ptr_block_start(char**, int);
void move_ptr_block(char**, char**, int); 
void skip_initialization_var(char**);
char* init_variable_offset(char*);
void move_current_sequence(char**, char**, char**, bool, int);
void move_final_pointer(char**, char**, char**, bool, int);
void update_node_capacity(char*, size_t);
size_t get_node_capacity(char*);
size_t get_node_use(char*);
bool check_update_node_size(char**, size_t);
void update_key_count(char**, int);

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

    printf("The size of size_t is: %zu bytes\n", sizeof(size_t));

    printf("The size of int is: %zu bytes\n", sizeof(int));

    return 0;
}

/* Function that builds an empty B-tree (empty root node). */
char* B_tree_create() {
    char* root = initialize_node(T_DEGREE, true, INITIAL_KEYS);
    return root;
}

/* Function that creates an emtpy node that takes as input degree of T and boolean is_leaf */
char* initialize_node(int degree_t, bool is_leaf, int num_keys) {

    /* Allocate memory for node */
    char* node = (char*)malloc(INITIAL_NODE_BYTES * sizeof(char));
    char* tmp = node;


    /* Initialize variables for the node: size = 28 bytes (29 including null-byte) */
    *(bool*)(tmp) = is_leaf; tmp += sizeof(bool); // Initialize is_leaf boolean
    *(int*)(tmp) = num_keys; tmp += sizeof(int); // Initialize number of keys
    *(int*)(node) = degree_t; tmp += sizeof(int); // Initialize degree of node 
    *(size_t*)(tmp) = INITIAL_NODE_BYTES ; tmp += sizeof(size_t); // Initialize node byte size  
    *(size_t*)(tmp) = INITIAL_NODE_USE; tmp += sizeof(size_t); // Initialize node byte use
    *tmp = '\0';

    return node;
}

/* Function moves a pointer at start of node past the initialization variables to ptr key alternating sequence */
// For maintainability
void skip_initialization_var(char** tmp_ptr){
    (*tmp_ptr) += INIT_VAR_SIZE;
}

/* Function that splits a full node in a B tree. It takes parent node and child node index as input */
/* Note that node is the parent, child_y is the "left child", child_z is the "right child" */
char* B_tree_split_child(char* node, int index) {
    char* offset = node; // 20 is the sizeof(initialization variables)
    skip_initialization_var(&offset);

    /* Search for the ith child (same as read i - 1 keys) */
    char* child_index_address = get_child(node, index);
    char* child_y = *(char**)(child_index_address); // LHS child 
    
    /* First 4 bytes of child is is_leaf boolean */
    bool sibling_leaf = *(bool*)(child_y);
    char* new_child_z = initialize_node(T_DEGREE, sibling_leaf, T_DEGREE - 1); // RHS child

    /* Move last (T_degree - 1) keys from child_y to child_z and the last pointer */
    move_ptr_block(&new_child_z, &child_y, T_DEGREE - 1); // Move the pointer key sequence from y to z

    /* Update the child_y current key count correctly */
    update_key_count(&child_y, T_DEGREE - 1);

    /* Update the parent node, move mid to end ptr block sequences to the right, and 
     * push middle y_node key up to parent */
    
    parent_node_push_up(); // working on this now. Note that the last block of y has not been deleted yet


    return NULL; // Temporarily here 

}

/* Function that gets the index child of the node (skip first i - 1 keys) */
char* get_child(char* node, int index) {

    char* offset = node; // 20 goes to first pointer block in the node
    skip_initialization_var(&offset);

    /* Skip the first (index - 1) keys so offset pointer is pointer at node.child_index*/
    for (int i = 0; i < index - 1; i++) {
       skip_ptr_block(&offset);
    }

    return offset; // return offset, which now points to the node.child_index 
}

/* Function that skips the current pointer block  sequence and changes the offset ptr accordingly */
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

/* Function that skips the initial variables, and the next "index" number of ptr block sequences. It updates offset to the appropriate address */
char* skip_ptr_block_start(char** offset_ptr, int index) {
    skip_initialization_var(offset_ptr); // Skip the initial variables
    
    /* Skip the first "index" pointer block sequences */
    for (int i = 0; i < index; i++) {
        skip_ptr_block(offset_ptr);
    }
    return NULL;
}

/* Function that moves the first "end" keys from y to z. (It deletes the ptr and blocks accordingly) */
// Note that *y_child_ptr and *z_child_ptr points to start of child nodes

void move_ptr_block(char** y_child_ptr, char** z_child_ptr, int end) {
    bool y_is_leaf = *(bool*)(*y_child_ptr);
    char* y_child_cmp = *y_child_ptr; char* z_child_cmp = *z_child_ptr;
    
    /* Skips first T_DEGREE - 1 keys of y_child_cmp pointer. Moves z_child_cmp pointer to first ptr_key */
    skip_ptr_block_start(&y_child_cmp, T_DEGREE - 1);
    skip_initialization_var(&z_child_cmp);

    char* tmp_y = y_child_cmp; // save start address of the y_child ptr block sequence to be removed
    
    /* Copy ptr key from y to z, and delete from y */
    move_current_sequence(&z_child_cmp, z_child_ptr, &y_child_cmp, y_is_leaf, 0);


    // This is not the last block of y, haven't deleted middle key yet 
    // *tmp_y = '\0'; //Store last byte of y_child_ptr (initial pointer) as null-byte for first iteration

    for (int i = 1; i < end; i++) {
        move_current_sequence(&z_child_cmp, z_child_ptr, &y_child_cmp, y_is_leaf, i);
    }
    move_final_pointer(&z_child_cmp, z_child_ptr, &y_child_cmp, y_is_leaf, end);


    *z_child_cmp = '\0'; // Store last byte of z_child (pointer updated) as null-byte when iteration finished

}

/* Function that offsets char* pointer by initial parameters of node. It returns a char* pointer that pointers to ptr key alternating sequence */
char* init_variable_offset(char* node) {
    return node += INIT_VAR_SIZE;
}

/* Function that moves the ptr block from y_child_ptr_del to z_child_ptr, clear the memory for y and updates z_child and y_child_ptr_del pointer */
// NOT DONE YET: If array size too small, double the size for z_child. Seems like the function needs array bytes used by child_z. It requires the pointer to the start of the node...
// Wrong, when we change the node size, its a different memory address, so we no longer can use z_child_ptr, the memory is gone. 
// Rather than saving memory addresses, we should use offsets (size_t...)
void move_current_sequence(char** z_child_ptr, char** z_child_init, char** y_child_ptr_del, bool y_is_leaf, 
                           int block_offset) {
    int tmp_length;
    
    /* Double byte size of child z node if cannot fit children pointer */
    /* If doubled, update z_child_ptr to the correct block(address) based on the index 
     * since the memory is reallocated
     * */
    if(check_update_node_size(z_child_init, sizeof(char*))) {
        // reset *z_child_ptr to start and update *z_child_ptr to correct_block offset index address 
        *z_child_ptr = *z_child_init; 
        skip_ptr_block_start(z_child_ptr, block_offset);
    }

    /* if not leaf node */
    if (!y_is_leaf) {

        /* Copy pointer from y to z  */
        *(char**)(*z_child_ptr) = *y_child_ptr_del; // Copy pointer from y_child to z_child and delete pointer
        
    }
    else { // if y is leaf (z is also leaf)
        
        /* Copy pointer from y to z, end y with null-byte, clear memory of y and update y and z pointer position */
        *(char**)(*z_child_ptr) = NULL; // Since leaf node, z_child should have no children 
        
        /* Sanity check: confirm that child y has no children */
        if (*(char**)(*y_child_ptr_del) != NULL) {
            fprintf(stderr, "y child pointer should be a NULL pointer\n");
            return;
        }
    }

    /* Copy block data from y to z, and clear memory in y */
    memset(*y_child_ptr_del, 0xFF, sizeof(char*)); // Set memory of y to 0xFF to indicate unused memory
    *z_child_ptr += sizeof(char*); *y_child_ptr_del += sizeof(char*); //update offset so y and z points to block
    

    tmp_length = *(int*)(*y_child_ptr_del); 

    /* Double size of z_child byte array if cannot fit block (strlength, string, counter and null-byte) 
     * If node memory is reallocated, update z_child_cmp memory address accordingly based on block_offset index 
     * Also offset by char* since the child pointer of z_child was read 
     * */
    if(check_update_node_size(z_child_init, sizeof(int) + tmp_length + 1 + sizeof(int) + 1)) {
        *z_child_ptr = *z_child_init;
        skip_ptr_block_start(z_child_ptr, block_offset);
        *z_child_ptr += sizeof(char*);
    }

    *(int*)(*z_child_ptr) = tmp_length;
    memset(*y_child_ptr_del, 0xFF, sizeof(int)); // Clear y memory

    *z_child_ptr += sizeof(int); *y_child_ptr_del += sizeof(int);
    
    /* Copy the rest of the block: the string with null byte and the counter and clear y_child memory */
    memcpy(*z_child_ptr, *y_child_ptr_del, tmp_length + 1 + sizeof(int));
    memset(*y_child_ptr_del, 0xFF, tmp_length + 1 + sizeof(int));
    *z_child_ptr += tmp_length + 1 + sizeof(int); *y_child_ptr_del += tmp_length + 1 + sizeof(int); //offset to next ptr block sequence
}

/* Function that updates the node size if required */
bool check_update_node_size(char** node_ptr, size_t data_size) {
    size_t node_capacity = get_node_capacity(*node_ptr);
    size_t current_node_use = get_node_use(*node_ptr);

    bool enlarged = false;

    /* while data and null-byte cannot fit, double the node size */
    while(current_node_use + data_size + 1 > node_capacity) {
        /* reallocate double the memory */
        char* new_node = realloc(*node_ptr, DOUBLE_SIZE * node_capacity);
        if (!new_node) {
            fprintf(stderr, "New Memory allocation unsucessful!\n");
            return false;
        }

        /* Update double pointer address so main function has new node address and update node_capacity */
        *node_ptr = new_node;
        update_node_capacity(*node_ptr, node_capacity * DOUBLE_SIZE);
        
        enlarged = true;
    }

    return enlarged; 

}

/* Function that takes a char** node_ptr and changes the node capacity to a new size */
void update_node_capacity(char* node, size_t new_capacity) {
    char *tmp = node;
    tmp += sizeof(bool) + sizeof(int) * 2;
    *(size_t*)(tmp) = new_capacity;
}

/* Function that takes a char* node as input and outputs the node_capactiy */
size_t get_node_capacity(char* node) {
    /* Temporary variable to apply offset */
    char* tmp = node;
    tmp += sizeof(bool) + sizeof(int) * 2; //move pointer offset to read node_capacity 
    
    size_t capacity = *(size_t*)(tmp);
    return capacity;
}

/* Function that takes a char* node as input and outputs the current_node_use */
size_t get_node_use(char* node) {
    /* Temporary variable to apply offset */
    char* tmp = node;
    tmp += sizeof(bool) + sizeof(int) * 2 + sizeof(size_t);
    size_t node_use = *(size_t*)(tmp);
    
    return node_use;
}

/* Function that copies last child pointer from y node to z. Also clears memory for y */
void move_final_pointer(char** z_child_ptr, char** z_child_init, char** y_child_ptr_del,
                        bool y_is_leaf, int final) {

    /* Check whether z_child_node has space to store a char* child pointer*/
    if(check_update_node_size(z_child_init, sizeof(char*))) {
        // reset *z_child_ptr to start and update *z_child_ptr to correct_block offset index address 
        *z_child_ptr = *z_child_init; 
        skip_ptr_block_start(z_child_ptr, final);
    }

    /* if not leaf node */
    if (!y_is_leaf) {

        /* Copy pointer from y to z  */
        *(char**)(*z_child_ptr) = *y_child_ptr_del; // Copy pointer from y_child to z_child and delete pointer
        
    }
    else { // if y is leaf (z is also leaf)
        
        /* Copy pointer from y to z, end y with null-byte, clear memory of y and update y and z pointer position */
        *(char**)(*z_child_ptr) = NULL; // Since leaf node, z_child should have no children 
        
        /* Sanity check: confirm that child y has no children */
        if (*(char**)(*y_child_ptr_del) != NULL) {
            fprintf(stderr, "y child pointer should be a NULL pointer\n");
            return;
        }
    }
    if (*(char*)(*y_child_ptr_del + sizeof(char*) + 1) != '\0') {
        fprintf(stderr, "Last byte of y_child isn't null-byte, it seems like something is wrong...\n");
        return;
    } 

    /* Copy block data from y to z, and clear memory in y */
    memset(*y_child_ptr_del, 0xFF, sizeof(char*) + 1); // Set memory of y to 0xFF to indicate unused memory including last null-byte
    *z_child_ptr += sizeof(char*); *y_child_ptr_del += sizeof(char*); //update offset so y and z points to block
}


/* Function is given double pointer to start of node, and updates the current number of keys in the node */
void update_key_count(char** node_start, int key_count) {
    *(int*)(*node_start + sizeof(bool)) = key_count;
    return;
}

/* Some saved code 
 *
 *        Copy pointer from y to z, end y with null-byte, clear memory of y and update y and z pointer position 
 *
 *        *(char**)(*z_child_ptr) = *y_child_ptr_del; *(char*)(*y_child_ptr_del) = '\0'; // Copy pointer from y_child to z_child and delete pointer
 *        memset(*y_child_ptr_del + sizeof(char), 0xFF, sizeof(char*) - sizeof(char)); // Set memory of y to 0xFF to indicate unused memory
 *
 *
 *
 *
 * */
