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
#define INITIAL_LEAF_NODE_SIZE_USE sizeof(bool) + sizeof(size_t) + sizeof(char*)
#define INITIAL_NONLEAF_NODE_SIZE_USE sizeof(bool) + sizeof(size_t)

/* For convenience */
#define INIT_PARAM_OFFSET 9 //Initial node maintainence parameter offset
#define NODE_MID_SIZE (INIT_PARAM_OFFSET + (NODE_SIZE - INIT_PARAM_OFFSET)/ 2.0)
// #define NODE_MID_SIZE (NODE_SIZE / 2.0)
#define STR_SMALLER -1
#define POSITIVE 10 // This is just any integer, so it returns something 
#define ALLOCATE_OVERHEAD 8

#define PRINT_TOGGLE 0
#define KEY_COUNT 1
#define CHECK_TOGGLE 1
#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000


/* Parameter choice */
// #define T_DEGREE 10
#define NODE_SIZE 512
#define WORDS_NUM TEN_MILLION // Parameter to control how many words to get from text file 
#define FILENAME "wordstream.txt"
// #define FILENAME "wikipedia_with_cap.txt"


/* Main function prototypes */
char* Bplus_create(void);
char* Bplus_split(char* parent, char* child, size_t parent_node_use, size_t child_node_use,
                  char* child_location, size_t child_location_offset);
int Bplus_insert(char**, const char*);
int Bplus_insert_nonfull(char*, const char*);
int Bplus_search(char*, char*, const char*);

void free_Bplus(char* root);
void free_array_list(char** word_list);


/* Supplementary main function prototypes */
size_t leaf_move_mid_node(char** node_ptr, size_t node_use);
size_t nonleaf_move_mid_node(char** node_ptr, size_t node_use);

int custom_strcmp(char** str_ptr, const char* str, int key_str_length);

/* Foundational Supplementary functions */
size_t leaf_skip_single_key(char** node_ptr);
size_t non_leaf_skip_single_key(char** node_ptr);
size_t nonleaf_skip_single_key(char** node_ptr);
size_t skip_child_ptr(char** node_ptr);


/* Supplementary quick function prototypes */
char* initialize_node(bool is_leaf, size_t node_size);
size_t get_node_use(char* node);
bool node_is_leaf(char* node);

size_t leaf_get_init_param_offset(void);
size_t nonleaf_get_init_param_offset(void);

char* get_next_leaf_node(char* node);
int update_node_use(char* node, size_t new_size);
size_t get_max_block_size(bool isleaf);
size_t leaf_get_doubleblock_size(int str_length);
size_t nonleaf_get_single_key_size(char* node);

size_t leaf_skip_block_from_start(char** node_ptr, int index);
size_t nonleaf_skip_block_from_start(char** node_ptr, int index);

size_t leaf_get_single_key_size(char* node);
size_t leaf_skip_initial_parameters(char** node_ptr);
size_t nonleaf_skip_initial_parameters(char** node_ptr);

size_t leaf_skip_single_block(char** node_ptr);
size_t nonleaf_skip_single_block(char** node_ptr);

size_t leaf_skip_key_to_key(char** node_ptr);
size_t nonleaf_skip_key_to_key(char** node_ptr);

int leaf_increment_block_counter(char* block);

int compare_current_string(char* block, char* tmp_array, const char* str_cmp);


int leaf_compare_second_string_move(char** node_ptr, size_t node_size, size_t* offset_ptr,
                                           char* tmp_array, const char* str_cmp);
int nonleaf_compare_second_string_move(char** node_ptr, size_t node_size, size_t* offset_ptr,
                                           char* tmp_array, const char* str_cmp);



/* Functions for printing and checking */

void nonleaf_print_node_lexigraphic_check(char* node);
void leaf_print_node_lexigraphic_check(char* node);


char* left_most_node(char* root);
void print_all_keys (char* mostleft_node);

/* Think about the node structure of B-trees and B+ trees. This will cause differences in the code... 
 * Also beware about the skipping functions you use. Internal nodes don't have the counter field */


// Note that the node structure is different for leaf nodes and internal nodes 

static size_t memory_usage = 0;
static int unique_key_counter = 0;
static int non_unique_key_counter = 0;
static size_t keys_processed = 0;
static int number_of_nodes = 0;
static double avg_node_use_ratio = 0;

