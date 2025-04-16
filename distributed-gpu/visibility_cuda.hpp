#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>

struct dimensions {
    int x_width;
    int y_height;
};

struct Point {
    int x;
    int y;
};

// left_offset is how far away the from_point area inside the altitude data is from the left of the side of the altitude data
int* run_visibility_search(uint16_t* altitude_data, dimensions altitude_dim, int left_offset, dimensions from_point_dim, Point* offsets, dimensions block_dim, int* visibility_results, int rank);
Point* pixelList_offset();