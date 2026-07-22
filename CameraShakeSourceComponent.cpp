
#include "CameraShakeSourceComponent.h"

#ifdef _DEBUG
/// engine
#include "scene/Scene.h"

/// ECS
// component
#include "component/transform/CameraTransform.h"

/// GUI
#include "myGui/MyGui.h"
#endif // _DEBUG

/// util
#include "myRandom/MyRandom.h"

using namespace OriGine;

CameraShakeSourceComponent::CameraShakeSourceComponent() : IComponent() {}
CameraShakeSourceComponent::~CameraShakeSourceComponent() {}

void CameraShakeSourceComponent::Initialize(Scene* /*_scene*/, const EntityHandle& /*_owner*/) {
    if (isActive_) {
        StartShake();
    }
}
void CameraShakeSourceComponent::Finalize() {}

void CameraShakeSourceComponent::StartShake() {
    isActive_ = true;

    elapsedTime_ = 0.0f;

    // Spring 用初期化
    springVelocity_ = Vec3f(0.0f, 0.0f, 0.0f);
    for (size_t i = 0; i < 3; ++i) {
        // 軸ごとに正負をランダムに決定し、初期変位を振幅分だけずらす
        float sign         = MyRandom::Float(-1.f, 1.f).Get() >= 0.f ? 1.f : -1.f;
        springPosition_[i] = sign * axisParameters_[i].amplitude;
    }
}

void CameraShakeSourceComponent::StopShake() {
    isActive_ = false;
}

void CameraShakeSourceComponent::Edit([[maybe_unused]] OriGine::Scene* _scene, [[maybe_unused]] const EntityHandle& _owner, [[maybe_unused]] const std::string& _parentLabel) {
#ifdef _DEBUG

    std::string label = "";

    // シェイクの種類
    {
        const char* items[] = {"SinCurve", "Noise", "Spring"};
        int currentItem     = static_cast<int>(type_);

        ImGui::Text("Shake Type :");
        label = "##ShakeType" + _parentLabel;
        if (ImGui::BeginCombo(label.c_str(), items[currentItem])) {
            bool isSelected        = false;
            int32_t shakeTypeIndex = 0;
            for (auto* shakeTypeName : items) {
                isSelected = shakeTypeName == items[currentItem];

                if (ImGui::Selectable(shakeTypeName, isSelected)) {
                    OriGine::EditorController::GetInstance()->PushCommand(
                        std::make_unique<SetterCommand<ShakeSourceType>>(&type_, static_cast<ShakeSourceType>(shakeTypeIndex)));

                    break;
                }

                ++shakeTypeIndex;
            }
            ImGui::EndCombo();
        }
    }

    ImGui::Spacing();

    auto& cameraTransforms     = _scene->GetComponents<CameraTransform>(_owner);
    int32_t entityMaterialSize = static_cast<int32_t>(cameraTransforms.size());
    // カメラTransformコンポーネントインデックス
    InputGuiCommand<int32_t>("Camera Transform Index##" + _parentLabel, cameraTransformIndex_, "%d", [this, entityMaterialSize](int32_t* _newT) {
        this->cameraTransformIndex_ = std::clamp(*_newT, -1, entityMaterialSize - 1);
    });

    ImGui::Spacing();

    label = "Start Shake ##" + _parentLabel;
    if (ImGui::Button(label.c_str())) {
        StartShake();
    }

    CheckBoxCommand("Is Active##" + _parentLabel, isActive_);
    // ループ有無
    CheckBoxCommand("Loop##" + _parentLabel, isLoop_);
    // 継続時間
    DragGuiCommand("Duration##" + _parentLabel, duration_, 0.01f);

    ImGui::Spacing();

    label = "Spring Parameters##" + _parentLabel;

    if (ImGui::TreeNode(label.c_str())) {
        DragVectorGui("Spring Position##" + _parentLabel, springPosition_, 0.01f);
        DragVectorGui("Spring Velocity##" + _parentLabel, springVelocity_, 0.01f);

        ImGui::TreePop();
    }

    ImGui::Spacing();

    label = "X##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        // シェイクの強さ
        DragGuiCommand("Amplitude##X" + _parentLabel, axisParameters_[X].amplitude, 0.01f);
        // シェイクの速さ
        DragGuiCommand("Frequency##X" + _parentLabel, axisParameters_[X].frequency, 0.01f);
        // 減衰比 (Spring専用)
        DragGuiCommand("Damping Ratio##X" + _parentLabel, axisParameters_[X].dampingRatio, 0.01f);

        ImGui::TreePop();
    }

    ImGui::Spacing();

    label = "Y##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        // シェイクの強さ
        DragGuiCommand("Amplitude##Y" + _parentLabel, axisParameters_[Y].amplitude, 0.01f);
        // シェイクの速さ
        DragGuiCommand("Frequency##Y" + _parentLabel, axisParameters_[Y].frequency, 0.01f);
        // 減衰比 (Spring専用)
        DragGuiCommand("Damping Ratio##Y" + _parentLabel, axisParameters_[Y].dampingRatio, 0.01f);

        ImGui::TreePop();
    }

    ImGui::Spacing();

    label = "Z##" + _parentLabel;
    if (ImGui::TreeNode(label.c_str())) {
        // シェイクの強さ
        DragGuiCommand("Amplitude##Z" + _parentLabel, axisParameters_[Z].amplitude, 0.01f);
        // シェイクの速さ
        DragGuiCommand("Frequency##Z" + _parentLabel, axisParameters_[Z].frequency, 0.01f);
        // 減衰比 (Spring専用)
        DragGuiCommand("Damping Ratio##Z" + _parentLabel, axisParameters_[Z].dampingRatio, 0.01f);

        ImGui::TreePop();
    }

