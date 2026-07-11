#include "ComponentApplyFunc.h"

/// std
#include <unordered_map>
#include <memory>

/// engine
#include "Engine/Core/Utility/Utility.h"

/// components
#include "Engine/Asset/Collection/AssetCollection.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Transform/Transform.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Agent/AgentIntentComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Camera/CameraComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Animator/Animator.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/MeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Mesh/DissolveMeshRenderer.h"
#include "Engine/ECS/Component/Components/RendererComponents/Sprite/SpriteRenderer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/Script/MonoScriptEngine.h"


#include "Engine/Graphics/Buffer/Data/UVTransform.h"

using namespace ONEngine;

namespace {

std::unordered_map<MonoClass*, ComponentApplyFunc> gApplyFuncMap;
std::unordered_map<MonoClass*, ComponentFetchFunc> gFetchFuncMap;
std::unordered_map<MonoClass*, size_t> gComponentBatchSize;


struct TransformBatch {
	uint32_t compId;
	Vector3 position;
	Quaternion rotate;
	Vector3 scale;
	Matrix4x4 matWorld;
};

struct MeshRendererBatch {
	uint32_t compId;
	Vector4 color;
	uint32_t postEffectFlags;
	UVTransform uvTransform;
};

struct DissolveBatch {
	uint32_t compId;
	float threshold;
	UVTransform uvTransform;
};

struct SpriteBatch {
	uint32_t compId;
	Vector4 color;
	Vector2 textureSize;
	UVTransform uvTransform;
};

struct CameraBatch {
	uint32_t compId;
	Matrix4x4 matVP;
	Matrix4x4 matView;
	Matrix4x4 matProjection;
	float fovY;
	float nearClip;
	float farClip;
	int cameraType;
};

struct AnimatorBatch {
    uint32_t compId;
    AnimationLayer layers[MAX_ANIMATION_LAYERS];
};

struct BoxCollider2DBatch {
	uint32_t compId;
	Vector2 size;
	int32_t isTrigger;
	float mass;
	int32_t useOwnerScale;
};

} /// unnamed namespace


void ComponentApplyFuncs::ApplyTransform(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<TransformBatch*>(element);
	auto* tArray = ecsGroup->GetComponentArray<Transform>();
	if(!CheckComponentArrayEnable(tArray)) {
		return;
	}

	if(Transform* t = tArray->GetComponent(data->compId)) {
		t->SetPosition(data->position);
		t->SetRotate(data->rotate);
		t->SetScale(data->scale);
		// matWorldはC++側で再計算されるためC#からの値で上書きしない
	}
}

void ComponentApplyFuncs::ApplyMeshRenderer(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<MeshRendererBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<MeshRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(MeshRenderer* mr = array->GetComponent(data->compId)) {
		mr->SetColor(data->color);
		mr->SetPostEffectFlags(data->postEffectFlags);
		mr->SetUVTransform(data->uvTransform);
	}
}

void ONEngine::ComponentApplyFuncs::ApplyDissolve(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<DissolveBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<DissolveMeshRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(DissolveMeshRenderer* mr = array->GetComponent(data->compId)) {
		mr->SetThreshold(data->threshold);
		mr->SetUVTransform(data->uvTransform);
	}
}

void ONEngine::ComponentApplyFuncs::ApplySprite(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<SpriteBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<SpriteRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(SpriteRenderer* sr = array->GetComponent(data->compId)) {
		sr->SetColor(data->color);
		sr->SetUVTransform(data->uvTransform);
	}
}

void ONEngine::ComponentApplyFuncs::ApplyAgentIntent(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<AgentIntentComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<AgentIntentComponent>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(AgentIntentComponent* ai = array->GetComponent(data->compId)) {
		ai->desiredMoveDirection = data->desiredMoveDirection;
		ai->desiredRotation = data->desiredRotation;
		ai->rotationSpeed = data->rotationSpeed;
		ai->maxSpeed = data->maxSpeed;
		ai->useDesiredRotation = (data->useDesiredRotation != 0);
		ai->isAttacking = (data->isAttacking != 0);
		ai->targetEntityId = data->targetEntityId;
	}
}

