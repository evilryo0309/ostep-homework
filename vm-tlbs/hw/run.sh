#!/bin/bash

# Set number of the loops
TRIALS=1000000

echo "Pages, Time(ns)"

# Number of pages starts from 1 and doubles each time until 8192
for (( pages=1; pages<=8192; pages*=2 ))
do
    # Run the TLB simulation and print the results
    # Use taskset to bind the process to a specific CPU core (e.g., core 0) for more consistent timing results
    taskset -c 0 ./tlb $pages $TRIALS
done
