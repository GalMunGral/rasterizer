#include <vector>
#include <stdexcept>
#include "buffer.hpp"

using vec = std::vector<double>;
using mat = std::vector<std::vector<double>>;

class rasterizer
{
public:
    int width = 0, height = 0;
    void resize(int w, int h);
    void add_vec(double x, double y, double z, double w);
    void set_color(double r, double g, double b);
    void output();
    unsigned char *data();
    void draw_pixel(vec pixel);
    void draw_triangle(int i1, int i2, int i3);
    void draw_triangle(std::vector<vec> vertices);
    void enable_depth();
    void enable_srgb();
    void enable_perspective();
    void enable_frustum_clipping();
    void enable_fsaa(int level);

private:
    double r = 0xff, g = 0xff, b = 0xff;
    int fsaa_level = 1;
    bool depth_enabled;
    bool srgb_enabled;
    bool perspective_enabled;
    bool frustum_clipping_enabled;
    frame_buffer<unsigned char> output_buf;
    frame_buffer<double> render_buf;
    depth_buffer depth_buf;
    std::vector<vec> vertices;
    vec &ith_vec(int i);
};
