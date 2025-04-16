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



# ----------------------------------------------- Run Serial implementation

cd ./serial

# Parameter List <Data Size>
sbatch run.sh 500
sbatch run.sh 1000
sbatch run.sh 2000

cd ../



# -------------------- Run Threaded implementation (Shared Memory Parallel)

cd ./shared-cpu
# Parameter List <Data Size> <Number of Threads>
sbatch run.sh 500 4
sbatch run.sh 500 8
sbatch run.sh 500 16

sbatch run.sh 1000 4
sbatch run.sh 1000 8
sbatch run.sh 1000 16

sbatch run.sh 2000 4
sbatch run.sh 2000 8
sbatch run.sh 2000 16

cd ../



# -------------------- Run MPI implementation (Distributed Memory Parallel)

# Insert calls here










# -------------------------------- Run GPU implementation (Non-Distributed)

cd ./shared-gpu
# Parameter List <Data Size> <Block Size>
sbatch run.sh 500 5
sbatch run.sh 500 15
sbatch run.sh 500 32

sbatch run.sh 2000 5
sbatch run.sh 2000 15
sbatch run.sh 2000 32

sbatch run.sh 6000 5
sbatch run.sh 6000 15
sbatch run.sh 6000 32

cd ../



# ------------------------------------ Run GPU implementation (Distributed)

cd ./distributed-gpu
# Parameter List <Data Size> <Block Size>

# Insert calls here

cd ../



# ------------------------------------- (future?) Validate all output files

# Validation happens here