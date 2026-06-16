#pragma once

/// std
#include <unordered_map>
#include <memory>
#include <string>

/// externals
#include <jit/jit.h>

/// engine
#include "ECSGroup.h"
#include "../Entity/Collection/EntityCollection.h"
#include "../Entity/Prefab/EntityPrefabCollection.h"
#include "../Component/Collection/ComponentCollection.h"
#include "../System/SystemCollection/SystemCollection.h"

#include "Engine/Editor/Commands/ComponentEditCommands/ComponentEditCommands.h"
#include <Engine/ECS/Component/Array/ComponentArray.h>

namespace ONEngine {
class DxManager;
class DxDevice;
class CameraComponent;
}

namespace ONEngine::Asset {
class AssetCollection;
}


namespace ONEngine {

void SetEntityComponentSystemPtr(ECSGroup* gameGroup, ECSGroup* debugGroup);
ECSGroup* GetEntityComponentSystemPtr();

/// ///////////////////////////////////////////////////
/// ECSの基盤クラス
/// ///////////////////////////////////////////////////
class EntityComponentSystem final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	EntityComponentSystem(DxManager* pDxManager);
	~EntityComponentSystem();

	void Initialize(Asset::AssetCollection* assetCollection);
	void Update();
	void OutsideOfUpdate();

	void DebuggingUpdate();

	/// ----- group  ----- ///

	/// 追加
	ECSGroup* AddECSGroup(const std::string& name);

	/// 取得
	ECSGroup* GetECSGroup(const std::string& name) const;
	ECSGroup* GetCurrentGroup() const;

	/// main cameraの設定
	void MainCameraSetting();

	/// 現在のグループ
	void SetCurrentGroupName(const std::string& name);
	const std::string& GetCurrentGroupName() const;

	DxManager* GetDxManager() const { return pDxManager_; }

	/// すべてのECSグループの取得
	const std::unordered_map<std::string, std::unique_ptr<ECSGroup>>& GetECSGroups() const;

	/// ----- prefab ----- ///

	void ReloadPrefab(const std::string& prefabName);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	/// ----- other objects ----- ///
	Asset::AssetCollection* pAssetCollection_;
	DxManager* pDxManager_;
	DxDevice* pDxDevice_;

	/// ----- groups ----- ///
	std::unordered_map<std::string, std::unique_ptr<ECSGroup>> ecsGroups_;
	ECSGroup* debugGroup_ = nullptr;
	std::string currentGroupName_;

};


/// =============================================
/// お助け関数群
/// =============================================

/// @brief 親のエンティティが有効かどうかを再帰的にチェックする
/// @param entity 対象のエンティティ
bool CheckParentEntityEnable(GameEntity* entity);

/// @brief このコンポーネントが有効かどうかをチェックする
/// @param component 対象のコンポーネント
/// @return true: 有効, false: 無効
bool CheckComponentEnable(IComponent* component);

/// @brief ComponentArray<T>が有効かどうかをチェックする
/// @param componentArray 
/// @return true: 有効, false: 無効
bool CheckComponentArrayEnable(IComponentArray* componentArray);



/// =============================================
/// monoを使ったC#スクリプトエンジンのコンポーネント
/// =============================================

