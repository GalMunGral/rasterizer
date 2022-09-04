#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "rasterize.hpp"

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
            raster.resize(width, height);
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
            raster.set_color(r, g, b, 1.0);
        }
        else if (cmd == "tri")
        {
            int i1, i2, i3;
            ss >> i1 >> i2 >> i3;
            raster.disable_texture();
            raster.draw_triangle(i1, i2, i3);
        }
        else if (cmd == "depth")
        {
            raster.enable_depth();
        }
        else if (cmd == "sRGB")
        {
            raster.enable_srgb();
        }
        else if (cmd == "rgba")
        {
            // TODO: after sRGB
            double r, g, b, a;
            ss >> r >> g >> b >> a;
            raster.set_color(r, g, b, a);
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
        else if (cmd == "fsaa")
        {
            int level;
            ss >> level;
            raster.enable_fsaa(level);
        }
        else if (cmd == "cull")
        {
            raster.cull_face();
        }
        else if (cmd == "texcoord")
        {
            double u, v;
            ss >> u >> v;
            raster.set_texcoord(u, v);
        }
        else if (cmd == "texture")
        {
            std::string filename;
            ss >> filename;
            raster.load_texture(filename);
        }
        else if (cmd == "trit")
        {
            int i1, i2, i3;
            ss >> i1 >> i2 >> i3;
            raster.enable_texture();
            raster.draw_triangle(i1, i2, i3);
        }
        else if (cmd == "point")
        {
            int i;
            double size;
            ss >> size >> i;
            raster.disable_texture();
            raster.draw_point(i, size);
        }
        else if (cmd == "billboard")
        {
            int i;
            double size;
            ss >> size >> i;
            raster.enable_texture();
            raster.draw_point(i, size);
        }
    }
    raster.output();
    unsigned err;
    if ((err = lodepng::encode(filename, raster.data(), raster.width, raster.height, LCT_RGBA, 8)))
    {
        std::cerr << lodepng_error_text(err) << '\n';
    };
    return 0;
}
