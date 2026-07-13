#include "Variables.h"

/// std
#include <fstream>
#include <filesystem>
#include <format>
#include <vector>
#include <variant>

/// external
#include <imgui.h>
#include <Externals/imgui/dialog/ImGuiFileDialog.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/blob.h>
#include <mono/metadata/loader.h>
#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/threads.h>

/// engine
#include "Engine/Core/Utility/Math/Math.h"
#include "Engine/ECS/EntityComponentSystem/ECSGroup.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Entity/EntityJsonConverter.h"
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/Script/MonoScriptEngine.h"

/// editor 
#include "Engine/Editor/Math/ImGuiMath.h"

using namespace ONEngine;
using json = nlohmann::json;

namespace {

	bool IsVectorN(const json& j, int n) {
		if (!j.is_object()) {
			return false;
		}

		static const char* keys[] = { "x", "y", "z", "w" };
		for (int i = 0; i < n; ++i) {
			if (!j.contains(keys[i]) || !j[keys[i]].is_number()) {
				return false;
			}
		}
		return true;
	}

	bool HasSerializeField(MonoClassField* field) {
		const char* fieldName = mono_field_get_name(field);
		if (fieldName && (fieldName[0] == '<' || strstr(fieldName, "k__BackingField"))) {
			return false;
		}

		MonoClass* klass = mono_field_get_parent(field);
		if (!klass) {
			return false;
		}

		std::string className = mono_class_get_name(klass);
		std::string key = className + "." + (fieldName ? fieldName : "");

		static std::unordered_map<std::string, bool> serializeCache;
		auto it = serializeCache.find(key);
		if (it != serializeCache.end()) {
			return it->second;
		}

		MonoCustomAttrInfo* attrs = mono_custom_attrs_from_field(klass, field);
		if (!attrs) {
			Console::Log(std::format("[SerializeField] attrs=null for field: {}", fieldName ? fieldName : ""), ONEngine::LogCategory::ScriptEngine);
			serializeCache[key] = false;
			return false;
		}

		MonoImage* klassImage = mono_class_get_image(klass);
		MonoClass* serializeFieldAttr = mono_class_from_name(klassImage, "", "SerializeField");
		bool has = serializeFieldAttr && mono_custom_attrs_has_attr(attrs, serializeFieldAttr);

		if (!has) {
			MonoImage* engineImage = MonoScriptEngine::GetInstance().Image();
			if (engineImage && engineImage != klassImage) {
				MonoClass* serializeFieldAttrEngine = mono_class_from_name(engineImage, "", "SerializeField");
				if (serializeFieldAttrEngine && mono_custom_attrs_has_attr(attrs, serializeFieldAttrEngine)) {
					has = true;
				}
			}
		}

		Console::Log(std::format("[SerializeField] field={} has={}", fieldName ? fieldName : "", has ? "true" : "false"), ONEngine::LogCategory::ScriptEngine);
		mono_custom_attrs_free(attrs);

		serializeCache[key] = has;
		return has;
	}

	bool IsPublicField(MonoClassField* field) {
		uint32_t flags = mono_field_get_flags(field);
		return (flags & 0x0006) == 0x0006; // FIELD_ATTRIBUTE_PUBLIC
	}

	bool ShouldSerialize(MonoClassField* field) {
		// Releaseビルドでは SafeInvoke を経由しない Mono API 呼び出し前に
		// 必ずスレッドをドメインにアタッチする必要がある
		MonoDomain* domain = MonoScriptEngine::GetInstance().Domain();
		if (mono_thread_current() == nullptr) {
			mono_thread_attach(domain);
		} else {
			mono_domain_set(domain, false);
		}
		return IsPublicField(field) || HasSerializeField(field);
	}

	Variables::Var JsonToVar(const json& varValue);
	json VarToJson(const Variables::Var& var);

	std::shared_ptr<Variables::GenericObject> JsonToObject(const json& j) {
		auto obj = std::make_shared<Variables::GenericObject>();
		if (j.contains("__type")) obj->typeName = j["__type"];
		for (auto& [k, v] : j.items()) {
			if (k == "__type") continue;
			obj->fields[k] = JsonToVar(v);
		}
		return obj;
	}

