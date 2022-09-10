#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <queue>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>
#include <cassert>
#include <stdexcept>
#include "buffer.hpp"

using vec = std::vector<double>;
using mat = std::vector<std::vector<double>>;
using tri = std::array<vec, 3>;

#define TEXTURE_SIZE 512

class rasterizer
{
public:
    rasterizer();
    int width = 0, height = 0;
    void resize(int w, int h);
    void add_vec(double x, double y, double z, double w);
    void set_color(double r, double g, double b, double a);
    void set_texcoord(double s, double t);
    void output();
    std::vector<unsigned char> &data();
    void draw_pixel(vec pixel);
    void draw_point(int i, double size);
    void draw_triangle(int i1, int i2, int i3);
    void draw_line(int i1, int i2);
    void draw_wuline(int i1, int i2);
    void enable_depth();
    void enable_srgb();
    void enable_perspective();
    void enable_frustum_clipping();
    void enable_fsaa(int level);
    void cull_face();
    void load_texture(std::string &filename);
    void enable_texture();
    void disable_texture();
    void enable_decals();
    void clip(double p1, double p2, double p3, double p4);

private:
    double r, g, b, a, s, t;
    int fsaa_level;
    bool depth_enabled;
    bool srgb_enabled;
    bool perspective_enabled;
    bool frustum_clipping_enabled;
    bool cull_enabled;
    bool texture_enabled;
    bool decals_enabled;
    frame_buffer<unsigned char> texture;
    frame_buffer<unsigned char> output_buf;
    frame_buffer<double> render_buf;
    depth_buffer depth_buf;
    std::vector<vec> vertices;
    std::vector<vec> clip_planes;
    vec &nth_vertex(int i);
    vec project(vec p);
    void draw_triangle_clipped(tri triangle);
    void draw_triangle(tri triangle);
};
