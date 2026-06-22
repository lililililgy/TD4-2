#include "ModifyScriptVariableCommand.h"

/// engine
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Core/Utility/Utility.h"

/// external
#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>

namespace Editor {

ModifyScriptVariableCommand::ModifyScriptVariableCommand(ONEngine::GameEntity* entity, const std::string& scriptName, const std::string& fieldName, int monoType, const VariantValue& oldValue, const VariantValue& newValue)
    : entityGuid_(entity->GetGuid()), scriptName_(scriptName), fieldName_(fieldName), monoType_(monoType), oldValue_(oldValue), newValue_(newValue) {
}

EDITOR_STATE ModifyScriptVariableCommand::Execute() {
    ApplyValue(newValue_);
    return EDITOR_STATE_FINISH;
}

EDITOR_STATE ModifyScriptVariableCommand::Undo() {
    ApplyValue(oldValue_);
    return EDITOR_STATE_FINISH;
}

void ModifyScriptVariableCommand::ApplyValue(const VariantValue& value) {
    auto& monoEngine = ONEngine::MonoScriptEngine::GetInstance();
    
    ONEngine::GameEntity* entity = monoEngine.GetOwnerEntity(entityGuid_);
    if (!entity) {
        ONEngine::Console::LogError(std::format("[ModifyScriptVariableCommand] FAILED: Entity not found for GUID: {}", entityGuid_.ToString()));
        return;
    }

    std::string groupName = monoEngine.GetGroupNameByEntityGuid(entityGuid_);
    if (groupName.empty()) groupName = "GameScene";

    // 1. C#側のインスタンスがあれば更新を試みる
    bool appliedToMono = false;
    MonoObject* obj = monoEngine.GetMonoBehaviorFromCS(groupName, entity->GetId(), scriptName_);
    if (obj) {
        MonoClass* monoClass = mono_object_get_class(obj);
        MonoClassField* field = mono_class_get_field_from_name(monoClass, fieldName_.c_str());
        if (field) {
            std::visit([&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    MonoString* ms = mono_string_new(mono_domain_get(), arg.c_str());
                    mono_field_set_value(obj, field, ms);
                } else if constexpr (std::is_same_v<T, std::shared_ptr<ONEngine::Variables::GenericObject>>) {
                    MonoObject* structObj = mono_field_get_value_object(mono_domain_get(), field, obj);
                    if (structObj && arg) {
                        MonoClass* structClass = mono_object_get_class(structObj);
                        ONEngine::Variables::VarToMonoObject(structObj, structClass, arg);
                        if (mono_class_is_valuetype(structClass)) {
                            void* unboxed = mono_object_unbox(structObj);
                            mono_field_set_value(obj, field, unboxed);
                        }
                    }
                } else {
                    mono_field_set_value(obj, field, (void*)&arg);
                }
                appliedToMono = true;
            }, value);
        }
    }

    // 2. Variablesコンポーネント（永続化データ）を更新
    // これにより、起動前でもUndo/Redoが効き、次回の起動時に反映される
    ONEngine::Variables* vars = entity->GetComponent<ONEngine::Variables>();
    if (!vars) {
        vars = entity->AddComponent<ONEngine::Variables>();
    }

    if (vars) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            ONEngine::Variables::Var v;
            if constexpr (std::is_same_v<T, double>) {
                v = static_cast<float>(arg);
            } else {
                v = arg;
            }
            
            vars->SetVariable(scriptName_, fieldName_, v);
        }, value);
        
        if (!appliedToMono) {
            ONEngine::Console::Log(std::format("[UndoDebug] SUCCESS: Applied to Variables component for '{}' (Mono instance not ready)", fieldName_));
        }
    }

    if (appliedToMono) {
        ONEngine::Console::Log(std::format("[ModifyScriptVariableCommand] SUCCESS: Applied to Mono instance for '{}' on '{}'", fieldName_, entity->GetName()));
    }
}

} /// Editor
