#pragma once

/// std
#include <string>

/// externals
#include <mono/jit/jit.h>

/*
* C#側のComponentアタッチ関数が呼び出されたときにC++側で処理を行うComponentごとの関数群
*/

namespace ONEngine {

class ECSGroup;

using ComponentApplyFunc = void(*)(void* data, ECSGroup* ecsGroup);
using ComponentFetchFunc = void(*)(void* data, ECSGroup* ecsGroup);
//using ComponentApplyFunc = void(*)(MonoObject* element, MonoClass* class, ECSGroup* ecsGroup);

namespace ComponentApplyFuncs {

/// Apply

void ApplyTransform(void* element, ECSGroup* ecsGroup);
void ApplyMeshRenderer(void* element, ECSGroup* ecsGroup);
void ApplyDissolve(void* element, ECSGroup* ecsGroup);
void ApplySprite(void* element, ECSGroup* ecsGroup);
void ApplyText(void* element, ECSGroup* ecsGroup);
void ApplyAgentIntent(void* element, ECSGroup* ecsGroup);
void ApplyCamera(void* element, ECSGroup* ecsGroup);
void ApplyAnimator(void* element, ECSGroup* ecsGroup);
void ApplyUIGroup(void* element, ECSGroup* ecsGroup);
void ApplyUIElement(void* element, ECSGroup* ecsGroup);
void ApplyBoxCollider2D(void* element, ECSGroup* ecsGroup);

/// Fetch

void FetchTransform(void* element, ECSGroup* ecsGroup);
void FetchMeshRenderer(void* element, ECSGroup* ecsGroup);
void FetchDissolve(void* element, ECSGroup* ecsGroup);
void FetchSprite(void* element, ECSGroup* ecsGroup);
void FetchText(void* element, ECSGroup* ecsGroup);
void FetchAgentIntent(void* element, ECSGroup* ecsGroup);
void FetchCamera(void* element, ECSGroup* ecsGroup);
void FetchAnimator(void* element, ECSGroup* ecsGroup);
void FetchUIGroup(void* element, ECSGroup* ecsGroup);
void FetchUIElement(void* element, ECSGroup* ecsGroup);
void FetchBoxCollider2D(void* element, ECSGroup* ecsGroup);


ComponentApplyFunc GetApplyFunc(MonoClass* monoClass);
ComponentFetchFunc GetFetchFunc(MonoClass* monoClass);

size_t GetBatchElementSize(MonoClass* monoClass);

void Initialize(MonoImage* monoImage);
}


} /// namespace ONEngine