#include <fstream>
#include <chrono>
#include <cstdint>

#include "visibility.hpp"

int main(int argc, char* argv[]) {
	
    if (argc < 6) {
		std::cerr << "Missing Arguments. Format in the following order: <input_filename> <output_filename> <height> <width> <Num threads>" << std::endl;
		return 1;
	}

    // Reading in parameters
    const char* input_filename = argv[1];
    const char* output_filename = argv[2];
    int height = std::stoi(argv[3]);
	int width = std::stoi(argv[4]);
    int num_threads = std::stoi(argv[5]);

    // Printing run info for logging purposes
    printf("\nRunning a Threaded program with data size %d x %d and %d threads.\n", height, width, num_threads);

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

    // Thread stuff: Create lists of pixels for each thread to process based on the number of threads
    std::thread threads[num_threads];
    std::mutex mutex;
    std::vector<std::vector<std::pair<int, int>>> task_pixels(num_threads);
    int thread_number = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            
            task_pixels[thread_number].push_back(std::make_pair(i, j));
            
            if (thread_number == num_threads - 1) {
                thread_number = 0;
            } else {
                thread_number++;
            }
        }
    }

    // Beginning the threads
    auto start_time = std::chrono::high_resolution_clock::now();

    // Creating threads
    for (int i = 0; i < num_threads; i++) {
        threads[i] = std::thread(worker_thread, std::ref(altitude_data), std::ref(visibility_data), height, width, std::ref(mutex), task_pixels[i]);
    }
    // Joining the threads
    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
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


void worker_thread(std::vector<uint16_t> &altitude_data, std::vector<uint32_t> &visibility_data, int height, int width, std::mutex &mutex, std::vector<std::pair<int, int>> pixels) {
    
    // Build temporary data structure to hold the visibility data
	std::vector<uint32_t> local_visibility_data(height * width);
    
    for (const auto& pixel : pixels) {
        visible_points(altitude_data, local_visibility_data, height, width, pixel.first, pixel.second, mutex);
    }

    // Put the visibility data into the main visibility data structure
    {
        std::lock_guard<std::mutex> lock(mutex);
        for (int i = 0; i < height; i += 1) {
            for (int j = 0; j < width; j += 1) {
                visibility_data[j * width + i] += local_visibility_data[j * width + i];
            }
        }
    }
}