void ONEngine::ComponentApplyFuncs::ApplyCamera(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<CameraBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<CameraComponent>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(CameraComponent* camera = array->GetComponent(data->compId)) {
		// matView と matProjection は現状 C++ 側で計算されていることが多いが、
		// C# 側から上書きしたい場合のために実装しておく
		// ただし、UpdateViewProjection で上書きされる可能性があるので注意

		camera->fovY_ = data->fovY;
		camera->nearClip_ = data->nearClip;
		camera->farClip_ = data->farClip;
		camera->cameraType_ = data->cameraType;
	}
}

void ONEngine::ComponentApplyFuncs::ApplyAnimator(void* element, ECSGroup* ecsGroup) {
    auto* data = static_cast<AnimatorBatch*>(element);
    auto* array = ecsGroup->GetComponentArray<Animator>();
    if (!CheckComponentArrayEnable(array)) return;

    if (Animator* animator = array->GetComponent(data->compId)) {
        for (uint32_t i = 0; i < MAX_ANIMATION_LAYERS; ++i) {
            animator->layers[i] = data->layers[i];
        }
    }
}

void ONEngine::ComponentApplyFuncs::ApplyUIGroup(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<UIGroupComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<UIGroupComponent>();
	if (!CheckComponentArrayEnable(array)) return;

	if (UIGroupComponent* comp = array->GetComponent(data->compId)) {
		comp->isFocused = (data->isFocused != 0);
		comp->isVisible = (data->isVisible != 0);
		comp->currentSelected = ecsGroup->GetEntity(data->currentSelectedId);
		comp->parentGroup = ecsGroup->GetEntity(data->parentGroupId);
		if (comp->currentSelected) comp->currentSelectedGuid = comp->currentSelected->GetGuid();
		if (comp->parentGroup) comp->parentGroupGuid = comp->parentGroup->GetGuid();
	}
}

void ONEngine::ComponentApplyFuncs::ApplyUIElement(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<UIElementComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<UIElementComponent>();
	if (!CheckComponentArrayEnable(array)) return;

	if (UIElementComponent* comp = array->GetComponent(data->compId)) {
		comp->elementIndex = data->elementIndex;
		comp->groupEntity = ecsGroup->GetEntity(data->groupIdId);
		if (comp->groupEntity) comp->groupId = comp->groupEntity->GetGuid();
	}
}

void ONEngine::ComponentApplyFuncs::FetchTransform(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<TransformBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<Transform>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(Transform* t = array->GetComponent(data->compId)) {
		data->position = t->GetPosition();
		data->rotate = t->GetRotate();
		data->scale = t->GetScale();
		data->matWorld = t->GetMatWorld();
	} else {
		Console::LogWarning("[SYNC_ERROR] FetchTransform: Component with ID " + std::to_string(data->compId) + " not found in group " + ecsGroup->GetGroupName());
	}
}

void ONEngine::ComponentApplyFuncs::FetchMeshRenderer(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<MeshRendererBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<MeshRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(MeshRenderer* mr = array->GetComponent(data->compId)) {
		data->color = mr->GetColor();
		data->postEffectFlags = mr->GetPostEffectFlags();
		data->uvTransform = mr->GetUVTransform();
	}
}

void ONEngine::ComponentApplyFuncs::FetchDissolve(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<DissolveBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<DissolveMeshRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(DissolveMeshRenderer* mr = array->GetComponent(data->compId)) {
		data->threshold = mr->GetDissolveThreshold();
		data->compId = mr->GetDissolveCompare();
		data->uvTransform = mr->GetUVTransform();
	}
}

void ONEngine::ComponentApplyFuncs::FetchSprite(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<SpriteBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<SpriteRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(SpriteRenderer* sr = array->GetComponent(data->compId)) {
		data->color = sr->GetColor();
		data->textureSize = sr->GetTextureSize(Asset::AssetCollection::GetInstance());
		data->uvTransform = sr->GetUVTransform();
	}
}

