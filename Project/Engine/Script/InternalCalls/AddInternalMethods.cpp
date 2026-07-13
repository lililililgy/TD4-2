#include "AddInternalMethods.h"

/// externals
#include <mono/jit/jit.h>

/// engine
#include "Engine/Core/Utility/Input/Input.h"
#include "Engine/Core/Utility/Input/InputSystem.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/Scene/SceneManager.h"

#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/Terrain.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/BGMPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Audio/SEPlayer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Effect/Effect.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Terrain/TerrainCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/SphereCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/CircleCollider.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/ECS/Component/Components/RendererComponents/Skybox/Skybox.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/CustomMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/SkinMesh/SkinMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line2DRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Primitive/Line3DRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/ScreenPostEffectTag/ScreenPostEffectTag.h"
#include "Engine/ECS/Component/Components/RendererComponents/Text/TextRenderer.h"
#include "Engine/Core/Utility/Font/FontRasterizer.h"
#include <mono/metadata/object.h>
#include <mono/metadata/appdomain.h>
#include "Engine/Core/Window/WindowManager.h"
#include "Engine/Core/Utility/Tools/Log.h"


#include "Engine/Core/Utility/Tools/Gizmo.h"
#include <mutex>

namespace {
	std::mutex g_GizmoMutex;

	void Internal_SubmitLineBatch(MonoArray* batch, int count) {
		if (!batch) return;

		// 検証用ログ (毎フレーム出ると多すぎるため、最初の10回だけ詳細を出力)
		static int receiveLogCount = 0;
		if (receiveLogCount < 10) {
			std::string msg = "[C++ Bridge] Internal_SubmitLineBatch received " + std::to_string(count) + " items.";
			
			// 最初の1要素の座標と太さをサンプルとして出力
			if (count > 0) {
				ONEngine::Gizmo::LineData* sample = (ONEngine::Gizmo::LineData*)mono_array_addr(batch, ONEngine::Gizmo::LineData, 0);
				if (sample) {
					msg += " Sample[0]: Start(" + std::to_string(sample->startPosition.x) + "," + std::to_string(sample->startPosition.y) + "," + std::to_string(sample->startPosition.z) + ")";
					msg += " End(" + std::to_string(sample->endPosition.x) + "," + std::to_string(sample->endPosition.y) + "," + std::to_string(sample->endPosition.z) + ")";
					msg += " Thickness: " + std::to_string(sample->thickness);
				}
			}
			ONEngine::Console::Log(msg, ONEngine::LogCategory::ScriptEngine);
			receiveLogCount++;
		}
		
		std::lock_guard<std::mutex> lock(g_GizmoMutex);
		for (int i = 0; i < count; ++i) {
			ONEngine::Gizmo::LineData* data = (ONEngine::Gizmo::LineData*)mono_array_addr(batch, ONEngine::Gizmo::LineData, i);
			if (!data) continue;

			// エンジン側のGizmo::DrawLineを呼び出す
			// 注意: Gizmo::DrawLineは内部でgGizmoSystemを使用するため、初期化済みである必要がある
			ONEngine::Gizmo::DrawLine(data->startPosition, data->endPosition, data->color, data->thickness);
		}
	}

	bool InternalGenerateFontTexture(MonoString* text, MonoString* fontAssetPath, int fontSize, MonoString* texturePath) {
		if (!text || !fontAssetPath || !texturePath) return false;

		char* textStr = mono_string_to_utf8(text);
		char* fontPathStr = mono_string_to_utf8(fontAssetPath);
		char* texPathStr = mono_string_to_utf8(texturePath);

		bool result = ONEngine::FontRasterizer::GenerateTexture(textStr, fontPathStr, fontSize, texPathStr);

		mono_free(textStr);
		mono_free(fontPathStr);
		mono_free(texPathStr);

		return result;
	}
}

using namespace ONEngine;
using namespace MonoInternalMethods;

#define ADD_INTERNAL_CALL(name) mono_add_internal_call(name, (void*)Internal##name)

namespace {
	void AddTransformInternalCalls() {
		mono_add_internal_call("Transform::InternalGetPosition", (void*)InternalGetPosition);
		mono_add_internal_call("Transform::InternalGetLocalPosition", (void*)InternalGetLocalPosition);
		mono_add_internal_call("Transform::InternalGetRotate", (void*)InternalGetRotate);
		mono_add_internal_call("Transform::InternalGetScale", (void*)InternalGetScale);
		mono_add_internal_call("Transform::InternalSetPosition", (void*)InternalSetPosition);
		mono_add_internal_call("Transform::InternalSetLocalPosition", (void*)InternalSetLocalPosition);
		mono_add_internal_call("Transform::InternalSetRotate", (void*)InternalSetRotate);
		mono_add_internal_call("Transform::InternalSetScale", (void*)InternalSetScale);
	}

