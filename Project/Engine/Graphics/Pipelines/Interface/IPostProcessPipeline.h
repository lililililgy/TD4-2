#pragma once

/// std
#include <memory>
#include <string>

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



namespace ONEngine {

/// ///////////////////////////////////////////////////
/// PostProcessのinterfaceクラス
/// ///////////////////////////////////////////////////
class IPostProcessPipeline {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	virtual ~IPostProcessPipeline() = default;

	/// @brief pipelineの初期化を行う
	/// @param shaderCompiler ShaderCompilerへのポインタ
	/// @param dxm DxManagerへのポインタ
	virtual void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) = 0;

	/// @brief post processの実行
	virtual void Execute(
		const std::string& textureName,
		DxCommand* dxCommand,
		Asset::AssetCollection* assetCollection,
		EntityComponentSystem* pEntityComponentSystem
	) = 0;

protected:
	/// ===================================================
	/// protected : objects
	/// ===================================================

	std::unique_ptr<ComputePipeline> pipeline_;

};


/// ===================================================
/// 部分的なpost processとスクリーンに適用するpost processのinterfaceを統一
/// ===================================================
using PerObjectPostProcess = IPostProcessPipeline;
using ScreenPostProcess = IPostProcessPipeline;


/// @brief リソースの内容をコピーする
/// @param src 大本のリソース
/// @param dst コピー元のリソース
/// @param cmdList CommandListのポインタ
void CopyResource(ID3D12Resource* src, ID3D12Resource* dst, ID3D12GraphicsCommandList6* cmdList);

} /// ONEngine
