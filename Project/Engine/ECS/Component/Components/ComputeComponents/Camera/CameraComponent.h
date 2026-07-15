#pragma once

/// externals
#include <nlohmann/json.hpp>

/// engine
#include "../../Interface/IComponent.h"
#include "Engine/Graphics/Buffer/ConstantBuffer.h"
#include "Engine/Graphics/Buffer/Data/ViewProjection.h"
#include <Engine/Core/Utility/Math/Vector3.h>


/// ----- 前方宣言 ----- ///
namespace ONEngine {

class CameraComponent;

/// @brief カメラの種類
enum class CameraType {
	Type3D, ///< 3Dカメラ
	Type2D, ///< 2Dカメラ
};


/// @brief Componentのデバッグ関数
namespace ComponentDebug {
void CameraDebug(CameraComponent* camera);
}

namespace ComponentApplyFuncs {
void ApplyCamera(void* element, class ECSGroup* ecsGroup);
void FetchCamera(void* element, class ECSGroup* ecsGroup);
}

/// Json変換
void from_json(const nlohmann::json& j, CameraComponent& c);
void to_json(nlohmann::json& j, const CameraComponent& c);


/// ///////////////////////////////////////////////////
/// カメラのコンポーネント
/// ///////////////////////////////////////////////////
class CameraComponent : public IComponent {
	/// ----- friend class ----- ///
	friend class CameraUpdateSystem;

	/// ----- friend function ----- ///
	friend void ComponentDebug::CameraDebug(CameraComponent* camera);
	friend void ComponentApplyFuncs::ApplyCamera(void* element, class ECSGroup* ecsGroup);
	friend void ComponentApplyFuncs::FetchCamera(void* element, class ECSGroup* ecsGroup);
	friend void from_json(const nlohmann::json& j, CameraComponent& c);
	friend void to_json(nlohmann::json& j, const CameraComponent& c);

public:

	struct FogParams {
		Vector3 color;   // フォグの色
		float fogStart; // フォグの開始距離
		float fogEnd;   // フォグの終了距離
		int32_t offsetX;
		int32_t offsetY;
	};

public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	CameraComponent();
	~CameraComponent() override;

	/// @brief ViewProjection行列の更新
	void UpdateViewProjection();


	/// @brief カメラのフラスタム内にあるか判定を撮る
	/// @param center 対象の中心
	/// @param size 対象の大きさ
	/// @return true: カメラから見える、 false: カメラから見えない
	bool IsVisible(const Vector3& center, const Vector3& size) const;

	/// @brief カメラを特定方向に向ける
	/// @param direction 向ける方向
	void LookAt(const Vector3& direction);

private:
	/// ===================================================
	/// private : methods
	/// ===================================================

	/// @brief ViewProjectionのBufferを作成
	/// @param dxDevice Buffer作成に使うDxDevice
	void MakeViewProjection(class DxDevice* dxDevice);


private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	ConstantBuffer<ViewProjection> viewProjection_;
	ConstantBuffer<Vector4> cameraPosBuffer_;
	ConstantBuffer<FogParams> cBufferFogParams_;

	float fovY_;
	float nearClip_;
	float farClip_;

	Matrix4x4 matView_;
	Matrix4x4 matProjection_;

	int cameraType_;
	bool isMainCameraRequest_;
	bool isDrawFrustum_;

	Vector2 orthographicSize_;

	/// fog parameters
	FogParams fogParams_;

public:
	/// ====================================================
	/// public : accessor
	/// ====================================================

	void SetIsMainCameraRequest(bool isMainCamera);
	void SetCameraType(int cameraType);
	void SetOrthographicSize(const Vector2& size);


	bool GetIsMainCameraRequest() const;
	int GetCameraType() const;

	bool IsMakeViewProjection() const;

	const ViewProjection& GetViewProjection() const;
	ConstantBuffer<ViewProjection>& GetViewProjectionBuffer();

	ConstantBuffer<Vector4>& GetCameraPosBuffer();
	ConstantBuffer<FogParams>& GetFogParamsBuffer();
	const FogParams& GetFogParams() const { return fogParams_; }

	const Matrix4x4& GetViewMatrix() const;
	const Matrix4x4& GetProjectionMatrix() const;

};



/// @brief カメラ関連の数学関数群
namespace CameraMath {

/// @brief perspective matrix の作成
/// @param fovY 視野角
/// @param aspectRatio アスペクト比 
/// @param nearClip 最小描画距離
/// @param farClip 最大描画距離
/// @return 作成された perspective matrix
Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

/// @brief 平行投影行列の作成
/// @param left 左
/// @param right 右
/// @param bottom 下
/// @param top 上
/// @param znear 手前
/// @param zfar 奥行き
/// @return 平行投影行列
Matrix4x4 MakeOrthographicMatrix(float left, float right, float bottom, float top, float znear, float zfar);

}


} /// ONEngine
