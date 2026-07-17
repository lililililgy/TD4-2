#include "EntityCollection.h"
#include <nlohmann/json.hpp>
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem2D/ParticleSystem2D.h"
#include "Engine/ECS/Component/Components/ComputeComponents/ParticleSystem/ParticleSystem.h"
#include "Engine/ECS/System/ParticleSystem2DUpdateSystem/ParticleSystem2DUpdateSystem.h"
#include "Engine/ECS/System/ParticleSystemUpdateSystem/ParticleSystemUpdateSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"

using namespace ONEngine;

/// std
#include <fstream>

/// engine
#include "Engine/Core/DirectX12/Manager/DxManager.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Entity/EntityJsonConverter.h"

/// ecs
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"

EntityCollection::EntityCollection(ECSGroup* ecsGroup, DxManager* dxm)
	: pEcsGroup_(ecsGroup), pDxManager_(dxm) {
	mainCamera_ = nullptr;
	mainCamera2D_ = nullptr;
	pDxDevice_ = pDxManager_->GetDxDevice();
	entities_.reserve(256);

	LoadPrefabAll();
}

EntityCollection::~EntityCollection() {}

GameEntity* EntityCollection::GenerateEntity(const Guid& guid, bool isRuntime) {
	Console::LogInfo("[SOURCE_DETECTOR] Engine creating Entity (Internal)");
	auto entity = std::make_unique<GameEntity>();
	if (entity) {
		entities_.emplace_back(std::move(entity));

		/// 初期化
		GameEntity* entityPtr = entities_.back().get();
		entityPtr->pEcsGroup_ = pEcsGroup_;
		entityPtr->id_ = NewEntityID(isRuntime);
		entityPtr->guid_ = guid;
		entityPtr->Awake();

		guidEntityMap_[guid] = entityPtr;

		return entities_.back().get();
	}
	return nullptr;
}

void EntityCollection::RemoveEntity(GameEntity* entity, bool deleteChildren) {

	if (entity == nullptr) {
		return;
	}

	/// ------------------------------
	/// 破棄可能かチェック
	/// ------------------------------
	auto doNotDestroyEntityItr = std::find_if(doNotDestroyEntities_.begin(), doNotDestroyEntities_.end(),
		[&entity](GameEntity* e) {
			return e == entity;
		}
	);

	if (doNotDestroyEntityItr != doNotDestroyEntities_.end()) {
		Console::Log("Cannot remove entity: " + entity->GetName() + " because it is marked as do not destroy.");
		return;
	}


	/// ------------------------------
	/// 実際に破棄する
	/// ------------------------------

	// ParticleSystem2D の退避
	if (auto* ps2d = entity->GetComponent<ParticleSystem2D>()) {
		if (ps2d->aliveCount > 0 && entity->GetTransform()) {
			ParticleSystem2DUpdateSystem::RegisterGhost(ps2d, entity->GetTransform()->matWorld);
		}
	}
	// ParticleSystem (3D) の退避
	if (auto* ps = entity->GetComponent<ParticleSystem>()) {
		if (ps->aliveCount > 0 && entity->GetTransform()) {
			ParticleSystemUpdateSystem::RegisterGhost(ps, entity->GetTransform()->matWorld);
		}
	}

	/// Componentの破棄
	entity->RemoveComponentAll();

	guidEntityMap_.erase(entity->GetGuid());
	RemoveEntityId(entity->GetId());

	/// 親子関係の解除
	entity->RemoveParent();
	if (deleteChildren) {

		if (entity->GetChildren().size() > 0) {
			auto children = entity->GetChildren();
			for (auto& child : children) {
				RemoveEntity(child, deleteChildren); ///< 子供の親子関係を解除した後に再帰的に削除
			}
		}
	} else {
		for (auto& child : entity->GetChildren()) {
			child->RemoveParent();
		}
	}

	/// entityの削除
	auto entityItr = std::remove_if(entities_.begin(), entities_.end(),
		[entity](const std::unique_ptr<GameEntity>& e) {
			return e.get() == entity;
		}
	);

	if (entityItr != entities_.end()) {
		entities_.erase(entityItr, entities_.end());
	}

}

