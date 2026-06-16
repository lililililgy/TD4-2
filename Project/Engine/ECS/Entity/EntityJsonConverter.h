#pragma once

/// external
#include <nlohmann/json.hpp>

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"

namespace ONEngine {

namespace EntityJsonConverter {
nlohmann::json ToJson(const GameEntity* entity, bool forceFull = false);


/// @brief JsonからGameEntityを生成する
/// @param json GameEntityのJsonデータ
/// @param entity 生成に使用するGameEntityのポインタ
/// @param groupName 生成元のECSGroup名
void FromJson(const nlohmann::json& json, GameEntity* entity, const std::string& groupName, bool merge = true);

/// @brief Transform専用のJsonからGameEntityを生成する
/// @param json 生成元のJsonデータ
/// @param entity 生成先のGameEntityのポインタ
void TransformFromJson(const nlohmann::json& json, GameEntity* entity);
};

} /// namespace ONEngine