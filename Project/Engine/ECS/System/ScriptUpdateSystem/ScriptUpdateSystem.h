#pragma once

/// std
#include <list>
#include <string>

/// externals
#include <mono/jit/jit.h>


/// engine
#include "../Interface/ECSISystem.h"

/// /////////////////////////////////////////////////
/// scriptの更新を行うシステム
/// /////////////////////////////////////////////////
namespace ONEngine {

class ScriptUpdateSystem : public ECSISystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	ScriptUpdateSystem(class ECSGroup* ecs);
	~ScriptUpdateSystem() override;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

	/// エンティティとコンポーネントをC#に追加
	void AddAllEntitiesAndComponents(class ECSGroup* ecsGroup);
	bool AddEntityToScript(class GameEntity* entity);

	/// @brief EcsGroupの更新を呼び出す
	void CallUpdateEcsGroup();

	/// 生成
	void MakeScriptMethod(MonoImage* image, const std::string& ecsGroupName);

	/// 解放
	void ReleaseGCHandle();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::string ecsGroupName_{};
	MonoClass*  monoClass_ = nullptr;
	uint32_t    gcHandle_ = 0;
	MonoMethod* updateEntitiesMethod_ = nullptr;
	MonoMethod* addEntityMethod_ = nullptr;
	MonoMethod* addScriptMethod_ = nullptr;
	int32_t     lastReloadCounter_ = 0;

};



/// /////////////////////////////////////////////////
/// デバッグ用のスクリプト更新システム
/// /////////////////////////////////////////////////
class DebugScriptUpdateSystem : public ScriptUpdateSystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	DebugScriptUpdateSystem(class ECSGroup* ecs);
	~DebugScriptUpdateSystem() override;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;
};

} /// ONEngine