	void AddMeshRendererInternalCalls() {
		mono_add_internal_call("MeshRenderer::InternalGetMeshName", (void*)InternalGetMeshName);
		mono_add_internal_call("MeshRenderer::InternalSetMeshName", (void*)InternalSetMeshName);
		mono_add_internal_call("MeshRenderer::InternalGetColor", (void*)InternalGetMeshColor);
		mono_add_internal_call("MeshRenderer::InternalSetColor", (void*)InternalSetMeshColor);
		mono_add_internal_call("MeshRenderer::InternalGetPostEffectFlags", (void*)InternalGetPostEffectFlags);
		mono_add_internal_call("MeshRenderer::InternalSetPostEffectFlags", (void*)InternalSetPostEffectFlags);
		mono_add_internal_call("MeshRenderer::InternalGetRenderQueue", (void*)InternalGetRenderQueue);
		mono_add_internal_call("MeshRenderer::InternalSetRenderQueue", (void*)InternalSetRenderQueue);
	}

	void AddSkinMeshInternalCalls() {
		mono_add_internal_call("SkinMeshRenderer::InternalGetMeshPath", (void*)InternalGetMeshPath);
		mono_add_internal_call("SkinMeshRenderer::InternalSetMeshPath", (void*)InternalSetMeshPath);
		mono_add_internal_call("SkinMeshRenderer::InternalGetTexturePath", (void*)InternalGetTexturePath);
		mono_add_internal_call("SkinMeshRenderer::InternalSetTexturePath", (void*)InternalSetTexturePath);
		mono_add_internal_call("SkinMeshRenderer::InternalGetIsPlaying", (void*)InternalGetIsPlaying);
		mono_add_internal_call("SkinMeshRenderer::InternalSetIsPlaying", (void*)InternalSetIsPlaying);
		mono_add_internal_call("SkinMeshRenderer::InternalGetAnimationTime", (void*)InternalGetAnimationTime);
		mono_add_internal_call("SkinMeshRenderer::InternalSetAnimationTime", (void*)InternalSetAnimationTime);
		mono_add_internal_call("SkinMeshRenderer::InternalGetAnimationScale", (void*)InternalGetAnimationScale);
		mono_add_internal_call("SkinMeshRenderer::InternalSetAnimationScale", (void*)InternalSetAnimationScale);
		mono_add_internal_call("SkinMeshRenderer::InternalGetJointTransform", (void*)InternalGetJointTransform);

		mono_add_internal_call("Animator::Internal_Play", (void*)Internal_Play);
		mono_add_internal_call("Animator::Internal_CrossFade", (void*)Internal_CrossFade);
		mono_add_internal_call("Animator::Internal_SetPlaybackSpeed", (void*)Internal_SetPlaybackSpeed);
		mono_add_internal_call("Animator::Internal_SetLoop", (void*)Internal_SetLoop);
		mono_add_internal_call("Animator::Internal_GetAnimationDuration", (void*)Internal_GetAnimationDuration);
	}

	void AddColliderInternalCalls() {
		mono_add_internal_call("SphereCollider::InternalGetRadius", (void*)InternalGetRadius);
		mono_add_internal_call("SphereCollider::InternalSetRadius", (void*)InternalSetRadius);
		mono_add_internal_call("SphereCollider::InternalIsTrigger", (void*)InternalIsTriggerSphere);
		mono_add_internal_call("SphereCollider::InternalSetTrigger", (void*)InternalSetTriggerSphere);
		mono_add_internal_call("SphereCollider::InternalGetMass", (void*)InternalGetMass);
		mono_add_internal_call("SphereCollider::InternalSetMass", (void*)InternalSetMass);
		mono_add_internal_call("SphereCollider::InternalIsUseOwnerScale", (void*)InternalIsUseOwnerScaleSphere);
		mono_add_internal_call("SphereCollider::InternalSetUseOwnerScale", (void*)InternalSetUseOwnerScaleSphere);

		mono_add_internal_call("BoxCollider::InternalGetSize", (void*)InternalGetSize);
		mono_add_internal_call("BoxCollider::InternalSetSize", (void*)InternalSetSize);
		mono_add_internal_call("BoxCollider::InternalIsTrigger", (void*)InternalIsTriggerBox);
		mono_add_internal_call("BoxCollider::InternalSetTrigger", (void*)InternalSetTriggerBox);
		mono_add_internal_call("BoxCollider::InternalGetMassBox", (void*)InternalGetMassBox);
		mono_add_internal_call("BoxCollider::InternalSetMassBox", (void*)InternalSetMassBox);
		mono_add_internal_call("BoxCollider::InternalIsUseOwnerScale", (void*)InternalIsUseOwnerScaleBox);
		mono_add_internal_call("BoxCollider::InternalSetUseOwnerScale", (void*)InternalSetUseOwnerScaleBox);

		/// CircleCollider
		mono_add_internal_call("CircleCollider::InternalGetRadius", (void*)InternalGetRadiusCircle);
		mono_add_internal_call("CircleCollider::InternalSetRadius", (void*)InternalSetRadiusCircle);
		mono_add_internal_call("CircleCollider::InternalIsTriggerCircle", (void*)InternalIsTriggerCircle);
		mono_add_internal_call("CircleCollider::InternalSetTriggerCircle", (void*)InternalSetTriggerCircle);
		mono_add_internal_call("CircleCollider::InternalGetMassCircle", (void*)InternalGetMassCircle);
		mono_add_internal_call("CircleCollider::InternalSetMassCircle", (void*)InternalSetMassCircle);
		mono_add_internal_call("CircleCollider::InternalIsUseOwnerScaleCircle", (void*)InternalIsUseOwnerScaleCircle);
		mono_add_internal_call("CircleCollider::InternalSetUseOwnerScaleCircle", (void*)InternalSetUseOwnerScaleCircle);


	}

