
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
// #define NODE_MID_SIZE (INIT_PARAM_OFFSET + (NODE_SIZE - INIT_PARAM_OFFSET)/ 2.0)
#define NODE_MID_SIZE (NODE_SIZE / 2.0)
#define STR_SMALLER -1
#define POSITIVE 10 // This is just any integer, so it returns something 
#define ALLOCATE_OVERHEAD 8

#define PRINT_TOGGLE 0
#define CHECK_TOGGLE 1
#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000



/* Parameter choice */
// #define T_DEGREE 10
#define NODE_SIZE 512
#define WORDS_NUM HUNDRED_MILLION // Parameter to control how many words to get from text file 
// #define FILENAME "wordstream.txt"
#define FILENAME "wikipedia_with_cap.txt"



/* Main function prototypes */
char* Bplus_create(void);


/* Supplementary main function prototypes */
size_t move_mid_node(char** node_ptr);

/* Supplementary quick function prototypes */
char* initialize_node(bool is_leaf, size_t node_size);
inline size_t get_node_use(char* node);
inline bool node_is_leaf(char* node);




/* Think about the node structure of B-trees and B+ trees. This will cause differences in the code... */



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
char* Bplus_split(char* parent, char* child, char* child_location) {
    size_t parent_node_use = get_node_use(parent);
    size_t child_node_use = get_node_use(child);
    

    /* Create child right node */
    char* child_right = initialize_node(node_is_leaf(child), NODE_SIZE);
    
    /* Find the middle key of the child node (to copy to the child_right node) */
    char* mid_child_ptr = child;
    size_t mid_child_offset = move_mid_node(&mid_child_ptr);



    return NULL;
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
    *(size_t*)(tmp) = INITIAL_NODE_SIZE_USE; 

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


size_t move_mid_node(char** node_ptr) {

    size_t node_used = get_node_use(*node_ptr);

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
    size_t prev_start_offset = 0; size_t prev_key_size, curr_key_size;
    prev_start_offset += get_init_param_offset();
    prev_start_offset += sizeof(char*);

    /* Size of previous key to get the index */
    prev_key_size = get_single_key_size(tmp + prev_start_offset);
    
    /* The node is not sufficiently full to perform a node-split (Just an approximation) */
    if (prev_start_offset + prev_key_size + sizeof(char*) + get_max_block_size() >= node_used) {
        printf("Node size parameter is too small, unable to do child split...\n");
        assert(0);
    }
    
    
    /* Move tmp pointer to second key of the node */
    size_t curr_start_offset = skip_block_from_start(&tmp, 1);
    curr_start_offset += skip_child_ptr(&tmp);
    
    prev_distance = fabs(NODE_MID_SIZE - (prev_start_offset + prev_key_size - 1));
    curr_distance = fabs(NODE_MID_SIZE - curr_start_offset);

    curr_key_size = get_single_key_size(tmp);

    /* Initialize minimum key offset */
    if (prev_distance < curr_distance) {
        printf("Node size is most likely too small...\n");
        assert(0);

        min_distance = prev_distance;
        min_offset = prev_start_offset;

    } else {
        
        /* If second key is the last key, then node split probably shouldn't happen... */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_used) {
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
        curr_start_offset += skip_key_to_key(&tmp);
        curr_key_size = get_single_key_size(tmp);

        /* Stop if current key is the last key. Can't node split when nothing on the 
         * right side of the middle */
        if (curr_start_offset + curr_key_size + sizeof(char*) >= node_used)
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
size_t get_nonleaf_init_param_offset(void) {
    return sizeof(bool) + sizeof(size_t); 
}

/* Function that takes no input and returns the size of initial parameters used for leaf node housekeeping */
size_t get_leaf_init_param_offset(void) {
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


// Note that the node structure is different for leaf nodes and internal nodes 