int main(int argc, char** argv) {
    
    FILE* file;
    char word[100]; size_t counter = 1;
    int str_length; int flag = 0;
    
    // char word_list[WORDS_NUM][MAX_STRING_BYTES];
    /* Allocate memory for word list */

    /* Row allocation */
    char **word_list = (char**)malloc(sizeof(char*) * WORDS_NUM);
    if (!word_list) {
        printf("Invalid word_list allocation\n");
        return 1;
    }


    /* Variables for measuring time */
    struct timespec prec_start, prec_end;
    double elapsed1, elapsed2;


    file = fopen(FILENAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    /* Initialize tree */
    char* tree_root = Bplus_create();
    

    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {

        /* Only store strings that aren't too large */
        if ((str_length = strlen(word)) < MAX_STRING_BYTES) {
            
            word_list[counter - 1] = (char*)malloc(sizeof(char) * (str_length + 1));
            strcpy(word_list[counter - 1], word);
            counter++;
            
            /* Print read file word to stdout */
            //printf("%s ", word);
        }

        /* Store the specified number of strings only */
        if (counter > WORDS_NUM) {
            flag = 1; 
            printf("Enough word data: %zu words loaded, stop loading words from text file.\n", counter - 1);
            break;
        }
    }

    if (!flag) {
        printf("Not enough words in txt file, only loaded %zu words\n", counter - 1);
    }


    printf("Beginning B+-tree insertions\n\n\n");
    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i ++) {
        // printf("Word to insert in tree is %s\n", word_list[i]);
        Bplus_insert(&tree_root, word_list[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed1 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    printf("Beginning B+-tree word_list searches\n\n\n");
    /* Search for the words in the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i++) {
        if(!Bplus_search(tree_root, word, word_list[i])) {
            /* Word not found for some reason */
            assert(0);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed2 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    /* Print all nodes */
    char* left_most = left_most_node(tree_root);
    print_all_keys(left_most);


    printf("%d, SZX, B+-tree, node size parameter\n", NODE_SIZE);
    printf("%d, NUX, B+-tree, non-unique strings\n", non_unique_key_counter);
    printf("%.3f, INX, B+-tree, seconds to insert\n", elapsed1);
    printf("%.3f, SRX, B+-tree, seconds to search\n", elapsed2);
    printf("%zu, MUX, B+-tree, memory usage\n", memory_usage);
    printf("%d, UKX, B+-tree, unique strings\n", unique_key_counter);
    printf("%zu, KPX, B+-tree, keys processed\n", keys_processed);
    printf("%d, NNX, B+-tree, number of nodes\n", number_of_nodes);
    printf("%lf, NRX, B+-tree, average node fill ratio\n", avg_node_use_ratio / number_of_nodes);


    //printf("\n\n\n");
    //printf("For B-tree with node size %d:\n", NODE_SIZE);
    //printf("There are %d non-unique strings in the B-tree, there should be %zu non-unique strings (inserted strings)\n",
    //       non_unique_key_counter, counter - 1);
    //printf("Inserting %zu non-unique strings took %.3f seconds. Searching all the strings took %.3f seconds\n", counter - 1, elapsed1, elapsed2);
    //printf("The total memory usage was %zu bytes\n", memory_usage);
    //printf("There are %d unique keys in the B-tree\n", unique_key_counter);
    //printf("%zu keys were processed\n", keys_processed);
    //printf("There are %d nodes in the B-tree\n", number_of_nodes);
    //
    //if (CHECK_TOGGLE) {
    //    printf("The average node fill ratio is %lf\n", avg_node_use_ratio / number_of_nodes);
    //}


    printf("Cleaning up memory...\n");
    /* Free the whole tree node */
    free_Bplus(tree_root);
    /* Free the array list */
    free_array_list(word_list);

    

    fclose(file);

    return 0;
}



/* Main functions */

char* Bplus_create(void) {
    char* root = initialize_node(true, NODE_SIZE); 
    return root;
}


/* Function that takes the parent node, child node and the location of the child node pointer in the parent node as input. 
 * The function performs the tree split operation. (Note that this is different B-tree split)  */
char* Bplus_split(char* parent, char* child, size_t parent_node_use, size_t child_node_use,
                  char* child_location, size_t child_location_offset) {

    //size_t parent_node_use = get_node_use(parent);
    //size_t child_node_use = get_node_use(child);
    
    bool child_leaf = node_is_leaf(child);


    /* Create child right node */
    char* child_right = initialize_node(child_leaf, NODE_SIZE);
    size_t child_right_node_use = get_node_use(child_right);

    /* Find the middle key of the child node (to copy to the child_right node) */
    char* mid_child_ptr = child;
    size_t mid_child_offset;

    if (child_leaf) {
        mid_child_offset = leaf_move_mid_node(&mid_child_ptr, child_node_use);
        
    } else {
        mid_child_offset = nonleaf_move_mid_node(&mid_child_ptr, child_node_use);

    }


    /* Get information of middle key of child node */
    int tmp_length = *(int*)mid_child_ptr; 
    size_t key_size;

    /* When child node is leaf, child key is copied to parent node */
    // KEY size shoule remain the same I think, counter isn't copied to parent node !!
   // if (child_leaf) {
   //     key_size = sizeof(int) + tmp_length + 1 + sizeof(int);

   // } else {  // When child node isn't a leaf, node split is same as B-tree 
   //     key_size = sizeof(int) + tmp_length + 1;
   // }

    key_size = sizeof(int) + tmp_length + 1;

    size_t insertion_offset = key_size + sizeof(char*) + sizeof(char*); // insertion offset to insert middle block into parent node

    /* Offset for parent node */ 
    size_t key_offset = child_location_offset + sizeof(char*); // offset from start parent node to end of child_location pointer


    /* Refer to Cormen: Move W onwards and leave space for ptr, S, ptr */
    memmove(child_location + insertion_offset, child_location + sizeof(char*), parent_node_use + 1 - key_offset);
    child_location += sizeof(char*); 

    /* Copy middle key in child node to praent node. Note that parent node is never a leaf node, there is no counter field in the key */
    *(int*)child_location = tmp_length; child_location += sizeof(int);
    memcpy(child_location, mid_child_ptr + sizeof(int), tmp_length + 1); child_location += tmp_length + 1;
    *(char**)child_location = child_right; // Store right child pointer of parent node 


    /* Now we move half the child node keys to the new child node */
    if (child_leaf) {

        /* Add a null pointer */
        *(char**)(child_right + leaf_get_init_param_offset()) = NULL; 

        /* Copy keys from child node to child_right */
        memmove(child_right + leaf_get_init_param_offset() + sizeof(char*), mid_child_ptr, child_node_use + 1 - mid_child_offset);

        /* Update new child linked list pointer */
        *(char**)(child_right + nonleaf_get_init_param_offset()) = *(char**)(child + nonleaf_get_init_param_offset());
        /* Save pointer to next leaf node in child node before child_right node is modified */
        *(char**)(child + nonleaf_get_init_param_offset()) = child_right;


        /* Update node sizes*/
        update_node_use(child_right, child_right_node_use + (child_node_use - mid_child_offset) + sizeof(char*));
        update_node_use(child, mid_child_offset);
        
    /* IF child node is not a leaf node (internal node) */
    } else { /* Whether child node is leaf determines whether node is pushed up or copied like a B-tree */
        
        mid_child_ptr += key_size; mid_child_offset += key_size;
        memmove(child_right + nonleaf_get_init_param_offset(), mid_child_ptr, child_node_use + 1 - mid_child_offset);

        /* Update node sizes*/
        update_node_use(child_right, child_right_node_use + (child_node_use - mid_child_offset));
        update_node_use(child, mid_child_offset - key_size);
    }
    
    /* Update parent node use */
    update_node_use(parent, parent_node_use + key_size + sizeof(char*));
    
    
    ///* For checking: Make sure nodes are split correctly... */
    //printf("After node splits... where child is leaf? : %d \n", child_leaf );

    //nonleaf_print_node_lexigraphic_check(parent);

    //if (child_leaf) {
    //    leaf_print_node_lexigraphic_check(child);
    //    leaf_print_node_lexigraphic_check(child_right);
    //} else {
    //    nonleaf_print_node_lexigraphic_check(child);
    //    nonleaf_print_node_lexigraphic_check(child_right);
    //}

    return NULL;
}


/* Function that takes a non-full node, creates a new key, and 
 * inserts it in the correct position */
int Bplus_insert_nonfull(char* node, const char* str) {;
    size_t node_use, offset, block_size;
    int str_length, store, flag;
    
    char* tmp;

    char tmp_word[MAX_STRING_BYTES + 1];

    char* prev; int tmp_length;

    while(true) {
        node_use = get_node_use(node);
        tmp = node;
        flag = 1;


        // For checking 


        /* If the node is a leaf, place the key in the correct position */
        if (node_is_leaf(node)) {

            /* Checking */
            // leaf_print_node_lexigraphic_check(node);

            offset = leaf_skip_initial_parameters(&tmp);

            /* If initial leaf node is emtpy */
            if (node_use == INITIAL_LEAF_NODE_SIZE_USE) {


                str_length = strlen(str);
                block_size = sizeof(char*) + sizeof(int) + str_length + 1 + sizeof(int) + sizeof(char*);

                /* Shift the blocks of the node to the right */
                memmove(tmp + block_size, tmp, node_use + 1 - offset);
                
                /* Store the key into the node */
                *(char**)tmp = NULL; tmp += sizeof(char*);
                *(int*)tmp = str_length; tmp += sizeof(int);
                memcpy(tmp, str, str_length + 1); tmp += str_length + 1;
                *(int*)tmp = 1; tmp += sizeof(int);
                *(char**)tmp = NULL;

                
            } else { /* Initial leaf node is non-empty i.e. node_use > initial parameters (no need for extra pointer) */

                while (true) {
                    // Save previous pointer 
                    prev = tmp;
                    if (offset + sizeof(char*) == node_use) {
                        break;
                    }

                    /* Compare current key and move pointer to start of next block */
                    tmp += sizeof(char*); 
                    tmp_length = *(int*)tmp; 
                    tmp += sizeof(int);

                    /* Just checking */

                    /* Compare string and update pointer */
                    store = custom_strcmp(&tmp, str, tmp_length);

                    /* Stop if current key is smaller than string to insert */
                    if (store >= 0) {
                        /* Increment counter if existing string */
                        if (store == 0) {
                            *(int*)tmp += 1;
                            return POSITIVE;
                        }
                        break;

                    /* Update offset only for next key and tmp pointer */
                    } else {
                        offset += sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int);
                    }
                    /* Move pointer past counter */
                    tmp += sizeof(int);
                }
                /* Move to correct block position */
                tmp = prev;

                str_length = strlen(str);

                // assert(str_length >= 0);

                /* Shift everything to the right to fit the new block */
                block_size = sizeof(char*) + sizeof(int) + str_length + 1 + sizeof(int);


                memmove(tmp + block_size, tmp, node_use + 1 - offset);
                *(char**)(tmp) = NULL; // Set child ptrs to NULL 

                tmp += sizeof(char*);
                *(int*)tmp = str_length; tmp += sizeof(int); // Store strlength in key 
                memcpy(tmp, str, str_length + 1); // Store string inside node 

                tmp += str_length + 1;
                *(int*)tmp = INITIAL_COUNT; 
            }

            /* Update the node use, and quit loop to end the function */
            /* New key inserted */
            unique_key_counter++;
            update_node_use(node, node_use + block_size);
            

            break;


            /* When node is not leaf, find the child node */
        } else {
            // Note when equal, we move to the RHS of the matching key (hence also includes equality)
            // nonleaf_print_node_lexigraphic_check(node);
            // nonleaf_print_node_lexigraphic_check(node);


            offset = nonleaf_skip_initial_parameters(&tmp);


            while (true) {
                // Save previous pointer 
                prev = tmp;
                if (offset + sizeof(char*) == node_use) {
                    break;
                }

                /* Compare current key and move pointer to start of next block */
                tmp += sizeof(char*); 
                tmp_length = *(int*)tmp; tmp += sizeof(int);

                /* Just checking */

                /* Compare string and update pointer */
                store = custom_strcmp(&tmp, str, tmp_length);

                /* Stop if current key is smaller than string to insert */
                if (store > 0) {
                    break;

                /* Update offset only for next key and tmp pointer */
                } else {
                    offset += sizeof(char*) + sizeof(int) + tmp_length + 1;
                }
            }
            /* Move to correct block position */
            tmp = prev;
                

            /* Get correct child pointer node */
            char* child = *(char**)tmp; 
            // printf("child address is %p\n", child);

            /* Split child node if it is full (we assume child node is leaf for safer split) */

            size_t child_node_use;
            if ((child_node_use = get_node_use(child)) + get_max_block_size(true) + 1 > NODE_SIZE) {
                Bplus_split(node, child, node_use, child_node_use, tmp, offset);

                /* Shift by one block is str is larger or equal to current key in parent */
                if ((store = compare_current_string(tmp, tmp_word, str)) <= 0) {
                    nonleaf_skip_single_block(&tmp);
                }
            }
            /* Move to the appropriate child node for next iteration */ 
            node = *(char**)(tmp);
        }
    }

    return 1;
}


/* NOTE: This function has no been checked yet!!! */

/* The function inserts the key into the correct position of the tree */
int Bplus_insert(char** root_ptr, const char* str) {
   
    size_t prev_root_size = get_node_use(*root_ptr);
    bool is_leaf = node_is_leaf(*root_ptr);

    /* If current root is full */
    if (prev_root_size + get_max_block_size(is_leaf) + 1 > NODE_SIZE) {
        char* prev_root = *root_ptr;
        
        /* Create a new root */
        *root_ptr = initialize_node(false, NODE_SIZE);
        char* tmp = *root_ptr; 

        /* Skip initial parameters, add new child pointer and update node size */
        tmp += nonleaf_get_init_param_offset();
        *(char**)tmp = prev_root; 

        size_t root_node_use = nonleaf_get_init_param_offset() + sizeof(char*);
        update_node_use(*root_ptr, root_node_use);
        *(tmp + sizeof(char*)) = '\0';

        /* Perform node split on previously full node and then insert node */  
        Bplus_split(*root_ptr, prev_root, root_node_use, prev_root_size, 
                    tmp, nonleaf_get_init_param_offset());
        
        /* Now *root_ptr is not full */
        Bplus_insert_nonfull(*root_ptr, str);

    } else { // Insert as usual, since root node is not full
        
        Bplus_insert_nonfull(*root_ptr, str);
    }

    non_unique_key_counter++;

    return 1;
}


/* Function that takes the root node as input, a temporary heap array, and search string as input. The function outputs whether 
 * 1 if string is found in B+ tree and 0 vice-versa */
int Bplus_search(char* root, char* word_store, const char* str) {
    
    char* current = root; size_t curr_node_size; bool is_leaf;
    int store, flag;
    char* tmp; 
    size_t offset;

    while (current != NULL) {
        /* Get important properties of the node first */
        curr_node_size = get_node_use(current); 
        tmp = current;
        flag = 1;


        char* prev; int tmp_length, store;
        
        /* Depending on type of node, offsets are different */

        /* If node is a leaf */
        if (node_is_leaf(current)) {

            /* CHECKING */
            // leaf_print_node_lexigraphic_check(current);

            /* Skip the initial parameters */
            offset = leaf_skip_initial_parameters(&tmp);

            while (true) {
                // Save previous pointer 
                prev = tmp;
                if (offset + sizeof(char*) == curr_node_size) {
                    break;
                }

                /* Compare current key and move pointer to start of next block */
                tmp += sizeof(char*); 
                tmp_length = *(int*)tmp; 
                tmp += sizeof(int);

                /* Just checking */

                /* Compare string and update pointer */
                store = custom_strcmp(&tmp, str, tmp_length);

                /* Stop if current key is smaller than string to insert */
                if (store >= 0) {
                    /* Increment counter if existing string */
                    if (store == 0) {
                        // printf("word to search is %s, word currently stored is %s. They should the same :) \n", str, word_store);
                        return 1;
                    } else {
                        printf("String %s not found for some reason, this shouldn't be the case\n", str);
                        return 0;
                    }
                /* Update offset only for next key and tmp pointer */
                } else {
                    offset += sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int);
                }
                /* Move pointer past counter */
                tmp += sizeof(int);
            }
            /* Move to correct block position */
            tmp = prev;
            return 0;

        /* If node is not leaf */
        } else {
            
            /* CHECKING */
            // nonleaf_print_node_lexigraphic_check(current);

            /* Skip the initial parameters */
            offset = nonleaf_skip_initial_parameters(&tmp);

            while (true) {
                // Save previous pointer 
                prev = tmp;
                if (offset + sizeof(char*) == curr_node_size) {
                    break;
                }

                /* Compare current key and move pointer to start of next block */
                tmp += sizeof(char*); 
                tmp_length = *(int*)tmp; tmp += sizeof(int);

                /* Just checking */

                /* Compare string and update pointer */
                store = custom_strcmp(&tmp, str, tmp_length);

                /* Stop if current key is smaller than string to insert */
                if (store > 0) {
                    break;

                /* Update offset only for next key and tmp pointer */
                } else {
                    offset += sizeof(char*) + sizeof(int) + tmp_length + 1;
                }
            }
            /* Move to correct block position */
            current = *(char**)prev;
        }
    }
    return 0;
}


/* Function that takes the tree root as input and frees memory of all nodes */
void free_Bplus(char* root) {
    if (root == NULL)
        return;
    
    bool is_leaf = node_is_leaf(root);
    size_t offset;

    char* ptr = root; int tmp_length;
    if (is_leaf) {
        offset = leaf_skip_initial_parameters(&ptr);
    } else {
        offset = nonleaf_skip_initial_parameters(&ptr);
    }

    size_t node_space_used = get_node_use(root);

    /* Free each child ptr node */
    while(offset < node_space_used) {
        /* Recurse child ptr */
        free_Bplus(*(char**)(ptr));

        ptr += sizeof(char*); offset += sizeof(char*);

        if (offset >= node_space_used) {
            break;
        }
        
        /* Move pointer and offset to the next child ptr */
        tmp_length = *(int*)(ptr); 
        ptr += sizeof(int) + tmp_length + 1;
        offset += sizeof(int) + tmp_length + 1;

        if (is_leaf) {
            ptr += sizeof(int); offset += sizeof(int);
        }

    }

    /* Free the node */
    free(root);
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

    /* Initialize size of node and linked list pointer (if leaf node) */
    if (is_leaf) {
        *(size_t*)(tmp) = sizeof(bool) + sizeof(size_t) + sizeof(char*); 
        tmp += sizeof(size_t); *(char**)tmp = NULL;

    } else
        *(size_t*)(tmp) = sizeof(bool) + sizeof(size_t); 


    /* Note: Need to leave space for linked list pointer to next node */
   // if (is_leaf) {
   //     tmp += sizeof(size_t); // Initialize curr_node_size_use parameter 
   //     *(char**)tmp = NULL;
   // }


    // *tmp = '\0'; // End node with null-byte
    
    memory_usage += (node_size * sizeof(char));
    memory_usage += ALLOCATE_OVERHEAD;
    
    number_of_nodes++;
    return node;
}


/* Note that this function is not done yet. Need to have move_mid node leaf and nonleaf */
size_t leaf_move_mid_node(char** node_ptr, size_t node_use) {

    // size_t node_used = get_node_use(*node_ptr);

    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* Make a copy to modify (not saved) */
    char* tmp = *node_ptr;

    /* Variables to keep track of offset of closest key to mid point */
    size_t min_offset, min_distance; // size_t tmp_min; // tmp_min not being used
    double prev_distance, curr_distance;

    /* Go to first and second key for prev_end_offset and curr_start_offset respectively */
    size_t prev_key_size, curr_key_size;

    size_t prev_start_offset = leaf_skip_initial_parameters(&tmp);
    prev_start_offset += skip_child_ptr(&tmp);


    /* Size of previous key to get the index */
    prev_key_size = leaf_get_single_key_size(tmp);
    
    /* The node is not sufficiently full to perform a node-split (Just an approximation) */
    if (prev_start_offset + prev_key_size + sizeof(char*) + get_max_block_size(true) >= node_use) {
        printf("Node size parameter is too small, unable to do child split...\n");
        assert(0);
    }
    
    
    /* Move tmp pointer to second key of the node */
    size_t curr_start_offset = prev_start_offset + leaf_skip_single_key(&tmp);
    curr_start_offset += skip_child_ptr(&tmp);
    
    prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
    curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

    curr_key_size = leaf_get_single_key_size(tmp);

    /* Initialize minimum key offset */
    if (prev_distance < curr_distance) {
        printf("prev distance is %lf, curr distance is %lf\n", prev_distance, curr_distance);
        printf("Node size is most likely too small...\n");
        assert(0);

        min_distance = prev_distance;
        min_offset = prev_start_offset;

    } else {
        
        /* If second key is the last key, then node split probably shouldn't happen... */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_use) {
            printf("Something seems off... Node size probably too small... Doing a node split on 2 keys...\n");
            assert(0);
        }

        min_offset = curr_start_offset; 
        min_distance = curr_distance;
    }

    /* Go through blocks of the node and find the key_offset that is closest to NODE_MID_SIZE */
    while(prev_distance > curr_distance) {

        /* Get key size of curr_start_offset and update prev_start_offset */
        prev_key_size = curr_key_size;
        prev_start_offset = curr_start_offset;

        /* Update curr_start offset to next key */
        curr_start_offset += leaf_skip_key_to_key(&tmp);
        curr_key_size = leaf_get_single_key_size(tmp);

        /* Stop if current key is the last key. Can't node split when nothing on the 
         * right side of the middle */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_use)
        {
            // printf("For large enough node sizes, this shouldn't run...\n");
            break;
        }
        
        /* Cacluate the new distances between the keys and the node mid point */
        prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
        curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

        /* Update the min_offset and min_distance accordingly for next while loop */
        if ((prev_distance < min_distance)) {
            min_distance = prev_distance;
            min_offset = prev_start_offset;
        }

        if ((curr_distance < min_distance)) {
            min_distance = curr_distance;
            min_offset = curr_start_offset;
        }     
    } 

    /* Update *node_ptr to start of the key that is closest to the NODE_MID_SIZE */
    *node_ptr += min_offset;
    return min_offset;
}


