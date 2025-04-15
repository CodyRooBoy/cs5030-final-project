#include <fstream>
#include <chrono>
#include <iostream>
#include <cmath>
#include <utility>
#include "visibility_cuda.hpp"
#include <mpi.h>
 
std::pair<int, int> closest_factors(int N);

int main(int argc, char* argv[]) {
   
    if (argc < 5) {
        std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width> <block w/h>" << std::endl;
        return 1;
    }
 
    MPI_Init(NULL, NULL);
    int rank, num_processes;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &num_processes);
 
    // Rank 0 use items
    const char* input_filename;
    const char* output_filename = argv[2];
    uint16_t* complete_altitude_data;
    int* visibility_data;
    Point* process_data_start_points = new Point[num_processes];
    dimensions* process_data_dimensions = new dimensions[num_processes];
 
    // All Ranks need
    int complete_data_height;
    int complete_data_width;
    int block_w_h;

    Point my_from_points_section_xy;
    dimensions my_from_points_dim;
    dimensions general_from_points_dim; // The largest the from_points_dim can be, my_from_points_dim contains the used dimensions and might be smaller
    int my_left_offset;

    dimensions my_altitude_data_dim;
    uint16_t* my_altitude_data;

    int* my_visibility_data;



    // Reading in input data and distributing the necessary bits
 
    if (rank == 0) {
        // Reading in parameters
        input_filename = argv[1];
        // output_filename = argv[2];
        complete_data_height = std::stoi(argv[3]);
        complete_data_width = std::stoi(argv[4]);
        block_w_h = std::stoi(argv[5]);
 
        // Initializing input and output memory space
        complete_altitude_data = new uint16_t[complete_data_height * complete_data_width];
        visibility_data = new int[complete_data_height * complete_data_width]();
 
        // Reading in altitude data
        std::ifstream file(input_filename, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file\n";
            return 1;
        }
        file.read(reinterpret_cast<char*>(complete_altitude_data), complete_data_height * complete_data_width * sizeof(uint16_t));
        file.close();
 
        // Printing run info for logging purposes
        printf("\nRunning a distributed memory GPU program with data size %d x %d and %d processes.\n", complete_data_height, complete_data_width, num_processes);

        // Send out input dimentions
        for (int i = 1; i < num_processes; i++) {
            MPI_Send(&complete_data_height, 1, MPI_INT, i, 0, comm);
            MPI_Send(&complete_data_width, 1, MPI_INT, i, 0, comm);
            MPI_Send(&block_w_h, 1, MPI_INT, i, 0, comm);
        }
    }

    // Receiving input data
    if (rank != 0) {
        MPI_Recv(&complete_data_height, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
        MPI_Recv(&complete_data_width, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
        MPI_Recv(&block_w_h, 1, MPI_INT, 0, 0, comm, MPI_STATUS_IGNORE);
    }

    // Figure out how to split the data up
    // Bias towards the processes being stacked vertically
    std::pair<int,int> factors = closest_factors(num_processes);
    int num_processes_x = std::min(factors.first, factors.second);
    int num_processes_y = std::max(factors.first, factors.second);
 
    // Calculate how big each section is
    general_from_points_dim.x_width = ceil((float)complete_data_width / num_processes_x);
    general_from_points_dim.y_height = ceil((float)complete_data_height / num_processes_y);
 
    // Calculate all the data ranges for each rank. If you are rank 0, gather the data and send it to the right rank
    for (int a_data_y = num_processes_y - 1; a_data_y >= 0; a_data_y--) {
        for (int a_data_x = num_processes_x - 1; a_data_x >= 0; a_data_x--) {
           
            int calculated_rank = (a_data_x + (a_data_y * num_processes_x));
           
            if (rank == calculated_rank || rank == 0) {
                   
                // Calculate the from_point location for this process
                my_from_points_section_xy.x = general_from_points_dim.x_width * a_data_x;
                my_from_points_section_xy.y = general_from_points_dim.y_height * a_data_y;
                // Save it for Rank 0
                if (rank == 0) {
                    process_data_start_points[calculated_rank] = my_from_points_section_xy;
                }
 
                // Calculate the dimensions of the from_point section
                my_from_points_dim.x_width = std::min(general_from_points_dim.x_width, complete_data_width - my_from_points_section_xy.x);
                my_from_points_dim.y_height = std::min(general_from_points_dim.y_height, complete_data_height - my_from_points_section_xy.y);
                // Save it for Rank 0
                if (rank == 0) {
                    process_data_dimensions[calculated_rank] = my_from_points_dim;
                }
 
                // Calculate the altitude data section XY
                Point altitude_data_section_xy;
                altitude_data_section_xy.x = std::max(my_from_points_section_xy.x - 100, 0);
                altitude_data_section_xy.y = my_from_points_section_xy.y;
                
                // Calculate the dimensions of the altitude data section
                my_altitude_data_dim.x_width = std::min(my_from_points_dim.x_width + 200, complete_data_width - altitude_data_section_xy.x);
                my_altitude_data_dim.y_height = std::min(my_from_points_dim.y_height + 100, complete_data_height - altitude_data_section_xy.y);
                
                // Calculate how far to the left the from_point section is
                my_left_offset = my_from_points_section_xy.x - altitude_data_section_xy.x;
               
                // Initialize the altitude data structure
                int my_altitude_data_size = my_altitude_data_dim.x_width * my_altitude_data_dim.y_height;
                my_altitude_data = new uint16_t[my_altitude_data_size];
 
                if (rank == 0) {
                    // Gather the data according to the currently calculated parameters and send it to the calculated rank
 
                    for (int y = 0; y < my_altitude_data_dim.y_height; y++) {
                        for (int x = 0; x < my_altitude_data_dim.x_width; x++) {
                            // my_altitude_data[y * my_altitude_data_dim.x_width + x] =  complete_altitude_data[(altitude_data_section_xy.y * complete_data_width + altitude_data_section_xy.x) + (y * complete_data_width + x + altitude_data_section_xy.x)]; // (outside) + (inside)
                            my_altitude_data[x + (y * my_altitude_data_dim.x_width)] = complete_altitude_data[(altitude_data_section_xy.x + x) + ((altitude_data_section_xy.y + y) * complete_data_width)];
                        }
                    }

                    // printf("\nRank %d Data:\n", calculated_rank);
                    // for (int y = 0; y < 10; y++) {
                    //     for (int x = 0; x < 10; x++) {
                    //         std::cout << my_altitude_data[x + (y * my_altitude_data_dim.x_width)] << " ";
                    //     }
                    //     std::cout << "\n";
                    // }
                    // std::cout << "\n";
                   
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
        // printf("Rank %d got it all okay!\n", rank);
    }

    // // Print out everything received for rank 1
    // if (rank == 3) {

    //     printf("\nRank %d Data:\n", rank);
    //     for (int y = 0; y < 10; y++) {
    //         for (int x = 0; x < 10; x++) {
    //             std::cout << my_altitude_data[x + (y * my_altitude_data_dim.x_width)] << " ";
    //         }
    //         std::cout << "\n";
    //     }
    //     std::cout << "\n";

    //     printf("my_altitude_data_dim: %d x %d\n", my_altitude_data_dim.x_width, my_altitude_data_dim.y_height);
    //     printf("my_from_points_dim: %d x %d\n", my_from_points_dim.x_width, my_from_points_dim.y_height);
    //     printf("my_left_offset: %d\n", my_left_offset);
    //     printf("my_from_points_section_xy: %d x %d\n", my_from_points_section_xy.x, my_from_points_section_xy.y);
    // }
    
 
    // define a visibility data structure for this process
    my_visibility_data = new int[my_altitude_data_dim.x_width * my_altitude_data_dim.y_height]();
 
    // Build out offset data structure
    Point* offset_pairs = pixelList_offset();
 
    // Running the GPU visibility search
    MPI_Barrier(comm);
    auto start_time = std::chrono::high_resolution_clock::now();
 
    dimensions block_size = {block_w_h, block_w_h};
    run_visibility_search(my_altitude_data, my_altitude_data_dim, my_left_offset, my_from_points_dim, offset_pairs, block_size, my_visibility_data, rank);
    printf("Rank %d finished running the visibility search\n", rank);
    MPI_Barrier(comm);
    fflush(stdout);
    
    if (rank == 0) {
        auto end_time = std::chrono::high_resolution_clock::now();
 
        // Printing out time data
        std::chrono::duration<double> duration = end_time - start_time;
        std::cout << "Total Time: " << duration.count() << " seconds" << std::endl;
    }

    // Rank 0 adds its visibility data to the visibility data array
    if (rank == 0) {
        for (int y = 0; y < my_altitude_data_dim.y_height; y++) {
            for (int x = 0; x < my_altitude_data_dim.x_width; x++) {
                visibility_data[(my_from_points_section_xy.x + x) + ((my_from_points_section_xy.y + y) * complete_data_width)] = my_visibility_data[x + (y * my_altitude_data_dim.x_width)];
            }
        }
    }

    // Send back the visibility data to rank 0
    if (rank != 0) {
        // Send back the visibility data you calculated
        MPI_Send(my_visibility_data, my_altitude_data_dim.x_width * my_altitude_data_dim.y_height, MPI_INT, 0, 0, comm);
    }
    
    printf("Is this even happening? Rank %d\n", rank);
    fflush(stdout);

    // Rank 0 receives the visibility data from all other ranks
    if (rank == 0) {
        for (int i = 1; i < num_processes; i++) {
            // Calculate the size of the data to receive
            int my_altitude_data_size = process_data_dimensions[i].x_width * process_data_dimensions[i].y_height;
            // Allocate space for the data
            int* my_visibility_data = new int[my_altitude_data_size]();
            // Receive the data
            printf("Rank %d receiving data from rank %d\n", rank, i);
            MPI_Recv(my_visibility_data, my_altitude_data_size, MPI_INT, i, 0, comm, MPI_STATUS_IGNORE);
 
            // Copy the data into the correct location in the visibility data array
            for (int y = 0; y < process_data_dimensions[i].y_height; y++) {
                for (int x = 0; x < process_data_dimensions[i].x_width; x++) {
                    visibility_data[(process_data_start_points[i].x + x) + ((process_data_start_points[i].y + y) * complete_data_width)] = my_visibility_data[x + (y * process_data_dimensions[i].x_width)];
                }
            }
        }
    }

    printf("What about this? Rank %d\n", rank);
    MPI_Barrier(comm);
    fflush(stdout);
 
    // Puttin' the visibilty data into a uint32_t array
    if (rank == 0) {
        uint32_t* visibility_data_32 = new uint32_t[complete_data_height * complete_data_width];
        for (int i = 0; i < complete_data_height * complete_data_width; i++) {
            visibility_data_32[i] = static_cast<uint32_t>(visibility_data[i]);
        }

        // Writing out visiblity data
        printf("Writing the visibility data to file called %s\n", output_filename);
        fflush(stdout);
        std::ofstream out_file(output_filename, std::ios::binary);
        if (!out_file) {
            std::cerr << "Error opening output file\n";
            return 1;
        }
        out_file.write(reinterpret_cast<char*>(visibility_data_32), complete_data_height * complete_data_width * sizeof(uint32_t));
        out_file.close();
    }
    fflush(stdout);
    MPI_Barrier(comm);

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