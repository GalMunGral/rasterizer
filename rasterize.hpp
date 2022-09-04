#include <vector>
#include <stdexcept>
#include "buffer.hpp"

using vec = std::vector<double>;
using mat = std::vector<std::vector<double>>;

#define TEXTURE_SIZE 512

class rasterizer
{
public:
    int width = 0, height = 0;
    std::vector<unsigned char> &data();
    void resize(int w, int h);
    void add_vec(double x, double y, double z, double w);
    void set_color(double r, double g, double b, double a);
    void set_texcoord(double s, double t);
    void output();
    void draw_pixel(vec pixel);
    void draw_triangle(int i1, int i2, int i3);
    void draw_triangle(std::vector<vec> vertices);
    void enable_depth();
    void enable_srgb();
    void enable_perspective();
    void enable_frustum_clipping();
    void enable_fsaa(int level);
    void cull_face();
    void load_texture(std::string &filename);
    void enable_texture();
    void disable_texture();

private:
    double r = 255.0, g = 255.0, b = 255.0, a = 1.0, s = 0.0, t = 0.0;
    int fsaa_level = 1;
    bool depth_enabled;
    bool srgb_enabled;
    bool perspective_enabled;
    bool frustum_clipping_enabled;
    bool cull_enabled;
    bool texture_enabled;
    frame_buffer<unsigned char> texture;
    frame_buffer<unsigned char> output_buf;
    frame_buffer<double> render_buf;
    depth_buffer depth_buf;
    std::vector<vec> vertices;
    vec &ith_vec(int i);
};
