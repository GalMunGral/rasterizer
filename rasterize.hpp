#include <vector>
#include <stdexcept>
#include "display.hpp"

using vertex = std::vector<double>;

class rasterizer
{
public:
    void add_vertex(double x, double y, double z, double w);
    void set_color(double r, double g, double b);
    void draw_triangle(display &disp, int i1, int i2, int i3);

private:
    double r = 0xff, g = 0xff, b = 0xff;
    std::vector<vertex> vertices;
    vertex &ith_vertex(int i);
};