void EntityCollection::RemoveEntityId(int32_t id) {
	if (id > 0) {
		/// 初期化時のidから削除
		initEntityIDs_.usedIds.erase(std::remove(initEntityIDs_.usedIds.begin(), initEntityIDs_.usedIds.end(), id), initEntityIDs_.usedIds.end());
		initEntityIDs_.removedIds.push_back(id);
	} else if (id < 0) {
		/// 実行時のidから削除
		runtimeEntityIDs_.usedIds.erase(std::remove(runtimeEntityIDs_.usedIds.begin(), runtimeEntityIDs_.usedIds.end(), id), runtimeEntityIDs_.usedIds.end());
		runtimeEntityIDs_.removedIds.push_back(id);
	} else {
		Console::LogWarning("Invalid entity ID: " + std::to_string(id));
		return;
	}
}

void EntityCollection::RemoveEntityAll() {
	std::vector<GameEntity*> toRemove;
	toRemove.reserve(entities_.size()); // 最適化

	for (const auto& entity : entities_) {
		/// 親がいるエンティティは再帰的に処理されるのでここでは追加しない
		if (!entity->GetParent()) {
			toRemove.push_back(entity.get()); // ポインタだけをコピー
		}
	}

	for (auto& entity : toRemove) {
		if (!entity->GetParent()) {
			RemoveEntity(entity, true); ///< 全てのエンティティを削除する
		}
	}

}

void EntityCollection::AddDoNotDestroyEntity(GameEntity* entity) {
	if (entity == nullptr) {
		return;
	}

	/// 既に存在する場合は追加しない
	if (std::find(doNotDestroyEntities_.begin(), doNotDestroyEntities_.end(), entity) != doNotDestroyEntities_.end()) {
		return;
	}

	doNotDestroyEntities_.push_back(entity);
}

void EntityCollection::RemoveDoNotDestroyEntity(GameEntity* entity) {
	auto itr = std::remove(doNotDestroyEntities_.begin(), doNotDestroyEntities_.end(), entity);
	if (itr != doNotDestroyEntities_.end()) {
		doNotDestroyEntities_.erase(itr);
	}
}

void EntityCollection::MoveEntity(GameEntity* entity, size_t newIndex) {
	auto it = std::find_if(entities_.begin(), entities_.end(), [entity](const std::unique_ptr<GameEntity>& e) {
		return e.get() == entity;
	});

	if (it != entities_.end()) {
		std::unique_ptr<GameEntity> entityPtr = std::move(*it);
		entities_.erase(it);

		if (newIndex > entities_.size()) {
			newIndex = entities_.size();
		}
		entities_.insert(entities_.begin() + newIndex, std::move(entityPtr));
	}
}

int32_t EntityCollection::NewEntityID(bool isRuntime) {
	int32_t resultId = 0;

	if (isRuntime) {
		/// Runtime起動後のID生成

		// 削除されたIDがあればそれを使用
		if (runtimeEntityIDs_.removedIds.size() > 0) {
			resultId = runtimeEntityIDs_.removedIds.front();
			runtimeEntityIDs_.removedIds.pop_front();
			runtimeEntityIDs_.usedIds.push_back(resultId);
		} else {
			// なければ新しいIDを生成
			resultId = static_cast<int32_t>(runtimeEntityIDs_.usedIds.size() + 1) * -1;
			runtimeEntityIDs_.usedIds.push_back(resultId);
		}


	} else {

		/// Runtime起動前のID生成
		if (initEntityIDs_.removedIds.size() > 0) {
			resultId = initEntityIDs_.removedIds.front();
			initEntityIDs_.removedIds.pop_front();
			initEntityIDs_.usedIds.push_back(resultId);
		} else {
			resultId = static_cast<int32_t>(initEntityIDs_.usedIds.size()) + 1;
			initEntityIDs_.usedIds.push_back(resultId);
		}
	}

	return resultId;
}

