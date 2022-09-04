#pragma once
#include <vector>
#include "lodepng.h"
#define cimg_display 0

class depth_buffer
{
public:
    int width, height;
    depth_buffer();
    depth_buffer(int w, int h);
    double &operator()(int x, int y);

private:
    std::vector<double> buf;
};

template <class T>
class frame_buffer
{
public:
    int width, height;
    frame_buffer();
    frame_buffer(int w, int h);
    T *data();
    T &operator()(int x, int y, int channel);
    void set_color(int x, int y, T r, T g, T b, T a);
    void save(std::string &filename);

private:
    std::vector<T> buf;
};
