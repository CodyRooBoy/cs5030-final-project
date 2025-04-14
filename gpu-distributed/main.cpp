#include <fstream>
#include <chrono>
#include <iostream>
#include <cmath>
#include <utility>
#include "visibility_cuda.hpp"
// #include <mpi.h>

std::pair<int, int> closest_factors(int N);

int main(int argc, char* argv[]) {
	
    if (argc < 5) {
		std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width> <block w/h>" << std::endl;
		return 1;
	}


    MPI_Init(&argc, &argv);
    int rank, num_processes;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_processes);


    // Rank 0 use items
    const char* input_filename;
    const char* output_filename;
    int input_height;
    int input_width;
    int block_w_h;
    uint16_t* complete_altitude_data;
    int* visibility_data;

    if (rank == 0) {
        // Reading in parameters
        input_filename = argv[1];
        output_filename = argv[2];
        input_height = std::stoi(argv[3]);
        input_width = std::stoi(argv[4]);
        block_w_h = std::stoi(argv[5]);

        // Initializing input and output memory space
        complete_altitude_data = new uint16_t[input_height * input_width];
        visibility_data = new int[input_height * input_width]();

        // Reading in altitude data
        std::ifstream file(input_filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file\n";
            return 1;
        }
        file.read(reinterpret_cast<char*>(complete_altitude_data), input_height * input_width * sizeof(uint16_t));
        file.close();


        // // Printing out the altitude_data
        // for (int x = 0; x < 10; x++) {
        //     for (int y = 0; y < 10; y++) {
        //         std::cout << altitude_data[y + x * width] << " ";
        //     }
        //     std::cout << "\n";
        // }
        // std::cout << "\n";


        // Printing run info for logging purposes
        printf("\nRunning a distributed memory GPU program with data size %d x %d and %d.\n", input_height, input_width, num_processes);

        // Broadcast the input info
    }

    // Figure out how to split the data up
    // Bias towards the processes being stacked vertically
    std::pair<int,int> factors = closest_factors(num_processes);
    int num_processes_x = std::min(factors.first, factors.second);
    int num_processes_y = std::max(factors.first, factors.second);

    // Calculate how big each section is
    dimensions general_from_points_dim;
    general_from_points_dim.x_width = ceil((float)input_width / num_processes_x);
    general_from_points_dim.y_height = ceil((float)input_height / num_processes_y);
    
    Point my_from_points_section_xy;
    dimensions my_from_points_dim;
    dimensions my_altitude_data_dim;
    int my_left_offset;
    uint16_t* my_altitude_data;

    // i COULD just send this all out instead...
    // Build out the altitude data structures
    for (int y = 0; y < num_processes_y; y++) {
        for (int x = 0; x < num_processes_x; x++) {
            if (rank == (x + (y * num_processes_x))) {
                    
                // Calculate the from_point location for this process
                my_from_points_section_xy.x = general_from_points_dim.x_width * x;
                my_from_points_section_xy.y = general_from_points_dim.y_height * y;

                // Calculate the dimensions of the from_point section
                my_from_points_dim.x_width = std::min(general_from_points_dim.x_width, input_width - my_from_points_section_xy.x);
                my_from_points_dim.y_height = std::min(general_from_points_dim.y_height, input_height - my_from_points_section_xy.y);

                // Calculate the beginning and end of the altitude data
                Point altitude_data_section_xy;
                altitude_data_section_xy.x = std::max(my_from_points_section_xy.x - 100, 0);
                altitude_data_section_xy.y = my_from_points_section_xy.y;

                my_altitude_data_dim.x_width = std::min(my_from_points_dim.x_width + 200, input_width - altitude_data_section_xy.x);
                my_altitude_data_dim.y_height = std::min(my_from_points_dim.y_height + 100, input_height - altitude_data_section_xy.y);

                my_left_offset = my_from_points_section_xy.x - altitude_data_section_xy.x; // DOUBLE CHECK ALL THESE VALUES

                my_altitude_data = new uint16_t[my_altitude_data_dim.x_width * my_altitude_data_dim.y_height];
            }
        }
    }

    // Rank 0 figures that the same stuff for each process and sends it out to all of them
    // Build out the altitude data structures
    if (rank == 0) {
        for (int y = 0; y < num_processes_y; y++) {
            for (int x = 0; x < num_processes_x; x++) {

                Point from_points_section_xy;
                dimensions altitude_data_dim;
                int left_offset;
                uint16_t* altitude_data_to_send;
                    
                // Calculate the from_point location for this process
                from_points_section_xy.x = general_from_points_dim.x_width * x;
                from_points_section_xy.y = general_from_points_dim.y_height * y;

                // Calculate the dimensions of the from_point section
                my_from_points_dim.x_width = std::min(general_from_points_dim.x_width, input_width - my_from_points_section_xy.x);
                my_from_points_dim.y_height = std::min(general_from_points_dim.y_height, input_height - my_from_points_section_xy.y);

                // Calculate the altitude data section xy
                Point altitude_data_section_xy;
                altitude_data_section_xy.x = std::max(from_points_section_xy.x - 100, 0);
                altitude_data_section_xy.y = from_points_section_xy.y;

                altitude_data_dim.x_width = std::min(my_from_points_dim.x_width + 200, input_width - altitude_data_section_xy.x);
                altitude_data_dim.y_height = std::min(my_from_points_dim.y_height + 100, input_height - altitude_data_section_xy.y);

                left_offset = from_points_section_xy.x - altitude_data_section_xy.x; // DOUBLE CHECK ALL THESE VALUES

                altitude_data_to_send = new uint16_t[altitude_data_dim.x_width * altitude_data_dim.y_height];
                // Copy the data into the new array
                for (int i = 0; i < my_altitude_data_dim.y_height; i++) {
                    for (int j = 0; j < my_altitude_data_dim.x_width; j++) {
                        my_altitude_data[j + (i * altitude_data_dim.x_width)] = complete_altitude_data[(altitude_data_section_xy.x + j) + ((altitude_data_section_xy.y + i) * input_width)];
                        // altitude_data_to_send[i * altitude_data_dim.x_width + j] = complete_altitude_data[(input_width * altitude_data_section_xy.y + altitude_data_section_xy.x) + (j * input_width + i + altitude_data_section_xy.x)] // Outside + Inside
                    }
                }

                int rank_to_send_to = (x + (y * num_processes_x));
                // Broadcast the data to the specific process
                if (rank_to_send_to == 0)
                    std::memcpy(my_altitude_data, altitude_data_to_send, sizeof(uint16_t) * my_altitude_data_dim.x_width * my_altitude_data_dim.y_height);
                else {
                    MPI_Send(altitude_data_to_send, altitude_data_dim.x_width * altitude_data_dim.y_height, MPI_UINT16_T, rank_to_send_to, 0, comm);
                    delete[] altitude_data_to_send;
                }
            }
        }
    }

    // All other processes receive the data
    if (rank != 0) {
        MPI_Recv(my_altitude_data, my_altitude_data_dim.x_width * my_altitude_data_dim.y_height, MPI_UINT16_T, 0, 0, comm, MPI_STATUS_IGNORE);
    }

    visibility_data = new int[my_altitude_data_dim.x_width * my_altitude_data_dim.y_height]();

    exit(0);

    // Build out offset data structure
    Point* offset_pairs = pixelList_offset();

    // Running the GPU visibility search
    // auto start_time = std::chrono::high_resolution_clock::now();

    dimensions block_size = {block_w_h, block_w_h};
    run_visibility_search(my_altitude_data, my_altitude_data_dim, my_left_offset, my_from_points_dim, offset_pairs, block_size, visibility_data);
    auto end_time = std::chrono::high_resolution_clock::now();

    // Printing out time data
    // std::chrono::duration<double> duration = end_time - start_time;
    // std::cout << "Total Time: " << duration.count() << " seconds" << std::endl;

    // // Printing out the visibility_data
    // for (int y = 0; y < 10; y++) {
    //     for (int x = 0; x < 10; x++) {
    //         std::cout << visibility_data[x + (y * width)] << " ";
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";


    // Puttin' the visibilty data into a uint32_t array
    uint32_t* visibility_data_32 = new uint32_t[input_height * input_width];
    for (int i = 0; i < input_height * input_width; i++) {
        visibility_data_32[i] = static_cast<uint32_t>(visibility_data[i]);
    }

    // Writing out visiblity data
    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error opening output file\n";
        return 1;
    }
    out_file.write(reinterpret_cast<char*>(visibility_data_32), input_height * input_width * sizeof(uint32_t));
    out_file.close();

    return 0;
}


std::pair<int, int> closest_factors(int N) {
    int a = static_cast<int>(std::sqrt(N));

    // Search downward from sqrt(N) until we find a divisor
    while (N % a != 0) {
        --a;
    }

    int b = N / a;
    return {a, b};
}