/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define INITIAL_ARRAY_BYTES 1
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1

/* Function Prototypes */

/* Plan:    
 * Create empty tree
 * B-tree search
 * B-tree split child
 * B-tree insert and B-tree insert non-full
 * Not yet: B-tree delete key
 *
 * */

/* Problems:
 * Need to think of node structure: num_keys (probably not using), is_leaf (bool is size_t 4, same as int)
 *
 *
 *
 *
 * */


int main(int argc, char** argv) {
    //printf("bool true and false is size %zu and %zu\n", sizeof(true), sizeof(false));

    return 0;
}

/* Function that builds an empty B-tree (empty root node). */
char* B_tree_create(){
    return NULL;
}

