#include <fstream>
#include <chrono>

#include "visibility.hpp"

int main() {
    const char* filename = "srtm_14_04_6000x6000_short16.raw";
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file\n";
        return 1;
    }

    short (*data)[6000] = new short[6000][6000];
    file.read(reinterpret_cast<char*>(data), 6000 * 6000 * sizeof(short));
    file.close();

    short (*output_points)[6000] = new short[6000][6000];

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 6000; i += 1) {
        auto start_time_2 = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < 6000; j += 1) {
            visible_points(data, output_points, i, j);
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

    for (int i = 0; i < 6000; ++i) {
        for (int j = 0; j < 6000; ++j) {
            int visibility_count = static_cast<int>(output_points[i][j]);
            out_file.write(reinterpret_cast<char*>(&visibility_count), sizeof(int));
        }
    }

    out_file.close();

    delete[] data;
    delete[] output_points;

    return 0;
}