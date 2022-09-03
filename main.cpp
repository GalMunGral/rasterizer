#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "lodepng.h"

#define cimg_display 0

std::vector<unsigned char> img;
std::string filename;
int width, height;

inline unsigned char hex(char c) {
    return isdigit(c) ? c - '0' : c - 'a' + 10;
}

inline unsigned char byte(std::string& s, int i) {
    return hex(s[i]) << 4 | hex(s[i + 1]);
}

inline unsigned char& pixel(int x, int y, int c) {
    return img[(y * width + x) * 4 + c];
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "No filename specified\n";
        return -1;
    }
    std::ifstream file (argv[1]);
    for (std::string line; std::getline(file, line);) {
        std::istringstream ss (line);
        std::string cmd;
        ss >> cmd;
        if (cmd == "png") {
            ss >> width >> height >> filename;
            img.resize(width * height * 4);
        } else if (cmd == "xyrgb") {
            int x, y, r, g, b;
            ss >> x >> y >> r >> g >> b;
            if (x >= 0 && x < width && y >= 0 && y < height) {
                pixel(x, y, 0) = r;
                pixel(x, y, 1) = g;
                pixel(x, y, 2) = b;
                pixel(x, y, 3) = 0xff;
            }
        } else if (cmd == "xyc") {
            int x, y;
            std::string hex_color;
            ss >> x >> y >> hex_color;
            pixel(x, y, 0) = byte(hex_color, 1);
            pixel(x, y, 1) = byte(hex_color, 3);
            pixel(x, y, 2) = byte(hex_color, 5);
            pixel(x, y, 3) = 0xff;
        }
    }
    int res;
    if ((res = lodepng_encode_file(filename.c_str(), img.data(), width, height, LCT_RGBA, 8))) {
        std::cerr << lodepng_error_text(res) << '\n';
        return -1;
    }
    return 0;
}
