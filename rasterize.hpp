#include <vector>
#include <stdexcept>
#include "display.hpp"

using vec = std::vector<double>;
using mat = std::vector<std::vector<double>>;

class rasterizer
{
public:
    void add_vec(double x, double y, double z, double w);
    void set_color(double r, double g, double b);
    void draw_triangle(display &disp, int i1, int i2, int i3);
    void draw_triangle(display &disp, std::vector<vec> vertices);
    void resize(int width, int height);
    void enable_depth();
    void enable_srgb();
    void enable_perspective();
    void enable_frustum_clipping();

private:
    bool depth_enabled;
    bool srgb_enabled;
    bool perspective_enabled;
    bool frustum_clipping_enabled;
    double r = 0xff, g = 0xff, b = 0xff;
    std::vector<vec> vertices;
    vec &ith_vec(int i);
};