void ONEngine::ComponentApplyFuncs::FetchAgentIntent(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<AgentIntentComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<AgentIntentComponent>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(AgentIntentComponent* ai = array->GetComponent(data->compId)) {
		data->desiredMoveDirection = ai->desiredMoveDirection;
		data->desiredRotation = ai->desiredRotation;
		data->rotationSpeed = ai->rotationSpeed;
		data->maxSpeed = ai->maxSpeed;
		data->useDesiredRotation = ai->useDesiredRotation;
		data->isAttacking = ai->isAttacking;
		data->targetEntityId = ai->targetEntityId;
	} else {
		Console::LogWarning("[SYNC_ERROR] FetchAgentIntent: Component with ID " + std::to_string(data->compId) + " not found in group " + ecsGroup->GetGroupName());
	}
}

void ONEngine::ComponentApplyFuncs::FetchCamera(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<CameraBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<CameraComponent>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(CameraComponent* camera = array->GetComponent(data->compId)) {
		if (camera->IsMakeViewProjection()) {
			const ViewProjection& vp = camera->GetViewProjection();
			data->matVP = vp.matVP;
			data->matView = vp.matView;
			data->matProjection = vp.matProjection;
		}
		else {
			data->matVP = Matrix4x4::kIdentity;
			data->matView = Matrix4x4::kIdentity;
			data->matProjection = Matrix4x4::kIdentity;
		}

		data->fovY = camera->fovY_;
		data->nearClip = camera->nearClip_;
		data->farClip = camera->farClip_;
		data->cameraType = camera->cameraType_;
	}
}

void ONEngine::ComponentApplyFuncs::FetchAnimator(void* element, ECSGroup* ecsGroup) {
    auto* data = static_cast<AnimatorBatch*>(element);
    auto* array = ecsGroup->GetComponentArray<Animator>();
    if (!CheckComponentArrayEnable(array)) return;

    if (Animator* animator = array->GetComponent(data->compId)) {
        for (uint32_t i = 0; i < MAX_ANIMATION_LAYERS; ++i) {
            data->layers[i] = animator->layers[i];
        }
    }
}

void ONEngine::ComponentApplyFuncs::FetchUIGroup(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<UIGroupComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<UIGroupComponent>();
	if (!CheckComponentArrayEnable(array)) return;

	if (UIGroupComponent* comp = array->GetComponent(data->compId)) {
		data->isFocused = comp->isFocused ? 1 : 0;
		data->isVisible = comp->isVisible ? 1 : 0;
		data->currentSelectedId = comp->currentSelected ? comp->currentSelected->GetId() : 0;
		data->parentGroupId = comp->parentGroup ? comp->parentGroup->GetId() : 0;
	}
}

void ONEngine::ComponentApplyFuncs::FetchUIElement(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<UIElementComponent::BatchData*>(element);
	auto* array = ecsGroup->GetComponentArray<UIElementComponent>();
	if (!CheckComponentArrayEnable(array)) return;

	if (UIElementComponent* comp = array->GetComponent(data->compId)) {
		data->elementIndex = comp->elementIndex;
		data->groupIdId = comp->groupEntity ? comp->groupEntity->GetId() : 0;
	}
}

void ONEngine::ComponentApplyFuncs::ApplyBoxCollider2D(void* element, ECSGroup* ecsGroup) {
	BoxCollider2DBatch* data = static_cast<BoxCollider2DBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<BoxCollider2D>();
	if (!array) return;

	if (BoxCollider2D* comp = array->GetComponent(data->compId)) {
		comp->SetSize(data->size);
		comp->SetTrigger(data->isTrigger != 0);
		comp->SetMass(data->mass);
		comp->SetUseOwnerScale(data->useOwnerScale != 0);
	}
}

void ONEngine::ComponentApplyFuncs::FetchBoxCollider2D(void* element, ECSGroup* ecsGroup) {
	BoxCollider2DBatch* data = static_cast<BoxCollider2DBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<BoxCollider2D>();
	if (!array) return;

	if (BoxCollider2D* comp = array->GetComponent(data->compId)) {
		data->size = comp->GetSize();
		data->isTrigger = comp->IsTrigger() ? 1 : 0;
		data->mass = comp->GetMass();
		data->useOwnerScale = comp->IsUseOwnerScale() ? 1 : 0;
	}
}

