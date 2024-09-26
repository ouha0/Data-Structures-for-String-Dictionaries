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

#ifndef INITIAL_ARR_SIZE 
#define INITIAL_ARR_SIZE 300 // Default node size
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
// #define TABLE_SIZE (1 << 14)
// #define INITIAL_ARR_SIZE 300
/* Initial array size needs to be at least 2 * sizeof(size_t) to 
 * store housekeeping variables */

#define PRINT_TOGGLE 0



// #define WORDS_NUM HUNDRED_MILLION // Parameter to control how many words to get from text file 
// #define FILENAME "wordstream.txt"
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
char* create_array(void);

size_t get_array_size(char* array);
size_t get_array_use(char* array);
size_t get_initial_array_use(void);
size_t skip_initial_parameters(char** array_ptr);


/* Function prototypes for checking */
void check_print(hashtable_t* table, bool print, char* buffer);

/* Some information about this data structure: Note that the buckets 
 * of this hashtable store arrays. When the array becomes full, it is doubled */


/* TODO don't forget to calculate memory usage of hashtable_t. Note that lots of keys are processed, 
 * especially when you need to realloc */


static int seed = SEED;

static size_t memory_usage = 0;
static int unique_key_counter = 0;
static int non_unique_key_counter = 0;
static size_t keys_processed = 0;
static int number_of_nodes = 0;

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

    hashtable_t *table = create_hashtable(TABLE_SIZE);

    printf("Beginning Array-hash insertion and search\n");
    clock_gettime(CLOCK_MONOTONIC, &prec_start);
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
    printf("%d, TSX, Array-hash, table size\n", TABLE_SIZE);
    printf("%d, NUX, Array-hash, non-unique strings\n", non_unique_key_counter);
    printf("%.3f, INX, Array-hash, seconds to insert\n", elapsed1);
    printf("%.3f, SRX, Array-hash, seconds to search\n", elapsed2);
    printf("%zu, MUX, Array-hash, memory usage\n", memory_usage);
    printf("%d, UKX, Array-hash, unique strings\n", unique_key_counter);
    printf("%zu, KPX, Array-hash, keys processed\n", keys_processed);
    // printf("%d, NNX, Array-hash, number of nodes\n", number_of_nodes);
    printf("\n");

    fclose(file);
    return 0;
}

/* Function that creates the hash table */
hashtable_t* create_hashtable(int size) {
    hashtable_t* hash = malloc(sizeof(hashtable_t));
    hash -> buckets = calloc(size, sizeof(char**));
    hash -> table_size = size;

    /* Memory for hashtable, bucket size, integer to hold table size. And overhead */
    memory_usage += sizeof(hashtable_t) + (size * sizeof(char**)) + sizeof(int);
    memory_usage += ALLOCATE_OVERHEAD;

    return hash;
}



/* Function that takes no input. The function allocates memory for an initial size array 
 * and outputs the address of the array */
char* create_array(void) {

    /* Allocate memory for array and make sure it is non empty */
    char* tmp = malloc(sizeof(char) * INITIAL_ARR_SIZE);

    /* First 8 bytes of array is size_t array_size and size_t current_array_use */
    *(size_t*)tmp = INITIAL_ARR_SIZE; 
    *(size_t*)(tmp + sizeof(size_t)) = 2 * sizeof(size_t);

    // assert(tmp != NULL);

    memory_usage += (sizeof(char) * INITIAL_ARR_SIZE);
    memory_usage += ALLOCATE_OVERHEAD;

    number_of_nodes++;

    return tmp;
}

/* Function that takes the initial array pointer as input. The function outputs 
 * a pointer to the new array that is double the size of the original */
char* double_array(char* array) {

    /* Reallocate array to be double the original size */
    size_t array_size = get_array_size(array);
    char* tmp = realloc(array, sizeof(char*) * (array_size * 2));
    
    /* Update the array size in the doubled array */
    *(size_t*)tmp = 2 * array_size;

    memory_usage += (sizeof(char) * array_size);
    
    return tmp;
}