size_t nonleaf_move_mid_node(char** node_ptr, size_t node_use) {

    // size_t node_used = get_node_use(*node_ptr);

    /* Sanity Check */
    if (!node_ptr) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    /* Make a copy to modify (not saved) */
    char* tmp = *node_ptr;

    /* Variables to keep track of offset of closest key to mid point */
    size_t min_offset, min_distance; // size_t tmp_min; // tmp_min not being used
    double prev_distance, curr_distance;

    /* Go to first and second key for prev_end_offset and curr_start_offset respectively */
    size_t prev_key_size, curr_key_size;

    size_t prev_start_offset = nonleaf_skip_initial_parameters(&tmp);
    prev_start_offset += skip_child_ptr(&tmp);


    /* Size of previous key to get the index */
    prev_key_size = nonleaf_get_single_key_size(tmp);
    
    /* The node is not sufficiently full to perform a node-split (Just an approximation) */
    if (prev_start_offset + prev_key_size + sizeof(char*) + get_max_block_size(true) >= node_use) {
        printf("Node size parameter is too small, unable to do child split...\n");
        assert(0);
    }
    
    
    /* Move tmp pointer to second key of the node */
    size_t curr_start_offset = prev_start_offset + nonleaf_skip_single_key(&tmp);
    curr_start_offset += skip_child_ptr(&tmp);
    
    prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
    curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

    curr_key_size = nonleaf_get_single_key_size(tmp);

    /* Initialize minimum key offset */
    if (prev_distance < curr_distance) {
        printf("prev distance is %lf, curr distance is %lf\n", prev_distance, curr_distance);
        printf("Node size is most likely too small...\n");
        assert(0);

        min_distance = prev_distance;
        min_offset = prev_start_offset;

    } else {
        
        /* If second key is the last key, then node split probably shouldn't happen... */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_use) {
            printf("Something seems off... Node size probably too small... Doing a node split on 2 keys...\n");
            assert(0);
        }

        min_offset = curr_start_offset; 
        min_distance = curr_distance;
    }

    /* Go through blocks of the node and find the key_offset that is closest to NODE_MID_SIZE */
    while(prev_distance > curr_distance) {

        /* Get key size of curr_start_offset and update prev_start_offset */
        prev_key_size = curr_key_size;
        prev_start_offset = curr_start_offset;

        /* Update curr_start offset to next key */
        curr_start_offset += nonleaf_skip_key_to_key(&tmp);
        curr_key_size = nonleaf_get_single_key_size(tmp);

        /* Stop if current key is the last key. Can't node split when nothing on the 
         * right side of the middle */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_use)
        {
            // printf("For large enough node sizes, this shouldn't run...\n");
            break;
        }
        
        /* Cacluate the new distances between the keys and the node mid point */
        prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
        curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

        /* Update the min_offset and min_distance accordingly for next while loop */
        if ((prev_distance < min_distance)) {
            min_distance = prev_distance;
            min_offset = prev_start_offset;
        }

        if ((curr_distance < min_distance)) {
            min_distance = curr_distance;
            min_offset = curr_start_offset;
        }     
    } 

    /* Update *node_ptr to start of the key that is closest to the NODE_MID_SIZE */
    *node_ptr += min_offset;
    return min_offset;
}


