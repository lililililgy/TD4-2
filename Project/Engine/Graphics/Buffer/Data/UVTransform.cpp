#include "UVTransform.h"

using namespace ONEngine;


/// ------------------------------------------------
/// UVTransform Json変換
/// ------------------------------------------------

void ONEngine::to_json(nlohmann::json& j, const UVTransform& uvTransform) {
	j = nlohmann::json{
		{ "position", uvTransform.position },
		{ "scale", uvTransform.scale },
		{ "rotate", uvTransform.rotate },
	};
}

void ONEngine::from_json(const nlohmann::json& j, UVTransform& uvTransform) {
	uvTransform.position = j.value("position", Vector2::Zero);
	uvTransform.scale = j.value("scale", Vector2::One);
	uvTransform.rotate = j.value("rotate", 0.0f);
}
