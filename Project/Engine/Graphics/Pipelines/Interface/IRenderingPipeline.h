#pragma once

/// std
#include <memory>

/// engine
#include "Engine/Core/DirectX12/Command/DxCommand.h"
#include "Engine/Graphics/Shader/GraphicsPipeline.h"
#include "Engine/Graphics/Shader/ShaderCompiler.h"



/// ///////////////////////////////////////////////////
/// 描画の interface クラス
/// ///////////////////////////////////////////////////
namespace ONEngine {

class IRenderingPipeline {
public:
	/// ===================================================
	/// public : virtual methods
	/// ===================================================

	virtual ~IRenderingPipeline() = default;

	/// @brief このクラスの初期化関数
	/// @param shaderCompiler シェーダーのコンパイラーへのポインタ
	virtual void Initialize(ShaderCompiler* shaderCompiler, class DxManager* dxm) = 0;

	/// @brief 描画前の処理を行う
	/// @param ecs ECSGroupへのポインタ
	/// @param camera 描画に用いるカメラへのポインタ
	/// @param dxCommand DxCommandへのポインタ
	virtual void PreDraw(class ECSGroup* /*ecs*/, class CameraComponent* /*camera*/, DxCommand* /*dxCommand*/) {}

	/// @brief 描画処理を行う
	/// @param dxCommand DxCommandへのポインタ
	/// @param entityCollection EntityCollectionへのポインタ
	virtual void Draw(class ECSGroup* ecs, class CameraComponent* camera, DxCommand* dxCommand) = 0;

protected:
	/// ===================================================
	/// protected : objects
	/// ===================================================

	std::unique_ptr<GraphicsPipeline> pipeline_;
};



} /// ONEngine
