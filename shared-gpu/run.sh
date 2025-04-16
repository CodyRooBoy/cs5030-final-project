#!/bin/bash
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH --gres=gpu:1
#SBATCH -o ./%j/slurmjob-%j.out-%N
#SBATCH -e ./%j/slurmjob-%j.err-%N
#SBATCH --account=kingspeak-gpu
#SBATCH --partition=kingspeak-gpu

# Parameter List <Data Size> <Block Size>

# Set up the run
DATA_SIZE=$1
BLOCK_SIZE=$2
OUTPUT_NAME="output_${DATA_SIZE}x${DATA_SIZE}_non-dist_GPU_block_size_${BLOCK_SIZE}.raw"
INPUT_NAME="input_${DATA_SIZE}x${DATA_SIZE}_for_bs_${BLOCK_SIZE}.raw"

# Generate input file with resize tool
../tools/resize ../tools/6000x6000.raw 6000 6000 $INPUT_NAME $DATA_SIZE $DATA_SIZE

# set up scratch directory
SCRDIR=/scratch/general/vast/$USER/$SLURM_JOB_ID
mkdir -p $SCRDIR

cp main.cpp $SCRDIR
cp visibility_cuda.cu $SCRDIR
cp visibility_cuda.hpp $SCRDIR
cp Makefile $SCRDIR
cp $INPUT_NAME $SCRDIR
rm $INPUT_NAME
cd $SCRDIR

# load your module
module load cuda/12.5.0

# compile the program
make

# run the program
./non_distributed_gpu $INPUT_NAME $OUTPUT_NAME $DATA_SIZE $DATA_SIZE $BLOCK_SIZE

cp $OUTPUT_NAME $SLURM_SUBMIT_DIR/$SLURM_JOB_ID

# Remove the directory in sratch
rm -rf $SCRDIR