	void AddAudioInternalCalls() {
		mono_add_internal_call("BGMPlayer::InternalGetParams", (void*)BGMPlayer_GetParams);
		mono_add_internal_call("BGMPlayer::InternalSetParams", (void*)BGMPlayer_SetParams);
		mono_add_internal_call("BGMPlayer::InternalPlay", (void*)BGMPlayer_Play);
		mono_add_internal_call("BGMPlayer::InternalStop", (void*)BGMPlayer_Stop);

		mono_add_internal_call("SEPlayer::InternalGetParams", (void*)SEPlayer_GetParams);
		mono_add_internal_call("SEPlayer::InternalSetParams", (void*)SEPlayer_SetParams);
		mono_add_internal_call("SEPlayer::InternalPlay", (void*)SEPlayer_Play);
		mono_add_internal_call("SEPlayer::InternalStop", (void*)SEPlayer_Stop);
		mono_add_internal_call("SEPlayer::InternalPlayOneShot", (void*)SEPlayer_PlayOneShot);
	}
}

void ONEngine::AddWindowInternalCalls() {
	mono_add_internal_call("Window::InternalGetSize", (void*)InternalGetWindowSize);
}

void ONEngine::AddComponentInternalCalls() {

	/// batch
	mono_add_internal_call("ComponentBatchManager::InternalSetBatch", (void*)InternalSetBatch);
	mono_add_internal_call("ComponentBatchManager::InternalGetBatch", (void*)InternalGetBatch);

	AddTransformInternalCalls();
	AddMeshRendererInternalCalls();
	AddSkinMeshInternalCalls();
	AddColliderInternalCalls();
	AddAudioInternalCalls();

	/// sprite renderer
	mono_add_internal_call("SpriteRenderer::InternalGetColor", (void*)InternalGetColor);
	mono_add_internal_call("SpriteRenderer::InternalSetColor", (void*)InternalSetColor);
	mono_add_internal_call("SpriteRenderer::InternalGetTextureSize", (void*)InternalGetTextureSize);
	mono_add_internal_call("SpriteRenderer::InternalGetPixelPerfect", (void*)InternalGetPixelPerfect);
	mono_add_internal_call("SpriteRenderer::InternalSetPixelPerfect", (void*)InternalSetPixelPerfect);

	/// text renderer (Temporarily commented out)
	// mono_add_internal_call("TextRenderer::InternalGetColor", (void*)InternalGetTextColor);
	// mono_add_internal_call("TextRenderer::InternalSetColor", (void*)InternalSetTextColor);
	// mono_add_internal_call("TextRenderer::InternalGetText", (void*)InternalGetTextText);
	// mono_add_internal_call("TextRenderer::InternalSetText", (void*)InternalSetTextText);
	// mono_add_internal_call("TextRenderer::InternalGetFontPath", (void*)InternalGetTextFontPath);
	// mono_add_internal_call("TextRenderer::InternalSetFontPath", (void*)InternalSetTextFontPath);
	// mono_add_internal_call("TextRenderer::InternalGetFontSize", (void*)InternalGetTextFontSize);
	// mono_add_internal_call("TextRenderer::InternalSetFontSize", (void*)InternalSetTextFontSize);

	/// font rasterizer
	mono_add_internal_call("FontRasterizer::Internal_GenerateTexture", (void*)InternalGenerateFontTexture);

}

