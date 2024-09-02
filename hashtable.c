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

/* Parameter choice */
#define TABLE_SIZE (1 << 10)
#define WORDS_NUM ONE_MILLION // Parameter to control how many words to get from text file 
#define FILENAME "wordstream.txt"
// #define FILENAME "wikipedia_with_cap.txt"


/* Hash table structure */
typedef struct{
    char** buckets;
    int* bucket_size;
    int table_size;
}hashtable_t;

unsigned int xorhash(char *word, int tsize);
hashtable_t* create_hashtable(int size);


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





    fclose(file);
    return 0;
}

/* Function that creates the hash table */
hashtable_t* create_hashtable(int size) {
    hashtable_t* hash = malloc(sizeof(hashtable_t));
    hash -> buckets = malloc(sizeof(char**) * size);
    hash -> bucket_size = calloc(size, sizeof(int));
    hash -> table_size = size;

    /* Initialize all buckets to be NULL pointer */
    for (int i = 0; i < size; i++) {
        hash -> buckets[i] = NULL;
    }


    return hash;
}


/* Function that takes a str as input, creates the key and inserts the string into the hash table */
void insert(hashtable_t *table, char* str) {

    unsigned int index = xorhash(str, TABLE_SIZE);

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



