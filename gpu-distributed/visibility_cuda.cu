#include "visibility_cuda.hpp"

__global__ void get_visibility_gpu(uint16_t* altitude_data, dimensions altitude_dim, int left_offset, dimensions from_points_dim, Point* offsets, int* visibility_results_d, int* from_point_visibility_d);
__device__ bool visibility_path(uint16_t* altitude_data, float slope, int x1, int y1, int x2, int y2, int a_data_width);
__device__ float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2);


int* run_visibility_search(
        uint16_t* altitude_data, 
        dimensions altitude_dim,
        int left_offset,
        dimensions from_point_dim,
        Point* offsets,
        dimensions block_dim,
        int* visibility_results,
        int rank) {


    // Allocate memory for Altitude data
    uint16_t* altitude_data_d;
    int altitude_data_size = altitude_dim.x_width * altitude_dim.y_height;
    cudaMalloc((void **) &altitude_data_d, altitude_data_size * sizeof(uint16_t));
    cudaMemcpy(altitude_data_d, altitude_data, altitude_data_size * sizeof(uint16_t), cudaMemcpyHostToDevice);

    // Allocate memory for visibility results for each to-point in altitude data (will be same size)
    int* visibility_results_d;
    cudaMalloc((void **) &visibility_results_d, altitude_data_size * sizeof(int));

    // Allocate memory for from point data (will be same size as from point dimensions)
    int* from_point_visibility_d;
    cudaMalloc((void **) &from_point_visibility_d, from_point_dim.x_width * from_point_dim.y_height * sizeof(int));

    // Allocate memory for offset values
    Point* offsets_d;
    cudaMalloc((void **) &offsets_d, 19999 * sizeof(Point));
    cudaMemcpy(offsets_d, offsets, 19999 * sizeof(Point), cudaMemcpyHostToDevice);

    // Build out the grid and block dimensions
    dim3 input_block_dim(block_dim.x_width, block_dim.y_height);
    dim3 grid_dim((int)ceil((float)from_point_dim.x_width / block_dim.x_width), (int)ceil((float)from_point_dim.y_height / block_dim.y_height));

    fflush(stdout);

    // Figure out which CUDA device to use based on the rank
    int device_count;
    cudaGetDeviceCount(&device_count);
    // printf("Device Count: %d\n", device_count);
    cudaSetDevice(rank % device_count);

    get_visibility_gpu<<<grid_dim, input_block_dim>>>(
        altitude_data_d,
        altitude_dim,
        left_offset,
        from_point_dim,
        offsets_d,
        visibility_results_d,
        from_point_visibility_d
    );

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("CUDA error: %s\n", cudaGetErrorString(err));
    }
    cudaDeviceSynchronize();

    // Copy the visibility results back to the host
    cudaMemcpy(visibility_results, visibility_results_d, altitude_data_size * sizeof(int), cudaMemcpyDeviceToHost);

    // Copy the from point visibility results back to the host
    int* from_point_visibility = (int*)malloc(from_point_dim.x_width * from_point_dim.y_height * sizeof(int));
    cudaMemcpy(from_point_visibility, from_point_visibility_d, from_point_dim.x_width * from_point_dim.y_height * sizeof(int), cudaMemcpyDeviceToHost);
    
    // Combine the from_point data and to_point data
    for (int x = 0; x < from_point_dim.x_width; x++) {
        for (int y = 0; y < from_point_dim.y_height; y++) {
            visibility_results[y * altitude_dim.x_width + x + left_offset] += from_point_visibility[y * from_point_dim.x_width + x];
        }
    }

    return 0;
}

__global__ void get_visibility_gpu(
        uint16_t* altitude_data,
        dimensions altitude_dim,
        int left_offset,
        dimensions from_points_dim,
        Point* offsets,
        int* visibility_results_d,
        int* from_point_visibility_d) {

    // Get XY position relative to the altitude data as well as the from_point section
    Point from_section_xy = {(int)(blockIdx.x * blockDim.x + threadIdx.x), (int)(blockIdx.y * blockDim.y + threadIdx.y)};
    Point a_data_xy = {(int)(blockIdx.x * blockDim.x + threadIdx.x + left_offset), (int)(blockIdx.y * blockDim.y + threadIdx.y)};

    // Check if the thread is within the bounds of the from_point dimensions, exits if it is
    if (from_section_xy.x >= from_points_dim.x_width || from_section_xy.y >= from_points_dim.y_height) {
        return;
    }

    for (int i = 0; i < 19999; i++) {
        
        Point current_offset = offsets[i];

        // Get the xy coordinates of the offset within the altitude data
        Point a_data_to_xy = {a_data_xy.x + current_offset.x, a_data_xy.y + current_offset.y};

        // Check if the offset is within the bounds of the altitude data
        if (a_data_to_xy.x < 0 || a_data_to_xy.x >= altitude_dim.x_width || a_data_to_xy.y < 0 || a_data_to_xy.y >= altitude_dim.y_height) {
            continue;
        }

        // Get the visibility between the main point and the offset point
        float slope = visibility_line_slope(
            altitude_data[a_data_xy.y * altitude_dim.x_width + a_data_xy.x],
            altitude_data[a_data_to_xy.y * altitude_dim.x_width + a_data_to_xy.x],
            a_data_xy.x,
            a_data_xy.y,
            a_data_to_xy.x,
            a_data_to_xy.y
        );

        bool visible = visibility_path(altitude_data, slope, a_data_xy.x, a_data_xy.y, a_data_to_xy.x, a_data_to_xy.y, altitude_dim.x_width);

        // If the point is visible, increment the visibility result for this point in the from point visibility data as well as in the to point data
        if (visible) {

            // Increment the visibility result for this point in the from point visibility data
            atomicAdd(&from_point_visibility_d[from_section_xy.y * from_points_dim.x_width + from_section_xy.x], 1);

            // Increment the visibility result for this point in the to point data
            atomicAdd(&visibility_results_d[a_data_to_xy.y * altitude_dim.x_width + a_data_to_xy.x], 1);
        }
    }
}

__device__ bool visibility_path(uint16_t* altitude_data, float slope, int x1, int y1, int x2, int y2, int a_data_width)
{

    // std::pair<int, int> point;
    short altitude = altitude_data[y1 * a_data_width + x1];

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
            Point point = {x, y};
            if (x != x1 && y != y1 && x != x2 && y != y2) {
                if ((altitude_data[point.y * a_data_width + point.x]) < (altitude + slope)) {
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
            Point point = {x, y};
            if (x != x1 && y != y1 && x != x2 && y != y2) {
                if ((altitude_data[point.y * a_data_width + point.x]) < (altitude + slope)) {
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

__device__ float visibility_line_slope(short starting_altitude, short ending_altitude, short x1, short y1, short x2, short y2) {
    return (ending_altitude - starting_altitude) / sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

Point* pixelList_offset() {

    // Create a place to store pixel values
    Point* pixels = (Point*)malloc(sizeof(Point) * 19999);

    // Set the bounds of the relevant pixel box
    int starting_x = -100;
    int stopping_x = 100;
    int starting_y = 0;
    int stopping_y = 99;

    int place = 0;

    for (int y = starting_y; y <= stopping_y; y++) {
        for (int x = starting_x; x <= stopping_x; x++) {
            if (y == starting_y && x <= 0) continue;
            pixels[place] = { x, y };
            place++;
        }
    }

    return pixels;
}










