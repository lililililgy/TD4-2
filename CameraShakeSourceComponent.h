#pragma once

#include "component/IComponent.h"

/// math
#include "math/Vector3.h"

/// <summary>
/// シェイクの種類
/// </summary>
enum class ShakeSourceType {
    SinCurve, // サインカーブ
    Noise, // ノイズ
    Spring // バネ
};

/// <summary>
/// カメラシェイクの発生源コンポーネント
/// </summary>
class CameraShakeSourceComponent
    : public OriGine::IComponent {
    friend void to_json(nlohmann::json& _j, const CameraShakeSourceComponent& _c);
    friend void from_json(const nlohmann::json& _j, CameraShakeSourceComponent& _c);

public:
    CameraShakeSourceComponent();
    ~CameraShakeSourceComponent() override;

    void Initialize(OriGine::Scene* _scene, const OriGine::EntityHandle& _owner) override;
    void Edit(OriGine::Scene* _scene, const OriGine::EntityHandle& _owner, const std::string& _parentLabel) override;
    void Finalize() override;

    /// <summary>
    /// シェイクを開始する。経過時間とSpring用状態をリセットする。
    /// </summary>
    void StartShake();
    /// <summary>
    /// シェイクを停止する（非アクティブ化）。
    /// </summary>
    void StopShake();

public:
    /// <summary>
    /// シェイクのパラメータを設定する構造体
    /// </summary>
    struct ShakeParameters {
        float amplitude    = 1.0f; // シェイクの強さ
        float frequency    = 1.0f; // シェイクの速さ (Hz)
        float dampingRatio = 0.3f; // 減衰比 (0<ζ<1 で振動する、1で臨界減衰)
    };

public:
    bool IsActive() const { return isActive_; }
    void SetActive(bool _isActive) { isActive_ = _isActive; }

    ShakeSourceType GetType() const { return type_; }
    void SetType(ShakeSourceType _type) { type_ = _type; }

    int32_t GetCameraTransformIndex() const { return cameraTransformIndex_; }
    void SetCameraTransformIndex(int32_t _index) { cameraTransformIndex_ = _index; }

    OriGine::Vec3<ShakeParameters>& GetAxisParameters() { return axisParameters_; }
    const OriGine::Vec3<ShakeParameters>& GetAxisParameters() const { return axisParameters_; }

    bool IsLoop() const { return isLoop_; }
    void SetLoop(bool _isLoop) { isLoop_ = _isLoop; }

    float GetDuration() const { return duration_; }
    void SetDuration(float _duration) { duration_ = _duration; }

    float GetElapsedTime() const { return elapsedTime_; }
    void SetElapsedTime(float _elapsedTime) { elapsedTime_ = _elapsedTime; }
    void AddElapsedTime(float _deltaTime) { elapsedTime_ += _deltaTime; }

    OriGine::Vec3f& GetSpringPosition() { return springPosition_; }
    const OriGine::Vec3f& GetSpringPosition() const { return springPosition_; }
    OriGine::Vec3f& GetSpringVelocity() { return springVelocity_; }
    const OriGine::Vec3f& GetSpringVelocity() const { return springVelocity_; }

private:
    bool isActive_                = true; // シェイクの有効無効
    ShakeSourceType type_         = ShakeSourceType::SinCurve; // シェイクの種類
    int32_t cameraTransformIndex_ = -1; // シェイクを適用するカメラのTransformコンポーネントインデックス

    OriGine::Vec3<ShakeParameters> axisParameters_ = {
        {1.0f, 1.0f}, // X軸のパラメータ
        {1.0f, 1.0f}, // Y軸のパラメータ
        {1.0f, 1.0f} // Z軸のパラメータ
    };

    bool isLoop_       = true; // シェイクのループ有無
    float duration_    = 1.0f; // シェイクの継続時間
    float elapsedTime_ = 0.0f; // 経過時間

    // Spring 用の状態 (ランタイムのみ・シリアライズ不要)
    OriGine::Vec3f springPosition_ = OriGine::Vec3f(0.0f, 0.0f, 0.0f); // バネの現在変位
    OriGine::Vec3f springVelocity_ = OriGine::Vec3f(0.0f, 0.0f, 0.0f); // バネの現在速度
};
