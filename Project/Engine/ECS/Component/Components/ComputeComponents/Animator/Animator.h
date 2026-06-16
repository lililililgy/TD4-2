#pragma once

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Asset/Assets/Mesh/Skinning.h"

namespace ONEngine {

/// @brief コンパイル定数による最大数の制限
static constexpr uint32_t MAX_ANIMATION_LAYERS = 4;
static constexpr uint32_t MAX_ANIMATION_STATES_PER_LAYER = 2;

/// @brief アニメーションステート (再生中のクリップ情報)
struct AnimationState {
    uint32_t clipId = 0;
    float time = 0.0f;
    float weight = 0.0f;
    bool isLoop = true;
    float playbackSpeed = 1.0f;

    // 前フレームの再生時間 (イベント検出用)
    float prevTime = 0.0f;
};

/// @brief アニメーションレイヤー
struct AnimationLayer {
    AnimationState states[MAX_ANIMATION_STATES_PER_LAYER];
    float weight = 1.0f;
    uint32_t boneMaskHash = 0; // 0 はマスクなし

    // トランジション（クロスフェード）用
    float transitionDuration = 0.0f;
    float transitionTimer = 0.0f;
};

/// @brief Animator コンポーネント
class Animator : public IComponent {
public:
    Animator();
    ~Animator() override = default;

    /// @brief アニメーションの再生
    void Play(uint32_t clipId, uint32_t layerIndex = 0);

    /// @brief クロスフェード (Phase 2で詳細実装)
    void CrossFade(uint32_t clipId, float duration, uint32_t layerIndex = 0);

    /// @brief 再生速度の設定
    void SetPlaybackSpeed(float speed, uint32_t layerIndex = 0);

    /// @brief ループ設定
    void SetLoop(bool isLoop, uint32_t layerIndex = 0);

    /// @brief 指定したクリップの再生時間を取得
    float GetAnimationDuration(uint32_t clipId) const;

    /// @brief デフォルトクリップの設定
    void SetDefaultClip(uint32_t clipId) { defaultClipId = clipId; }
    uint32_t GetDefaultClip() const { return defaultClipId; }

public:
    /// ----- objects ----- ///
    
    // 固定長バッファによるDoD最適化
    AnimationLayer layers[MAX_ANIMATION_LAYERS];

    uint32_t defaultClipId = 0;
};

/// @brief json変換
void from_json(const nlohmann::json& j, Animator& animator);
void to_json(nlohmann::json& j, const Animator& animator);

namespace ComponentDebug {
	void AnimatorDebug(Animator* animator);
	void AnimatorDebug(const std::vector<Animator*>& animators);
}

/// @brief mono からのAnimator操作用関数
void Internal_Play(uint64_t nativeHandle, uint32_t clipId, uint32_t layerIndex);
void Internal_CrossFade(uint64_t nativeHandle, uint32_t clipId, float duration, uint32_t layerIndex);
void Internal_SetPlaybackSpeed(uint64_t nativeHandle, float speed, uint32_t layerIndex);
void Internal_SetLoop(uint64_t nativeHandle, bool isLoop, uint32_t layerIndex);
float Internal_GetAnimationDuration(uint64_t nativeHandle, uint32_t clipId);

} // namespace ONEngine
