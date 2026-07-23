#pragma once

/// std
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <deque>

/// externals
#include <nlohmann/json_fwd.hpp>

#include "../GameEntity/GameEntity.h"
#include "../Prefab/EntityPrefab.h"

namespace ONEngine {

class CameraComponent;

/// ///////////////////////////////////////////////////
/// Entityのコレクションクラス
/// ///////////////////////////////////////////////////
class EntityCollection final {
private:
	/// =========================================
	/// private : sub classes
	/// =========================================

	/// @brief EntityIdの管理用コンテナ
	struct IdContainer {
		std::deque<int32_t> usedIds;    ///< 使用中のID
		std::deque<int32_t> removedIds; ///< 削除されたID
		int32_t nextId = 0;              ///< 未使用の新規IDカウンター
	};

public:
	/// =========================================
	/// public : methods
	/// =========================================

	EntityCollection(class ECSGroup* ecsGroup, class DxManager* dxm);
	~EntityCollection();

	/// 生成
	GameEntity* GenerateEntity(const Guid& guid, bool isRuntime = false);
	int32_t NewEntityID(bool isRuntime);

	/// 取得
	uint32_t GetEntityId(const std::string& name);
	GameEntity* GetEntity(int32_t entityId);
	GameEntity* GetEntityFromGuid(const Guid& guid);

	/// 削除
	void RemoveEntity(GameEntity* entity, bool deleteChildren = true);
	void RemoveEntityId(int32_t id);
	void RemoveEntityAll();

	/// 非破棄エンティティの追加と削除
	void AddDoNotDestroyEntity(GameEntity* entity);
	void RemoveDoNotDestroyEntity(GameEntity* entity);

	/// エンティティの順番を入れ替える
	void MoveEntity(GameEntity* entity, size_t newIndex);




	/* ----- prefab ----- */

	void LoadPrefabAll();
	void ReloadPrefab(const std::string& prefabName);

	GameEntity* GenerateEntityFromPrefab(const std::string& prefabName, bool isRuntime = true);
	EntityPrefab* GetPrefab(const std::string& fileName);

	/// prefabの内容をEntityに反映する
	void ApplyPrefabToEntity(GameEntity* entity, const std::string& prefabName);

private:


	/// @brief 再帰的にEntityを生成する
	GameEntity* GenerateEntityRecursive(const nlohmann::json& json, GameEntity* entity, bool isRuntime);


private:
	/// =========================================
	/// private : objects
	/// =========================================

	class ECSGroup* pEcsGroup_;
	class DxManager* pDxManager_;
	class DxDevice* pDxDevice_;

	/// entityのIDを管理するためのdeque
	IdContainer initEntityIDs_;
	IdContainer runtimeEntityIDs_;

	/// entityの本体を持つ配列
	std::vector<std::unique_ptr<GameEntity>> entities_;
	std::vector<GameEntity*> doNotDestroyEntities_;
	std::unordered_map<Guid, GameEntity*> guidEntityMap_;

	CameraComponent* mainCamera_ = nullptr;
	CameraComponent* mainCamera2D_ = nullptr;

	/// prefab
	std::unordered_map<std::string, std::unique_ptr<EntityPrefab>> prefabs_;

public:
	/// =========================================
	/// public : accessor
	/// =========================================

	void SetMainCamera(CameraComponent* cameraComponent);
	void SetMainCamera2D(CameraComponent* cameraComponent);

	CameraComponent* GetMainCamera();
	CameraComponent* GetMainCamera2D();

	const std::vector<std::unique_ptr<GameEntity>>& GetEntities() const;

};

} /// ONEngine
