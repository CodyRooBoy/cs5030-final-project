#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>


int main(int argc, char* argv[]) {

    if (argc < 5) {
        std::cout << "Usage:\n";
        std:: cout << "\tParameter list -->  <input file name> <input file name> <height> <width>\n\n";
        exit(1);
    }

    std::string input_name_1 = argv[1];
    std::string input_name_2 = argv[2];
    int height = std::stoi(argv[3]);
    int width = std::stoi(argv[4]);
    int input_file_size = height * width;


    // Open first input file
    std::ifstream input_file_1(input_name_1, std::ios::binary); 
    if (!input_file_1) {
        std::cerr << "Error opening file.\n";
        return 1;
    }
    // Read in first input file
    std::vector<uint16_t> input_1_data(input_file_size);
    input_file_1.read(reinterpret_cast<char*>(input_1_data.data()), input_file_size * 2);

    // Open second input file
    std::ifstream input_file_2(input_name_2, std::ios::binary); 
    if (!input_file_2) {
        std::cerr << "Error opening file.\n";
        return 1;
    }
    // Read in second input file
    std::vector<uint16_t> input_2_data(input_file_size);
    input_file_2.read(reinterpret_cast<char*>(input_2_data.data()), input_file_size * 2);

    // Close input files
    input_file_1.close();
    input_file_2.close();


    // Compare files
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            if (input_1_data[y + x * width] != input_2_data[y + x * width]) {
                std::cout << input_name_1 << " VS " << input_name_2 << " --> Discrepancy found!" << std::endl;
                return 1;
            }
        }
    }

    std::cout << input_name_1 << " VS " << input_name_2 << " --> They are identical!" << std::endl;
    return 0;
}
