#include "ECSGroup.h"

using namespace ONEngine;

/// engine
#include "AddECSComponentFactoryFunction.h"
#include "Engine/ECS/System/ParticleSystemUpdateSystem/ParticleSystemUpdateSystem.h"
#include "Engine/ECS/System/ParticleSystem2DUpdateSystem/ParticleSystem2DUpdateSystem.h"
#include "Engine/Script/MonoScriptEngine.h"

ECSGroup::ECSGroup(DxManager* dxm) {
	/// インスタンスの生成
	entityCollection_ = std::make_unique<EntityCollection>(this, dxm);
	componentCollection_ = std::make_unique<ComponentCollection>();
	systemCollection_ = std::make_unique<SystemCollection>();
}

ECSGroup::~ECSGroup() {}

void ECSGroup::Initialize(const std::string& groupName) {
	/// このECSGroupの名前を決める
	groupName_ = groupName;

	AddComponentFactoryFunction(componentCollection_.get());
}

void ECSGroup::Update() {}

GameEntity* ECSGroup::GenerateEntity(const Guid& guid, bool isRuntime) {
	return entityCollection_->GenerateEntity(guid, isRuntime);
}

GameEntity* ECSGroup::GenerateEntityFromPrefab(const std::string& prefabName, bool isRuntime) {
	return entityCollection_->GenerateEntityFromPrefab(prefabName, isRuntime);
}


GameEntity* ECSGroup::GetEntityFromGuid(const Guid& guid) {
	/// 例外チェック(無効値なら nullptr を返す)
	if (!guid.CheckValid()) {
		Console::LogError("ECSGroup::GetEntityFromGuid: Invalid Guid provided.");
		return nullptr;
	}

	const auto& entities = entityCollection_->GetEntities();
	for (const auto& entity : entities) {
		if (entity->GetGuid() == guid) {
			return entity.get();
		}
	}

	return nullptr;
}

void ECSGroup::RemoveEntity(GameEntity* entity, bool deleteChildren) {
	/// 例外チェック
	if (entity == nullptr) {
		Console::LogError("ECSGroup::RemoveEntity: Null entity provided.");
		return;
	}

	MonoScriptEngine::GetInstance().RemoveEntityFromNativeCS(groupName_, entity->GetId());
	entityCollection_->RemoveEntity(entity, deleteChildren);
}

void ECSGroup::RemoveEntityAll() {
	entityCollection_->RemoveEntityAll();

	// ゴーストパーティクルをクリア
	if (auto* psUpdateSys = GetSystem<ParticleSystemUpdateSystem>()) {
		psUpdateSys->ClearGhosts();
	}
	if (auto* ps2DUpdateSys = GetSystem<ParticleSystem2DUpdateSystem>()) {
		ps2DUpdateSys->ClearGhosts();
	}
}

void ECSGroup::AddDoNotDestroyEntity(GameEntity* entity) {
	if (entity == nullptr) {
		Console::LogError("ECSGroup::AddDoNotDestroyEntity: Null entity provided.");
		return;
	}

	entityCollection_->AddDoNotDestroyEntity(entity);
}

void ECSGroup::RemoveDoNotDestroyEntity(GameEntity* entity) {
	if (entity == nullptr) {
		Console::LogError("ECSGroup::RemoveDoNotDestroyEntity: Null entity provided.");
		return;
	}

	entityCollection_->RemoveDoNotDestroyEntity(entity);
}

uint32_t ECSGroup::GetEntityId(const std::string& name) {
	return entityCollection_->GetEntityId(name);
}

uint32_t ECSGroup::CountEntity(const std::string& name) {
	const auto& entities = entityCollection_->GetEntities();
	return static_cast<uint32_t>(std::count_if(entities.begin(), entities.end(),
		[&name](const std::unique_ptr<GameEntity>& entity) {
			std::string entityName = entity->GetName();
			/// 後ろから"_"を検索、"_"を含む場合はその前までを比較する
			if (entityName.find_last_of('_') != std::string::npos) {
				return entityName.substr(0, entityName.find_last_of('_')) == name;
			}

			return entityName == name;
		}
	));
}

IComponent* ECSGroup::AddComponent(const std::string& compName) {
	return componentCollection_->AddComponent(compName);
}

void ECSGroup::RemoveComponent(size_t hash, uint32_t compId) {
	componentCollection_->RemoveComponent(hash, compId);
}

void ECSGroup::RemoveComponentAll(GameEntity* entity) {
	if (entity == nullptr) {
		return;
	}

	componentCollection_->RemoveComponentAll(entity);
}

void ECSGroup::LoadComponent(GameEntity* entity) {
	componentInputCommand_.SetEntity(entity);
	componentInputCommand_.Execute();
}

void ECSGroup::OutsideOfRuntimeUpdateSystems() {
	systemCollection_->OutsideOfRuntimeUpdate(this);
}

void ECSGroup::RuntimeUpdateSystems() {
	systemCollection_->RuntimeUpdate(this);
}

void ECSGroup::SetMainCamera(CameraComponent* camera) {
	entityCollection_->SetMainCamera(camera);
}

void ECSGroup::SetMainCamera2D(CameraComponent* camera) {
	entityCollection_->SetMainCamera2D(camera);
}

EntityCollection* ECSGroup::GetEntityCollection() {
	return entityCollection_.get();
}

const std::vector<std::unique_ptr<GameEntity>>& ECSGroup::GetEntities() const {
	return entityCollection_->GetEntities();
}

GameEntity* ECSGroup::GetEntity(int32_t id) const {
	return entityCollection_->GetEntity(id);
}

const CameraComponent* ECSGroup::GetMainCamera() const {
	return entityCollection_->GetMainCamera();
}

CameraComponent* ECSGroup::GetMainCamera() {
	return entityCollection_->GetMainCamera();
}

const CameraComponent* ECSGroup::GetMainCamera2D() const {
	return entityCollection_->GetMainCamera2D();
}

CameraComponent* ECSGroup::GetMainCamera2D() {
	return entityCollection_->GetMainCamera2D();
}

const std::string& ECSGroup::GetGroupName() const {
	return groupName_;
}

void ECSGroup::SetUpdatePaused(bool paused) {
	isUpdatePaused_ = paused;
}

bool ECSGroup::IsUpdatePaused() const {
	return isUpdatePaused_;
}

void ECSGroup::SetDrawPaused(bool paused) {
	isDrawPaused_ = paused;
}

bool ECSGroup::IsDrawPaused() const {
	return isDrawPaused_;
}

void ECSGroup::SetHasClearColor(bool hasClearColor) {
	hasClearColor_ = hasClearColor;
}

void ECSGroup::SetClearColor(const Vector4& clearColor) {
	clearColor_ = clearColor;
}

bool ECSGroup::HasClearColor() const {
	return hasClearColor_;
}

const Vector4& ECSGroup::GetClearColor() const {
	return clearColor_;
}
