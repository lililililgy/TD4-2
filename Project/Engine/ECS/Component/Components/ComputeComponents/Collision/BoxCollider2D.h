#pragma once

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "ICollider.h"
#include "Engine/Core/Utility/Math/Vector2.h"

namespace ONEngine {

class BoxCollider2D;

namespace ComponentDebug {
/// @brief BoxCollider2Dのデバッグ表示
/// @param boxCollider2D 
void BoxCollider2DDebug(BoxCollider2D* boxCollider2D);
}

/// //////////////////////////////////////
/// BoxCollider2D
/// //////////////////////////////////////
class BoxCollider2D : public ICollider {
	friend void ComponentDebug::BoxCollider2DDebug(BoxCollider2D* boxCollider2D);
	friend void from_json(const nlohmann::json& j, BoxCollider2D& b);
	friend void to_json(nlohmann::json& j, const BoxCollider2D& b);
public:
	/// ====================================================
	/// public : methods
	/// ====================================================

	BoxCollider2D();
	~BoxCollider2D() override = default;

private:
	/// =====================================================
	/// private : objects
	/// =====================================================

	Vector2 size_;

public:
	/// =====================================================
	/// public : accessors
	/// =====================================================

	void SetSize(const Vector2& size);
	const Vector2& GetSize() const;

};

} /// ONEngine