namespace MonoInternalMethods {

/// エンティティのidからEntityを取得
GameEntity* GetEntityById(int32_t entityId, const std::string& groupName);

/// @brief Componentの追加
/// @param entityId 対象エンティティ
/// @param monoTypeName Componentの型名
/// @param groupName ECSGroupの名前
/// @return 追加したComponentのポインタの整数
uint64_t InternalAddComponent(int32_t entityId, MonoString* monoTypeName, MonoString* groupName, uint32_t* compId);

/// @brief Componentの取得
/// @param entityId 対象のエンティティID
/// @param monoTypeName Componentの型名
/// @param groupName ECSGroupの名前
/// @return ゲットしたComponentのポインタの整数
uint64_t InternalGetComponent(int32_t entityId, MonoString* monoTypeName, MonoString* groupName, uint32_t* compId);

/// @brief エンティティの名前の取得
/// @param entityId 対象のエンティティID
/// @param groupName ECSGroupの名前
/// @return 取得した名前の文字列ポインタ
const char* InternalGetName(int32_t entityId, MonoString* groupName);

/// @brief エンティティの命名
/// @param entityId 対象のエンティティID
/// @param name 新規の名前
/// @param groupName ECSGroupの名前
void InternalSetName(int32_t entityId, MonoString* name, MonoString* groupName);

/// @brief エンティティの子のIDを取得
/// @param entityId 対象のエンティティID
/// @param childIndex 子のインデックス
/// @param groupName ECSGroupの名前
/// @return 見つかった子エンティティのID
int32_t InternalGetChildId(int32_t entityId, uint32_t childIndex, MonoString* groupName);

/// @brief 子エンティティの数を取得する
/// @param entityId 親エンティティID
/// @param groupName ECSGroupの名前
/// @return 見つかった子エンティティの数
int32_t InternalGetChildrenCount(int32_t entityId, MonoString* groupName);

/// @brief エンティティの親のIDを取得
/// @param entityId 対象のエンティティID
/// @param groupName ECSGroupの名前
/// @return 見つかった親エンティティのID
int32_t InternalGetParentId(int32_t entityId, MonoString* groupName);

/// @brief エンティティの親の設定
/// @param entityId エンティティID
/// @param parentId 親エンティティID
/// @param groupName ECSGroupの名前
void InternalSetParent(int32_t entityId, int32_t parentId, MonoString* groupName);

/// @brief C#スクリプトの追加
/// @param entityId 対象のエンティティID
/// @param scriptName 追加するスクリプト名
/// @param groupName ECSGroupの名前
void InternalAddScript(int32_t entityId, MonoString* scriptName, MonoString* groupName);

/// @brief C#スクリプトの取得
/// @param entityId 対象のエンティティID
/// @param scriptName ゲットするスクリプト名
/// @param groupName ECSGroupの名前
/// @return 見つかったかどうか
bool InternalGetScript(int32_t entityId, MonoString* scriptName, MonoString* groupName);

/// @brief エンティティの生成
/// @param entityId 作成されたエンティティIDのポインタ
/// @param prefabName 作成するPrefab名
/// @param groupName ECSGroupの名前
void InternalCreateEntity(int32_t* entityId, MonoString* prefabName, MonoString* groupName);

/// @brief エンティティの削除
/// @param ecsGroupName ECSGroupの名前
/// @param entityId 対象のエンティティID
void InternalDestroyEntity(MonoString* ecsGroupName, int32_t entityId);

int32_t InternalGetRootEntityCount(MonoString* groupName);
int32_t InternalGetRootEntityId(MonoString* groupName, int32_t index);


/// @brief エンティティの有効/無効の取得
/// @param entityId 対象のエンティティ
/// @param ecsGroupName ECSGroupの名前
/// @return true: 有効, false: 無効
bool InternalGetEnable(int32_t entityId, MonoString* ecsGroupName);

/// @brief エンティティの有効/無効の設定
/// @param entityId 対象のエンティティID
/// @param enable 設定する値
/// @param ecsGroupName ECSGroupの名前
void InternalSetEnable(int32_t entityId, bool enable, MonoString* ecsGroupName);


void InternalSetBatch(MonoReflectionType* typeReflection, MonoArray* batchArray, int count, MonoString* ecsGroupName);
void InternalGetBatch(MonoReflectionType* typeReflection, MonoArray* batchArray, int count, MonoString* ecsGroupName);

void Internal_UpdateNodeStatus(uint32_t nodeIdHash, int status, MonoString* treePath);
void Internal_UpdateBlackboardValue(uint32_t keyHash, MonoString* value, MonoString* typeName);
void Internal_OnBreakpointHit(uint32_t nodeIdHash);

} // namespace MonoInternalMethods

} /// ONEngine
