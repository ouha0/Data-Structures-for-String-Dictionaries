#!/bin/zsh 

# Navigate to root directory 
cd "$(dirname "$0")/.."


# Remove previous results 
rm -f ./hash_results.txt

WORDS=1000000
DATA=2
TABLE_SIZE=$((1<<10))
SEED=73802
INIT_ARR_SIZE=300

# Compile programs 

gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D INITIAL_ARR_SIZE=$INIT_ARR_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED array_hashtable.c -o ahash
gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED  linked_hashtable.c -o lhash
gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D INITIAL_ARR_SIZE=$INIT_ARR_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED mtfarray_hashtable.c -o mtfahash
gcc -fsanitize=address -g -D TABLE_SIZE=$TABLE_SIZE -D DATASET_TYPE=$DATA -D WORDS_NUM=$WORDS -D SEED=$SEED  mtflinked_hashtable.c -o mtflhash

# Check if compilation was successful
if [[ $? != 0 ]]; then
  echo "Compilation error, stopping script."
  exit 1
fi

num_runs=3

# Should include output parameters of data structure into stdout / txt file 
# Execute each program multiple times
echo "-------" >> hash_results.txt
for (( i=1; i<=num_runs; i++ )); do
  echo "-------" >> hash_results.txt
  echo "Run $i for Array-Hash" >> hash_results.txt
  ./ahash >> hash_results.txt
  echo "Run $i for Linked-Hash" >> hash_results.txt
  ./lhash >> hash_results.txt
  echo "Run $i for Move-to-Front Array-Hash" >> hash_results.txt
  ./mtfahash >> hash_results.txt
  echo "Run $i for Move-to-Front Linked-Hash" >> hash_results.txt
  ./mtflhash >> hash_results.txt
  echo "-------" >> hash_results.txt
done
echo "-------" >> hash_results.txt

# Print or process results here or in another program
echo "Experiments completed. Results stored in hash_results.txt."
