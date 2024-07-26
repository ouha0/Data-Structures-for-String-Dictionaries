#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define INITIAL_ARRAY_BYTES 30
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1

/* Function Prototypes */
void store_string(char*, char**, size_t*, size_t*);
void print_array_strings(char*);
int search_counter(char*, char*, size_t*, size_t*);
int check_valid_string(char*);
int insert_sorted(char*, char*, size_t*, size_t*);
void update_array_size(int*, char**, size_t*, size_t*);

/* Haven't done: String is not in sorted order. Not sure if current code is robust enough. */
// Note that you can combine the search and insertion function together. If can't find it find it alphabetically, insert it immediately

/* Problems: curr_array_use doesn't include the nullbyte at the final end. Not sure that if it would cause a problem */


int main(int argc, char** argv) {

    /* Basic timing library */
    clock_t start, end;
    double time_used;

    /* More precise timer */
    struct timespec prec_start, prec_end;
    double elapsed;

    /* Just some strings for testing... */
    char* s1 = "hello"; char* s2 = "byebye"; char* s3 = "just a test for semi long string"; char* s4 = "hello"; 
    char* s5 = "a very very very very very very very very very very very very very very very very long string";
    char* s6 = ""; char* s7 = "just a test for semi long string";

    /* Initialize array, related variables and set first byte to '\0' */
    char *array = (char*)malloc(INITIAL_ARRAY_BYTES);
    if (array == NULL){
        fprintf(stderr, "Memory allocation failed");
        return 1;
    }
    /* Set first byte of array as nullbyte */
    array[0] = '\0'; 
    size_t curr_array_capacity = INITIAL_ARRAY_BYTES; size_t curr_array_use = 0; // Note: curr_array_use doesn't include final nullbyte at end of sub-block

    /* Store test strings */
    start = clock();
    store_string(s1, &array, &curr_array_capacity, &curr_array_use);
    store_string(s2, &array, &curr_array_capacity, &curr_array_use);
    store_string(s3, &array, &curr_array_capacity, &curr_array_use);
    store_string(s4, &array, &curr_array_capacity, &curr_array_use);
    store_string(s5, &array, &curr_array_capacity, &curr_array_use);
    store_string(s6, &array, &curr_array_capacity, &curr_array_use);
    store_string(s7, &array, &curr_array_capacity, &curr_array_use);
    end = clock();
    time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    /* Check array parameters to see whether programming is working as expected. */
    printf("\nAfter store_strings, some variables:\ncurr_array_use: %d\ncurr_array_capacity: %d\n", (int)curr_array_use, (int)curr_array_capacity);
    printf("Storing the strings took %lf seconds\n", time_used);
    
    /* Print the strings of the array */
    printf("\nNow we begin printing the strings in the array\n");
    clock_gettime(CLOCK_MONOTONIC, &prec_start);
    print_array_strings(array);
    clock_gettime(CLOCK_MONOTONIC, &prec_end);
    elapsed = (prec_end.tv_sec - prec_start.tv_sec) + (prec_end.tv_nsec - prec_start.tv_nsec) / 1E9; // Number of seconds and nanoseconds (converted to seconds)
    printf("Printing the strings took %.9f seconds\n", elapsed);

    /* Free allocated array */
    free(array);
    return 0;
}



/* Function that stores strlength, string, and counter into the array */
void store_string(char* s, char** array_ptr, size_t* curr_array_capacity, size_t* curr_array_use) {

    /* Null String or Array */
    if (!s || !(*array_ptr)) {
        printf("Checking is this is running...\n");
        fprintf(stdout, "Emtpy String or Empty Array!\n");
        return;
    }

    /* Checks whether string to be stored has valid length. */
    if (!check_valid_string(s)) {
        return;
    }


    int length = strlen(s);
    size_t data_size = length + 1 + 2 * (sizeof(int)); // Size of sub-block (string length, string, counter)


    /* Search for string in the array. Increase counter if found. Otherwise append as a new string. (Note that new string has to fit, if doesn't fit, need to double array size) */
    if (search_counter(s, *array_ptr, curr_array_capacity, curr_array_use)) {
        printf("String \"%s\" found, increasing counter ...\n", s);
    }
    /* Store string as new data for the array. Double the memory size if too small */
    else {
        
        /* Double array size if new string cannot fit. Free old orray and create new one. */
        /* While loop caters for initially small array sizes. Since data size may be relatively larger than array capacity */
        while (*curr_array_use + data_size + 1 > *curr_array_capacity) { // if equal, curr_array_capacity can exactly fit 

            printf("Looks like array is full... Increasing size.\n");

            char* new_array = realloc(*array_ptr, DOUBLE_SIZE * (*curr_array_capacity)); 
            if (!new_array) {
                fprintf(stderr, "New Memory allocation unsucessful!\n");
                return;
            }
            /* Update curr_array capacity and initial array pointer in main */
            *curr_array_capacity *= 2;
            *array_ptr = new_array;
        } 

        /* Searches for the correct sub-block to insert the new string block */
        if (!insert_sorted(s, *array_ptr, curr_array_capacity, curr_array_use)) {
            fprintf(stderr ,"String not successfully inserted\n");
            return;
        }
    }
}

