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
    MonoDomain* domain = monoEngine.Domain();
    
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
                    MonoString* ms = mono_string_new(domain, arg.c_str());
                    mono_field_set_value(obj, field, ms);
                } else if constexpr (std::is_same_v<T, std::shared_ptr<ONEngine::Variables::GenericObject>>) {
                    MonoObject* structObj = mono_field_get_value_object(domain, field, obj);
                    if (structObj && arg) {
                        MonoClass* structClass = mono_object_get_class(structObj);
                        ONEngine::Variables::VarToMonoObject(structObj, structClass, arg);
                        if (mono_class_is_valuetype(structClass)) {
                            void* unboxed = mono_object_unbox(structObj);
                            mono_field_set_value(obj, field, unboxed);
                        }
                    }
                } else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<std::string>> || std::is_same_v<T, std::vector<ONEngine::Vector3>>) {
                    MonoObject* list = mono_field_get_value_object(domain, field, obj);
                    if (list) {
                        MonoClass* lc = mono_object_get_class(list);
                        MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
                        if (clear) mono_runtime_invoke(clear, list, nullptr, nullptr);
                        MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
                        if (add) {
                            MonoType* elemType = (MonoType*)ONEngine::Variables::GetListElementType(lc);
                            MonoClass* ek = elemType ? mono_class_from_mono_type(elemType) : nullptr;
                            MonoType* baseType = ek && mono_class_is_enum(ek) ? mono_class_enum_basetype(ek) : nullptr;
                            int baseTypeId = baseType ? mono_type_get_type(baseType) : -1;

                            for (auto itemVal : arg) {
                                if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                                    MonoString* s = mono_string_new(domain, itemVal.c_str());
                                    void* args[1] = { s };
                                    mono_runtime_invoke(add, list, args, nullptr);
                                } else if constexpr (std::is_same_v<T, std::vector<int>>) {
                                    int intVal = 0;
                                    if constexpr (std::is_same_v<T, std::vector<int>>) {
                                        intVal = itemVal;
                                    }
                                    if (baseTypeId == MONO_TYPE_I1 || baseTypeId == MONO_TYPE_U1) {
                                        int8_t temp = (int8_t)intVal;
                                        void* args[1] = { &temp };
                                        mono_runtime_invoke(add, list, args, nullptr);
                                    } else if (baseTypeId == MONO_TYPE_I2 || baseTypeId == MONO_TYPE_U2) {
                                        int16_t temp = (int16_t)intVal;
                                        void* args[1] = { &temp };
                                        mono_runtime_invoke(add, list, args, nullptr);
                                    } else {
                                        int32_t temp = (int32_t)intVal;
                                        void* args[1] = { &temp };
                                        mono_runtime_invoke(add, list, args, nullptr);
                                    }
                                } else {
                                    void* args[1] = { (void*)&itemVal };
                                    mono_runtime_invoke(add, list, args, nullptr);
                                }
                            }
                        }
                    }
                } else if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>>) {
                    MonoObject* list = mono_field_get_value_object(domain, field, obj);
                    if (list) {
                        MonoClass* lc = mono_object_get_class(list);
                        MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
                        if (clear) mono_runtime_invoke(clear, list, nullptr, nullptr);
                        MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
                        if (add) {
                            MonoType* et = mono_signature_get_return_type(mono_method_signature(mono_class_get_method_from_name(lc, "get_Item", 1)));
                            MonoClass* ek = mono_class_from_mono_type(et);
                            for (auto& itemGen : arg) {
                                MonoObject* item = mono_object_new(domain, ek);
                                if (!mono_class_is_valuetype(ek)) {
                                    mono_runtime_object_init(item);
                                }
                                ONEngine::Variables::VarToMonoObject(item, ek, itemGen);
                                void* args[1] = { item };
                                mono_runtime_invoke(add, list, args, nullptr);
                            }
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
