/* Libraries */
#include <_time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>


#ifndef DATASET_TYPE
#define DATASET_TYPE 1  // Default dataset type
#endif

#ifndef WORDS_NUM
#define WORDS_NUM 1000000  // Default dataset type
#endif

/* Global Variables */
#define MAX_STRING_BYTES 64

/* For convenience */
#define STR_SMALLER -1
#define POSITIVE 10 // This is just any integer, so it returns something 
#define LEFT_PTR_OFFSET 0
#define RIGHT_PTR_OFFSET (sizeof(char*))
#define INITIAL_COUNT 1
#define ALLOCATE_OVERHEAD 8

// #define FILENAME "wordstream.txt"
// #define FILENAME "wikipedia_with_cap.txt"

#define PRINT_TOGGLE 0

#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000

/* Data Volume */
//#define WORDS_NUM TEN_MILLION // Parameter to control how many words to get from text file 

/* PROBLEMS */

/* Function Prototypes */

/* Main Function Prototypes */
char* create_node(const char* str);
void binary_tree_insert(char** root_ptr, const char* str);
int binary_tree_search(char* node, const char* str);



inline int custom_strcmp(const char* str, char** str_ptr, int key_str_length);

/* Supplementary Function Prototypes */
void move_to_left_pointer(char** node_ptr);
void move_to_right_pointer(char** node_ptr);
int compare_key_string(char* n1, char* n2);
int compare_str_node(const char* str, char* n1, char* buffer);
void increment_counter(char* node);
void print_binary_node(char* node, char* word);
void print_node_string(char* node, char* word);
void binary_inorder(char* root, char* word);
void free_all_nodes(char* root);

void free_array_list(char** word_list);



/* Measuring variables */
static int non_unique_key_counter = 0;
static int unique_key_counter = 0;
static size_t memory_usage = 0;
static size_t keys_processed = 0;
static int number_of_nodes = 0;



/* Variables for debugging */
static size_t total_node_search_visits = 0;
static size_t optimization_counter = 0;


int main(int argc, char** argv) {


    FILE* file;
    char word[100]; size_t counter = 1; int str_length, flag = 0;

    /* Variables for measuring time */
    struct timespec prec_start, prec_end, search_start, search_end;
    long double elapsed1, elapsed2;
    double single_search_time;
    
    /* Row allocation */
    char **word_list = (char**)malloc(sizeof(char*) * WORDS_NUM);
    if (!word_list) {
        printf("Invalid word_list allocation\n");
        return 1;
    }
    


    /* Read file depending on dataset type */
    if (DATASET_TYPE == 1) { // Unique words
        file = fopen("wordstream.txt", "r");
    } else if (DATASET_TYPE == 2) { // Wikipedia 
        file = fopen("wikipedia_with_cap.txt", "r");
    } else { // Wikipedia without common function words
        assert(DATASET_TYPE == 3); 
        file = fopen("wikicap_without_common.txt", "r");
    }

    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }


    char* tree_root = NULL; 
    
    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {

        /* Only store strings that aren't too large */
        if ((str_length = strlen(word)) < MAX_STRING_BYTES) {
            word_list[counter - 1] = (char*)malloc(sizeof(char) * (str_length + 1));
            strcpy(word_list[counter - 1], word);
            counter++;
        }

        /* Store the specified number of strings only */
        if (counter > WORDS_NUM) {
            flag = 1; 
            printf("Enough word data: %d, stop loading words from text file.\n", WORDS_NUM);
            break;
        }
    }

    if (!flag) {
        printf("Not enough words in txt file, only loaded %zu words\n", counter - 1);
    }

    printf("Beginning binary tree word insertions and searches\n");
    /* Insert the words into the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i ++) {
        // printf("Word to insert in tree is %s\n", word_list[i]);
        binary_tree_insert(&tree_root, word_list[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed1 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)

    /* Search for the words in the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &search_start);
    //clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i++) {
        if(!binary_tree_search(tree_root, word_list[i])) {
            /* Word not found for some reason */
            assert(0);
        }
        /* For checking and accumulate individual search times to cumulative search times */
        // printf("The time to search for %s was %lf\n", word_list[i], single_search_time);
    }
    clock_gettime(CLOCK_MONOTONIC, &search_end);
    // clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed2 = (search_end.tv_sec - search_start.tv_sec) + (search_end.tv_nsec - search_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    /* Printing all strings in order */
    binary_inorder(tree_root, word);
    printf("\n");

    /* Printing all the measuring variable data */
    printf("For Binary tree:\n");
    printf("%d, NUX, Binary tree, non-unique strings\n", non_unique_key_counter);
    printf("%.3Lf, INX, Binary tree, seconds to insert\n", elapsed1);
    printf("%.3Lf, SRX, Binary tree, seconds to search\n", elapsed2);
    printf("%zu, MUX, Binary tree, memory usage\n", memory_usage);
    printf("%d, UKX, Binary tree, unique strings\n", unique_key_counter);
    printf("%zu, KPX, Binary tree, keys processed\n", keys_processed);
    printf("%d, NNX, Binary tree, number of nodes\n", number_of_nodes);
    printf("\n");


    printf("These measurements are used for debugging\n");
    printf("The average node visits per string search is %lf\n", (double)total_node_search_visits/non_unique_key_counter);
    printf("The average word search time is %.11Lf\n", elapsed2/non_unique_key_counter);
    


    free_all_nodes(tree_root);
    free_array_list(word_list);
    fclose(file);
    return 0;
}


