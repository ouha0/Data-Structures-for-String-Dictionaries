#!/bin/zsh 

# Navigate to root directory 
cd "$(dirname "$0")/.."


# Remove previous results 
rm -f ./results.txt

# Compile programs 
gcc -fsanitize=address -g B+_tree.c -o plus
gcc -fsanitize=address -g B_tree.c -o tree
gcc -fsanitize=address -g Binary_tree.c -o binary
gcc -fsanitize=address -g array_hashtable.c -o ahash
gcc -fsanitize=address -g linked_hashtable.c -o lhash

# Check if compilation was successful
if [[ $? != 0 ]]; then
  echo "Compilation error, stopping script."
  exit 1
fi

num_runs=5    

# Execute each program multiple times
echo "-------" >> results.txt
for (( i=1; i<=num_runs; i++ )); do
  echo "-------" >> results.txt
  echo "Run $i for Binary Tree" >> results.txt
  ./binary >> results.txt
  echo "Run $i for B-Tree" >> results.txt
  ./tree >> results.txt
  echo "Run $i for B+ Tree" >> results.txt
  ./plus >> results.txt
  echo "Run $i for Array-Hash" >> results.txt
  ./ahash >> results.txt
  echo "Run $i for Linked-Hash" >> results.txt
  ./lhash >> results.txt
  echo "-------" >> results.txt
done
echo "-------" >> results.txt

# Print or process results here or in another program
echo "Experiments completed. Results stored in results.txt."