/* Function that takes a node(char*) as input. The function outputs true 
 * if node is a leaf node, false (0) if node is not a leaf node */
inline bool node_is_leaf(char* node) {
    return *(bool*)(node);
}

/* Function that takes a node(char*) as input. The function outputs the 
 * the curr_node_size(size_t) used, which can be used as an offset */
inline size_t get_node_use(char* node) {
    node += sizeof(bool); // add offset to skip is_leaf boolean
    return *(size_t*)(node);
}

/* Function that takes no input and returns the size of initial parameters used for node housekeeping. */
inline size_t nonleaf_get_init_param_offset(void) {
    return sizeof(bool) + sizeof(size_t); 
}

/* Function that takes no input and returns the size of initial parameters used for leaf node housekeeping */
inline size_t leaf_get_init_param_offset(void) {
    return sizeof(bool) + sizeof(size_t) + sizeof(char*);
}

/* Function that takes no input and returns the maximum size of a block. (Based on MAX_STRING_BYTES key and ptr) */
// leaf node has an extra counter which is integer type 
size_t get_max_block_size(bool isleaf) {
    /* String length integer + String with null-byte + counter + child_ptr */
    if (isleaf)
        return (sizeof(int) + MAX_STRING_BYTES + 1 + sizeof(int) + sizeof(char*));
    else 
        return (sizeof(int) + MAX_STRING_BYTES + 1 + sizeof(char*));
}