/* Function that takes a char** root_ptr where *root_ptr points to tree root. The function creates a node for "str", and inserts 
 * it at the appropriate position of the tree. */
void binary_tree_insert(char** root_ptr, const char* str) {
    char* y = NULL;
    char* x = *root_ptr;
    int store;

    char* tmp; int tmp_length;
    const char* str_reuse;

    // char buffer[MAX_STRING_BYTES + 1];

    while(x != NULL) {
        /* Save previous subroot */
        y = x;

        /* Compare keys to determine which child node to access */
        /* String smaller than current key */
        //if ((store = compare_str_node(str, x, buffer)) <= 0)

        /* The following is a custom string comparison function */
        tmp = x + 2 * sizeof(char*); tmp_length = *(int*)tmp;
        tmp += sizeof(int);

        str_reuse = str;

        while(*tmp && (*tmp == *str_reuse)) {
            tmp++;
            str_reuse++;
            tmp_length--;
        }

        /* Lexigraphic difference between the two strings */
        store = (unsigned char)(*str_reuse) - (unsigned char)(*tmp);
        tmp += tmp_length + 1;

        keys_processed++;

        // if ((store = custom_strcmp(str, &tmp, tmp_length)) <= 0) {
        // if ((store = strcmp(str, x + 2 * sizeof(char*) + sizeof(int))) <= 0) {
        if (store <= 0) {

            /* If same string, increment counter for x */
            if (store == 0) {
                *(int*)tmp += 1;
                return;
            } else {
                x = *(char**)(x);
            }

        /* When the string is larger than the key */
        } else {
            x = *(char**)(x + sizeof(char*));
        }
    }

    /* The parent of z is y now, not saved in this code setup */

    /* Create Z node and put it in the correct position. Update sub-parent child pointer node */
    char* z = create_node(str);
    if (y == NULL) {
        *root_ptr = z;
        // printf("New root\n");
    // } else if (compare_key_string(z, y) < 0) {
    } else if (strcmp(str, y + 2 * sizeof(char*) + sizeof(int)) < 0) {
        keys_processed++;
        *(char**)(y) = z;
    } else {
        keys_processed++;
        *(char**)(y + sizeof(char*)) = z;
    }
}

/* Function that takes a root node and string to search as input. The function outputs
 * the node if the string is found. Otherwise, it outputs to stdout that string
 * was not found. */
