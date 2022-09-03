#include <iostream>
#include <stdexcept>
#include "image.hpp"

inline unsigned char hex(char c)
{
    return isdigit(c) ? c - '0' : c - 'a' + 10;
}

inline unsigned char byte(std::string &s, int i)
{
    return hex(s[i]) << 4 | hex(s[i + 1]);
}

void image::resize(int w, int h)
{
    width = w;
    height = h;
    buffer.resize(w * h * 4);
}

unsigned char &image::operator()(int x, int y, int channel)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return buffer[(y * width + x) * 4 + channel];
}

unsigned char *image::data()
{
    return buffer.data();
}

void image::set_color(int x, int y, int r, int g, int b, int a)
{
    buffer[(y * width + x) * 4 + 0] = r;
    buffer[(y * width + x) * 4 + 1] = g;
    buffer[(y * width + x) * 4 + 2] = b;
    buffer[(y * width + x) * 4 + 3] = a;
}

void image::set_color(int x, int y, std::string &hex_color)
{
    set_color(x, y, byte(hex_color, 1), byte(hex_color, 3), byte(hex_color, 5), 0xff);
}
