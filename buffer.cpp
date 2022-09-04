#include <iostream>
#include <stdexcept>
#include "buffer.hpp"

depth_buffer::depth_buffer() : depth_buffer(0, 0) {}

depth_buffer::depth_buffer(int w, int h) : width{w}, height{h}, buf(w * h, 1.0) {}

double &depth_buffer::operator()(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return buf[y * width + x];
}

template <class T>
frame_buffer<T>::frame_buffer() : frame_buffer(0, 0) {}

template <class T>
frame_buffer<T>::frame_buffer(int w, int h) : width{w}, height{h}, buf(w * h * 4, 0) {}

template <class T>
T *frame_buffer<T>::data()
{
    return buf.data();
}

template <class T>
T &frame_buffer<T>::operator()(int x, int y, int channel)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        throw std::out_of_range("pixel index out of range");
    }
    return buf[(y * width + x) * 4 + channel];
}

template <class T>
void frame_buffer<T>::set_color(int x, int y, T r, T g, T b, T a)
{
    buf[(y * width + x) * 4 + 0] = r;
    buf[(y * width + x) * 4 + 1] = g;
    buf[(y * width + x) * 4 + 2] = b;
    buf[(y * width + x) * 4 + 3] = a;
}
template class frame_buffer<double>;
template class frame_buffer<unsigned char>;