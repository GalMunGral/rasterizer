#pragma once
#include <vector>
#include "lodepng.h"
#define cimg_display 0

class display
{
public:
    int width;
    int height;
    void resize(int w, int h);
    unsigned char *data();
    unsigned char &operator()(int x, int y, int channel);
    double &depth(int x, int y);
    void set_color(int x, int y, int r, int g, int b, int a);
    void set_color(int x, int y, std::string &hex_color);
    void save(std::string &filename);

private:
    std::vector<unsigned char> frame_buffer;
    std::vector<double> depth_buffer;
};