ComponentApplyFunc ComponentApplyFuncs::GetApplyFunc(MonoClass* monoClass) {
	auto itr = gApplyFuncMap.find(monoClass);
	if(itr == gApplyFuncMap.end()) {
		return nullptr;
	}
	return itr->second;
}

ComponentFetchFunc ONEngine::ComponentApplyFuncs::GetFetchFunc(MonoClass* monoClass) {
	auto itr = gFetchFuncMap.find(monoClass);
	if(itr == gFetchFuncMap.end()) {
		return nullptr;
	}
	return itr->second;
}

size_t ONEngine::ComponentApplyFuncs::GetBatchElementSize(MonoClass* monoClass) {
	auto itr = gComponentBatchSize.find(monoClass);
	if(itr == gComponentBatchSize.end()) {
		return 0;
	}
	return itr->second;
}

void ONEngine::ComponentApplyFuncs::Initialize(MonoImage* monoImage) {
	gApplyFuncMap.clear();
	gFetchFuncMap.clear();
	gComponentBatchSize.clear();

	{	/// Transform
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "Transform");
		gApplyFuncMap[monoClass] = ApplyTransform;
		gFetchFuncMap[monoClass] = FetchTransform;
		gComponentBatchSize[monoClass] = sizeof(TransformBatch);
	}

	{	/// MeshRenderer
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "MeshRenderer");
		gApplyFuncMap[monoClass] = ApplyMeshRenderer;
		gFetchFuncMap[monoClass] = FetchMeshRenderer;
		gComponentBatchSize[monoClass] = sizeof(MeshRendererBatch);
	}

	{	/// DissolveMeshRenderer
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "DissolveMeshRenderer");
		gApplyFuncMap[monoClass] = ApplyDissolve;
		gFetchFuncMap[monoClass] = FetchDissolve;
		gComponentBatchSize[monoClass] = sizeof(DissolveBatch);
	}

	{	/// SpriteRenderer
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "SpriteRenderer");
		gApplyFuncMap[monoClass] = ApplySprite;
		gFetchFuncMap[monoClass] = FetchSprite;
		gComponentBatchSize[monoClass] = sizeof(SpriteBatch);
	}

	{	/// AgentIntentComponent
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "AgentIntentComponent");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyAgentIntent;
			gFetchFuncMap[monoClass] = FetchAgentIntent;
			gComponentBatchSize[monoClass] = sizeof(AgentIntentComponent::BatchData);
		}
	}

	{	/// CameraComponent
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "CameraComponent");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyCamera;
			gFetchFuncMap[monoClass] = FetchCamera;
			gComponentBatchSize[monoClass] = sizeof(CameraBatch);
		}
	}

	{	/// Animator
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "Animator");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyAnimator;
			gFetchFuncMap[monoClass] = FetchAnimator;
			gComponentBatchSize[monoClass] = sizeof(AnimatorBatch);
		}
	}

	{	/// UIGroupComponent
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "UIGroupComponent");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyUIGroup;
			gFetchFuncMap[monoClass] = FetchUIGroup;
			gComponentBatchSize[monoClass] = sizeof(UIGroupComponent::BatchData);
		}
	}

	{	/// UIElementComponent
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "UIElementComponent");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyUIElement;
			gFetchFuncMap[monoClass] = FetchUIElement;
			gComponentBatchSize[monoClass] = sizeof(UIElementComponent::BatchData);
		}
	}

	{	/// BoxCollider2D
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "BoxCollider2D");
		if (monoClass) {
			gApplyFuncMap[monoClass] = ApplyBoxCollider2D;
			gFetchFuncMap[monoClass] = FetchBoxCollider2D;
			gComponentBatchSize[monoClass] = sizeof(BoxCollider2DBatch);
		}
	}
}
