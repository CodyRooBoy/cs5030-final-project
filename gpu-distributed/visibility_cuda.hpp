#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct dimensions {
    int x_width;
    int y_height;
};

struct Point {
    int x;
    int y;
};


uint16_t* run_visibility_search(uint16_t* altitude_data, int altitude_data_size, int left_offset, dimensions altitude_dim, dimensions from_point_dim, Point* offsets, dimensions block_dim, uint16_t* visibility_results);
void get_visibility_gpu(uint16_t* altitude_data, int altitude_data_size, int left_offset, dimensions altitude_dim, dimensions from_points_dim, Point* offsets, uint16_t* visibility_results_d, uint16_t* from_point_visibility_d);
bool visibility_path(uint16_t* altitude_data, float slope, int x1, int y1, int x2, int y2, int a_data_width);
float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2);
Point* pixelList_offset();