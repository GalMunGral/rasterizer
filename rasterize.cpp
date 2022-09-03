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

void rasterizer::draw_triangle(display &disp, int i1, int i2, int i3)
{
    std::vector<vertex> vertices{ith_vertex(i1), ith_vertex(i2), ith_vertex(i3)};
    for (auto &v : vertices)
    {
        double w = v[3];
        if (false)
        {
            v = v / w;
            v[3] = 1 / w;
        }
        v[0] = (v[0] / w + 1) * disp.width / 2;
        v[1] = (v[1] / w + 1) * disp.height / 2;
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

    auto draw_pixel = [&](vertex &v)
    {
        if (false)
        {
            for (size_t i = 4; i < v.size(); ++i)
            {
                v[i] /= v[3];
            }
            v[3] = 1 / v[3];
        }
        disp.set_color(v[0], v[1], v[4], v[5], v[6], v[7]);
    };

    for (size_t i = 0; i < bound1.size(); ++i)
    {
        dda_scan(bound1[i], bound2[i], 0, draw_pixel);
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