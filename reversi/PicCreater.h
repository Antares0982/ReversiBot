#pragma clang diagnostic push
#pragma ide diagnostic ignored "cppcoreguidelines-narrowing-conversions"
//
// Created by antares on 4/26/22.
//

#ifndef MIRAICP_PICCREATER_H
#define MIRAICP_PICCREATER_H

#include <cmath>
#include <cstring>
#include <memory>
#include <string>

struct ColorType {
    unsigned char b;
    unsigned char g;
    unsigned char r;
};

class PicCreater {
    std::unique_ptr<unsigned char[]> pic_char;
    int width;
    int height;

public:
    PicCreater(const int _width, const int _height, ColorType _defaultColor = {0, 0, 0})
        : pic_char(new unsigned char[_width * _height * 3]), width(_width), height(_height) {
        if ((_defaultColor.r | _defaultColor.g | _defaultColor.b) == 0) {
            memset(pic_char.get(), 0, sizeof(unsigned char) * width * height * 3);
            return;
        }
        for (int i = 0; i < width * height; ++i) {
            memcpy(&pic_char[i * 3], &_defaultColor, 3 * sizeof(unsigned char));
        }
    }

    PicCreater(const PicCreater &_o) : pic_char(new unsigned char[_o.width * _o.height * 3]), width(_o.width), height(_o.height) {
        memcpy(&pic_char[0], &_o.pic_char[0], _o.width * _o.height * 3);
    }

    PicCreater(PicCreater &&_o) noexcept : pic_char(std::move(_o.pic_char)), width(_o.width), height(_o.height) {}

    virtual ~PicCreater() = default;

public:
    void writePixel(const int x, const int y, const ColorType &color) {
        memcpy(&pic_char[(x * width + y) * 3], &color, 3 * sizeof(unsigned char));
    }

    void drawCircle(const float centerx, const float centery, const float radius, const ColorType color) {
        int x = std::max(0, static_cast<int>(centerx - radius));
        if (centerx - x > radius) ++x;
        int y = static_cast<int>(centery - std::sqrt(radius * radius - (centerx - x) * (centerx - x))) - 1;
        for (; x - centerx < radius && x < width; ++x) {
            if (x < centerx)
                while (inCircle(x, y - 1, centerx, centery, radius)) --y;
            if (x >= centerx)
                while (!inCircle(x, y, centerx, centery, radius)) ++y;

            int ey = std::min(height - 1, static_cast<int>(2 * centery - y));
            for (int yy = std::max(0, y); yy <= ey; ++yy) {
                writePixel(x, yy, color);
            }
        }
    }

    void drawRectangle(const float x, const float y, const float mx, const float my, const ColorType color) {
        int xx = std::max(0, int(x));
        if (xx < x) ++xx;
        int sy = std::max(0, int(y));
        if (sy < y) ++sy;

        int maxx = std::min(width - 1, int(mx));
        int maxy = std::min(height - 1, int(my));

        for (; xx <= maxx; ++xx) {
            int yy = sy;
            for (; yy <= maxy; ++yy) writePixel(xx, yy, color);
        }
    }

    void save(const std::string &_path);

private:
    static bool inCircle(const float x, const float y, const float ax, const float ay, const float r) {
        return (x - ax) * (x - ax) + (y - ay) * (y - ay) < r * r;
    }
};

#endif //MIRAICP_PICCREATER_H

#pragma clang diagnostic pop