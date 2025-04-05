#!/bin/bash
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH -o ./%j/slurmjob-%j.out-%N
#SBATCH -e ./%j/slurmjob-%j.err-%N
#SBATCH --account=usucs5030
#SBATCH --partition=kingspeak

# set up scratch directory
SCRDIR=/scratch/general/vast/$USER/$SLURM_JOB_ID
mkdir -p $SCRDIR

cp main.cpp $SCRDIR
cp visibility.cpp $SCRDIR
cp visibility.hpp $SCRDIR
cp Makefile $SCRDIR
cp 1000x1000.raw $SCRDIR
cd $SCRDIR

# compile the program
make

# run the program
./serial.exe 1000x1000.raw output_visibility.raw 1000 1000

cp ./output_visibility.raw $SLURM_SUBMIT_DIR/$SLURM_JOB_ID

# Remove the directory in sratch
rm -rf $SCRDIR