/* Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_null.h>
#include <time.h>
#include <stdbool.h>

/* Global Variables */
#define MAX_STRING_BYTES 64
#define DOUBLE_SIZE 2
#define INITIAL_COUNT 1
#define INITIAL_KEYS 0
#define INITIAL_NODE_USE 0

/* For convenience */
#define INIT_VAR_SIZE 28 //Initial node maintainence parameter offset

/* Parameter choice */
#define T_DEGREE 100
#define NODE_SIZE 500


/* Function Prototypes(main) */
char* B_tree_create(void);
void parent_node_push_up(char**, char**, char*, int, int);

/* Function Prototypes (supplementary) */
char* initialize_node(int, bool, int);
size_t skip_initialization_var(char**);
size_t get_node_capacity(char*);
size_t get_node_use(char*);

int main(int argc, char** argv) {

    return 0;
}



