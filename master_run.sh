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
sbatch serial_run.sh 500
sbatch serial_run.sh 1000
sbatch serial_run.sh 2000

cd ../



# -------------------- Run Threaded implementation (Shared Memory Parallel)

cd ./shared-cpu
# Parameter List <Data Size> <Number of Threads>
sbatch threads_run.sh 500 4
sbatch threads_run.sh 500 8
sbatch threads_run.sh 500 16

sbatch threads_run.sh 1000 4
sbatch threads_run.sh 1000 8
sbatch threads_run.sh 1000 16

sbatch threads_run.sh 2000 4
sbatch threads_run.sh 2000 8
sbatch threads_run.sh 2000 16

cd ../



# -------------------- Run MPI implementation (Distributed Memory Parallel)

# Insert calls here

cd ./distributed-cpu
# Parameter List <Data Size> <Number of Processes>
sbatch distributed_cpu_run.sh 500 4
sbatch distributed_cpu_run.sh 500 8
sbatch distributed_cpu_run.sh 500 16

sbatch distributed_cpu_run.sh 1000 4
sbatch distributed_cpu_run.sh 1000 8
sbatch distributed_cpu_run.sh 1000 16

sbatch distributed_cpu_run.sh 2000 4
sbatch distributed_cpu_run.sh 2000 8
sbatch distributed_cpu_run.sh 2000 16

cd ../

# -------------------------------- Run GPU implementation (Non-Distributed)

cd ./shared-gpu
# Parameter List <Data Size> <Block Size>
sbatch shared_gpu_run.sh 500 5
sbatch shared_gpu_run.sh 500 15
sbatch shared_gpu_run.sh 500 32

sbatch shared_gpu_run.sh 2000 5
sbatch shared_gpu_run.sh 2000 15
sbatch shared_gpu_run.sh 2000 32

sbatch shared_gpu_run.sh 6000 5
sbatch shared_gpu_run.sh 6000 15
sbatch shared_gpu_run.sh 6000 32

cd ../



# ------------------------------------ Run GPU implementation (Distributed)

cd ./distributed-gpu
# Parameter List <Data Size> <Block Size>

# Insert calls here

cd ../



# ------------------------------------- (future?) Validate all output files

# Validation happens here