#include "ImGuiShowField.h"

/// external
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/class.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/loader.h>

/// std
#include <vector>
#include <format>

/// engine
#include "Engine/Core/Utility/Utility.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Variables/Variables.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Script/MonoScriptEngine.h"

/// editor
#include "ImGuiMath.h"
#include "../Commands/ImGuiCommand/ImGuiCommand.h"
#include "../Commands/ComponentEditCommands/ModifyScriptVariableCommand.h"
#include "../Manager/EditorManager.h"
#include "../Manager/EditCommand.h"

using namespace Editor;

namespace {
bool HasSerializeField(MonoClassField* field) {
	MonoClass* klass = mono_field_get_parent(field);
	MonoCustomAttrInfo* attrs = mono_custom_attrs_from_field(klass, field);
	if (!attrs) return false;

	MonoClass* serializeFieldAttr = mono_class_from_name(ONEngine::MonoScriptEngine::GetInstance().Image(), "", "SerializeField");

	bool has = serializeFieldAttr && mono_custom_attrs_has_attr(attrs, serializeFieldAttr);
	mono_custom_attrs_free(attrs);
	return has;
}

bool IsPublicField(MonoClassField* field) {
	uint32_t flags = mono_field_get_flags(field);
	return (flags & 0x0006) == 0x0006;
}

bool ShouldSerialize(MonoClassField* field) {
	return IsPublicField(field) || HasSerializeField(field);
}

std::unordered_map<int, std::unique_ptr<CSGui::ImGuiShowField>> gFieldDrawers;

void RegisterFieldDrawers() {
	gFieldDrawers[MONO_TYPE_I4] = std::make_unique<CSGui::IntField>();
	gFieldDrawers[MONO_TYPE_R4] = std::make_unique<CSGui::FloatField>();
	gFieldDrawers[MONO_TYPE_R8] = std::make_unique<CSGui::DoubleField>();
	gFieldDrawers[MONO_TYPE_BOOLEAN] = std::make_unique<CSGui::BoolField>();
	gFieldDrawers[MONO_TYPE_STRING] = std::make_unique<CSGui::StringField>();
	gFieldDrawers[MONO_TYPE_VALUETYPE] = std::make_unique<CSGui::StructGui>();
	gFieldDrawers[MONO_TYPE_ENUM] = std::make_unique<CSGui::EnumField>();
	gFieldDrawers[MONO_TYPE_GENERICINST] = std::make_unique<CSGui::ListField>();
}

bool DrawEnumCombo(MonoClass* enumClass, const char* name, int& value) {
	void* iter = nullptr; MonoClassField* enumField; std::vector<std::string> names; std::vector<int> values; int currentIndex = -1, i = 0;
	MonoVTable* vtable = mono_class_vtable(mono_domain_get(), enumClass);
	while ((enumField = mono_class_get_fields(enumClass, &iter))) {
		if (mono_field_get_flags(enumField) & MONO_FIELD_ATTR_STATIC) {
			names.push_back(mono_field_get_name(enumField));
			int val = 0; if (vtable) mono_field_static_get_value(vtable, enumField, &val); else mono_field_get_value(nullptr, enumField, &val);
			values.push_back(val); if (val == value) currentIndex = i; i++;
		}
	}
	if (names.empty()) return false;
	if (currentIndex == -1) currentIndex = 0;
	std::vector<const char*> namePtrs; for (const auto& str : names) namePtrs.push_back(str.c_str());
	if (ImGui::Combo(name, &currentIndex, namePtrs.data(), (int)namePtrs.size())) {
		value = values[currentIndex];
		return true;
	}
	return false;
}

void DrawGenericObject(std::shared_ptr<ONEngine::Variables::GenericObject> obj) {
	if (!obj) return;
	for (auto& [name, val] : obj->fields) {
		ImGui::PushID(name.c_str());
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int>) ImGui::DragInt(name.c_str(), &arg);
			else if constexpr (std::is_same_v<T, float>) ImGui::DragFloat(name.c_str(), &arg);
			else if constexpr (std::is_same_v<T, bool>) ImGui::Checkbox(name.c_str(), &arg);
			else if constexpr (std::is_same_v<T, std::string>) ImGuiInputText(name.c_str(), &arg);
			else if constexpr (std::is_same_v<T, ONEngine::Vector2>) ImGui::DragFloat2(name.c_str(), &arg.x);
			else if constexpr (std::is_same_v<T, ONEngine::Vector3>) ImGui::DragFloat3(name.c_str(), &arg.x);
			else if constexpr (std::is_same_v<T, ONEngine::Vector4>) ImGui::DragFloat4(name.c_str(), &arg.x);
			else if constexpr (std::is_same_v<T, std::shared_ptr<ONEngine::Variables::GenericObject>>) {
				if (ImGui::CollapsingHeader(name.c_str())) {
					ImGui::Indent();
					DrawGenericObject(arg);
					ImGui::Unindent();
				}
			}
			}, val);
		ImGui::PopID();
	}
}

