#!/bin/zsh

# Ensure the script is executed from the root directory of the project
cd "$(dirname "$0")/.."



results_file="results/B_tree_vary_words.txt"
rm -f ./$results_file

WORD_VALUES=(10000 50000 250000 1250000 6250000)
DATA=1
NSIZE=4096



for WORDS in $WORD_VALUES; do
    echo "------- Running experiments with DATA=$DATA WORDS=$WORDS NSIZE=$NSIZE -------" | tee -a $results_file

    # Compile programs 
    #gcc -fsanitize=address -g -D NODE_SIZE=$NSIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS B+_tree.c -o plus
    #gcc -fsanitize=address -g -D NODE_SIZE=$NSIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS B_tree.c -o tree
    gcc -fsanitize=address -g -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS Binary_tree.c -o binary
    

    # Check if compilation was successful
    if [[ $? != 0 ]]; then
      echo "Compilation error, stopping script."
      exit 1
    fi

    #echo "Run for B-Tree with WORDS=$WORDS DATA=$DATA NSIZE=$NSIZE" | tee -a $results_file
    #./tree | tee -a $results_file
    #echo "Run for B+-tree with WORDS=$WORDS DATA=$DATA NSIZE=$NSIZE" | tee -a $results_file
    #./plus| tee -a $results_file
    echo "Run for Binary tree with WORDS=$WORDS" | tee -a $results_file
    ./binary| tee -a $results_file
    echo "-------" | tee -a $results_file

done