	Variables::Var JsonToVar(const json& varValue) {
		if (varValue.is_array()) {
			if (varValue.empty()) {
				return std::vector<int>();
			}
			if (varValue[0].is_object() && !IsVectorN(varValue[0], 2)) {
				std::vector<std::shared_ptr<Variables::GenericObject>> list;
				for (const auto& v : varValue) list.push_back(JsonToObject(v));
				return list;
			}
			if (varValue[0].is_number_integer()) return varValue.get<std::vector<int>>();
			if (varValue[0].is_number_float()) return varValue.get<std::vector<float>>();
			if (varValue[0].is_boolean()) return varValue.get<std::vector<bool>>();
			if (varValue[0].is_string()) return varValue.get<std::vector<std::string>>();
			if (IsVectorN(varValue[0], 3)) {
				std::vector<Vector3> vecs;
				for (const auto& v : varValue) vecs.push_back(v);
				return vecs;
			}
			return std::vector<int>();
		}
		if (varValue.is_object()) {
			if (IsVectorN(varValue, 4)) return (Vector4)varValue;
			if (IsVectorN(varValue, 3)) return (Vector3)varValue;
			if (IsVectorN(varValue, 2)) return (Vector2)varValue;
			return JsonToObject(varValue);
		}
		if (varValue.is_number_integer()) return varValue.get<int>();
		if (varValue.is_number_float()) return varValue.get<float>();
		if (varValue.is_boolean()) return varValue.get<bool>();
		if (varValue.is_string()) return varValue.get<std::string>();
		return 0;
	}

	json VarToJson(const Variables::Var& var) {
		return std::visit([](auto&& arg) -> json {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, bool> || std::is_same_v<T, std::string> ||
				std::is_same_v<T, Vector2> || std::is_same_v<T, Vector3> || std::is_same_v<T, Vector4> ||
				std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> ||
				std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<std::string>>) {
				return arg;
			} else if constexpr (std::is_same_v<T, std::vector<Vector3>>) {
				json j = json::array();
				for (const auto& v : arg) j.push_back(v);
				return j;
			} else if constexpr (std::is_same_v<T, std::shared_ptr<Variables::GenericObject>>) {
				json j = json::object();
				if (arg) {
					j["__type"] = arg->typeName;
					for (auto& [k, v] : arg->fields) j[k] = VarToJson(v);
				}
				return j;
			} else {
				static_assert(std::is_same_v<T, std::vector<std::shared_ptr<Variables::GenericObject>>>, "Unhandled type in VarToJson");
				json j = json::array();
				for (const auto& o : arg) {
					json oj = json::object();
					if (o) {
						oj["__type"] = o->typeName;
						for (auto& [k, v] : o->fields) oj[k] = VarToJson(v);
					}
					j.push_back(oj);
				}
				return j;
			}
			}, var);
	}

}	/// namespace

