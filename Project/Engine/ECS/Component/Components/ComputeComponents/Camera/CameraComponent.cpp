#include "CameraComponent.h"

#include <array>

/// externals
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/Core/Utility/Math/Math.h"
#include "Engine/Core/Utility/Math/Primitive.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/Editor/EditorUtils.h"

using namespace ONEngine;

namespace {


/// @brief ViewProjection行列から視錐台を作成する
/// @param matVP ViewProjection行列
/// @return Frustum
Frustum CreateFrustumFromMatrix(const Matrix4x4& matVP) {
	Frustum frustum;

	// Left
	frustum.planes[0].normal.x = matVP.m[0][3] + matVP.m[0][0];
	frustum.planes[0].normal.y = matVP.m[1][3] + matVP.m[1][0];
	frustum.planes[0].normal.z = matVP.m[2][3] + matVP.m[2][0];
	frustum.planes[0].d = matVP.m[3][3] + matVP.m[3][0];

	// Right
	frustum.planes[1].normal.x = matVP.m[0][3] - matVP.m[0][0];
	frustum.planes[1].normal.y = matVP.m[1][3] - matVP.m[1][0];
	frustum.planes[1].normal.z = matVP.m[2][3] - matVP.m[2][0];
	frustum.planes[1].d = matVP.m[3][3] - matVP.m[3][0];

	// Bottom
	frustum.planes[2].normal.x = matVP.m[0][3] + matVP.m[0][1];
	frustum.planes[2].normal.y = matVP.m[1][3] + matVP.m[1][1];
	frustum.planes[2].normal.z = matVP.m[2][3] + matVP.m[2][1];
	frustum.planes[2].d = matVP.m[3][3] + matVP.m[3][1];

	// Top
	frustum.planes[3].normal.x = matVP.m[0][3] - matVP.m[0][1];
	frustum.planes[3].normal.y = matVP.m[1][3] - matVP.m[1][1];
	frustum.planes[3].normal.z = matVP.m[2][3] - matVP.m[2][1];
	frustum.planes[3].d = matVP.m[3][3] - matVP.m[3][1];

	// Near
	frustum.planes[4].normal.x = matVP.m[0][2];
	frustum.planes[4].normal.y = matVP.m[1][2];
	frustum.planes[4].normal.z = matVP.m[2][2];
	frustum.planes[4].d = matVP.m[3][2];

	// Far
	frustum.planes[5].normal.x = matVP.m[0][3] - matVP.m[0][2];
	frustum.planes[5].normal.y = matVP.m[1][3] - matVP.m[1][2];
	frustum.planes[5].normal.z = matVP.m[2][3] - matVP.m[2][2];
	frustum.planes[5].d = matVP.m[3][3] - matVP.m[3][2];

	// 法線を正規化
	for(auto& p : frustum.planes) {
		p.d /= p.normal.Length(); // Normalize 内で0除算済みでも安全のため
		p.normal = p.normal.Normalize();
	}

	return frustum;
}


/// @brief 視錐台の情報をImGuiで表示する
/// @param frustum 表示元のデータ
void ImGuiShowFrustum(const Frustum& frustum) {
	if(ImGui::CollapsingHeader("Frustum")) {
		const char* names[6] = { "Left", "Right", "Bottom", "Top", "Near", "Far" };
		std::string clipboardText;

		for(int i = 0; i < 6; ++i) {
			ImGui::Text("%s plane:", names[i]);
			ImGui::Text("  Normal: %.3f, %.3f, %.3f",
						frustum.planes[i].normal.x,
						frustum.planes[i].normal.y,
						frustum.planes[i].normal.z);
			ImGui::Text("  Distance: %.3f", frustum.planes[i].d);

			// クリップボード用文字列を作成
			clipboardText += names[i];
			clipboardText += " plane: ";
			clipboardText += "Normal(";
			clipboardText += std::to_string(frustum.planes[i].normal.x) + ", ";
			clipboardText += std::to_string(frustum.planes[i].normal.y) + ", ";
			clipboardText += std::to_string(frustum.planes[i].normal.z) + "), ";
			clipboardText += "Distance(" + std::to_string(frustum.planes[i].d) + ")\n";
		}

		// Clipboardコピー用ボタン
		if(ImGui::Button("Copy Frustum Data to Clipboard")) {
			ImGui::SetClipboardText(clipboardText.c_str());
		}
	}
}


}	/// namespace


