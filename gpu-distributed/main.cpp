#include <fstream>
#include <chrono>
#include <cstdint>
#include <iostream>
#include "visibility_cuda.hpp"

int main(int argc, char* argv[]) {
	
    if (argc < 5) {
		std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width>" << std::endl;
		return 1;
	}

    // Reading in parameters
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int height = std::stoi(argv[3]);
	int width = std::stoi(argv[4]);

    // Printing run info for logging purposes
    printf("\nRunning a distributed memory GPU program with data size %d x %d.\n", height, width);

    // Initializing input and output memory space
	uint16_t* altitude_data = new uint16_t[height * width];
    uint16_t* visibility_data = new uint16_t[height * width]();

    // Reading in altitude data
    std::ifstream file(input_filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    file.read(reinterpret_cast<char*>(altitude_data), height * width * sizeof(uint16_t));
    file.close();

    // Build out offset data structure
    Point* offset_pairs = pixelList_offset();

    // Beginning the threads
    auto start_time = std::chrono::high_resolution_clock::now();

    run_visibility_search(altitude_data, width * height, 0, {width, height}, {width, height}, offset_pairs, {32, 32}, visibility_data);
    
    auto end_time = std::chrono::high_resolution_clock::now();

    
    // Printing out time data
    std::chrono::duration<double> duration = end_time - start_time;
    std::cout << "Total Time: " << duration.count() << " seconds" << std::endl;

    // // Printing out the visibility_data
    // for (int x = 0; x < 30; x++) {
    //     for (int y = 0; y < 30; y++) {
    //         std::cout << visibility_data.at(y + x * width) << " ";
    //     }
    //     std::cout << "\n";
    // }
    // std::cout << "\n";


    // // Writing out visiblity data
    // std::ofstream out_file(output_filename, std::ios::binary);
    // if (!out_file) {
    //     std::cerr << "Error opening output file\n";
    //     return 1;
    // }
    // out_file.write(reinterpret_cast<char*>(visibility_data.data()), height * width * sizeof(uint32_t));
    // out_file.close();

    return 0;
}