Variables::Var Variables::MonoObjectToVar(void* obj, void* type) {
	int typeId = mono_type_get_type((MonoType*)type);

	if (!obj) {
		switch (typeId) {
		case MONO_TYPE_I4: return 0;
		case MONO_TYPE_R4: return 0.0f;
		case MONO_TYPE_BOOLEAN: return false;
		case MONO_TYPE_STRING: return std::string("");
		case MONO_TYPE_VALUETYPE:
		{
			MonoClass* klass = mono_class_from_mono_type((MonoType*)type);
			const char* name = mono_class_get_name(klass);
			if (strcmp(name, "Vector2") == 0) return Vector2::Zero;
			if (strcmp(name, "Vector3") == 0) return Vector3::Zero;
			if (strcmp(name, "Vector4") == 0) return Vector4::Zero;
			if (mono_class_is_enum(klass)) return 0;
			auto gen = std::make_shared<Variables::GenericObject>();
			gen->typeName = name;
			void* iter = nullptr; MonoClassField* f;
			while ((f = mono_class_get_fields(klass, &iter))) {
				if (ShouldSerialize(f)) gen->fields[mono_field_get_name(f)] = MonoObjectToVar(nullptr, mono_field_get_type(f));
			}
			return gen;
		}
		case MONO_TYPE_CLASS:
		{
			MonoClass* klass = mono_class_from_mono_type((MonoType*)type);
			if (!klass) return std::shared_ptr<GenericObject>(nullptr);
			const char* name = mono_class_get_name(klass);
			if (strcmp(name, "String") == 0) return std::string("");

			auto gen = std::make_shared<Variables::GenericObject>();
			gen->typeName = name;
			MonoClass* currentClass = klass;
			while (currentClass) {
				void* iter = nullptr; MonoClassField* f;
				while ((f = mono_class_get_fields(currentClass, &iter))) {
					if (ShouldSerialize(f)) gen->fields[mono_field_get_name(f)] = MonoObjectToVar(nullptr, mono_field_get_type(f));
				}
				currentClass = mono_class_get_parent(currentClass);
			}
			return gen;
		}
		case MONO_TYPE_GENERICINST: return std::vector<int>();
		default: return 0;
		}
	}

	switch (typeId) {
	case MONO_TYPE_I4: return *(int*)mono_object_unbox((MonoObject*)obj);
	case MONO_TYPE_R4: return *(float*)mono_object_unbox((MonoObject*)obj);
	case MONO_TYPE_BOOLEAN: return *(bool*)mono_object_unbox((MonoObject*)obj);
	case MONO_TYPE_STRING: return mono_string_to_utf8((MonoString*)obj);
	case MONO_TYPE_VALUETYPE:
	{
		MonoClass* klass = mono_class_from_mono_type((MonoType*)type);
		const char* name = mono_class_get_name(klass);
		if (strcmp(name, "Vector2") == 0) return *(Vector2*)mono_object_unbox((MonoObject*)obj);
		if (strcmp(name, "Vector3") == 0) return *(Vector3*)mono_object_unbox((MonoObject*)obj);
		if (strcmp(name, "Vector4") == 0) return *(Vector4*)mono_object_unbox((MonoObject*)obj);
		if (mono_class_is_enum(klass)) return *(int*)mono_object_unbox((MonoObject*)obj);
		return MonoObjectToGeneric(obj);
	}
	case MONO_TYPE_CLASS: return MonoObjectToGeneric(obj);
	case MONO_TYPE_GENERICINST:
	{
		MonoClass* klass = mono_object_get_class((MonoObject*)obj);
		if (strcmp(mono_class_get_name(klass), "List`1") != 0) return 0;
		MonoMethod* getCount = mono_class_get_method_from_name(klass, "get_Count", 0);
		int count = *(int*)mono_object_unbox(mono_runtime_invoke(getCount, (MonoObject*)obj, nullptr, nullptr));
		MonoMethod* getItem = mono_class_get_method_from_name(klass, "get_Item", 1);
		MonoType* elemType = mono_signature_get_return_type(mono_method_signature(getItem));

		int etid = mono_type_get_type(elemType);
		if (etid == MONO_TYPE_I4) {
			std::vector<int> l(count);
			for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = *(int*)mono_object_unbox(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
			return l;
		}
		if (etid == MONO_TYPE_R4) {
			std::vector<float> l(count);
			for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = *(float*)mono_object_unbox(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
			return l;
		}
		if (etid == MONO_TYPE_BOOLEAN) {
			std::vector<bool> l(count);
			for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = *(bool*)mono_object_unbox(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
			return l;
		}
		if (etid == MONO_TYPE_STRING) {
			std::vector<std::string> l(count);
			for (int i = 0; i < count; ++i) { void* a[1] = { &i }; MonoObject* s = mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr); if (s) l[i] = mono_string_to_utf8((MonoString*)s); }
			return l;
		}
		if (etid == MONO_TYPE_VALUETYPE || etid == MONO_TYPE_CLASS) {
			MonoClass* ek = mono_class_from_mono_type(elemType);
			if (strcmp(mono_class_get_name(ek), "Vector3") == 0) {
				std::vector<Vector3> l(count);
				for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = *(Vector3*)mono_object_unbox(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
				return l;
			}
			if (mono_class_is_enum(ek)) {
				std::vector<int> l(count);
				for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = *(int*)mono_object_unbox(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
				return l;
			}
			std::vector<std::shared_ptr<Variables::GenericObject>> l(count);
			for (int i = 0; i < count; ++i) { void* a[1] = { &i }; l[i] = MonoObjectToGeneric(mono_runtime_invoke(getItem, (MonoObject*)obj, a, nullptr)); }
			return l;
		}
	}
	}
	return 0;
}

std::shared_ptr<Variables::GenericObject> Variables::MonoObjectToGeneric(void* obj) {
	if (!obj) return nullptr;
	auto gen = std::make_shared<Variables::GenericObject>();
	MonoClass* klass = mono_object_get_class((MonoObject*)obj);
	gen->typeName = mono_class_get_name(klass);

	MonoClass* currentClass = klass;
	while (currentClass) {
		void* iter = nullptr;
		MonoClassField* field = nullptr;
		while ((field = mono_class_get_fields(currentClass, &iter))) {
			if (ShouldSerialize(field)) {
				const char* name = mono_field_get_name(field);
				MonoType* type = mono_field_get_type(field);
				gen->fields[name] = MonoObjectToVar(mono_field_get_value_object(mono_domain_get(), field, (MonoObject*)obj), type);
			}
		}
		currentClass = mono_class_get_parent(currentClass);
	}
	return gen;
}

void Variables::VarToMonoObject(void* obj, void* klass, const Variables::Var& var) {
	if (!obj || !std::holds_alternative<std::shared_ptr<Variables::GenericObject>>(var)) return;
	auto gen = std::get<std::shared_ptr<Variables::GenericObject>>(var);
	if (!gen) return;

	for (auto& [name, val] : gen->fields) {
		MonoClassField* field = ONEngine::MonoScriptEngineUtils::FindFieldRecursive((MonoClass*)klass, name.c_str());
		if (!field) continue;

		std::visit([&](auto&& arg) {
			using T = std::decay_t<decltype(arg)>;
			if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, bool> || std::is_same_v<T, Vector2> || std::is_same_v<T, Vector3> || std::is_same_v<T, Vector4>) {
				mono_field_set_value((MonoObject*)obj, field, (void*)&arg);
			} else if constexpr (std::is_same_v<T, std::string>) {
				MonoString* s = mono_string_new(mono_domain_get(), arg.c_str());
				mono_field_set_value((MonoObject*)obj, field, s);
			} else if constexpr (std::is_same_v<T, std::shared_ptr<Variables::GenericObject>>) {
				MonoObject* child = mono_field_get_value_object(mono_domain_get(), field, (MonoObject*)obj);
				if (child) VarToMonoObject(child, mono_object_get_class(child), val);
			} else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<std::string>> || std::is_same_v<T, std::vector<Vector3>>) {
				MonoObject* list = mono_field_get_value_object(mono_domain_get(), field, (MonoObject*)obj);
				if (list) {
					MonoClass* lc = mono_object_get_class(list);
					MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
					if (clear) mono_runtime_invoke(clear, list, nullptr, nullptr);
					MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
					if (add) {
						for (auto itemVal : arg) {
							if constexpr (std::is_same_v<T, std::vector<std::string>>) {
								MonoString* s = mono_string_new(mono_domain_get(), itemVal.c_str());
								void* args[1] = { s };
								mono_runtime_invoke(add, list, args, nullptr);
							} else {
								void* args[1] = { (void*)&itemVal };
								mono_runtime_invoke(add, list, args, nullptr);
							}
						}
					}
				}
			} else if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<Variables::GenericObject>>>) {
				MonoObject* list = mono_field_get_value_object(mono_domain_get(), field, (MonoObject*)obj);
				if (list) {
					MonoClass* lc = mono_object_get_class(list);
					MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
					mono_runtime_invoke(clear, list, nullptr, nullptr);
					MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
					MonoType* et = mono_signature_get_return_type(mono_method_signature(mono_class_get_method_from_name(lc, "get_Item", 1)));
					MonoClass* ek = mono_class_from_mono_type(et);
					for (auto& itemGen : arg) {
						MonoObject* item = mono_object_new(mono_domain_get(), ek);
						if (!mono_class_is_valuetype(ek)) {
							mono_runtime_object_init(item);
						}
						VarToMonoObject(item, ek, itemGen);
						void* args[1] = { item };
						mono_runtime_invoke(add, list, args, nullptr);
					}
				}
			}
			}, val);
	}
}

std::shared_ptr<Variables::GenericObject> Variables::CloneGenericObject(const std::shared_ptr<Variables::GenericObject>& src) {
	if (!src) return nullptr;
	auto dst = std::make_shared<Variables::GenericObject>();
	dst->typeName = src->typeName;
	for (const auto& [k, v] : src->fields) {
		dst->fields[k] = CloneVar(v);
	}
	return dst;
}

bool Variables::IsEqualGenericObject(const std::shared_ptr<Variables::GenericObject>& a, const std::shared_ptr<Variables::GenericObject>& b) {
	if (a == b) return true;
	if (!a || !b) return false;
	if (a->typeName != b->typeName) return false;
	if (a->fields.size() != b->fields.size()) return false;
	for (auto& [k, v] : a->fields) {
		if (!b->fields.contains(k)) return false;
		if (!IsEqualVar(v, b->fields.at(k))) return false;
	}
	return true;
}

Variables::Var Variables::CloneVar(const Variables::Var& src) {
	return std::visit([](auto&& arg) -> Variables::Var {
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::shared_ptr<Variables::GenericObject>>) {
			return CloneGenericObject(arg);
		} else if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<Variables::GenericObject>>>) {
			std::vector<std::shared_ptr<Variables::GenericObject>> listDst;
			for (const auto& item : arg) {
				listDst.push_back(CloneGenericObject(item));
			}
			return listDst;
		} else {
			return arg;
		}
	}, src);
}