//int binary_tree_search(char* root, const char* str) {
//    char tmp_word[MAX_STRING_BYTES + 1];
//
//    char* current = root;
//
//    /* Node empty or key found  */
//    if (root == NULL) {
//        printf("String is not found\n");
//        assert(0); // String should always be found
//        return 0;
//    } 
//    
//    if (compare_str_node(str, root, tmp_word) == 0) {
//        // printf("String is found\n");
//        // print_binary_node(root, tmp_word);
//        return 1;
//    }
//    
//    /* Search children nodes */
//    if (compare_str_node(str, root, tmp_word) < 0) {
//        return binary_tree_search(*(char**)(root + LEFT_PTR_OFFSET), str);
//    }
//    else{
//        return binary_tree_search(*(char**)(root + RIGHT_PTR_OFFSET), str);
//    }
//}


/* Function that takes a root node and string to search as input. The function outputs
 * the node if the string is found. Otherwise, it outputs to stdout that string
 * was not found. */
int binary_tree_search(char* root, const char* str) {
    // char buffer[200];
    char* current = root;
    int store;


    /* For checking */
    // int nodes_visited = 0;

    while (current != NULL) {
        // if ((store = compare_str_node(str, current, buffer)) == 0) {
        //if ((store = custom_strcmp(str, current + sizeof(char*) + sizeof(char*) + sizeof(int))) == 0) {

        // nodes_visited++;
        keys_processed++; // string comparison 
        
        if ((store = strcmp(str, current + 2 * sizeof(char*) + sizeof(int))) == 0) {

            //printf("String is found\n");
            //print_binary_node(current, buffer);
            // printf("To find %s, there were %d node visits\n", str, nodes_visited);

            //total_node_search_visits += nodes_visited;
            //printf("To find %s, there were %d node visits\n", str, nodes_visited);
            return 1;

        /* WHen string smaller than key, traverse left branch */
        } else if (store < 0) {
            current = *(char**)(current);
        /* When string is larger than the key, traverse right branch */
        } else {
            current = *(char**)(current + sizeof(char*));
        }
    }

    /* Performance measure: Increase tally */
    //total_node_search_visits += nodes_visited;
    return 0;
}



/* Function that takes a constant string as input. The function creates a node of the form
 * left child pointer, right child pointer and key. The key is defined as string length (integer), 
 * string (char*) and string counter (integer). The function outputs the node address of node */
inline char* create_node(const char* str) {
    /* Get length of string */
    int tmp_length = strlen(str);

    /* Dynamically allocate memory to new node */
    char* tmp = (char*)malloc(sizeof(char*) + sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int));
    
    /* Set left and right child pointer to NULL */
    *(char**)(tmp + LEFT_PTR_OFFSET) = NULL;
    *(char**)(tmp + RIGHT_PTR_OFFSET) = NULL;
    
    *(int*)(tmp + 2 * sizeof(char*)) = tmp_length; 
    memcpy(tmp + 2 * sizeof(char*) + sizeof(int), str, tmp_length + 1);
    *(int*)(tmp + 2 * sizeof(char*) + sizeof(int) + tmp_length + 1) = INITIAL_COUNT;

    /* Calculate memory usage of dynamic memory and housekeeping memory */
    memory_usage += (sizeof(char*) + sizeof(char*) + sizeof(int) + tmp_length + 1 + sizeof(int));
    memory_usage += ALLOCATE_OVERHEAD;

    number_of_nodes++;
    keys_processed++;

    return tmp;
}



/* Function that takes a ptr to a str (in a key) and string to compare as input. The funciton compares 
 * the relative lexigraphic size of the strings, and outputs the results. The function also moves the 
 * ptr to the string to the start of the next block */
//inline int custom_strcmp(const char* str, char* str_ptr) {
//    /* String comparision between the two strings */
//    while(*str_ptr && (*str_ptr == *str)) {
//        str_ptr++;
//        str++;
//    }
//
//    keys_processed++;
//    /* Lexigraphic difference between the two strings */
//    
//    return (unsigned char)(*str_ptr) - (unsigned char)(*str);
//    
//}



