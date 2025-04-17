#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <cstdint>

void visible_points(std::vector<uint16_t> &altitude_data, std::vector<uint32_t> &visibility_data, int height, int width, short x1, short y1);
bool visibility_line_exists(std::vector<uint16_t> &altitude_data, short x1, short y1, short x2, short y2, int width);
bool visibility_path(std::vector<uint16_t> &altitude_data, float slope, int x1, int y1, int x2, int y2, int width);
float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2);

std::vector<std::pair<int, int>> pixelList(int x0, int y0, int maxX, int maxY, int radius);