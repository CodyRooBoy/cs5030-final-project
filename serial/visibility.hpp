#pragma once

#include <vector>
#include <iostream>
#include <cmath>

short visible_points(short (*data)[6000], short (*output_points)[6000], short x1, short y1);
bool visibility_line_exists(short (*data)[6000], short x1, short y1, short x2, short y2);
bool visibility_path(short (*data)[6000], float slope, int x1, int y1, int x2, int y2);
float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2);

std::vector<std::pair<int, int>> pixelList(int x0, int y0, int maxX, int maxY, int radius);