#include <fstream>
#include <chrono>
#include <iostream>
#include <cmath>
#include <utility>
#include "visibility_cuda.hpp"
// #include <mpi.h>
 
std::pair<int, int> closest_factors(int N);
 
struct data_info {
   
}
 
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
    uint16_t* complete_altitude_data;
    int* visibility_data;
 
    // All Ranks need
    int block_w_h;
    Point my_from_points_section_xy;
    dimensions my_from_points_dim;
    dimensions my_altitude_data_dim;
    int my_left_offset;
    uint16_t* my_altitude_data;
    dimensions general_from_points_dim;
    int* my_visibility_data;
 
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
    general_from_points_dim.x_width = ceil((float)input_width / num_processes_x);
    general_from_points_dim.y_height = ceil((float)input_height / num_processes_y);
 
    // Calculate all the data ranges for each rank. If you are rank 0, gather the data and send it to the right rank
    for (int a_data_y = num_processes_y - 1; a_data_y >= 0; a_data_y--) {
        for (int a_data_x = num_processes_x - 1; a_data_x >= 0; a_data_x--) {
           
            int calculated_rank = (a_data_x + (a_data_y * num_processes_x));
           
            if (rank == calculated_rank || rank == 0) {
                   
                // Calculate the from_point location for this process
                my_from_points_section_xy.x = general_from_points_dim.x_width * a_data_x;
                my_from_points_section_xy.y = general_from_points_dim.y_height * a_data_y;
 
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
               
                int my_altitude_data_size = my_altitude_data_dim.x_width * my_altitude_data_dim.y_height;
                my_altitude_data = new uint16_t[my_altitude_data_size];
 
                if (rank == 0) {
                    // Gather the data according to the currently calculated parameters and send it to the calculated rank
 
                    for (int y = 0; y < my_altitude_data_dim.y_height; y++) {
                        for (int x = 0; x < my_altitude_data_dim.x_width; x++) {
                            // my_altitude_data[y * my_altitude_data_dim.x_width + x] =  complete_altitude_data[(altitude_data_section_xy.y * input_width + altitude_data_section_xy.x) + (y * input_width + x + altitude_data_section_xy.x)]; // (outside) + (inside)
                            my_altitude_data[x + (y * my_altitude_data_dim.x_width)] = complete_altitude_data[(altitude_data_section_xy.x + x) + ((altitude_data_section_xy.y + y) * input_width)];
                        }
                    }
                   
                    // Unless the rank is 0, in which case just calculate your own data and don't send it to anyone
                    if (calculated_rank != 0) {
                        // Send info to calculated rank
                        MPI_Send(my_altitude_data, my_altitude_data_size, MPI_UINT16_T, calculated_rank, 0, comm);
                    }
                }
            }
        }
    }
 
    // All other processes receive the data
    if (rank != 0) {
        MPI_Recv(my_altitude_data, my_altitude_data_dim.x_width * my_altitude_data_dim.y_height, MPI_UINT16_T, 0, 0, comm, MPI_STATUS_IGNORE);
    }
 
    // define a visibility data structure for
    my_visibility_data = new int[my_altitude_data_dim.x_width * my_altitude_data_dim.y_height]();

    exit(0);
 
    // Build out offset data structure
    Point* offset_pairs = pixelList_offset();
 
    // Running the GPU visibility search
    // auto start_time = std::chrono::high_resolution_clock::now();
 
    dimensions block_size = {block_w_h, block_w_h};
    run_visibility_search(my_altitude_data, my_altitude_data_dim, my_left_offset, my_from_points_dim, offset_pairs, block_size, my_visibility_data);
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
 
    if (rank == 0) {
        // Put my_visibility_data into the visibility_data structure
    }
    // Send back the visibility data
 
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