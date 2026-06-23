#include "ScriptUpdateSystem.h"

using namespace ONEngine;

/// std
#include <list>
#include <chrono>

/// external
#include <mono/metadata/mono-gc.h>

/// engine
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Script/Script.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animation/AnimationPlayer.h"
#include "Engine/Script/MonoScriptEngine.h"
#include "Engine/Core/Utility/Time/CPUTimeStamp.h"

ScriptUpdateSystem::ScriptUpdateSystem(ECSGroup* ecs) {
	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
	MakeScriptMethod(monoEngine.Image(), ecs->GetGroupName());
}

ScriptUpdateSystem::~ScriptUpdateSystem() {
	ReleaseGCHandle();
}

void ScriptUpdateSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	/// ----- HotReloadをしたときにC#側がリセットされるのでスクリプトを追加し直す----- ///

	MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();

	if(monoEngine.GetIsHotReloadRequest()) {
		// 古いドメインのハンドルを解放
		ReleaseGCHandle();

		// 新しいドメインで再初期化
		MakeScriptMethod(monoEngine.Image(), ecs->GetGroupName());

		/// C#側のECSGroupを取得、更新関数を呼ぶ
		ComponentArray<Script>* scriptArray = ecs->GetComponentArray<Script>();
		if(!scriptArray || scriptArray->GetUsedComponents().empty()) {
			return;
		}

		for(auto& script : scriptArray->GetUsedComponents()) {
			script->SetIsAdded(false);
			for(auto& data : script->GetScriptDataList()) {
				data.isAdded = false;
				data.collisionEventMethods.fill(nullptr);
				data.collisionEventMethods2D.fill(nullptr);
			}
		}

		ComponentArray<AnimationPlayer>* animPlayerArray = ecs->GetComponentArray<AnimationPlayer>();
		if(animPlayerArray) {
			for(auto& animPlayer : animPlayerArray->GetUsedComponents()) {
				animPlayer->ClearBindings();
			}
		}

		ReleaseGCHandle();
		MakeScriptMethod(monoEngine.Image(), ecs->GetGroupName());
	}
}

void ScriptUpdateSystem::RuntimeUpdate(ECSGroup* ecs) {
#ifdef DEBUG_MODE
	CPUTimeStamp::GetInstance().BeginTimeStamp(CPUTimeStampID::CSharpScriptUpdate);
#endif // DEBUG_MODE

	/// C#側に未追加にエンティティとコンポーネントを追加する
	AddAllEntitiesAndComponents(ecs);

	/// 関数呼び出しの条件
	CallUpdateEcsGroup();


#ifdef DEBUG_MODE
	CPUTimeStamp::GetInstance().EndTimeStamp(CPUTimeStampID::CSharpScriptUpdate);
#endif // DEBUG_MODE
}

void ScriptUpdateSystem::AddAllEntitiesAndComponents(ECSGroup* ecsGroup) {
	/// スクリプトを持たないエンティティも追加することでC#で扱いやすくする
	for(auto& entity : ecsGroup->GetEntities()) {
		AddEntityToScript(entity.get());
	}
}

bool ScriptUpdateSystem::AddEntityToScript(GameEntity* entity) {
	/// runtime中に生成したオブジェクトは無視
	//if (entity->GetId() < 0) {
	//	return false;
	//}

	/// スクリプトが有効でない場合はスキップ
	MonoObject* ecsGroupObj = mono_gchandle_get_target(gcHandle_);
	if(!ecsGroupObj) {
		Console::LogError("Failed to get target from gcHandle_");
		return false;
	}

	/// --------------------------------------------------------------------------------
	/// Entityの追加関数を呼び出す
	/// --------------------------------------------------------------------------------
	void* addEntityArgs[1];
	int32_t entityId = entity->GetId();
	addEntityArgs[0] = &entityId;

	MonoObject* exc = nullptr;
	MonoScriptEngineUtils::SafeInvoke(addEntityMethod_, ecsGroupObj, addEntityArgs, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
	}

	Variables* vars = entity->GetComponent<Variables>();
	Script* script = entity->GetComponent<Script>();

	if(script) {
		/// --------------------------------------------------------------------------------
		/// スクリプトの追加
		/// --------------------------------------------------------------------------------
		for(auto& data : script->GetScriptDataList()) {

			/// すでに追加済みなら処理しない
			if(data.isAdded) {
				continue;
			}
			data.isAdded = true;

			/// スクリプト名からMonoObjectを生成する
			MonoScriptEngine& monoEngine = MonoScriptEngine::GetInstance();
			MonoClass* behaviorClass = mono_class_from_name(monoEngine.Image(), "", data.scriptName.c_str());
			if(!behaviorClass) {
				Console::LogError("Failed to find MonoBehavior class");
				continue;
			}

			/// インスタンスを生成
			MonoObject* scriptInstance = mono_object_new(mono_domain_get(), behaviorClass);
			mono_runtime_object_init(scriptInstance); /// クラスの初期化、コンストラクタをイメージ
			if(!script) {
				continue;
			}

			void* addScriptArgs[3];
			addScriptArgs[0] = &entityId;
			addScriptArgs[1] = scriptInstance;
			addScriptArgs[2] = &data.enable;

			exc = nullptr;
			MonoScriptEngineUtils::SafeInvoke(addScriptMethod_, ecsGroupObj, addScriptArgs, &exc);

			if(exc) {
				MonoScriptEngineUtils::HandleException(exc);
			}

			/// variablesの設定
			if(vars) {
				vars->SetScriptVariables(data.scriptName);
			}


		}
	}


	/// 正常に追加できた
	return true;
}

