#include <iostream>
#include <stdexcept>
#include "buffer.hpp"

depth_buffer::depth_buffer() : depth_buffer(0, 0) {}

depth_buffer::depth_buffer(unsigned w, unsigned h) : width{w}, height{h}, buf(w * h, 1.0) {}

double &depth_buffer::operator()(unsigned x, unsigned y)
{
    if (x >= width || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return buf[y * width + x];
}

template <class T>
frame_buffer<T>::frame_buffer() : frame_buffer(0, 0) {}

template <class T>
frame_buffer<T>::frame_buffer(unsigned w, unsigned h) : width{w}, height{h}, buf(w * h * 4, 0) {}

template <class T>
std::vector<T> &frame_buffer<T>::data()
{
    return buf;
}

template <class T>
T &frame_buffer<T>::operator()(unsigned x, unsigned y, unsigned channel)
{
    if (x >= width || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return buf[(y * width + x) * 4 + channel];
}

template <class T>
void frame_buffer<T>::set_color(unsigned x, unsigned y, T r, T g, T b, T a)
{
    buf[(y * width + x) * 4 + 0] = b;
    buf[(y * width + x) * 4 + 1] = g;
    buf[(y * width + x) * 4 + 2] = r;
    buf[(y * width + x) * 4 + 3] = a;
}
template class frame_buffer<double>;
template class frame_buffer<unsigned char>;
