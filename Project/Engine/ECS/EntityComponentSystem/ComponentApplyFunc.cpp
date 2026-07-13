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
#include "Engine/ECS/Component/Components/RendererComponents/Text/TextRenderer.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIGroupComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/UI/UIElementComponent.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Collision/BoxCollider2D.h"
#include "Engine/Script/MonoScriptEngine.h"


#include "Engine/Graphics/Buffer/Data/UVTransform.h"

using namespace ONEngine;

namespace {

std::unordered_map<std::string, ComponentApplyFunc> gApplyFuncMap;
std::unordered_map<std::string, ComponentFetchFunc> gFetchFuncMap;
std::unordered_map<std::string, size_t> gComponentBatchSize;

static void Register(MonoClass* monoClass, ComponentApplyFunc applyFunc, ComponentFetchFunc fetchFunc, size_t batchSize) {
	if (!monoClass) return;
	std::string className = mono_class_get_name(monoClass);
	gApplyFuncMap[className] = applyFunc;
	gFetchFuncMap[className] = fetchFunc;
	gComponentBatchSize[className] = batchSize;
}


#pragma pack(push, 4)
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

struct TextBatch {
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
#pragma pack(pop)

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

void ONEngine::ComponentApplyFuncs::ApplyText(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<TextBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<TextRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(TextRenderer* tr = array->GetComponent(data->compId)) {
		tr->SetColor(data->color);
		tr->SetUVTransform(data->uvTransform);
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

void ONEngine::ComponentApplyFuncs::FetchText(void* element, ECSGroup* ecsGroup) {
	auto* data = static_cast<TextBatch*>(element);
	auto* array = ecsGroup->GetComponentArray<TextRenderer>();
	if(!CheckComponentArrayEnable(array)) {
		return;
	}

	if(TextRenderer* tr = array->GetComponent(data->compId)) {
		data->color = tr->GetColor();
		data->textureSize = tr->GetTextureSize(Asset::AssetCollection::GetInstance());
		data->uvTransform = tr->GetUVTransform();
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
	if (!monoClass) return nullptr;
	std::string className = mono_class_get_name(monoClass);
	auto itr = gApplyFuncMap.find(className);
	if(itr == gApplyFuncMap.end()) {
		return nullptr;
	}
	return itr->second;
}

ComponentFetchFunc ONEngine::ComponentApplyFuncs::GetFetchFunc(MonoClass* monoClass) {
	if (!monoClass) return nullptr;
	std::string className = mono_class_get_name(monoClass);
	auto itr = gFetchFuncMap.find(className);
	if(itr == gFetchFuncMap.end()) {
		return nullptr;
	}
	return itr->second;
}

size_t ONEngine::ComponentApplyFuncs::GetBatchElementSize(MonoClass* monoClass) {
	if (!monoClass) return 0;
	std::string className = mono_class_get_name(monoClass);
	auto itr = gComponentBatchSize.find(className);
	if(itr != gComponentBatchSize.end() && itr->second > 0) {
		return itr->second;
	}

	// 安全のためのハードコードフォールバック（アセンブリリロード時などのキー紛失・アライメントズレ防止）
	if (className == "Transform") return sizeof(TransformBatch);
	if (className == "MeshRenderer") return sizeof(MeshRendererBatch);
	if (className == "DissolveMeshRenderer") return sizeof(DissolveBatch);
	if (className == "SpriteRenderer") return sizeof(SpriteBatch);
	if (className == "TextRenderer") return sizeof(TextBatch);
	if (className == "AgentIntentComponent") return sizeof(AgentIntentComponent::BatchData);
	if (className == "CameraComponent") return sizeof(CameraBatch);
	if (className == "Animator") return sizeof(AnimatorBatch);
	if (className == "UIGroupComponent") return sizeof(UIGroupComponent::BatchData);
	if (className == "UIElementComponent") return sizeof(UIElementComponent::BatchData);
	if (className == "BoxCollider2D") return sizeof(BoxCollider2DBatch);

	Console::LogError("[JIT_DEBUG] GetBatchElementSize - Unknown component class name requested: " + className);
	return 64; // アボート防止用のデフォルトの安全アライメントサイズ
}

void ONEngine::ComponentApplyFuncs::Initialize(MonoImage* monoImage) {
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(TransformBatch) = {}", sizeof(TransformBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(MeshRendererBatch) = {}", sizeof(MeshRendererBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(DissolveBatch) = {}", sizeof(DissolveBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(SpriteBatch) = {}", sizeof(SpriteBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(TextBatch) = {}", sizeof(TextBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(CameraBatch) = {}", sizeof(CameraBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(AnimatorBatch) = {}", sizeof(AnimatorBatch)));
	Console::Log(std::format("[JIT_DEBUG] C++ sizeof(BoxCollider2DBatch) = {}", sizeof(BoxCollider2DBatch)));

	gApplyFuncMap.clear();
	gFetchFuncMap.clear();
	gComponentBatchSize.clear();

	Register(mono_class_from_name(monoImage, "", "Transform"), ApplyTransform, FetchTransform, sizeof(TransformBatch));
	Register(mono_class_from_name(monoImage, "", "MeshRenderer"), ApplyMeshRenderer, FetchMeshRenderer, sizeof(MeshRendererBatch));
	Register(mono_class_from_name(monoImage, "", "DissolveMeshRenderer"), ApplyDissolve, FetchDissolve, sizeof(DissolveBatch));
	Register(mono_class_from_name(monoImage, "", "SpriteRenderer"), ApplySprite, FetchSprite, sizeof(SpriteBatch));

	{	/// TextRenderer
		MonoClass* monoClass = mono_class_from_name(monoImage, "", "TextRenderer");
		Console::Log(std::format("[JIT_DEBUG] TextRenderer MonoClass pointer: {}", (void*)monoClass));
		if (monoClass) {
			Register(monoClass, ApplyText, FetchText, sizeof(TextBatch));
			Console::Log(std::format("[JIT_DEBUG] sizeof(TextBatch) = {}", sizeof(TextBatch)));
			Console::Log(std::format("[JIT_DEBUG] sizeof(SpriteBatch) = {}", sizeof(SpriteBatch)));
			Console::Log("[JIT_DEBUG] Registered TextRenderer in maps successfully.");
		} else {
			Console::LogError("[JIT_DEBUG] Failed to find TextRenderer MonoClass!");
		}
	}

	Register(mono_class_from_name(monoImage, "", "AgentIntentComponent"), ApplyAgentIntent, FetchAgentIntent, sizeof(AgentIntentComponent::BatchData));
	Register(mono_class_from_name(monoImage, "", "CameraComponent"), ApplyCamera, FetchCamera, sizeof(CameraBatch));
	Register(mono_class_from_name(monoImage, "", "Animator"), ApplyAnimator, FetchAnimator, sizeof(AnimatorBatch));
	Register(mono_class_from_name(monoImage, "", "UIGroupComponent"), ApplyUIGroup, FetchUIGroup, sizeof(UIGroupComponent::BatchData));
	Register(mono_class_from_name(monoImage, "", "UIElementComponent"), ApplyUIElement, FetchUIElement, sizeof(UIElementComponent::BatchData));
	Register(mono_class_from_name(monoImage, "", "BoxCollider2D"), ApplyBoxCollider2D, FetchBoxCollider2D, sizeof(BoxCollider2DBatch));
}
