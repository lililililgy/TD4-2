#pragma once

/// std
#include <string>

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "Engine/Asset/Guid/Guid.h"

/// editor
#include "../IEditCommand.h"

namespace ONEngine {
class GameEntity;
class ECSGroup;
}


/// ///////////////////////////////////////////////////
/// ゲームオブジェクトの作成コマンド
/// ///////////////////////////////////////////////////
namespace Editor {

class CreateGameObjectCommand : public IEditCommand {
public:
	CreateGameObjectCommand(ONEngine::ECSGroup* ecs, const std::string& name = "NewEntity", ONEngine::GameEntity* parentEntity = nullptr);
	~CreateGameObjectCommand();

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_ = nullptr;
	ONEngine::GameEntity* generatedEntity_ = nullptr;
	ONEngine::Guid generatedGuid_;
	ONEngine::Guid parentGuid_;
	const std::string entityName_;
};


/// ///////////////////////////////////////////////////
/// プリミティブなオブジェクトの作成コマンド
/// ///////////////////////////////////////////////////
class CreatePrimitiveCommand : public IEditCommand {
public:
	enum class Type {
		Camera,
		DirectionalLight,
		Mesh,
		Sprite,
	};

	CreatePrimitiveCommand(ONEngine::ECSGroup* ecs, Type type, ONEngine::GameEntity* parentEntity = nullptr);
	~CreatePrimitiveCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_ = nullptr;
	ONEngine::GameEntity* generatedEntity_ = nullptr;
	ONEngine::Guid generatedGuid_;
	ONEngine::Guid parentGuid_;
	Type type_;
};


/// ///////////////////////////////////////////////////
/// シーンに配置してあるオブジェクトの名前をへんこうする 
/// ///////////////////////////////////////////////////
class EntityRenameCommand : public IEditCommand {
public:
	EntityRenameCommand(ONEngine::GameEntity* entity, const std::string& newName);
	~EntityRenameCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::GameEntity* pEntity_;
	std::string oldName_ = "";
	std::string newName_ = "";
};


/// ///////////////////////////////////////////////////
/// シーンにあるオブジェクトから新しいクラスを作る
/// ///////////////////////////////////////////////////
class CreateNewEntityClassCommand : public IEditCommand {
public:
	CreateNewEntityClassCommand(ONEngine::GameEntity* entity, const std::string& outputFilePath);
	~CreateNewEntityClassCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

	EDITOR_STATE CreateNewClassFile(const std::string& srcFilePath, const std::string& outputFileName, const std::string& newClassName);

private:
	ONEngine::GameEntity* pEntity_ = nullptr;

	std::string sourceClassPath_;
	std::string sourceClassName_;
	std::string outputFilePath_;
};


/// ///////////////////////////////////////////////////
/// プレハブを作成するコマンド
/// ///////////////////////////////////////////////////
class CreatePrefabCommand : public IEditCommand {
public:
	CreatePrefabCommand(ONEngine::GameEntity* entity);
	~CreatePrefabCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

	/// @brief 再帰的にエンティティをシリアライズする
	void SerializeRecursive(ONEngine::GameEntity* entity, nlohmann::json& json);

private:
	ONEngine::GameEntity* pEntity_ = nullptr;
	std::string prefabPath_ = "./Assets/Prefabs/";
	std::string prefabName_ = "NewPrefab.json";
};


/// ///////////////////////////////////////////////////
/// エンティティを削除するコマンド
/// ///////////////////////////////////////////////////
class DeleteEntityCommand : public IEditCommand {
public:
	DeleteEntityCommand(ONEngine::ECSGroup* ecs, ONEngine::GameEntity* entity);
	~DeleteEntityCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_;
	ONEngine::GameEntity* pEntity_;
};


/// ///////////////////////////////////////////////////
/// エンティティをコピーするコマンド
/// ///////////////////////////////////////////////////
class CopyEntityCommand : public IEditCommand {
public:
	CopyEntityCommand(ONEngine::GameEntity* entity);
	~CopyEntityCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;
private:
	ONEngine::GameEntity* pEntity_;
	nlohmann::json entityJson_;
};


/// ///////////////////////////////////////////////////
/// エンティティをペーストするコマンド
/// ///////////////////////////////////////////////////
class PasteEntityCommand : public IEditCommand {
public:
	PasteEntityCommand(ONEngine::ECSGroup* ecs, ONEngine::GameEntity* selectedEntity);
	~PasteEntityCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_;
	ONEngine::GameEntity* pSelectedEntity_ = nullptr;
	ONEngine::GameEntity* pastedEntity_ = nullptr;
};

/// ///////////////////////////////////////////////////
/// エンティティの親子付けを変更するコマンド
/// ///////////////////////////////////////////////////
class ChangeEntityParentCommand : public IEditCommand {
public:
	ChangeEntityParentCommand(ONEngine::GameEntity* entity, ONEngine::GameEntity* newParent);
	~ChangeEntityParentCommand() = default;
	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;
private:
	ONEngine::GameEntity* pEntity_ = nullptr;
	ONEngine::GameEntity* pNewParent_ = nullptr;
	ONEngine::GameEntity* pOldParent_ = nullptr;
};

/// ///////////////////////////////////////////////////
/// エンティティの順番を入れ替えるコマンド
/// ///////////////////////////////////////////////////
class ReorderEntityCommand : public IEditCommand {
public:
	ReorderEntityCommand(ONEngine::ECSGroup* ecsGroup, ONEngine::GameEntity* entity, ONEngine::GameEntity* newParent, uint32_t newIndex);
	~ReorderEntityCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_;
	ONEngine::GameEntity* pEntity_;
	ONEngine::GameEntity* pNewParent_;
	ONEngine::GameEntity* pOldParent_;
	uint32_t newIndex_;
	uint32_t oldIndex_;
};

/// ///////////////////////////////////////////////////
/// プレハブからインスタンスを作成するコマンド
/// ///////////////////////////////////////////////////
class InstantiatePrefabCommand : public IEditCommand {
public:
	InstantiatePrefabCommand(ONEngine::ECSGroup* ecs, const std::string& prefabPath, ONEngine::GameEntity* parentEntity = nullptr);
	~InstantiatePrefabCommand() = default;

	EDITOR_STATE Execute() override;
	EDITOR_STATE Undo() override;

private:
	ONEngine::ECSGroup* pEcsGroup_ = nullptr;
	ONEngine::GameEntity* generatedEntity_ = nullptr;
	ONEngine::Guid generatedGuid_;
	ONEngine::Guid parentGuid_;
	const std::string prefabPath_;
};

} /// Editor
