#include "rasterize.hpp"

std::ostream &operator<<(std::ostream &os, const vec &v)
{
    for (auto m : v)
    {
        os << m << ',';
    }
    return os;
}

vec operator+(const vec &a, const vec &b)
{
    auto n = std::max(a.size(), b.size());
    vec res(n);
    while (n--)
    {
        auto va = n < a.size() ? a[n] : 0;
        auto vb = n < b.size() ? b[n] : 0;
        res[n] = va + vb;
    }
    return res;
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
    {
        auto va = n < a.size() ? a[n] : 0;
        auto vb = n < b.size() ? b[n] : 0;
        res[n] = va - vb;
    }
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

rasterizer::rasterizer()
    : width{0}, height{0},
      r{255.0}, g{255.0}, b{255.0}, a{1.0},
      s{0.0}, t{0.0},
      fsaa_level{1},
      clip_planes{
          {1.0, 0, 0, 1.0},
          {-1.0, 0, 0, 1.0},
          {0, 1.0, 0, 1.0},
          {0, -1.0, 0, 1.0},
          {0, 0, 1.0, 1.0},
          {0, 0, -1.0, 1.0}}
{
    // std::cout << "FSAA::::" << fsaa_level;
}

std::vector<unsigned char> &rasterizer::data()
{
    return output_buf.data();
}

void rasterizer::resize(int w, int h)
{
    width = w;
    height = h;
    output_buf = frame_buffer<unsigned char>(w, h);
    render_buf = frame_buffer<double>(w * fsaa_level, h * fsaa_level);
    depth_buf = depth_buffer(w * fsaa_level, h * fsaa_level);
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

void rasterizer::enable_fsaa(int level)
{
    fsaa_level = level;
    render_buf = frame_buffer<double>(width * level, height * level);
    depth_buf = depth_buffer(width * level, height * level);
}

void rasterizer::cull_face()
{
    cull_enabled = true;
}

void rasterizer::add_vec(double x, double y, double z, double w)
{
    vertices.push_back({x, y, z, w, r, g, b, a, s, t});
}

vec &rasterizer::nth_vertex(int i)
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

void rasterizer::set_color(double _r, double _g, double _b, double _a)
{
    r = _r;
    g = _g;
    b = _b;
    a = _a;
}

void rasterizer::set_texcoord(double _s, double _t)
{
    s = _s;
    t = _t;
}

void rasterizer::load_texture(std::string &filename)
{
    std::cout << filename << std::endl;
    // lodepng::decode(texture.data(), texture.width, texture.height, filename, LCT_RGBA, 8);
    // TODO: load png
    texture.data() = std::vector<uint8_t>(10000);
    for (int i = 0; i < 10000; ++i) texture.data()[i] = rand() % 256;
    texture.height = 50;
    texture.width = 50;
}

void rasterizer::enable_texture()
{
    texture_enabled = true;
}

void rasterizer::disable_texture()
{
    texture_enabled = false;
}

void rasterizer::enable_decals()
{
    decals_enabled = true;
}

void rasterizer::clip(double p1, double p2, double p3, double p4)
{
    clip_planes.push_back({p1, p2, p3, p4});
    // clip_planes = {{p1, p2, p3, p4}};
}

vec rasterizer::project(vec in)
{
    vec out(in);
    if (srgb_enabled)
    {
        out[4] = srgb_to_linear(out[4] / 255.0);
        out[5] = srgb_to_linear(out[5] / 255.0);
        out[6] = srgb_to_linear(out[6] / 255.0);
    }
    auto w = out[3];
    out[0] = (out[0] / w + 1) * render_buf.width / 2;
    out[1] = (out[1] / w + 1) * render_buf.height / 2;
    out[2] /= w;
    out[3] = 1 / w;
    if (perspective_enabled)
    {
        // TODO: there are some bugs...
        for (size_t i = 4; i < out.size(); ++i)
        {
            out[i] /= w;
        }
    }
    return out;
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

inline vec alpha_blend(vec src, vec dst)
{
    // "over" operator
    auto a = src[3] + dst[3] * (1.0 - src[3]);
    auto as = src[3], ad = (dst[3] * (1.0 - src[3]));
    auto r = (as * src[0] + ad * dst[0]) / a;
    auto g = (as * src[1] + ad * dst[1]) / a;
    auto b = (as * src[2] + ad * dst[2]) / a;
    return {r, g, b, a};
}

void rasterizer::draw_pixel(vec v) // copy since it will be modified
{
    if (perspective_enabled)
    {
        for (size_t i = 4; i < v.size(); ++i)
        {
            v[i] /= v[3];
        }
    }

    unsigned x = v[0], y = v[1];
    if (x >= render_buf.width || y >= render_buf.height)
    {
        // TODO: this should only happen when drawing points
        return;
    }

    vec cs, cd;
    cd = {
        render_buf(x, y, 0),
        render_buf(x, y, 1),
        render_buf(x, y, 2),
        render_buf(x, y, 3)};

    if (texture_enabled)
    {
        // TODO: round ?? there are some differences...
        auto s = v[8], t = v[9];
        int x = static_cast<int>(s * texture.width + 0.5) % texture.width;
        int y = static_cast<int>(t * texture.height + 0.5) % texture.height;

        cs = {
            texture(x, y, 0) / 1.0,
            texture(x, y, 1) / 1.0,
            texture(x, y, 2) / 1.0,
            texture(x, y, 3) / 255.0};

        if (srgb_enabled)
        {
            cs[0] = srgb_to_linear(cs[0] / 255.0);
            cs[1] = srgb_to_linear(cs[1] / 255.0);
            cs[2] = srgb_to_linear(cs[2] / 255.0);
        }
        if (decals_enabled)
        {
            cs = alpha_blend(cs, {v[4], v[5], v[6], v[7]});
        }
    }
    else
    {

        cs = {v[4], v[5], v[6], v[7]};
    }

    vec c = alpha_blend(cs, cd);
    if (depth_enabled)
    {
        if (v[2] >= -1.0 && v[2] < depth_buf(v[0], v[1]))
        {
            render_buf.set_color(x, y, c[0], c[1], c[2], c[3]);
            depth_buf(x, y) = v[2];
        }
    }
    else
    {
        render_buf.set_color(x, y, c[0], c[1], c[2], c[3]);
    }
}

void rasterizer::draw_triangle(tri triangle)
{
    for (auto &v : triangle)
        v = project(v);

    sort(triangle.begin(), triangle.end(), [](vec &a, vec &b)
         { return a[1] <= b[1]; });

    std::vector<vec> bound1, bound2;
    dda_scan(triangle[0], triangle[2], 1, [&](vec &v)
             { bound1.push_back(v); });
    dda_scan(triangle[0], triangle[1], 1, [&](vec &v)
             { bound2.push_back(v); });
    dda_scan(triangle[1], triangle[2], 1, [&](vec &v)
             { bound2.push_back(v); });
            
    assert(bound1.size() == bound2.size());
    auto n = bound1.size();
    for (size_t i = 0; i < n; ++i)
    {
        dda_scan(bound1[i], bound2[i], 0, [&](vec v)
                 { draw_pixel(v); });
    }
}

vec intersect(const vec &p1, const vec &p2, const vec &plane)
{
    auto d1 = plane * p1, d2 = plane * p2;
    return (d2 * p1 - d1 * p2) / (d2 - d1);
}

void rasterizer::draw_triangle_clipped(tri triangle)
{
    std::queue<std::array<vec, 3>> queue;
    queue.push(triangle);

    // TODO: check w > 0

    // use one clip plane at a time
    for (vec &plane : clip_planes)
    {
        // clip all triangles
        auto n = queue.size();
        while (n--)
        {
            auto triangle = queue.front();
            queue.pop();

            std::vector<vec> out, in;
            for (auto &v : triangle)
            {
                if (plane * v >= 0)
                    in.push_back(v);
                else
                    out.push_back(v);
            }

            if (in.size() == 3)
            {
                queue.push(triangle);
            }
            else if (in.size() == 2)
            {
                auto v1 = intersect(in[0], out[0], plane);
                auto v2 = intersect(in[1], out[0], plane);
                queue.push({in[0], in[1], v1});
                queue.push({v1, v2, in[1]});
            }
            else if (in.size() == 1)
            {
                auto v1 = intersect(in[0], out[0], plane);
                auto v2 = intersect(in[0], out[1], plane);
                queue.push({v1, v2, in[0]});
            }
        }
    }

    while (queue.size())
    {
        auto triangle = queue.front();
        queue.pop();
        draw_triangle(triangle);
    }
}

vec normal(const tri &triangle)
{
    vec a = triangle[1] - triangle[0];
    vec b = triangle[2] - triangle[1];
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]};
}

void rasterizer::draw_triangle(int i1, int i2, int i3)
{
    tri triangle = {nth_vertex(i1),
                    nth_vertex(i2),
                    nth_vertex(i3)};

    // facing down (+z direction)
    if (cull_enabled && normal(triangle)[2] >= 0)
        return;

    // TODO: always?
    if (frustum_clipping_enabled || true)
    {
        draw_triangle_clipped(triangle);
    }
    else
    {
        draw_triangle(triangle);
    }
}

void rasterizer::draw_point(int i, double size)
{
    auto w = size / 2;
    vec o = project(nth_vertex(i));
    vec v1 = {o[0] - w, o[1] - w, o[2], o[3], o[4], o[5], o[6], o[7], 0, 0};
    vec v2 = {o[0] - w, o[1] + w, o[2], o[3], o[4], o[5], o[6], o[7], 0, 1};
    // this might cause points to be off-screen
    dda_scan(v1, v2, 1, [&](vec l)
             { dda_scan(l, l + vec{size, 0, 0, 0, 0, 0, 0, 0, 1, 0}, 0, [&](vec p)
                        { draw_pixel(p); }); });
}

void rasterizer::draw_line(int i1, int i2)
{
    // TODO: what about color
    auto v1 = project(nth_vertex(i1)), v2 = project(nth_vertex(i2));
    auto d0 = std::abs(v1[0] - v2[0]), d1 = std::abs(v1[1] - v2[1]);
    int i = d0 > d1 ? 0 : 1, j = i ^ 1;
    dda_scan(v1, v2, i, [&](vec p)
             {  p[j] = std::round(p[j]);
                draw_pixel(p); });
}

void rasterizer::draw_wuline(int i1, int i2)
{
    // TODO: only rgb??
    // TODO: what about color
    auto v1 = project(nth_vertex(i1)), v2 = project(nth_vertex(i2));
    auto d0 = std::abs(v1[0] - v2[0]), d1 = std::abs(v1[1] - v2[1]);
    int i = d0 > d1 ? 0 : 1, j = i ^ 1;
    dda_scan(v1, v2, i, [&](vec p)
             {
                 auto x = p[j];
                 auto d = x - std::floor(x);
                 p[j] = std::floor(x);
                 p[7] = 1 - d;
                 draw_pixel(p);
                 p[j] = std::ceil(x);
                 p[7] = d;
                 draw_pixel(p); });
}

void rasterizer::output()
{
    int out_height = output_buf.height, out_width = output_buf.width;
    for (int x = 0; x < out_width; ++x)
    {
        for (int y = 0; y < out_height; ++y)
        {
            double r = 0.0, g = 0.0, b = 0.0, a = 0.0;
            for (int i = 0; i < fsaa_level; ++i)
            {
                for (int j = 0; j < fsaa_level; ++j)
                {
                    auto src_x = fsaa_level * x + j;
                    auto src_y = fsaa_level * y + i;
                    auto alpha = render_buf(src_x, src_y, 3);
                    r += alpha * render_buf(src_x, src_y, 0);
                    g += alpha * render_buf(src_x, src_y, 1);
                    b += alpha * render_buf(src_x, src_y, 2);
                    a += alpha;
                }
            }
            if (a)
            {
                r /= a;
                g /= a;
                b /= a;
                a /= fsaa_level * fsaa_level;
            }
            if (srgb_enabled)
            {
                r = linear_to_srgb(r) * 255.0;
                g = linear_to_srgb(g) * 255.0;
                b = linear_to_srgb(b) * 255.0;
            }
            a *= 255.0;
            output_buf.set_color(x, y, r, g, b, a);
        }
    }
}