void DrawGenericObjectWithTracking(std::shared_ptr<ONEngine::Variables::GenericObject> obj, bool& anyActive, bool& anyDeactivated) {
	if (!obj) return;
	for (auto& [name, val] : obj->fields) {
		ImGui::PushID(name.c_str());
		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int>) {
				ImGui::DragInt(name.c_str(), &arg);
				if (ImGui::IsItemActivated()) anyActive = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
			}
			else if constexpr (std::is_same_v<T, float>) {
				ImGui::DragFloat(name.c_str(), &arg);
				if (ImGui::IsItemActivated()) anyActive = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
			}
			else if constexpr (std::is_same_v<T, bool>) {
				if (ImGui::Checkbox(name.c_str(), &arg)) {
					anyActive = true;
					anyDeactivated = true;
				}
			}
			else if constexpr (std::is_same_v<T, std::string>) {
				if (ImGuiInputText(name.c_str(), &arg)) {
					anyActive = true;
					anyDeactivated = true;
				}
			}
			else if constexpr (std::is_same_v<T, ONEngine::Vector2>) {
				ImGui::DragFloat2(name.c_str(), &arg.x);
				if (ImGui::IsItemActivated()) anyActive = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
			}
			else if constexpr (std::is_same_v<T, ONEngine::Vector3>) {
				ImGui::DragFloat3(name.c_str(), &arg.x);
				if (ImGui::IsItemActivated()) anyActive = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
			}
			else if constexpr (std::is_same_v<T, ONEngine::Vector4>) {
				ImGui::DragFloat4(name.c_str(), &arg.x);
				if (ImGui::IsItemActivated()) anyActive = true;
				if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
			}
			else if constexpr (std::is_same_v<T, std::shared_ptr<ONEngine::Variables::GenericObject>>) {
				if (ImGui::CollapsingHeader(name.c_str())) {
					ImGui::Indent();
					DrawGenericObjectWithTracking(arg, anyActive, anyDeactivated);
					ImGui::Unindent();
				}
			}
			}, val);
		ImGui::PopID();
	}
}

} /// namespace


void CSGui::ShowField(const std::string& scriptName, int type, MonoObject* obj, MonoClassField* field, const char* name) {
	if(gFieldDrawers.empty()) RegisterFieldDrawers();

	if (type == MONO_TYPE_VALUETYPE || type == MONO_TYPE_CLASS) {
		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldClass = mono_class_from_mono_type(fieldType);
		if (fieldClass) {
			const char* className = mono_class_get_name(fieldClass);
			if (strcmp(className, "Vector2") != 0 && strcmp(className, "Vector3") != 0 && strcmp(className, "Vector4") != 0 && !mono_class_is_enum(fieldClass)) {
				if (gFieldDrawers.find(MONO_TYPE_VALUETYPE) != gFieldDrawers.end()) {
					gFieldDrawers[MONO_TYPE_VALUETYPE]->Draw(scriptName, obj, field, name);
				}
				return;
			}
		}
	}

	if(gFieldDrawers.find(type) == gFieldDrawers.end()) return;
	gFieldDrawers[type]->Draw(scriptName, obj, field, name);
}