void ScriptUpdateSystem::CallUpdateEcsGroup() {

	/// 関数呼び出しの条件
	if(gcHandle_ != 0) {
		if(updateEntitiesMethod_) {

			/// 更新関数を呼び出す
			MonoObject* ecsGroupObj = mono_gchandle_get_target(gcHandle_);
			if(!ecsGroupObj) {
				Console::LogError("Failed to get target from gcHandle_");
				return;
			}

			MonoObject* exc = nullptr;
			MonoScriptEngineUtils::SafeInvoke(updateEntitiesMethod_, ecsGroupObj, nullptr, &exc);

			if(exc) {
				MonoScriptEngineUtils::HandleException(exc);
			}
		}
	}
}


void ScriptUpdateSystem::MakeScriptMethod(MonoImage* image, const std::string& ecsGroupName) {

	/// --------------------------------------------------------------------------------
	/// EntityComponentSystemの関数から新規にグループを追加する
	/// --------------------------------------------------------------------------------

	MonoClass* ecsClass = mono_class_from_name(image, "", "EntityComponentSystem");
	if(!ecsClass) {
		Console::LogError("Failed to find class: EntityComponentSystem");
		return;
	}

	/// EntityComponentSystemのAddECSGroup関数を取得
	MonoMethod* addGroupMethod = MonoScriptEngineUtils::FindMethodInClassOrParents(ecsClass, "AddECSGroup", 1);

	/// 関数の引数
	void* args[1];
	args[0] = mono_string_new(mono_domain_get(), ecsGroupName.c_str());; /// ECSのGroup名

	/// 関数を呼び出す
	MonoObject* exc = nullptr;
	MonoObject* ecsGroup = MonoScriptEngineUtils::SafeInvoke(addGroupMethod, nullptr, args, &exc);

	if(exc) {
		MonoScriptEngineUtils::HandleException(exc);
	}


	/// --------------------------------------------------------------------------------
	/// C#側のECSGroupクラスを取得
	/// --------------------------------------------------------------------------------

	/// C#側のクラスを取得
	monoClass_ = mono_object_get_class(ecsGroup);
	if(!monoClass_) {
		Console::LogError("Failed to find class: ECSGroup");
		return;
	}

	gcHandle_ = mono_gchandle_new(ecsGroup, false);

	/// 呼び出し対象の関数を取得
	updateEntitiesMethod_ = mono_class_get_method_from_name(monoClass_, "UpdateEntities", 0);
	addEntityMethod_ = mono_class_get_method_from_name(monoClass_, "AddEntity", 1);
	addScriptMethod_ = mono_class_get_method_from_name(monoClass_, "AddScript", 3);

}

void ScriptUpdateSystem::ReleaseGCHandle() {
	if(gcHandle_ != 0) {
		mono_gchandle_free(gcHandle_);
		gcHandle_ = 0;
	}
}


/// ///////////////////////////////////////////////
/// デバッグ用のスクリプト更新システム
/// ///////////////////////////////////////////////

DebugScriptUpdateSystem::DebugScriptUpdateSystem(ECSGroup* ecs)
	: ScriptUpdateSystem(ecs) {}
DebugScriptUpdateSystem::~DebugScriptUpdateSystem() {}

void DebugScriptUpdateSystem::OutsideOfRuntimeUpdate(ECSGroup* ecs) {
	/// 作成中のPrefabを更新してしまう問題を防ぐため、デバッグカメラのみ追加する

	ScriptUpdateSystem::OutsideOfRuntimeUpdate(ecs);
	CameraComponent* camera3d = ecs->GetMainCamera();
	if(camera3d) {
		if(GameEntity* debugCamera = camera3d->GetOwner()) {
			AddEntityToScript(debugCamera);
		}
	}


	CameraComponent* camera2d = ecs->GetMainCamera2D();
	if(camera2d) {
		if(GameEntity* debugCamera = camera2d->GetOwner()) {
			AddEntityToScript(debugCamera);
		}
	}

	/// 関数呼び出しの条件
	CallUpdateEcsGroup();
}

void DebugScriptUpdateSystem::RuntimeUpdate(ECSGroup* /*ecs*/) {}