bool Variables::IsEqualVar(const Variables::Var& a, const Variables::Var& b) {
	if (a.index() != b.index()) return false;
	return std::visit([&](auto&& argA) -> bool {
		using T = std::decay_t<decltype(argA)>;
		auto&& argB = std::get<T>(b);
		if constexpr (std::is_same_v<T, std::shared_ptr<Variables::GenericObject>>) {
			return IsEqualGenericObject(argA, argB);
		} else if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<Variables::GenericObject>>>) {
			if (argA.size() != argB.size()) return false;
			for (size_t i = 0; i < argA.size(); ++i) {
				if (!IsEqualGenericObject(argA[i], argB[i])) return false;
			}
			return true;
		} else if constexpr (std::is_same_v<T, Vector2>) {
			return argA.x == argB.x && argA.y == argB.y;
		} else if constexpr (std::is_same_v<T, Vector3>) {
			return argA.x == argB.x && argA.y == argB.y && argA.z == argB.z;
		} else if constexpr (std::is_same_v<T, Vector4>) {
			return argA.x == argB.x && argA.y == argB.y && argA.z == argB.z && argA.w == argB.w;
		} else if constexpr (std::is_same_v<T, std::vector<Vector3>>) {
			if (argA.size() != argB.size()) return false;
			for (size_t i = 0; i < argA.size(); ++i) {
				if (argA[i].x != argB[i].x || argA[i].y != argB[i].y || argA[i].z != argB[i].z) return false;
			}
			return true;
		} else {
			return argA == argB;
		}
	}, a);
}

