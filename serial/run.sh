#!/bin/bash
#SBATCH --time=24:00:00
#SBATCH --nodes=1
#SBATCH -o ./%j/slurmjob-%j.out-%N
#SBATCH -e ./%j/slurmjob-%j.err-%N
#SBATCH --account=group-name
#SBATCH --partition=kingspeak

# set up scratch directory
SCRDIR=/scratch/general/vast/$USER/$SLURM_JOB_ID
mkdir -p $SCRDIR

cp main.cpp $SCRDIR
cp visibility.cpp $SCRDIR
cp visibility.h $SCRDIR
cp Makefile $SCRDIR
cp strm_14_04_6000x6000_short16.raw $SCRDIR

# compile the program
make

# run the program
./serial.exe

# copy output back
cp ./output_visibility.raw.raw $SLURM_SUBMIT_DIR/$SLURM_JOB_ID

# Remove the directory in scatch
rm -rf $SCRDIR