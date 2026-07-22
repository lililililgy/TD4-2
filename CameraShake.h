#pragma once
#include "system/ISystem.h"

/// <summary>
/// CameraShakeSourceComponentで カメラシェイクを制御するシステム
/// </summary>
class CameraShake
	: public OriGine::ISystem{
public:
	CameraShake();
	~CameraShake() override;

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private:
	/// <summary>
	/// CameraShakeSourceComponentを持つEntityごとにシェイクを計算し、CameraTransformへ反映する
	/// </summary>
	/// <param name="_handle">対象のエンティティハンドル</param>
	void UpdateEntity(const OriGine::EntityHandle& _handle) override;
};