#!/bin/bash
# Script to process datasets in a loop

for k in '-name' '-size'; do
# Loop over values of A and B
for i in A B; do
  # Loop over values of threads
  for j in 1 2 3 4 5 8 16 24; do
    # Execute the old-photo-parallel-A command
    ./run_all_B.sh;
    ./old-photo-parallel-B ./Dataset-"$i" "$j" "$k";
    # Copy specific files to the Timings directory
    cp Dataset-"$i"/timi* BTimings/Dataset-"$i"
  done
done
done
# Exit the script
exit 0

