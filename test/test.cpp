#include <fstream>
#include <chrono>
#include <iostream>
#include <cmath>
#include <utility>
#include "visibility_cuda.hpp"
// #include <mpi.h>

std::pair<int, int> closest_factors(int N);

int main(int argc, char* argv[]) {
	
    int input_height = 298;
    int input_width = 298;
    int block_w_h = 5;
    int num_processes = 4;

    printf("\nRunning a distributed memory GPU program with data size %d x %d.\n", input_height, input_width);
    printf("Number of processes: %d\n\n", num_processes);

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

            printf("This is rank %d\n", x + (y * num_processes_x));

            // Calculate the from_point location for this process
            my_from_points_section_xy.x = general_from_points_dim.x_width * x;
            my_from_points_section_xy.y = general_from_points_dim.y_height * y;

            printf("\t From_point section XY: %d,%d\n", my_from_points_section_xy.x, my_from_points_section_xy.y);

            // Calculate the dimensions of the from_point section
            my_from_points_dim.x_width = std::min(general_from_points_dim.x_width, input_width - my_from_points_section_xy.x);
            my_from_points_dim.y_height = std::min(general_from_points_dim.y_height, input_height - my_from_points_section_xy.y);

            printf("\t From_point section dim: %d,%d\n", my_from_points_dim.x_width, my_from_points_dim.y_height);

            // Calculate the beginning and end of the altitude data
            Point altitude_data_section_xy;
            altitude_data_section_xy.x = std::max(my_from_points_section_xy.x - 100, 0);
            altitude_data_section_xy.y = my_from_points_section_xy.y;

            printf("\t Altitude data section XY: %d,%d\n", altitude_data_section_xy.x, altitude_data_section_xy.y);

            my_altitude_data_dim.x_width = std::min(my_from_points_dim.x_width + 200, input_width - altitude_data_section_xy.x);
            my_altitude_data_dim.y_height = std::min(my_from_points_dim.y_height + 100, input_height - altitude_data_section_xy.y);

            printf("\t Altitude data section dim: %d,%d\n", my_altitude_data_dim.x_width, my_altitude_data_dim.y_height);

            my_left_offset = my_from_points_section_xy.x - altitude_data_section_xy.x; // DOUBLE CHECK ALL THESE VALUES

            printf("\t Left offset: %d\n", my_left_offset);

            my_altitude_data = new uint16_t[my_altitude_data_dim.x_width * my_altitude_data_dim.y_height];

            printf("\n\n\n");
        }
    }


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