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




#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000

#define ALLOCATE_OVERHEAD 8

/* Parameter choice */
#define TABLE_SIZE (1 << 21)
#define WORDS_NUM TEN_MILLION // Parameter to control how many words to get from text file 
#define FILENAME "wordstream.txt"
// #define FILENAME "wikipedia_with_cap.txt"


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

static int seed = 73802;

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

    hashtable_t *table = create_hashtable(TABLE_SIZE);

    printf("Beginning Linked-Hash insertions\n\n\n");
    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i ++) {
        // printf("Word to insert in tree is %s\n", word_list[i]);
        hash_insert(table, word_list[i], word);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed1 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    printf("Beginning Linked-Hash word_list searches\n\n\n");
    /* Search for the words in the binary tree */

    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    for (int i = 0; i < counter - 1; i++) {
        get_hash(table, word_list[i], word);
    }
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed2 = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)


    /* Measuring data */
    printf("%d, NUX, Linked-Hash, non-unique strings\n", non_unique_key_counter);
    printf("%.3f, INX, Linked-Hash, seconds to insert\n", elapsed1);
    printf("%.3f, SRX, Linked-Hash, seconds to search\n", elapsed2);
    printf("%zu, MUX, Linked-Hash, memory usage\n", memory_usage);
    printf("%d, UKX, Linked-Hash, unique strings\n", unique_key_counter);
    printf("%zu, KPX, Linked-Hash, keys processed\n", keys_processed);
    printf("%d, NNX, Linked-Hash, number of nodes\n", number_of_nodes);

    fclose(file);
    return 0;
}

/* Function that creates the hash table */
hashtable_t* create_hashtable(int size) {
    hashtable_t* hash = malloc(sizeof(hashtable_t));
    hash -> buckets = calloc(size, sizeof(char**));
    hash -> table_size = size;

    return hash;
}

/* Function that takes a string as input. The function initializes the node with the string data and outputs 
 * the node address */
char* create_node(char* str) {
    int str_length = strlen(str);
    
    char* node = malloc(sizeof(char*) * (sizeof(char*) + sizeof(int) + (str_length + 1) + sizeof(int)));
    
    char* tmp = node;

    /* Insert the values of the string into the new key / node */
    *(char**)tmp = NULL; tmp += sizeof(char*);
    *(int*)tmp = str_length; tmp += sizeof(int);
    memcpy(tmp, str, str_length + 1); tmp += str_length + 1;
    *(int*)tmp = 1;

    number_of_nodes++;
    memory_usage += (sizeof(char*) + sizeof(int) + (str_length + 1) + sizeof(int));
    memory_usage += ALLOCATE_OVERHEAD;

    return node;
}


/* Function that takes a str as input, creates the key and inserts the string into the hash table */
void hash_insert(hashtable_t *table, char* str, char* buffer) {

    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the linked list head */
    char* node = table -> buckets[index];

    /* Empty bucket */
    if (node == NULL) {
        table -> buckets[index] = create_node(str);

    /* Non empty bucket */
    } else {

        char* next;
        int store_length, store_counter;
        /* FInd the correct position to insert the new key */
        char* prev;

        while (node != NULL) {
            /* Store address of previous node */
            prev = node;

            /* Get key data of the node */
            node += sizeof(char*); 

            /* Store the string and string length */
            store_length = *(int*)node; node += sizeof(int); 
            memcpy(buffer, node, store_length + 1);

            /* If the string to insert is the same as linked list, increase the counter */
            if (strcmp(buffer, str) == 0) {
                *(int*)(node + store_length + 1) += 1;
                return;
            }
            /* String comparison -> keys processed++ */
            keys_processed++;
            
            /* Go to next node */
            node = *(char**)(prev);
        }
        
        /* If the key currently isn't present in the bucket, create a new node and insert it at the end */
        *(char**)prev = create_node(str); 
    }

}

/* Function that takes a hashtable pointer, string to find and a buffer as input. The function returns the address of the 
 * node containing the string as the key if found. Otherwise, the function returns NULL if not found */
char* get_hash(hashtable_t *table, char* str, char* buffer) {
    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the node from the bucket */
    char* node = table -> buckets[index];
    char* next, *prev;

    int store_length;
    
    /* Linear search along the nodes corresponding to the bucket to find the key */
    while (node != NULL) {
        prev = node;

        node += sizeof(char*);
        
        store_length = *(int*)node; node += sizeof(int);
        memcpy(buffer, node, store_length + 1);

        /* Return the node address if the string is found in node */
        if (strcmp(buffer, str) == 0) {
             
            /* Just for checking purposes */
           // printf("The string to search is %s\n", str);
           // printf("Node data address %p ...\n", prev);
           // printf("The string is %s\n", buffer);
           // printf("The string counter is %d\n", *(int*)(node + store_length + 1));
            return prev;
        }

        /* String comparison -> Keys processed */
        keys_processed++;

        /* Go to next node of the linked list */
        node = *(char**)prev;
    }

    /* Otherwise the string is not found, return 0*/
    printf("The string is not found!\n");
    assert(0);
    return NULL;
}



/* Hash function */
unsigned int xorhash(char *word, int tsize)
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
