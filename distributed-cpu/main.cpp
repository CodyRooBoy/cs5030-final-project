#include <fstream>
#include <chrono>
#include <cstdint>

#include "visibility.hpp"

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
    printf("\nRunning a serial program with data size %d x %d.\n", height, width);

    // Initializing input and output memory space
	std::vector<uint16_t> altitude_data(height * width);
    std::vector<uint32_t> visibility_data(height * width);

    // Reading in altitude data
    std::ifstream file(input_filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }
    file.read(reinterpret_cast<char*>(altitude_data.data()), height * width * sizeof(uint16_t));
    file.close();


    // Begining the search for visibility
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < height; i += 1) {
        // auto start_time_2 = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < width; j += 1) {
            visible_points(altitude_data, visibility_data, height, width, i, j);
            // std::cout << ".";
            // std::cout.flush();
        }
        // auto end_time_2 = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double> duration_2 = end_time_2 - start_time_2;
        // std::cout << "Time taken for row " << i << ": " << duration_2.count() << " seconds" << std::endl;
    }
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


    // Writing out visiblity data
    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error opening output file\n";
        return 1;
    }
    out_file.write(reinterpret_cast<char*>(visibility_data.data()), height * width * sizeof(uint32_t));
    out_file.close();

    return 0;
}