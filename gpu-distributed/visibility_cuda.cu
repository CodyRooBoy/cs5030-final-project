

// left_offset is the number of pixels from the first from_pixel to the edge of the altitude data on the left


uint16_t* run_visibility_search(
        uint16_t* altitude_data, 
        int altitude_data_size,
        int left_offset,
        dimensions altitude_dim,
        dimensions from_point_dim,
        Point* offsets,
        dimensions block_dim,
        uint16_t* visibility_results) {

    // Needs alitidue data and data dimentions
    // Needs from point dimentions

    // All data will start from the very first thread

    // Allocate memory for Altitude data
    uint16_t* altitude_data_d;
    cudaMalloc((void **) &altitude_data_d, altitude_data_size * sizeof(uint16_t));
    cudaMemcpy(altitude_data_d, altitude_data, altitude_data_size * sizeof(uint16_t), cudaMemcpyHostToDevice);

    // Allocate memory for visibility results for each to point in altitude data (will be same size)
    uint16_t* visibility_results_d;
    cudaMalloc((void **) &visibility_results_d, altitude_data_size * sizeof(uint16_t));

    // Allocate memory for from point data (will be same size as from point dimensions)
    uint16_t* from_point_visibility_d;
    cudaMalloc((void **) &from_point_visibility_d, from_point_dim.x_width * from_point_dim.y_height * sizeof(uint16_t));

    // Allocate memory for offset values
    Point* offsets_d;
    cudaMalloc((void **) &offsets_d, from_point_dim.x_width * from_point_dim.y_height * sizeof(Point));

    

    // Build out block and grid dimensions
    dim3 input_block_dim(block_dim.x_width, block_dim.y_height);
    dim3 grid_dim((int)ceil((float)altitude_dim.x_width / block_dim.x_width), (int)ceil((float)altitude_dim.y_height / block_dim.y_height));
    
    get_visibility_gpu<<<grid_dim, input_block_dim>>>(
        altitude_data_d,
        altitude_data_size,
        left_offset,
        altitude_dim,
        from_point_dim,
        offsets_d,
        visibility_results_d,
        from_point_visibility_d
    );


    
}

__global__ get_visibility_gpu(
        uint16_t* altitude_data,
        int altitude_data_size,
        int left_offset,
        dimensions altitude_dim,
        dimensions from_points_dim,
        Point* offsets,
        uint16_t* visibility_results_d,
        uint16_t* from_point_visibility_d) {


    // Iterate through all the offsets

    // determine if the offset is within the bounds of the altitude data

    // If it is, check if the point is visible from the from point
        // If it is, increment the visibility result for this point in the from point visibility data as well as in the to point data
    // If it is not, do nothing

    Point from_section_xy = {blockIdx.x * blockDim.x + threadIdx.x, blockIdx.y * blockDim.y + threadIdx.y}
    Point a_data_xy = {from_section_x + left_offset, blockIdx.y * blockDim.y + threadIdx.y};

    // Check if the thread is within the bounds of the from_point dimensions, exits if it is
    if (from_section_xy.x_width > from_points_dim.x_width || from_section_xy.y_height > from_points_dim.y_height) {
        return;
    }

    for (int i = 0; i < 20200; i++) {
        
        Point current_offset = offsets[i];
        // Get the xy coordinates of the offset within the altitude data
        Point a_data_to_xy = {a_data_xy.x_width + current_offset.x_width, a_data_xy.y_height + current_offset.y_height};
        // Check if the offset is within the bounds of the altitude data
        if (a_data_to_xy.x_width < 0 || a_data_to_xy.x_width >= altitude_dim.x_width || a_data_to_xy.y_height < 0 || a_data_to_xy.y_height >= altitude_dim.y_height) {
            continue;
        }

        // Get the visibility between the main point and the offset point
        float slope = visibility_line_slope(
            altitude_data[a_data_xy.y_height * altitude_dim.x_width + a_data_xy.x_width],
            altitude_data[a_data_to_xy.y_height * altitude_dim.x_width + a_data_to_xy.x_width],
            a_data_xy.x_width,
            a_data_xy.y_height,
            a_data_to_xy.x_width,
            a_data_to_xy.y_height
        );
        bool visibile = visibility_path(altitude_data, slope, a_data_xy.x_width, a_data_xy.y_height, a_data_to_xy.x_width, a_data_to_xy.y_height, altitude_dim.x_width);

        // If the point is visible, increment the visibility result for this point in the from point visibility data as well as in the to point data
        if (visibile) {
            // Increment the visibility result for this point in the from point visibility data
            from_point_visibility_d[from_section_xy.y_height * from_points_dim.x_width + from_section_xy.x_width] += 1;
            // Increment the visibility result for this point in the to point data
            visibility_results_d[a_data_to_xy.y_height * altitude_dim.x_width + a_data_to_xy.x_width] += 1;
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
                if ((altitude_data[point.y_height * a_data_width + point.x_width]) < (altitude + slope)) {
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
                if ((altitude_data[point.y_height * a_data_width + point.x_width]) < (altitude + slope)) {
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
    Point* pixels = (Point*)malloc(sizeof(Point) * 20200);

    // Set the bounds of the relevant pixel box
    int starting_x = -100;
    int stopping_x = 100;
    int starting_y = 0;
    int stopping_y = 100;

    int place = 0;

    for (int y = starting_y; y <= stopping_y; y++) {
        for (int x = starting_x; x <= stopping_x; x++) {
            if (y == starting_y && x <= 0) continue;
            pixels[place] = { x, y };
            place++;
        }
    }

    // This code assumes x values go across and y values up and down

    return pixels;
}










