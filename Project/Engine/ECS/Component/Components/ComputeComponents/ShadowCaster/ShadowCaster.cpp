#include "ShadowCaster.h"

/// external
#include <imgui.h>

/// engine
#include "Engine/Core/Config/EngineConfig.h"
#include "Engine/ECS/Entity/GameEntity/GameEntity.h"
#include "Engine/ECS/EntityComponentSystem/EntityComponentSystem.h"
#include "Engine/ECS/Component/Components/ComputeComponents/Light/Light.h"
#include "Engine/Editor/Commands/ImGuiCommand/ImGuiCommand.h"

using namespace ONEngine;

namespace {

	Vector2 CalcAspectSize(float aspectRatio, const Vector2& size) {
		Vector2 result{};
		if (size.x > 0.0f) {
			result.x = size.x;
			result.y = size.y / aspectRatio;
		} else if (size.y > 0.0f) {
			result.y = size.y;
			result.x = result.y * aspectRatio;
		} else {
			result = EngineConfig::kWindowSize;
		}

		return result;
	}

}


void ComponentDebug::ShadowCasterDebug(ShadowCaster* shadowCaster) {
	if (!shadowCaster) {
		return;
	}


	if (ImGui::Button("toggle camera type")) {
		ShadowCaster* shadowCaster = shadowCaster;
		CameraComponent* camera = shadowCaster->GetShadowCasterCamera();
		if (camera) {
			int currentType = camera->GetCameraType();
			if (currentType == static_cast<int>(CameraType::Type2D)) {
				camera->SetCameraType(static_cast<int>(CameraType::Type3D));
			} else {
				camera->SetCameraType(static_cast<int>(CameraType::Type2D));
			}
		}
	}


	if (ImGui::Button("create camera")) {
		shadowCaster->CreateShadowCaster();
	}


	ImGui::SeparatorText("light parameter");

	Editor::ImMathf::DragFloat3("base light pos", &shadowCaster->baseLightPos_, 1.0f, -10000.0f, 10000.0f);
	Editor::ImMathf::DragFloat("light length", &shadowCaster->lightLength_, 1.0f, 0.0f, 10000.0f);


	ImGui::SeparatorText("shadow caster parameters");

	Editor::ImMathf::DragFloat2("window size", &shadowCaster->orthographicSize_, 1.0f, 0.0f, 0.0f);
	Editor::ImMathf::DragFloat("scale factor", &shadowCaster->scaleFactor_, 0.01f, 0.0f, 10.0f);


	ImGui::Spacing();

	Editor::ImMathf::DragFloat2("texel size", &shadowCaster->texelSizeShadow_, 0.01f, 0.0f, 0.0f);
	Editor::ImMathf::DragFloat("shadow bias", &shadowCaster->shadowBias_, 0.001f, 0.0f, 1.0f);
	Editor::ImMathf::DragFloat("shadow darkness", &shadowCaster->shadowDarkness_, 0.01f, 0.0f, 1.0f);
	Editor::ImMathf::DragInt("pcf radius", &shadowCaster->pcfRadius_, 1, 0, 10);

}

void ONEngine::from_json(const nlohmann::json& j, ShadowCaster& c) {
	c.enable = j.value("enable", static_cast<int>(true));

	c.orthographicSize_ = j.value("orthographicSize", EngineConfig::kWindowSize);
	c.scaleFactor_ = j.value("scaleFactor", 1.0f);
	c.texelSizeShadow_ = j.value("texelSizeShadow", Vector2(1.0f / EngineConfig::kWindowSize.x, 1.0f / EngineConfig::kWindowSize.y));
	c.shadowBias_ = j.value("shadowBias", 0.005f);
	c.shadowDarkness_ = j.value("shadowDarkness", 0.7f);
	c.pcfRadius_ = j.value("pcfRadius", 2);
	c.baseLightPos_ = j.value("baseLightPos", Vector3(500, 0, 500));
	c.lightLength_ = j.value("lightLength", 300.0f);

}

void ONEngine::to_json(nlohmann::json& j, const ShadowCaster& c) {
	j = {
		{ "type", "ShadowCaster" },
		{ "enable", c.enable },
		{ "orthographicSize", c.orthographicSize_ },
		{ "scaleFactor", c.scaleFactor_ },
		{ "texelSizeShadow", c.texelSizeShadow_ },
		{ "shadowBias", c.shadowBias_ },
		{ "shadowDarkness", c.shadowDarkness_ },
		{ "pcfRadius", c.pcfRadius_ },
		{ "baseLightPos", c.baseLightPos_ },
		{ "lightLength", c.lightLength_ }
	};
}


/// ///////////////////////////////////////////////////
/// 影の投影を行うためのコンポーネント
/// ///////////////////////////////////////////////////

ShadowCaster::ShadowCaster()
	: camera_(nullptr),
	isCreated_(false),
	orthographicSize_(EngineConfig::kWindowSize),
	texelSizeShadow_(Vector2(1.0f / EngineConfig::kWindowSize.x, 1.0f / EngineConfig::kWindowSize.y)),
	shadowBias_(0.005f),
	shadowDarkness_(0.7f),
	pcfRadius_(2),
	baseLightPos_(Vector3(500, 0, 500)),
	lightLength_(300) {

};

ShadowCaster::~ShadowCaster() = default;

void ShadowCaster::CreateShadowCaster() {
	/// 生成済みかチェック
	if (!isCreated_) {
		isCreated_ = true;
	} else {
		//Console::LogWarning("ShadowCaster::CreateShadowCaster: ShadowCaster is already created.");
		return;
	}

	camera_ = GetOwner()->AddComponent<CameraComponent>();
	camera_->SetCameraType(static_cast<int>(CameraType::Type2D));
	camera_->SetIsMainCameraRequest(false); /// シャドウキャスター用カメラはメインカメラにしない
	const float factor = 0.01f;
	camera_->SetOrthographicSize(Vector2::HD * factor);
}

void ShadowCaster::CalculationLightViewMatrix(ECSGroup* ecsGroup, DirectionalLight* directionLight) {
	/// nullチェック
	if (!directionLight) {
		Console::LogError("ShadowCaster::CalculationLightViewMatrix: DirectionalLight is null");
		return;
	}


	/// オーナーの位置を取得し、ライトの方向を基にカメラの回転を計算
	GameEntity* owner = GetOwner();

	const Vector3& dir = directionLight->GetDirection();
	Quaternion cameraRotation = Quaternion::LookAt(
		{}, dir,
		Vector3::Up
	);

	/// カメラの回転を設定
	owner->SetRotate(cameraRotation);

	/// 現在のグループのmain cameraを取得し、座標を補正
	CameraComponent* mainCamera = ecsGroup->GetMainCamera();
	GameEntity* mainCamEntity = mainCamera->GetOwner();
	mainCamEntity->UpdateTransform();
	Vector3 mainCamPos = mainCamEntity->GetPosition();

	/// ライトの方向に合わせて座標を計算
	owner->SetPosition(mainCamPos + baseLightPos_ - dir * lightLength_);

	camera_->SetOrthographicSize(orthographicSize_ * scaleFactor_);
	camera_->UpdateViewProjection();

}

CameraComponent* ShadowCaster::GetShadowCasterCamera() {
	return camera_;
}

ShadowParameter ShadowCaster::GetShadowParameters() const {
	return ShadowParameter{
		.screenSize = EngineConfig::kWindowSize,
		.texelSizeShadow = texelSizeShadow_,
		.shadowBias = shadowBias_,
		.shadowDarkness = shadowDarkness_,
		.pcfRadius = pcfRadius_,
	};
}

