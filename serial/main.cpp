#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

bool visibility_line_exists(short (*data)[6000], short x1, short y1, short x2, short y2);
std::vector<std::pair<int, int>> visibility_path(int x1, int y1, int x2, int y2);
float visibility_line_slope(short starting_altitude, short ending_altitude, int path_length);
void printGrid(short (*data)[6000], int rows, int cols);

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

    printGrid(data, 10, 10);

    std::cout << visibility_line_exists(data, 0, 0, 0, 5) << std::endl;

    return 0;
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
    std::vector<std::pair<int, int>> path = visibility_path(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2));
    float slope = visibility_line_slope(data[x1][y1], data[x2][y2], path.size() - 1);
    std::cout << "Slope: " << slope << std::endl;
    float starting_altitude = static_cast<float>(data[x1][y1]);
    std::cout << "(" << path[0].first << ", " << path[0].second << ") : " << data[path[0].first][path[0].second] << std::endl;
    path.erase(path.begin());
    for (const auto& point : path) {
        std::cout << "(" << point.first << ", " << point.second << ") : " << data[point.first][point.second] << std::endl;
        if (static_cast<float>(data[point.first][point.second]) < (starting_altitude + slope)) {
            starting_altitude = starting_altitude + slope;
        }
        else {
            return false;
        }
    }

    return true;
}

float visibility_line_slope(short starting_altitude, short ending_altitude, int path_length) {
    return (ending_altitude - starting_altitude) / path_length;
}

std::vector<std::pair<int, int>> visibility_path(int x1, int y1, int x2, int y2)
{
    std::vector<std::pair<int, int>> path;

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
            path.push_back(point);

			// Print the current coordinate
			std::cout << "(" << x << "," << y << ")" << std::endl;

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
            path.push_back(point);

			// Print the current coordinate
			std::cout << "(" << x << "," << y << ")" << std::endl;

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

    return path;
}