void ComponentDebug::CameraDebug(CameraComponent* camera) {
	if(!camera) {
		return;
	}

	Editor::ImMathf::DragFloat("fovY", &camera->fovY_, 0.01f, 0.1f, Math::PI);
	Editor::ImMathf::DragFloat("near clip", &camera->nearClip_, 0.01f, 0.01f, 100.0f);
	Editor::ImMathf::DragFloat("far clip", &camera->farClip_, 0.01f, 100.0f, 10000.0f);


	ImGui::Spacing();

	/// type debug
	ImGui::Combo("camera type", &camera->cameraType_, "3D\0 2D\0");

	ImGui::Spacing();

	if(ImGui::Button("main camera setting")) {
		camera->SetIsMainCameraRequest(true);
	}


	/// frustum debug
	ImGui::Checkbox("Draw Frustum", &camera->isDrawFrustum_);

	if(ImGui::CollapsingHeader("Frustum Debug")) {
		Frustum frustum = CreateFrustumFromMatrix(camera->GetViewProjection().matVP);
		ImGuiShowFrustum(frustum);
	}

	if(ImGui::CollapsingHeader("Matrix Debug")) {

		/// 行列のデバッグ表示
		const ViewProjection& vp = camera->GetViewProjection();
		ImGui::Text("View Matrix:");
		for(int i = 0; i < 4; ++i) {
			ImGui::Text("%.3f %.3f %.3f %.3f", vp.matView.m[i][0], vp.matView.m[i][1], vp.matView.m[i][2], vp.matView.m[i][3]);
		}

		/// View行列をClipboardにコピーする
		if(ImGui::Button("Copy View Matrix to Clipboard")) {
			std::string viewMatrixStr;
			for(int i = 0; i < 4; ++i) {
				viewMatrixStr += std::format("{:.6f} {:.6f} {:.6f} {:.6f}\n", vp.matView.m[i][0], vp.matView.m[i][1], vp.matView.m[i][2], vp.matView.m[i][3]);
			}
			ImGui::SetClipboardText(viewMatrixStr.c_str());
		}


		ImGui::Text("Projection Matrix:");
		for(int i = 0; i < 4; ++i) {
			ImGui::Text("%.3f %.3f %.3f %.3f", vp.matProjection.m[i][0], vp.matProjection.m[i][1], vp.matProjection.m[i][2], vp.matProjection.m[i][3]);
		}

		/// Projection行列をClipboardにコピーする
		if(ImGui::Button("Copy Projection Matrix to Clipboard")) {
			std::string projectionMatrixStr;
			for(int i = 0; i < 4; ++i) {
				projectionMatrixStr += std::format("{:.6f} {:.6f} {:.6f} {:.6f}\n", vp.matProjection.m[i][0], vp.matProjection.m[i][1], vp.matProjection.m[i][2], vp.matProjection.m[i][3]);
			}
			ImGui::SetClipboardText(projectionMatrixStr.c_str());
		}
	}


	if(ImGui::CollapsingHeader("FogParams")) {
		Vector3& color = camera->fogParams_.color;
		float& fogStart = camera->fogParams_.fogStart;
		float& fogEnd = camera->fogParams_.fogEnd;

		ImGui::ColorEdit3("Fog Color Edit", &color.x);
		Editor::DragFloat("Fog Start", fogStart, 0.1f, 0.0f, 1000.0f);
		Editor::DragFloat("Fog End", fogEnd, 0.1f, 0.0f, 1000.0f);
	}

}

