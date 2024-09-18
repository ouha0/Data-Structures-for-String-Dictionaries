#!/bin/zsh 

# Navigate to root directory 
cd "$(dirname "$0")/.."


# Remove previous results 
results_file="results/tree_results.txt"

WORDS=200000
DATA=3
NSIZE=512

# Compile programs 
gcc -fsanitize=address -g -D NODE_SIZE=$NSIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS B+_tree.c -o plus
gcc -fsanitize=address -g -D NODE_SIZE=$NSIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS B_tree.c -o tree
gcc -fsanitize=address -g -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS Binary_tree.c -o binary

# Check if compilation was successful
if [[ $? != 0 ]]; then
  echo "Compilation error, stopping script."
  exit 1
fi

num_runs=3

# Execute each program multiple times
echo "-------" >> $results_file
for (( i=1; i<=num_runs; i++ )); do
  echo "-------" >> $results_file
  echo "Run $i for Binary Tree" >> $results_file
  ./binary >> $results_file
  #echo "Run $i for B-Tree" >> $results_file
  #./tree >> $results_file
  #echo "Run $i for B+ Tree" >> $results_file
  #./plus >> $results_file
  echo "-------" >> $results_file
done
echo "-------" >> $results_file

# Print or process results here or in another program
./scripts/average.awk $results_file 

echo "Experiments completed. Results stored in $results_file"
