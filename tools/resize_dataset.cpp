#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {

    if (argc < 7) {
        std::cout << "Usage:\n";
        std:: cout << "\tParameter list -->  <input file name> <input height> <input width> <output file name> <output height> <output width>\n\n";
        exit(1);
    }

    std::string input_name = argv[1];
    int input_height = std::stoi(argv[2]);
    int input_width = std::stoi(argv[3]);
    std::string output_name = argv[4];
    int output_height = std::stoi(argv[5]);
    int output_width = std::stoi(argv[6]);


    // Open input file
    std::ifstream input_file(input_name, std::ios::binary); 
    if (!input_file) {
        std::cerr << "Error opening file.\n";
        return 1;
    }

    // Read in input file
    int input_file_size = input_height * input_width;
    std::vector<short> input_data(input_file_size);
    input_file.read(reinterpret_cast<char*>(input_data.data()), input_file_size * 2);


    // // Print out 10x10 matrix of input data
    // for (int x = 0; x < 10; x++) {
    //     for (int y = 0; y < 10; y++) {
    //         printf("%d ", input_data.at(y + x * input_width));
    //     }
    //     printf("\n");
    // }


    // Build smaller data structure
    int output_file_size = output_height * output_width;
    std::vector<short> output_data(output_file_size);

    // Fill output data structure with subsections of input data
    for (int x = 0; x < output_height; x++) {
        for (int y = 0; y < output_width; y++) {
            int input_location = y + x * input_width;
            int output_location = y + x * output_width;
            output_data[output_location] = input_data[input_location];
        }
    }

    // // Print out 10x10 matrix of output data
    // printf("\n");
    // for (int x = 0; x < 10; x++) {
    //     for (int y = 0; y < 10; y++) {
    //         printf("%d ", output_data[y + x * output_width]);
    //     }
    //     printf("\n");
    // }


    // Place output data in file
    std::ofstream output_file(output_name);
    if (!output_file) {
        std::cerr << "Error opening file for writing.\n";
        return 1;
    }
    output_file.write(reinterpret_cast<const char*>(output_data.data()), output_file_size);


    return 0;
}
