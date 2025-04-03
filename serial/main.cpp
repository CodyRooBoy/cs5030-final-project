#include <fstream>
#include <chrono>

#include "visibility.hpp"

int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cerr << "Missing Arguments. Format in the following order: <filename> <rows> <columns>" << std::endl;
		return 1;
	}

    const char* filename = argv[1];
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

	int rows = std::stoi(argv[2]);
	int columns = std::stoi(argv[3]);

	short** data = new short*[rows];
	for (int i = 0; i < rows; ++i) {
        data[i] = new short[columns];
    }

    file.read(reinterpret_cast<char*>(*data), rows * columns * sizeof(short));
    file.close();

	short** output_points = new short*[rows];
	for (int i = 0; i < rows; ++i) {
        output_points[i] = new short[columns];
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < rows; i += 1) {
        auto start_time_2 = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < columns; j += 1) {
            visible_points(data, output_points, rows, columns, i, j);
        }
        auto end_time_2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_2 = end_time_2 - start_time_2;
        std::cout << "Time taken for row " << i << ": " << duration_2.count() << " seconds" << std::endl;
    }

    auto end_time = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end_time - start_time;

    std::cout << "Time taken for visible_points: " << duration.count() << " seconds" << std::endl;

    const char* output_filename = "output_visibility.raw";
    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file) {
        std::cerr << "Error opening output file\n";
        return 1;
    }

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            int visibility_count = static_cast<int>(output_points[i][j]);
            out_file.write(reinterpret_cast<char*>(&visibility_count), sizeof(int));
        }
    }

    out_file.close();

    delete[] data;
    delete[] output_points;

    return 0;
}