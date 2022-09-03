#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "display.hpp"
#include "rasterize.hpp"

display disp;
rasterizer raster;
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
        else if (cmd == "xyzw")
        {
            double x, y, z, w;
            ss >> x >> y >> z >> w;
            raster.add_vec(x, y, z, w);
        }
        else if (cmd == "rgb")
        {
            double r, g, b;
            ss >> r >> g >> b;
            raster.set_color(r, g, b);
        }
        else if (cmd == "tri")
        {
            int i1, i2, i3;
            ss >> i1 >> i2 >> i3;
            raster.draw_triangle(disp, i1, i2, i3);
        }
        else if (cmd == "depth")
        {
            raster.enable_depth();
        }
        else if (cmd == "sRGB")
        {
            raster.enable_srgb();
        }
        else if (cmd == "hyp")
        {
            // TODO: only after sRGB
            raster.enable_perspective();
        }
        else if (cmd == "frustum")
        {
            raster.enable_frustum_clipping();
        }
    }
    int err;
    if ((err = lodepng_encode_file(filename.c_str(), disp.data(), disp.width, disp.height, LCT_RGBA, 8)))
    {
        for (int i = 0; i < disp.width * disp.height * 4; ++i)
        {
            std::cout << disp.data()[i];
        }
        std::cerr << lodepng_error_text(err) << '\n';
        return -1;
    };

    return 0;
}
