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


#ifndef TABLE_SIZE  
#define TABLE_SIZE (1 << 10) // Default node size
#endif

#ifndef DATASET_TYPE
#define DATASET_TYPE 1  // Default dataset type
#endif

#ifndef WORDS_NUM
#define WORDS_NUM 1000000  // Default dataset type
#endif

#ifndef SEED
#define SEED 73802 // Default node size
#endif

/* Global Variables */
#define MAX_STRING_BYTES 64
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0




#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000

#define ALLOCATE_OVERHEAD 8

/* Parameter choice */
// #define FILENAME "wordstream.txt"
// #define FILENAME "wikipedia_with_cap.txt"

/* Toggle */
#define PRINT_TOGGLE 0


/* Hash table structure */
typedef struct{
    char** buckets;
    int table_size;
}hashtable_t;


/* Main function prototypes */
void hash_insert(hashtable_t *table, char* str, char* buffer);
char* get_hash(hashtable_t *table, char* str, char* buffer);
unsigned int xorhash(char *word, int tsize);
void freehash(hashtable_t *table);

/* Supplementary function prototypes */
hashtable_t* create_hashtable(int size);
char* create_node(char* const str);

/* Prototypes for checking */
void check_print(hashtable_t* table, bool print, char* buffer);

static int seed = SEED;

static size_t memory_usage = 0;
static int unique_key_counter = 0;
static int non_unique_key_counter = 0;
static size_t keys_processed = 0;
static int number_of_nodes = 0;
static double avg_node_use_ratio = 0;

/* Static variables for checking */
static size_t optimization_counter = 0;

/* Note that for this algorithm, move to front strategy has been implemented 
 * for both insertion and search. The node is always inserted at the beginning
 * and if a query if found, the node is moved to the start. */

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
    long double elapsed1, elapsed2;


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


    printf("Beginning Linked-Hash creation, insertion and search\n");
    
    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    hashtable_t *table = create_hashtable(TABLE_SIZE);
    for (int i = 0; i < counter - 1; i ++) {
        // printf("Word to insert in tree is %s\n", word_list[i]);
        hash_insert(table, word_list[i], word);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed1 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    /* Search for the words in the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i++) {
        get_hash(table, word_list[i], word);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed2 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    check_print(table, PRINT_TOGGLE, word);

    /* Measuring data */
    printf("%d, NUX, MTFLinked-Hash, non-unique strings\n", non_unique_key_counter);
    printf("%.3Lf, INX, MTFLinked-Hash, seconds to insert\n", elapsed1);
    printf("%.3Lf, SRX, MTFLinked-Hash, seconds to search\n", elapsed2);
    printf("%zu, MUX, MTFLinked-Hash, memory usage\n", memory_usage);
    printf("%d, UKX, MTFLinked-Hash, unique strings\n", unique_key_counter);
    printf("%zu, KPX, MTFLinked-Hash, keys processed\n", keys_processed);
    printf("%d, NNX, MTFLinked-Hash, number of nodes\n", number_of_nodes);
    printf("%.11Lf, ASX, MTFLinked-Hash, average string search time\n", elapsed2/non_unique_key_counter);
    printf("\n");

    printf("The following variables are used purely for checking\n");
    printf("%zu, OCX, Linked-Hash, Optimizattion Counter\n", optimization_counter);
    printf("\n\n\n\n");

    fclose(file);
    return 0;
}

/* Function that creates the hash table */
inline hashtable_t* create_hashtable(int size) {
    hashtable_t* hash = malloc(sizeof(hashtable_t));
    hash -> buckets = calloc(size, sizeof(char**));
    hash -> table_size = size;

    return hash;
}

/* Function that takes a string as input. The function initializes the node with the string data and outputs 
 * the node address */
inline char* create_node(char* str) {
    int str_length = strlen(str);
    
    char* node = malloc(sizeof(char*) * (sizeof(char*) + sizeof(int) + (str_length + 1) + sizeof(int)));
    
    char* tmp = node;

    /* Insert the values of the string into the new key / node */
    *(char**)tmp = NULL; tmp += sizeof(char*);
    *(int*)tmp = str_length; tmp += sizeof(int);
    memcpy(tmp, str, str_length + 1); tmp += str_length + 1;
    *(int*)tmp = 1;

    /* Key is stored into a new node -> keys_processed */
    keys_processed++;

    number_of_nodes++;
    memory_usage += (sizeof(char*) + sizeof(int) + (str_length + 1) + sizeof(int));
    memory_usage += ALLOCATE_OVERHEAD;

    return node;
}


/* Function that takes a str as input, creates the key and inserts the string into the hash table */
void hash_insert(hashtable_t *table, char* str, char* buffer) {

    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the linked list head */
    char* initial = table -> buckets[index];

    char* node = initial;

    /* Empty bucket */
    if (node == NULL) {
        table -> buckets[index] = create_node(str);

    /* Non empty bucket */
    } else {

        char* next, *str_reuse;
        int store_length, store_counter, length_vary, str_cmp_difference;
        /* FInd the correct position to insert the new key */
        char* node_start = NULL;
        char* prev_node;
        

        while (node != NULL) {
            /* Store address of previous node */
            prev_node = node_start;
            
            /* Keep track of current node start */
            node_start = node;

            /* Get key data of the node */
            node += sizeof(char*); 

            /* Store the string and string length */
            store_length = *(int*)node; node += sizeof(int); 

            
            length_vary = store_length;
            str_reuse = str;

            while(*node && (*node == *str_reuse)) {
                node++;
                str_reuse++;
                length_vary--;
            }
            
            /* Lexigraphic difference between the two strings */
            str_cmp_difference = (unsigned char)(*node) - (unsigned char)(*str_reuse);
            node += length_vary + 1; 

            /* string compare keys processed */
            keys_processed++;


            /* If the string to insert is the same as linked list, 
             * increase the counter. Also implement MTF */
            if (str_cmp_difference == 0) {
                *(int*)(node) += 1;

                /* Move the current node to the start of the bucket. Update
                 * linked list */
                /* If accessed node is not the first node */
                if (prev_node != NULL) {
                    *(char**)prev_node = *(char**)node_start; // Update next pointer of previous node 
                    *(char**)node_start = initial;
                    table -> buckets[index] = node_start;
                }

                return;
            }
            
            /* Go to next node */
            node = *(char**)(node_start);
        }

        
        /* If the key currently isn't present in the bucket, create a new node and
         * insert at the start. Update the pointers accordinly */
        node = create_node(str);
        *(char**)node = initial;
        table -> buckets[index] = node;

        // *(char**)node_start = create_node(str); 
    }

}