/* Function that calculates the block size given the string length */
size_t leaf_get_doubleblock_size(int str_length) {
    return (sizeof(int) + str_length + 1 + sizeof(int) + sizeof(char*) + sizeof(char*));
}

/* Function that takes a leaf node as input. The function outputs the address of the consecutive leaf node */
char* get_next_leaf_node(char* node) {
    /* A node must be a leaf to have a pointer to the next consecutive node */
    assert(node_is_leaf(node));

    /* Return the address of the next node */
    return *(char**)(node + leaf_get_init_param_offset());
}


/* Function that takes a node and size_t node_size as input. 
 * The function updates the current node use to new_size */
inline int update_node_use(char* node, size_t new_size) {
    if (!node) {
        fprintf(stderr, "Null pointer...\n");
        return 0;
    }

    node += sizeof(bool);
    *(size_t*)node = new_size;

    return 1;    
}

/* These functions behave differently for internal and leaf nodes:
 * skip_block_from_start, get_single_key_size, skip_key_to_key, get_single_key_size
 *
 *
 *
 *
 * */


/* Function that takes a char* LEAF node pointer. The function assumes the node points
 * to the start of the block. The function increments the counter of part if the key 
 * by 1 (+= 1) */
int increment_block_counter(char* node) {
    if (!node || !node_is_leaf(node)) {
        printf("Null node\n");
        assert(0);
    }

    /* Increment string counter by 1, and add to keys processed */
    node += sizeof(char*);
    int tmp_length = *(int*)node; node += sizeof(int) + tmp_length + 1;
    *(int*)(node) += 1;

    keys_processed++;
    return 1;
}

/* Note that this function is for non-leaf nodes, and double pointer points to start of a key 
 * Function that takes a double pointer node as input and offsets pointer to node by a single key. */
inline size_t non_leaf_skip_single_key(char** node_ptr) {
    if (!node_ptr) {
        printf("Null input\n");
        assert(0);
    }

    /* Non-leaf node does not have counter */
    int tmp_length = *(int*)(*node_ptr); *node_ptr += sizeof(int) + tmp_length + 1;
    keys_processed++;

    return sizeof(int) + tmp_length + 1;
}

/* Note that this function is for leaf nodes, and double pointer points to start of a key 
 * Function that takes a double pointer node as input and offsets pointer to node by a single key. */
