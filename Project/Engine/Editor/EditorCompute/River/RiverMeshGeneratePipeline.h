#pragma once

#include "../Interface/IEditorCompute.h"

/// //////////////////////////////////////////
/// 川のメッシュを作成するpipeline
/// //////////////////////////////////////////
namespace Editor {

class RiverMeshGeneratePipeline : public IEditorCompute {

	enum ROOT_PARAM {
		CBV_PARAMS,
		SRV_CONTROL_POINTS,
		UAV_VERTICES,
		UAV_INDICES,
	};

public:
	/// =========================================
	/// public : methods
	/// =========================================

	RiverMeshGeneratePipeline();
	~RiverMeshGeneratePipeline();

	void Initialize(ONEngine::ShaderCompiler* shaderCompiler, ONEngine::DxManager* dxm) override;
	void Execute(ONEngine::EntityComponentSystem* ecs, ONEngine::DxCommand* dxCommand, ONEngine::Asset::AssetCollection* assetCollection) override;

private:
	/// =========================================
	/// private : objects
	/// =========================================

	ONEngine::DxManager* pDxManager_;

};

} /// Editor
