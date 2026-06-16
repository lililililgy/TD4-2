#pragma once

/// engine
#include "../Interface/ECSISystem.h"

/// ///////////////////////////////////////////////////
/// カメラ更新システム
/// ///////////////////////////////////////////////////
namespace ONEngine {

class CameraUpdateSystem : public ECSISystem {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	CameraUpdateSystem(class DxDevice* dxDevice);
	~CameraUpdateSystem() override = default;

	void OutsideOfRuntimeUpdate(class ECSGroup* ecs) override;
	void RuntimeUpdate(class ECSGroup* ecs) override;

	void Update(class ECSGroup* ecs);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================
	
	class DxDevice* pDxDevice_;
	class CameraComponent* pMainCamera_;
	class CameraComponent* pMainCamera2D_;

};


} /// ONEngine
