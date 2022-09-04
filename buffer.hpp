#pragma once
#include <vector>
#include "lodepng.h"
#define cimg_display 0

class depth_buffer
{
public:
    unsigned width, height;
    depth_buffer();
    depth_buffer(unsigned w, unsigned h);
    double &operator()(unsigned x, unsigned y);

private:
    std::vector<double> buf;
};

template <class T>
class frame_buffer
{
public:
    unsigned width, height;
    frame_buffer();
    frame_buffer(unsigned w, unsigned h);
    std::vector<T> &data();
    T &operator()(unsigned x, unsigned y, unsigned channel);
    void set_color(unsigned x, unsigned y, T r, T g, T b, T a);
    void save(std::string &filename);

private:
    std::vector<T> buf;
};