/* Function that checks whether a string has valid length i.e. Not empty or too large. Returns 1 if valid, 0 if invalid. */
int check_valid_string(char* s) {
    /* Discard string that is too large */
    if (strlen(s) > MAX_STRING_BYTES) {
        printf("String \"%s\" too large, skipping...\n", s);
        return 0;
    }
    
    /* Discard string that is emtpy */
    if (strlen(s) == 0) {
        printf("String \"%s\" is empty, skipping...\n", s);
        return 0;
    }
    return 1;
}



/* This function searches the array for a particular string. If the string is found, update the counter. Otherwise return 0 (string not found) */
// Think about whether to output position of the nullbyte to save some time at the end
int search_counter(char* string, char* array, size_t* curr_array_capacity, size_t* curr_array_use) {
    /* Null string or array */
    if (!string || !array) {
        fprintf(stderr, "Empty String or Emtpy Array!\n");
        return 0;
    } 

//    /* This case shouldn't happen. Before insertion, string is checked for emptiness and max bytes */
//    if (strlen(string) == 0) {
//        fprintf(stderr, "Searched string is empty\n");
//        return 0;
//    }

    /* Search for the string inside the array */
    size_t offset = 0;
    int stored_length, stored_counter; 
    char* stored_string;

    /* Iterate over all strings of the array to find a string match (case 1 doesn't never false, unless sub-block stored incorrectly)*/
    while(offset < (*curr_array_capacity) & *(array + offset) != '\0') { 

        /* Store string length, string, and counter */
        memcpy(&stored_length, array + offset, sizeof(int)); 
        offset += sizeof(int);

        stored_string = (char*)malloc(stored_length + 1);
        if (!stored_string){
            fprintf(stderr, "Memory not successfully allocated.\n");
            return 0;
        }

        memcpy(stored_string, array + offset, stored_length + 1);
        offset += stored_length + 1;

        /* If string found in the array, update the counter. Otherwise continue searching. */ 
        if (strcmp(string, stored_string) == 0) {

            /* Update the counter for the correct string in the array. */
            memcpy(&stored_counter, array + offset, sizeof(int));
            stored_counter++;
            memcpy(array + offset, &stored_counter, sizeof(int));
            return 1;
        }
        /* Otherwise, update offset */
        else {
            offset += sizeof(int);
        }
        free(stored_string);
    }
    
    /* Offset should stop at the last nullbyte */
    if (*curr_array_use != offset) {
        fprintf(stderr, "Currently used is not consistent with offset!\n");
        return 0;
    }
    /* String is not found */
    return 0; 
}


/* Function that assumes string "s" is not in the array, and inserts the new string in the correct alphabetical order. The function 
 * outputs 1 if successful insertion and 0 if unsucessful */