/* Function that takes a str as input, creates the key and inserts the string into the hash table */
void hash_insert(hashtable_t *table, char* str, char* buffer) {

    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the linked list head */
    char* array = table -> buckets[index];
    
    char* tmp_ptr; size_t offset;

    /* Empty bucket */
    if (array == NULL) {

        int tmp_length = strlen(str);
        
        /* Create an array and store the address at the bucket */
        array = create_array();
        table -> buckets[index] = array;
        
        /* Keep doubling array if too small */
        while((2 * sizeof(size_t)) + sizeof(int) + tmp_length + 1 + sizeof(int) >= 
        *(size_t*)array) {

            /* Double the array size, save the pointer to the correct bucket */
            array = double_array(array);
            table -> buckets[index] = array ;
            
            /* Move all elements of array into another array -> update keys processed */
            keys_processed += 2 * sizeof(size_t);
        }




        /* Store new key at start of array after housekeeping variables */ 
        tmp_ptr = array;
        tmp_ptr += 2 * sizeof(size_t);
        
        /* Store the key */
        *(int*)tmp_ptr = tmp_length; tmp_ptr += sizeof(int);
        memcpy(tmp_ptr, str, tmp_length + 1);
        *(int*)(tmp_ptr + tmp_length + 1) = 1;

        keys_processed++;

        /* Update the current array use */
        *(size_t*)(array + sizeof(size_t)) = get_initial_array_use() + (sizeof(int) + tmp_length + 1 + sizeof(int));


    /* Non empty bucket */
    } else {
        tmp_ptr = array;
        
        /* Save the housekeeping variables, save the offset and move the pointer accordingly */
        size_t array_size = *(size_t*)tmp_ptr; tmp_ptr += sizeof(size_t);
        size_t array_use = *(size_t*)tmp_ptr; tmp_ptr += sizeof(size_t);
        offset = 2 * sizeof(size_t);


        
        int tmp_length, tmp_counter;
        
        /* Find if the key is already stored in the array */
        while(offset < array_use) {
            tmp_length = *(int*)tmp_ptr; tmp_ptr += sizeof(int);
            memcpy(buffer, tmp_ptr, tmp_length + 1);

            /* String comparison -> Keys processed */
            keys_processed++;

            /* If there is a string match, increment the string 
             * counter and end the function */
            if (strcmp(buffer, str) == 0) {
                *(int*)(tmp_ptr + tmp_length + 1) += 1;
                return;
            }

            /* Otherwise, update the ptr as well as the offset to the next string */
            tmp_ptr += tmp_length + 1 + sizeof(int);
            offset += sizeof(int) + tmp_length + 1 + sizeof(int);
        }

        /* Just a sanity check */
        // assert(offset == array_use);

        /* The string to insert wasn't found, hence it is a new string */
        tmp_length = strlen(str);
        
        /* Make sure current array size is large enough, double if not large enough 
         * and update table. The function also updates the tmp_ptr 
         * to point to the key insertion location */

        /* Note that this function is a little slow, might as well quadruple it if we
         * know doubling it isn't enough */
        while (offset + sizeof(int) + tmp_length + 1 + sizeof(int) >= array_size) {

            /* Double the array size, save the pointer to the correct bucket */
            array = double_array(array);
            table -> buckets[index] = array ;
            
            /* Move all elements of array into another array -> update keys processed */
            keys_processed += array_use;

            array_size *= 2;
        }

        /* Move the tmp pointer to the correct offset position */
        tmp_ptr = array + offset;
        
        /* Insert the key data at the end of the array */
        *(int*)(tmp_ptr) = tmp_length; tmp_ptr += sizeof(int);
        memcpy(tmp_ptr, str, tmp_length + 1); tmp_ptr += tmp_length + 1;
        *(int*)tmp_ptr = 1;


        /* New key stored -> keys processed */
        keys_processed++;
 
        /* Update current array use */ 
        *(size_t*)(array + sizeof(size_t)) = array_use + (sizeof(int) + tmp_length + 1 + sizeof(int));
    }
}



/* Function that takes a hashtable pointer, string to find and a buffer as input. The function returns the address of the 
 * node containing the string as the key if found. Otherwise, the function returns NULL if not found */
char* get_hash(hashtable_t *table, char* str, char* buffer) {
    unsigned int index = xorhash(str, TABLE_SIZE);

    /* Get the address of the node from the bucket */
    char* array = table -> buckets[index];

    /* Bucket is emtpy. Note that this shouldn't happen */
    if (array == NULL) {
        printf("The bucket is emtpy! This shouldn't happen!\n");
        assert(0);
    /* Otherwise the bucket is non empty. Search the array to find the key match */
    } else {
        char* tmp = array;
        
        /* Think the array_size is not required */
        //size_t array_size = *(size_t*)tmp; 
        tmp += sizeof(size_t);
        
        size_t array_use = *(size_t*)tmp; tmp += sizeof(size_t);
        size_t offset = 2 * sizeof(size_t);

        int tmp_length, tmp_counter;

        /* Loop through the array to find the key */
        while(offset < array_use) {
            tmp_length = *(int*)tmp; tmp += sizeof(int);

            /* Copy the string into the buffer */
            memcpy(buffer, tmp, tmp_length + 1);
            
            /* If the string is found, 
             * print the key data, and return the address of 
             * key */
            if (strcmp(buffer, str) == 0) {

                
                /* Just for checking purposes */
                //printf("The string to search is %s\n", str);
                //printf("key data belongs to bucket address %p\n", array);
                //printf("The string is %s\n", buffer);
                //printf("The string counter is %d\n", *(int*)(tmp + tmp_length + 1));
                

                /* Return the address of the key (inside the array) */
                return (tmp - sizeof(int));
            }
            
            /* Move the temporary pointer to the next key in the array 
             * and update the offset size accordinly */
            tmp += tmp_length + 1 + sizeof(int);
            offset += sizeof(int) + tmp_length + 1 + sizeof(int);
        }

        printf("The array has been looped. The string has not been found!\n");
        assert(0);
    }
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

    /* Loop through all buckets, and free the array */
    for (int i = 0; i < table -> table_size; i++) {
        node = table -> buckets[i];
        
        /* If the bucket is non-empty, free the bucket */
        if (node != NULL)
            free(node);

    }

    /* Free the table data structure */
    free(table -> buckets);
    free(table);
}





