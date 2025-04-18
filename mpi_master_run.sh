#!/bin/bash


# --------------------------------------- Build resize and validation tools

cd ./tools

# check if 6000x6000.raw exists
if [ ! -f "6000x6000.raw" ]; then
    echo "6000x6000.raw not found in tools directory... EXITING"
    exit 1
fi

g++ resize_dataset.cpp -o resize
g++ validate_datasets.cpp -o validate

cd ../



# -------------------- Run MPI implementation (Distributed Memory Parallel)

cd ./distributed-cpu
# Parameter List <Data Size> <Number of Processes>
sbatch --nodes=2 distributed_cpu_run.sh 500 4
sbatch --nodes=2 distributed_cpu_run.sh 500 8
sbatch --nodes=2 distributed_cpu_run.sh 500 16

sbatch --nodes=2 distributed_cpu_run.sh 1000 4
sbatch --nodes=2 distributed_cpu_run.sh 1000 8
sbatch --nodes=2 distributed_cpu_run.sh 1000 16

sbatch --nodes=2 distributed_cpu_run.sh 2000 4
sbatch --nodes=2 distributed_cpu_run.sh 2000 8
sbatch --nodes=2 distributed_cpu_run.sh 2000 16

cd ../