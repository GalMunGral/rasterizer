#include <iostream>
#include <stdexcept>
#include "display.hpp"

inline unsigned char hex(char c)
{
    return isdigit(c) ? c - '0' : c - 'a' + 10;
}

inline unsigned char byte(std::string &s, int i)
{
    return hex(s[i]) << 4 | hex(s[i + 1]);
}

void display::resize(int w, int h)
{
    width = w;
    height = h;
    frame_buffer.resize(w * h * 4, 0);
    depth_buffer.resize(w * h, 1.0);
}

unsigned char *display::data()
{
    return frame_buffer.data();
}

unsigned char &display::operator()(int x, int y, int channel)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return frame_buffer[(y * width + x) * 4 + channel];
}

double &display::depth(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return depth_buffer[y * width + x];
}

void display::set_color(int x, int y, int r, int g, int b, int a)
{
    frame_buffer[(y * width + x) * 4 + 0] = r;
    frame_buffer[(y * width + x) * 4 + 1] = g;
    frame_buffer[(y * width + x) * 4 + 2] = b;
    frame_buffer[(y * width + x) * 4 + 3] = a;
}

void display::set_color(int x, int y, std::string &hex_color)
{
    set_color(x, y, byte(hex_color, 1), byte(hex_color, 3), byte(hex_color, 5), 0xff);
}