void CSGui::ShowFieldForVariables(ONEngine::Variables* vars, const std::string& groupName, int type, MonoClassField* field, const char* name) {
	if(!vars) return;
	if(!vars->HasGroup(groupName)) vars->AddGroup(groupName);
	auto& group = const_cast<ONEngine::Variables::Group&>(vars->GetGroup(groupName));

	switch(type) {
	case MONO_TYPE_I4:
	case MONO_TYPE_ENUM:
	{
		if(!group.Has(name) || !std::holds_alternative<int>(group.Get(name))) {
			int defaultValue = 0;
			if (group.Has(name)) {
				auto& var = group.Get(name);
				if (std::holds_alternative<float>(var)) defaultValue = static_cast<int>(std::get<float>(var));
			}
			group.Add(name, defaultValue);
		}
		int value = group.Get<int>(name);
		if(type == MONO_TYPE_ENUM) {
			MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
			int oldValue = value;
			if (DrawEnumCombo(fieldClass, name, value)) {
				ONEngine::GameEntity* entity = vars->GetOwner();
				if (entity && oldValue != value) {
					EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_ENUM, oldValue, value);
				} else {
					group.Add(name, value);
				}
			}
			break;
		}
		static int startIntValue = 0;
		if (ImGui::IsItemActivated()) {
			startIntValue = value;
		}
		if(ImGui::DragInt(name, &value)) {
			group.Add(name, value);
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			ONEngine::GameEntity* entity = vars->GetOwner();
			if (entity && startIntValue != value) {
				EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_I4, startIntValue, value);
			}
		}
		break;
	}
	case MONO_TYPE_R4:
	{
		if(!group.Has(name) || !std::holds_alternative<float>(group.Get(name))) {
			float defaultValue = 0.0f;
			if (group.Has(name)) {
				auto& var = group.Get(name);
				if (std::holds_alternative<int>(var)) defaultValue = static_cast<float>(std::get<int>(var));
			}
			group.Add(name, defaultValue);
		}
		float value = group.Get<float>(name);
		static float startFloatValue = 0.0f;
		if (ImGui::IsItemActivated()) {
			startFloatValue = value;
		}
		if(ImGui::DragFloat(name, &value)) {
			group.Add(name, value);
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			ONEngine::GameEntity* entity = vars->GetOwner();
			if (entity && startFloatValue != value) {
				EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_R4, startFloatValue, value);
			}
		}
		break;
	}
	case MONO_TYPE_BOOLEAN:
	{
		if(!group.Has(name) || !std::holds_alternative<bool>(group.Get(name))) {
			bool defaultValue = false;
			if (group.Has(name)) {
				auto& var = group.Get(name);
				if (std::holds_alternative<int>(var)) defaultValue = (std::get<int>(var) != 0);
				else if (std::holds_alternative<float>(var)) defaultValue = (std::get<float>(var) != 0.0f);
			}
			group.Add(name, defaultValue);
		}
		bool value = group.Get<bool>(name);
		bool oldValue = value;
		if(ImGui::Checkbox(name, &value)) {
			ONEngine::GameEntity* entity = vars->GetOwner();
			if (entity && oldValue != value) {
				EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_BOOLEAN, oldValue, value);
			} else {
				group.Add(name, value);
			}
		}
		break;
	}
	case MONO_TYPE_STRING:
	{
		if(!group.Has(name) || !std::holds_alternative<std::string>(group.Get(name))) {
			group.Add(name, std::string(""));
		}
		std::string value = group.Get<std::string>(name);
		static std::string startStrValue = "";
		if (ImGui::IsItemActivated()) {
			startStrValue = value;
		}
		if(ImGuiInputText(name, &value)) {
			group.Add(name, value);
		}
		if (ImGui::IsItemDeactivatedAfterEdit()) {
			ONEngine::GameEntity* entity = vars->GetOwner();
			if (entity && startStrValue != value) {
				EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_STRING, startStrValue, value);
			}
		}
		break;
	}
	case MONO_TYPE_VALUETYPE:
	{
		MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
		const char* className = mono_class_get_name(fieldClass);
		if(strcmp(className, "Vector2") == 0) {
			if(!group.Has(name) || !std::holds_alternative<ONEngine::Vector2>(group.Get(name))) {
				group.Add(name, ONEngine::Vector2::Zero);
			}
			ONEngine::Vector2 value = group.Get<ONEngine::Vector2>(name);
			static ONEngine::Vector2 startVec2Value = ONEngine::Vector2::Zero;
			if (ImGui::IsItemActivated()) {
				startVec2Value = value;
			}
			if(ImGui::DragFloat2(name, &value.x)) {
				group.Add(name, value);
			}
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				ONEngine::GameEntity* entity = vars->GetOwner();
				if (entity && (startVec2Value.x != value.x || startVec2Value.y != value.y)) {
					EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_VALUETYPE, startVec2Value, value);
				}
			}
		} else if(strcmp(className, "Vector3") == 0) {
			if(!group.Has(name) || !std::holds_alternative<ONEngine::Vector3>(group.Get(name))) {
				group.Add(name, ONEngine::Vector3::Zero);
			}
			ONEngine::Vector3 value = group.Get<ONEngine::Vector3>(name);
			static ONEngine::Vector3 startVec3Value = ONEngine::Vector3::Zero;
			if (ImGui::IsItemActivated()) {
				startVec3Value = value;
			}
			if(ImGui::DragFloat3(name, &value.x)) {
				group.Add(name, value);
			}
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				ONEngine::GameEntity* entity = vars->GetOwner();
				if (entity && (startVec3Value.x != value.x || startVec3Value.y != value.y || startVec3Value.z != value.z)) {
					EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_VALUETYPE, startVec3Value, value);
				}
			}
		} else if(strcmp(className, "Vector4") == 0) {
			if(!group.Has(name) || !std::holds_alternative<ONEngine::Vector4>(group.Get(name))) {
				group.Add(name, ONEngine::Vector4::Zero);
			}
			ONEngine::Vector4 value = group.Get<ONEngine::Vector4>(name);
			static ONEngine::Vector4 startVec4Value = ONEngine::Vector4::Zero;
			if (ImGui::IsItemActivated()) {
				startVec4Value = value;
			}
			if(ImGui::DragFloat4(name, &value.x)) {
				group.Add(name, value);
			}
			if (ImGui::IsItemDeactivatedAfterEdit()) {
				ONEngine::GameEntity* entity = vars->GetOwner();
				if (entity && (startVec4Value.x != value.x || startVec4Value.y != value.y || startVec4Value.z != value.z || startVec4Value.w != value.w)) {
					EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_VALUETYPE, startVec4Value, value);
				}
			}
		} else if(!mono_class_is_enum(fieldClass)) {
			// カスタム構造体
			if (!group.Has(name) || !std::holds_alternative<std::shared_ptr<ONEngine::Variables::GenericObject>>(group.Get(name))) {
				auto elemVar = ONEngine::Variables::MonoObjectToVar(nullptr, mono_field_get_type(field));
				if (std::holds_alternative<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar)) {
					group.Add(name, std::get<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar));
				} else {
					auto gen = std::make_shared<ONEngine::Variables::GenericObject>();
					gen->typeName = className;
					group.Add(name, gen);
				}
			}
			auto& gen = std::get<std::shared_ptr<ONEngine::Variables::GenericObject>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
			if (ImGui::CollapsingHeader(name)) {
				ImGui::Indent();
				DrawGenericObject(gen);
				ImGui::Unindent();
			}
		}
		break;
	}
	case MONO_TYPE_GENERICINST:
	{
		ImGui::PushID(name);
		MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
		if (strcmp(mono_class_get_name(fieldClass), "List`1") != 0) { ImGui::PopID(); break; }
		MonoMethod* getItemMethod = mono_class_get_method_from_name(fieldClass, "get_Item", 1);
		MonoType* elemType = mono_signature_get_return_type(mono_method_signature(getItemMethod));
		int elemTypeId = mono_type_get_type(elemType);

		std::string key = std::format("{}_{}_{}", (void*)vars, groupName, name);
		static std::unordered_map<std::string, ONEngine::Variables::Var> startListValues;

		if (ImGui::CollapsingHeader(name)) {
			ImGui::Indent();
			if (elemTypeId == MONO_TYPE_I4) {
				if (!group.Has(name) || !std::holds_alternative<std::vector<int>>(group.Get(name))) group.Add(name, std::vector<int>());
				auto& list = std::get<std::vector<int>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size();
				if (ImGui::InputInt("Size", &size)) {
					if (size < 0) size = 0;
					ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
					list.resize(size);
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
					}
				}
				bool anyActive = false;
				bool anyDeactivated = false;
				bool changed = false;
				for (int i = 0; i < (int)list.size(); ++i) {
					ImGui::PushID(i);
					if (ImGui::DragInt(std::format("[{}]", i).c_str(), &list[i])) changed = true;
					if (ImGui::IsItemActivated()) anyActive = true;
					if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
					ImGui::PopID();
				}
				if (changed) vars->SetScriptVariables(groupName);
				if (anyActive) startListValues[key] = ONEngine::Variables::CloneVar(group.Get(name));
				if (anyDeactivated) {
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						if (startListValues.contains(key)) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, startListValues[key], group.Get(name));
							startListValues.erase(key);
						}
					}
				}
			} else if (elemTypeId == MONO_TYPE_R4) {
				if (!group.Has(name) || !std::holds_alternative<std::vector<float>>(group.Get(name))) group.Add(name, std::vector<float>());
				auto& list = std::get<std::vector<float>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size();
				if (ImGui::InputInt("Size", &size)) {
					if (size < 0) size = 0;
					ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
					list.resize(size);
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
					}
				}
				bool anyActive = false;
				bool anyDeactivated = false;
				bool changed = false;
				for (int i = 0; i < (int)list.size(); ++i) {
					ImGui::PushID(i);
					if (ImGui::DragFloat(std::format("[{}]", i).c_str(), &list[i])) changed = true;
					if (ImGui::IsItemActivated()) anyActive = true;
					if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
					ImGui::PopID();
				}
				if (changed) vars->SetScriptVariables(groupName);
				if (anyActive) startListValues[key] = ONEngine::Variables::CloneVar(group.Get(name));
				if (anyDeactivated) {
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						if (startListValues.contains(key)) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, startListValues[key], group.Get(name));
							startListValues.erase(key);
						}
					}
				}
			} else if (elemTypeId == MONO_TYPE_BOOLEAN) {
				if (!group.Has(name) || !std::holds_alternative<std::vector<bool>>(group.Get(name))) group.Add(name, std::vector<bool>());
				auto& list = std::get<std::vector<bool>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size();
				if (ImGui::InputInt("Size", &size)) {
					if (size < 0) size = 0;
					ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
					list.resize(size);
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
					}
				}
				for (int i = 0; i < (int)list.size(); ++i) {
					ImGui::PushID(i);
					bool b = list[i];
					if (ImGui::Checkbox(std::format("[{}]", i).c_str(), &b)) {
						ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
						list[i] = b;
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
						} else {
							vars->SetScriptVariables(groupName);
						}
					}
					ImGui::PopID();
				}
			} else if (elemTypeId == MONO_TYPE_STRING) {
				if (!group.Has(name) || !std::holds_alternative<std::vector<std::string>>(group.Get(name))) group.Add(name, std::vector<std::string>());
				auto& list = std::get<std::vector<std::string>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size();
				if (ImGui::InputInt("Size", &size)) {
					if (size < 0) size = 0;
					ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
					list.resize(size);
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
					}
				}
				bool anyActive = false;
				bool anyDeactivated = false;
				bool changed = false;
				for (int i = 0; i < (int)list.size(); ++i) {
					ImGui::PushID(i);
					if (ImGuiInputText(std::format("[{}]", i).c_str(), &list[i])) changed = true;
					if (ImGui::IsItemActivated()) anyActive = true;
					if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
					ImGui::PopID();
				}
				if (changed) vars->SetScriptVariables(groupName);
				if (anyActive) startListValues[key] = ONEngine::Variables::CloneVar(group.Get(name));
				if (anyDeactivated) {
					if (ONEngine::GameEntity* entity = vars->GetOwner()) {
						if (startListValues.contains(key)) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, startListValues[key], group.Get(name));
							startListValues.erase(key);
						}
					}
				}
			} else if (elemTypeId == MONO_TYPE_VALUETYPE || elemTypeId == MONO_TYPE_CLASS) {
				MonoClass* elemClass = mono_class_from_mono_type(elemType);
				if (strcmp(mono_class_get_name(elemClass), "Vector3") == 0) {
					if (!group.Has(name) || !std::holds_alternative<std::vector<ONEngine::Vector3>>(group.Get(name))) group.Add(name, std::vector<ONEngine::Vector3>());
					auto& list = std::get<std::vector<ONEngine::Vector3>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size();
					if (ImGui::InputInt("Size", &size)) {
						if (size < 0) size = 0;
						ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
						list.resize(size);
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
						}
					}
					bool anyActive = false;
					bool anyDeactivated = false;
					bool changed = false;
					for (int i = 0; i < (int)list.size(); ++i) {
						ImGui::PushID(i);
						if (ImGui::DragFloat3(std::format("[{}]", i).c_str(), &list[i].x)) changed = true;
						if (ImGui::IsItemActivated()) anyActive = true;
						if (ImGui::IsItemDeactivatedAfterEdit()) anyDeactivated = true;
						ImGui::PopID();
					}
					if (changed) vars->SetScriptVariables(groupName);
					if (anyActive) startListValues[key] = ONEngine::Variables::CloneVar(group.Get(name));
					if (anyDeactivated) {
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							if (startListValues.contains(key)) {
								EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, startListValues[key], group.Get(name));
								startListValues.erase(key);
							}
						}
					}
				} else if (mono_class_is_enum(elemClass)) {
					if (!group.Has(name) || !std::holds_alternative<std::vector<int>>(group.Get(name))) group.Add(name, std::vector<int>());
					auto& list = std::get<std::vector<int>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size();
					if (ImGui::InputInt("Size", &size)) {
						if (size < 0) size = 0;
						ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
						list.resize(size);
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
						}
					}
					for (int i = 0; i < (int)list.size(); ++i) {
						ImGui::PushID(i);
						int val = list[i];
						if (DrawEnumCombo(elemClass, std::format("[{}]", i).c_str(), val)) {
							ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
							list[i] = val;
							if (ONEngine::GameEntity* entity = vars->GetOwner()) {
								EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
							} else {
								vars->SetScriptVariables(groupName);
							}
						}
						ImGui::PopID();
					}
				} else {
					if (!group.Has(name) || !std::holds_alternative<std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>>(group.Get(name)))
						group.Add(name, std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>());
					auto& list = std::get<std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size();
					if (ImGui::InputInt("Size", &size)) {
						if (size < 0) size = 0;
						ONEngine::Variables::Var oldVal = ONEngine::Variables::CloneVar(group.Get(name));
						size_t oldSize = list.size(); list.resize(size);
						for (size_t i = oldSize; i < list.size(); ++i) {
							auto elemVar = ONEngine::Variables::MonoObjectToVar(nullptr, elemType);
							if (std::holds_alternative<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar) && std::get<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar) != nullptr) {
								list[i] = std::get<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar);
							} else {
								list[i] = std::make_shared<ONEngine::Variables::GenericObject>();
								list[i]->typeName = mono_class_get_name(elemClass);
							}
						}
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, oldVal, group.Get(name));
						}
					}
					bool anyActive = false;
					bool anyDeactivated = false;
					bool changed = false;
					for (int i = 0; i < (int)list.size(); ++i) {
						if (list[i]) {
							if (list[i]->typeName.empty()) {
								list[i]->typeName = mono_class_get_name(elemClass);
							}
							void* iter = nullptr;
							MonoClassField* f = nullptr;
							while ((f = mono_class_get_fields(elemClass, &iter))) {
								if (ShouldSerialize(f)) {
									std::string fieldName = mono_field_get_name(f);
									if (!list[i]->fields.contains(fieldName)) {
										list[i]->fields[fieldName] = ONEngine::Variables::MonoObjectToVar(nullptr, mono_field_get_type(f));
									}
								}
							}
						}

						ImGui::PushID(i);
						if (ImGui::CollapsingHeader(std::format("[{}]", i).c_str())) {
							ImGui::Indent();
							
							auto tempGeneric = ONEngine::Variables::CloneGenericObject(list[i]);
							bool anyItemActive = false;
							bool anyItemDeactivatedAfterEdit = false;
							
							DrawGenericObjectWithTracking(tempGeneric, anyItemActive, anyItemDeactivatedAfterEdit);
							
							if (anyItemActive) anyActive = true;
							if (anyItemDeactivatedAfterEdit) anyDeactivated = true;
							
							if (tempGeneric && !ONEngine::Variables::IsEqualGenericObject(list[i], tempGeneric)) {
								list[i] = tempGeneric;
								changed = true;
							}
							
							ImGui::Unindent();
						}
						ImGui::PopID();
					}
					if (changed) vars->SetScriptVariables(groupName);
					if (anyActive) startListValues[key] = ONEngine::Variables::CloneVar(group.Get(name));
					if (anyDeactivated) {
						if (ONEngine::GameEntity* entity = vars->GetOwner()) {
							if (startListValues.contains(key)) {
								EditCommand::Execute<ModifyScriptVariableCommand>(entity, groupName, name, MONO_TYPE_GENERICINST, startListValues[key], group.Get(name));
								startListValues.erase(key);
							}
						}
					}
				}
			}
			ImGui::Unindent();
		}
		ImGui::PopID();
		break;
	}
	}
}