inline int custom_strcmp(const char* str, char** str_ptr, int key_str_length) {
    /* String comparision between the two strings */
    while(**str_ptr && (**str_ptr == *str)) {
        (*str_ptr)++;
        str++;
        key_str_length--;
    }

    /* Lexigraphic difference between the two strings */
    int difference = (unsigned char)(*str) - (unsigned char)(**str_ptr);
    *(str_ptr) += key_str_length + 1;

    // assert(*(*str_ptr - 1) == '\0');
    
    keys_processed++;
    return difference;
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
    *node_ptr += sizeof(char*) + sizeof(char*);

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

    /* When two keys are compared lexigraphically. Treat this as one key being processed */
    keys_processed++;

    return strcmp(n1, n2);
}


/* Function that takes a string and node as input, and compares the two strings (const string and string in node key)
 * lexigraphically. The function outputs <0 if constant string is less than n1 string , 0 if 
 * constant string and n1 string are equal, and >0 if constant string is larger than n1 string. */
int compare_str_node(const char* str, char* n1, char* buffer_1) {

    int n1_length = move_to_string(&n1);
    memcpy(buffer_1, n1, n1_length + 1);


    /* When a key is compared lexigraphically to determine where to insert the string */
    keys_processed++;
    return strcmp(str, n1);
}





/* Function that takes a char* node as input, and increases the counter by 1 */
inline void increment_counter(char* node){
    /* Skip child pointers */
    node += 2 * sizeof(char*);

    /* Move pointer to counter position */
    int tmp_length = *(int*)node; node += sizeof(int) + tmp_length + 1;
    
    /* Increase the counter by 1 */
    *(int*)node += 1;

    /* When a key counter is incremented by 1 */
    keys_processed++;
}

/* Function that takes a node as input and outputs all relevant data */
void print_binary_node(char* node, char* tmp_word) {
    if (!node) {
        printf("The node is emtpy\n");
        assert(0);
    }

    /* Prints the address of the child pointers */
    // THIS MIGHT BE WRONG. NO GUARENTEE THAT CHILD POINTERS ARE NON-EMPTY

    printf("Node address: %p\n", node);


    printf("LP: %p RP: %p", 
           *(char**)(node + LEFT_PTR_OFFSET), *(char**)(node + RIGHT_PTR_OFFSET));


    /* Prints the key of the node: length, string and counter */
    node += RIGHT_PTR_OFFSET + sizeof(char*);
    int tmp_length = *(int*)node; node += sizeof(int);

    memcpy(tmp_word, node, tmp_length + 1); node += tmp_length + 1;
    
    int tmp_counter = *(int*)node;

    printf(" Length: %d String: %s Count: %d \n\n", tmp_length, tmp_word, tmp_counter);

    return;
}



/* This function takes the root node as input. The function prints all strings in lexigraphic ascending order */
void binary_inorder(char* root, char* word) {
    if (root != NULL) {
        binary_inorder(*(char**)(root + LEFT_PTR_OFFSET), word);
        print_node_string(root, word);
        binary_inorder(*(char**)(root + RIGHT_PTR_OFFSET), word);
    }
}


/* The function takes a node, and pointer to heap array as input. The function outputs string of the node.
 * Note that this function also increments unique and non-unique key counter. */
void print_node_string(char* node, char* word) {
    node += 2 * sizeof(char*);

    int tmp_length = *(int*)node; node += sizeof(int);
    memcpy(word, node, tmp_length + 1);

    if (PRINT_TOGGLE)
        printf("%s ", word);

    /* Store the counter */
    node += tmp_length + 1;
    int tmp_counter = *(int*)node;

    /* Increment unique and non unique keys correctly */
    non_unique_key_counter += tmp_counter;
    unique_key_counter++;


}

/* Function that takes the root node as input and free all tree nodes */
void free_all_nodes(char* root) {
    if (root != NULL)
        return;

    free_all_nodes(*(char**)(root + LEFT_PTR_OFFSET));
    free_all_nodes(*(char**)(root + RIGHT_PTR_OFFSET));
    free(root);
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
