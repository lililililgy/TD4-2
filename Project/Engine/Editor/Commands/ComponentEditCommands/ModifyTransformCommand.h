#pragma once

/// std
#include <vector>

/// engine
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "../IEditCommand.h"

namespace Editor {

/// ///////////////////////////////////////////////
/// Transformの値を変更するコマンド
/// ///////////////////////////////////////////////
class ModifyTransformCommand : public IEditCommand {
public:
    enum class Target { Position, Rotation, Scale };

    struct Data {
        ONEngine::Transform* pTransform;
        ONEngine::Vector3 oldVal;
        ONEngine::Vector3 newVal;
    };

    ModifyTransformCommand(Target target, const std::vector<Data>& data)
        : target_(target), data_(data) {}

    EDITOR_STATE Execute() override {
        if (data_.empty()) return EDITOR_STATE_FAILED;
        for (const auto& d : data_) {
            ApplyValue(d.pTransform, d.newVal);
        }
        return EDITOR_STATE_FINISH;
    }

    EDITOR_STATE Undo() override {
        if (data_.empty()) return EDITOR_STATE_FAILED;
        for (const auto& d : data_) {
            ApplyValue(d.pTransform, d.oldVal);
        }
        return EDITOR_STATE_FINISH;
    }

private:
    void ApplyValue(ONEngine::Transform* pTransform, const ONEngine::Vector3& val) {
        if (!pTransform) return;

        switch (target_) {
        case Target::Position: 
            pTransform->position = val; 
            break;
        case Target::Rotation: 
            // 度数法(Degrees) -> 弧度法(Euler Radian) -> Quaternion の順で変換
            pTransform->euler = val; 
            pTransform->SyncQuaternionFromEuler();
            break;
        case Target::Scale:    
            pTransform->scale = val;    
            break;
        }
        pTransform->Update();
    }

    Target target_;
    std::vector<Data> data_;
};

} /// namespace Editor
