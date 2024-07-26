#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define INITIAL_ARRAY_BYTES 1
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1

/* Function Prototypes */
void store_string(char*, char**, size_t*, size_t*);
void print_array_strings(char*);
int check_valid_string(char*);
int insert_sorted(char*, char**, size_t*, size_t*);
void update_array_size(size_t*, char**, size_t*, size_t*);

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
    // Note: curr_array_capacity includes nullbyte at end but curr_array_use doesn't 
    size_t curr_array_capacity = INITIAL_ARRAY_BYTES; size_t curr_array_use = 0; 

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

    /* Function to insert the string at the correct position of the "node" / array. Double size 
     * of array if required. */ 
    if (!insert_sorted(s, array_ptr, curr_array_capacity, curr_array_use)) {
        fprintf(stderr ,"String not successfully inserted\n");
        return;
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


/* Function that assumes string "s" is not in the array, and inserts the new string in the correct alphabetical order. The function 
 * outputs 1 if successful insertion and 0 if unsucessful */

// Note that updating array size requires a double pointer 
int insert_sorted(char* s, char** arr_ptr, size_t* curr_array_capacity, size_t* curr_array_use) {
    
    /* When null string or array */
   if (!s || !(*arr_ptr)) {
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

    int tmp_cmp; // Used for strcmp 

    /* When array is initially empty; start of block */
    if (**arr_ptr == '\0') {
        
        if (offset != *curr_array_use) {
            fprintf(stderr, "Nullbyte array. Inconsistent offset and curr_array_use\n");
            return 0;
        }
        /* Update array size if required */
        update_array_size(&data_size, arr_ptr, curr_array_capacity, curr_array_use);
        
        *(int *)(*arr_ptr + offset)  = insert_s_length; // treat location as an integer
        strcpy(*arr_ptr + offset + sizeof(int), s); 
        *(int *)(*arr_ptr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT; // treat location as an integer 
        *(*arr_ptr + *curr_array_use + 2 * sizeof(int) + insert_s_length + 1) = '\0'; // Set a nullbyte at the end of the length, string, counter block
        
        /* Update currently used bytes of array */
        (*curr_array_use) += data_size;

        /* Successful insertion; End function */
        return 1;
    }

    /* Case when in between block */
    while(offset < (*curr_array_capacity) & *(*arr_ptr + offset) != '\0') {

        /* Store string length and string (to be changed afterwards for speeding up) (if not found, insert at the end using curr_array_use) */
        memcpy(&tmp_stored_length, *arr_ptr + offset, sizeof(int)); 
        
        tmp_string = (char*)malloc(tmp_stored_length + 1);
        if (!tmp_string){
            fprintf(stderr, "Memory not successfully allocated.\n");
            return 0;
        }
        memcpy(tmp_string, *arr_ptr + offset + sizeof(int), tmp_stored_length + 1);

        /* If string to be stored is less than current sub-block string, insert it. */
        if ((tmp_cmp = strcmp(s, tmp_string)) < 0) {

            /* Update array size if required */
            update_array_size(&data_size, arr_ptr, curr_array_capacity, curr_array_use);

            /* Shift array to the right and store the string correctly */
            memmove(*arr_ptr + offset + data_size, *arr_ptr + offset, *curr_array_use + 1 - offset); // curr_array_use doesn't include finishing nullbyte

            *(int*)(*arr_ptr + offset) = insert_s_length;
            strcpy(*arr_ptr + offset + sizeof(int), s);
            *(int*)(*arr_ptr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT;

            /* Free memory and update curr_array_use */
            free(tmp_string); 
            (*curr_array_use) += data_size;

            /* Function finishes after successful insertion */
            return 1;

        // Copied string is equal to inserted string 
        } else if (tmp_cmp == 0) { 

            /* Update string counter at correct offset position */
            int* counter_ptr = (int*)(*arr_ptr + offset + sizeof(int) + tmp_stored_length + 1);
            (*counter_ptr)++;

            /* Free memory */
            free(tmp_string);
            return 1;

        // When offset not at correct position of sub-block, go to next block via while loop
        } else {

        /* Free memory, update offset, and continue while loop */
        free(tmp_string);
        offset += 2 * sizeof(int) + 1 + tmp_stored_length; // size of temporary block 
        }
    }

    /* Sanity check that offset is at the final nullbyte */ 
    if (offset != *curr_array_use) { // Should be consistent
        fprintf(stderr, "Nullbyte array. Inconsistent offset and curr_array_use\n");
        return 0;
    }

    /* Update array size if required */
    update_array_size(&data_size, arr_ptr, curr_array_capacity, curr_array_use);

    /* Insert string at the end of the "node" */
    *(int *)(*arr_ptr + offset)  = insert_s_length; // treat location as an integer
    strcpy(*arr_ptr + offset + sizeof(int), s); 
    *(int *)(*arr_ptr + offset + sizeof(int) + insert_s_length + 1) = INITIAL_COUNT; // treat location as an integer 
    *(*arr_ptr + *curr_array_use + 2 * sizeof(int) + insert_s_length + 1) = '\0'; // Set a nullbyte at the end of the length, string, counter block
    
    /* Update currently used bytes of array */
    (*curr_array_use) += data_size;

    return 1;
}

/* Function that checks and doubles array size of too small. */
void update_array_size(size_t* data_size, char** array_ptr, size_t* curr_array_capacity, size_t* curr_array_use)  {
    
    while(*curr_array_use + *data_size + 1 > *curr_array_capacity) {
        printf("Looks like array is full... Doubling size.\n");

        /* Double size of array and copy block. Update relevant variables */
        char* new_array = realloc(*array_ptr, DOUBLE_SIZE * (*curr_array_capacity)); 
        if (!new_array) {
            fprintf(stderr, "New Memory allocation unsucessful!\n");
            return;
        }
        /* Update curr_array capacity and initial array pointer in main */
        *curr_array_capacity *= 2;
        *array_ptr = new_array;
    }

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