void ONEngine::from_json(const nlohmann::json& j, Variables& v) {
	v.groupKeyMap_.clear();
	v.groups_.clear();

	for (auto& [groupKey, groupValue] : j.items()) {
		if (groupKey == "type") continue;

		if (!v.HasGroup(groupKey)) v.AddGroup(groupKey);
		Variables::Group& group = v.groups_[v.groupKeyMap_.at(groupKey)];

		for (auto& [varKey, varValue] : groupValue.items()) {
			group.Add(varKey, JsonToVar(varValue));
		}
	}
}

void ONEngine::to_json(nlohmann::json& j, const Variables& v) {
	j = nlohmann::json::object();
	j["type"] = "Variables";

	for (const auto& [groupKey, value] : v.groupKeyMap_) {
		j[groupKey] = nlohmann::json::object();
		for (const auto& [varKey, varValue] : v.groups_[value].keyMap) {
			j[groupKey][varKey] = VarToJson(v.groups_[value].variables[varValue]);
		}
	}
}

Variables::Variables() {
	groupKeyMap_.clear();
	groups_.clear();
}

Variables::~Variables() = default;

void Variables::LoadJson(const std::string& path) {
	std::string ext = FileSystem::FileExtension(path);
	if (ext != ".json" && ext != ".entity") return;
	if (!std::filesystem::exists(path)) return;

	nlohmann::json j;
	{
		std::ifstream ifs(path);
		if (!ifs.is_open()) return;
		ifs >> j;
		ifs.close();
	}

	if (j.contains("components")) {
		for (const auto& compJson : j["components"]) {
			if (compJson.value("type", "") == "Variables") {
				from_json(compJson, *this);
				break;
			}
		}
	} else {
		from_json(j, *this);
	}

	if (Script* script = GetOwner()->GetComponent<Script>()) {
		for (const auto& data : script->GetScriptDataList()) {
			SetScriptVariables(data.scriptName);
		}
	}
}