/* Function that takes an array address (bucket holds this address), and outputs the size of the array */
size_t get_array_size(char* array) {
    return *(size_t*)array;
}

/* Function that takes an array address (bucket holds this address), and outputs the currently used space of the array */
size_t get_array_use(char* array) {
    return *(size_t*)(array + sizeof(size_t));
}


/* Function that takes an array ptr as input. Note that this pointer points to the start of the array. The function 
 * then moves the pointer past the initial housekeeping parameters and returns the total offset. */
size_t skip_initial_parameters(char** array_ptr) {
    /* Initial parameter offset */
    size_t tmp = 2 * sizeof(size_t);
    
    /* Move the array pointer */
    *array_ptr += tmp;
    return tmp;
}



/* Note: This function is just for convenience and future proofing 
 * Function that takes no input. The function outputs the array use of a
 * newly created array. */
size_t get_initial_array_use(void) {
    return (2 * sizeof(size_t));
}



/* Function that takes the hashtable_t* table, print toggle boolean and buffer 
 * as input. The function counts the unique and non-unique strings and prints 
 * the key values if chosen */
void check_print(hashtable_t* table, bool print, char* buffer) {
    char* bucket;
    size_t array_size, array_use;

    size_t offset;

    int tmp_length, tmp_counter; 

    if (print) {
        /* Iterate over all buckets to access array and print the keys */
        for (int i = 0; i < TABLE_SIZE; i++) {
            bucket = table -> buckets[i];

            /* If the bucket is not null, print the array elements */
            if (bucket != NULL) {
                array_size = *(size_t*)bucket; bucket += sizeof(size_t);
                array_use = *(size_t*)bucket; bucket += sizeof(size_t);
                offset = 2 * sizeof(size_t);

                assert(array_use < array_size);

                /* Print all keys from the array (connected to the bucket) */
                while (offset < array_use) {

                    /* Print key address */
                    printf("key data belongs to bucket address %p\n", bucket);

                    tmp_length = *(int*)(bucket); 
                    bucket += sizeof(int); offset += sizeof(int);

                    memcpy(buffer, bucket, tmp_length + 1);
                    bucket += tmp_length + 1; offset += tmp_length + 1;

                    tmp_counter = *(int*)bucket; 
                    bucket += sizeof(int); offset += sizeof(int);

                    /* Print key values */
                    printf("The string is %s\n", buffer);
                    printf("The string counter is %d\n", tmp_counter);

                    /* Update measuring variables */
                    unique_key_counter++;
                    non_unique_key_counter += tmp_counter;
                }
            }
        }

    } else {

        /* Iterate over all buckets to access array and print the keys */
        for (int i = 0; i < TABLE_SIZE; i++) {
            bucket = table -> buckets[i];

            /* If the bucket is not null, print the array elements */
            if (bucket != NULL) {
                array_size = *(size_t*)bucket; bucket += sizeof(size_t);
                array_use = *(size_t*)bucket; bucket += sizeof(size_t);
                offset = 2 * sizeof(size_t);

                assert(array_use < array_size);

                /* Print all keys from the array (connected to the bucket) */
                while (offset < array_use) {

                    tmp_length = *(int*)(bucket); 
                    bucket += sizeof(int); offset += sizeof(int);

                    memcpy(buffer, bucket, tmp_length + 1);
                    bucket += tmp_length + 1; offset += tmp_length + 1;

                    tmp_counter = *(int*)bucket; 
                    bucket += sizeof(int); offset += sizeof(int);

                    /* Update measuring variables */
                    unique_key_counter++;
                    non_unique_key_counter += tmp_counter;
                }
            }


        }
    }
}