inline size_t leaf_skip_single_key(char** node_ptr) {
    if (!node_ptr) {
        printf("Null input\n");
        assert(0);
    }

    /* Non-leaf node does not have counter */
    int tmp_length = *(int*)(*node_ptr); *node_ptr += sizeof(int) + tmp_length + 1 + sizeof(int);
    keys_processed++;

    return sizeof(int) + tmp_length + 1 + sizeof(int);
}


/* Function that takes a double pointer node as input and offsets pointer to node by ptr (child node) */
inline size_t skip_child_ptr(char** node_ptr) {
    /* Sanity Check */
    //if (!node_ptr) {
    //    fprintf(stderr, "Null pointer...\n");
    //    return 0;
    //}

    /* offset by ptr */
    *node_ptr += sizeof(char*);
    return sizeof(char*);
}

/* Function that takes a char* node as input. char* node currently points to a block
 * The function outputs the address of the child node in the block. */
inline char* get_child_node(char* node) {
    return *(char**)node;
}


/* Note: The funciton works for both internal and leaf nodes. The function assumes the input is pointer to the start of some block
 * Funtion that takes a pointer to the start of a block in some node as input. The function assumes the pointer
 * currently points to the start of a block and does not modify the pointer. The 
 * function outputs the strlength of the key in the current block. */
int get_str_length(char* node) {
    //if (!node) {
    //    fprintf(stderr, "Null pointer input");
    //    return -1;
    //}

    /* Skip initial parameters, the child pointer and returns the length of the string */
    int tmp_length = *(int*)(node + sizeof(char*));

    /* Getting the size of the string of a block */
    keys_processed++;
    return tmp_length;
}


/* Note: The function is used for internal nodes (without string counter)
 * Function that takes a char* pointer to the start of some key as input. 
 * The function outputs the size of the key */
size_t nonleaf_get_single_key_size(char* node) {

   // assert(!node_is_leaf(node));
   // if (!node) {
   //     printf("Null node\n");
   //     assert(0);
   // }

    int tmp_length = *(int*)node;
    
    return sizeof(int) + tmp_length + 1;
}

/* Note: The function is used for leaf nodes (with string counter)
 * Function that takes a char* pointer to the start of some key as input. 
 * The function outputs the size of the key */
size_t leaf_get_single_key_size(char* node) { 

   // assert(node_is_leaf(node));
   // if (!node) {
   //    printf("Null node\n");
   //    assert(0);
   // }
    
    int tmp_length = *(int*)node;

    return sizeof(int) + tmp_length + 1 + sizeof(int); // Leaf node has string counter in key 

}

/* Note: this function is used for leaf nodes
 * Function that takes a double pointer to the start of a node and index as input. 
 * The function offsets the ptr to node by the number of index blocks to skip
 * and outputs the offset size */
size_t leaf_skip_block_from_start(char** node_ptr, int index) {
   
   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }
   // assert(node_is_leaf(*node_ptr));
    size_t tmp_offset = leaf_skip_initial_parameters(node_ptr);

    for (int i = 0; i < index; i++) {
        tmp_offset += leaf_skip_single_block(node_ptr);
    }

    return tmp_offset;

}


/* Note: This function is used for leaf nodes 
 * Function that takes a double pointer to the start of a node as input and offsets pointer to 
 * node by initial housekeeping parameters. It also outputs the offset */
size_t leaf_skip_initial_parameters(char** node_ptr) {
    
   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }
   // assert(node_is_leaf(*node_ptr));
    
    size_t tmp_init = leaf_get_init_param_offset();
    *node_ptr += tmp_init;

    return tmp_init;
}


/* Note: This function is for leaf nodes 
 * Function that takes a double pointer to the start of some block
 * and offsets pointer to node by one block */
inline size_t leaf_skip_single_block(char** node_ptr) {
   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }


    size_t tmp_offset = skip_child_ptr(node_ptr); // Skip child_ptr
    tmp_offset += leaf_skip_single_key(node_ptr); // Skip the key

    return tmp_offset;
}





/* Note: this function is used for non-leaf nodes
 * Function that takes a double pointer to the start of a node and index as input. 
 * The function offsets the ptr to node by the number of index blocks to skip
 * and outputs the offset size */
inline size_t nonleaf_skip_block_from_start(char** node_ptr, int index) { 


   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }
   // assert(node_is_leaf(*node_ptr));


    size_t tmp_offset = nonleaf_skip_initial_parameters(node_ptr);

    /* Skip index blocks */
    for (int i = 0; i < index; i++) {
        tmp_offset += nonleaf_skip_single_block(node_ptr);
    }

    return tmp_offset;
}


/* Note: This function is used for leaf nodes 
 * Function that takes a double pointer to the start of a node as input and offsets pointer to 
 * node by initial housekeeping parameters. It also outputs the offset */
inline size_t nonleaf_skip_initial_parameters(char** node_ptr) {
    
   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }
   // assert(node_is_leaf(*node_ptr));
    
    size_t tmp_init = nonleaf_get_init_param_offset();
    *node_ptr += tmp_init;

    return tmp_init;
}



/* Note: This function is for non-leaf nodes 
 * Function that takes a double pointer to the start of some block
 * and offsets pointer to node by one block */
inline size_t nonleaf_skip_single_block(char** node_ptr) {
   // if (!node_ptr) {
   //     printf("Null input\n");
   //     assert(0);
   // }


    size_t tmp_offset = skip_child_ptr(node_ptr); // Skip child_ptr
    tmp_offset += nonleaf_skip_single_key(node_ptr); // Skip the key

    return tmp_offset;
}


/* Note that this function is for leaf nodes, and double pointer points to start of a key 
 * Function that takes a double pointer node as input and offsets pointer to node by a single key. */
inline size_t nonleaf_skip_single_key(char** node_ptr) {
    //if (!node_ptr) {
    //    printf("Null input\n");
    //    assert(0);
    //}

    /* Non-leaf node does not have counter */
    int tmp_length = *(int*)(*node_ptr); *node_ptr += sizeof(int) + tmp_length + 1;
    keys_processed++;

    return sizeof(int) + tmp_length + 1;
}



/* Note: This function is for leaf nodes
 * Function that takes a char** node_ptr as input and assumes *node_ptr 
 * points to the start of the key. This moves the *node_ptr to the start 
 * of the next key and outputs the offset. */
size_t leaf_skip_key_to_key(char** node_ptr) {
    /* Sanity Checks */
   // if (!node_ptr) {
   //     fprintf(stderr, "Null pointer...\n");
   //     return 0;
   // }

    /* Move to next key and output offset. Skip key and then ptr */
    size_t next_key_offset = leaf_skip_single_key(node_ptr);
    next_key_offset += skip_child_ptr(node_ptr);

    return next_key_offset;
}



/* Note: This function is for non-leaf nodes
 * Function that takes a char** node_ptr as input and assumes *node_ptr 
 * points to the start of the key. This moves the *node_ptr to the start 
 * of the next key and outputs the offset. */
