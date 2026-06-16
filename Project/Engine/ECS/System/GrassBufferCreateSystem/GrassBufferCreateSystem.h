#pragma once

/// engine
#include "Engine/ECS/System/Interface/ECSISystem.h"

/// /////////////////////////////////////////////////////
/// 草のBufferを生成するシステム
/// /////////////////////////////////////////////////////
namespace ONEngine {

class GrassBufferCreateSystem : public ECSISystem {
public:
	/// ==================================================
	/// public : methods
	/// ==================================================

	GrassBufferCreateSystem(class DxManager* dxm);
	~GrassBufferCreateSystem() override;

	void OutsideOfRuntimeUpdate(ECSGroup* ecs) override;
	void RuntimeUpdate(ECSGroup* ecs) override;

private:
	/// ==================================================
	/// private : objects
	/// ==================================================

	class DxManager* pDxManager_;

};

} /// ONEngine
