#!/bin/bash

make clean
make

grid_sizes=("32" "64" "128" "256" "512" "1024" "2048" "4096")

#### Sequential strategy ####
current_line="seq"
echo "Running experiments for sequential strategy..."
for grid_size in "${grid_sizes[@]}"; do
    for i in {1..15}; do
        result=$(./time_test --grid_size $grid_size --impl seq)
        current_line="$current_line,$result"
    done
done
echo "Finished experiments for sequential strategy!"

# Append results to file:
echo $current_line >> results.txt    
       
#### Parallelized strategies ####
versions=("omp", "pth")
num_threads=("1" "2" "4" "8" "16" "32")

for version in "${versions[@]}"; do
    for grid_size in "${grid_sizes[@]}"; do
        for threads in "${num_threads[@]}"; do
            
            current_line_label="$version g=$grid_size t=$threads"
            current_line="$current_line_label"
            
            # Run same experiment 15 times:
            for i in {1..15}; do
                result=$(./time_test --grid_size $grid_size --impl $version --num_threads $threads)
                current_line="$current_line,$result"
            done
            
            # Append results to file:
            echo $current_line >> results.txt
        done
    done
done