size_t nonleaf_skip_key_to_key(char** node_ptr) {
    /* Sanity Checks */
   // if (!node_ptr) {
   //     fprintf(stderr, "Null pointer...\n");
   //     return 0;
   // }

    /* Move to next key and output offset. Skip key and then ptr */
    size_t next_key_offset = nonleaf_skip_single_key(node_ptr);
    next_key_offset += skip_child_ptr(node_ptr);

    return next_key_offset;
}



/* Function that takes a a pointer to the start of some block, array from stack,
 * and string to compare as input. 
 * The function outputs < 0 if string in block smaller than str_cmp, 0 if equal, 
 * > 0 if string in block is larger than str_cmp. NOTE THAT THIS FUNCTION DOESN'T
 * AlTER THE PARENT NODE_PTR
 * */



// Probably better to get rid of this function altogether and create a custom strcmp function that moves the pointer
// as the string is compared, but we work on that later
inline int compare_current_string(char* block, char* tmp_array, const char* str_cmp) {
    //if (!block|| !tmp_array) {
    //    fprintf(stderr, "Emtpy pointer input...\n");
    //    return POSITIVE;
    //}
    
    /* Get the string from current block and store in temporary array */
    int tmp_length = *(int*)(block + sizeof(char*));

    /* Checking */
    assert(tmp_length >= 0);

    // assert(tmp_length >= 0);
    memcpy(tmp_array, block + sizeof(char*) + sizeof(int), tmp_length + 1);

    /* When string in block compared with string to find */
    keys_processed++;

    /* return comparison results of two strings */
    return strcmp(tmp_array, str_cmp);
}



/* Note: This function requires the block to belong to a leaf node, since internal nodes have no string 
 * counters. 
 * Function that takes a char* pointer to the start of some block. The function assumes the pointer points
 * to the start of the block. The function increments the counter of part if the key 
 * by 1 (+= 1) */
int leaf_increment_block_counter(char* block) {
   // if (!block) {
   //     fprintf(stderr, "Null input...\n");
   //     return 0;
   // }

    /* Skip past child pointer, str_length, string so the pointer points to counter */
    block += sizeof(char*);
    int tmp_length = *(int*)block; block += sizeof(int) + tmp_length + 1;
    
    *(int*)(block) += 1; //Increment node key counter by 1
    
    keys_processed++; // measuring variable 

    return 1;
}



/* Note: This function is for blocks belonging to leaf nodes 
 * Function that compares the string in the second block with strcmp. It 
 * *node_ptr is currently pointer to the start of the first block.
 * The function outputs -1 if string in block smaller than str_cmp, 0 if equal 
 * and +1 if string in block is larger than str_cmp;
 * */

/* ********* !!!!!! */
/* This function might be the one causing the overhead. We compare strings without moving the pointer
 * just incase we still need to position of the pointer ...*/
inline int leaf_compare_second_string_move(char** node_ptr, size_t node_size, size_t* offset_ptr, char* tmp_array, 
                          const char* str_cmp) {

    //if (!node_ptr || !tmp_array) {
    //    printf("Something's wrong...\n");
    //    assert(0);
    //}

    /* This condition is required for the start, where only a single pointer*/
    //if (*offset_ptr + sizeof(char*) >= node_size) {
    //    printf("Currently at the last block (compare_second_string_move)");
    //    return POSITIVE;
    //}
    

    /* Move *node_ptr to next block, and store string length */
    *node_ptr += sizeof(char*);  // Skip child ptr
    int tmp_length = *(int*)(*node_ptr);  // store string length 
    
    (*node_ptr) += sizeof(int) + tmp_length + 1 + sizeof(int); // Skip string length, string and string counter 

    /* Move offset forward one block to be consistent with *node_ptr */
    *offset_ptr += sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int); 
    

    /* Currently at the relative second block. if this block has no keys, then it is the last block
     * we don't modify this */
    if (*offset_ptr + sizeof(char*) == node_size) {
        return POSITIVE;
    }

    
    //tmp_length = get_str_length(*node_ptr);
    tmp_length = *(int*)((*node_ptr) + sizeof(char*));


    /* Copy the string in the second block to tmp array */
    memcpy(tmp_array, *node_ptr + sizeof(char*) + sizeof(int), tmp_length + 1);

    keys_processed++;
    
    /* Compare the second block string with the string to insert */
    return strcmp(tmp_array, str_cmp);
}



/* Note: This function is for blocks belonging to leaf nodes 
 * Function that compares the string in the second block with strcmp. It 
 * *node_ptr is currently pointer to the start of the first block.
 * The function outputs -1 if string in block smaller than str_cmp, 0 if equal 
 * and +1 if string in block is larger than str_cmp;
 * */

/* ********* !!!!!! */
/* This function might be the one causing the overhead. We compare strings without moving the pointer
 * just incase we still need to position of the pointer ...*/
inline int nonleaf_compare_second_string_move(char** node_ptr, size_t node_size, size_t* offset_ptr, char* tmp_array, 
                          const char* str_cmp) {

    if (!node_ptr || !tmp_array) {
        printf("Something's wrong...\n");
        assert(0);
    }

    /* This condition is required for the start, where only a single pointer*/
    //if (*offset_ptr + sizeof(char*) >= node_size) {
    //    printf("Currently at the last block (compare_second_string_move)");
    //    return POSITIVE;
    //}
    

    /* Move *node_ptr to next block, and store string length */
    *node_ptr += sizeof(char*);  // Skip child ptr
    int tmp_length = *(int*)(*node_ptr);  // store string length 
    (*node_ptr) += sizeof(int) + tmp_length + 1; // Skip string length, string (without string counter)

    /* Move offset forward one block to be consistent with *node_ptr */
    *offset_ptr += sizeof(char*) + sizeof(int) + tmp_length + 1; 
    

    /* Currently at the relative second block. if this block has no keys, then it is the last block
     * we don't modify this */
    if (*offset_ptr + sizeof(char*) == node_size) {
        return POSITIVE;
    }

    
    // tmp_length = get_str_length(*node_ptr);
    tmp_length = *(int*)(*node_ptr + sizeof(char*));

    /* Copy the string in the second block to tmp array */
    memcpy(tmp_array, *node_ptr + sizeof(char*) + sizeof(int), tmp_length + 1);

    keys_processed++;
    
    /* Compare the second block string with the string to insert */
    return strcmp(tmp_array, str_cmp);
}




