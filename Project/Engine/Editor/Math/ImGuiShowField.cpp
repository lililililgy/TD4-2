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
	}
}

} /// namespace


void CSGui::ShowField(const std::string& scriptName, int type, MonoObject* obj, MonoClassField* field, const char* name) {
	if(gFieldDrawers.empty()) RegisterFieldDrawers();

	if (type == MONO_TYPE_VALUETYPE || type == MONO_TYPE_CLASS) {
		MonoType* fieldType = mono_field_get_type(field);
		MonoClass* fieldClass = mono_class_from_mono_type(fieldType);
		const char* className = mono_class_get_name(fieldClass);
		if (strcmp(className, "Vector2") != 0 && strcmp(className, "Vector3") != 0 && strcmp(className, "Vector4") != 0 && !mono_class_is_enum(fieldClass)) {
			if (ImGui::CollapsingHeader(name)) {
				ImGui::Indent();
				MonoObject* subObj = mono_field_get_value_object(mono_domain_get(), field, obj);
				if (subObj) {
					void* iter = nullptr;
					MonoClassField* subField = nullptr;
					while ((subField = mono_class_get_fields(fieldClass, &iter))) {
						ShowField(scriptName, mono_type_get_type(mono_field_get_type(subField)), subObj, subField, mono_field_get_name(subField));
					}
				}
				ImGui::Unindent();
			}
			return;
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
		if(!group.Has(name)) group.Add(name, 0);
		int value = group.Get<int>(name);
		if(type == MONO_TYPE_ENUM) {
			MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
			if (DrawEnumCombo(fieldClass, name, value)) group.Add(name, value);
			break;
		}
		if(ImGui::DragInt(name, &value)) group.Add(name, value);
		break;
	}
	case MONO_TYPE_R4:
	{
		if(!group.Has(name)) group.Add(name, 0.0f);
		float value = group.Get<float>(name);
		if(ImGui::DragFloat(name, &value)) group.Add(name, value);
		break;
	}
	case MONO_TYPE_BOOLEAN:
	{
		if(!group.Has(name)) group.Add(name, false);
		bool value = group.Get<bool>(name);
		if(ImGui::Checkbox(name, &value)) group.Add(name, value);
		break;
	}
	case MONO_TYPE_STRING:
	{
		if(!group.Has(name)) group.Add(name, std::string(""));
		std::string value = group.Get<std::string>(name);
		if(ImGuiInputText(name, &value)) group.Add(name, value);
		break;
	}
	case MONO_TYPE_VALUETYPE:
	{
		MonoClass* fieldClass = mono_class_from_mono_type(mono_field_get_type(field));
		const char* className = mono_class_get_name(fieldClass);
		if(strcmp(className, "Vector2") == 0) {
			if(!group.Has(name)) group.Add(name, ONEngine::Vector2::Zero);
			ONEngine::Vector2 value = group.Get<ONEngine::Vector2>(name);
			if(ImGui::DragFloat2(name, &value.x)) group.Add(name, value);
		} else if(strcmp(className, "Vector3") == 0) {
			if(!group.Has(name)) group.Add(name, ONEngine::Vector3::Zero);
			ONEngine::Vector3 value = group.Get<ONEngine::Vector3>(name);
			if(ImGui::DragFloat3(name, &value.x)) group.Add(name, value);
		} else if(strcmp(className, "Vector4") == 0) {
			if(!group.Has(name)) group.Add(name, ONEngine::Vector4::Zero);
			ONEngine::Vector4 value = group.Get<ONEngine::Vector4>(name);
			if(ImGui::DragFloat4(name, &value.x)) group.Add(name, value);
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
		if (ImGui::CollapsingHeader(name)) {
			ImGui::Indent();
			if (elemTypeId == MONO_TYPE_I4) {
				if (!group.Has(name)) group.Add(name, std::vector<int>());
				auto& list = std::get<std::vector<int>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
				for (int i = 0; i < (int)list.size(); ++i) ImGui::DragInt(std::format("[{}]", i).c_str(), &list[i]);
			} else if (elemTypeId == MONO_TYPE_R4) {
				if (!group.Has(name)) group.Add(name, std::vector<float>());
				auto& list = std::get<std::vector<float>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
				for (int i = 0; i < (int)list.size(); ++i) ImGui::DragFloat(std::format("[{}]", i).c_str(), &list[i]);
			} else if (elemTypeId == MONO_TYPE_BOOLEAN) {
				if (!group.Has(name)) group.Add(name, std::vector<bool>());
				auto& list = std::get<std::vector<bool>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
				for (int i = 0; i < (int)list.size(); ++i) { bool b = list[i]; if (ImGui::Checkbox(std::format("[{}]", i).c_str(), &b)) list[i] = b; }
			} else if (elemTypeId == MONO_TYPE_STRING) {
				if (!group.Has(name)) group.Add(name, std::vector<std::string>());
				auto& list = std::get<std::vector<std::string>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
				int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
				for (int i = 0; i < (int)list.size(); ++i) ImGuiInputText(std::format("[{}]", i).c_str(), &list[i]);
			} else if (elemTypeId == MONO_TYPE_VALUETYPE || elemTypeId == MONO_TYPE_CLASS) {
				MonoClass* elemClass = mono_class_from_mono_type(elemType);
				if (strcmp(mono_class_get_name(elemClass), "Vector3") == 0) {
					if (!group.Has(name)) group.Add(name, std::vector<ONEngine::Vector3>());
					auto& list = std::get<std::vector<ONEngine::Vector3>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
					for (int i = 0; i < (int)list.size(); ++i) ImGui::DragFloat3(std::format("[{}]", i).c_str(), &list[i].x);
				} else if (mono_class_is_enum(elemClass)) {
					if (!group.Has(name)) group.Add(name, std::vector<int>());
					auto& list = std::get<std::vector<int>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size(); if (ImGui::InputInt("Size", &size)) { if (size < 0) size = 0; list.resize(size); }
					for (int i = 0; i < (int)list.size(); ++i) DrawEnumCombo(elemClass, std::format("[{}]", i).c_str(), list[i]);
				} else {
					if (!group.Has(name)) group.Add(name, std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>());
					auto& list = std::get<std::vector<std::shared_ptr<ONEngine::Variables::GenericObject>>>(const_cast<ONEngine::Variables::Var&>(group.Get(name)));
					int size = (int)list.size();
					if (ImGui::InputInt("Size", &size)) {
						if (size < 0) size = 0;
						size_t oldSize = list.size(); list.resize(size);
						for (size_t i = oldSize; i < list.size(); ++i) {
							auto elemVar = ONEngine::Variables::MonoObjectToVar(nullptr, elemType);
							if (std::holds_alternative<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar)) {
								list[i] = std::get<std::shared_ptr<ONEngine::Variables::GenericObject>>(elemVar);
							} else {
								list[i] = std::make_shared<ONEngine::Variables::GenericObject>();
								list[i]->typeName = mono_class_get_name(elemClass);
							}
						}
					}
					for (int i = 0; i < (int)list.size(); ++i) {
						if (ImGui::CollapsingHeader(std::format("[{}]", i).c_str())) {
							ImGui::Indent(); DrawGenericObject(list[i]); ImGui::Unindent();
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
						else { MonoObject* item = mono_object_new(domain, ek); mono_runtime_object_init(item); void* args[1] = { item }; mono_runtime_invoke(addMethod, listObj, args, nullptr); }
					}
				}
			} else if(size < count) {
				for(int i = 0; i < count - size; ++i) { int idx = size; void* args[1] = { &idx }; mono_runtime_invoke(removeAtMethod, listObj, args, nullptr); }
			}
			count = size;
		}
		for(int i = 0; i < count; ++i) {
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
	if(fieldDrawers.find(typeName) == fieldDrawers.end()) return;
	fieldDrawers[typeName]->Draw(scriptName, obj, field, name);
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
