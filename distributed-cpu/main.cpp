#include <fstream>
#include <chrono>
#include <cstdint>

#include <mpi.h>

#include "visibility.hpp"

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);

    if (argc < 5) {
		std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width>" << std::endl;
        MPI_Finalize();
		return 1;
    }

    // Reading in parameters
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int height = std::stoi(argv[3]);
	int width = std::stoi(argv[4]);

    // Printing run info for logging purposes
    if (rank == 0) {
        printf("\nRunning MPI program with %d processes on data size %d x %d.\n", size, height, width);
    }

    // Initializing input and output memory space
	std::vector<uint16_t> altitude_data(height * width);

    if (rank == 0) {
        // Reading in altitude data
        std::ifstream file(input_filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file\n";
            return 1;
        }
        file.read(reinterpret_cast<char*>(altitude_data.data()), height * width * sizeof(uint16_t));
        file.close();
    }

    MPI_Bcast(altitude_data.data(), height * width, MPI_UINT16_T, 0, MPI_COMM_WORLD);

    int rows_per_process = height / size;
    int remainder = height % size;
    int start_row = rank * rows_per_process + std::min(rank, remainder);
    int local_rows = rows_per_process + (rank < remainder ? 1 : 0);

    std::vector<uint32_t> local_visibility(height * width, 0);

    MPI_Barrier(MPI_COMM_WORLD);

    // Begining the search for visibility
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = start_row; i < start_row + local_rows; i += 1) {
        for (int j = 0; j < width; j += 1) {
            visible_points(altitude_data, local_visibility, height, width, i, j);
        }
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> local_duration = end_time - start_time;
    double local_time = local_duration.count();
    double total_duration = 0.0;

    MPI_Reduce(&local_duration, &total_duration, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

    std::vector<uint32_t> visibility_data(height * width, 0);
    MPI_Reduce(local_visibility.data(), visibility_data.data(), height * width, MPI_UINT32_T, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        // Printing out time data
        std::chrono::duration<double> duration = end_time - start_time;
        std::cout << "Total Time: " << duration.count() << " seconds" << std::endl;

        // Writing out visiblity data
        std::ofstream out_file(output_filename, std::ios::binary);
        if (!out_file) {
            std::cerr << "Error opening output file\n";
            return 1;
        }
        out_file.write(reinterpret_cast<char*>(visibility_data.data()), height * width * sizeof(uint32_t));
        out_file.close();
    }

    MPI_Finalize();
    return 0;
}