/* Function that compares the string in the middle of some block */
// THIS FUNCTION SEEMS THE SAME AS COMPARE CURRENT STRING 
int compare_middle_string(char* node, char* block, char* tmp_array, const char* str_cmp) {
   // if (!block|| !tmp_array || !node) {
   //     fprintf(stderr, "Emtpy pointer input...\n");
   //     assert(0);
   // }
    
    /* This should be required */
    /* If start of current string is nullbyte, should only happen in the start. Node has no keys */
    //if (get_node_use(node) == INITIAL_NODE_SIZE_USE) {
    //    assert(0);
    //    return POSITIVE;
    //}

    /* Get the string from current block and store in temporary array */
    // int tmp_length = get_str_length(block);
    int tmp_length = *(int*)(block + sizeof(char*));

    memcpy(tmp_array, block + sizeof(char*) + sizeof(int), tmp_length + 1);

    
    /* return comparison results of two strings */
    return strcmp(tmp_array, str_cmp);
}



/* Print all blocks from the start of the node */
void leaf_print_node_lexigraphic_check(char* node) {
    if (!node) {
        printf("Node is null...\n");
        assert(0);
    }

    printf("\nPrinting the leaf node with address %p\n", node);
    

    size_t node_space_used = get_node_use(node);

    if (node_space_used == leaf_get_init_param_offset()) {
        return;
    }

    printf("Find node size: %zu\n", node_space_used);
    printf("The linked list next pointer is %p\n", *(char**)(node + nonleaf_get_init_param_offset()));

    char* ptr = node; 
    size_t offset = 0; offset += leaf_skip_initial_parameters(&ptr);

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
        stored_length = *(int*)ptr;
        ptr += sizeof(int); offset += sizeof(int);

        /* Storing string and String counter*/
        memcpy(stored_string, ptr, stored_length + 1);
        // printf("Previous string is %s, current string is %s\n", prev_store, stored_string);

        assert(strcmp(prev_store, stored_string) <= 0);
        strcpy(prev_store, stored_string);

        ptr += stored_length + 1; offset += stored_length + 1;

        stored_counter = *(int*)ptr; 
        ptr += sizeof(int); offset += sizeof(int);
        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length:%d  ", stored_length);
        fprintf(stdout, "String:%s  ", stored_string);
        fprintf(stdout, "Counter:%d  ", stored_counter);


    }
    printf("\n");
    
}


/* Print all blocks from the start of the node */
void nonleaf_print_node_lexigraphic_check(char* node) {
    if (!node) {
        printf("Node is null...\n");
        assert(0);
    }

    printf("\nPrinting internal (nonleaf) node with address %p\n", node);
    

    size_t node_space_used = get_node_use(node);

    if (node_space_used == nonleaf_get_init_param_offset()) {
        return;
    }

    printf("Find node size: %zu\n", node_space_used);

    char* ptr = node; 
    size_t offset = 0; offset += nonleaf_skip_initial_parameters(&ptr);

    /* Temp variatbles to output data to stdout */
    int stored_length, stored_counter; 
    char stored_string[MAX_STRING_BYTES];


    /* Null character has the smallest ASCII value */
    char prev_store[MAX_STRING_BYTES] = "\0";
    
    // Note that memcpy is used to indicate that overlapping memory isn't the intention 
    while(offset < node_space_used) {

        printf("%p  ", *(char**)ptr);
        ptr += sizeof(char*); offset += sizeof(char*);

        /* Last child ptr */
        if (offset >= node_space_used) {
            printf("End of node\n");
            assert(offset == node_space_used);
            return;
        }
        
        /* Store the strings in temporary variables */
        stored_length = *(int*)ptr;

        ptr += sizeof(int); offset += sizeof(int);

        /* Storing string and String counter*/
        memcpy(stored_string, ptr, stored_length + 1);
        // printf("Previous string is %s, current string is %s\n", prev_store, stored_string);

        assert(strcmp(prev_store, stored_string) <= 0);
        strcpy(prev_store, stored_string);

        ptr += stored_length + 1; offset += stored_length + 1;

        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length:%d  ", stored_length);
        fprintf(stdout, "String:%s  ", stored_string);

    }
    printf("\n");
    
}



/* Function that takes as input a pointer to a char** 2d array of strings. The funtion frees all strings in the array */
void free_array_list(char** word_list) {
    /* Free columns */
    for (int i = 0; i < WORDS_NUM; i++) {
        free(word_list[i]);
    }
    /* Free the row */
    free(word_list);
}



/* Function that returns the left most leaf node in the B+ tree */
char* left_most_node(char* root) {
    char* current = root;

    while ((current != NULL) & (!node_is_leaf(current))) {
        current = *(char**)(current + nonleaf_get_init_param_offset());
    }

    return current;
}

/* Function that takes the left most node as input, and prints all keys 
 * in the B+-tree */
void print_all_keys (char* mostleft_node) {
    if (!mostleft_node) {
        printf("invalid input\n");
        assert(0);
    }
    
    char* current = mostleft_node;
    char* next, *tmp; 
    size_t node_use, offset;
    int tmp_length, tmp_counter; 

    char word[MAX_STRING_BYTES + 1];
    double tmp_ratio;

    while (current != NULL) {
        node_use = get_node_use(current);
        tmp = current;
        
        /* Node ratio */
        tmp_ratio = ((double)(node_use) / NODE_SIZE);
        avg_node_use_ratio += tmp_ratio;


        offset = leaf_skip_initial_parameters(&tmp);

        if (KEY_COUNT) {
            while(offset < node_use) {

                /* Go past child pointer */
                tmp += sizeof(char*); offset += sizeof(char*);
                if (offset >= node_use) {
                    break;
                }
                
                /* Move pointer and offset to the next child ptr */
                tmp_length = *(int*)(tmp); 
                tmp += sizeof(int); offset += sizeof(int);
                memcpy(word, tmp, tmp_length + 1);

                tmp += tmp_length + 1; offset += tmp_length + 1;

                tmp_counter = *(int*)tmp; 
                tmp += sizeof(int); offset += sizeof(int);

                /* Print the data to stdout (for checking mainly) */

                if (PRINT_TOGGLE) {
                    fprintf(stdout, "Length:%d  ", tmp_length);
                    fprintf(stdout, "String:%s  ", word);
                    fprintf(stdout, "Counter:%d  ", tmp_counter);
                }
                // unique_key_counter++;
            }
        }
        /* Move to next leaf node */
        current = *(char**)(current + nonleaf_get_init_param_offset());
    }

    printf("\n");
    return;
}






/* Experimenting */

/* Function that takes a ptr to a str (in a key) and string to compare as input. The funciton compares 
 * the relative lexigraphic size of the strings, and outputs the results. The function also moves the 
 * ptr to the string to the start of the next block */
inline int custom_strcmp(char** str_ptr, const char* str, int key_str_length) {
    /* String comparision between the two strings */
    while(**str_ptr && (**str_ptr == *str)) {
        (*str_ptr)++;
        str++;
        key_str_length--;
    }

    /* Lexigraphic difference between the two strings */
    int difference = (unsigned char)(**str_ptr) - (unsigned char)(*str);
    *(str_ptr) += key_str_length + 1;
    
    keys_processed++;
    return difference;
}