void ONEngine::AddEntityInternalCalls() {
	/// entity
	mono_add_internal_call("Entity::InternalAddComponent", (void*)InternalAddComponent);
	mono_add_internal_call("Entity::InternalGetComponent", (void*)InternalGetComponent);
	mono_add_internal_call("Entity::InternalGetName", (void*)InternalGetName);
	mono_add_internal_call("Entity::InternalSetName", (void*)InternalSetName);
	mono_add_internal_call("Entity::InternalGetChildId", (void*)InternalGetChildId);
	mono_add_internal_call("Entity::InternalGetChildrenCount", (void*)InternalGetChildrenCount);
	mono_add_internal_call("Entity::InternalGetParentId", (void*)InternalGetParentId);
	mono_add_internal_call("Entity::InternalSetParent", (void*)InternalSetParent);
	mono_add_internal_call("Entity::InternalAddScript", (void*)InternalAddScript);
	mono_add_internal_call("Entity::InternalGetScript", (void*)InternalGetScript);
	mono_add_internal_call("Entity::InternalGetEnable", (void*)InternalGetEnable);
	mono_add_internal_call("Entity::InternalSetEnable", (void*)InternalSetEnable);
	mono_add_internal_call("UIGroupComponent::InternalSetVisible", (void*)UIGroupComponent_SetVisible);
	mono_add_internal_call("UIGroupComponent::InternalSetFocused", (void*)UIGroupComponent_SetFocused);
	mono_add_internal_call("UIElementComponent::InternalGetElementId", (void*)InternalGetElementId);

	mono_add_internal_call("ECSGroup::InternalCreateEntity", (void*)InternalCreateEntity);
	mono_add_internal_call("ECSGroup::InternalDestroyEntity", (void*)InternalDestroyEntity);
	mono_add_internal_call("ECSGroup::InternalGetRootEntityCount", (void*)InternalGetRootEntityCount);
	mono_add_internal_call("ECSGroup::InternalGetRootEntityId", (void*)InternalGetRootEntityId);

	// AI Debug
	mono_add_internal_call("BehaviorTree::Internal_UpdateNodeStatus", (void*)Internal_UpdateNodeStatus);
	mono_add_internal_call("Blackboard::Internal_UpdateBlackboardValue", (void*)Internal_UpdateBlackboardValue);
	mono_add_internal_call("BehaviorNode::Internal_OnBreakpointHit", (void*)Internal_OnBreakpointHit);
	}


void ONEngine::AddInputInternalCalls() {
	mono_add_internal_call("Input::InternalTriggerKey", (void*)Input::TriggerKey);
	mono_add_internal_call("Input::InternalPressKey", (void*)Input::PressKey);
	mono_add_internal_call("Input::InternalReleaseKey", (void*)Input::ReleaseKey);

	mono_add_internal_call("Input::InternalTriggerGamepad", (void*)Input::TriggerGamepad);
	mono_add_internal_call("Input::InternalPressGamepad", (void*)Input::PressGamepad);
	mono_add_internal_call("Input::InternalReleaseGamepad", (void*)Input::ReleaseGamepad);
	mono_add_internal_call("Input::InternalGetGamepadThumb", (void*)InternalGetGamepadThumb);

	mono_add_internal_call("Input::InternalTriggerMouse", (void*)Input::TriggerMouse);
	mono_add_internal_call("Input::InternalPressMouse", (void*)Input::PressMouse);
	mono_add_internal_call("Input::InternalReleaseMouse", (void*)Input::ReleaseMouse);
	mono_add_internal_call("Input::InternalGetMousePosition", (void*)InternalGetMousePosition);
	mono_add_internal_call("Input::InternalGetMouseVelocity", (void*)InternalGetMouseVelocity);
	mono_add_internal_call("Input::InternalGetMouseWheel", (void*)InternalGetMouseWheel);
}

void ONEngine::AddSceneInternalCalls() {
	mono_add_internal_call("SceneManager::InternalLoadScene", (void*)InternalLoadScene);
	mono_add_internal_call("SceneManager::InternalAddScene", (void*)InternalAddScene);
	mono_add_internal_call("SceneManager::InternalUnloadScene", (void*)InternalUnloadScene);
	mono_add_internal_call("SceneManager::InternalSetUpdatePaused", (void*)InternalSetUpdatePaused);
	mono_add_internal_call("SceneManager::InternalIsUpdatePaused", (void*)InternalIsUpdatePaused);
	mono_add_internal_call("SceneManager::InternalSetDrawPaused", (void*)InternalSetDrawPaused);
	mono_add_internal_call("SceneManager::InternalIsDrawPaused", (void*)InternalIsDrawPaused);
}

void ONEngine::AddGizmoInternalCalls() {
	mono_add_internal_call("GizmoBatch::Internal_SubmitLineBatch", (void*)Internal_SubmitLineBatch);
}

void ONEngine::AddAnimationInternalCalls(); // implementation in AnimationInternalCalls.cpp