uint32_t EntityCollection::GetEntityId(const std::string& name) {
	for (auto& entity : entities_) {
		if (entity->name_ == name) {
			return static_cast<uint32_t>(entity->GetId());
		}
	}

	return 0;
}

GameEntity* EntityCollection::GetEntity(int32_t entityId) {
	auto itr = std::find_if(entities_.begin(), entities_.end(),
		[entityId](const std::unique_ptr<GameEntity>& entity) {
			return entity->GetId() == entityId;
		}
	);

	if (itr != entities_.end()) {
		return (*itr).get();
	}

	return nullptr;
}

GameEntity* EntityCollection::GetEntityFromGuid(const Guid& guid) {
	auto itr = guidEntityMap_.find(guid);
	if (itr != guidEntityMap_.end()) {
		return itr->second;
	}
	Console::LogWarning("Entity not found for Guid: " + guid.ToString());
	return nullptr;
}

void EntityCollection::LoadPrefabAll() {
	/// Assets/Prefabs フォルダから全てのプレハブを読み込む
	std::string prefabPath = "./Assets/Prefabs/";

	std::vector<File> prefabFiles = FileSystem::GetFiles(prefabPath, ".prefab");

	if (prefabFiles.empty()) {
		Console::Log("No prefab files found in: " + prefabPath);
		return;
	}

	/// directoryを探索
	for (const auto& file : prefabFiles) {
		prefabs_[file.second] = std::make_unique<EntityPrefab>(file.first);
	}
}

void EntityCollection::ReloadPrefab(const std::string& prefabName) {
	auto itr = prefabs_.find(prefabName);
	if (itr == prefabs_.end()) {
		// 拡張子なしで検索された場合に備えて ".prefab" を付けて再試行
		std::string nameWithExt = prefabName + ".prefab";
		itr = prefabs_.find(nameWithExt);
	}

	if (itr == prefabs_.end()) {
		/// もう一度Fileを探索して確認
		File file = FileSystem::GetFile("./Assets/Prefabs/", prefabName);

		if (file.first.empty()) {
			Console::LogWarning("Prefab not found: " + prefabName);
			return;
		}

		///!< 複数あった場合は最初に見つかったものを使用する
		prefabs_[file.second] = std::make_unique<EntityPrefab>(file.first);

		itr = prefabs_.find(file.second);
	}

	// 1. 各エンティティの現在の「カスタム変更（差分）」を一時保存する
	// ※リロード前のPrefab状態と比較して、何がオーバーライドされているかを記録
	const std::string& actualPrefabName = itr->first;
	std::unordered_map<GameEntity*, nlohmann::json> entityDiffs;
	for (auto& entity : entities_) {
		if (entity->GetPrefabName() == actualPrefabName || 
			(!actualPrefabName.empty() && entity->GetPrefabName() == FileSystem::FileNameWithoutExtension(actualPrefabName))) {
			// ToJson(entity, false) は現在の Prefab との差分を返す
			entityDiffs[entity.get()] = EntityJsonConverter::ToJson(entity.get(), false);
		}
	}

	/// 2. prefabをディスクから再読み込み（これでベースラインが更新される）
	itr->second->Reload();

	/// 3. 更新されたPrefabベースに、保存しておいた差分を再適用する
	for (auto& [entity, diff] : entityDiffs) {
		// まず、新しいPrefabを強制的に適用（ベースラインの更新）
		ApplyPrefabToEntity(entity, actualPrefabName);
		
		// 次に、保存しておいたカスタム差分をマージ適用（オーバーライドの復元）
		EntityJsonConverter::FromJson(diff, entity, pEcsGroup_->GetGroupName(), true);
	}
}

