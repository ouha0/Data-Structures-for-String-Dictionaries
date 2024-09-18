#!/bin/zsh

# Ensure the script is executed from the root directory of the project
cd "$(dirname "$0")/.."



results_file="results/hash_vary_words.txt"
rm -f ./$results_file

WORD_VALUES=(10000 50000 250000 1250000 6250000)
DATA=1
TABLE_SIZE=$((1<<10))
SEED=73802
INIT_ARR_SIZE=300



for WORDS in $WORD_VALUES; do
    echo "------- Running experiments with DATA=$DATA WORDS=$WORDS NSIZE=$NSIZE -------" | tee -a $results_file

    # Compile programs 
    gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D INITIAL_ARR_SIZE=$INIT_ARR_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED array_hashtable.c -o ahash
    #gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED  linked_hashtable.c -o lhash
    #gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D INITIAL_ARR_SIZE=$INIT_ARR_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED mtfarray_hashtable.c -o mtfahash
    #gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED  mtflinked_hashtable.c -o mtflhash
    

    # Check if compilation was successful
    if [[ $? != 0 ]]; then
      echo "Compilation error, stopping script."
      exit 1
    fi

    echo "Run for Array Hash with WORDS=$WORDS DATA=$DATA TABLE_SIZE=$TABLE_SIZE INIT_ARR_SIZE=$INIT_ARR_SIZE SEED=$SEED" | tee -a $results_file
    ./ahash| tee -a $results_file
    #echo "Run for Linked Hash with WORDS=$WORDS DATA=$DATA TABLE_SIZE=$TABLE_SIZE SEED=$SEED" | tee -a $results_file
    #./lhash| tee -a $results_file
    #echo "Run for MTF Array Hash with WORDS=$WORDS DATA=$DATA TABLE_SIZE=$TABLE_SIZE INIT_ARR_SIZE=$INIT_ARR_SIZE SEED=$SEED" | tee -a $results_file
    #./mtfahash| tee -a $results_file
    #echo "Run for MTF Linked Hash with WORDS=$WORDS DATA=$DATA TABLE_SIZE=$TABLE_SIZE SEED=$SEED" | tee -a $results_file
    #./mtflhash| tee -a $results_file

done