void CSGui::IntField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	int value = 0; mono_field_get_value(obj, field, &value);
	static int startValue = 0; if (ImGui::IsItemActivated()) startValue = value;
	if(ImGui::DragInt(name, &value)) mono_field_set_value(obj, field, &value);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity && startValue != value) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_I4, startValue, value);
	}
}

void CSGui::FloatField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	float value = 0.0f; mono_field_get_value(obj, field, &value);
	static float startValue = 0.0f; if (ImGui::IsItemActivated()) startValue = value;
	if(ImGui::DragFloat(name, &value)) mono_field_set_value(obj, field, &value);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity && startValue != value) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_R4, startValue, value);
	}
}

void CSGui::DoubleField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	double value = 0.0; mono_field_get_value(obj, field, &value); double oldValue = value;
	float floatValue = static_cast<float>(value);
	if(ImGui::DragFloat(name, &floatValue)) {
		value = static_cast<double>(floatValue);
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_R8, oldValue, value);
		else mono_field_set_value(obj, field, &value);
	}
}

void CSGui::BoolField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	bool value = false; mono_field_get_value(obj, field, &value); bool oldValue = value;
	if(ImGui::Checkbox(name, &value)) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_BOOLEAN, oldValue, value);
		else mono_field_set_value(obj, field, &value);
	}
}

