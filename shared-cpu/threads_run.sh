#!/bin/bash
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH -o ./%j/slurmjob-%j.out-%N
#SBATCH -e ./%j/slurmjob-%j.err-%N
#SBATCH --account=usucs5030
#SBATCH --partition=kingspeak

# Parameter List <Data Size> <Number of Threads>

# Set up the run
DATA_SIZE=$1
NUM_THREADS=$2
OUTPUT_NAME="output_${DATA_SIZE}x${DATA_SIZE}_with_${NUM_THREADS}_threads.raw"
INPUT_NAME="input_${DATA_SIZE}x${DATA_SIZE}_for_nt_${NUM_THREADS}.raw"

# Generate input file with resize tool
../tools/resize ../tools/6000x6000.raw 6000 6000 $INPUT_NAME $DATA_SIZE $DATA_SIZE

# set up scratch directory
SCRDIR=/scratch/general/vast/$USER/$SLURM_JOB_ID
mkdir -p $SCRDIR

cp main.cpp $SCRDIR
cp visibility.cpp $SCRDIR
cp visibility.hpp $SCRDIR
cp Makefile $SCRDIR
cp $INPUT_NAME $SCRDIR
rm $INPUT_NAME
cd $SCRDIR

# compile the program
make

# run the program
./threaded.exe $INPUT_NAME $OUTPUT_NAME $DATA_SIZE $DATA_SIZE $NUM_THREADS

cp $OUTPUT_NAME $SLURM_SUBMIT_DIR/$SLURM_JOB_ID

# Remove the directory in sratch
rm -rf $SCRDIR