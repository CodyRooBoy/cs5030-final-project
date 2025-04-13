#include <fstream>
#include <chrono>
#include <iostream>
#include "visibility_cuda.hpp"

int main(int argc, char* argv[]) {
	
    if (argc < 5) {
		std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width> <block w/h>" << std::endl;
		return 1;
	}

    // Reading in parameters
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int height = std::stoi(argv[3]);
	int width = std::stoi(argv[4]);
    int block_w_h = std::stoi(argv[5]);

    // Printing run info for logging purposes
    printf("\nRunning a distributed memory GPU program with data size %d x %d.\n", height, width);

    // Initializing input and output memory space
	uint16_t* altitude_data = new uint16_t[height * width];
    int* visibility_data = new int[height * width]();

    // Reading in altitude data
    std::ifstream file(input_filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    file.read(reinterpret_cast<char*>(altitude_data), height * width * sizeof(uint16_t));
    file.close();

    // // Printing out the altitude_data
    // for (int x = 0; x < 10; x++) {
    //     for (int y = 0; y < 10; y++) {
    //         std::cout << altitude_data[y + x * width] << " ";
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";

    // Build out offset data structure
    Point* offset_pairs = pixelList_offset();

    // Running the GPU visibility search
    auto start_time = std::chrono::high_resolution_clock::now();

    int left_offset = 0;
    dimensions from_points_dim = {width, height};
    dimensions altitude_data_dim = {width, height};
    dimensions block_size = {block_w_h, block_w_h};
    run_visibility_search(altitude_data, altitude_data_dim, left_offset, from_points_dim, offset_pairs, block_size, visibility_data);
    auto end_time = std::chrono::high_resolution_clock::now();

    // Printing out time data
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Total Time: " << duration.count() << " seconds" << std::endl;

    // Printing out the visibility_data
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            std::cout << visibility_data[x + (y * width)] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";


    // puttin the visibilty data into a uint32_t array
    uint32_t* visibility_data_32 = new uint32_t[height * width];
    for (int i = 0; i < height * width; i++) {
        visibility_data_32[i] = static_cast<uint32_t>(visibility_data[i]);
    }

    // Writing out visiblity data
    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error opening output file\n";
        return 1;
    }
    out_file.write(reinterpret_cast<char*>(visibility_data_32), height * width * sizeof(uint32_t));
    out_file.close();

    return 0;
}