void Variables::SaveJson(const std::string& path) {
	nlohmann::json j;
	GameEntity* owner = GetOwner();
	if (!owner) return;

	to_json(j, *this);
	if (j.contains("type")) j.erase("type");

	std::filesystem::path fsPath(path);
	std::filesystem::create_directories(fsPath.parent_path());

	std::ofstream ofs(fsPath);
	if (!ofs) throw std::runtime_error("Failed to open: " + path);
	ofs << j.dump(4);
}

void Variables::RegisterScriptVariables() {
	// Releaseビルドでのスレッドアタッチを保証する
	MonoDomain* domain = MonoScriptEngine::GetInstance().Domain();
	if (mono_thread_current() == nullptr) {
		mono_thread_attach(domain);
	} else {
		mono_domain_set(domain, false);
	}

	Script* script = GetOwner()->GetComponent<Script>();
	if (!script) return;

	for (const auto& data : script->GetScriptDataList()) {
		size_t groupIndex = 0;
		if (!HasGroup(data.scriptName)) groupIndex = AddGroup(data.scriptName);
		else groupIndex = groupKeyMap_.at(data.scriptName);
		Group& group = groups_[groupIndex];

		{
			MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
			GameEntity* entity = GetOwner();
			MonoObject* safeObj = monoEngine.GetMonoBehaviorFromCS(entity->GetECSGroup()->GetGroupName(), entity->GetId(), data.scriptName);
			if (!safeObj) continue;

			MonoClass* currentClass = mono_object_get_class(safeObj);
			while (currentClass) {
				void* iter = nullptr;
				MonoClassField* field = nullptr;
				while ((field = mono_class_get_fields(currentClass, &iter))) {
					if (!ShouldSerialize(field)) continue;
					const char* fieldName = mono_field_get_name(field);
					if (group.Has(fieldName)) continue;
					group.Add(fieldName, MonoObjectToVar(mono_field_get_value_object(mono_domain_get(), field, safeObj), mono_field_get_type(field)));
				}
				currentClass = mono_class_get_parent(currentClass);
			}
		}
	}
}

void Variables::ReloadScriptVariables() {
	// Releaseビルドでのスレッドアタッチを保証する
	MonoDomain* domain = MonoScriptEngine::GetInstance().Domain();
	if (mono_thread_current() == nullptr) {
		mono_thread_attach(domain);
	} else {
		mono_domain_set(domain, false);
	}

	Script* script = GetOwner()->GetComponent<Script>();
	if (!script) return;

	for (const auto& data : script->GetScriptDataList()) {
		size_t groupIndex = 0;
		if (!HasGroup(data.scriptName)) groupIndex = AddGroup(data.scriptName);
		else groupIndex = groupKeyMap_.at(data.scriptName);
		Group& group = groups_[groupIndex];

		{
			MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
			GameEntity* entity = GetOwner();
			MonoObject* safeObj = monoEngine.GetMonoBehaviorFromCS(entity->GetECSGroup()->GetGroupName(), entity->GetId(), data.scriptName);
			if (!safeObj) continue;

			MonoClass* currentClass = mono_object_get_class(safeObj);
			while (currentClass) {
				void* iter = nullptr;
				MonoClassField* field = nullptr;
				while ((field = mono_class_get_fields(currentClass, &iter))) {
					if (!ShouldSerialize(field)) continue;
					group.Add(mono_field_get_name(field), MonoObjectToVar(mono_field_get_value_object(mono_domain_get(), field, safeObj), mono_field_get_type(field)));
				}
				currentClass = mono_class_get_parent(currentClass);
			}
		}
	}
}

