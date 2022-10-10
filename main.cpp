#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <emscripten/fetch.h>
#include <SDL.h>
#include "rasterize.hpp"

rasterizer raster;
std::string filename;

SDL_Surface *screen;

// HACK
int scale;

void drawRandomPixels()
{
    if (!screen) return;

    if (SDL_MUSTLOCK(screen))
        SDL_LockSurface(screen);

    int n = raster.data().size();
    for (int i = 0; i < n; ++i)
        ((uint8_t *)screen->pixels)[i] = raster.data()[i];

    if (SDL_MUSTLOCK(screen))
        SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}

void downloadSucceeded(emscripten_fetch_t *fetch)
{
    // printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through fetch->data[fetch->numBytes-1];
    std::stringstream file;
    file << fetch->data;
    emscripten_fetch_close(fetch); // Free data associated with the fetch.

    for (std::string line; std::getline(file, line);)
    {
        std::istringstream ss(line);
        std::string cmd;
        ss >> cmd;
        
        if (cmd.size()) std::cout << line << std::endl;

        if (cmd == "png")
        {
            int width, height;
            ss >> width >> height >> filename;

            scale = (500 + height - 1) / height;
            width *= scale;
            height *= scale;

            std::cout << width << height << std::endl;

            // TODO resize
            screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
            raster.resize(width, height);
        }
        else if (cmd == "xyzw")
        {
            double x, y, z, w;
            ss >> x >> y >> z >> w;

            x *= scale;
            y *= scale;
            z *= scale;
            w *= scale;

            raster.add_vec(x, y, z, w);
        }
        else if (cmd == "rgb")
        {
            double r, g, b;
            ss >> r >> g >> b;
            raster.set_color(r, g, b, 1.0);
        }
        else if (cmd == "tri")
        {
            int i1, i2, i3;
            ss >> i1 >> i2 >> i3;
            raster.disable_texture();
            raster.draw_triangle(i1, i2, i3);
        }
        else if (cmd == "depth")
        {
            raster.enable_depth();
        }
        else if (cmd == "sRGB")
        {
            raster.enable_srgb();
        }
        else if (cmd == "rgba")
        {
            // TODO: after sRGB
            double r, g, b, a;
            ss >> r >> g >> b >> a;
            raster.set_color(r, g, b, a);
        }
        else if (cmd == "hyp")
        {
            // TODO: only after sRGB
            raster.enable_perspective();
        }
        else if (cmd == "frustum")
        {
            raster.enable_frustum_clipping();
        }
        else if (cmd == "fsaa")
        {
            int level;
            ss >> level;
            raster.enable_fsaa(level);
        }
        else if (cmd == "cull")
        {
            raster.cull_face();
        }
        else if (cmd == "texcoord")
        {
            double u, v;
            ss >> u >> v;
            raster.set_texcoord(u, v);
        }
        else if (cmd == "texture")
        {
            std::string filename;
            ss >> filename;
            raster.load_texture(filename);
        }
        else if (cmd == "trit")
        {
            int i1, i2, i3;
            ss >> i1 >> i2 >> i3;
            raster.enable_texture();
            raster.draw_triangle(i1, i2, i3);
        }
        else if (cmd == "point")
        {
            int i;
            double size;
            ss >> size >> i;

            size *= scale;

            raster.disable_texture();
            raster.draw_point(i, size);
        }
        else if (cmd == "billboard")
        {
            int i;
            double size;
            ss >> size >> i;

            size *= scale;

            raster.enable_texture();
            raster.draw_point(i, size);
        }
        else if (cmd == "decals")
        {
            raster.enable_decals();
        }
        else if (cmd == "clipplane")
        {
            double p1, p2, p3, p4;
            ss >> p1 >> p2 >> p3 >> p4;

            p1 *= scale;
            p2 *= scale;
            p3 *= scale;
            p4 *= scale;
            
            raster.clip(p1, p2, p3, p4);
        }
        else if (cmd == "line")
        {
            int i1, i2;
            ss >> i1 >> i2;
            raster.draw_line(i1, i2);
        }
        else if (cmd == "wuline")
        {
            int i1, i2;
            ss >> i1 >> i2;
            raster.draw_wuline(i1, i2);
        }
    }
    raster.output();
}

void downloadFailed(emscripten_fetch_t *fetch)
{
    printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
    emscripten_fetch_close(fetch); // Also free data on failure.
}

EM_JS(const char *, get_input_filename, (), {
    const filename = new URLSearchParams(location.search).get('file');
    const jsString = 'inputs/' + filename;
    const lengthBytes = lengthBytesUTF8(jsString) + 1;
    const stringOnWasmHeap = _malloc(lengthBytes);
    stringToUTF8(jsString, stringOnWasmHeap, lengthBytes);
    return stringOnWasmHeap;
})

int main(void)
{
    SDL_Init(SDL_INIT_VIDEO);

    const char *filenmae = get_input_filename();

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    emscripten_fetch(&attr, filenmae);

    emscripten_set_main_loop(drawRandomPixels, 60, 1);

    return 0;
}
