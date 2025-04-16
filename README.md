# CS 5030 Final Project
## Team Members - Brandon Herrin and Enoch Shumway

## CHPC Instructions
**Prerequisites for All Run Instructions:** Clone the Git Repository and add the project dataset [`srtm_14_04_6000x6000_short16.raw`](https://usu.instructure.com/courses/778161/files/93555732/download?wrap=1) to the [`tools`](/tools/) subdirectory and rename the file to `6000x6000.raw`.

```bash
# Clone Git Repository
git clone https://github.com/CodyRooBoy/cs5030-final-project.git
cd cs5030-final-project/
```

Each implementation can be run on their own, but the entire scaling study can be run through the `master_run.sh` Bash script.

```bash
# Change the permissions on the Bash script to make it executable
chmod +x ./master_run.sh
# Run the Bash script
./master_run.sh
```

### Serial CPU Implementation
The serial CPU implementation is in the [`serial`](/serial/) subdirectory. To compile and run the code on the Kingspeak CHPC cluster, run the `serial_run.sh` Bash script along with the argument for the width and height of the output file.

```bash
cd serial/
# sbatch serial_run.sh <Data Size>
sbatch serial_run.sh 1000
```

The script will schedule a job on the Kingspeak CHPC cluster. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd serial/

make

# Command Line Format: ./serial.exe $INPUT_FILE_NAME $OUTPUT_FILE_NAME $WIDTH $HEIGHT
./serial.exe 1000x1000.raw output_1000x1000_serial.raw 1000 1000
```

### Parallel Shared Memory CPU Implementation
The parallel shared memory CPU implementation is in the [`threads`](/threads/) subdirectory. To compile and run the code on the Kingspeak CHPC cluster, run the `threads_run.sh` Bash script along with the argument for the width and height of the output file.

```bash
cd threads/
# sbatch threads_run.sh <Data Size> <Number of Threads>
sbatch threads_run.sh 1000 8
```

The script will schedule a job on the Kingspeak CHPC cluster. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd threads/

make

# Command Line Format: ./threaded $INPUT_FILE_NAME $OUTPUT_FILE_NAME $WIDTH $HEIGHT $NUM_THREADS
./threaded 1000x1000.raw output_1000x1000_threaded.raw 1000 1000 8
```

### Parallel CUDA GPU Implementation
The parallel CUDA GPU implementation is in the [`shared-gpu`](/shared-gpu/) subdirectory. To compile and run the code on the CHPC, run the `shared_gpu_run.sh` Bash script along with the argument for the width and height of the output file and the block size for the GPU to use.

```bash
cd shared-gpu/
# sbatch shared_gpu_run.sh <Data Size> <Block Size>
sbatch shared_gpu_run.sh 2000 5
```

The script will schedule a job on the CHPC. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd shared-gpu/

module load cuda/12.5.0

make

# Command Line Format: ./non_distributed_gpu $INPUT_NAME $OUTPUT_NAME $DATA_SIZE $DATA_SIZE $BLOCK_SIZE
./non_distributed_gpu 1000x1000.raw output_1000x1000_non-dist_GPU_block_size_32.raw 1000 1000 32
```

### Parallel Distributed Memory CPU Implementation
The parallel distributed memory CPU implementation is in the [`distributed-cpu`](/distributed-cpu/) subdirectory. To compile and run the code on the Kingspeak CHPC cluster, run the `distributed_cpu_run.sh` Bash script along with the argument for the width and height of the output file.

```bash
cd distributed-cpu/
# ./distributed_cpu_run.sh <Data Size> <Number of Processes>
sbatch distributed_cpu_run.sh 1000 8
```

The script will schedule a job on the Kingspeak CHPC cluster. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd distributed-cpu/

module load gcc/8.5.0 intel-mpi

make

# Command Line Format: mpirun -np $NUM_PROCESSES ./distributed_cpu.exe $INPUT_FILE_NAME $OUTPUT_FILE_NAME $WIDTH $HEIGHT
mpirun -np 8 ./distributed_cpu.exe 1000x1000.raw output_1000x1000_mpi.raw 1000 1000
```

### Distributed Memory GPU Implementation

## Our Approach

## Scaling Study

## Output Visualization