void Variables::SetScriptVariables(const std::string& scriptName) {
	// Releaseビルドでは SafeInvoke を経由しない Mono API (mono_field_set_value 等) を
	// 直接呼び出すため、スレッドをドメインにアタッチしておく
	MonoDomain* domain = MonoScriptEngine::GetInstance().Domain();
	if (mono_thread_current() == nullptr) {
		mono_thread_attach(domain);
	} else {
		mono_domain_set(domain, false);
	}

	GameEntity* owner = GetOwner();
	if (!owner) {
		Console::Log("[SetScriptVars] owner is null for script: " + scriptName, ONEngine::LogCategory::ScriptEngine);
		return;
	}
	Script* script = owner->GetComponent<Script>();
	if (!script) {
		Console::Log("[SetScriptVars] no Script component for: " + scriptName, ONEngine::LogCategory::ScriptEngine);
		return;
	}
	if (!HasGroup(scriptName)) {
		Console::Log("[SetScriptVars] no group for scriptName: " + scriptName + "  groups: " + std::to_string(groups_.size()), ONEngine::LogCategory::ScriptEngine);
		return;
	}

	Group& group = groups_[groupKeyMap_.at(scriptName)];
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
	MonoObject* safeObj = monoEngine.GetMonoBehaviorFromCS(owner->GetECSGroup()->GetGroupName(), owner->GetId(), scriptName);
	if (!safeObj) {
		Console::Log("[SetScriptVars] GetMonoBehaviorFromCS returned null for: " + scriptName, ONEngine::LogCategory::ScriptEngine);
		return;
	}
	Console::Log("[SetScriptVars] OK setting fields for: " + scriptName, ONEngine::LogCategory::ScriptEngine);

	MonoClass* currentClass = mono_object_get_class(safeObj);
	while (currentClass) {
		void* iter = nullptr;
		MonoClassField* field = nullptr;
		while ((field = mono_class_get_fields(currentClass, &iter))) {
			if (!ShouldSerialize(field)) continue;
			const char* name = mono_field_get_name(field);
			if (!group.Has(name)) continue;

			// C#側の型情報に合わせてC++側のVariablesの型を自動補正する
			MonoType* fieldType = mono_field_get_type(field);
			int typeId = mono_type_get_type(fieldType);
			auto& var = const_cast<Var&>(group.Get(name));

			if (typeId == MONO_TYPE_R4 && std::holds_alternative<int>(var)) {
				group.Add(name, static_cast<float>(std::get<int>(var)));
			} else if (typeId == MONO_TYPE_I4 && std::holds_alternative<float>(var)) {
				group.Add(name, static_cast<int>(std::get<float>(var)));
			} else if (typeId == MONO_TYPE_BOOLEAN && std::holds_alternative<int>(var)) {
				group.Add(name, std::get<int>(var) != 0);
			} else if (typeId == MONO_TYPE_BOOLEAN && std::holds_alternative<float>(var)) {
				group.Add(name, std::get<float>(var) != 0.0f);
			} else if (typeId == MONO_TYPE_VALUETYPE || typeId == MONO_TYPE_CLASS) {
				MonoClass* fieldClass = mono_class_from_mono_type(fieldType);
				if (fieldClass) {
					const char* className = mono_class_get_name(fieldClass);
					if (strcmp(className, "Vector2") == 0 && !std::holds_alternative<Vector2>(var)) {
						group.Add(name, Vector2::Zero);
					} else if (strcmp(className, "Vector3") == 0 && !std::holds_alternative<Vector3>(var)) {
						group.Add(name, Vector3::Zero);
					} else if (strcmp(className, "Vector4") == 0 && !std::holds_alternative<Vector4>(var)) {
						group.Add(name, Vector4::Zero);
					}
				}
			} else if (typeId == MONO_TYPE_STRING && !std::holds_alternative<std::string>(var)) {
				group.Add(name, std::string(""));
			}

			auto& val = group.Get(name);

			std::visit([&](auto&& arg) {
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<T, bool> || std::is_same_v<T, Vector2> || std::is_same_v<T, Vector3> || std::is_same_v<T, Vector4>) {
					mono_field_set_value(safeObj, field, (void*)&arg);
				} else if constexpr (std::is_same_v<T, std::string>) {
					mono_field_set_value(safeObj, field, mono_string_new(mono_domain_get(), arg.c_str()));
				} else if constexpr (std::is_same_v<T, std::shared_ptr<Variables::GenericObject>>) {
					MonoObject* obj = mono_field_get_value_object(mono_domain_get(), field, safeObj);
					if (obj) VarToMonoObject(obj, mono_object_get_class(obj), val);
				} else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<bool>> || std::is_same_v<T, std::vector<std::string>> || std::is_same_v<T, std::vector<Vector3>>) {
					MonoObject* list = mono_field_get_value_object(mono_domain_get(), field, safeObj);
					if (list) {
						MonoClass* lc = mono_object_get_class(list);
						MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
						if (clear) mono_runtime_invoke(clear, list, nullptr, nullptr);
						MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
						if (add) {
							for (auto itemVal : arg) {
								if constexpr (std::is_same_v<T, std::vector<std::string>>) {
									MonoString* s = mono_string_new(mono_domain_get(), itemVal.c_str());
									void* args[1] = { s };
									mono_runtime_invoke(add, list, args, nullptr);
								} else {
									void* args[1] = { (void*)&itemVal };
									mono_runtime_invoke(add, list, args, nullptr);
								}
							}
						}
					}
				} else if constexpr (std::is_same_v<T, std::vector<std::shared_ptr<Variables::GenericObject>>>) {
					MonoObject* list = mono_field_get_value_object(mono_domain_get(), field, safeObj);
					if (list) {
						MonoClass* lc = mono_object_get_class(list);
						MonoMethod* clear = mono_class_get_method_from_name(lc, "Clear", 0);
						mono_runtime_invoke(clear, list, nullptr, nullptr);
						MonoMethod* add = mono_class_get_method_from_name(lc, "Add", 1);
						MonoType* et = mono_signature_get_return_type(mono_method_signature(mono_class_get_method_from_name(lc, "get_Item", 1)));
						MonoClass* ek = mono_class_from_mono_type(et);
						for (auto& itemGen : arg) {
							MonoObject* item = mono_object_new(mono_domain_get(), ek);
							if (!mono_class_is_valuetype(ek)) {
								mono_runtime_object_init(item);
							}
							VarToMonoObject(item, ek, itemGen);
							void* args[1] = { item };
							mono_runtime_invoke(add, list, args, nullptr);
						}
					}
				}
				}, val);
		}
		currentClass = mono_class_get_parent(currentClass);
	}
}

