#!/bin/bash


# ----------------------------------------------- Run Serial implementation

cd ./serial

# Parameter List <Data Size>
sbatch run.sh 500
sbatch run.sh 1000
sbatch run.sh 2000

cd ../



# -------------------- Run Threaded implementation (Shared Memory Parallel)

cd ./threaded
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

cd ./gpu-non-distributed
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

cd ./gpu-distributed
# Parameter List <Data Size> <Block Size>

# Insert calls here

cd ../