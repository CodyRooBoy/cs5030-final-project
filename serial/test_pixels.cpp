#include "visibility.hpp"
#include "visibility.cpp"
#include <stdio.h>
#include <cmath>

std::vector<std::pair<int, int>> pixelList_square(int x0, int y0, int maxX, int maxY, int radius);

int main() {

    // The point for which we are checking visiblity
    int x1 = 10;
    int y1 = 20;

    // Visualization dimentions
    const int vs = 100;


    // Getting a list of all the pixels that we need to check visibility for
    std::vector<std::pair<int, int>> output = pixelList_square(x1, y1, 101, 100, 50);
    // std::vector<std::pair<int, int>> output = pixelList(x1, y1, 100, 100, 50);


    // Putting this pixel list into a 2d array to help visualize what points were selected
    short (*values)[vs] = new short [vs][vs]();
    values[x1][y1] = 2;
    for (auto pixel : output) {
        int x = pixel.first;
        int y = pixel.second;
        if (x > -1 && x < vs && y > -1 && y < vs) {
            values[x][y] = 1;
        } else {
            // If this happens, it means a point was returned by our pixel list function
            // that is outside the bounds of our map
            std::cout << "Somthing happened";
        }
    }


    // Display the visualization
    for (int x = 0; x < vs; x++) {
        for (int y = 0; y < vs; y++) {
            std::cout << values[x][y];
        }
        std::cout << std::endl;
    }
}


std::vector<std::pair<int, int>> pixelList_square(int x0, int y0, int maxX, int maxY, int radius) {

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