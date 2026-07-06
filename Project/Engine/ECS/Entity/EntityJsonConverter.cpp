#include "EntityJsonConverter.h"

/// engine
#include "Engine/Editor/Commands/ComponentEditCommands/ComponentJsonConverter.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Entity/Collection/EntityCollection.h"
#include "Engine/ECS/Entity/Prefab/EntityPrefab.h"

using namespace ONEngine;

namespace {
	// コンポーネントのリストから指定したタイプのコンポーネントJSONを探す
	nlohmann::json FindComponentByType(const nlohmann::json& components, const std::string& type) {
		if (!components.is_array()) return nlohmann::json();
		for (const auto& comp : components) {
			if (comp.value("type", "") == type) {
				return comp;
			}
		}
		return nlohmann::json();
	}

	// 数値の微小な誤差を考慮した比較
	bool IsJsonValueEqual(const nlohmann::json& a, const nlohmann::json& b) {
		if (a.is_number() && b.is_number()) {
			return std::abs(a.get<double>() - b.get<double>()) < 1e-4;
		}
		return a == b;
	}

	// 2つのJSONオブジェクトの差異を抽出する（再帰的）
	nlohmann::json GetJsonDiff(const nlohmann::json& current, const nlohmann::json& base) {
		if (IsJsonValueEqual(current, base)) return nlohmann::json();
		if (!current.is_object() || !base.is_object()) return current;

		nlohmann::json diff = nlohmann::json::object();
		for (auto it = current.begin(); it != current.end(); ++it) {
			const std::string& key = it.key();
			// typeは常に含める（識別のため）
			if (key == "type") {
				diff[key] = it.value();
				continue;
			}

			if (!base.contains(key)) {
				diff[key] = it.value();
			} else if (!IsJsonValueEqual(it.value(), base[key])) {
				if (it.value().is_object() && base[key].is_object()) {
					nlohmann::json subDiff = GetJsonDiff(it.value(), base[key]);
					// type以外の実質的な差異がある場合のみ追加
					if (!subDiff.is_null() && subDiff.size() > (subDiff.contains("type") ? 1 : 0)) {
						diff[key] = subDiff;
					}
				} else {
					diff[key] = it.value();
				}
			}
		}
		return diff;
	}
// ... (MergeJson remains same)

	// パッチJSONをベースJSONにマージする（再帰的）
	void MergeJson(nlohmann::json& base, const nlohmann::json& patch) {
		if (!patch.is_object() || !base.is_object()) return;
		for (auto it = patch.begin(); it != patch.end(); ++it) {
			const std::string& key = it.key();
			if (it.value().is_object() && base.contains(key) && base[key].is_object()) {
				MergeJson(base[key], it.value());
			} else {
				base[key] = it.value();
			}
		}
	}
}

nlohmann::json EntityJsonConverter::ToJson(const GameEntity* entity, bool forceFull) {
	if (!entity) {
		return nlohmann::json();
	}
	nlohmann::json entityJson = nlohmann::json::object();
	entityJson["prefabName"] = entity->GetPrefabName();
	entityJson["name"] = entity->GetName();
	entityJson["active"] = entity->active;
	// entityJson["id"] = entity->GetId(); // DEPRECATED: id is now runtime-only
	entityJson["guid"] = entity->GetGuid().ToString();

	// Prefabがある場合は差分のみを書き出す (forceFullがfalseの場合のみ)
	nlohmann::json prefabComponents;
	bool hasPrefab = false;
	if (!forceFull && entity->GetPrefabName() != "") {
		auto* collection = entity->GetECSGroup()->GetEntityCollection();
		auto* prefab = collection->GetPrefab(entity->GetPrefabName());
		if (prefab) {
			prefabComponents = prefab->GetJson().value("components", nlohmann::json::array());
			hasPrefab = true;
		}
	}

	// コンポーネントの情報を追加
	auto& components = entity->GetComponents();
	for (const auto& component : components) {
		nlohmann::json compJson = ComponentJsonConverter::ToJson(component.second);
		if (compJson.empty()) continue;

		if (hasPrefab && !forceFull) {
			std::string type = compJson.value("type", "");
			nlohmann::json prefabComp = FindComponentByType(prefabComponents, type);

			if (prefabComp.empty()) {
				// Prefabにないコンポーネントはそのまま保存
				entityJson["components"].push_back(compJson);
			} else {
				// 差分のみを計算
				nlohmann::json diff = GetJsonDiff(compJson, prefabComp);
				// type以外に変更があれば保存
				if (diff.size() > 1) {
					entityJson["components"].push_back(diff);
				}
			}
		} else {
			entityJson["components"].push_back(compJson);
		}
	}

	/// 親子関係の情報を追加
	if (entity->GetParent()) {
		entityJson["parentGuid"] = entity->GetParent()->GetGuid().ToString();
	} else {
		entityJson["parentGuid"] = nullptr;
	}

	return entityJson;
}

void EntityJsonConverter::FromJson(const nlohmann::json& json, GameEntity* entity, const std::string& /*groupName*/, bool merge) {

	/// name, prefabNameを設定
	if (json.contains("name")) {
		entity->SetName(json.at("name").get<std::string>());
	}

	if (json.contains("prefabName")) {
		const std::string& prefabName = json.at("prefabName").get<std::string>();
		if (prefabName != "") {
			entity->SetPrefabName(prefabName);
		}
	}

	if (json.contains("active")) {
		entity->active = json.at("active").get<bool>();
	}

	/// コンポーネントを追加
	if (json.contains("components")) {
		for (const auto& componentJson : json["components"]) {

			/// jsonにtypeが無ければスキップ
			if (!componentJson.contains("type")) {
				continue;
			}

			const std::string componentType = componentJson.at("type").get<std::string>();

			Console::Log(std::format("[CPP ECS] FromJson - Adding component '{}' to entity '{}'", componentType, entity->GetName()));

			IComponent* comp = entity->AddComponent(componentType);
			if (comp) {
				if (merge) {
					// 現在の状態をベースにマージして適用
					nlohmann::json base = ComponentJsonConverter::ToJson(comp);
					MergeJson(base, componentJson);
					ComponentJsonConverter::FromJson(base, comp);
				} else {
					// 直接上書き
					ComponentJsonConverter::FromJson(componentJson, comp);
				}
				comp->SetOwner(entity);
			} else {
				// コンポーネントの追加に失敗した場合のログ
				Console::LogError("failed add component: " + componentType);
			}
		}
	}
}

void EntityJsonConverter::TransformFromJson(const nlohmann::json& json, GameEntity* entity) {
	/// transformだけjsonから読み込む

	/// コンポーネントを追加
	if (json.contains("components")) {
		for (const auto& componentJson : json["components"]) {
			/// jsonにtypeが無ければスキップ
			if (!componentJson.contains("type")) {
				continue;
			}

			const std::string componentType = componentJson.at("type").get<std::string>();
			if (componentType != "Transform") {
				continue; // Transformコンポーネント以外はスキップ
			}


			IComponent* comp = entity->AddComponent(componentType);
			if (comp) {
				ComponentJsonConverter::FromJson(componentJson, comp);
				comp->SetOwner(entity);
			} else {
				// コンポーネントの追加に失敗した場合のログ
				Console::LogError("failed add component: " + componentType);
			}
		}
	}
}
