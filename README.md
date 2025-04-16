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
# ./serial_run.sh <Data Size>
./serial_run.sh 1000
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
# ./threads_run.sh <Data Size> <Number of Threads>
./threads_run.sh 1000 8
```

The script will schedule a job on the Kingspeak CHPC cluster. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd threads/

make

# Command Line Format: ./threaded $INPUT_FILE_NAME $OUTPUT_FILE_NAME $WIDTH $HEIGHT $NUM_THREADS
./threaded 1000x1000.raw output_1000x1000_serial.raw 1000 1000 8
```

### Parallel CUDA GPU Implementation

### Parallel Distributed Memory CPU Implementation
The parallel distributed memory CPU implementation is in the [`distributed-cpu`](/distributed-cpu/) subdirectory. To compile and run the code on the Kingspeak CHPC cluster, run the `distributed_cpu_run.sh` Bash script along with the argument for the width and height of the output file.

```bash
cd distributed-cpu/
# ./distributed_cpu_run.sh <Data Size> <Number of Processes>
./distributed_cpu_run.sh 1000 8
```

The script will schedule a job on the Kingspeak CHPC cluster. The output file will be created in a subdirectory that is named the SLURM Job ID.

To manually run the serial implementation, use the following commands:

```bash
cd distributed-cpu/

module load gcc/8.5.0 intel-mpi

make

# Command Line Format: mpirun -np $NUM_PROCESSES ./distributed_cpu.exe $INPUT_FILE_NAME $OUTPUT_FILE_NAME $WIDTH $HEIGHT
mpirun -np 8 ./distributed_cpu.exe 1000x1000.raw output_1000x1000_serial.raw 1000 1000
```

### Distributed Memory GPU Implementation

## Our Approach

## Scaling Study

## Output Visualization