GameEntity* EntityCollection::GenerateEntityFromPrefab(const std::string& prefabName, bool isRuntime) {
	/// prefabが存在するかチェック
	auto prefabItr = prefabs_.find(prefabName);
	if (prefabItr == prefabs_.end()) {
		// 拡張子なしで検索された場合に備えて ".prefab" を付けて再試行
		std::string nameWithExt = prefabName + ".prefab";
		prefabItr = prefabs_.find(nameWithExt);

		if (prefabItr == prefabs_.end()) {
			Console::LogError("Prefab not found: " + prefabName);
			return nullptr;
		}
	}

	/// prefabを取得
	EntityPrefab* prefab = prefabItr->second.get();

	/// entityを生成する
	return GenerateEntityRecursive(prefab->GetJson(), nullptr, isRuntime);
}

EntityPrefab* EntityCollection::GetPrefab(const std::string& fileName) {
	if (prefabs_.contains(fileName)) {
		return prefabs_[fileName].get();
	}

	// 拡張子なしで検索された場合に備えて ".prefab" を付けて再試行
	std::string nameWithExt = fileName + ".prefab";
	if (prefabs_.contains(nameWithExt)) {
		return prefabs_[nameWithExt].get();
	}

	return nullptr;
}

void EntityCollection::ApplyPrefabToEntity(GameEntity* entity, const std::string& prefabName) {
	if (!entity) {
		Console::LogError("Entity is null when applying prefab: " + prefabName);
		return;
	}

	/// prefabが存在するかチェック
	auto prefabItr = prefabs_.find(prefabName);
	if (prefabItr == prefabs_.end()) {
		// 拡張子なしで検索された場合に備えて ".prefab" を付けて再試行
		std::string nameWithExt = prefabName + ".prefab";
		prefabItr = prefabs_.find(nameWithExt);

		if (prefabItr == prefabs_.end()) {
			Console::LogError("Prefab not found: " + prefabName);
			return;
		}
	}

	/// prefabを取得
	EntityPrefab* prefab = prefabItr->second.get();

	/// Jsonからコンポーネントを生成 (Prefab適用時はマージではなく上書き)
	EntityJsonConverter::FromJson(prefab->GetJson(), entity, pEcsGroup_->GetGroupName(), false);
	}

GameEntity* EntityCollection::GenerateEntityRecursive(const nlohmann::json& json, GameEntity* parent, bool isRuntime) {

	/*
	* Guidはファイル内のものではなく新規生成する
	* でないと同じPrefabから生成したときにGuidが被ってしまう
	*/

	/// entityを生成する
	GameEntity* entity = GenerateEntity(GenerateGuid(), isRuntime);
	if (!entity) {
		Console::LogError("Failed to generate entity from JSON.");
		return nullptr;
	}

	/// 親子関係の設定
	if (parent) {
		entity->SetParent(parent);
	}

	/// 名前とPrefab名を設定
	const std::string prefabName = json.value("prefabName", "");
	std::string name = FileSystem::FileNameWithoutExtension(prefabName);
	if (prefabName.empty()) {
		name = json.value("name", "New Entity");
	}

	/// runtime中かどうかで名前を変更
	if (isRuntime) {
		entity->SetName(name + "(Clone)");
	} else {
		entity->SetName(name);
	}

	entity->SetPrefabName(prefabName);

	/// Jsonからコンポーネントを生成
	EntityJsonConverter::FromJson(json, entity, pEcsGroup_->GetGroupName());

	/// 子エンティティの生成
	if (json.contains("children")) {
		for (auto& childJson : json["children"]) {
			GenerateEntityRecursive(childJson, entity, isRuntime);
		}
	}

	return entity;
}



void EntityCollection::SetMainCamera(CameraComponent* camera) {
	mainCamera_ = camera;
}

void EntityCollection::SetMainCamera2D(CameraComponent* camera) {
	mainCamera2D_ = camera;
}

CameraComponent* EntityCollection::GetMainCamera() {
	return mainCamera_;
}

CameraComponent* EntityCollection::GetMainCamera2D() {
	return mainCamera2D_;
}

const std::vector<std::unique_ptr<GameEntity>>& EntityCollection::GetEntities() const {
	return entities_;
}



