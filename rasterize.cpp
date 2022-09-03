#include <iostream>
#include <cmath>
#include <map>
#include <utility>
#include <vector>
#include "rasterize.hpp"

vertex operator+(const vertex &a, const vertex &b)
{
    auto n = std::min(a.size(), b.size());
    vertex res(n);
    while (n--)
        res[n] = a[n] + b[n];
    return res;
}

std::ostream &operator<<(std::ostream &os, const vertex &v)
{
    for (auto m : v)
    {
        os << m << ',';
    }
    os << '\n';
    return os;
}

vertex operator+=(vertex &a, const vertex &b)
{
    a = a + b;
    return a;
}

vertex operator-(const vertex &a, const vertex &b)
{
    auto n = std::min(a.size(), b.size());
    vertex res(n);
    while (n--)
        res[n] = a[n] - b[n];
    return res;
}

vertex operator*(const vertex &v, double k)
{
    auto n = v.size();
    vertex res(n);
    while (n--)
        res[n] = v[n] * k;
    return res;
}

vertex operator*(double k, const vertex &v)
{
    return v * k;
}

vertex operator/(const vertex &v, double k)
{
    auto n = v.size();
    vertex res(n);
    while (n--)
        res[n] = v[n] / k;
    return res;
}

void rasterizer::enable_depth()
{
    depth_enabled = true;
}

inline double srgb_to_linear(double c)
{
    return c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
}

inline double linear_to_srgb(double c)
{
    return c <= 0.0031308 ? 12.92 * c : 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
}

void rasterizer::enable_srgb()
{
    srgb_enabled = true;
}

void rasterizer::enable_perspective()
{
    perspective_enabled = true;
}

void rasterizer::enable_frustum_clipping()
{
    frustum_clipping_enabled = true;
}

void rasterizer::add_vertex(double x, double y, double z, double w)
{
    vertices.push_back({x, y, z, w, r, g, b, 0xff, 0, 0});
}

void rasterizer::set_color(double _r, double _g, double _b)
{
    r = _r;
    g = _g;
    b = _b;
}

template <class Operation>
void dda_scan(vertex a, vertex b, int i, Operation f)
{
    if (a[i] == b[i])
        return;
    if (a[i] > b[i])
        std::swap(a, b);
    auto s = (b - a) / (b[i] - a[i]);
    for (auto p = a + (std::ceil(a[i]) - a[i]) * s; p[i] < b[i]; p += s)
    {
        f(p);
    }
}

inline bool clipped(vertex &v)
{
    return (v[3] <= 0 ||
            v[0] < -v[3] || v[3] > v[3] ||
            v[1] < -v[3] || v[1] > v[3] ||
            v[2] < -v[3] || v[2] > v[3]);
}

void rasterizer::draw_triangle(display &disp, std::vector<vertex> vertices)
{
    for (auto &v : vertices)
    {
        if (srgb_enabled)
        {
            v[4] = srgb_to_linear(v[4] / 255.0);
            v[5] = srgb_to_linear(v[5] / 255.0);
            v[6] = srgb_to_linear(v[6] / 255.0);
        }
        auto w = v[3];
        v[0] = (v[0] / w + 1) * disp.width / 2;
        v[1] = (v[1] / w + 1) * disp.height / 2;
        v[2] /= w;
        v[3] = 1 / w;
        if (perspective_enabled)
        {
            // TODO: there are some bugs...
            for (size_t i = 4; i < v.size(); ++i)
            {
                v[i] /= w;
            }
        }
    }

    sort(vertices.begin(), vertices.end(), [](vertex &a, vertex &b)
         { return a[1] <= b[1]; });

    std::vector<vertex> bound1, bound2;
    dda_scan(vertices[0], vertices[2], 1, [&](vertex &v)
             { bound1.push_back(v); });
    dda_scan(vertices[0], vertices[1], 1, [&](vertex &v)
             { bound2.push_back(v); });
    dda_scan(vertices[1], vertices[2], 1, [&](vertex &v)
             { bound2.push_back(v); });

    assert(bound1.size() == bound2.size());

    auto draw_pixel = [&](vertex v) // copy since it will be modified
    {
        if (perspective_enabled)
        {
            for (size_t i = 4; i < v.size(); ++i)
            {
                v[i] /= v[3];
            }
        }
        if (srgb_enabled)
        {
            v[4] = linear_to_srgb(v[4]) * 255.0;
            v[5] = linear_to_srgb(v[5]) * 255.0;
            v[6] = linear_to_srgb(v[6]) * 255.0;
        }
        if (depth_enabled)
        {
            if (v[2] >= -1.0 && v[2] < disp.depth(v[0], v[1]))
            {
                disp.set_color(v[0], v[1], v[4], v[5], v[6], v[7]);
                disp.depth(v[0], v[1]) = v[2];
            }
        }
        else
        {
            disp.set_color(v[0], v[1], v[4], v[5], v[6], v[7]);
        }
    };

    for (size_t i = 0; i < bound1.size(); ++i)
    {
        dda_scan(bound1[i], bound2[i], 0, draw_pixel);
    }
}

vertex intersect(vertex v1, vertex v2)
{
    std::cout << v1 << ',' << v2;
    // throw std::logic_error("I dont know what's going on");
    return (v2 + v1) / 2;
}

void rasterizer::draw_triangle(display &disp, int i1, int i2, int i3)
{
    auto v1 = ith_vertex(i1), v2 = ith_vertex(i2), v3 = ith_vertex(i3);
    std::vector<vertex> outside, inside;
    for (auto &v : vertices)
    {
        if (clipped(v))
        {
            outside.push_back(v);
        }
        else
        {
            inside.push_back(v);
        }
    }
    if (outside.size() == 3)
        return;
    if (outside.size() == 2)
    {
        auto v1 = intersect(inside[0], outside[0]);
        auto v2 = intersect(inside[0], outside[1]);
        draw_triangle(disp, {v1, v2, inside[0]});
    }
    else if (outside.size() == 1)
    {
        auto v1 = intersect(inside[0], outside[0]);
        auto v2 = intersect(inside[1], outside[0]);
        draw_triangle(disp, {inside[0], inside[1], v1});
        draw_triangle(disp, {inside[0], inside[1], v2});
    }
    else
    {
        draw_triangle(disp, {v1, v2, v3});
    }
}

vertex &rasterizer::ith_vertex(int i)
{
    if (i == 0)
        throw std::out_of_range("index cannot be 0");
    if (i < 0)
    {
        auto &v = vertices[vertices.size() + i];
        return v;
    }
    return vertices[i - 1];
}