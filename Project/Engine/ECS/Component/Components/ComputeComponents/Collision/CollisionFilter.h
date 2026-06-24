#pragma once

/// std
#include <cstdint>
/// externals
#include <magic_enum/magic_enum.hpp>

namespace ONEngine {

	///
	/// 衝突レイヤーの定義
	///
	enum class CollisionFilter : uint32_t{
		Default      = 1 << 0,
		Player       = 1 << 1,
		PlayerBullet = 1 << 2,
		Enemy        = 1 << 3,
		EnemyBullet  = 1 << 4,
		StageObject  = 1 << 5,
		Exp          = 1 << 6,
		ExpAttractor = 1 << 7,
		ALL = 0xFFFFFFFF
	};

}

template <>
struct magic_enum::customize::enum_range<ONEngine::CollisionFilter>{
	static constexpr int min = 0;
	static constexpr int max = 1024;
};