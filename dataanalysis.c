#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>


#define ONE_MILLION 1000000
#define TEN_MILLION 10000000
#define HUNDRED_MILLION 100000000
#define ALL 1000000000

#define WORDS_NUM ALL // Parameter to control how many words to get from text file 
//#define FILENAME "wordstream.txt"
//#define FILENAME "wikipedia_with_cap.txt"
#define FILENAME "wikicap_without_common.txt"

/* Global Variables */
#define MAX_STRING_BYTES 64
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


    file = fopen(FILENAME, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return 1;
    }

    /* Data structure to store number of words of length < MAX_STRING_BYTES */
    size_t* data_count = calloc(sizeof(size_t) * MAX_STRING_BYTES, sizeof(size_t));

    /* Insert all words into word_list  */
    while(fscanf(file, "%s", word) == 1) {

        /* Only store strings that aren't too large */
        if ((str_length = strlen(word)) < MAX_STRING_BYTES) {

            /* Create data count */
            data_count[str_length]++;
            
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



    /* Print data analysis of Wikipedia words */
    // Fix the printing format of this, this kinda looks ugly
    for (int i = 0; i < MAX_STRING_BYTES; i++) {
        // printf("There are %-7zu words of length %-2d\n", data_count[i], i);
        printf("(%d, %zu)  ", i, data_count[i]);
    }

    free(data_count);

    return 0;

}
