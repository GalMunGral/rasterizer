#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "image.hpp"

image disp;
std::string filename;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "No filename specified\n";
        return -1;
    }
    std::ifstream file(argv[1]);
    for (std::string line; std::getline(file, line);)
    {
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        if (cmd == "png")
        {
            int width, height;
            ss >> width >> height >> filename;
            disp.resize(width, height);
        }
        else if (cmd == "xyrgb")
        {
            int x, y, r, g, b;
            ss >> x >> y >> r >> g >> b;
            disp.set_color(x, y, r, g, b, 0xff);
        }
        else if (cmd == "xyc")
        {
            int x, y;
            std::string hex_color;
            ss >> x >> y >> hex_color;
            disp.set_color(x, y, hex_color);
        }
    }
    int err;
    if ((err = lodepng_encode_file(filename.c_str(), disp.data(), disp.width, disp.height, LCT_RGBA, 8)))
    {
        for (int i = 0; i < disp.width * disp.height * 4; ++i) {
            std::cout << disp.data()[i];
        }
        std::cerr << lodepng_error_text(err) << '\n';
        return -1;
    };
    std::cout << err;
    return 0;
}
