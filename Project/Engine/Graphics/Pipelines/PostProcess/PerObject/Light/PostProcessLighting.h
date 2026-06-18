#pragma once

///std
#include <array>
#include <cstdint>
#include <memory>

/// engine
#include "Engine/Graphics/Pipelines/Interface/IPostProcessPipeline.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/Buffer/StructuredBuffer.h"
#include "Engine/Core/Utility/Math/Vector3.h"
#include "Engine/Core/Utility/Math/Vector4.h"

/// ///////////////////////////////////////////////////
/// PostProcessLighting
/// ///////////////////////////////////////////////////
namespace ONEngine {

class PostProcessLighting : public PerObjectPostProcess {
private:
	/// ===================================================
	/// private : sub class
	/// ===================================================

	struct DirectionalLightBufferData final {
		Vector4 position;
		Vector4 color;
		Vector3 direction;
		float intensity;
	};

	struct PointLightBufferData final {
		Vector4 position;
		Vector4 color;
		float intensity;
		float radius;
		float padding[2];
	};

	struct SpotLightBufferData final {
		Vector4 position;
		Vector4 color;
		Vector3 direction;
		float intensity;
		float radius;
		float innerAngle;
		float outerAngle;
		float padding;
	};

	struct LightCountBufferData final {
		uint32_t directionalLightCount;
		uint32_t pointLightCount;
		uint32_t spotLightCount;
		uint32_t padding;
	};

	struct CameraBufferData final {
		Vector4 position;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	PostProcessLighting();
	~PostProcessLighting();
	
	/// @brief pipelineの初期化を行う
	/// @param shaderCompiler ShaderCompilerへのポインタ
	/// @param dxm DxManagerへのポインタ
	void Initialize(ShaderCompiler* shaderCompiler, DxManager* dxm) override;
	
	/// @brief post processの実行
	void Execute(
		const std::string& textureName,
		DxCommand* dxCommand, 
		Asset::AssetCollection* assetCollection,
		EntityComponentSystem* pEntityComponentSystem
	) override;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::array<size_t, 6> textureIndices_;
	std::unique_ptr<ConstantBuffer<LightCountBufferData>> lightCountBufferData_;
	std::unique_ptr<StructuredBuffer<DirectionalLightBufferData>> directionalLightBuffer_;
	std::unique_ptr<StructuredBuffer<PointLightBufferData>> pointLightBuffer_;
	std::unique_ptr<StructuredBuffer<SpotLightBufferData>> spotLightBuffer_;
	std::unique_ptr<ConstantBuffer<CameraBufferData>> cameraBufferData_;

};


} /// ONEngine