void CSGui::StringField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	MonoString* monoStr = (MonoString*)mono_field_get_value_object(mono_domain_get(), field, obj);
	if(!monoStr) return;
	char* utf8 = mono_string_to_utf8(monoStr); std::string oldValue = utf8; std::string value = utf8; mono_free(utf8);
	if(ImGuiInputText(name, &value, ImGuiInputTextFlags_EnterReturnsTrue)) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_STRING, oldValue, value);
		else mono_field_set_value(obj, field, mono_string_new(mono_domain_get(), value.c_str()));
	}
}

void CSGui::ListField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	MonoDomain* domain = mono_domain_get();
	MonoObject* listObj = mono_field_get_value_object(domain, field, obj);
	if(!listObj) { ImGui::Text("%s: (null)", name); return; }
	ImGui::PushID(field);
	MonoClass* listClass = mono_object_get_class(listObj);
	MonoMethod* getCountMethod = mono_class_get_method_from_name(listClass, "get_Count", 0);
	int count = *(int*)mono_object_unbox(mono_runtime_invoke(getCountMethod, listObj, nullptr, nullptr));
	if(ImGui::CollapsingHeader(name)) {
		ImGui::Indent();
		MonoMethod* getItemMethod = mono_class_get_method_from_name(listClass, "get_Item", 1);
		MonoMethod* setItemMethod = mono_class_get_method_from_name(listClass, "set_Item", 2);
		MonoMethod* addMethod = mono_class_get_method_from_name(listClass, "Add", 1);
		MonoMethod* removeAtMethod = mono_class_get_method_from_name(listClass, "RemoveAt", 1);
		MonoType* elemType = mono_signature_get_return_type(mono_method_signature(getItemMethod));
		int elemTypeId = mono_type_get_type(elemType);
		int size = count;
		if(ImGui::InputInt("Size", &size)) {
			if(size < 0) size = 0;
			if(size > count) {
				for(int i = 0; i < size - count; ++i) {
					if(elemTypeId == MONO_TYPE_I4) { int v = 0; void* args[1] = { &v }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
					else if(elemTypeId == MONO_TYPE_R4) { float v = 0.0f; void* args[1] = { &v }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
					else if(elemTypeId == MONO_TYPE_BOOLEAN) { bool v = false; void* args[1] = { &v }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
					else if(elemTypeId == MONO_TYPE_STRING) { void* args[1] = { mono_string_new(domain, "") }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
					else if(elemTypeId == MONO_TYPE_VALUETYPE || elemTypeId == MONO_TYPE_CLASS) {
						MonoClass* ek = mono_class_from_mono_type(elemType);
						if(strcmp(mono_class_get_name(ek), "Vector3") == 0) { ONEngine::Vector3 v = ONEngine::Vector3::Zero; void* args[1] = { &v }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
						else if(mono_class_is_enum(ek)) { int v = 0; void* args[1] = { &v }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
						else {
							MonoObject* item = mono_object_new(domain, ek);
							if (!mono_class_is_valuetype(ek)) {
								mono_runtime_object_init(item);
							}
							void* args[1] = { item };
							mono_runtime_invoke(addMethod, listObj, args, nullptr);
						}
					}
				}
			} else if(size < count) {
				for(int i = 0; i < count - size; ++i) { int idx = size; void* args[1] = { &idx }; mono_runtime_invoke(removeAtMethod, listObj, args, nullptr); }
			}
			count = size;
		}
		for(int i = 0; i < count; ++i) {
			ImGui::PushID(i);
			void* getArgs[1] = { &i }; MonoObject* itemObj = mono_runtime_invoke(getItemMethod, listObj, getArgs, nullptr);
			std::string itemName = std::format("[{}]", i);
			if(elemTypeId == MONO_TYPE_I4) { int v = *(int*)mono_object_unbox(itemObj); if(ImGui::DragInt(itemName.c_str(), &v)) { void* setArgs[2] = { &i, &v }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
			else if(elemTypeId == MONO_TYPE_R4) { float v = *(float*)mono_object_unbox(itemObj); if(ImGui::DragFloat(itemName.c_str(), &v)) { void* setArgs[2] = { &i, &v }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
			else if(elemTypeId == MONO_TYPE_BOOLEAN) { bool v = *(bool*)mono_object_unbox(itemObj); if(ImGui::Checkbox(itemName.c_str(), &v)) { void* setArgs[2] = { &i, &v }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
			else if(elemTypeId == MONO_TYPE_STRING) { char* u = mono_string_to_utf8((MonoString*)itemObj); std::string v = u; mono_free(u); if(ImGuiInputText(itemName.c_str(), &v)) { void* setArgs[2] = { &i, mono_string_new(domain, v.c_str()) }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
			else if(elemTypeId == MONO_TYPE_VALUETYPE || elemTypeId == MONO_TYPE_CLASS) {
				MonoClass* ek = mono_class_from_mono_type(elemType);
				if(strcmp(mono_class_get_name(ek), "Vector3") == 0) { ONEngine::Vector3 v = *(ONEngine::Vector3*)mono_object_unbox(itemObj); if(ImGui::DragFloat3(itemName.c_str(), &v.x)) { void* setArgs[2] = { &i, &v }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
				else if(mono_class_is_enum(ek)) { int v = *(int*)mono_object_unbox(itemObj); if(DrawEnumCombo(ek, itemName.c_str(), v)) { void* setArgs[2] = { &i, &v }; mono_runtime_invoke(setItemMethod, listObj, setArgs, nullptr); } }
				else {
					if (ImGui::CollapsingHeader(itemName.c_str())) {
						ImGui::Indent(); void* iter = nullptr; MonoClassField* f;
						while ((f = mono_class_get_fields(ek, &iter))) ShowField(scriptName, mono_type_get_type(mono_field_get_type(f)), itemObj, f, mono_field_get_name(f));
						ImGui::Unindent();
					}
				}
			}
			ImGui::PopID();
		}
		ImGui::Unindent();
	}
	ImGui::PopID();
}

void CSGui::EnumField::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
	int currentValue = 0; mono_field_get_value(obj, field, &currentValue); int oldValue = currentValue;
	if (DrawEnumCombo(fieldClass, name, currentValue)) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_ENUM, oldValue, currentValue);
		else mono_field_set_value(obj, field, &currentValue);
	}
}

void CSGui::StructGui::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
	if(mono_class_is_enum(fieldClass)) { static EnumField enumDrawer; enumDrawer.Draw(scriptName, obj, field, name); return; }
	if(fieldDrawers.empty()) Register();
	
	const char* typeName = mono_class_get_name(fieldClass);
	if(fieldDrawers.find(typeName) != fieldDrawers.end()) {
		fieldDrawers[typeName]->Draw(scriptName, obj, field, name);
		return;
	}

	// --- カスタム構造体の描画とトラッキング ---
	ImGui::PushID(field);

	MonoObject* structObj = mono_field_get_value_object(mono_domain_get(), field, obj);
	if (!structObj) {
		ImGui::Text("%s: (null)", name);
		ImGui::PopID();
		return;
	}

	auto currentGeneric = ONEngine::Variables::MonoObjectToGeneric(structObj);
	if (!currentGeneric) {
		ImGui::PopID();
		return;
	}

	std::string key = std::format("{}_{}", (void*)obj, (void*)field);

	if (ImGui::CollapsingHeader(name)) {
		ImGui::Indent();

		auto tempGeneric = ONEngine::Variables::CloneGenericObject(currentGeneric);
		bool anyItemActive = false;
		bool anyItemDeactivatedAfterEdit = false;

		DrawGenericObjectWithTracking(tempGeneric, anyItemActive, anyItemDeactivatedAfterEdit);

		if (anyItemActive) {
			startValues[key] = ONEngine::Variables::CloneGenericObject(currentGeneric);
		}

		if (tempGeneric && !ONEngine::Variables::IsEqualGenericObject(currentGeneric, tempGeneric)) {
			// C# 側に即時仮反映
			ONEngine::Variables::VarToMonoObject(structObj, fieldClass, tempGeneric);
			if (mono_class_is_valuetype(fieldClass)) {
				void* unboxed = mono_object_unbox(structObj);
				mono_field_set_value(obj, field, unboxed);
			}

			// Variables コンポーネントに即時仮反映
			ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
			if (entity) {
				ONEngine::Variables* vars = entity->GetComponent<ONEngine::Variables>();
				if (vars) {
					vars->SetVariable(scriptName, name, tempGeneric);
				}
			}
		}

		if (anyItemDeactivatedAfterEdit) {
			ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
			auto startValIt = startValues.find(key);
			if (entity && startValIt != startValues.end()) {
				auto startVal = startValIt->second;
				EditCommand::Execute<ModifyScriptVariableCommand>(
					entity, scriptName, name, MONO_TYPE_VALUETYPE, startVal, tempGeneric
				);
				startValues.erase(startValIt);
			}
		}

		ImGui::Unindent();
	}

	ImGui::PopID();
}

void CSGui::StructGui::Register() {
	fieldDrawers["Vector2"] = std::make_unique<Vector2Field>();
	fieldDrawers["Vector3"] = std::make_unique<Vector3Field>();
	fieldDrawers["Vector4"] = std::make_unique<Vector4Field>();
}

void CSGui::Vector2Field::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	ONEngine::Vector2 s; mono_field_get_value(obj, field, &s); static ONEngine::Vector2 start; if (ImGui::IsItemActivated()) start = s;
	if(ImGui::DragFloat2(name, &s.x)) mono_field_set_value(obj, field, &s);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity && (start.x != s.x || start.y != s.y)) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_VALUETYPE, start, s);
	}
}

void CSGui::Vector3Field::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	ONEngine::Vector3 s; mono_field_get_value(obj, field, &s); static ONEngine::Vector3 start; if (ImGui::IsItemActivated()) start = s;
	if(ImGui::DragFloat3(name, &s.x)) mono_field_set_value(obj, field, &s);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity && (start.x != s.x || start.y != s.y || start.z != s.z)) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_VALUETYPE, start, s);
	}
}

void CSGui::Vector4Field::Draw(const std::string& scriptName, MonoObject* obj, MonoClassField* field, const char* name) {
	ONEngine::Vector4 s; mono_field_get_value(obj, field, &s); static ONEngine::Vector4 start; if (ImGui::IsItemActivated()) start = s;
	if(ImGui::DragFloat4(name, &s.x)) mono_field_set_value(obj, field, &s);
	if (ImGui::IsItemDeactivatedAfterEdit()) {
		ONEngine::GameEntity* entity = ONEngine::MonoScriptEngine::GetInstance().GetOwnerEntity(obj);
		if (entity && (start.x != s.x || start.y != s.y || start.z != s.z || start.w != s.w)) EditCommand::Execute<ModifyScriptVariableCommand>(entity, scriptName, name, MONO_TYPE_VALUETYPE, start, s);
	}
}
