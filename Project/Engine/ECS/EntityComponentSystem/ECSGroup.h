#pragma once

/// engine
#include "../Entity/Collection/EntityCollection.h"
#include "../Component/Collection/ComponentCollection.h"
#include "../System/SystemCollection/SystemCollection.h"

#include "Engine/Editor/Commands/ComponentEditCommands/ComponentEditCommands.h"

namespace ONEngine {

template<typename T>
concept SystemType = std::is_base_of_v<ECSISystem, T>;

/// ///////////////////////////////////////////////////
/// ECSのコレクションのグループ
/// ///////////////////////////////////////////////////
class ECSGroup {
public:
	/// ====================================================
	/// public : methods
	/// ====================================================

	ECSGroup(class DxManager* dxm);
	~ECSGroup();

	void Initialize(const std::string& groupName);
	void Update();


	/// ----- entity ----- ///

	/// 生成
	GameEntity* GenerateEntity(const Guid& guid, bool isRuntime);
	GameEntity* GenerateEntityFromPrefab(const std::string& prefabName, bool isRuntime = true);

	/// 取得
	GameEntity* GetEntityFromGuid(const Guid& guid);

	/// 削除
	void RemoveEntity(GameEntity* entity, bool deleteChildren = true);
	void RemoveEntityAll();

	/// 非破棄エンティティの追加と削除
	void AddDoNotDestroyEntity(GameEntity* entity);
	void RemoveDoNotDestroyEntity(GameEntity* entity);

	/// Id
	uint32_t GetEntityId(const std::string& name);

	/// nameのEntityが何体いるのかチェックする
	uint32_t CountEntity(const std::string& name);


	/// ----- component ----- ///

	/// 生成
	template<IsComponent Comp>
	Comp* AddComponent();
	IComponent* AddComponent(const std::string& compName);

	/// 取得
	template<IsComponent Comp>
	Comp* GetComponent(size_t entityId);
	template<IsComponent Comp>
	ComponentArray<Comp>* GetComponentArray();

	/// 削除
	template<IsComponent Comp>
	void RemoveComponent(uint32_t compId);
	void RemoveComponent(size_t hash, uint32_t compId);
	void RemoveComponentAll(GameEntity* entity);

	/// 読み込み
	void LoadComponent(GameEntity* entity);


	/// ----- system ----- ///

	/// 追加
	template<SystemType Sys, typename... Args>
	void AddSystem(Args... args);

	/// 更新
	void OutsideOfRuntimeUpdateSystems();
	void RuntimeUpdateSystems();

	/// 取得
	template<SystemType Sys>
	Sys* GetSystem() {
		return systemCollection_->GetSystem<Sys>();
	}


private:
	/// ===================================================
	/// public : objects
	/// ===================================================

	/// ----- parameters ----- ///
	std::string groupName_;
	bool isUpdatePaused_ = false;
	bool isDrawPaused_ = false;

	/// ----- collections ----- ///
	std::unique_ptr<EntityCollection> entityCollection_;
	std::unique_ptr<ComponentCollection> componentCollection_;
	std::unique_ptr<SystemCollection> systemCollection_;

	/// ----- command ----- ///
	Editor::EntityDataInputCommand componentInputCommand_;


public:
	/// ===================================================
	/// public : accessors
	/// ===================================================

	/// ----- setter ----- ///

	void SetMainCamera(CameraComponent* camera);
	void SetMainCamera2D(CameraComponent* camera);

	void SetUpdatePaused(bool paused);
	void SetDrawPaused(bool paused);


	/// ----- getter ----- ///

	EntityCollection* GetEntityCollection();
	const std::vector<std::unique_ptr<GameEntity>>& GetEntities() const;

	GameEntity* GetEntity(int32_t id) const;

	const CameraComponent* GetMainCamera() const;
	CameraComponent* GetMainCamera();
	const CameraComponent* GetMainCamera2D() const;
	CameraComponent* GetMainCamera2D();

	const std::string& GetGroupName() const;

	bool IsUpdatePaused() const;
	bool IsDrawPaused() const;
};

/// ===================================================
/// inline methods
/// ===================================================

template<IsComponent Comp>
inline Comp* ECSGroup::AddComponent() {
	return componentCollection_->AddComponent<Comp>();
}

template<IsComponent Comp>
inline Comp* ECSGroup::GetComponent(size_t entityId) {
	GameEntity* entity = entityCollection_->GetEntity(entityId);
	if (entity) {
		return entity->GetComponent<Comp>();
	}

	return nullptr;
}

template<IsComponent Comp>
inline ComponentArray<Comp>* ECSGroup::GetComponentArray() {
	return componentCollection_->GetComponentArray<Comp>();
}

template<IsComponent Comp>
inline void ECSGroup::RemoveComponent(uint32_t compId) {
	componentCollection_->RemoveComponent<Comp>(compId);
}

template<SystemType Sys, typename ...Args>
inline void ECSGroup::AddSystem(Args ...args) {
	systemCollection_->AddSystem(std::make_unique<Sys>(args...));
}

} /// ONEngine
