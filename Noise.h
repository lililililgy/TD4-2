#pragma once

/// math
#include <cmath>
#include <math/Vector2.h>
#include <math/Vector4.h>

namespace OriGine {
namespace FractalBrownianMotionNoise {

/// 参考:
// https://thebookofshaders.com/13/?lan=jp

/// <summary>
/// 小数部分を取得
/// </summary>
/// <param name="x"></param>
/// <returns></returns>
float Fract(float x);

/// <summary>
/// 各要素を切り捨て
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
Vec2f Floor(const Vec2f& v);

/// <summary>
/// 各要素の小数部分を取得
/// </summary>
/// <param name="v"></param>
/// <returns></returns>
Vec2f Fract(const Vec2f& v);

/// <summary>
/// 疑似乱数生成
/// </summary>
/// <param name="st"></param>
/// <returns></returns>
float Random(const Vec2f& st);

/// <summary>
/// 2D ノイズ
/// </summary>
/// <param name="st"></param>
/// <returns></returns>
float Noise(const Vec2f& st);

/// <summary>
/// fractal brownian motion ノイズ
/// </summary>
/// <param name="st"></param>
/// <returns></returns>
float Fbm(Vec2f st);

/// <summary>
///  ピクセル座標から色を取得
/// </summary>
/// <param name="fragCoord"></param>
/// <param name="resolution"></param>
/// <returns></returns>
Vec4f ShadePixel(const Vec2f& fragCoord, const Vec2f& resolution);

} // namespace FractalBrownianMotionNoise

namespace FbmNoise = FractalBrownianMotionNoise;

} // namespace OriGine
