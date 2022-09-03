#include <iostream>
#include <cmath>
#include <map>
#include <utility>
#include <vector>
#include "rasterize.hpp"

vec operator+(const vec &a, const vec &b)
{
    auto n = std::min(a.size(), b.size());
    vec res(n);
    while (n--)
        res[n] = a[n] + b[n];
    return res;
}

std::ostream &operator<<(std::ostream &os, const vec &v)
{
    for (auto m : v)
    {
        os << m << ',';
    }
    return os;
}

vec operator+=(vec &a, const vec &b)
{
    a = a + b;
    return a;
}

vec operator-(const vec &a, const vec &b)
{
    auto n = std::min(a.size(), b.size());
    vec res(n);
    while (n--)
        res[n] = a[n] - b[n];
    return res;
}

vec operator*(const vec &v, double k)
{
    auto n = v.size();
    vec res(n);
    while (n--)
        res[n] = v[n] * k;
    return res;
}

double operator*(const vec &a, const vec &b)
{
    double res = 0;
    auto n = std::min(a.size(), b.size());
    while (n--)
    {
        res += a[n] * b[n];
    }
    return res;
}

vec operator*(double k, const vec &v)
{
    return v * k;
}

vec operator/(const vec &v, double k)
{
    auto n = v.size();
    vec res(n);
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

void rasterizer::add_vec(double x, double y, double z, double w)
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
void dda_scan(vec a, vec b, int i, Operation f)
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

vec intersect(vec v1, vec v2)
{
    mat M = {
        {1, 0, 0, 1},
        {-1, 0, 0, 1},
        {0, 1, 0, 1},
        {0, -1, 0, 1},
        {0, 0, 1, 1},
        {0, 0, -1, 1},
    };
    for (auto &u : M)
    {
        auto d1 = u * v1, d2 = u * v2;
        auto v = (d2 * v1 - d1 * v2) / (d2 - d1);
        if ((d1 < 0 || d2 < 0) && std::all_of(M.begin(), M.end(), [&](vec &u)
                                              { return u * v >= 0; }))
        {
            // if on an edge/vertex of the frustum, the intersection will be computed multiple times
            return v;
        }
    }
    throw std::logic_error("Hmmm...");
}

inline bool clipped(vec &v)
{
    return (v[3] <= 0 ||
            v[0] < -v[3] || v[0] > v[3] ||
            v[1] < -v[3] || v[1] > v[3] ||
            v[2] < -v[3] || v[2] > v[3]);
}

void rasterizer::draw_triangle(display &disp, std::vector<vec> vertices)
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

    sort(vertices.begin(), vertices.end(), [](vec &a, vec &b)
         { return a[1] <= b[1]; });

    std::vector<vec> bound1, bound2;
    dda_scan(vertices[0], vertices[2], 1, [&](vec &v)
             { bound1.push_back(v); });
    dda_scan(vertices[0], vertices[1], 1, [&](vec &v)
             { bound2.push_back(v); });
    dda_scan(vertices[1], vertices[2], 1, [&](vec &v)
             { bound2.push_back(v); });

    assert(bound1.size() == bound2.size());

    auto draw_pixel = [&](vec v) // copy since it will be modified
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

void rasterizer::draw_triangle(display &disp, int i1, int i2, int i3)
{
    std::vector<vec> vertices{ith_vec(i1), ith_vec(i2), ith_vec(i3)};
    if (frustum_clipping_enabled)
    {
        std::vector<vec> outside, inside;
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
            draw_triangle(disp, {inside[1], v1, v2});
        }
        else
        {
            draw_triangle(disp, vertices);
        }
    }
    else
    {
        draw_triangle(disp, vertices);
    }
}

vec &rasterizer::ith_vec(int i)
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