void ONEngine::from_json(const nlohmann::json& j, CameraComponent& c) {
	c.isMainCameraRequest_ = j.value("isMainCamera", true);
	c.fovY_ = j.value("fovY", 0.7f);
	c.nearClip_ = j.value("nearClip", 0.1f);
	c.farClip_ = j.value("farClip", 1000.0f);
	c.cameraType_ = j.value("cameraType", static_cast<int>(CameraType::Type3D));
	c.isDrawFrustum_ = j.value("isDrawFrustum", false);
	c.fogParams_ = {
		.color = j.value("fogColor", Vector3::One),
		.fogStart = j.value("fogStart", 0.0f),
		.fogEnd = j.value("fogEnd", 1000.0f)
	};
}

void ONEngine::to_json(nlohmann::json& j, const CameraComponent& c) {
	j = nlohmann::json{
		{ "type", "CameraComponent" },
		{ "enable", c.enable },
		{ "fovY", c.fovY_ },
		{ "nearClip", c.nearClip_ },
		{ "farClip", c.farClip_ },
		{ "cameraType", c.cameraType_ },
		{ "isMainCamera", c.isMainCameraRequest_ },
		{ "isDrawFrustum", c.isDrawFrustum_ },
		{ "fogColor", c.fogParams_.color },
		{ "fogStart", c.fogParams_.fogStart },
		{ "fogEnd", c.fogParams_.fogEnd }
	};
}



/// ///////////////////////////////////////////////////
/// カメラのコンポーネント
/// ///////////////////////////////////////////////////
CameraComponent::CameraComponent() {
	/// デフォルト値を設定
	fovY_ = 0.7f;
	nearClip_ = 0.1f;
	farClip_ = 1000.0f;
	isMainCameraRequest_ = true;
	cameraType_ = static_cast<int>(CameraType::Type3D);
	isDrawFrustum_ = false;
	orthographicSize_ = EngineConfig::kWindowSize;
	fogParams_.fogStart = 0.0f;
	fogParams_.fogEnd = 10000.0f;
}
CameraComponent::~CameraComponent() {}

void CameraComponent::UpdateViewProjection() {
	GameEntity* entity = GetOwner();
	if(!entity) {
		return; // エンティティが存在しない場合は何もしない
	}

	entity->UpdateTransform(); /// transformの更新し忘れ防止
	matView_ = entity->GetTransform()->GetMatWorld().Inverse();

	if(cameraType_ == static_cast<int>(CameraType::Type3D)) {
		/// 3Dカメラの場合
		matProjection_ = CameraMath::MakePerspectiveFovMatrix(
			fovY_, EngineConfig::kWindowSize.x / EngineConfig::kWindowSize.y,
			nearClip_, farClip_
		);

	} else {
		/// 2Dカメラの場合

		matProjection_ = CameraMath::MakeOrthographicMatrix(
			-(orthographicSize_.x / 2.0f), (orthographicSize_.x / 2.0f),
			-(orthographicSize_.y / 2.0f), (orthographicSize_.y / 2.0f),
			nearClip_, farClip_
		);

	}

	viewProjection_.SetMappedData(ViewProjection(matView_ * matProjection_, matView_, matProjection_));
	Vector4 cameraPos = Math::ConvertToVector4(entity->GetPosition(), 1.0f);
	cameraPosBuffer_.SetMappedData(cameraPos);
	cBufferFogParams_.SetMappedData(fogParams_);
}

bool CameraComponent::IsVisible(const Vector3& center, const Vector3& size) const {
	Vector3 min = center - (size / 2.0f);
	Vector3 max = center + (size / 2.0f);
	Frustum frustum = CreateFrustumFromMatrix(GetViewProjection().matVP);

	Vector3 vertices[8] = {
		min,
		Vector3(max.x, min.y, min.z),
		Vector3(min.x, max.y, min.z),
		Vector3(max.x, max.y, min.z),
		Vector3(min.x, min.y, max.z),
		Vector3(max.x, min.y, max.z),
		Vector3(min.x, max.y, max.z),
		max
	};

	for(int i = 0; i < 6; ++i) {
		bool allOutside = true;
		for(int v = 0; v < 8; ++v) {
			float d = Vector3::Dot(frustum.planes[i].normal, vertices[v]) + frustum.planes[i].d;
			if(d >= 0) {
				allOutside = false;
				break;
			}
		}
		if(allOutside) {
			return false;
		}
	}
	return true;
}

