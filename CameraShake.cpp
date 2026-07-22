#include "CameraShake.h"

/// engine
#include "Engine.h"

#include "camera/CameraManager.h"

/// ECS
// component
#include "component/Camera/CameraShakeSourceComponent.h"
#include "component/transform/CameraTransform.h"

/// math
#include "math/MathEnv.h"
#include "math/Noise.h"

#include "EffectConfig.h"
#include "IShakeStrategy.h"

using namespace OriGine;

//=============================================================================
// Factory
//=============================================================================

// シェイクの種類に対応するIShakeStrategyの実体を返す（各戦略はstaticで使い回す）
static IShakeStrategy* GetShakeStrategy(ShakeSourceType _type) {
    static SinCurveShakeStrategy sinCurve;
    static NoiseShakeStrategy noise;
    static SpringShakeStrategy spring;

    switch (_type) {
    case ShakeSourceType::SinCurve: return &sinCurve;
    case ShakeSourceType::Noise:    return &noise;
    case ShakeSourceType::Spring:   return &spring;
    default:                        return nullptr;
    }
}

//=============================================================================
// CameraShake system
//=============================================================================

CameraShake::CameraShake() : ISystem(OriGine::SystemCategory::Effect) {}
CameraShake::~CameraShake() {}

void CameraShake::Initialize() {}
void CameraShake::Finalize() {}

void CameraShake::UpdateEntity(const OriGine::EntityHandle& _handle) {
    auto& cameraShakeSources = GetComponents<CameraShakeSourceComponent>(_handle);

    const float deltaTime = Engine::GetInstance()->GetDeltaTimer()->GetScaledDeltaTime("Effect");

    if (cameraShakeSources.empty()) {
        return;
    }
    for (auto& cameraShakeSource : cameraShakeSources) {
        auto cameraTransform = GetComponent<CameraTransform>(_handle, cameraShakeSource.GetCameraTransformIndex());
        if (!cameraTransform) {
            continue;
        }

        if (!cameraShakeSource.IsActive()) {
            continue;
        }

        cameraShakeSource.AddElapsedTime(deltaTime);
        // ループしない場合、経過時間が持続時間を超えたら非アクティブ化して終了
        if (!cameraShakeSource.IsLoop()) {
            if (cameraShakeSource.GetDuration() <= cameraShakeSource.GetElapsedTime()) {
                cameraShakeSource.SetActive(false);
                continue;
            }
        }

        IShakeStrategy* strategy = GetShakeStrategy(cameraShakeSource.GetType());
        if (!strategy) {
            continue;
        }

        // シェイク量を計算し、カメラのTransformへ加算反映する
        Vec3f shakeOffset = strategy->Calculate(cameraShakeSource, deltaTime);

        cameraTransform->translate += shakeOffset;
        cameraTransform->UpdateMatrix();
        CameraManager::GetInstance()->SetTransform(GetScene(), *cameraTransform);
    }
}
