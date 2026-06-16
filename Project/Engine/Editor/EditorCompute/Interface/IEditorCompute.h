#pragma once

/// std
#include <memory>

/// engine
#include "Engine/Graphics/Shader/ComputePipeline.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"

namespace ONEngine {
class DxManager;
class DxCommand;
class EntityComponentSystem;
}

namespace ONEngine::Asset {
class AssetCollection;
}


/// /////////////////////////////////////////////////
/// コンピュートシェーダーを利用したエディターのパイプライン
/// /////////////////////////////////////////////////
namespace Editor {

class IEditorCompute {
public:
	/// =========================================
	/// public : methods
	/// =========================================

	IEditorCompute() = default;
	virtual ~IEditorCompute() = default;

	virtual void Initialize(ONEngine::ShaderCompiler* shaderCompiler, ONEngine::DxManager* dxm) = 0;
	virtual void Execute(ONEngine::EntityComponentSystem* ecs, ONEngine::DxCommand* dxCommand, ONEngine::Asset::AssetCollection* assetCollection) = 0;


protected:
	/// =========================================
	/// protected : objects
	/// =========================================

	std::unique_ptr<ONEngine::ComputePipeline> pipeline_;

};

} /// Editor