void CameraComponent::LookAt(const Vector3& direction) {
	Transform* transform = GetOwner()->GetComponent<Transform>();
	if(!transform) { return; }

	/// ディレクション方向に向ける
	transform->rotate = Quaternion::LookAt(Vector3::Zero, direction);
}

void CameraComponent::MakeViewProjection(DxDevice* dxDevice) {
	viewProjection_.Create(dxDevice);
	viewProjection_.SetMappedData(ViewProjection(
		Matrix4x4::kIdentity
	));

	cameraPosBuffer_.Create(dxDevice);
	Vector4 cameraPos = Math::ConvertToVector4(GetOwner()->GetPosition(), 1.0f);
	cameraPosBuffer_.SetMappedData(cameraPos);

	cBufferFogParams_.Create(dxDevice);
	cBufferFogParams_.SetMappedData(FogParams{});
}


/// ///////////////////////////////////////////////////
/// CameraMath
/// ///////////////////////////////////////////////////

Matrix4x4 CameraMath::MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip) {
	/// ----- 透視投影行列の作成 ----- ///

	return Matrix4x4(
		(1 / aspectRatio) * Math::Cot(fovY / 2.0f), 0.0f, 0.0f, 0.0f,
		0.0f, Math::Cot(fovY / 2.0f), 0.0f, 0.0f,
		0.0f, 0.0f, farClip / (farClip - nearClip), 1.0f,
		0.0f, 0.0f, (-nearClip * farClip) / (farClip - nearClip), 0.0f
	);
}

Matrix4x4 CameraMath::MakeOrthographicMatrix(float left, float right, float bottom, float top, float znear, float zfar) {
	/// ----- 平行投影行列の作成 ----- ///

	Matrix4x4 result = {};

	float width = right - left;
	float height = top - bottom;
	float depth = zfar - znear;

	result.m[0][0] = 2.0f / width;
	result.m[1][1] = 2.0f / height;
	result.m[2][2] = 1.0f / depth;
	result.m[3][0] = -(right + left) / width;
	result.m[3][1] = -(top + bottom) / height;
	result.m[3][2] = -znear / depth;
	result.m[3][3] = 1.0f;

	return result;
}

void CameraComponent::SetIsMainCameraRequest(bool isMainCamera) {
	isMainCameraRequest_ = isMainCamera;
}

void CameraComponent::SetCameraType(int cameraType) {
	cameraType_ = cameraType;
}

void CameraComponent::SetOrthographicSize(const Vector2& size) {
	orthographicSize_ = size;
}

bool CameraComponent::GetIsMainCameraRequest() const {
	return isMainCameraRequest_;
}

int CameraComponent::GetCameraType() const {
	return cameraType_;
}

bool CameraComponent::IsMakeViewProjection() const {
	return viewProjection_.Get() != nullptr;
}

const ViewProjection& CameraComponent::GetViewProjection() const {
	return viewProjection_.GetMappingData();
}

ConstantBuffer<ViewProjection>& CameraComponent::GetViewProjectionBuffer() {
	return viewProjection_;
}

ConstantBuffer<Vector4>& CameraComponent::GetCameraPosBuffer() {
	return cameraPosBuffer_;
}

ConstantBuffer<CameraComponent::FogParams>& ONEngine::CameraComponent::GetFogParamsBuffer() {
	return cBufferFogParams_;
}

const Matrix4x4& CameraComponent::GetViewMatrix() const {
	return matView_;
}

const Matrix4x4& CameraComponent::GetProjectionMatrix() const {
	return matProjection_;
}

