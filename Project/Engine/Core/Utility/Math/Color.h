#pragma once

/// std
#include <cstdint>

/// engine
#include "Vector4.h"

/// ///////////////////////////////////////////////////
/// Colorクラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

struct Color final {
	/// ===================================================  
	/// public : static methods, objects
	/// ===================================================  

	/// @brief HSVからRGBに変換する
	/// @param h 色相
	/// @param s 彩度
	/// @param v 明度
	/// @return RGB
	static Vector4 HSVtoRGB(float h, float s, float v);

	static const Color kWhite; ///< 白
	static const Color kBlack; ///< 黒
	static const Color kRed;   ///< 赤
	static const Color kGreen; ///< 緑
	static const Color kBlue;  ///< 青

	/// ===================================================  
	/// public : methods  
	/// ===================================================  
	Color() = default;
	Color(const Vector4& color) : r(color.x), g(color.y), b(color.z), a(color.w) {}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
	Color(uint32_t colorCode);
	~Color() = default;


	/// @brief HSVからRGBに変換してセットする
	/// @param h 色相
	/// @param s 彩度
	/// @param v 明度
	void SetHSVtoRGB(float h, float s, float v);

	/// @brief カラーコードからRGBAに変換してセットする
	/// @param colorCode カラーコード 
	void SetColorCode(uint32_t colorCode);

	/// ===================================================  
	/// public : objects  
	/// ===================================================  

	float r; ///< 赤
	float g; ///< 緑
	float b; ///< 青
	float a; ///< アルファ


	/// ===================================================  
	/// public : operator  
	/// ===================================================  

	inline operator Vector4() const { return { r, g, b, a }; } ///< Vector4に変換

};

} /// ONEngine