size_t Variables::AddGroup(const std::string& name) {
	if (groupKeyMap_.contains(name)) return groupKeyMap_.at(name);
	Group group; group.name = name;
	size_t index = groups_.size();
	groups_.push_back(group);
	groupKeyMap_[name] = index;
	return index;
}

const Variables::Group& Variables::GetGroup(const std::string& name) const { return groups_[groupKeyMap_.at(name)]; }
bool Variables::HasGroup(const std::string& name) const { return groupKeyMap_.contains(name); }
const std::unordered_map<std::string, size_t>& Variables::GetGroupKeyMap() const { return groupKeyMap_; }
const std::vector<Variables::Group>& Variables::GetGroups() const { return groups_; }

void Variables::SetVariable(const std::string& groupName, const std::string& varName, const Var& value) {
	size_t groupIdx = HasGroup(groupName) ? groupKeyMap_.at(groupName) : AddGroup(groupName);
	groups_[groupIdx].Add(varName, value);
}

void ComponentDebug::VariablesDebug(Variables* variables) {
	if (!variables) return;
	if (ImGui::Button("export entity")) {
		GameEntity* entity = variables->GetOwner();
		variables->ReloadScriptVariables();
		nlohmann::json entityJson = EntityJsonConverter::ToJson(entity);
		std::string path = "Assets/Scene/" + entity->GetECSGroup()->GetGroupName() + "/" + entity->GetName() + ".entity";
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());
		std::ofstream ofs(path);
		if (ofs) { ofs << entityJson.dump(4); Console::Log("Exported to: " + path); }
	}
}

const Variables::Var& Variables::Group::Get(const std::string& varName) const { return variables[keyMap.at(varName)]; }
bool Variables::Group::Has(const std::string& varName) const { return keyMap.contains(varName); }
