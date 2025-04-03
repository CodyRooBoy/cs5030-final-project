#include "visibility.hpp"

short visible_points(short **data, short **output_points, int rows, int columns, short x1, short y1) {
    short visible_points = 0;

    std::vector<std::pair<int, int>> pixels = pixelList(x1, y1, rows, columns, 100);
    for (const auto& pixel : pixels) {
        if (visibility_line_exists(data, x1, y1, pixel.first, pixel.second)) {
            output_points[x1][y1]++;
            output_points[pixel.first][pixel.second]++;
            visible_points++;
        }
    }

    return visible_points;
}

bool visibility_line_exists(short **data, short x1, short y1, short x2, short y2) {
    float slope = visibility_line_slope(data[x1][y1], data[x2][y2], x1, y1, x2, y2);
    return(visibility_path(data, slope, static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2)));
}

bool visibility_path(short **data, float slope, int x1, int y1, int x2, int y2)
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
            if (x != x1 && y != y1 && x != x2 && y != y2) {
                if ((data[point.first][point.second]) < (altitude + slope)) {
                    altitude = altitude + slope;
                }
                else {
                    return false;
                }
            }

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
            if (x != x1 && y != y1 && x != x2 && y != y2) {
                if ((data[point.first][point.second]) < (altitude + slope)) {
                    altitude = altitude + slope;
                }
                else {
                    return false;
                }
            }

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

float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2) {
    return (ending_altitude - starting_altitude) / sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

std::vector<std::pair<int, int>> pixelList(int x0, int y0, int maxX, int maxY, int radius) {

    // Create a place to store pixel values
    std::vector<std::pair<int, int>> pixels;

    // Calculate the bounds of the relevant pixel box
    int starting_y = std::max(y0 - radius, 0);
    int stopping_y = std::min(radius + y0 + 1, maxY);
    int starting_x = x0;
    int stopping_x = std::min(x0 + radius, maxX);

    // std::cout << "starting_y: " << starting_y << std::endl;
    // std::cout << "stopping_y: " << stopping_y << std::endl;
    // std::cout << "starting_x: " << starting_x << std::endl;
    // std::cout << "stopping_x: " << stopping_x << std::endl; 


    // Add relevant pixels to list

    // Select each row starting with row x0
    // Move along each row starting with y0 - radius or 0 (whichever is greater)
    // add each point to the list, stop when y == radius + y0 + 1 or y == maxY (whichever is smaller)
    // If you are on row x0, dont add any points until the y value of the point you are on is greater than the input point y0 value
    // stop going down when x == x0 + radius or x == maxX (whichever is smaller)

    for (int x = starting_x; x < stopping_x; x++) {
        for (int y = starting_y; y < stopping_y; y++) {
            if (x == starting_x && y <= y0) continue;
            pixels.emplace_back(x, y);
        }
    }

    // This code assumes x values go up and down and y values go across

    return pixels;
}