int insert_sorted(char* s, char* arr, size_t* curr_array_capacity, size_t* curr_array_use) {
    
    /* When null string or array */
   if (!s || !arr) {
        fprintf(stderr, "Invalid: Null String or Array\n");
        return 0;
    } 


//    /* This case shouldn't happen. Before insertion, string is checked for emptiness and max bytes */
//    if (strlen(s) == 0) {
//        fprintf(stderr, "Inserted string is empty\n");
//        return 0;
//    }

    /* While loop over strings, stop at correct "key" and insert string. Use memmove to shift everything to the right. (check if required to enlarge array size).
     * Insert the string inside the correct position. */

    /* Temporary variables for looping */
    size_t offset = 0; int tmp_stored_length, tmp_stored_counter; 
    char * tmp_string;

    /* Variables for string to be inserted */
    int insert_s_length = strlen(s); 
    size_t data_size = insert_s_length + 1 + 2 * sizeof(int); // Strlength + String with nullbyte + counter

    int tmp_cmp; // This variable will be used for refined algorithm 

    /* When array is initially empty; start of block*/
    if (arr[0] == '\0') {
        
        if (offset != *curr_array_use) {
            fprintf(stderr, "Nullbyte array. Inconsistent offset and curr_array_use\n");
            return 0;
        }

        *(int *)(arr + offset)  = insert_s_length; // treat location as an integer
        strcpy(arr + offset + sizeof(int), s); 
        *(int *)(arr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT; // treat location as an integer 
        *(arr + *curr_array_use + 2 * sizeof(int) + insert_s_length + 1) = '\0'; // Set a nullbyte at the end of the length, string, counter block
        
        /* Update currently used bytes of array */
        (*curr_array_use) += data_size;

        /* Successful insertion; End function */
        return 1;
    }

    /* Case when in between block */
    while(offset < (*curr_array_capacity) & *(arr + offset) != '\0') {

        /* Store string length and string (to be changed afterwards for speeding up) (if not found, insert at the end using curr_array_use) */
        memcpy(&tmp_stored_length, arr + offset, sizeof(int)); 
        
        tmp_string = (char*)malloc(tmp_stored_length + 1);
        if (!tmp_string){
            fprintf(stderr, "Memory not successfully allocated.\n");
            return 0;
        }
        memcpy(tmp_string, arr + offset + sizeof(int), tmp_stored_length + 1);

        /* If string to be stored is less than current sub-block string, insert it. */
        if (strcmp(s, tmp_string) < 0) {

            /* Double array size if array is too small.
             * Improve this later, currently we assume array size 
             * has already been checked. 
             * */

            /* Shift array to the right and store the string correctly */
            memmove(arr + offset + data_size, arr + offset, *curr_array_use + 1 - offset); // curr_array_use doesn't include finishing nullbyte

            *(int*)(arr + offset) = insert_s_length;
            strcpy(arr + offset + sizeof(int), s);
            *(int*)(arr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT;

            /* Free memory and update curr_array_use */
            free(tmp_string); 
            (*curr_array_use) += data_size;

            /* Function finishes after successful insertion */
            return 1;
        }

        free(tmp_string);
        offset += 2 * sizeof(int) + 1 + tmp_stored_length; // size of temporary block 

    }

    /* Insert the string at the end of the "node" */
    if (offset != *curr_array_use) { // Should be consistent
        fprintf(stderr, "Nullbyte array. Inconsistent offset and curr_array_use\n");
        return 0;
    }

    *(int *)(arr + offset)  = insert_s_length; // treat location as an integer
    strcpy(arr + offset + sizeof(int), s); 
    *(int *)(arr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT; // treat location as an integer 
    *(arr + *curr_array_use + 2 * sizeof(int) + insert_s_length + 1) = '\0'; // Set a nullbyte at the end of the length, string, counter block
    
    /* Update currently used bytes of array */
    (*curr_array_use) += data_size;

    return 1;
}

// This function has not been completed 
/* Function that checks and doubles array size of too small. */
void update_array_size(int* data_size, char** array_ptr, size_t* curr_array_capacity, size_t* curr_array_use)  {

    return;
} 




/* Print data in the array to stdout (mainly used for checking) */
void print_array_strings(char* arr) {
    if (arr == NULL) {
        fprintf(stderr, "Empty array!");
        return;
    }

    /* Temp variatbles to output data to stdout */
    int stored_length, stored_counter; 
    char *stored_string;

    size_t offset = 0;
    char* ptr = arr;
    
    // Note that memcpy is used to indicate that overlapping memory isn't the intention 
    while(*ptr != '\0') {

        /* Store the strings in temporary variables */
        memcpy(&stored_length, arr + offset, sizeof(int)); // Store length of string
        offset += sizeof(int);
        ptr += sizeof(int);

        /* Allocate memory to string */
        stored_string = (char*)malloc(stored_length + 1);
        if (!stored_string){
            fprintf(stderr, "Memory not successfully allocated.\n");
            return;
        }

        /* Storing string and String counter*/
        memcpy(stored_string, arr + offset, stored_length + 1);
        offset += stored_length + 1;
        ptr += stored_length + 1;

        memcpy(&stored_counter, arr + offset, sizeof(int));
        offset += sizeof(int);
        ptr += sizeof(int);
        
        /* Print the data to stdout (for checking mainly) */
        fprintf(stdout, "Length of string: %d\n", stored_length);
        fprintf(stdout, "String: %s\n", stored_string);
        fprintf(stdout, "String counter: %d\n", stored_counter);

        /* Free memory fore stored_string */
        free(stored_string);
    }
    
}
