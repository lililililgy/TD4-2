#include "Noise.h"

namespace OriGine {
namespace FractalBrownianMotionNoise {

float Fract(float x) {
    return x - std::floor(x);
}

Vec2f Floor(const Vec2f& v) {
    return Vec2f(std::floor(v[X]), std::floor(v[Y]));
}

Vec2f Fract(const Vec2f& v) {
    return Vec2f(Fract(v[X]), Fract(v[Y]));
}

float Random(const Vec2f& st) {
    return Fract(std::sin(st[X] * 12.9898f + st[Y] * 78.233f) * 43758.5453123f);
}

float Noise(const Vec2f& st) {
    Vec2f i = Floor(st);
    Vec2f f = Fract(st);

    float a = Random(i);
    float b = Random(Vec2f(i[X] + 1.0f, i[Y]));
    float c = Random(Vec2f(i[X], i[Y] + 1.0f));
    float d = Random(Vec2f(i[X] + 1.0f, i[Y] + 1.0f));

    Vec2f u(f[X] * f[X] * (3.0f - 2.0f * f[X]),
        f[Y] * f[Y] * (3.0f - 2.0f * f[Y]));

    return std::lerp(a, b, u[X])
           + (c - a) * u[Y] * (1.0f - u[X])
           + (d - b) * u[X] * u[Y];
}

float Fbm(Vec2f st) {
    constexpr int kOctaves = 6;
    float value            = 0.0f;
    float amplitude        = 0.5f;

    for (int i = 0; i < kOctaves; i++) {
        value += amplitude * Noise(st);
        st[X] *= 2.0f;
        st[Y] *= 2.0f;
        amplitude *= 0.5f;
    }
    return value;
}

Vec4f ShadePixel(const Vec2f& fragCoord, const Vec2f& resolution) {
    Vec2f st(fragCoord[X] / resolution[X],
        fragCoord[Y] / resolution[Y]);

    // アスペクト比補正
    st[X] *= resolution[X] / resolution[Y];

    Vec3f color(0, 0, 0);
    float f = Fbm(Vec2f(st[X] * 3.0f, st[Y] * 3.0f));
    color   = Vec3f(f, f, f);

    return Vec4f(color[X], color[Y], color[Z], 1.0f);
}

} // namespace FractalBrownianMotionNoise
} // namespace OriGine
