#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <chrono>

bool visibility_line_exists(short (*data)[6000], short x1, short y1, short x2, short y2);
bool visibility_path(short (*data)[6000], float slope, int x1, int y1, int x2, int y2);
float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2);
void printGrid(short (*data)[6000], int rows, int cols);
short visible_points(short (*data)[6000], short (*output_points)[6000], short x1, short y1);
std::vector<std::pair<int, int>> pixelList(int x0, int y0, int maxX, int maxY, int radius = 100);

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

    short x1, y1;
    std::cout << "Enter x1: ";
    std::cin >> x1;
    std::cout << "Enter y1: ";
    std::cin >> y1;

    auto start_time = std::chrono::high_resolution_clock::now();

    // std::cout << "Visible points: " << visible_points(data, output_points, x1, y1) << std::endl;

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

short visible_points(short (*data)[6000], short (*output_points)[6000], short x1, short y1) {
    short visible_points = 0;
    // for (int i = y1; i < y1 + 100 && i < 6000; ++i) {
    //     for (int j = x1 + 1; j < x1 + 100 && j < 6000; ++j) {
    //         if (visibility_line_exists(data, x1, y1, j, i)) {
    //             output_points[x1][y1]++;
    //             output_points[j][i]++;
    //             visible_points++;
    //         }
    //     }
    // }

    // for (int i = y1 + 1; i < y1 + 100 && i < 6000; ++i) {
    //     for (int j = x1 - 1; j >= x1 - 100 && j >= 0; --j) {
    //         if (visibility_line_exists(data, x1, y1, j, i)) {
    //             output_points[x1][y1]++;
    //             output_points[j][i]++;
    //             visible_points++;
    //         }
    //     }
    // }

    std::vector<std::pair<int, int>> pixels = pixelList(x1, y1, 6000, 6000, 100);
    for (const auto& pixel : pixels) {
        if (visibility_line_exists(data, x1, y1, pixel.first, pixel.second)) {
            output_points[x1][y1]++;
            output_points[pixel.first][pixel.second]++;
            visible_points++;
        }
    }

    return visible_points;
}

void printGrid(short (*data)[6000], int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            std::cout << data[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

bool visibility_line_exists(short (*data)[6000], short x1, short y1, short x2, short y2) {
    float slope = visibility_line_slope(data[x1][y1], data[x2][y2], x1, y1, x2, y2);
    // std::cout << "Slope: " << slope << std::endl;
    return(visibility_path(data, slope, static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2)));
    // float starting_altitude = static_cast<float>(data[x1][y1]);
    // std::cout << "(" << path[0].first << ", " << path[0].second << ") : " << data[path[0].first][path[0].second] << std::endl;
    // path.erase(path.begin());
    // for (const auto& point : path) {
    //     // std::cout << "(" << point.first << ", " << point.second << ") : " << data[point.first][point.second] << std::endl;
    //     if (static_cast<float>(data[point.first][point.second]) < (starting_altitude + slope)) {
    //         starting_altitude = starting_altitude + slope;
    //     }
    //     else {
    //         return false;
    //     }
    // }

    // return true;
}

float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2) {
    // technically this does not return a float, potentially change later but this is a good enough approximation for right now
    return (ending_altitude - starting_altitude) / sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

std::vector<std::pair<int, int>> pixelList(int x0, int y0, int maxX, int maxY, int radius) {
    std::vector<std::pair<int, int>> pixels;

    for (int x = x0; x <= std::min(x0 + radius, maxX - 1); x++) {
        for (int y = y0; y <= std::min(y0 + radius, maxY - 1); y++) {
            if (x == x0 && y <= y0) continue;

            if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= radius * radius) {
                pixels.emplace_back(x, y);
            }
        }
    }

    return pixels;
}

bool visibility_path(short (*data)[6000], float slope, int x1, int y1, int x2, int y2)
{
    std::pair<int, int> point;
    short altitude = data[x1][y1];

	// Compute the differences between start and end points
	int dx = x2 - x1;
	int dy = y2 - y1;

	// Absolute values of the change in x and y
	const int abs_dx = abs(dx);
	const int abs_dy = abs(dy);

	// Initial point
	int x = x1;
	int y = y1;

	// Proceed based on the absolute differences to support all octants
	if (abs_dx > abs_dy)
	{
		// If the line is moving to the left, set dx accordingly
		int dx_update;
		if (dx > 0)
		{
			dx_update = 1;
		}
		else
		{
			dx_update = -1;
		}

		// Calculate the initial decision parameter
		int p = 2 * abs_dy - abs_dx;

		// Draw the line for the x-major case
		for (int i = 0; i <= abs_dx; i++)
		{
            std::pair<int, int> point = std::make_pair(x, y);
            // std::cout << point.first << " " << point.second << std::endl;
            if (x != x1 && y != y1) {
                if (static_cast<float>(data[point.first][point.second]) < (altitude + slope)) {
                    altitude = altitude + slope;
                }
                else {
                    return false;
                }
            }

			// Print the current coordinate
			// std::cout << "(" << x << "," << y << ")" << std::endl;

			// Threshold for deciding whether or not to update y
			if (p < 0)
			{
				p = p + 2 * abs_dy;
			}
			else
			{
				// Update y
				if (dy >= 0)
				{
					y += 1;
				}
				else
				{
					y += -1;
				}

				p = p + 2 * abs_dy - 2 * abs_dx;
			}

			// Always update x
			x += dx_update;
		}
	}
	else
	{
		// If the line is moving downwards, set dy accordingly
		int dy_update;
		if (dy > 0)
		{
			dy_update = 1;
		}
		else
		{
			dy_update = -1;
		}

		// Calculate the initial decision parameter
		int p = 2 * abs_dx - abs_dy;

		// Draw the line for the y-major case
		for (int i = 0; i <= abs_dy; i++)
		{
            std::pair<int, int> point = std::make_pair(x, y);
            // std::cout << point.first << " " << point.second << std::endl;
            if (x != x1 && y != y1) {
                if (static_cast<float>(data[point.first][point.second]) < (altitude + slope)) {
                    altitude = altitude + slope;
                }
                else {
                    return false;
                }
            }

			// Print the current coordinate
			// std::cout << "(" << x << "," << y << ")" << std::endl;

			// Threshold for deciding whether or not to update x
			if (p < 0)
			{
				p = p + 2 * abs_dx;
			}
			else
			{
				// Update x
				if (dx >= 0)
				{
					x += 1;
				}
				else
				{
					x += -1;
				}

				p = p + 2 * abs_dx - 2 * abs_dy;
			}

			// Always update y
			y += dy_update;
		}
	}

    return true;
}