#endif // _DEBUG
}

void to_json(nlohmann::json& _j, const CameraShakeSourceComponent& _c) {
    _j = nlohmann::json{
        {"isActive", _c.isActive_},
        {"type", static_cast<int32_t>(_c.type_)},
        {"cameraTransformIndex", _c.cameraTransformIndex_},
        {"isLoop", _c.isLoop_},
        {"duration", _c.duration_},
        {"X_Parameter_Amplitude", _c.axisParameters_[X].amplitude},
        {"X_Parameter_Frequency", _c.axisParameters_[X].frequency},
        {"X_Parameter_DampingRatio", _c.axisParameters_[X].dampingRatio},
        {"Y_Parameter_Amplitude", _c.axisParameters_[Y].amplitude},
        {"Y_Parameter_Frequency", _c.axisParameters_[Y].frequency},
        {"Y_Parameter_DampingRatio", _c.axisParameters_[Y].dampingRatio},
        {"Z_Parameter_Amplitude", _c.axisParameters_[Z].amplitude},
        {"Z_Parameter_Frequency", _c.axisParameters_[Z].frequency},
        {"Z_Parameter_DampingRatio", _c.axisParameters_[Z].dampingRatio},
    };
}

void from_json(const nlohmann::json& _j, CameraShakeSourceComponent& _c) {
    _j.at("isActive").get_to(_c.isActive_);
    int32_t typeInt;
    _j.at("type").get_to(typeInt);
    _c.type_ = static_cast<ShakeSourceType>(typeInt);
    _j.at("cameraTransformIndex").get_to(_c.cameraTransformIndex_);
    _j.at("isLoop").get_to(_c.isLoop_);
    _j.at("duration").get_to(_c.duration_);
    _j.at("X_Parameter_Amplitude").get_to(_c.axisParameters_[X].amplitude);
    _j.at("X_Parameter_Frequency").get_to(_c.axisParameters_[X].frequency);
    if (_j.contains("X_Parameter_DampingRatio")) {
        _j.at("X_Parameter_DampingRatio").get_to(_c.axisParameters_[X].dampingRatio);
    }
    _j.at("Y_Parameter_Amplitude").get_to(_c.axisParameters_[Y].amplitude);
    _j.at("Y_Parameter_Frequency").get_to(_c.axisParameters_[Y].frequency);
    if (_j.contains("Y_Parameter_DampingRatio")) {
        _j.at("Y_Parameter_DampingRatio").get_to(_c.axisParameters_[Y].dampingRatio);
    }
    _j.at("Z_Parameter_Amplitude").get_to(_c.axisParameters_[Z].amplitude);
    _j.at("Z_Parameter_Frequency").get_to(_c.axisParameters_[Z].frequency);
    if (_j.contains("Z_Parameter_DampingRatio")) {
        _j.at("Z_Parameter_DampingRatio").get_to(_c.axisParameters_[Z].dampingRatio);
    }
}
