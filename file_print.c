
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
    
    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {

        /* Only store strings that aren't too large */
        if (strlen(word) < MAX_STRING_BYTES) {
            /* Print strings to stdout */
            printf("%s\n", word);
            counter++;
            
        }

        /* Store the specified number of strings only */
        if (counter > WORDS_NUM) {
            break;
        }
    }


    fclose(file);

    return 0;
}
