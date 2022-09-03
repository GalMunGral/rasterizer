#include <vector>
#include "lodepng.h"
#define cimg_display 0

class image
{
public:
    int width;
    int height;
    void resize(int w, int h);
    unsigned char &operator()(int x, int y, int channel);
    unsigned char *data();
    void set_color(int x, int y, int r, int g, int b, int a);
    void set_color(int x, int y, std::string &hex_color);
    void save(std::string &filename);

private:
    std::vector<unsigned char> buffer;
};
