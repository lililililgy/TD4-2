#pragma once

/// std
#include <optional>
#include <cstdint>
#include <unordered_map>

/// engine
#include "../Interface/ECSISystem.h"
#include "Engine/Asset/Assets/Mesh/Skinning.h"

namespace ONEngine {

/// @brief Animator コンポーネントの更新と、SkinMeshRenderer への適用を行うシステム
class AnimatorUpdateSystem : public ECSISystem {
public:
    AnimatorUpdateSystem() = default;
    ~AnimatorUpdateSystem() override = default;

    /// @brief 更新
    void RuntimeUpdate(class ECSGroup* ecs) override;

    /// @brief ボーンマスクの登録
    static void RegisterBoneMask(const BoneMask& boneMask);

private:
    void UpdateSkeletonRecursive(class SkinMeshRenderer* smr, int32_t jointIndex, const std::optional<int32_t>& parentIndex);
    void UpdateSkinCluster(class SkinMeshRenderer* smr);

    static std::unordered_map<uint32_t, BoneMask> boneMasks_;
};

} // namespace ONEngine
