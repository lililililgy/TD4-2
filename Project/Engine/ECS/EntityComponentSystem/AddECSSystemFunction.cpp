#include "AddECSSystemFunction.h"


/// engine
#include "ECSGroup.h"
#include "Engine/Core/DirectX12/Manager/DxManager.h"

/// systems
#include "../System/Animator/AnimatorUpdateSystem.h"
#include "../System/Audio/AudioPlaybackSystem.h"
#include "../System/MeshBufferRecreate/MeshBufferRecreate.h"
#include "../System/EffectUpdateSystem/EffectUpdateSystem.h"
#include "../System/ParticleSystemUpdateSystem/ParticleSystemUpdateSystem.h"
#include "../System/ParticleSystem2DUpdateSystem/ParticleSystem2DUpdateSystem.h"
#include "../System/ScriptUpdateSystem/ScriptUpdateSystem.h"
#include "../System/Collision/CollisionSystem.h"
#include "../System/Collision/Collision2DSystem.h"
#include "../System/Collision/ColliderRenderQueueSystem.h"
#include "../System/SkinMesh/SkinMeshUpdateSystem.h"
#include "../System/CameraUpdate/CameraUpdateSystem.h"
#include "../System/Terrain/TerrainColliderVertexGenerator.h"
#include "../System/Terrain/TerrainCollision.h"
#include "../System/Transform/TransformUpdateSystem.h"
#include "../System/ShadowCaster/ShadowCasterUpdateSystem.h"
#include "../System/GrassBufferCreateSystem/GrassBufferCreateSystem.h"
#include "../System/AI/AISystem.h"
#include "../System/Movement/MovementSystem.h"
#include "../System/Animation/AnimationSystem.h"
#include "../System/UI/UIHierarchySystem.h"
#include "../System/UI/UIInputNavigationSystem.h"

using namespace ONEngine;

/// ---------------------------------------------------
/// ゲームように使用するシステム追加関数
/// ---------------------------------------------------
void ONEngine::GameECSGroupAddSystemFunction(ECSGroup* ecs, DxManager* dxm, Asset::AssetCollection* assetCollection) {

	/// 初期化に使うsystem
	ecs->AddSystem<TerrainColliderVertexGenerator>(dxm);
	ecs->AddSystem<GrassBufferCreateSystem>(dxm);

	/// 更新に使うsystem
	ecs->AddSystem<TransformUpdateSystem>();
	ecs->AddSystem<AnimationSystem>();
	ecs->AddSystem<AnimatorUpdateSystem>();
	ecs->AddSystem<SkinMeshUpdateSystem>(dxm, assetCollection);
	ecs->AddSystem<ScriptUpdateSystem>(ecs);
	ecs->AddSystem<UIHierarchySystem>();
	ecs->AddSystem<UIInputNavigationSystem>();
	ecs->AddSystem<AISystem>();
	ecs->AddSystem<MovementSystem>();
	ecs->AddSystem<AudioPlaybackSystem>(assetCollection);
	ecs->AddSystem<EffectUpdateSystem>();
	ecs->AddSystem<ParticleSystemUpdateSystem>();
	ecs->AddSystem<ParticleSystem2DUpdateSystem>();
	ecs->AddSystem<TransformUpdateSystem>();

	/// 衝突判定に使うsystem
	ecs->AddSystem<TerrainCollision>();
	ecs->AddSystem<CollisionSystem>();
	ecs->AddSystem<Collision2DSystem>();
	ecs->AddSystem<TransformUpdateSystem>();

	ecs->AddSystem<CameraUpdateSystem>(dxm->GetDxDevice());
	ecs->AddSystem<ShadowCasterUpdateSystem>();


	/// 描画に使うsystem
	ecs->AddSystem<MeshBufferRecreate>(dxm->GetDxDevice());
	ecs->AddSystem<ColliderRenderQueueSystem>();
}


/// ---------------------------------------------------
/// DebugGroup用のシステム追加関数 (Debugでしか用いないシステムをここに追加する)
/// ---------------------------------------------------
void ONEngine::DebugECSGroupAddSystemFunction(ECSGroup* ecs, DxManager* dxm, Asset::AssetCollection* assetCollection) {

	/// 初期化に使うsystem
	ecs->AddSystem<TerrainColliderVertexGenerator>(dxm);
	ecs->AddSystem<GrassBufferCreateSystem>(dxm);

	/// 更新に使うsystem
	ecs->AddSystem<AnimationSystem>();
	ecs->AddSystem<CameraUpdateSystem>(dxm->GetDxDevice());
	ecs->AddSystem<AnimatorUpdateSystem>();
	ecs->AddSystem<SkinMeshUpdateSystem>(dxm, assetCollection);
	ecs->AddSystem<DebugScriptUpdateSystem>(ecs);
	ecs->AddSystem<UIHierarchySystem>();
	ecs->AddSystem<UIInputNavigationSystem>();
	ecs->AddSystem<AISystem>();
	ecs->AddSystem<MovementSystem>();
	ecs->AddSystem<AudioPlaybackSystem>(assetCollection);
	ecs->AddSystem<EffectUpdateSystem>();
	ecs->AddSystem<ParticleSystemUpdateSystem>();
	ecs->AddSystem<ParticleSystem2DUpdateSystem>();
	ecs->AddSystem<TransformUpdateSystem>();
	ecs->AddSystem<ShadowCasterUpdateSystem>();

	/// 衝突判定に使うsystem
	ecs->AddSystem<TerrainCollision>();
	ecs->AddSystem<CollisionSystem>();
	ecs->AddSystem<Collision2DSystem>();
	ecs->AddSystem<TransformUpdateSystem>();

	/// 描画に使うsystem
	ecs->AddSystem<MeshBufferRecreate>(dxm->GetDxDevice());
	ecs->AddSystem<ColliderRenderQueueSystem>();
}