/* Function that takes a hashtable pointer, string to find and a buffer as input. The function returns the address of the 
 * node containing the string as the key if found. Otherwise, the function returns NULL if not found */
char* get_hash(hashtable_t *table, char* str, char* buffer) {
    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the node from the bucket */
    char* initial = table -> buckets[index];
    char* node = initial;

    char* next, *node_start = NULL;
    char* prev_node, *str_reuse;
    int store_length, length_vary, str_cmp_difference;
    
    /* Linear search along the nodes corresponding to the bucket to find the key */
    while (node != NULL) {
        prev_node = node_start;

        node_start = node;

        node += sizeof(char*);
        
        store_length = *(int*)node; node += sizeof(int);



        /* Copy variables to reuse */
        length_vary = store_length;
        str_reuse = str;

        while(*node && (*node == *str_reuse)) {
            node++;
            str_reuse++;
            length_vary--;
        }
        
        /* Lexigraphic difference between the two strings */
        str_cmp_difference = (unsigned char)(*node) - (unsigned char)(*str_reuse);

        // node += length_vary + 1;  // Counter not required

        /* String comparison -> Keys processed */
        keys_processed++;


        /* Return the node address if the string is found in node */
        if (str_cmp_difference == 0) {
             
            /* Just for checking purposes */
           // printf("The string to search is %s\n", str);
           // printf("Node data address %p ...\n", prev);
           // printf("The string is %s\n", buffer);
           // printf("The string counter is %d\n", *(int*)(node + store_length + 1));
            
            /* If accessed node is not the first node, implement move to front */
            if (prev_node != NULL) {
                *(char**)prev_node = *(char**)node_start;
                *(char**)node_start = initial;
                table -> buckets[index] = node_start;
            }
           
            optimization_counter++;
            return node_start;
        }


        /* Go to next node of the linked list */
        node = *(char**)node_start;
    }

    /* Otherwise the string is not found, return 0*/
    printf("The string %s is not found!\n", str);
    assert(0);
    return NULL;
}



/* Hash function */
inline unsigned int xorhash(char *word, int tsize)
{
    char	c;
    unsigned int	h;

    h = seed;
    for( ; ( c=*word )!='\0' ; word++ )
    {
	/* stuff adapted from titan */
	/* if (i == wordlen - 1 && wordlen & 1)
		c = *word;
	else if (i & 1)
		c = *(word++ - 1);
	else
		c = *(word++ + 1); */
	h ^= ( (h << 5) + c + (h >> 2) );
    }
    return((unsigned int)((h&0x7fffffff) % tsize));
}


/* Function that takes a hashtable pointer as input and frees all memory */
void freehash(hashtable_t *table) {
    char* node, *prev;


    for (int i = 0; i < table -> table_size; i++) {
        node = table -> buckets[i];
        
        /* Free linked list for each bucket */
        while(node != NULL) {
            /* Save next node before freeing memory */
            prev = node;
            node = *(char**)prev;   
            free(prev);
        }
    }

    /* Free the table data structure */
    free(table -> buckets);
    free(table);
}



/* Function that takes the hashtable_t* table, print toggle boolean and buffer 
 * as input. The function counts the unique and non-unique strings and prints 
 * the key values if chosen */
void check_print(hashtable_t* table, bool print, char* buffer) {
    char* bucket; char* curr; 

    int tmp_length, tmp_counter; 

    if (print) {
        /* Iterate over all buckets to access array and print the keys */
        for (int i = 0; i < TABLE_SIZE; i++) {
            bucket = table -> buckets[i];

            while(bucket != NULL) {
                /* Save current node and next node */

                /* Print key address */
                printf("key data belongs to address %p\n", bucket);

                curr = bucket; curr += sizeof(char*);
                bucket = *(char**)bucket;
                
                tmp_length = *(int*)curr; curr += sizeof(int);
                memcpy(buffer, curr, tmp_length + 1); curr += tmp_length + 1;
                tmp_counter = *(int*)curr;

                /* Print key values */
                printf("The string is %s\n", buffer);
                printf("The string counter is %d\n", tmp_counter);

                /* Updating measuring variables*/
                unique_key_counter++;
                non_unique_key_counter += tmp_counter;
            }
        }

    } else {

        /* Iterate over all buckets to access array and print the keys */
        for (int i = 0; i < TABLE_SIZE; i++) {
            bucket = table -> buckets[i];

            while(bucket != NULL) {
                /* Save current node and next node */
                curr = bucket; curr += sizeof(char*);
                bucket = *(char**)bucket;
                
                tmp_length = *(int*)curr; curr += sizeof(int);
                memcpy(buffer, curr, tmp_length + 1); curr += tmp_length + 1;
                tmp_counter = *(int*)curr;

                /* Updating measuring variables*/
                unique_key_counter++;
                non_unique_key_counter += tmp_counter;

            }
        }
    }
}
